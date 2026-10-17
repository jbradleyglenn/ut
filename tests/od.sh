# od
set -eu
o="$XCU_BIN/od"
fail() { echo "od: $*" >&2; exit 1; }
bad()  { if printf abc | "$o" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare od output (blanks squeezed) with want
is()   { in=$1 want=$2; shift 2; got=$(printf "$in" | "$o" "$@" | LC_ALL=C tr -s ' ' | LC_ALL=C tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
is 'ab'            '0000000 061141|0000002|'                    # default -t oS, native (little) endian
is 'ab'            ' 61 62|'                  -A n -t x1
is 'abc'           '0000000 61 62 63|0000003|' -t xC
is 'a\n\0\1\\'     '0000000 a \n \0 001 \|0000005|' -t c
is 'a \n\177\377'  '0000000 a sp nl del del|0000005|' -t a   # low 7 bits
is '\377\377'      ' -1 -1| 65535| 255 255|'  -A n -t d1 -t u2 -t u1
is 'abc'           ' 00636261|'               -A n -t x4    # short block padded with nulls
is 'abcdef'        '0000000 c d|0000002|'     -j 2 -N 2 -t c  # offsets exclude skipped bytes
is 'abcdef'        '0000000 b c d e f|0000005|' -j 0x1 -t c
is '\0\0\0\0\0\0\360\77' ' 1.00000000000000e+00|' -A n -t fD
x32=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
is "$x32"          '0000000 x x x x x x x x x x x x x x x x|*|0000060|' -t c
is "$x32"          ' 7878 7878 7878|'         -A n -v -t x2 -N 6 -j 30
(export LC_ALL=en_US.UTF-8; is '\303\251' '0000000 303 251|0000002|' -t c)  # POSIX locale only: bytes, not é
# errors
bad -A q
bad -t q
bad -t x3
bad -t fS
bad -j 1x
bad -j 4                                    # skip past end
