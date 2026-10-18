# ls
set -eu
l="$XCU_BIN/ls"
export LC_ALL=C TZ=UTC
unset COLUMNS
fail() { echo "ls: $*" >&2; exit 1; }
bad()  { if "$l" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is want args...: compare output (newlines as |) with want
is()   { want=$1; shift; got=$("$l" "$@" | tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# co columns want args...: is, with COLUMNS set
co()   { c=$1 want=$2; shift 2; got=$(COLUMNS=$c "$l" "$@" | tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "COLUMNS=$c $*: want $want, got $got"; }
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
cd "$d"
mkdir -p a/b/.h a/c
printf 'aaa' >a/s3; printf 'a' >a/s1; printf 'aa' >a/s2; : >a/.y
touch -t 202001010000 a/s2; touch -t 202101010000 a/s3; touch -t 203001010000 a/s1
touch -a -t 203101010000 a/s2                       # last access: s2 s1 s3
chmod 755 a/s3
ln -s b a/lb; ln -s nope a/dg; mkfifo a/f
touch a/b/z
# names, hidden files
is 'b|c|dg|f|lb|s1|s2|s3|'                a
is '.|..|.y|b|c|dg|f|lb|s1|s2|s3|'        -a a
is '.y|b|c|dg|f|lb|s1|s2|s3|'             -A a
is '.y|b|c|dg|f|lb|s1|s2|s3|'             -aA a     # last of -A -a wins
is '.|..|.y|b|c|dg|f|lb|s1|s2|s3|'        -Aa a
is 'a/.y|'                                a/.y      # named: written
# sorting
is 'a/s1|a/s3|a/s2|'                      -t a/s1 a/s2 a/s3
is 'a/s2|a/s3|a/s1|'                      -tr a/s1 a/s2 a/s3
is 'a/s3|a/s2|a/s1|'                      -S a/s1 a/s2 a/s3
is 'a/s1|a/s2|a/s3|'                      -Sr a/s1 a/s2 a/s3
is 'a/s3|a/s2|a/s1|'                      -tS a/s1 a/s2 a/s3 # last of -S -t wins
is 'a/s1|a/s3|a/s2|'                      -St a/s1 a/s2 a/s3
is 'a/s2|a/s1|a/s3|'                      -tu a/s1 a/s2 a/s3
is 'a/s3|a/s2|a/s1|'                      -uS a/s1 a/s2 a/s3 # no -t: -u does not sort
is 'a/s2|a/s1|a/s3|'                      -tSu a/s1 a/s2 a/s3 # with -t: the last of -S -c -u wins
is 'a/s3|a/s1|a/s2|'                      -fd a/s3 a/s1 a/s2 # -f: not sorted
is 'a/s3|a/s2|a/s1||a/c:||a/b:|z|'        -r a/b a/c a/s1 a/s2 a/s3 # non-directories first, each sorted
# indicators
is 'b/|c/|dg@|f||lb@|s1|s2|s3*|'          -F a
is 'b/|c/|dg|f|lb|s1|s2|s3|'              -p a
is 'b/|c/|dg|f|lb|s1|s2|s3|'              -Fp a     # last wins
# symbolic links named as operands
is 'z|'                                   a/lb      # to a directory: its contents
is 'a/lb@|'                               -F a/lb
is 'a/lb|'                                -d a/lb
is 'z|'                                   -FH a/lb
is 'a/lb/|'                               -dFL a/lb # the referenced file's information, the link's name
is 'a/dg|'                                -L a/dg   # dangling: the link itself
# -d, operands, headers
is 'a|'                                   -d a
is '.|'                                   -d
is 'a/s1||a/b:|z||a/c:|'                  a/c a/s1 a/b
mkdir -p t/b/.h t/c; touch t/x t/b/z
is 't:|b|c|x||t/b:|z||t/c:|'              -R t
is 't:|b|c|x||t/b:|.h|z||t/b/.h:||t/c:|'  -RA t     # hidden directories: if listed
is 't:|x|c|b||t/c:||t/b:|z|'              -Rr t
is '/|'                                   -dR /     # -d: no listing to recurse into
# long format
is 'z|'                                   -lC a/b
got=$("$l" -ln a/s1 a/s3 | sed 's/^\(.\{10\}\) *1 [0-9]* *[0-9]* */\1 1 u g /')
[ "$got" = "-rw-r--r-- 1 u g 1 Jan  1  2030 a/s1
-rwxr-xr-x 1 u g 3 Jan  1  2021 a/s3" ] || fail "-ln: $got"   # future and old: the year
touch a/now
"$l" -l a/now | grep -q ' [0-2][0-9]:[0-5][0-9] a/now$' || fail '-l: recent file has no time'
"$l" -ln a/lb | grep -q '^l.* a/lb -> b$' || fail '-l: symlink contents'
"$l" -lnF a/lb | grep -q ' a/lb -> b/$' || fail '-lF: indicator for the referenced file'
"$l" -lnF a/dg | grep -q ' a/dg -> nope$' || fail '-lF: dangling link'
"$l" -lnL a/lb | grep -q '^total ' || fail '-lL: lists the referenced directory'
"$l" -ldn a | grep -q '^drwx' || fail '-ld: mode'
"$l" -ln a | grep -q '^p.* f$' || fail '-l: FIFO type'
"$l" -ln a | grep -q '^total [0-9]*$' || fail '-l: total line'
"$l" -ln a/s1 | grep -q '^total' && fail '-l: total line for a non-directory operand'
"$l" -ln /dev/null | grep -q '^c.* [0-9][0-9]* [A-Z][a-z][a-z] ' || fail '-l: device info'
mkdir m; chmod 7777 m; [ "$("$l" -ldn m | cut -c1-10)" = drwsrwsrwt ] || fail 'mode: set-ID and sticky'
chmod 7666 m; [ "$("$l" -ldn m | cut -c1-10)" = drwSrwSrwT ] || fail 'mode: not executable'
chmod 755 m
# -l, -n do not disable other formats permanently: the last of -l -C -m -x wins; -1 brings -l back
[ "$("$l" -lm a/s1)" = a/s1 ] || fail '-lm'
"$l" -ml a/s1 | grep -q '^-' || fail '-ml'
"$l" -lx1 a/s1 | grep -q '^-' || fail '-lx1'
# -i, -s, -k
i=$("$l" -i a/s1) ; case $i in [0-9]*' a/s1') ;; *) fail "-i: $i" ;; esac
s=$("$l" -s a/s1) ; case $s in [0-9]*' a/s1') ;; *) fail "-s: $s" ;; esac
"$l" -s a | grep -q '^total ' || fail '-s: total line'
b=$("$l" -s a/s1 | cut -d' ' -f1) k=$("$l" -sk a/s1 | cut -d' ' -f1)
[ "$k" = $(((b + 1) / 2)) ] || fail "-k: $b 512-byte blocks, $k 1024-byte"
# -q, newlines in names
touch "a/c/n
l" "$(printf 'a/c/t\tb')"
is 'n?l|t?b|'                             -q a/c
got=$("$l" a/c 2>/dev/null) && fail 'newline in name exited 0'
[ "$got" = "$(printf 't\tb')" ] || fail "newline in name: $got"
rm a/c/*
# columns
for n in 1 2 3 4 5 6 7; do : >"a/c/$n"; done
co 9  '1  4  7|2  5|3  6|'                -C a/c    # down
co 9  '1  2  3|4  5  6|7|'                -x a/c    # across
co 9  '1, 2, 3,|4, 5, 6,|7|'              -m a/c
co 1  '1|2|3|4|5|6|7|'                    -C a/c    # too narrow: one column, not truncated
co 1  '1,|2,|3,|4,|5,|6,|7|'              -m a/c
co 12 '1  3  5  7|2  4  6|'               -C a/c
co 80 '1  2  3  4  5  6  7|'              -C a/c
is '1, 2, 3, 4, 5, 6, 7|'                 -m a/c
is '1|2|3|4|5|6|7|'                       -mC1 a/c
# errors
out=$("$l" nope a/s1 2>/dev/null) && fail 'missing operand exited 0'
[ "$out" = a/s1 ] || fail 'did not continue past missing operand'
mkdir -p r/s; ln -s .. r/s/up
"$l" -RL r >/dev/null 2>&1 && fail '-RL cycle exited 0'
[ "$("$l" -R r | tr '\n' '|')" = 'r:|s||r/s:|up|' ] || fail '-R followed a symlink'
bad -z
bad -g
bad -o
