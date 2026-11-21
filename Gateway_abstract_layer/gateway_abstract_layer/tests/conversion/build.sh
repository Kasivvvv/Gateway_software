#!/bin/sh
# Build and run the conversion unit test on the host.
set -e
R=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${OUT:-${TMPDIR:-/tmp}/conversion_test}
${CC:-cc} ${CFLAGS:--std=gnu11 -g -O1 -Wall -Wextra} -I$R/tests/conversion -I$R/src/software_core/conversion \
  $R/src/software_core/conversion/conversion.c $R/tests/conversion/test_conversion.c -lm -o "$OUT"
"$OUT"
