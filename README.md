# ut

posix utilities in C99 + statement expression gnu ext

    make        # build into build/
    make t      # build, then run tests/a.sh
    make c      # remove build/

# rationale
* utils should be implemented as elegantly and simply as possible, using little code
* extensions like XSI should not be followed, in general
* only C and C.UTF-8 locales are supported
