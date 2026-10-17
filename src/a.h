/* a.h - shared declarations for all xcu utilities */
#pragma once
#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
void setprog(char *argv0);
void xwarn(char *fmt, ...);
void xwarnx(char *fmt, ...);          /* no errno text */
void xerr(int status, char *fmt, ...);
void xerrx(int status, char *fmt, ...); /* no errno text */
void *mal(size_t n);
void *rea(void *p, size_t n);
char *sdp(char *s);
void xusage(char *synopsis);
void nyi(void);                      /* "prog: nyi", exit 2 */
void xclose(void);                    /* fclose stdout, exit 1 on error */
