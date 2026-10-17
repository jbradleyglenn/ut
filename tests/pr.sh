# printf
set -eu
p="$XCU_BIN/pr"
fail() { echo "pr: $*" >&2; exit 1; }
# is want args...: compare output (newlines as |, made visible by od for odd bytes) with want (printf-escaped)
is()   { want=$(printf %b\| "$1"); shift; got=$("$p" "$@"; printf '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# err want args...: like is, but must also exit nonzero and write a diagnostic
err()  { want=$(printf %b\| "$1"); shift; got=$("$p" "$@" 2>/dev/null || :; printf '|')
         "$p" "$@" >/dev/null 2>&1 && fail "$* exited 0"; [ -n "$("$p" "$@" 2>&1 >/dev/null)" ] || fail "$*: no diagnostic"
         [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
hex()  { want=$1; shift; got=$("$p" "$@" | od -A n -t x1 | tr -d ' \n'); [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# EXAMPLES
is '    1  21\n  3214321\n54321   0\n'  '%5d%4d\n' 1 21 321 4321 54321   # reuse; missing is 0
is '10\n8\n16\n'                     '%d\n' 10 010 0x10
is '3\n3\n-3\n51\n43\n45\n'          '%d\n' 3 +3 -3 \'3 \"+3 "'-3"      # char constants; rest ignored
err '5\n'                            '%d\n' 5a
err '0\n'                            '%d\n' ABC
err '9223372036854775807\n'          '%d\n' 99999999999999999999         # overflow: strtoimax's value
err '-9223372036854775808\n'         '%d\n' -99999999999999999999
err '1\n2\n'                         '%d\n' 1x 2                         # keeps going
is '10.00\n10.00\n16.00\n1010.00\n1010.00\n64.25\n' '%.2f\n' 10 010 0x10 10.1e2 010.1e2 0x10.1p2
err '1.00\n'                         '%.2f\n' 1x
# integer conversions
is 'ff FF 10 010 0xff 18446744073709551615 42\n' '%x %X %o %#o %#x %u %i\n' 255 255 8 8 255 -1 42
is '+5| 5|00005|5   |005|   -5\n'    '%+d|% d|%05d|%-4d|%.3d|%5d\n' 5 5 5 5 5 -5
# strings
is '[   ab][ab   ][ab][x][   ]\n'    '[%5s][%-5s][%.2s][%c][%3c]\n' ab ab abc xyz ''
is 'a,b,c,'                          '%s,' a b c
is '|||0|\n'                         '%s|%b|%c|%d|\n'                    # missing: null string / 0
is 'hi\n'                            'hi\n' extra                        # no conversions: once
# format escapes
hex 614100715c25                     'a\101\0q\\%%'
hex 5c71215c63                       '\q\41\c'                           # unknown \ literal; \c not special here
hex 0a5c                             '\012\'                             # trailing backslash
hex 5334                             '\1234'                             # at most 3 digits
# %b
is 'a\tbA'                           '%b|%s\n' 'a\tb\0101\c' x           # \c stops everything
hex 0809000a                         '%b\n' '\010\011\0'                 # \0ddd, bare \0
hex 0131                             '%b' '\00011'                       # \0 plus at most 3 digits
is 'x\\\n'                           '%b\n' 'x\'
is '    x|x    |\n'                  '%5.1b|%-5b|\n' xyz x
hex 2000610a                         '%3b\n' '\0a'                       # null bytes are written, and count
# numbered arguments
is 'three 1\n'                       '%3$s %1$d\n' 1 2 three
is 'b a\nd c\n'                      '%2$s %1$s\n' a b c d               # reuse past the highest used
is '1%2\n'                           '%1$d%%%2$d\n' 1 2
err 'a b\nc '                       '%1$s %2$s\n' a b c                 # missing numbered: error, mid-pass
# multibyte char constant
export LC_ALL=en_US.UTF-8; is '233 8364\n' '%d %d\n' "'é" "'€"; unset LC_ALL
# usage and bad formats
is '-- x\n'                          -- '%s %s\n' -- x                   # first -- discarded
"$p" >/dev/null 2>&1 && fail 'no format exited 0'
err 'x'                              'x%y'
err 'x'                              'x%'
err ''                               '%*d' 5 1                           # * is not supported
# write errors must give exit 1
if "$p" hi >&- 2>/dev/null; then fail 'closed stdout exited 0'; fi
