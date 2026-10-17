// cut -b list [-n] [file...]
// cut -c list [file...]
// cut -f list [-d delim] [-s] [file...]
#include"a.h"
#define U "-b list [-n] [file...] | -c list [file...] | -f list [-d delim] [-s] [file...]"
struct r{UJ lo,hi;}*r;I nr,ns,sp,rc;char m; // list ranges (hi UINTMAX_MAX for low-); mode b c f; -n; -s
S dl;                                       // delimiter, one byte

UJ nu(S*s,S l){UJ v=isdigit((C)**s)?strtoumax(*s,s,10):0;R v?v:xerrx(2,"%s: invalid list",l);} // positive; too big saturates
V addl(S l){                                 // n  low-high  -high  low-  separated by , or blanks
    for(S s=l;;s++){
        I d=*s=='-';UJ lo=d?1:nu(&s,l),hi=lo;
        if(*s=='-')s++,hi=d||isdigit((C)*s)?nu(&s,l):UINTMAX_MAX;
        if(lo>hi||(*s&&!strchr(", \t",*s)))xerrx(2,"%s: invalid list",l);
        r=rea(r,++nr*sizeof*r);r[nr-1]=(struct r){lo,hi};
        if(!*s)R;
    }
}
I sel(UJ k){N(nr,P(r[i].lo<=k&&k<=r[i].hi,1))R 0;}
S dm(S q,S e){S d=memchr(q,*dl,e-q);R d?d:e;}  // next delimiter, or e
V ln(S p,size_t n){
    S e=p+n,q;UJ k=0;I o=0;
    if(m!='f')N(n,if(sel(i+1))putchar(p[i]))   // -b, -c: a character is a byte, so -n splits none
    else if((q=dm(p,e))==e){if(!sp)fwrite(p,1,n,stdout),putchar('\n');R;}  // no delimiter: intact, or dropped by -s
    else for(S f=p;;f=q+1,q=dm(f,e)){
        if(sel(++k)){if(o)putchar(*dl);fwrite(f,1,q-f,stdout);o=1;}
        if(q==e)break;
    }
    putchar('\n');
}
I main(I ac,S*av){
    I o;S*fs=(S[]){"-",0},b=0;size_t bz=0;ssize_t n;
    setprog(*av);
    W((o=getopt(ac,av,"b:c:d:f:ns"))!=-1)S(o,
        C('d',dl=optarg;if(!*dl||dl[1])xerrx(2,"%s: delimiter must be one character",dl))
        C('n',ns=1)
        C('s',sp=1),
        if(!strchr("bcf",o)||(m&&m!=o))xusage(U);m=o;addl(optarg))
    if(!m||(ns&&m!='b')||((sp||dl)&&m!='f'))xusage(U);   // exactly one of -b -c -f; -n only with -b; -d -s only with -f
    if(!dl)dl="\t";
    if(av[optind])fs=av+optind;
    for(FILE*f;*fs;fs++){
        if(!(f=strcmp(*fs,"-")?fopen(*fs,"r"):stdin)){rc=xwarn("%s",*fs);continue;} // keep going: the spec forbids stopping
        W((n=getline(&b,&bz,f))>0)ln(b,n-(b[n-1]=='\n'));
        if(ferror(f))rc=xwarn("%s",*fs);
        if(f==stdin)clearerr(f);else fclose(f);
    }
    xclose();
    R rc;
}
