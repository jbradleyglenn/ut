# tr
set -eu
t="$XCU_BIN/tr"
fail() { echo "tr: $*" >&2; exit 1; }
bad()  { if printf 'x\n' | "$t" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare output with want (both printf-escaped; od so nul and trailing newlines count)
is()   { in=$1 want=$(printf "$2" | od -An -c); shift 2; got=$(printf "$in" | "$t" "$@" | od -An -c) || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# translate
is 'hello\n'        'HELLO\n'        a-z A-Z
is 'hello\n'        'HELLO\n'        '[:lower:]' '[:upper:]'   # case conversion (EXAMPLES)
is 'HeLLo\n'        'hello\n'        '[:upper:]' '[:lower:]'
is 'xaAy\n'         'XAAY\n'         'x[:lower:]' 'X[:upper:]' # same relative position
is '[a]\n'          '(b)\n'          '[a]' '(b)'               # System V brackets translate to themselves (RATIONALE)
is 'a-b\n'          'xyb\n'          'a-' 'xy'                 # trailing - is itself
is 'a-b\n'          'yxb\n'          -- '-a' 'xy'              # leading - too
is 'abc\n'          'xyc\n'          'ab' 'xyz'                # string2 longer: extra ignored
is '0123456789\n'   'dddddddddd\n'   0-9 '[d*]'                # (APPLICATION USAGE)
is '0123456789\n'   'dddddddddd\n'   0-9 d                     # short string2 padded with its last char (BSD)
is 'abcdef\n'       'xyyyyz\n'       a-f 'x[y*]z'              # [x*] in the middle fills
is 'abcdef\n'       'xxyyyy\n'       a-f '[x*2][y*010]'        # n, leading 0 octal
is 'abcdef\n'       'xxxyzz\n'       a-f '[x*3]yz'
# escapes
is 'a\tb\n'         'a_b\n'          '\t' _
is 'abc\n'          'XYc\n'          '\141-\142' XY            # octal range
is 'a1\n'           'aX\n'           '\0611' X                 # at most 3 octal digits
is 'a-b\\\n'        'axbx\n'         '\-\\' xx
is 'a\0b\n'         'ab\n'           -d '\000'                 # nul in input (APPLICATION USAGE)
# complement
is 'abc\n'          '.b.\n'          -c 'b\n' .                # ascending binary value
is 'abc\n'          'b\n'            -cd 'b\n'
is 'abc123\n'       '123'            -Cd '[:digit:]'
is 'hello, world! 42\n' 'hello\nworld\n' -cs '[:alpha:]' '[\n*]' # one word per line (EXAMPLES)
# delete and squeeze
is 'hello   world\n' 'hello world\n' -s ' '
is 'aabbcc\n'       'xcc\n'          -s ab xx                  # squeeze after translation
is 'aaBB\n'         'AB\n'           -s '[:lower:]' '[:upper:]'
is 'a-a\n'          'a\n'            -ds - a                   # squeeze after deletion
is 'Ab1  \n'        'b1 \n'          -ds '[:upper:]' '[:space:][=1=]' # any class in string2 of -ds
is 'abc\n'          'abc\n'          -d ''
# POSIX locale only: every byte is a character, whatever LANG and LC_* say
export LC_ALL=en_US.UTF-8
is 'h\303\251llo\n' 'H\303\251LLO\n' '[:lower:]' '[:upper:]'   # bytes above 127 are in no class
is 'h\303\251llo\n' 'heello\n'       '\303\251' e              # two bytes, two characters
is 'h\303\251llo\n' 'hllo\n'         -d "$(printf '\303\251')"
is 'h\303\251llo\n' 'h??llo\n'       -c 'a-z\n' '?'
is 'a\377b\n'       'aXb\n'          '\377' X
is 'a\377b\n'       'A\377B\n'       a-z A-Z
is 'a\351b\n'       'axb\n'          -c '\0-\350' x            # the complement is the bytes above \350
is '\200\377\n'     'xx\n'           '\200-\377' x              # a range over high bytes
unset LC_ALL
# errors
bad
bad a
bad -d
bad -d a b
bad -ds a
bad -x a b
bad a ''
bad z-a x
bad '[:nope:]' x
bad a-z '[:digit:]'                 # only case conversion in string2
bad '[:lower:]' '[:lower:]'
bad 'a[:lower:]' '[:upper:]'        # not in the same position
bad a '[=a=]'
bad a-c '[x*][y*]'
