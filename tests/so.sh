# sort
set -eu
s="$XCU_BIN/so"
export LC_ALL=C
fail() { echo "so: $*" >&2; exit 1; }
bad()  { if printf 'b\na\n' | "$s" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare output (newlines as |) with want
is()   { in=$1 want=$(printf -- "$2"); shift 2; got=$(printf -- "$in" | "$s" "$@" | tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# st input want args...: exit status
st()   { in=$1 want=$2; shift 2; got=0; printf -- "$in" | "$s" "$@" >/dev/null 2>&1 || got=$?; [ "$got" = "$want" ] || fail "$*: want rc $want, got $got"; }
is 'b\na\nc\n'          'a|b|c|'
is 'b\na'               'a|b|'                      # final incomplete line gets a newline
is ''                   ''
is 'b\na\nc\n'          'c|b|a|'          -r
is 'a\nb\na\n'          'a|b|'            -u
is 'b\nA\na\nB\n'       'A|a|B|b|'        -f        # ties: all bytes, so A before a
is 'a\nA\n'             'a|'              -fu       # one of each set of equal keys
is 'b\n.c\n'            'b|.c|'           -d
is '\001b\na\n'         'a|\001b|'        -i
is '10\n9\n-1\n'        '-1|9|10|'        -n
is '-0\n0\n+1\n-.1\n-0.0\n.01\n0.1\n-10\n-9\n1e3\n' '-10|-9|-.1|+1|-0|-0.0|0|.01|0.1|1e3|' -n
is '1 b\n2 a\n'         '2 a|1 b|'        -k2
is 'x 10\nx 9\n'        'x 9|x 10|'       -k2n
is 'x 10\nx 9\n'        'x 10|x 9|'       -k2nr
is 'a  b\na c\n'        'a  b|a c|'       -k2       # blanks belong to the field
is 'a  c\na b\n'        'a b|a  c|'       -k2b
is 'a  c\na b\n'        'a b|a  c|'       -b -k2
is 'a:2:x\nb:10:y\n'    'a:2:x|b:10:y|'   -t: -k2n
is 'a::2\nb:1:1\n'      'a::2|b:1:1|'     -t: -k2,2 # each separator is significant
is 'b:1\na:1\nc:0\n'    'c:0|a:1|b:1|'    -t: -k2,2 -k1,1
is 'b:1\na:1\nc:0\n'    'c:0|b:1|a:1|'    -t: -k2,2 -k1,1r
is 'xab\nyaa\n'         'yaa|xab|'        -k1.2
is 'xab\nyaa\n'         'xab|yaa|'        -k1.2,1.2 # ties broken by whole line
is 'ab c\naa d\n'       'aa d|ab c|'      -k1.1,1.0 # .0: end of field
is 'a 2\nb 1\n'         'b 1|a 2|'        -k2,2n -k1,1
is 'x 1\ny 1\n'         'y 1|x 1|'        -r -k2,2n # -r still applies to the tie-break
is 'b\na\n'             'a|b|'            -m
is 'a b\nb a\n'         'a b|b a|'        -k3       # missing field: empty key
(export LC_ALL=en_US.UTF-8
 is '1,000\n999\n'      '999|1,000|'      -n        # thousands separator
 is '\303\251b\nea\n'   'ea|\303\251b|'   -k1.2     # characters, not bytes
 is 'x\303\227b\ny\303\227a\n' 'y\303\227a|x\303\227b|' -t "$(printf '\303\227')" -k2)
# -o may name an input
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
printf 'b\na\n' >"$d/f"; "$s" -o "$d/f" "$d/f"; [ "$(cat "$d/f")" = "$(printf 'a\nb')" ] || fail '-o input'
[ "$(printf 'c\n' | "$s" "$d/f" -)" = "$(printf 'a\nb\nc')" ] || fail 'file and -'
# -c -C
st 'a\nb\n' 0 -c
st 'b\na\n' 1 -c
st 'a\na\n' 0 -c
st 'a\na\n' 1 -cu
st 'b\na\n' 1 -C
[ -z "$(printf 'b\na\n' | "$s" -C 2>&1)" ] || fail '-C not silent'
printf 'b\na\n' | "$s" -c 2>&1 | grep -q disorder || fail '-c no message'
# errors
st '' 2 "$d/none"
bad -k0
bad -k1.0
bad -k1,0
bad -k1x
bad -t ab
bad -c "$d/f" "$d/f"
