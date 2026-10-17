# kill
set -eu
k="$XCU_BIN/ki"
fail() { echo "ki: $*" >&2; exit 1; }
ok()   { "$k" "$@" >/dev/null 2>&1 || fail "$* exited non-zero"; }
bad()  { if "$k" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
is()   { want=$1; shift; got=$("$k" "$@") || fail "$* failed"; [ "$got" = "$want" ] || fail "$*: want $want, got $got"; }
# -l
is TERM -l 15
is KILL -l 9
is TERM -l 143                  # exit status of a TERM-killed process
"$k" -l | grep -qx HUP || fail '-l missing HUP'
bad -l 0
bad -l x
bad -l 1 2
# send: wait's status is 128+sig
st() { sleep 30 & p=$!; "$k" "$@" $p || fail "$* $p failed"; s=0; wait $p || s=$?; echo $s; }
[ "$(st)" = 143 ]            || fail 'default not TERM'
[ "$(st -s KILL)" = 137 ]    || fail '-s KILL'
[ "$(st -s kill)" = 137 ]    || fail '-s kill (case)'
[ "$(st -s HUP --)" = 129 ]  || fail '-s HUP --'   # not INT: sh starts bg jobs with INT ignored
ok -s 0 $$                      # signal 0: existence check
# errors
bad
bad -s
bad -s BOGUS $$
bad -9 $$                       # XSI forms not supported
bad -TERM $$
bad -s 15 $$                    # -s takes a name
bad -s TERM -1                  # options end at -s's argument; needs --
bad x
bad 1x
bad 2147483647                  # no such process
