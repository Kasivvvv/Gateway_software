#!/bin/sh
# Build and run the host co-simulation (macOS clang or Linux gcc).
set -e
R=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${OUT:-${TMPDIR:-/tmp}/soft_esc_loop_test}
SOEM=$R/third_party/SOEM
SOES=$R/third_party/SOES
DEMO=$SOES/applications/linux_lan9252demo
SLV=$R/tools/rpi_ecat_slave
CC=${CC:-cc}
# macOS ld (chained fixups) rejects pointers in SOES packed OD structs
LDEXTRA=""
[ "$(uname)" = "Darwin" ] && LDEXTRA="-Wl,-no_fixup_chains"
$CC ${CFLAGS:--std=gnu11 -g -O1 -Wall} -DEC_VER2 \
  -I$R/tests/soft_esc_loop \
  -I$SOEM/soem -I$SOEM/osal -I$R/src/software_core/ethercat/soem_port -I$R/src/software_core/ethercat \
  -I$SOES/soes -I$SOES/soes/include/sys/gcc -I$DEMO -I$SLV -I$R/src/abstract_layer/modules/esc \
  $SOEM/soem/ethercatbase.c $SOEM/soem/ethercatcoe.c $SOEM/soem/ethercatconfig.c $SOEM/soem/ethercatdc.c \
  $SOEM/soem/ethercateoe.c $SOEM/soem/ethercatfoe.c $SOEM/soem/ethercatmain.c $SOEM/soem/ethercatsoe.c \
  $R/src/software_core/ethercat/ecat_master.c $R/src/software_core/ethercat/soem_port/nicdrv.c \
  $R/src/software_core/ethercat/soem_port/osal.c $R/src/software_core/ethercat/soem_port/oshw.c \
  $SOES/soes/esc.c $SOES/soes/esc_coe.c $SOES/soes/esc_foe.c $SOES/soes/esc_eoe.c \
  $SOES/soes/ecat_slv.c $DEMO/slave_objectlist.c \
  $SLV/esc_hw_soft.c $SLV/slave_app.c $SLV/slave_sii.c \
  $R/src/abstract_layer/modules/esc/soft_esc.c \
  $R/tests/soft_esc_loop/loop_test.c $LDEXTRA -o "$OUT"
"$OUT"
