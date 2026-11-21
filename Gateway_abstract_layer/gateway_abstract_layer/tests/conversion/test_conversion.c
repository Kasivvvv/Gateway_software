/* Host unit test of the conversion module (pure logic). */
#include <stdio.h>
#include <string.h>
#include "conversion.h"
#include "hwa_interface.h"

static int s_fail;
static unsigned s_t2, s_t3;
void hwa_marker_pulse(hwa_marker_id_t id) { if (id == HAL_MARKER_T2) s_t2++; if (id == HAL_MARKER_T3) s_t3++; }

#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); s_fail++; } } while (0)

static conv_msg_t msg(uint8_t v) { conv_msg_t m; memset(&m, 0, sizeof m); m.data[0] = v; m.len = 8; return m; }

static void init(uint32_t k, conv_service_t svc, uint32_t meanUs)
{
    conv_config_t c; memset(&c, 0, sizeof c);
    c.strategy = CONV_STRATEGY_A;
    c.dir[0].capacity = k; c.dir[0].service = svc; c.dir[0].serviceMeanUs = meanUs;
    c.dir[1].capacity = k;
    CHECK(conversion_init(&c));
}

int main(void)
{
    conv_msg_t m, out; conv_stats_t st; conv_config_t bad;
    uint64_t now = 1000;

    memset(&bad, 0, sizeof bad);
    CHECK(!conversion_init(&bad));                          /* K = 0 rejected */

    /* immediate service: FIFO order, t2/t3 markers, sequence numbers */
    init(4, CONV_SERVICE_IMMEDIATE, 0);
    m = msg(1); CHECK(conversion_push(CONV_DIR_CAN_TO_EC, &m, now));
    m = msg(2); CHECK(conversion_push(CONV_DIR_CAN_TO_EC, &m, now));
    CHECK(conversion_pop(CONV_DIR_CAN_TO_EC, &out, now) && out.data[0] == 1 && out.seq == 0);
    CHECK(conversion_pop(CONV_DIR_CAN_TO_EC, &out, now) && out.data[0] == 2 && out.seq == 1);
    CHECK(!conversion_pop(CONV_DIR_CAN_TO_EC, &out, now));
    CHECK(s_t2 == 2 && s_t3 == 2);

    /* capacity K = 4: fifth push overflows, fault raised, high-water = 4 */
    for (uint8_t i = 0; i < 4; i++) { m = msg(i); CHECK(conversion_push(CONV_DIR_CAN_TO_EC, &m, now)); }
    m = msg(9); CHECK(!conversion_push(CONV_DIR_CAN_TO_EC, &m, now));
    conversion_stats_get(CONV_DIR_CAN_TO_EC, &st);
    CHECK(st.overflows == 1 && st.highWater == 4 && st.fill == 4 && conversion_fault());
    conversion_fault_clear(); CHECK(!conversion_fault());
    conversion_flush(); conversion_stats_get(CONV_DIR_CAN_TO_EC, &st); CHECK(st.fill == 0);

    /* directions are independent */
    m = msg(7); CHECK(conversion_push(CONV_DIR_EC_TO_CAN, &m, now));
    CHECK(!conversion_pop(CONV_DIR_CAN_TO_EC, &out, now));
    CHECK(conversion_pop(CONV_DIR_EC_TO_CAN, &out, now) && out.data[0] == 7);

    /* HIL-03 deterministic service 500 us: removed at t3, returned at t3 + 500 us,
     * the message in service counts towards K */
    init(2, CONV_SERVICE_DETERMINISTIC, 500);
    m = msg(1); CHECK(conversion_push(CONV_DIR_CAN_TO_EC, &m, now));
    CHECK(!conversion_pop(CONV_DIR_CAN_TO_EC, &out, now));          /* service starts */
    m = msg(2); CHECK(conversion_push(CONV_DIR_CAN_TO_EC, &m, now));
    m = msg(3); CHECK(!conversion_push(CONV_DIR_CAN_TO_EC, &m, now)); /* 1 queued + 1 in service = K */
    CHECK(!conversion_pop(CONV_DIR_CAN_TO_EC, &out, now + 499));
    CHECK(conversion_pop(CONV_DIR_CAN_TO_EC, &out, now + 500) && out.data[0] == 1);
    CHECK(!conversion_pop(CONV_DIR_CAN_TO_EC, &out, now + 600));    /* next service starts */
    CHECK(conversion_pop(CONV_DIR_CAN_TO_EC, &out, now + 1100) && out.data[0] == 2);

    /* HIL-03 exponential service: mean of 20000 draws within 3 % of 400 us */
    init(1, CONV_SERVICE_EXPONENTIAL, 400);
    {
        double sum = 0; uint64_t t = 0; int n = 20000;
        for (int i = 0; i < n; i++) {
            uint64_t t3;
            m = msg(0); CHECK(conversion_push(CONV_DIR_CAN_TO_EC, &m, t));
            bool done = conversion_pop(CONV_DIR_CAN_TO_EC, &out, t); /* service starts */
            t3 = t;
            while (!done) { t++; done = conversion_pop(CONV_DIR_CAN_TO_EC, &out, t); }
            sum += (double)(t - t3);
        }
        printf("exponential mean service = %.1f us (expected 400)\n", sum / n);
        CHECK(sum / n > 388 && sum / n < 412);
    }

    printf("RESULT: %s\n", s_fail ? "FAIL" : "PASS");
    return s_fail ? 1 : 0;
}
