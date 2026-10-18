# mkdir
set -eu
c="$XCU_BIN/md"
fail() { echo "md: $*" >&2; exit 1; }
bad()  { if "$c" "$@" >/dev/null 2>&1; then fail "$* exited 0"; fi; }
pm()   { ls -ld "$1" | cut -c1-10; }
# is mode want: md -m mode t gives t mode want (under umask 022)
is()   { "$c" -m "$1" t || fail "-m $1 failed"; got=$(pm t); rmdir t; [ "$got" = "$2" ] || fail "-m $1: want $2, got $got"; }
d=$(mktemp -d); trap 'chmod -R u+rwx "$d"; rm -rf "$d"' EXIT
cd "$d"
umask 022
# default mode: a=rwx less the mask
"$c" a b; [ "$(pm a)" = drwxr-xr-x ] && [ "$(pm b)" = drwxr-xr-x ] || fail 'default mode'
(umask 077; "$c" u) && [ "$(pm u)" = drwx------ ] || fail 'default mode, umask 077'
# -m octal: absolute, regardless of the mask
is 755            drwxr-xr-x
is 777            drwxrwxrwx
is 0700           drwx------
is 0              d---------
is 4711           drws--x--x
is 1777           drwxrwxrwt
# -m symbolic: relative to a=rwx; no who means all, but not the bits in the mask
is u=rwx,go=rx    drwxr-xr-x
is go=            drwx------
is a-w            dr-xr-xr-x
is a-w,u+w        drwxr-xr-x
is -w             dr-xrwxrwx
is =r             dr--r--r--
is =rx,+w         drwxr-xr-x
is a+rwx,g-w,o-wx drwxr-xr--
is u+s            drwsrwxrwx
is o+s            drwxrwxrwx              # o and s: no set-ID bit
is og-x           drwxrw-rw-
is ug=rwX,o=      drwxrwx---              # a directory: X is x
is go-rwx,g=u     drwxrwx---              # permcopy: what u is now
is g=o-r          drwx-wxrwx              # one wholist, two actions
is u-rwx+r        dr--rwxrwx
is a=             d---------
is +              drwxrwxrwx              # op, no perm: no change
(umask 077; is -x drw-rwxrwx) || exit 1
# -p
"$c" -p p/q//r/ && [ -d p/q/r ] || fail '-p'
[ "$(pm p)" = drwxr-xr-x ] && [ "$(pm p/q)" = drwxr-xr-x ] || fail '-p: intermediate mode'
"$c" -p p p/q p/q/r || fail '-p: existing directories are not an error'
"$c" -p -m 700 m/n && [ "$(pm m)" = drwxr-xr-x ] && [ "$(pm m/n)" = drwx------ ] || fail '-p -m: mode of the last only'
(umask 777; "$c" -p x/y) && [ "$(pm x)" = d-wx------ ] && [ "$(pm x/y)" = d--------- ] || fail '-p: u+wx for intermediates'
"$c" -p /tmp/ || fail '-p: an absolute existing path'
# errors
out=$("$c" a z 2>&1) && fail 'existing directory exited 0'
[ -d z ] || fail 'did not continue past an error'
case $out in *a:*) ;; *) fail "no diagnostic: $out" ;; esac
bad q/z                                   # no -p: missing parent
[ ! -e q ] || fail 'without -p, made the parent'
: >f
bad f                                     # exists, not a directory
bad -p f
bad -p f/g
bad -p f/g/h
bad "$(printf 'n\nl')"
[ ! -e "$(printf 'n\nl')" ] || fail 'made a directory with a newline'
bad
bad -x a
bad -m
bad -m '' w
bad -m 8 w
bad -m 7778 w
bad -m 99999 w
bad -m u w
bad -m x w
bad -m u+rg w
bad -m u+r, w
bad -m ,u+r w
bad -m +t w                               # t: XSI
bad -m 'u=r g' w
[ ! -e w ] || fail 'invalid mode: made a directory'
