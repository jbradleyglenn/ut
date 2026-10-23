// date [-u] [+format]
#include"a.h"
#define U "[-u] [+format]"
I main(I ac,S*av){
    I o;size_t n=64,k;S f="+%a %b %e %H:%M:%S %Z %Y",b=0;time_t t;struct tm*m=0;
    setprog(*av);
    W((o=getopt(ac,av,"u"))!=-1)S(o,C('u',if(setenv("TZ","UTC0",1))xerr(1,"setenv")),xusage(U))
    av+=optind;
    if(*av){if(**av!='+'||av[1])xusage(U);f=*av;}       // mmddhhmm[[cc]yy] sets the clock: XSI, not supported
    f=strcat(strcpy(mal(strlen(f)+1),f+1),"\n");       // the newline makes the result nonempty, so 0 only means too small
    tzset();
    if(time(&t)==(time_t)-1||!(m=localtime(&t)))xerr(1,"time");
    W(!(k=strftime(b=rea(b,n),n,f,m)))if((n*=2)>1<<24)xerrx(1,"result too long");
    fwrite(b,1,k,stdout);
    xclose();R 0;
}
