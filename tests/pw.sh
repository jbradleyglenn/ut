# pw: PWD if it's a clean absolute name of the working directory (-L), else the physical name (-P)
set -eu
p="$XCU_BIN/pw"
fail() { echo "pw: $*" >&2; exit 1; }
bad()  { if "$p" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
is()   { [ "$1" = "$2" ] || fail "$3: want '$2', got '$1'"; }
d=$(mktemp -d); trap 'rm -rf "$d"' EXIT
d=$(cd -P "$d" && pwd)                    # physical: macOS's /var is a link
mkdir -p "$d/a/b"; ln -s a/b "$d/l"
cd "$d/l"
is "$(PWD="$d/l" "$p")" "$d/l" '-L default keeps the link'
is "$(PWD="$d/l" "$p" -L)" "$d/l" '-L'
is "$(PWD="$d/l" "$p" -P)" "$d/a/b" '-P resolves the link'
is "$(PWD="$d/l" "$p" -L -P)" "$d/a/b" 'last of -L -P'
is "$(PWD="$d/l" "$p" -P -L)" "$d/l" 'last of -P -L'
is "$(PWD="$d//l/" "$p")" "$d//l/" '-L: extra slashes are fine'
# -L falls back to -P when PWD is unusable
is "$(PWD="$d/l/." "$p")" "$d/a/b" 'dot'
is "$(PWD="$d/./l" "$p")" "$d/a/b" 'dot inside'
is "$(PWD="$d/a/b/../b" "$p")" "$d/a/b" 'dot-dot'
is "$(PWD="$d/a" "$p")" "$d/a/b" 'another directory'
is "$(PWD="$d/nope" "$p")" "$d/a/b" 'nonexistent'
is "$(PWD=l "$p")" "$d/a/b" 'relative'
is "$(PWD= "$p")" "$d/a/b" 'empty'
is "$(env -u PWD "$p")" "$d/a/b" 'unset'
mkdir "$d/.x"; cd "$d/.x"
is "$(PWD="$d/.x" "$p")" "$d/.x" 'dot-prefixed name is not dot'
# errors
bad x
bad -x
bad -- x
