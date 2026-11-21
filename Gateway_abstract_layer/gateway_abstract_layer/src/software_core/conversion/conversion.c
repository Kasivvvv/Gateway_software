#include "conversion.h"

#include <math.h>
#include <string.h>

#include "hwa_interface.h"

typedef struct
{
    conv_dir_config_t cfg;
    conv_msg_t queue[CONV_K_MAX];
    uint32_t   head;          /* index of the oldest queued message */
    uint32_t   count;         /* queued, not in service */
    bool       busy;          /* one message in service */
    conv_msg_t inService;
    uint64_t   serviceEndUs;
    uint32_t   nextSeq;
    conv_stats_t stats;
} conv_buffer_t;

static conv_buffer_t s_buf[CONV_DIR_COUNT];
static conv_strategy_t s_strategy;
static bool s_fault;
static uint32_t s_rng = 0x12345678U;

/* xorshift32: reproducible pseudo-random numbers for the HIL-03 test mode. */
static uint32_t conv_rand(void)
{
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 17;
    s_rng ^= s_rng << 5;
    return s_rng;
}

static uint64_t conv_service_time(const conv_dir_config_t *cfg)
{
    switch (cfg->service)
    {
    case CONV_SERVICE_DETERMINISTIC:
        return cfg->serviceMeanUs;

    case CONV_SERVICE_EXPONENTIAL:
    {
        /* inverse transform, u in (0, 1] */
        float u = ((float)(conv_rand() >> 8) + 1.0F) / 16777216.0F;
        return (uint64_t)(-(float)cfg->serviceMeanUs * logf(u));
    }

    case CONV_SERVICE_IMMEDIATE:
    default:
        return 0U;
    }
}

static void conv_update_fill(conv_buffer_t *b)
{
    b->stats.fill = b->count + (b->busy ? 1U : 0U);
    if (b->stats.fill > b->stats.highWater)
    {
        b->stats.highWater = b->stats.fill;
    }
}

static bool conv_dir_valid(conv_dir_t dir)
{
    return (uint32_t)dir < (uint32_t)CONV_DIR_COUNT;
}

bool conversion_init(const conv_config_t *cfg)
{
    uint32_t d;

    if (cfg == NULL)
    {
        return false;
    }
    for (d = 0U; d < (uint32_t)CONV_DIR_COUNT; d++)
    {
        if ((cfg->dir[d].capacity == 0U) || (cfg->dir[d].capacity > CONV_K_MAX))
        {
            return false;
        }
    }

    (void)memset(s_buf, 0, sizeof(s_buf));
    for (d = 0U; d < (uint32_t)CONV_DIR_COUNT; d++)
    {
        s_buf[d].cfg = cfg->dir[d];
    }
    s_strategy = cfg->strategy;
    s_fault = false;
    return true;
}

void conversion_flush(void)
{
    uint32_t d;

    for (d = 0U; d < (uint32_t)CONV_DIR_COUNT; d++)
    {
        s_buf[d].head = 0U;
        s_buf[d].count = 0U;
        s_buf[d].busy = false;
        conv_update_fill(&s_buf[d]);
    }
}

bool conversion_push(conv_dir_t dir, const conv_msg_t *msg, uint64_t nowUs)
{
    conv_buffer_t *b;
    conv_msg_t *slot;

    if (!conv_dir_valid(dir) || (msg == NULL))
    {
        return false;
    }
    b = &s_buf[dir];

    if ((b->count + (b->busy ? 1U : 0U)) >= b->cfg.capacity)
    {
        b->stats.overflows++;
        s_fault = true;
        return false;
    }

    slot = &b->queue[(b->head + b->count) % CONV_K_MAX];
    *slot = *msg;
    if (slot->len > CONV_DATA_MAX)
    {
        slot->len = CONV_DATA_MAX;
    }
    slot->seq = b->nextSeq++;
    slot->t2Us = nowUs;
    b->count++;
    b->stats.inserted++;
    conv_update_fill(b);

    hwa_marker_pulse(HAL_MARKER_T2); /* t2: inserted into the buffer */
    return true;
}

bool conversion_pop(conv_dir_t dir, conv_msg_t *msg, uint64_t nowUs)
{
    conv_buffer_t *b;

    if (!conv_dir_valid(dir) || (msg == NULL))
    {
        return false;
    }
    b = &s_buf[dir];
    (void)s_strategy; /* strategy A: no holding time, Ls = 0 */

    if (!b->busy && (b->count > 0U))
    {
        b->inService = b->queue[b->head];
        b->head = (b->head + 1U) % CONV_K_MAX;
        b->count--;
        b->busy = true;
        b->serviceEndUs = nowUs + conv_service_time(&b->cfg);
        hwa_marker_pulse(HAL_MARKER_T3); /* t3: removed from the buffer */
    }

    if (b->busy && (nowUs >= b->serviceEndUs))
    {
        *msg = b->inService;
        b->busy = false;
        b->stats.removed++;
        conv_update_fill(b);
        return true;
    }
    return false;
}

void conversion_stats_get(conv_dir_t dir, conv_stats_t *stats)
{
    if (conv_dir_valid(dir) && (stats != NULL))
    {
        *stats = s_buf[dir].stats;
    }
}

bool conversion_fault(void)
{
    return s_fault;
}

void conversion_fault_clear(void)
{
    s_fault = false;
}
