# ut

posix utilities in C99 + statement expression gnu ext

    make        # build into build/
    make t      # build, then run tests/a.sh
    make c      # remove build/

# rationale
* utils should be implemented as elegantly and simply as possible, using little code
* extensions like XSI should not be followed, in general
* only the POSIX (C) locale is supported, the code assumes the system has no other