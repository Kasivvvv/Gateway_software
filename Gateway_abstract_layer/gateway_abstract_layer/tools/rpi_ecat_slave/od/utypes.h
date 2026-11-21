#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"

/* Object dictionary storage of the gateway test slave. */
typedef struct
{
   /* Inputs (TxPDO 0x1A00 -> 0x6000:01..02), slave -> master */
   struct
   {
      uint64_t Data;
      uint64_t Data2;
   } Inputs;

   /* Outputs (RxPDO 0x1600 -> 0x7000:01..02), master -> slave */
   struct
   {
      uint64_t Data;
      uint64_t Data2;
   } Outputs;
} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
