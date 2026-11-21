#!/bin/sh
# Build and run the state machine / error handler unit test on the host.
set -e
R=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${OUT:-${TMPDIR:-/tmp}/state_machine_test}
${CC:-cc} ${CFLAGS:--std=gnu11 -g -O1 -Wall -Wextra} \
  -I$R/src/software_core/state_machine -I$R/src/software_core/error_handler \
  $R/src/software_core/state_machine/state_machine.c $R/src/software_core/error_handler/error_handler.c \
  $R/tests/state_machine/test_sm.c -o "$OUT"
"$OUT"
