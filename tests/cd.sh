# cd: a separate process, so test what it reports: stdout and exit status
set -eu
c="$XCU_BIN/cd"
fail() { echo "cd: $*" >&2; exit 1; }
bad()  { if "$c" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
is()   { [ "$1" = "$2" ] || fail "$3: want '$2', got '$1'"; }
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
d=$(cd -P "$d" && pwd)                    # physical: macOS's /var is a link
cd "$d"
mkdir -p a/b s/t; : >f; ln -s a/b l
# silent on success
is "$("$c" a)" "" 'relative'
is "$("$c" "$d/a/b")" "" 'absolute'
is "$(HOME="$d/a" "$c")" "" 'HOME'
"$c" . && "$c" .. && "$c" ./a/../s || fail 'dot, dot-dot'
# -: cd "$OLDPWD" && pwd
is "$(OLDPWD="$d/a" "$c" -)" "$d/a" '-'
is "$(OLDPWD="$d/a//./b/" "$c" -)" "$d/a/b" '-: canonical'
# -L: dot-dot is lexical; -P: physical
is "$(OLDPWD="$d/l/.." "$c" -)" "$d" '-L dot-dot past a link'
is "$(OLDPWD="$d/l/.." "$c" -L -)" "$d" '-L'
is "$(OLDPWD="$d/l" "$c" -)" "$d/l" '-L keeps the link'
is "$(OLDPWD="$d/l" "$c" -P -)" "$d/a/b" '-P resolves the link'
is "$(OLDPWD="$d/l/.." "$c" -P -)" "$d/a" '-P dot-dot past a link'
is "$(OLDPWD="$d/l" "$c" -L -P -)" "$d/a/b" 'last of -L -P'
is "$(OLDPWD="$d/l" "$c" -P -L -)" "$d/l" 'last of -P -L'
is "$(cd l && OLDPWD=.. "$c" -)" "$d" '-L: relative to PWD'
is "$(cd l && OLDPWD=.. "$c" -P -)" "$d/a" '-P: relative to the directory'
# CDPATH: print only when a non-null entry was used
is "$(CDPATH="$d/s" "$c" t)" "$d/s/t" 'CDPATH'
is "$(CDPATH="$d/nope:$d/s/" "$c" t)" "$d/s/t" 'CDPATH second entry, trailing slash'
is "$(CDPATH=":$d/s" "$c" a)" "" 'CDPATH null entry is dot'
is "$(CDPATH="$d/s" "$c" a)" "" 'CDPATH no match: the operand'
is "$(CDPATH=s "$c" t)" "$d/s/t" 'CDPATH relative entry'
is "$(CDPATH="$d/s" "$c" ./t 2>/dev/null || echo no)" no 'CDPATH: not for dot'
is "$(cd a && CDPATH="$d/s" "$c" ../s)" "" 'CDPATH: not for dot-dot'
is "$(CDPATH="$d/s" "$c" "$d/a")" "" 'CDPATH: not for absolute'
is "$(CDPATH="$d" "$c" l/..)" "$d" 'CDPATH -L'
is "$(CDPATH="$d" "$c" -P l/..)" "$d/a" 'CDPATH -P'
# errors
bad nope
bad f
bad f/..                                  # -L: dot-dot after a non-directory
bad -P f/..
bad ""
bad a s
bad -x a
(unset HOME; bad) || exit 1
HOME= bad
(unset OLDPWD; bad -) || exit 1
"$c" -P -e nope 2>/dev/null && fail '-P -e: exited 0'
"$c" -P -e nope 2>/dev/null || [ $? -gt 1 ] || fail '-P -e: error status not >1'
"$c" -e nope 2>/dev/null || [ $? -ge 1 ] || fail '-e without -P'
out=$("$c" nope 2>&1) || true
case $out in *nope:*) ;; *) fail "no diagnostic: $out" ;; esac
# no PWD: still works, physically
is "$(cd l && env -u PWD OLDPWD=.. "$c" -)" "$d/a" 'no PWD'
