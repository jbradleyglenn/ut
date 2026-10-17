# xcu

posix utilities in C99 + statement expression gnu ext

    make        # build into build/
    make t      # build, then run tests/a.sh
    make c      # remove build/

## Utilities

- `tr`, `fa`: true and false. Exit 0 / 1, ignore arguments.
- `cu`: POSIX cut. `cu -b list [-n] | -c list | -f list [-d delim] [-s]
  [file...]`; list is `n`, `lo-hi`, `-hi`, `lo-` separated by commas or
  blanks. `-c`, `-n` and a multibyte `-d` are LC_CTYPE-aware (an invalid byte
  counts as one character); `-n` writes a character iff its last byte is
  selected, which is what the POSIX low/high adjustment works out to. Mixing
  `-b`/`-c`/`-f`, `-n` without `-b`, or `-d`/`-s` without `-f` is a usage error.
- `ec`: POSIX echo (not XSI). Prints arguments separated by spaces, then a
  newline. No options, no backslash escapes: everything is printed as given.
- `ki`: POSIX kill (XSI forms too). `ki [-s sig | -sig] [--] pid...` sends
  sig (default TERM; name in any case, or number); `ki -l [status]` lists names
  or names the signal for a number or a 128+n exit status.
- `od`: POSIX od (not XSI). `od [-v] [-A d|o|x|n] [-j skip] [-N count]
  [-t type_string]... [file...]`; types `a c d[n] f[n] o[n] u[n] x[n]` with
  sizes `CSIL` / `FDL`, 16-byte lines, `*` for repeated lines, `c` is
  LC_CTYPE-aware (`**` marks the rest of a multibyte character).
