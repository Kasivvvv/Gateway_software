/* Host unit test: cycle generator of configuration 4 (fixed phase, R5). */
#include <stdio.h>
#include "cycle_gen.h"

static int s_fail;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); s_fail++; } } while (0)

int main(void)
{
    cycle_gen_t cg;
    uint32_t t, ec = 0, sy = 0, lastEc = 0, lastSy = 0;
    int badPhase = 0;

    CHECK(!cycle_gen_init(&cg, 5, 10, 10));
    CHECK(!cycle_gen_init(&cg, 0, 10, 1));
    CHECK(cycle_gen_init(&cg, 5, 10, 1));       /* T_EC 1 ms, T_SYNC 2 ms, SYNC 200 us later */

    for (t = 100; t < 100100; t++)
    {
        cycle_gen_out_t o = cycle_gen_step(&cg, t);
        if (o.ecDue) { CHECK((t % 5) == 0); ec++; if (lastEc) CHECK(t - lastEc == 5); lastEc = t; }
        if (o.syncDue) { sy++; if ((t % 10) != 1) badPhase++; if (lastSy) CHECK(t - lastSy == 10); lastSy = t; }
    }
    CHECK(ec == 20000 && sy == 10000 && badPhase == 0);
    printf("100000 ticks: %u EC cycles, %u SYNC, phase errors %d\n", ec, sy, badPhase);

    /* overrun: ticks 200..212 merged into one late iteration */
    CHECK(cycle_gen_init(&cg, 5, 10, 1));
    ec = sy = 0;
    for (t = 0; t < 400; t++)
    {
        cycle_gen_out_t o;
        if ((t > 200) && (t < 213)) continue;   /* iterations lost */
        o = cycle_gen_step(&cg, t);
        if (o.ecDue) ec++;
        if (o.syncDue) { sy++; if (t != 213) CHECK((t % 10) == 1); }
    }
    printf("overrun: %u EC cycles, %u SYNC (no burst, phase kept)\n", ec, sy);
    CHECK(ec == 80 - 1 && sy == 40 - 1);   /* two cycles merged into one */

    /* S7: T_SYNC 3 ms */
    CHECK(cycle_gen_init(&cg, 5, 15, 1));
    sy = 0;
    for (t = 0; t < 1500; t++) { if (cycle_gen_step(&cg, t).syncDue) { sy++; CHECK((t % 15) == 1); } }
    CHECK(sy == 100);

    printf("RESULT: %s\n", s_fail ? "FAIL" : "PASS");
    return s_fail ? 1 : 0;
}
