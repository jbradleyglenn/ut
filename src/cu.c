// cut -b list [-n] [file...]
// cut -c list [file...]
// cut -f list [-d delim] [-s] [file...]
#include"a.h"
#define U "-b list [-n] [file...] | -c list [file...] | -f list [-d delim] [-s] [file...]"
struct r{UJ lo,hi;}*r;I nr,ns,sp,rc;char m; // list ranges (hi UINTMAX_MAX for low-); mode b c f; -n; -s
S dl;size_t dn;                             // delimiter, one character of dn bytes

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
I cn(S p,size_t n){mbstate_t st={0};size_t z=mbrtowc(0,p,n,&st);R z-1<n?z:1;} // bytes in the character at p; nul or invalid byte is 1
S dm(S q,S e){W(q<e&&((size_t)(e-q)<dn||memcmp(q,dl,dn)))q+=cn(q,e-q);R q;}  // next delimiter, on a character boundary
V ln(S p,size_t n){
    S e=p+n,q;UJ k=0;I o=0,z;
    if(m!='f')for(q=p;q<e;q+=z){             // -n: a character is written iff its last byte is selected
        z=m=='b'&&!ns?1:cn(q,e-q);
        if(sel(k+=m=='c'?1:z))fwrite(q,1,z,stdout);
    }
    else if((q=dm(p,e))==e){if(!sp)fwrite(p,1,n,stdout),putchar('\n');R;}  // no delimiter: intact, or dropped by -s
    else for(S f=p;;f=q+dn,q=dm(f,e)){
        if(sel(++k)){if(o)fwrite(dl,1,dn,stdout);fwrite(f,1,q-f,stdout);o=1;}
        if(q==e)break;
    }
    putchar('\n');
}
I main(I ac,S*av){
    I o;S*fs=(S[]){"-",0},b=0;size_t bz=0;ssize_t n;
    setlocale(LC_ALL,"");
    setprog(*av);
    W((o=getopt(ac,av,"b:c:d:f:ns"))!=-1)S(o,
        C('d',dl=optarg;dn=strlen(dl);if(!dn||cn(dl,dn)!=(I)dn)xerrx(2,"%s: delimiter must be one character",dl))
        C('n',ns=1)
        C('s',sp=1),
        if(!strchr("bcf",o)||(m&&m!=o))xusage(U);m=o;addl(optarg))
    if(!m||(ns&&m!='b')||((sp||dl)&&m!='f'))xusage(U);   // exactly one of -b -c -f; -n only with -b; -d -s only with -f
    if(!dl)dl="\t",dn=1;
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
