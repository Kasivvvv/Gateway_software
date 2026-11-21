/* Host unit test: supervisory state machine (T1..T12) + error handler. */
#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "error_handler.h"

static int s_fail;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); s_fail++; } } while (0)

static sm_inputs_t si;     /* status from the stacks, persistent */
static eh_inputs_t ei;     /* error indications, persistent */
static uint64_t s_now;
static sm_output_t so;
static eh_output_t eo;

/* One superloop iteration as the Software Core does it. */
static uint8_t step(void)
{
    s_now += 200;
    eh_step(sm_state(), &ei, s_now, &eo);
    si.errorClass = eo.cls;
    si.errorReset = eo.errorReset;
    sm_step(&si, &so);
    ei.canRxOverflow = false;   /* events last one iteration */
    ei.convOverflow = false;
    return so.transition;
}

static void boot_to_working(void)
{
    memset(&si, 0, sizeof si); memset(&ei, 0, sizeof ei);
    si.lowLevelInitDone = true;
    CHECK(step() == 1 && so.state == GW_ST_STARTUP);
    si.selfTestDone = true; si.selfTestOk = true;
    CHECK(step() == 2 && so.state == GW_ST_CONFIGURATION);
    CHECK(step() == 0);                              /* stacks not ready yet */
    si.stacksReady = true;
    CHECK(step() == 4 && so.state == GW_ST_WORKING && so.convert);
}

