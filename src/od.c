// od [-v] [-A address_base] [-j skip] [-N count] [-t type_string]... [file...]
#include"a.h"
#define B 16 // block size: every type size is a power of two <= 16, so 16 is a multiple of their lcm
#define U "[-v] [-A address_base] [-j skip] [-N count] [-t type_string]... [file...]"
#define LD(T) ({T x;memcpy(&x,p,sizeof x);x;})
#define LU (UJ)(z==1?LD(uint8_t):z==2?LD(uint16_t):z==4?LD(uint32_t):LD(uint64_t))
#define LI (J)(z==1?LD(int8_t):z==2?LD(int16_t):z==4?LD(int32_t):LD(int64_t))
#define LF (z==sizeof(float)?LD(float):z==sizeof(double)?LD(double):LD(long double))
struct t{char k;I z,d,w;}*ts;I nt;       // type letter, size in bytes, natural width, field width
C buf[B],prv[B];I rc;                    // block; previous block
UJ left=UINTMAX_MAX;FILE*f;S*av,nm="stdin";
S nam[]={"nul","soh","stx","etx","eot","enq","ack","bel","bs","ht","nl","vt","ff","cr","so","si",
         "dle","dc1","dc2","dc3","dc4","nak","syn","etb","can","em","sub","esc","fs","gs","rs","us","sp"};

UJ num(S s){
    S e,q,sx="bkm";UJ v;
    P(!isdigit((C)*s),xusage(U))
    v=strtoumax(s,&e,0);                 // 0x hex, 0 octal, else decimal; a hex trailing b is a digit
    if(*e&&(q=strchr(sx,*e)))v<<=(I[]){9,10,20}[q-sx],e++;
    P(*e,xusage(U))
    R v;
}
V addt(S s){
    W(*s){
        char k=*s++;S sl=k=='f'?"FDL":"CSIL",q;
        I sz[2][4]={{sizeof(float),sizeof(double),sizeof(long double)},{1,sizeof(short),sizeof(int),sizeof(long)}};
        if(!strchr("acdfoux",k))xusage(U);
        I z=k=='a'||k=='c'?1:*s&&(q=strchr(sl,*s))?s++,sz[k!='f'][q-sl]:isdigit((C)*s)?strtol(s,&s,10):k=='f'?sizeof(double):sizeof(int);
        if(k=='f'?z!=sizeof(float)&&z!=sizeof(double)&&z!=sizeof(long double):z<1||z>8||z&(z-1))xerrx(2,"%c%d: unsupported size",k,z);
        UJ mx=z<=8?UINTMAX_MAX>>8*(sizeof(UJ)-z):0;
        I d=k=='o'?(8*z+2)/3:k=='x'?2*z:k=='u'?snprintf(0,0,"%ju",mx):k=='d'?snprintf(0,0,"%jd",-(J)(mx>>1)-1)
           :k=='f'?(z==sizeof(float)?FLT_DIG:z==sizeof(double)?DBL_DIG:LDBL_DIG)+8:3;
        ts=rea(ts,++nt*sizeof*ts);ts[nt-1]=(struct t){k,z,d,0};
    }
}
FILE*nxt(V){FILE*g;W(*av){P((g=fopen(nm=*av++,"r")),g)rc=xwarn("%s",nm);}R 0;}
I get(V){
    for(I c;f||(f=nxt());f=0){
        P((c=getc(f))!=EOF,c)
        if(ferror(f))rc=xwarn("%s",nm);
        if(f!=stdin)fclose(f);
    }
    R EOF;
}
size_t fill(V){                          // next block; its length
    I c;size_t n=0;W(n<B&&left&&(c=get())!=EOF)buf[n++]=c,left--;
    memset(buf+n,0,B-n);R n;             // pad a short last block with nulls
}
V line(struct t*t,size_t n){
    for(size_t i=0;i<n;i+=t->z){
        C*p=buf+i,c=*p&127;I z=t->z,d=t->d;S q,E="\a\b\f\n\r\t\v";
        O("%*s",t->w-d,"");
        S(t->k,
          C('o',O("%0*jo",d,LU))
          C('x',O("%0*jx",d,LU))
          C('u',O("%*ju",d,LU))
          C('d',O("%*jd",d,LI))
          C('f',O("%*.*Le",d,d-9,LF))
          C('a',O("%*s",d,c==127?"del":c<=32?nam[c]:(char[]){c,0}))
          C('c',$(isprint(*p),O("%*c",d,*p))
                $(!*p,O("%*s",d,"\\0"))
                $((q=strchr(E,*p)),O("%*c%c",d-1,'\\',"abfnrtv"[q-E]))
                O("%*.3o",d,*p)),)
    }
    putchar('\n');
}
I main(I ac,S*argv){
    I o,v=0,star=0,L=0;char a='o',af[]="%07jo";UJ skip=0,off=0;size_t n;
    setprog(*argv);
    W((o=getopt(ac,argv,"vA:j:N:t:"))!=-1)S(o,
        C('v',v=1)
        C('A',if(!*optarg||optarg[1]||!strchr("dnox",*optarg))xusage(U);a=*optarg)
        C('j',skip=num(optarg))
        C('N',left=num(optarg))
        C('t',addt(optarg)),
        xusage(U))
    if(!nt)addt("oS");
    av=argv+optind;if(!*av)f=stdin;
    W(skip--)P(get()==EOF,xerrx(1,"cannot skip past end of input"))
    N(nt,L=MX(L,(ts[i].d+1)*(B/ts[i].z)))
    N(nt,ts[i].w=L*ts[i].z/B)             // equal-width lines, so columns line up across types
    af[4]=a=='d'?'u':a;
    for(;(n=fill());off+=n){
        if(!v&&off&&n==B&&!memcmp(buf,prv,B)){if(!star)puts("*");star=1;continue;}
        star=0;memcpy(prv,buf,B);
        N(nt,if(a!='n')i?O("%7s",""):O(af,off);line(ts+i,n))
    }
    if(a!='n')O(af,off),putchar('\n');
    xclose();
    R rc;
}
