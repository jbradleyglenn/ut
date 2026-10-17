# true -- exits 0
# Must invoke the binary directly: true is a shell builtin, so a bare `true`
# here would pass even if nothing had been built.
set -eu

"$XCU_BIN/tu"
