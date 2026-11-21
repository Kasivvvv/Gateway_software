/*
 * SOEM OS/hardware abstraction for the bare-metal gateway (Cortex-M7, little
 * endian). Network byte order is big endian.
 */
#include <string.h>

#include "oshw.h"

static ec_adaptert s_adapter;

uint16 oshw_htons(uint16 host)
{
    return (uint16)__builtin_bswap16(host);
}

uint16 oshw_ntohs(uint16 network)
{
    return (uint16)__builtin_bswap16(network);
}

ec_adaptert *oshw_find_adapters(void)
{
    (void)memset(&s_adapter, 0, sizeof(s_adapter));
    (void)strncpy(s_adapter.name, OSHW_IFNAME, EC_MAXLEN_ADAPTERNAME - 1);
    (void)strncpy(s_adapter.desc, "SAM V71 GMAC + KSZ8061 (RMII)",
            EC_MAXLEN_ADAPTERNAME - 1);
    s_adapter.next = NULL;
    return &s_adapter;
}

void oshw_free_adapters(ec_adaptert *adapter)
{
    (void)adapter; /* static storage */
}
