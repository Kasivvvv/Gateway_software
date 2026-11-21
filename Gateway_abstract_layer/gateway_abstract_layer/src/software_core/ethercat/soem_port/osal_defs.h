/*
 * SOEM OSAL definitions for the bare-metal gateway (SAM V71, superloop).
 * Replaces SOEM osal/<os>/osal_defs.h; included by third_party/SOEM/osal/osal.h.
 */
#ifndef _osal_defs_
#define _osal_defs_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

/* No console output from inside SOEM (it would block or flood the UART). */
#define EC_PRINT(...) do {} while (0)

#ifndef PACKED
#define PACKED_BEGIN
#define PACKED  __attribute__((__packed__))
#define PACKED_END
#endif

#ifdef __cplusplus
}
#endif

#endif /* _osal_defs_ */
