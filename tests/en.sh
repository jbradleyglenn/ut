# env
set -eu
c="$XCU_BIN/en"
fail() { echo "en: $*" >&2; exit 1; }
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
# no utility: print the environment, one name=value per line
[ "$("$c" -i)" = "" ] || fail '-i: not empty'
[ "$("$c" -i a=1 b=2)" = "$(printf 'a=1\nb=2')" ] || fail '-i a=1 b=2'
[ "$("$c" -i a=1 a=2)" = a=2 ] || fail 'a later operand replaces an earlier one'
[ "$("$c" -i a= b==x)" = "$(printf 'a=\nb==x')" ] || fail 'name is up to the first ='
[ "$("$c" -ii -- a=1)" = a=1 ] || fail '-ii --'
[ "$("$c" - a=1)" = a=1 ] || fail '- as -i'
[ "$(EN_T=x "$c" | grep '^EN_T=')" = EN_T=x ] || fail 'inherited'
[ "$(EN_T=x "$c" EN_T=y | grep -c '^EN_T=')" = 1 ] || fail 'replace, not add'
[ "$(EN_T=x "$c" EN_T=y | grep '^EN_T=')" = EN_T=y ] || fail 'replace inherited'
# utility: run with the modified environment
[ "$(EN_T=x "$c" EN_U=y sh -c 'echo $EN_T$EN_U')" = xy ] || fail 'utility env'
[ "$(EN_T=x "$c" -i EN_U=y /bin/sh -c 'echo ${EN_T-unset}$EN_U')" = unsety ] || fail '-i utility env'
[ "$("$c" -- echo -i)" = -i ] || fail 'utility options are its own'
[ "$("$c" echo -i a=b)" = '-i a=b' ] || fail 'operands after utility'
[ "$("$c" "$c" -i a=1)" = a=1 ] || fail 'nested'
# PATH from an operand is used to find the utility
printf '#!/bin/sh\necho found\n' >"$d/en_t"; chmod +x "$d/en_t"
[ "$("$c" PATH="$d" en_t)" = found ] || fail 'PATH operand'
[ "$("$c" -i PATH="$d" en_t)" = found ] || fail '-i PATH operand'
# exit status
"$c" true || fail 'true'
set +e
"$c" false; [ $? = 1 ] || fail 'exit status of utility'
"$c" sh -c 'exit 42'; [ $? = 42 ] || fail 'exit status 42'
out=$("$c" en_no_such_utility 2>&1); [ $? = 127 ] || fail 'not found: 127'
case $out in *en_no_such_utility*) ;; *) fail "no diagnostic: $out" ;; esac
"$c" "$d/nope" 2>/dev/null; [ $? = 127 ] || fail 'not found, with slash: 127'
: >"$d/nx"; chmod -x "$d/nx"
"$c" "$d/nx" 2>/dev/null; [ $? = 126 ] || fail 'not executable: 126'
"$c" "$d" 2>/dev/null; [ $? = 126 ] || fail 'directory: 126'
"$c" -x 2>/dev/null; r=$?; [ $r -ge 1 ] && [ $r -le 125 ] || fail "-x: $r"
"$c" -i a=1 >&- 2>/dev/null; [ $? = 1 ] || fail "write error"
set -e
