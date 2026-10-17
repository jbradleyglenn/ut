# wc
set -eu
c="$XCU_BIN/wc"
fail() { echo "wc: $*" >&2; exit 1; }
bad()  { if printf 'a\n' | "$c" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare output (newlines as |) with want (both printf-escaped)
is()   { in=$1 want=$(printf "$2"); shift 2; got=$(printf "$in" | "$c" "$@" | tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# counts
is 'a b\nc\n'      '2 3 6|'                              # newlines words bytes, no name, no blanks
is ''              '0 0 0|'
is 'a'             '0 1 1|'                              # no newline: no line counted
is '\n\n'          '2 0 2|'
is '  a  \n'       '1 1 6|'
is 'a\tb\vc\fd\re f\n' '1 6 12|'                         # isspace white space
is 'a\0b\n'        '1 1 4|'                              # nul is not white space
# options select fields, always in default order
is 'a b\nc\n'      '2|'          -l
is 'a b\nc\n'      '3|'          -w
is 'a b\nc\n'      '6|'          -c
is 'a b\nc\n'      '6|'          -m
is 'a b\nc\n'      '2 3|'        -wl
is 'a b\nc\n'      '2 6|'        -c -l
is 'a b\nc\n'      '3 6|'        -mw
is 'a b\nc\n'      '2 3 6|'      -lwc
# POSIX locale only: every byte is a character, whatever LANG and LC_* say
export LC_ALL=en_US.UTF-8
is 'h\303\251llo\n' '7|'         -c
is 'h\303\251llo\n' '7|'         -m
is 'h\303\251llo\n' '1 1 7|'     -mlw
is 'a\343\200\200b\n' '1|'       -w                      # U+3000 bytes are not white space
is 'a\205b\240c\n' '1|'          -w                      # nor are any bytes above 127
unset LC_ALL
# files
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
printf 'a b\n' >"$d/1"; printf 'c\nd\n' >"$d/2"
[ "$("$c" "$d/1")" = "1 2 4 $d/1" ] || fail 'one file: no total'
[ "$("$c" -l "$d/1" "$d/2" | tr '\n' '|')" = "1 $d/1|2 $d/2|3 total|" ] || fail 'total'
[ "$(printf 'x y z' | "$c" -w "$d/1" - "$d/2" | tr '\n' '|')" = "2 $d/1|3 -|2 $d/2|7 total|" ] || fail '- is stdin'
out=$("$c" -l "$d/1" "$d/nope" "$d/2" 2>/dev/null) && fail 'missing file exited 0'
[ "$out" = "$(printf "1 $d/1\n2 $d/2\n3 total")" ] || fail 'did not continue past missing file'
out=$("$c" -l "$d" "$d/1" 2>/dev/null) && fail 'directory exited 0'
[ "$out" = "$(printf "1 $d/1\n1 total")" ] || fail 'directory'
n="$d/a
b"
printf 'x\n' >"$n"
out=$("$c" -l "$n" "$d/1" 2>/dev/null) && fail 'newline in pathname exited 0'
[ "$out" = "$(printf "1 $d/1\n1 total")" ] || fail 'newline in pathname'
# errors
bad -c -m
bad -mc
bad -x
