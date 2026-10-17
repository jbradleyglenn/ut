# ut

posix utilities in C99 + statement expression gnu ext

    make        # build into build/
    make t      # build, then run tests/a.sh
    make c      # remove build/

# rationale
* utils should be implemented as elegantly and simply as possible, using little code
* extensions like XSI should not be followed, in general
* only the POSIX (C) locale is supported, the one locale POSIX requires. the code assumes the system
  has no other: utilities never call setlocale, so LANG and LC_* are ignored, every byte is one
  character, and character classes and collation are those of the POSIX locale
