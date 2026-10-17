# cat
set -eu
c="$XCU_BIN/ca"
fail() { echo "ca: $*" >&2; exit 1; }
bad()  { if printf 'a\n' | "$c" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
# is input want args...: compare output (newlines as |) with want (both printf-escaped)
is()   { in=$1 want=$(printf "$2"); shift 2; got=$(printf "$in" | "$c" "$@" | tr '\n' '|') || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
printf 'f1\n' >"$d/1"; printf 'f2' >"$d/2"; : >"$d/e"
is 'a\nb\n'        'a|b|'                                 # no operands: stdin
is 'a'             'a'                                    # nothing added
is ''              ''
is 'a\n'           'a|'          -u
is 'in\n'          'f1|f2in|'    "$d/1" "$d/2" -          # in order, - is stdin
is 'in\n'          'f1|in|f1|'   "$d/1" - "$d/1" - "$d/e" # a second - is at end of a pipe
is 'x'             'f1|'         -- "$d/1"
# bytes, untouched
head -c 200000 /dev/urandom >"$d/r"                       # bigger than one read
"$c" "$d/r" | cmp -s - "$d/r" || fail 'binary file'
"$c" -u - <"$d/r" | cmp -s - "$d/r" || fail 'binary stdin'
cat "$d/r" "$d/1" "$d/r" >"$d/w"; "$c" "$d/r" "$d/1" "$d/r" | cmp -s - "$d/w" || fail 'binary files'
# errors: diagnose, keep going, exit nonzero
out=$("$c" "$d/1" "$d/nope" "$d/1" 2>"$d/err") && fail 'missing file exited 0'
[ "$out" = "$(printf 'f1\nf1')" ] || fail 'did not continue past missing file'
[ -s "$d/err" ] || fail 'no diagnostic'
out=$("$c" "$d" "$d/1" 2>/dev/null) && fail 'directory exited 0'
[ "$out" = f1 ] || fail 'directory'
bad -x
bad -u -n
