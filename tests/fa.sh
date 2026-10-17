# false -- exits non-zero
# Must invoke the binary directly: false is a shell builtin, so a bare `false`
# here would test the shell, not this utility.
set -eu

if "$XCU_BIN/fa"; then
    echo 'false exited 0' >&2
    exit 1
fi
