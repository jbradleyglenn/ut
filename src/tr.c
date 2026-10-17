// tr [-c|-C] [-s] string1 string2
// tr -s [-c|-C] string1
// tr -d [-c|-C] string1
// tr -ds [-c|-C] string1 string2
#include"a.h"
#define U "[-c|-C] [-s] string1 string2 | -s [-c|-C] string1 | -d [-c|-C] string1 | -ds [-c|-C] string1 string2"
enum{RB=0x110000,NV=RB+256};                // values: a wide character, or RB+b for a byte b that starts no character
typedef struct{I*v,n,f,x;char*t;}A;         // array of values; [x*] at f (-1: none) of x; tags: L l / U u for a [:lower:] / [:upper:] run
A*one;I lim;                                // string1; characters are below lim

V push(A*a,I v,char t){
    if(!(a->n&(a->n-1))){size_t z=a->n?2*a->n:1;a->v=rea(a->v,z*sizeof*a->v);a->t=rea(a->t,z);} // grow at 0 and powers of 2
    a->v[a->n]=v;a->t[a->n++]=t;
}
I bv(C b){wchar_t w;size_t z=mbrtowc(&w,(S)&b,1,&(mbstate_t){0});R z<=1&&w<RB?w:RB+b;}
I gc(S*s){wchar_t w;size_t z=mbrtowc(&w,*s,MB_LEN_MAX,&(mbstate_t){0});if(z-1<MB_LEN_MAX&&w<RB){*s+=z;R w;}R bv(*(*s)++);}
I oc(S*s){I b=0,k=0;W(k<3&&**s>='0'&&**s<='7'&&b*8+**s-'0'<256)b=b*8+*(*s)++-'0',k++;R b;} // \octal digits, at most a byte
I el(S*s){                                  // one character, or escape
    S p=*s+1,e="abfnrtv",q;
    if(**s!='\\'||!*p)R gc(s);
    if(*p>='0'&&*p<='7'){                   // concatenated \octal bytes can spell one multibyte character
        char b[MB_LEN_MAX];I n=0;wchar_t w;size_t z;
        b[n++]=oc(&p);*s=q=p;
        W((z=mbrtowc(&w,b,n,&(mbstate_t){0}))==(size_t)-2&&n<MB_LEN_MAX&&*q=='\\'&&q[1]>='0'&&q[1]<='7')q++,b[n++]=oc(&q);
        if(n>1&&z==(size_t)n&&w<RB){*s=q;R w;}
        R bv(*b);
    }
    if((q=strchr(e,*p))){*s=p+1;R "\a\b\f\n\r\t\v"[q-e];}
    *s=p;R gc(s);                           // \\, and any other \c is c
}
V ps(A*a,S s,I md){                         // md 0: string1; 1: string2; 2: string2 of -ds
    S q;I v,w;
    a->f=-1;
    W(*s){
        if(*s=='['&&s[1]==':'&&(q=strstr(s+2,":]"))){
            S nm=strndup(s+2,q-s-2);wctype_t t=wctype(nm);I up=!strcmp(nm,"upper"),cs=up||!strcmp(nm,"lower"),j=a->n;
            if(!t)xerrx(1,"[:%s:]: invalid class",nm);
            s=q+2;
            if(md==1){                          // only case conversion, against the other case in the same place in string1
                if(!cs||j>=one->n||one->t[j]!=(up?'L':'U'))xerrx(1,"[:%s:]: misplaced class in string2",nm);
                do push(a,up?towupper(one->v[j]):towlower(one->v[j]),0);W(++j<one->n&&one->t[j]==(up?'l':'u'));
            }
            else{char g=cs?(up?'U':'L'):0;for(I c=0;c<lim;c++)if(iswctype(c,t))push(a,c,g),g=tolower(g);}
            free(nm);
        }
        else if(*s=='['&&s[1]=='='&&s[2]&&(q=s+2,v=el(&q),*q=='='&&q[1]==']')){ // equivalence class: every character is its own
            if(md==1)xerrx(1,"[=%.*s=]: equivalence class in string2",(I)(q-s-2),s+2);
            push(a,v,0);s=q+2;
        }
        else if(md&&*s=='['&&s[1]&&(q=s+1,v=el(&q),*q=='*')&&q[1+(w=strspn(q+1,"0123456789"))]==']'){
            UJ n=strtoumax(q+1,0,q[1]=='0'?8:10);    // [x*n]; leading 0 is octal; none or 0 fills to the length of string1
            s=q+w+2;
            if(n)N(MN(n,(UJ)NV),push(a,v,0))
            else{if(a->f>=0)xerrx(1,"only one [x*] in string2");a->f=a->n;a->x=v;}
        }
        else{
            v=el(&s);
            if(*s!='-'||!s[1]){push(a,v,0);continue;}
            s++;w=el(&s);
            if(w<v)xerrx(1,"range out of order");
            for(I c=v;c<=w;c=c+1==lim?RB:c+1)push(a,c,0);
        }
    }
}
I rd(V){                                    // next input value, -1 at end
    static char b[MB_LEN_MAX];static I n;I c;wchar_t w;size_t z;
    if(!n&&(c=getchar_unlocked())!=EOF)b[n++]=c;
    if(!n)R -1;
    W((z=mbrtowc(&w,b,n,&(mbstate_t){0}))==(size_t)-2&&n<(I)MB_CUR_MAX&&(c=getchar_unlocked())!=EOF)b[n++]=c;
    $(z<=(size_t)n&&w<RB,c=w;z+=!z)c=RB+(C)*b,z=1;  // nul is one byte; an invalid byte is itself
    memmove(b,b+z,n-=z);
    R c;
}
V pt(I v){char b[MB_LEN_MAX];size_t z;$(v>=RB,putchar_unlocked(v-RB))if((z=wcrtomb(b,v,&(mbstate_t){0}))!=(size_t)-1)fwrite(b,1,z,stdout);}
char*set(A*a,I c){char*t=memset(mal(NV),c,NV);N(a->n,t[a->v[i]]=!c)R t;}
I main(I ac,S*av){
    I o,c=0,d=0,s=0,*m=0,n1,l=-1,v;char*in,*sq;A a={0},b={0};
    setlocale(LC_ALL,"");
    setprog(*av);
    W((o=getopt(ac,av,"cCds"))!=-1)S(o,C('c',c=1)C('C',c=1)C('d',d=1)C('s',s=1),xusage(U))
    av+=optind;ac-=optind;
    if(ac<1||ac>2||(d&&ac!=1+s)||(!d&&!s&&ac<2))xusage(U);
    lim=MB_CUR_MAX>1?RB:256;
    ps(one=&a,av[0],0);
    in=set(&a,c);                           // in[v]: v is in the (complemented) string1 array
    n1=a.n;if(c){n1=0;for(v=0;v<NV;v=v+1==lim?RB:v+1)n1+=in[v];}
    if(ac>1)ps(&b,av[1],d?2:1);
    if(b.f>=0){                             // [x*]: as many x as string1 is longer; at the end, padding does it
        I k=b.f==b.n?1:MX(n1-b.n,0),e=b.n;
        N(k,push(&b,b.x,0))
        memmove(b.v+b.f+k,b.v+b.f,(e-b.f)*sizeof*b.v);
        N(k,b.v[b.f+i]=b.x)
    }
    if(!d&&ac>1){                           // translate: string2 is padded with its last character
        if(n1&&!b.n)xerrx(1,"string2 must not be empty");
        m=mal(NV*sizeof*m);N(NV,m[i]=i)
        $(c,I k=0;for(v=0;v<NV;v=v+1==lim?RB:v+1)if(in[v])m[v]=b.v[MN(k,b.n-1)],k++) // ascending value order
        N(a.n,m[a.v[i]]=b.v[MN(i,b.n-1)])
    }
    sq=ac>1?set(&b,0):in;                   // squeeze the last operand's array
    W((v=rd())>=0){
        if(d&&in[v])continue;
        if(m)v=m[v];
        if(s&&sq[v]&&v==l)continue;
        pt(l=v);
    }
    if(ferror(stdin))xerr(1,"stdin");
    xclose();
    R 0;
}
