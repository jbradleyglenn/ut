#!/bin/sh
# Minimal shell-based test harness.  Each tests/<name>.sh is a shell script run
# with $XCU_BIN set to the build dir; a non-zero exit is a failure.  No framework, no deps.
set -u

root=$(cd "$(dirname "$0")/.." && pwd)
XCU_BIN="$root/build"
if [ ! -d "$XCU_BIN" ]; then
    echo "$XCU_BIN does not exist; run 'make' first (or just 'make t')" >&2
    exit 1
fi
export XCU_BIN

pass=0 fail=0
for t in "$root"/tests/*.sh; do
    [ -f "$t" ] || continue
    name=$(basename "$t" .sh)
    [ "$name" = a ] && continue  # this script
    if out=$(sh "$t" 2>&1); then
        pass=$((pass + 1))
        printf 'ok     %s\n' "$name"
    else
        fail=$((fail + 1))
        printf 'FAIL   %s\n%s\n' "$name" "$out"
    fi
done

printf '\n%d passed, %d failed\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
