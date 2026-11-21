/*
 * SOEM OS/hardware abstraction for the bare-metal gateway.
 */
#ifndef _oshw_
#define _oshw_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatmain.h"

/** Name of the only EtherCAT interface (on-board GMAC + KSZ8061). */
#define OSHW_IFNAME "gmac0"

uint16 oshw_htons(uint16 hostshort);
uint16 oshw_ntohs(uint16 networkshort);
ec_adaptert *oshw_find_adapters(void);
void oshw_free_adapters(ec_adaptert *adapter);

#ifdef __cplusplus
}
#endif

#endif /* _oshw_ */
