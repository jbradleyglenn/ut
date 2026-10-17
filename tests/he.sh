# head
set -eu
c="$XCU_BIN/he"
fail() { echo "he: $*" >&2; exit 1; }
bad()  { if printf 'a\n' | "$c" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare output (newlines as |) with want (both printf-escaped)
is()   { in=$1 want=$(printf "$2"); shift 2; got=$(printf "$in" | "$c" "$@" | LC_ALL=C tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# lines
is '1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n' '1|2|3|4|5|6|7|8|9|10|'   # default -n 10
is 'a\nb\nc\n'     'a|b|'        -n 2
is 'a\nb\nc\n'     'a|b|'        -n2
is 'a\nb\nc\n'     'a|b|c|'      -n 5                    # short file: all of it, not an error
is 'a\nb'          'a|b'         -n 5                    # no newline added
is ''              ''            -n 1
is 'a\nb\n'        'a|'          -n 001                  # decimal, leading zeros
is 'a\nb\n'        'a|b|'        -n 99999999999999999999999
is 'a\0b\nc\n'     'a\0b|'       -n 1                    # nul passes through
# bytes
is 'abcdef\n'      'abc'         -c 3
is 'ab\ncd\n'      'ab|c'        -c 4
is 'ab'            'ab'          -c 10
is 'a\0\377b'      'a\0\377'     -c 3                    # arbitrary data
is 'abc'           'a'           -c 3 -c 1               # last one wins
# files
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
printf 'a\nb\n' >"$d/1"; printf 'c\nd\n' >"$d/2"
[ "$("$c" -n 1 "$d/1")" = "a" ] || fail 'one file: no header'
[ "$("$c" -n 1 "$d/1" "$d/2" | tr '\n' '|')" = "==> $d/1 <==|a||==> $d/2 <==|c|" ] || fail 'headers'
[ "$(printf 'x\ny\n' | "$c" -n 1 "$d/1" - | tr '\n' '|')" = "==> $d/1 <==|a||==> - <==|x|" ] || fail '- is stdin'
out=$("$c" -n 1 "$d/nope" "$d/1" "$d/2" 2>/dev/null) && fail 'missing file exited 0'
[ "$(printf '%s' "$out" | tr '\n' '|')" = "==> $d/1 <==|a||==> $d/2 <==|c" ] || fail 'did not continue past missing file'
n="$d/a
b"
printf 'x\n' >"$n"
out=$("$c" -n 1 "$n" "$d/1" 2>/dev/null) && fail 'newline in pathname exited 0'
[ "$(printf '%s' "$out" | tr '\n' '|')" = "==> $d/1 <==|a" ] || fail 'newline in pathname'
# errors
bad -n 0
bad -c 0
bad -n -1
bad -n +1
bad -n 1x
bad -n ''
bad -c 1 -n 1
bad -n 1 -c 1
bad -x
bad -n
