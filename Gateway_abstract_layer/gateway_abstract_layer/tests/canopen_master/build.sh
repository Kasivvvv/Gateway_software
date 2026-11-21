#!/bin/sh
# Host build of canopen.c (master role) with the lely-core sources and the
# config.h of the firmware; CAN driver and time are stubbed by the test.
set -e
D=$(cd "$(dirname "$0")" && pwd)
R="$D/../.."
L="$R/third_party/lely-core"
OUT="${TMPDIR:-/tmp}/gw_test_canopen_master"
SRC="$L/lib/compat/string.c $L/lib/compat/strings.c"
for f in bits cmp diag dllist endian error membuf memory mempool pheap rbtree time ustring; do SRC="$SRC $L/lib/util/$f.c"; done
for f in buf msg net; do SRC="$SRC $L/lib/can/$f.c"; done
for f in crc dev nmt nmt_hb nmt_srv obj pdo rpdo sdo ssdo sync tpdo type val; do SRC="$SRC $L/lib/co/$f.c"; done
cc -std=gnu11 -w -DHAVE_CONFIG_H=1 -DLELY_NO_MALLOC=1 -DLELY_HAVE_UCHAR_H=0 -DLELY_HAVE_SYS_TYPES_H=1 \
   -I"$R/src/software_core/canopen/config" -I"$L/include" -I"$L/lib/co" -I"$L/lib/can" -I"$L/lib/util" -I"$L/lib/compat" \
   -I"$R/src/software_core/canopen" -I"$R/src/software_core/interface" -I"$R/src/abstract_layer/interface" \
   "$D/test_canopen_master.c" "$R/src/software_core/canopen/canopen.c" \
   "$R/src/software_core/canopen/od/gateway_od.c" "$R/src/software_core/canopen/od/gateway_master_od.c" \
   $SRC -o "$OUT"
"$OUT"
