// tr [-c|-C] [-s] string1 string2
// tr -s [-c|-C] string1
// tr -d [-c|-C] string1
// tr -ds [-c|-C] string1 string2
#include"a.h"
#define U "[-c|-C] [-s] string1 string2 | -s [-c|-C] string1 | -d [-c|-C] string1 | -ds [-c|-C] string1 string2"
typedef struct{I*v,n,f,x;char*t;}A;         // array of bytes; [x*] at f (-1: none) of x; tags: L l / U u for a [:lower:] / [:upper:] run
A*one;                                      // string1
S cs[]={"alnum","alpha","blank","cntrl","digit","graph","lower","print","punct","space","upper","xdigit",0};
I(*cf[])(I)={isalnum,isalpha,isblank,iscntrl,isdigit,isgraph,islower,isprint,ispunct,isspace,isupper,isxdigit};

V push(A*a,I v,char t){
    if(!(a->n&(a->n-1))){size_t z=a->n?2*a->n:1;a->v=rea(a->v,z*sizeof*a->v);a->t=rea(a->t,z);} // grow at 0 and powers of 2
    a->v[a->n]=v;a->t[a->n++]=t;
}
I oc(S*s){I b=0,k=0;W(k<3&&**s>='0'&&**s<='7'&&b*8+**s-'0'<256)b=b*8+*(*s)++-'0',k++;R b;} // \octal digits, at most a byte
I el(S*s){                                  // one byte, or escape
    S p=*s+1,e="abfnrtv",q;
    if(**s!='\\'||!*p)R(C)*(*s)++;
    if(*p>='0'&&*p<='7'){*s=p;R oc(s);}
    if((q=strchr(e,*p))){*s=p+1;R "\a\b\f\n\r\t\v"[q-e];}
    *s=p+1;R(C)*p;                          // \\, and any other \c is c
}
V ps(A*a,S s,I md){                         // md 0: string1; 1: string2; 2: string2 of -ds
    S q;I v,w;
    a->f=-1;
    W(*s){
        if(*s=='['&&s[1]==':'&&(q=strstr(s+2,":]"))){
            S nm=strndup(s+2,q-s-2);I t=0,up=!strcmp(nm,"upper"),lc=up||!strcmp(nm,"lower"),j=a->n;
            W(cs[t]&&strcmp(cs[t],nm))t++;
            if(!cs[t])xerrx(1,"[:%s:]: invalid class",nm);
            s=q+2;
            if(md==1){                          // only case conversion, against the other case in the same place in string1
                if(!lc||j>=one->n||one->t[j]!=(up?'L':'U'))xerrx(1,"[:%s:]: misplaced class in string2",nm);
                do push(a,up?toupper(one->v[j]):tolower(one->v[j]),0);W(++j<one->n&&one->t[j]==(up?'l':'u'));
            }
            else{char g=lc?(up?'U':'L'):0;N(256,if(cf[t](i))push(a,i,g),g=tolower(g))}
            free(nm);
        }
        else if(*s=='['&&s[1]=='='&&s[2]&&(q=s+2,v=el(&q),*q=='='&&q[1]==']')){ // equivalence class: every character is its own
            if(md==1)xerrx(1,"[=%.*s=]: equivalence class in string2",(I)(q-s-2),s+2);
            push(a,v,0);s=q+2;
        }
        else if(md&&*s=='['&&s[1]&&(q=s+1,v=el(&q),*q=='*')&&q[1+(w=strspn(q+1,"0123456789"))]==']'){
            UJ n=strtoumax(q+1,0,q[1]=='0'?8:10);    // [x*n]; leading 0 is octal; none or 0 fills to the length of string1
            s=q+w+2;
            w=MX(one->n,256);                      // more than string1 (or its complement) is never used
            if(n)N(MN(n,(UJ)w),push(a,v,0))
            else{if(a->f>=0)xerrx(1,"only one [x*] in string2");a->f=a->n;a->x=v;}
        }
        else{
            v=el(&s);
            if(*s!='-'||!s[1]){push(a,v,0);continue;}
            s++;w=el(&s);
            if(w<v)xerrx(1,"range out of order");
            for(I c=v;c<=w;c++)push(a,c,0);
        }
    }
}
char*set(A*a,I c){char*t=memset(mal(256),c,256);N(a->n,t[a->v[i]]=!c)R t;}
I main(I ac,S*av){
    I o,c=0,d=0,s=0,*m=0,n1,l=-1,v;char*in,*sq;A a={0},b={0};
    setprog(*av);
    W((o=getopt(ac,av,"cCds"))!=-1)S(o,C('c',c=1)C('C',c=1)C('d',d=1)C('s',s=1),xusage(U))
    av+=optind;ac-=optind;
    if(ac<1||ac>2||(d&&ac!=1+s)||(!d&&!s&&ac<2))xusage(U);
    ps(one=&a,av[0],0);
    in=set(&a,c);                           // in[v]: v is in the (complemented) string1 array
    n1=a.n;if(c){n1=0;N(256,n1+=in[i])}
    if(ac>1)ps(&b,av[1],d?2:1);
    if(b.f>=0){                             // [x*]: as many x as string1 is longer; at the end, padding does it
        I k=b.f==b.n?1:MX(n1-b.n,0),e=b.n;
        N(k,push(&b,b.x,0))
        memmove(b.v+b.f+k,b.v+b.f,(e-b.f)*sizeof*b.v);
        N(k,b.v[b.f+i]=b.x)
    }
    if(!d&&ac>1){                           // translate: string2 is padded with its last character
        if(n1&&!b.n)xerrx(1,"string2 must not be empty");
        m=mal(256*sizeof*m);N(256,m[i]=i)
        $(c,I k=0;N(256,if(in[i])m[i]=b.v[MN(k,b.n-1)],k++)) // ascending value order
        N(a.n,m[a.v[i]]=b.v[MN(i,b.n-1)])
    }
    sq=ac>1?set(&b,0):in;                   // squeeze the last operand's array
    W((v=getchar_unlocked())!=EOF){
        if(d&&in[v])continue;
        if(m)v=m[v];
        if(s&&sq[v]&&v==l)continue;
        putchar_unlocked(l=v);
    }
    if(ferror(stdin))xerr(1,"stdin");
    xclose();
    R 0;
}
