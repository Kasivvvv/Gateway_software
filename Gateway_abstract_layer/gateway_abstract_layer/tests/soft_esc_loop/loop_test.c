/*
 * Host co-simulation: gateway EtherCAT master (ecat_master.c + SOEM + SOEM
 * port) <-> wire <-> soft-ESC <-> SOES test slave, in one process.
 *
 * The "wire" is synchronous: hwa_eth_tx() lets the soft-ESC process the frame
 * and queues it for hwa_eth_rx(). Simulated time advances 1 us per clock read
 * (so SOEM busy-waits progress) and 200 us per Tloop; SOES runs every 50 us.
 * Pass criteria: master reaches OP with the expected WKC, the counter echo
 * comes back, no missed cycles once in OP.
 */
#include <stdio.h>
#include <string.h>

#include "hwa_interface.h"
#include "ecat_master.h"
#include "slave_app.h"

#define RXQ 32
static uint8_t  s_q[RXQ][1514];
static uint16_t s_ql[RXQ];
static unsigned s_qh, s_qt;
static uint64_t s_now, s_nextSlave;
static int      s_inSlave;
static bool     s_link = true;

static void slave_tick(void)
{
    if (!s_inSlave && (s_now >= s_nextSlave))
    {
        s_inSlave = 1;
        s_nextSlave = s_now + 50U;
        slave_app_poll();
        s_inSlave = 0;
    }
}

uint64_t hwa_time_now_us(void) { s_now++; slave_tick(); return s_now; }

size_t hwa_console_write_str(const char *s)
{
    printf("[%9.3f ms] %s", (double)s_now / 1000.0, s);
    return strlen(s);
}

void hwa_eth_link_get(hwa_eth_link_t *l) { l->up = s_link; l->fullDuplex = true; l->speedMbps = 100; }

bool hwa_eth_tx(const uint8_t *f, uint16_t len)
{
    uint8_t *b;
    if (!s_link || (s_qh - s_qt) >= RXQ) { return false; }
    b = s_q[s_qh % RXQ];
    memcpy(b, f, len);
    if (len < 60) { memset(b + len, 0, 60U - len); len = 60; } /* GMAC padding */
    (void)soft_esc_process_frame(slave_app_esc(), b, len);
    s_ql[s_qh % RXQ] = len;
    s_qh++;
    return true;
}

uint16_t hwa_eth_rx(uint8_t *f, uint16_t max)
{
    uint16_t l;
    if (s_qt == s_qh) { return 0; }
    l = s_ql[s_qt % RXQ];
    if (l > max) { l = max; }
    memcpy(f, s_q[s_qt % RXQ], l);
    s_qt++;
    return l;
}

static void report(const char *tag)
{
    ecat_master_stats_t s;
    ecat_master_stats_get(&s);
    printf("%-10s st=%s pwkc=%d cfg=%u cfg_us=%u sl=%d io=%u cyc=%u ok=%u miss=%u wkc=%d/%d out=%u in=%u lag=%u | AL=0x%02X soesOut=%u escDenied=%u sii=%u\n",
           tag, ecat_master_state_name(s.state), (int)s.probeWkc, s.configs, s.lastConfigUs,
           (int)s.slaves, s.iomapBytes, s.cycles, s.wkcOk, s.wkcMiss, (int)s.lastWkc,
           (int)s.expectedWkc, s.outValue, s.inValue, s.echoLag, slave_app_al_status(),
           slave_app_outputs_updates(), slave_app_esc()->stats.denied, slave_app_esc()->stats.siiReads);
}

static void run_ms(unsigned ms)
{
    for (unsigned t = 0; t < ms * 5U; t++)
    {
        s_now += 200U;
        slave_tick();
        ecat_master_step();
    }
}

int main(void)
{
    ecat_master_stats_t s;
    int ok;

    slave_app_init(1000U);
    ecat_master_init();

    run_ms(1000);  report("t=1s");
    run_ms(2000);  report("t=3s");
    ecat_master_stats_get(&s);
    ok = (s.state == ECAT_ST_OP) && (s.slaves == 1) && (s.lastWkc == s.expectedWkc)
         && (s.expectedWkc == 3) && (s.inValue != 0U) && (slave_app_al_status() == 0x08U);

    s_link = false; run_ms(100); report("link off");
    s_link = true;  run_ms(3000); report("link on");
    ecat_master_stats_get(&s);
    ok = ok && (s.state == ECAT_ST_OP);

    printf("RESULT: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
