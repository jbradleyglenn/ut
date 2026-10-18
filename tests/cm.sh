# cmp
set -eu
c="$XCU_BIN/cm"
fail() { echo "cm: $*" >&2; exit 1; }
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
mk()   { printf "$2" >"$d/$1"; }            # mk name content: content is printf-escaped
# is status out err args...: run in $d, compare exit status, stdout and stderr (newlines as |)
is()   {
    st=$1 want=$2 werr=$3; shift 3
    s=0; (cd "$d" && "$c" "$@" >"$d/.out" 2>"$d/.err") || s=$?
    got=$(LC_ALL=C tr '\n' '|' <"$d/.out") err=$(LC_ALL=C tr '\n' '|' <"$d/.err")
    [ "$s" = "$st" ]     || fail "$*: want status $st, got $s"
    [ "$got" = "$want" ] || fail "$*: want stdout '$want', got '$got'"
    [ "$err" = "$werr" ] || fail "$*: want stderr '$werr', got '$err'"
}
mk e  ''; mk e2 ''
mk a  'abc\n'
mk a2 'abc\n'
mk b  'abd\n'
mk p  'ab'
mk l  'abc\nmore'
mk m  'one\ntwo\nthree\n'
mk m2 'one\ntwo\nthrEe\n'
mk n  'one\n\nx'
mk n2 'one\n\ny'
mk x  'a\0\377z'
mk y  'b\0\0z'
# same
is 0 '' ''                     a a2
is 0 '' ''                     e e2
is 0 '' ''                     a a
is 0 '' ''                  -l a a2
is 0 '' ''                  -s a a2
# first difference
is 1 'a b differ: char 3, line 1|' ''              a b
is 1 'b a differ: char 3, line 1|' ''              b a
is 1 'm m2 differ: char 12, line 3|' ''            m m2
is 1 'n n2 differ: char 6, line 3|' ''             n n2   # empty line counts
is 1 'x y differ: char 1, line 1|' ''              x y
# prefix: EOF on the shorter, nothing on stdout
is 1 '' 'cm: EOF on p|'                            p a
is 1 '' 'cm: EOF on p|'                            a p
is 1 '' 'cm: EOF on e|'                            e a
# -l: every difference, decimal byte number, octal bytes
is 1 '3 143 144|' ''                            -l a b
is 1 '1 141 142|3 377 0|' ''                    -l x y
is 1 '' 'cm: EOF on p|'                         -l p a
is 1 '3 144 143|' 'cm: EOF on b|'               -l b l  # differences, then EOF
is 1 '1 141 157|2 142 156|3 143 145|' 'cm: EOF on a|' -l a m
# -s: silent
is 1 '' ''                                      -s a b
is 1 '' ''                                      -s p a
is 1 '' ''                                      -s e a
# - is standard input
[ "$(cd "$d" && "$c" - b <a)" = '- b differ: char 3, line 1' ] || fail '- as file1'
[ "$(cd "$d" && "$c" b - <a)" = 'b - differ: char 3, line 1' ] || fail '- as file2'
(cd "$d" && "$c" a - <a2) || fail '- same'
(cd "$d" && "$c" -- a - <a2) || fail '--'
# bigger than a stdio buffer; difference near the end
awk 'BEGIN{for(i=0;i<20000;i++)print i}' >"$d/big"
sed '$s/9/8/' "$d/big" >"$d/big2"
is 1 'big big2 differ: char 108886, line 20000|' '' big big2
# errors: status 2, even with -s
s=0; (cd "$d" && "$c" a nope) 2>/dev/null || s=$?;    [ $s = 2 ] || fail "missing file: status $s"
s=0; (cd "$d" && "$c" nope a) 2>/dev/null || s=$?;    [ $s = 2 ] || fail "missing file1: status $s"
s=0; (cd "$d" && "$c" -s a nope) 2>/dev/null || s=$?; [ $s = 2 ] || fail "-s missing file: status $s"
[ -n "$(cd "$d" && "$c" -s a nope 2>&1)" ] || fail '-s missing file: no diagnostic'
s=0; "$c" "$d" "$d/a" 2>/dev/null || s=$?;            [ $s = 2 ] || fail "directory: status $s"
s=0; "$c" - - </dev/null 2>/dev/null || s=$?;         [ $s = 2 ] || fail "- -: status $s"
nl="$d/x
y"
cp "$d/a" "$nl"
s=0; "$c" "$nl" "$d/a" 2>/dev/null || s=$?;           [ $s = 2 ] || fail "newline in pathname: status $s"
s=0; "$c" -l "$d/a" "$nl" 2>/dev/null || s=$?;        [ $s = 2 ] || fail "-l newline in pathname: status $s"
"$c" -s "$nl" "$d/a" || fail '-s: newline in pathname is never written'
for args in '' 'a' 'a b c' '-x a b' '-l -s a b' '-s -l a b' '-ls a b'; do
    s=0; (cd "$d" && "$c" $args) 2>/dev/null || s=$?; [ $s = 2 ] || fail "usage '$args': status $s"
done
(cd "$d" && "$c" -l -l a a2) || fail '-l twice'
