# command
set -eu
c="$XCU_BIN/co"
fail() { echo "co: $*" >&2; exit 1; }
d=$(cd "$(mktemp -d)" && pwd -P); trap 'rm -rf "$d"' EXIT   # -P: getcwd gives the physical path
# is status out err args...: run in $d with PATH=$P, compare exit status, stdout and stderr (newlines as |; err '?' means any non-empty)
P=$d/bin
is()   {
    st=$1 want=$2 werr=$3; shift 3
    s=0; (cd "$d" && PATH=$P "$c" "$@" >"$d/.out" 2>"$d/.err") || s=$?
    got=$(tr '\n' '|' <"$d/.out") err=$(tr '\n' '|' <"$d/.err")
    [ "$s" = "$st" ]     || fail "$*: want status $st, got $s"
    [ "$got" = "$want" ] || fail "$*: want stdout '$want', got '$got'"
    if [ "$werr" = '?' ]; then [ -n "$err" ] || fail "$*: want a diagnostic"
    else [ "$err" = "$werr" ] || fail "$*: want stderr '$werr', got '$err'"; fi
}
mkdir "$d/bin" "$d/a" "$d/bin/dd" "$d/a/dd"
printf '#!/bin/sh\necho hi "$@"\n' >"$d/bin/hi"
printf '#!/bin/sh\nexit 3\n' >"$d/bin/ex"
printf 'echo plain "$#" "$@"\n' >"$d/bin/plain"     # no #!: execv fails with ENOEXEC, so sh runs it
printf '#!/bin/sh\necho nox\n' >"$d/bin/nox"
printf '#!/bin/sh\necho shadowed\n' >"$d/a/hi"      # not executable: the search skips it
chmod +x "$d/bin/hi" "$d/bin/ex" "$d/bin/plain"
# -v: absolute pathnames for files found
is 0 "$d/bin/hi|" ''                -v hi
is 0 "$d/bin/hi|" ''             -v -- hi
is 1 '' ''                         -pv hi       # -p: the default path, without $d/bin
is 0 "$d/bin/hi|" ''           -pv bin/hi       # a slash: no search, -p or not
is 127 '' 'co: -v: not found|'   -- -v hi       # after --, -v is the command name
P=$d/a:$d/bin
is 0 "$d/bin/hi|" ''                -v hi       # not executable, then a directory: both skipped
is 1 '' ''                          -v dd
P=bin
is 0 "$d/bin/hi|" ''                -v hi       # a relative prefix
P=./bin/
is 0 "$d/bin/hi|" ''                -v hi       # a trailing slash: no second slash added
cp "$d/bin/hi" "$d/hi"
P=/nonexistent::$d/bin
is 0 "$d/hi|" ''                    -v hi       # a zero-length prefix is the current directory
P=$d/bin:
is 0 "$d/bin/hi|" ''                -v hi
rm "$d/hi"
P=$d/bin
is 0 "$d/bin/hi|" ''                -v ./bin/hi # a slash: no search, but still absolute
is 0 "$d/bin/hi|" ''                -v "$d/bin/hi"
is 0 "$d/bin/../bin/hi|" ''         -v bin/../bin/hi
is 1 '' ''                          -v ./hi
is 1 '' ''                          -v bin/nox
is 1 '' ''                          -v bin
is 1 '' ''                          -v nope
is 1 '' ''                          -v ''
# names whose meaning POSIX fixes, so the same in every shell
for w in ! '{' '}' case do done elif else esac fi for if in then until while \
         break : continue . eval exec exit export readonly return set shift times trap unset \
         alias bg cd command fc fg getopts hash jobs kill read type ulimit umask unalias wait; do
    is 0 "$w|" ''                   -v "$w"
done
# -V: unspecified format, but the kind and the pathname
is 0 "hi is $d/bin/hi|" ''          -V hi
is 0 'if is a reserved word|' ''    -V if
is 0 'exit is a special built-in utility|' '' -V exit
is 0 'cd is a regular built-in utility|' ''   -V cd
is 1 '' 'co: nope: not found|'      -V nope
is 1 '' 'co: nope: not found|'      -pV nope
# -p: the default path, without changing the utility's PATH
P=/nonexistent
is 1 '' ''                          -v sh
[ "$(PATH=/nonexistent "$c" -pv sh)" = "$(command -pv sh)" ] || fail '-p: not the default path'
[ "$(PATH=/nonexistent "$c" -p sh -c 'echo "$PATH"')" = /nonexistent ] || fail '-p: PATH changed'
P=$d/bin
# execution: the utility's status, arguments and options are its own
is 0 'hi|' ''                       hi
is 0 'hi a  b -v --|' ''            hi a ' b' -v --
is 0 'hi -p|' ''                 -- hi -p
is 3 '' ''                          ex
is 0 'plain 2 x y|' ''              plain x y
is 0 'hi|' ''                       ./bin/hi
is 0 'hi|' ''                       "$d/bin/hi"
is 0 'hi z|' ''                  -p "$d/bin/hi" z
[ "$(PATH=/nonexistent "$c" -p sh -c 'echo ok')" = ok ] || fail '-p: execute'
[ "$(PATH=$XCU_BIN "$c" co co ec x)" = x ] || fail 'command command'
# not found: 127; found but not invocable: 126
is 127 '' 'co: nope: not found|'    nope
is 127 '' 'co: : not found|'        ''
is 126 '' '?'                       nox         # in PATH but not executable: found, not invocable
is 127 '' 'co: exit: not found|'    exit        # a special built-in: no shell here to run it
is 127 '' '?'                       ./nope
is 127 '' '?'                       bin/nope/x
is 126 '' '?'                       bin/nox
is 126 '' '?'                       ./bin       # a directory
P=$d/a:$d/bin
is 0 'hi|' ''                       hi          # a file that fails is passed over, as with execvp
is 126 '' '?'                       dd
P=$d/bin
# usage: 127, an error in command itself
for args in '' '-v' '-x hi' '-vV hi' '-Vv hi' '-v hi hi' '-V hi x' '-p'; do
    is 127 '' '?' $args
done
is 0 "$d/bin/hi|" ''             -v -v hi
is 127 '' 'co: -: not found|'       -           # "-" is an operand
# FUTURE DIRECTIONS: a pathname with a newline is an error
nl="$d/x
y"
mkdir "$nl"; cp "$d/bin/hi" "$nl/"
P=$nl
is 1 '' '?'                         -v hi
is 1 '' '?'                         -V hi
is 0 'hi|' ''                       hi          # running it is fine
P=$d/bin
# write error
s=0; PATH=$P "$c" -v hi >&- 2>/dev/null || s=$?; [ $s = 1 ] || fail "write error: status $s"
:
