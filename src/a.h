/* a.h - shared declarations for all xcu utilities */
#pragma once
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
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
#include <strings.h>
#include <time.h>
#include <unistd.h>
// no complex.h/tgmath.h: complex.h #defines I

typedef unsigned char C;typedef char*S;typedef int I;typedef intmax_t J;typedef uintmax_t UJ;typedef void V; // J UJ match %jd %ju

#define R return
#define O printf
#define _(e)        ({e;})                          // statement expression: _(a;b) is a value
#define P(b,a...)   if(b)R({a;});                   // predicate: if b, return a
#define W(b...)     while((b))                      // while (extra parens: assignment ok)
#define N(n,a...)   {I i=0,_n=(n);W(i<_n){a;++i;}}  // do a n times, counter i
#define $(b,a...)   if(b){a;}else                   // cond
#define C(i,a...)   case i:{a;}break;               // case
#define S(i,c,a...) switch(i){c default:a;}         // switch: S(x,C(1,..)C(2,..),default)
#define MN(a,b)     ({__typeof__(a)_a=(a),_b=(b);_a<_b?_a:_b;})
#define MX(a,b)     ({__typeof__(a)_a=(a),_b=(b);_a>_b?_a:_b;})

V setprog(S argv0);
I dg(I x,I e,S f,...) __attribute__((format(printf,3,4))); // "prog: f[: strerror]"; exit x if x>=0, else return 1
#define xwarn(a...)   dg(-1,1,a)
#define xwarnx(a...)  dg(-1,0,a)                    /* no errno text */
#define xerr(x,a...)  dg(x,1,a)
#define xerrx(x,a...) dg(x,0,a)                     /* no errno text */
#define nyi()         xerrx(2,"nyi")                /* "prog: nyi", exit 2 */
#define mal(n)        rea(0,n)
V*rea(V*p,size_t n);
S sdp(S s);
I xusage(S synopsis);
V xclose(V);                                        /* fclose stdout, exit 1 on error */
