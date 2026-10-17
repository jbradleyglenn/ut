#include "a.h"
char *prog = "ut";
void setprog(char*argv0){
    char *p;
    if(argv0==NULL||*argv0=='\0')return;
    p=strrchr(argv0,'/');
    prog=p!=NULL?p+1:argv0;
}

static void vdiag(int use_errno, char *fmt, va_list ap)
{
    int saved = errno;
    fflush(stdout);
    fprintf(stderr, "%s: ", prog);
    if (fmt != NULL) vfprintf(stderr, fmt, ap);
    if (use_errno) fprintf(stderr, "%s%s", fmt != NULL ? ": " : "", strerror(saved));
    fputc('\n', stderr);
}

void
xwarn(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vdiag(1, fmt, ap);
    va_end(ap);
}

void
xwarnx(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vdiag(0, fmt, ap);
    va_end(ap);
}

void
xerr(int status, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vdiag(1, fmt, ap);
    va_end(ap);
    exit(status);
}

void xerrx(int status, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vdiag(0, fmt, ap);
    va_end(ap);
    exit(status);
}

void
xusage(char *synopsis)
{
    fprintf(stderr, "usage: %s %s\n", prog, synopsis);
    exit(2);
}

/* not yet implemented */
void
nyi(void)
{
    xerrx(2, "nyi");
}

void *
mal(size_t n)
{
    void *p = malloc(n != 0 ? n : 1);

    if (p == NULL)
        xerrx(1, "out of memory");
    return p;
}

void *
rea(void *p, size_t n)
{
    void *q = realloc(p, n != 0 ? n : 1);

    if (q == NULL)
        xerrx(1, "out of memory");
    return q;
}

char *
sdp(char *s)
{
    size_t n = strlen(s) + 1;
    char *p = mal(n);

    memcpy(p, s, n);
    return p;
}

/* close stdout, exit 1 on any write error (earlier or at close); call last */
void
xclose(void)
{
    int bad = ferror(stdout);
    errno = 0;
    if (fclose(stdout) != 0 || bad) {
        int e = errno;
        fprintf(stderr, "%s: stdout%s%s\n", prog, e ? ": " : "", e ? strerror(e) : ": write error");
        exit(1);
    }
}
