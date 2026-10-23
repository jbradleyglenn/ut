# date
set -eu
c="$XCU_BIN/da"
fail() { echo "da: $*" >&2; exit 1; }
# default: %a %b %e %H:%M:%S %Z %Y
out=$("$c" -u)
echo "$out" | grep -Eq '^(Sun|Mon|Tue|Wed|Thu|Fri|Sat) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [ 123][0-9] [0-2][0-9]:[0-5][0-9]:[0-6][0-9] [A-Z]+ [0-9]{4}$' || fail "default: $out"
# +format, with a newline always appended
[ "$("$c" +x)" = x ] || fail '+x'
[ "$("$c" + | od -An -c | tr -d ' ')" = '\n' ] || fail '+ is just a newline'
[ "$("$c" '+%%%n%t' | od -An -c | tr -d ' ')" = '%\n\t\n' ] || fail '%% %n %t'
[ "$("$c" -u +%Z)" = UTC ] || fail '-u: %Z'
[ "$(TZ=EST5 "$c" +%Z)" = EST ] || fail 'TZ'
[ "$(TZ=EST5 "$c" -u +%Z)" = UTC ] || fail '-u overrides TZ'
[ "$(TZ=EST5 "$c" +%H)" = "$(TZ=UTC0 "$c" +%H | awk '{printf "%02d", ($1+19)%24}')" ] || fail 'TZ offset' # racy only at an hour boundary
"$c" -u +%Y%m%d%H%M%S | grep -Eq '^[0-9]{14}$' || fail 'numeric fields'
[ "$("$c" -u '+%Y-%m-%d')" = "$("$c" -u +%F)" ] || fail '%F'
[ "$("$c" -u +%s)" -ge 1700000000 ] || fail '%s: seconds since the epoch'
[ "$("$c" -u +%Ey%Oy)" = "$("$c" -u +%y%y)" ] || fail 'E and O modifiers in the POSIX locale'
[ "$("$c" -- +x)" = x ] || fail '--'
# a long result
f=$(awk 'BEGIN{for(i=0;i<5000;i++)printf "%%Y"}')
[ "$("$c" "+$f" | wc -c | tr -d ' ')" = 20001 ] || fail 'long result'
# errors
set +e
"$c" -x 2>/dev/null; [ $? -gt 0 ] || fail '-x'
"$c" 10231200 2>/dev/null; [ $? -gt 0 ] || fail 'setting the clock is XSI, not supported'
"$c" +a +b 2>/dev/null; [ $? -gt 0 ] || fail 'two operands'
"$c" >&- 2>/dev/null; [ $? = 1 ] || fail 'write error'
set -e
