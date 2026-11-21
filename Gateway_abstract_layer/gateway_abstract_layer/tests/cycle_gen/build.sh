#!/bin/sh
# Host build and run of the cycle generator test.
set -e
D=$(cd "$(dirname "$0")" && pwd)
R="$D/../.."
OUT="${TMPDIR:-/tmp}/gw_test_cycle_gen"
cc -std=c11 -Wall -Wextra -Werror -I"$R/src/software_core/core" \
   "$D/test_cycle_gen.c" "$R/src/software_core/core/cycle_gen.c" -o "$OUT"
"$OUT"
