# cut
set -eu
c="$XCU_BIN/cu"
fail() { echo "cu: $*" >&2; exit 1; }
bad()  { if printf 'a:b\n' | "$c" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare output (newlines as |) with want (both printf-escaped)
is()   { in=$1 want=$(printf "$2"); shift 2; got=$(printf "$in" | "$c" "$@" | tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# lists
is 'abcdefghi\n'   'abdefg|'        -c 6,2,4-7,1         # input order, each once (RATIONALE)
is 'abcdefghij\n'  'abcdej|'        -c '-5 10'           # -high, blank-separated
is 'abcdefghi\n'   'ghi|'           -c 7-
is 'abcdefghi\n'   'bcd|'           -c 2-3,3-4           # overlap
is 'abc\n'         '|'              -c 5                 # past the end is not an error
is 'abc\n'         'abc|'           -c 1-99999999999999999999999
is 'ab\ncd'        'b|d|'           -b 2                 # last line without newline still gets one
# fields
is 'a:b:c\n'       'a:c|'           -d : -f 1,3          # joined by one delimiter
is 'a:b:c\n'       'b:c|'           -d: -f2-
is 'a:b:c\n'       '|'              -d : -f 4
is 'a::c\n'        ':c|'            -d : -f 2,3          # empty field
is 'a\tb\n'        'b|'             -f 2                 # default delimiter tab
is 'nodelim\n'     'nodelim|'       -d : -f 2            # passed through intact
is 'x\nnodelim\na:b\n' 'b|'         -d : -s -f 2         # -s drops it
is '\n'            '|'              -d : -f 1
# multibyte
export LC_ALL=en_US.UTF-8
is 'h\303\251llo\n' '\303\251|'     -c 2
is 'h\303\251llo\n' 'l|'            -c 4
is 'h\303\251llo\n' '\303\251l|'    -b 2-4
is 'h\303\251llo\n' '|'             -n -b 2              # first byte of a 2-byte char: dropped
is 'h\303\251llo\n' '\303\251|'     -n -b 3              # last byte: whole char
is 'h\303\251llo\n' 'h|'            -n -b 1-2            # high mid-char backs off
is 'h\303\251llo\n' '\303\251llo|'  -n -b 3-             # low mid-char backs up
is 'a\303\251b\303\251c\n' 'b\303\251c|' -d "$(printf '\303\251')" -f 2-
# -n split guarantee from APPLICATION USAGE: halves rejoin to the original
s=$(printf 'ab\303\251\303\251cd\303\251')
for i in 1 2 3 4 5 6 7 8 9 10; do
  [ "$(printf '%s\n' "$s" | "$c" -n -b 1-$i)$(printf '%s\n' "$s" | "$c" -n -b $((i+1))-)" = "$s" ] || fail "-n split at $i"
done
unset LC_ALL
# files
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
printf 'f1\n' >"$d/1"; printf 'f2\n' >"$d/2"
[ "$(printf 'in\n' | "$c" -c 1- "$d/1" - "$d/2" | tr '\n' '|')" = 'f1|in|f2|' ] || fail 'file operands and -'
out=$("$c" -c 1- "$d/1" "$d/nope" "$d/2" 2>/dev/null) && fail 'missing file exited 0'
[ "$out" = "$(printf 'f1\nf2')" ] || fail 'did not continue past missing file'
# errors
bad
bad -c 0
bad -c 3-2
bad -c 1,,2
bad -c 1-2-3
bad -c -
bad -c x
bad -b 1 -c 1                       # one list option only
bad -c 1 -n                         # -n is for -b
bad -c 1 -s                         # -s, -d are for -f
bad -c 1 -d :
bad -f 1 -d ''
bad -f 1 -d ::
