/*
 * Host co-simulation: gateway EtherCAT master (ecat_master.c + SOEM + SOEM
 * port) <-> wire <-> soft-ESC <-> SOES test slave, in one process.
 *
 * The "wire" is synchronous: hwa_eth_tx() lets the soft-ESC process the frame
 * and queues it for hwa_eth_rx(). Simulated time advances 1 us per clock read
 * (so SOEM busy-waits progress) and 200 us per Tloop; SOES runs every 50 us.
 * The EtherCAT cycle is triggered every 2 ms like a SYNC in configuration 3.
 * Pass criteria: master reaches OP with the expected WKC (8 B out, 8 B in),
 * outputs written by the master come back as inputs (slave echo), no missed
 * cycles once in OP, recovery after a link loss.
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

static uint64_t s_outPattern;
static bool s_safe;
static bool s_noSync;
static unsigned s_echoOk, s_echoBad, s_inputs;

static void report(const char *tag)
{
    ecat_master_stats_t s;
    ecat_master_stats_get(&s);
    printf("%-10s st=%s cfg=%u sl=%d io=%u cyc=%u ok=%u miss=%u run=%u wkc=%d/%d al=0x%02X/0x%02X sync=%s tmr=%u fault=%d echoOk=%u echoBad=%u | slaveAL=0x%02X slaveOut=%016llX\n",
           tag, ecat_master_state_name(s.state), s.configs,
           (int)s.slaves, s.iomapBytes, s.cycles, s.wkcOk, s.wkcMiss, s.missRun, (int)s.lastWkc,
           (int)s.expectedWkc, s.alStatus, s.alTarget, s.syncLost ? "LOST" : "ok", s.timerCycles, ecat_master_pd_fault(), s_echoOk, s_echoBad,
           slave_app_al_status(), (unsigned long long)slave_app_outputs());
}

/* Each 2 ms: new outputs + trigger (as core does on SYNC). The slave updates
 * its inputs (= outputs) after a frame has passed, so the inputs returned by
 * cycle n echo the outputs of cycle n-1: one cycle of echo delay. */
static void run_ms(unsigned ms)
{
    static uint64_t lastSent;
    static uint64_t prevSent;
    for (unsigned t = 0; t < ms * 5U; t++)
    {
        uint8_t in[8];
        uint32_t len;

        s_now += 200U;
        slave_tick();
        if ((t % 10U) == 0U)
        {
            uint8_t out[8];
            s_outPattern += 0x0101010101010101ULL;
            for (int i = 0; i < 8; i++) { out[i] = (uint8_t)(s_outPattern >> (8 * i)); }
            ecat_master_outputs_write(out, 8);
            if (ecat_master_operational()) { prevSent = lastSent; lastSent = s_outPattern; }
            if (s_safe) { ecat_master_outputs_safe(); prevSent = lastSent; lastSent = 0; }
            if (!s_noSync) { ecat_master_trigger(); }
        }
        ecat_master_step();
        if (ecat_master_inputs_take(in, 8, &len))
        {
            uint64_t v = 0;
            for (int i = 0; i < 8; i++) { v |= (uint64_t)in[i] << (8 * i); }
            s_inputs++;
            if ((len == 8) && (v == prevSent)) { s_echoOk++; } else { s_echoBad++; }
        }
    }
}

int main(void)
{
    ecat_master_stats_t s;
    int ok;

    slave_app_init(1000U);
    ecat_master_init(ECAT_CYCLE_EXTERNAL);
    run_ms(20);
    ecat_master_stats_get(&s);
    ok = (s.state == ECAT_ST_WAIT_LINK) && (s.configs == 0U);   /* no config without permission */
    ecat_master_set_config_enabled(true);

    run_ms(1000);  report("t=1s");
    run_ms(2000);  report("t=3s");
    ecat_master_set_config_enabled(false);
    ecat_master_stats_get(&s);
    ok = ok && (s.state == ECAT_ST_OP) && (s.slaves == 1) && (s.lastWkc == s.expectedWkc)
         && (s.expectedWkc == 3) && (s.iomapBytes == 16U) && (s.wkcMiss == 0U)
         && (s_echoOk > 1000U) && (s_echoBad <= 2U) && (slave_app_al_status() == 0x08U);

    /* safe reaction: zero outputs, slaves to SAFE-OP: no WKC fault reported */
    s_safe = true; ecat_master_request_al_state(ECAT_AL_SAFE_OP); run_ms(200); report("safe-op");
    ecat_master_stats_get(&s);
    ok = ok && (slave_app_al_status() == 0x04U) && ((s.alStatus & 0x0FU) == 0x04U)
         && (slave_app_outputs() == 0U) && !ecat_master_pd_fault() && (s.lastWkc < s.expectedWkc)
         && (s.targetWkc == 1) && (s.missRun == 0U);
    s_safe = false; ecat_master_request_al_state(ECAT_AL_OP); run_ms(200); report("back to op");
    ecat_master_stats_get(&s);
    ok = ok && (slave_app_al_status() == 0x08U) && (s.lastWkc == s.expectedWkc) && (s.missRun == 0U)
         && !ecat_master_pd_fault();

    /* SYNC loss: own timer keeps the slave in OP, SYNC return resumes it */
    {
        unsigned tmr0;
        ecat_master_stats_get(&s); tmr0 = s.timerCycles;
        s_noSync = true; run_ms(500); report("no sync");
        ecat_master_stats_get(&s);
        ok = ok && s.syncLost && (s.timerCycles > tmr0 + 400U) && (slave_app_al_status() == 0x08U)
             && (s.missRun == 0U) && !ecat_master_pd_fault();
        tmr0 = s.timerCycles;
        s_noSync = false; run_ms(200); report("sync back");
        ecat_master_stats_get(&s);
        ok = ok && !s.syncLost && (s.timerCycles == tmr0) && (s.lastWkc == s.expectedWkc);
    }

    /* link loss: configuration kept, cycles resume without re-configuration */
    s_link = false; run_ms(100); report("link off");
    s_link = true;  run_ms(3000); report("link on");
    ecat_master_stats_get(&s);
    ok = ok && (s.state == ECAT_ST_OP) && (s.configs == 1U) && (s.lastWkc == s.expectedWkc)
         && !ecat_master_pd_fault();

    printf("RESULT: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
