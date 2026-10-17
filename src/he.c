// head [-c number|-n number] [file...]
#include"a.h"
#define U "[-c number|-n number] [file...]"
UJ k=10;I m='n',rc;                         // count; unit: 'c' bytes, 'n' lines

UJ nu(S s){S e;UJ v=isdigit((C)*s)?strtoumax(s,&e,10):0;R v&&!*e?v:xerrx(2,"%s: invalid number",s);} // positive decimal; too big saturates
V cp(FILE*fp){I c;UJ n=0;W(n<k&&(c=getc_unlocked(fp))!=EOF)putchar(c),n+=m=='c'||c=='\n';} // shorter than k: all of it
I main(I ac,S*av){
    I o,h=0;S*fs;
    setprog(*av);
    W((o=getopt(ac,av,"c:n:"))!=-1){if(o=='?'||(h&&h!=o))xusage(U);h=m=o;k=nu(optarg);} // -c and -n exclude each other
    av+=optind;h=0;
    for(fs=*av?av:(S[]){"-",0};*fs;fs++){
        FILE*fp;
        if(strchr(*fs,'\n')){rc=xwarnx("pathname contains a newline");continue;} // FUTURE DIRECTIONS: an error
        if(!(fp=strcmp(*fs,"-")?fopen(*fs,"r"):stdin)){rc=xwarn("%s",*fs);continue;}
        if(av[0]&&av[1])O(&"\n==> %s <==\n"[!h++],*fs);   // the first header has no leading newline
        cp(fp);
        if(ferror(fp))rc=xwarn("%s",*fs);
        if(fp==stdin)clearerr(fp);else fclose(fp);
    }
    xclose();
    R rc;
}
