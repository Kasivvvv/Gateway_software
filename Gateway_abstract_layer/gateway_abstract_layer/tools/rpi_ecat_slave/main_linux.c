/*
 * EtherCAT test slave for Raspberry Pi (Linux): raw Ethernet socket +
 * software ESC + SOES, 16 bytes of process data per direction (echo). Partner of the gateway in configurations 3 and 4.
 *
 *   sudo ./rpi_ecat_slave [interface]      (default: eth0)
 *
 * Loop: wait up to 1 ms for an EtherCAT frame (EtherType 0x88A4), let the
 * soft-ESC process it in place and send it back on the same port (the slave
 * is the last one on the line, like an ESC with port 1 closed). SOES runs
 * after every frame and at least every 1 ms. A status line is printed once
 * per second, including the per-frame processing time (receive -> send).
 */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "slave_app.h"

#define ETH_P_ECAT_TYPE     0x88A4
#define FRAME_MAX           1518
#define POLL_TIMEOUT_MS     1
#define RT_PRIORITY         80
/* SOES software watchdog in slave_app_poll() calls (~1-2 calls per ms). */
#define SOES_WATCHDOG_CALLS 100U

static volatile sig_atomic_t s_run = 1;

static void on_signal(int sig)
{
    (void)sig;
    s_run = 0;
}

static uint64_t now_ns(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
}

static int open_socket(const char *ifname, struct sockaddr_ll *sll)
{
    int fd;
    int one = 1;
    struct packet_mreq mr;
    unsigned int ifindex = if_nametoindex(ifname);

    if (ifindex == 0U)
    {
        fprintf(stderr, "unknown interface %s\n", ifname);
        return -1;
    }

    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ECAT_TYPE));
    if (fd < 0)
    {
        perror("socket(AF_PACKET) - run with sudo");
        return -1;
    }

    memset(sll, 0, sizeof(*sll));
    sll->sll_family = AF_PACKET;
    sll->sll_protocol = htons(ETH_P_ECAT_TYPE);
    sll->sll_ifindex = (int)ifindex;
    sll->sll_halen = ETH_ALEN;
    memset(sll->sll_addr, 0xFF, ETH_ALEN);
    if (bind(fd, (struct sockaddr *)sll, sizeof(*sll)) < 0)
    {
        perror("bind");
        close(fd);
        return -1;
    }

    /* Bypass the qdisc layer: lower and steadier transmit latency. */
    (void)setsockopt(fd, SOL_PACKET, PACKET_QDISC_BYPASS, &one, sizeof(one));

    /* Promiscuous: also accept frames whose destination is not broadcast. */
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = (int)ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    (void)setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr));

    return fd;
}

static void go_realtime(void)
{
    struct sched_param sp;

    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
    {
        perror("mlockall (continuing)");
    }
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = RT_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0)
    {
        perror("SCHED_FIFO (continuing without RT priority)");
    }
}

int main(int argc, char **argv)
{
    const char *ifname = (argc > 1) ? argv[1] : "eth0";
    struct sockaddr_ll sll;
    uint8_t frame[FRAME_MAX];
    uint64_t nextReport;
    uint32_t frames = 0U;
    uint32_t sendErrors = 0U;
    uint64_t procMaxNs = 0U;
    uint64_t procSumNs = 0U;
    uint32_t procCount = 0U;
    int fd;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    fd = open_socket(ifname, &sll);
    if (fd < 0)
    {
        return 1;
    }
    go_realtime();

    slave_app_init(SOES_WATCHDOG_CALLS);
    printf("rpi_ecat_slave on %s: soft-ESC + SOES, 16 B out / 16 B in (echo)\n", ifname);

    nextReport = now_ns() + 1000000000ULL;
    while (s_run)
    {
        struct pollfd pfd = { .fd = fd, .events = POLLIN, .revents = 0 };
        int pr = poll(&pfd, 1, POLL_TIMEOUT_MS);

        if ((pr > 0) && ((pfd.revents & POLLIN) != 0))
        {
            struct sockaddr_ll from;
            socklen_t fromLen = sizeof(from);
            ssize_t n = recvfrom(fd, frame, sizeof(frame), 0,
                    (struct sockaddr *)&from, &fromLen);
            uint64_t t0 = now_ns();

            if ((n > 0) && (from.sll_pkttype != PACKET_OUTGOING)
                    && soft_esc_process_frame(slave_app_esc(), frame, (uint16_t)n))
            {
                if (sendto(fd, frame, (size_t)n, 0, (struct sockaddr *)&sll,
                        sizeof(sll)) < 0)
                {
                    sendErrors++;
                }
                else
                {
                    uint64_t dt = now_ns() - t0;
                    frames++;
                    procSumNs += dt;
                    procCount++;
                    if (dt > procMaxNs)
                    {
                        procMaxNs = dt;
                    }
                }
            }
        }
        else if ((pr < 0) && (errno != EINTR))
        {
            perror("poll");
            break;
        }

        slave_app_poll();

        if (now_ns() >= nextReport)
        {
            const soft_esc_stats_t *st = &slave_app_esc()->stats;

            nextReport += 1000000000ULL;
            printf("AL=0x%02X frames=%u dg=%u denied=%u sii=%u outUpd=%u out=%016llX:%016llX "
                   "in=%016llX proc_avg=%.1fus proc_max=%.1fus txerr=%u\n",
                   slave_app_al_status(), frames, st->datagrams, st->denied, st->siiReads,
                   slave_app_outputs_updates(), (unsigned long long)slave_app_outputs(),
                   (unsigned long long)slave_app_outputs2(),
                   (unsigned long long)slave_app_inputs(),
                   (procCount != 0U) ? ((double)procSumNs / procCount / 1000.0) : 0.0,
                   (double)procMaxNs / 1000.0, sendErrors);
            fflush(stdout);
            procSumNs = 0U;
            procCount = 0U;
            procMaxNs = 0U;
        }
    }

    close(fd);
    printf("stopped\n");
    return 0;
}