int main(void)
{
    sm_init(); eh_init();
    CHECK(sm_state() == GW_ST_INIT);
    boot_to_working();

    /* T7 safe request (e.g. link loss), T8 needs cause removed AND resume */
    si.safeCause = true;
    CHECK(step() == 7 && so.state == GW_ST_SAFE && so.commStop && !so.convert);
    si.resume = true; CHECK(step() == 0);            /* cause still present */
    si.safeCause = false; si.resume = false; CHECK(step() == 0); /* no resume */
    si.resume = true; CHECK(step() == 8 && so.state == GW_ST_WORKING && so.convert);
    si.resume = false;

    /* T6 bus-off -> REINIT -> T10 (CAN only) -> Startup -> T2 -> Configuration -> T4 */
    ei.canBusOff = true;
    CHECK(step() == 6 && so.state == GW_ST_ERROR && so.commStop && eo.code == GW_ERR_CAN_BUSOFF);
    CHECK(eo.cls == GW_ERR_CLASS_REINIT);
    CHECK(step() == 10 && so.state == GW_ST_STARTUP && so.commReset && eo.reinitCan && !eo.reinitEc);
    ei.canBusOff = false;                            /* recovered by the re-init */
    CHECK(step() == 2);
    CHECK(step() == 4 && eo.code == GW_ERR_NONE);

    /* error has priority over a simultaneous safe request */
    ei.canHbLost = true; si.safeCause = true;
    CHECK(step() == 6 && eo.code == GW_ERR_CAN_HB_TIMEOUT && eo.cls == GW_ERR_CLASS_RECOVERABLE);
    si.safeCause = false;
    CHECK(step() == 0 && so.state == GW_ST_ERROR);   /* heartbeat still missing */
    ei.canHbLost = false;
    CHECK(step() == 0 && so.state == GW_ST_ERROR);   /* node still Pre-operational */
    ei.canOperational = true;                        /* NMT start from the master */
    CHECK(step() == 9 && so.state == GW_ST_WORKING); /* T9: no re-initialisation */
    CHECK(step() == 0 && eo.code == GW_ERR_NONE);

    /* conversion overflow: T6 then T9 at once */
    ei.convOverflow = true;
    CHECK(step() == 6 && eo.code == GW_ERR_CONV_OVERFLOW);
    CHECK(step() == 9);

    /* T5 + escalation: EtherCAT init keeps failing -> T10 x3, then T11 */
    CHECK(step() == 0);                              /* Working timer starts */
    s_now += EH_STABLE_US; CHECK(step() == 0);       /* stable Working: count reset */
    CHECK(eo.reinitCount == 0);
    ei.ecWkcFault = true;
    CHECK(step() == 6 && eo.code == GW_ERR_EC_WKC && eo.reinitCount == 1);
    ei.ecWkcFault = false;
    CHECK(step() == 10 && eo.reinitEc && !eo.reinitCan);
    si.stacksReady = false; si.stacksFailed = true; ei.ecInitFailed = true; /* both from the core */
    {
        int t10 = 0, t11 = 0, guard = 0;
        while (!t11 && guard++ < 100)
        {
            uint8_t t = step();
            if (t == 10) t10++;
            if (t == 11) { t11++; CHECK(so.hwReset && so.state == GW_ST_INIT); }
        }
        printf("EC init failing: %d x T10 before T11 (escalation limit %u)\n", t10, EH_MAX_REINIT);
        CHECK(t11 == 1 && t10 == (int)EH_MAX_REINIT);  /* WKC fault before: not counted */
    }

    /* network faults never escalate: persistent bus-off / flapping slave */
    sm_init(); eh_init(); boot_to_working();
    {
        int t11 = 0;
        for (int i = 0; i < 10; i++) {
            ei.canBusOff = true; CHECK(step() == 6);
            uint8_t t = step(); if (t == 11) t11++;
            CHECK(t == 10 && eo.reinitCan && eo.reinitCount == (uint32_t)(i + 1));
            ei.canBusOff = false; CHECK(step() == 2); CHECK(step() == 4);
        }
        for (int i = 0; i < 10; i++) {
            ei.ecWkcFault = true; CHECK(step() == 6);
            uint8_t t = step(); if (t == 11) t11++;
            CHECK(t == 10 && eo.reinitEc);
            ei.ecWkcFault = false; CHECK(step() == 2); CHECK(step() == 4);
        }
        printf("20 network faults in a row: %d x T11\n", t11);
        CHECK(t11 == 0 && eo.reinitCount == 20);
    }

    /* T5 with EC_WKC: configured EtherCAT network lost while waiting in
     * Configuration (e.g. for the CAN bus-off recovery) -> re-configure EC,
     * never T11 */
    sm_init(); eh_init(); boot_to_working();
    ei.canBusOff = true; CHECK(step() == 6); CHECK(step() == 10 && eo.reinitCan);
    ei.canBusOff = false; CHECK(step() == 2);
    si.stacksReady = false;                           /* CAN recovery pending */
    CHECK(step() == 0 && so.state == GW_ST_CONFIGURATION);
    {
        int t11 = 0;
        for (int i = 0; i < 6; i++) {
            ei.ecWkcFault = true; si.stacksFailed = true; /* slave restarted */
            CHECK(step() == 5 && eo.code == GW_ERR_EC_WKC && eo.cls == GW_ERR_CLASS_REINIT);
            uint8_t t = step(); if (t == 11) t11++;
            CHECK(t == 10 && eo.reinitEc && !eo.reinitCan);
            ei.ecWkcFault = false; si.stacksFailed = false; /* reset by T10 */
            CHECK(step() == 2); CHECK(step() == 0);
        }
        CHECK(t11 == 0);
    }
    /* bus-off while waiting in Configuration (EC slave still missing) */
    ei.canBusOff = true; si.stacksFailed = true;
    CHECK(step() == 5 && eo.code == GW_ERR_CAN_BUSOFF);
    CHECK(step() == 10 && eo.reinitCan && !eo.reinitEc);
    /* recovery scheduled: the core reports no bus-off while it is pending,
     * so an EC fault in this window re-initialises EtherCAT only */
    ei.canBusOff = false; si.stacksFailed = false;
    CHECK(step() == 2);
    ei.ecWkcFault = true; si.stacksFailed = true;
    CHECK(step() == 5 && eo.code == GW_ERR_EC_WKC);
    CHECK(step() == 10 && eo.reinitEc && !eo.reinitCan);
    ei.ecWkcFault = false; si.stacksFailed = false;
    CHECK(step() == 2);
    si.stacksReady = true; CHECK(step() == 4 && eo.code == GW_ERR_NONE);

    /* escalation in the Error State: heartbeat timeout, then bus-off -> T10 */
    sm_init(); eh_init(); boot_to_working(); ei.canOperational = true;
    ei.canHbLost = true;
    CHECK(step() == 6 && eo.cls == GW_ERR_CLASS_RECOVERABLE);
    CHECK(step() == 0 && so.state == GW_ST_ERROR);
    ei.canBusOff = true;                              /* bus-off while in Error */
    CHECK(step() == 10 && eo.code == GW_ERR_CAN_BUSOFF && eo.reinitCan && !eo.reinitEc
          && eo.reinitCount == 1);
    ei.canBusOff = false; ei.canHbLost = false;
    CHECK(step() == 2); CHECK(step() == 4 && eo.code == GW_ERR_NONE && !eo.reinitCan);

    /* T12 Safe -> Error */
    sm_init(); eh_init(); boot_to_working(); ei.canOperational = true;
    si.safeCause = true;                              /* e.g. NMT stop */
    CHECK(step() == 7 && so.state == GW_ST_SAFE);
    ei.canRxOverflow = true;                          /* not critical: no T12 */
    CHECK(step() == 0 && so.state == GW_ST_SAFE);
    ei.canBusOff = true; si.resume = true;            /* error beats resume */
    CHECK(step() == 12 && so.state == GW_ST_ERROR && so.commStop && eo.code == GW_ERR_CAN_BUSOFF);
    CHECK(step() == 10 && eo.reinitCan && !eo.reinitEc);
    ei.canBusOff = false; si.safeCause = false; si.resume = false;
    CHECK(step() == 2); CHECK(step() == 4);

    si.safeCause = true;
    CHECK(step() == 7);
    ei.ecWkcFault = true;                             /* slave lost while Safe */
    CHECK(step() == 12 && eo.code == GW_ERR_EC_WKC);
    CHECK(step() == 10 && eo.reinitEc && !eo.reinitCan);
    ei.ecWkcFault = false;
    CHECK(step() == 2); CHECK(step() == 4); CHECK(step() == 7);  /* cause still present */

    ei.canHbLost = true; ei.canOperational = false;   /* master silent while Safe */
    CHECK(step() == 12 && eo.code == GW_ERR_CAN_HB_TIMEOUT && eo.cls == GW_ERR_CLASS_RECOVERABLE);
    ei.canHbLost = false;
    CHECK(step() == 0 && so.state == GW_ST_ERROR);   /* waits for NMT start */
    si.safeCause = false; ei.canOperational = true;
    CHECK(step() == 9 && so.state == GW_ST_WORKING);

    /* T3: self-test failure -> FATAL -> T11 */
    sm_init(); eh_init(); memset(&si, 0, sizeof si); memset(&ei, 0, sizeof ei);
    si.lowLevelInitDone = true; CHECK(step() == 1);
    si.selfTestDone = true; si.selfTestOk = false; ei.selfTestFailed = true;
    CHECK(step() == 3 && so.state == GW_ST_ERROR && eo.cls == GW_ERR_CLASS_FATAL);
    CHECK(step() == 11 && so.hwReset);

    /* stable Working resets the escalation counter */
    sm_init(); eh_init(); boot_to_working();
    for (int i = 0; i < 2; i++) {
        ei.canBusOff = true; CHECK(step() == 6); CHECK(step() == 10);
        ei.canBusOff = false; CHECK(step() == 2); CHECK(step() == 4);
    }
    CHECK(eo.reinitCount == 2);
    step(); s_now += EH_STABLE_US; step();
    CHECK(eo.reinitCount == 0);

    printf("RESULT: %s\n", s_fail ? "FAIL" : "PASS");
    return s_fail ? 1 : 0;
}
