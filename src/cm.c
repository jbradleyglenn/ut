// cmp [-l|-s] file1 file2
#include"a.h"
#define U "[-l|-s] file1 file2"
I m;                                        // mode: 0 first difference, 'l' every difference, 's' silent

FILE*op(S f){FILE*fp=strcmp(f,"-")?fopen(f,"r"):stdin;R fp?fp:(xerr(2,"%s",f),fp);}
I main(I ac,S*av){
    I o,a,b,rc=0;UJ n=1,l=1;S f,g;FILE*p,*q;  // n byte, l line of the current pair: both from 1
    setprog(*av);
    W((o=getopt(ac,av,"ls"))!=-1){if(o=='?'||(m&&m!=o))xusage(U);m=o;} // -l and -s exclude each other
    if(ac-optind!=2)xusage(U);
    f=av[optind];g=av[optind+1];
    if(!strcmp(f,"-")&&!strcmp(g,"-"))xerrx(2,"both operands are standard input"); // undefined: refuse
    if(m!='s'&&(strchr(f,'\n')||strchr(g,'\n')))xerrx(2,"pathname contains a newline"); // FUTURE DIRECTIONS: an error
    p=op(f);q=op(g);
    for(;;n++){
        a=getc_unlocked(p);b=getc_unlocked(q);
        if(a==EOF||b==EOF)break;
        if(a!=b){rc=1;if(m!='l')break;O("%ju %o %o\n",n,a,b);}
        l+=a=='\n';
    }
    if(ferror(p))xerr(2,"%s",f);
    if(ferror(q))xerr(2,"%s",g);
    if((a==EOF)!=(b==EOF))rc=1,m=='s'||xwarnx("EOF on %s",a==EOF?f:g); // one ended first: it is the shorter
    else if(rc&&!m)O("%s %s differ: char %ju, line %ju\n",f,g,n,l);
    if(fflush(stdout)||ferror(stdout))xerr(2,"stdout"); // exit 1 means "differ": a write error must exceed it
    xclose();
    R rc;
}
