#include "gw_config.h"

#include <stddef.h>

#define GW_NODES_MAX (7U)

static gw_config_t s_cfg;

const gw_config_t *gw_config_get(void)
{
    return &s_cfg;
}

bool gw_config_init(uint32_t tloopUs)
{
    s_cfg.roleConfig = (uint8_t)GW_ROLE_CONFIG;
    s_cfg.tecUs = 1000U;          /* Table 8.6: T_EC = 1 ms */
    s_cfg.tsyncUs = 2000U;        /* set S2 (3000 for S7) */
    s_cfg.syncPhaseUs = 200U;     /* SYNC one Tloop after the EtherCAT frame */
    s_cfg.nodeCount = 2U;         /* set S2: nodes 1 and 2 */
    s_cfg.strategy = CONV_STRATEGY_A;

    if ((s_cfg.roleConfig != 3U) && (s_cfg.roleConfig != 4U))
    {
        return false;
    }
    if ((tloopUs == 0U) || ((s_cfg.tecUs % tloopUs) != 0U)
            || ((s_cfg.tsyncUs % tloopUs) != 0U) || ((s_cfg.syncPhaseUs % tloopUs) != 0U)
            || (s_cfg.syncPhaseUs >= s_cfg.tsyncUs) || (s_cfg.tecUs == 0U)
            || (s_cfg.tsyncUs == 0U))
    {
        return false;
    }
    if ((s_cfg.nodeCount == 0U) || (s_cfg.nodeCount > GW_NODES_MAX))
    {
        return false;
    }
    return true;
}
