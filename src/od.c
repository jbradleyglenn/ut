// od [-v] [-A address_base] [-j skip] [-N count] [-t type_string]... [file...]
#include"a.h"
#define B 16 // block size: every type size is a power of two <= 16, so 16 is a multiple of their lcm
#define U "[-v] [-A address_base] [-j skip] [-N count] [-t type_string]... [file...]"
#define LD(T) ({T x;memcpy(&x,p,sizeof x);x;})
struct t{char k;int z,d,w;}*ts; int nt;   // type letter, size in bytes, natural width, field width
unsigned char buf[B+MB_LEN_MAX],prv[B];  // block + lookahead for type c; previous block
int cl[B],pcl[B],cont,rc;                // per byte, for type c: -1 "**", 0 not printable, n printable char of n bytes
size_t have; uintmax_t left=UINTMAX_MAX; FILE*f; char**av,*nm="stdin";
char*nam[]={"nul","soh","stx","etx","eot","enq","ack","bel","bs","ht","nl","vt","ff","cr","so","si",
            "dle","dc1","dc2","dc3","dc4","nak","syn","etb","can","em","sub","esc","fs","gs","rs","us","sp"};

uintmax_t num(char*s){
    char*e,*q,*sx="bkm"; uintmax_t v;
    if(!isdigit((unsigned char)*s))xusage(U);
    v=strtoumax(s,&e,0);                 // 0x hex, 0 octal, else decimal; a hex trailing b is a digit
    if(*e&&(q=strchr(sx,*e)))v<<=(int[]){9,10,20}[q-sx],e++;
    if(*e)xusage(U);
    return v;
}
void addt(char*s){
    while(*s){
        char k=*s++,*sl=k=='f'?"FDL":"CSIL",*q; int z;
        int sz[2][4]={{sizeof(float),sizeof(double),sizeof(long double)},{1,sizeof(short),sizeof(int),sizeof(long)}};
        if(!strchr("acdfoux",k))xusage(U);
        if(k=='a'||k=='c')z=1;
        else if(*s&&(q=strchr(sl,*s)))z=sz[k!='f'][q-sl],s++;
        else if(isdigit((unsigned char)*s))z=strtol(s,&s,10);
        else z=k=='f'?sizeof(double):sizeof(int);
        if(k=='f'?z!=sizeof(float)&&z!=sizeof(double)&&z!=sizeof(long double):z<1||z>8||z&(z-1))xerrx(2,"%c%d: unsupported size",k,z);
        uintmax_t mx=z<=8?UINTMAX_MAX>>8*(sizeof(uintmax_t)-z):0;
        int d=k=='o'?(8*z+2)/3:k=='x'?2*z:k=='u'?snprintf(0,0,"%ju",mx):k=='d'?snprintf(0,0,"%jd",-(intmax_t)(mx>>1)-1)
             :k=='f'?(z==sizeof(float)?FLT_DIG:z==sizeof(double)?DBL_DIG:LDBL_DIG)+8:3;
        ts=rea(ts,++nt*sizeof*ts); ts[nt-1]=(struct t){k,z,d,0};
    }
}
FILE*nxt(void){for(FILE*g;*av;)if((g=fopen(nm=*av++,"r")))return g;else xwarn("%s",nm),rc=1;return NULL;}
int get(void){
    for(int c;f||(f=nxt());f=NULL){
        if((c=getc(f))!=EOF)return c;
        if(ferror(f))xwarn("%s",nm),rc=1;
        if(f!=stdin)fclose(f);
    }
    return EOF;
}
void fill(void){
    for(int c;have<sizeof buf&&left&&(c=get())!=EOF;left--)buf[have++]=c;
    memset(buf+have,0,sizeof buf-have);  // pad a short last block with nulls
}
int clen(unsigned char*p,size_t m){       // length of the printable character at p, or 0
    mbstate_t st={0}; wchar_t wc; size_t r=mbrtowc(&wc,(char*)p,m,&st);
    return r>0&&r<=m&&iswprint(wc)?r:0;
}
void line(struct t*t,size_t n){
    for(size_t i=0;i<n;i+=t->z){
        unsigned char*p=buf+i,c=*p&127; int z=t->z,d=t->d; char o[8],*q,*E="\a\b\f\n\r\t\v";
        printf("%*s",t->w-d,"");
        switch(t->k){
        case 'o':printf("%0*jo",d,(uintmax_t)(z==1?LD(uint8_t):z==2?LD(uint16_t):z==4?LD(uint32_t):LD(uint64_t)));break;
        case 'x':printf("%0*jx",d,(uintmax_t)(z==1?LD(uint8_t):z==2?LD(uint16_t):z==4?LD(uint32_t):LD(uint64_t)));break;
        case 'u':printf("%*ju",d,(uintmax_t)(z==1?LD(uint8_t):z==2?LD(uint16_t):z==4?LD(uint32_t):LD(uint64_t)));break;
        case 'd':printf("%*jd",d,(intmax_t)(z==1?LD(int8_t):z==2?LD(int16_t):z==4?LD(int32_t):LD(int64_t)));break;
        case 'f':printf("%*.*Le",d,d-9,z==sizeof(float)?LD(float):z==sizeof(double)?LD(double):LD(long double));break;
        case 'a':printf("%*s",d,c==127?"del":c<=32?nam[c]:(char[]){c,0});break;
        case 'c':
            if(cl[i]>0){printf("%*s",d-1,"");fwrite(p,1,cl[i],stdout);break;}
            if(cl[i]<0)q="**";
            else if(!*p)q="\\0";
            else if((q=strchr(E,*p)))sprintf(o,"\\%c","abfnrtv"[q-E]),q=o;
            else sprintf(q=o,"%03o",*p);
            printf("%*s",d,q);
        }
    }
    putchar('\n');
}
int main(int ac,char**argv){
    int o,v=0,star=0,L=0; char a='o',af[]="%07jo"; uintmax_t skip=0,off=0; size_t n;
    setlocale(LC_ALL,"");
    setprog(argv[0]);
    while((o=getopt(ac,argv,"vA:j:N:t:"))!=-1)switch(o){
        case 'v':v=1;break;
        case 'A':if(!*optarg||optarg[1]||!strchr("dnox",*optarg))xusage(U);a=*optarg;break;
        case 'j':skip=num(optarg);break;
        case 'N':left=num(optarg);break;
        case 't':addt(optarg);break;
        default:xusage(U);
    }
    if(!nt)addt("oS");
    av=argv+optind; if(!*av)f=stdin;
    for(;skip;skip--)if(get()==EOF)xerrx(1,"cannot skip past end of input");
    for(int j=0;j<nt;j++)if((ts[j].d+1)*(B/ts[j].z)>L)L=(ts[j].d+1)*(B/ts[j].z);
    for(int j=0;j<nt;j++)ts[j].w=L*ts[j].z/B;  // equal-width lines, so columns line up across types
    af[4]=a=='d'?'u':a;
    for(;fill(),(n=have<B?have:B);off+=n,memmove(buf,buf+n,have-=n)){
        for(size_t i=0;i<n;i++)cl[i]=cont?(cont--,-1):(cont=(o=clen(buf+i,have-i))?o-1:0,o);
        if(!v&&off&&n==B&&!memcmp(buf,prv,B)&&!memcmp(cl,pcl,sizeof cl)){if(!star)puts("*");star=1;continue;}
        star=0; memcpy(prv,buf,B); memcpy(pcl,cl,sizeof cl);
        for(int j=0;j<nt;j++){if(a!='n')j?printf("%7s",""):printf(af,off);line(ts+j,n);}
    }
    if(a!='n')printf(af,off),putchar('\n');
    xclose();
    return rc;
}
