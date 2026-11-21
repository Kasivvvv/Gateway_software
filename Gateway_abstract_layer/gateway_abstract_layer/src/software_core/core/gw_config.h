#ifndef GW_CONFIG_H
#define GW_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#include "conversion.h"

/*
 * Configuration of the gateway (role configuration of thesis Table 5.1 and
 * its parameters). Until the Configuration manager (stage 9) it is fixed at
 * compile time: select the role configuration with GW_ROLE_CONFIG.
 *
 *   3  EtherCAT master + CANopen node 0x10, EtherCAT cycle started by the
 *      SYNC of the external CANopen master (set S1).
 *   4  EtherCAT master + CANopen master 0x7F, both cycles defined by the
 *      gateway timer with a fixed phase (R5), nodes 1..nodeCount (set S2).
 */
#ifndef GW_ROLE_CONFIG
#define GW_ROLE_CONFIG 4
#endif

typedef struct
{
    uint8_t  roleConfig;        /* 3 or 4 (Table 5.1) */
    /* Cycles defined by the gateway (configuration 4), multiples of Tloop */
    uint32_t tecUs;             /* EtherCAT cycle T_EC (1 ms) */
    uint32_t tsyncUs;           /* SYNC period T_SYNC (2 ms S2, 3 ms S7) */
    uint32_t syncPhaseUs;       /* SYNC after the start of an EtherCAT cycle */
    /* CANopen master (configuration 4) */
    uint8_t  nodeCount;         /* nodes 1..nodeCount (S2: 2, S7: 7) */
    /* Conversion */
    conv_strategy_t strategy;
} gw_config_t;

/** Configuration in use (valid after gw_config_init()). */
const gw_config_t *gw_config_get(void);

/** Load the configuration; false if it is inconsistent (self-test, T3). */
bool gw_config_init(uint32_t tloopUs);

static inline bool gw_config_can_master(void)
{
    return gw_config_get()->roleConfig == 4U;
}

#endif /* GW_CONFIG_H */
