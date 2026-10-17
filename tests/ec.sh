#!/bin/sh
set -eu
check() {
    expected=$1; shift
    actual=$("$XCU_BIN"/ec "$@")
    if [ "$actual" != "$expected" ]; then
        printf 'ec %s: expected %s, got %s\n' "$*" "$expected" "$actual" >&2
        exit 1
    fi
}
checkhex() {
    expected=$1; shift
    actual=$("$XCU_BIN"/ec "$@" | od -A n -t x1 | tr -d ' \n')
    if [ "$actual" != "$expected" ]; then
        printf 'ec %s: expected %s, got %s\n' "$*" "$expected" "$actual" >&2
        exit 1
    fi
}
check 'alpha'               alpha
check 'one two three'       one two three
checkhex 0a                 ''
check '-n -e -E --'         -n -e -E --
check 'one\ntwo'            'one\ntwo'          # no \ interp (not xsi)
check '\a\b\f\n\r\t\v\\'    '\a\b\f\n\r\t\v\\'
checkhex 6f6e655c6374776f2074687265650a 'one\ctwo' three   # \c not special
check '\q\u\i\d'            '\q\u\i\d'
check 'a\'                  'a\'                # trailing backslash
check '\01234 \0060 \077'   '\01234 \0060 \077' # no octal
# write errors must give exit 1
if "$XCU_BIN"/ec hi >&- 2>/dev/null; then echo 'ec: closed stdout exited 0' >&2; exit 1; fi
