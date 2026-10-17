// sort [-m] [-o output] [-bdfinru] [-t char] [-k keydef]... [file...]
// sort -c|-C [-bdfinru] [-t char] [-k keydef] [file]
#include"a.h"
#define U "[-cCmu] [-o output] [-bdfinr] [-t char] [-k keydef]... [file...]"
#define DG(c) ((c)>='0'&&(c)<='9')
#define SG(x) (((x)>0)-((x)<0))
enum{Kd=1,Kf=2,Ki=4,Kn=8,Kr=16};                 // order bits, in "dfinr" order
struct l{S s;size_t n;}*ls;size_t nl,mx;         // line: bytes, length; longest
struct k{I f1,c1,f2,c2,o,b1,b2;}*ks;I nk;        // key: start field.char; end field.char (f2 0: end of line, c2 0: end of field); order bits; -b at start, end
I g,gb,u;char tc;S ta,tb;                        // global order bits, global -b, -u; -t char; key buffers

I dn(S*s,I z){S e;long v;if(!isdigit((C)**s))xusage(U);v=strtol(*s,&e,10);*s=e;if(v<z)xusage(U);R MN(v,(long)INT_MAX);}
I md(S*s,I*b){I o=0;S q,M="bdfinr";W(**s&&(q=strchr(M,**s)))q==M?(*b=1):(o|=1<<(q-M-1)),++*s;R o;}
V key(S s){                                      // field_start[type][,field_end[type]]
    struct k k={0};
    k.f1=dn(&s,1);$(*s=='.',s++;k.c1=dn(&s,1))k.c1=1;
    k.o=md(&s,&k.b1);
    if(*s==',')s++,k.f2=dn(&s,1),k.c2=*s=='.'?(s++,dn(&s,0)):0,k.o|=md(&s,&k.b2);
    if(*s)xusage(U);
    ks=rea(ks,++nk*sizeof*ks);ks[nk-1]=k;
}
V rd(S nm){
    FILE*f=strcmp(nm,"-")?fopen(nm,"r"):stdin;S s=0;size_t m=0,cap=0;ssize_t n;
    if(!f)xerr(2,"%s",nm);
    W((n=getline(&s,&m,f))>0){
        if(s[n-1]=='\n')s[--n]=0;                // a final incomplete line gets its newline on output
        if(nl==cap)ls=rea(ls,(cap=nl*2+64)*sizeof*ls);
        ls[nl++]=(struct l){s,n};s=0;m=0;mx=MX(mx,(size_t)n);
    }
    if(ferror(f))xerr(2,"%s",nm);
    if(f!=stdin)fclose(f);
    free(s);
}
S bl(S p,S e){W(p<e&&isblank((C)*p))p++;R p;}
S fld(S p,S e){if(tc){W(p<e&&*p!=tc)p++;R p;}p=bl(p,e);W(p<e&&!isblank((C)*p))p++;R p;} // to end of field at p
S skp(S p,S e,I n){N(n,p=fld(p,e);if(tc&&p<e)p++)R p;}                                  // to start of field n+1
S adv(S p,S e,I n){R p+MN((ptrdiff_t)n,e-p);}
V rng(struct k*k,struct l*a,S*p,S*q){           // key k of line a is [*p,*q)
    S e=a->s+a->n,s=skp(a->s,e,k->f1-1),t=e;
    if(k->b1)s=bl(s,e);
    s=adv(s,e,k->c1-1);
    if(k->f2)t=skp(a->s,e,k->f2-1),t=k->c2?adv(k->b2?bl(t,e):t,e,k->c2):fld(t,e);
    *p=s;*q=MX(s,t);
}
I bc(S a,size_t m,S b,size_t n){I c=memcmp(a,b,MN(m,n));R c?SG(c):(m>n)-(m<n);} // bytes in order: the POSIX locale's collation
size_t xf(I o,S p,S e,S t){                     // key with -d -i dropped, -f folded, to t; its length
    S b=t;W(p<e){C c=*p++;if(!(o&Kd)||isblank(c)||isalnum(c))if(!(o&Ki)||isprint(c))*t++=o&Kf?toupper(c):c;}
    R t-b;
}
I nm(S p,S e,S t,I*ni,I*nf){                    // numeric string at p: int digits (no leading 0s) then fraction digits (no trailing 0s) to t; sign, 0 if zero
    I s=1,n=0;p=bl(p,e);
    if(p<e&&*p=='-')s=-1,p++;
    W(p<e&&*p=='0')p++;
    W(p<e&&DG(*p))t[n++]=*p++;
    *ni=n;
    if(p<e&&*p=='.')for(p++;p<e&&DG(*p);)t[n++]=*p++;  // the POSIX locale: radix '.', no thousands separator
    W(n>*ni&&t[n-1]=='0')n--;
    *nf=n-*ni;R n?s:0;
}
I nc(S p,S q,S r,S s){
    I i,j,f,h,a=nm(p,q,ta,&i,&f),b=nm(r,s,tb,&j,&h),c;
    P(a!=b,a<b?-1:1)
    c=i!=j?i-j:memcmp(ta,tb,i);
    if(!c)c=memcmp(ta+i,tb+j,MN(f,h));
    if(!c)c=f-h;                                 // no trailing 0s: the longer fraction is bigger
    R a*SG(c);
}
I kc(struct k*k,struct l*a,struct l*b){
    S p,q,r,s;I c;rng(k,a,&p,&q);rng(k,b,&r,&s);
    c=k->o&Kn?nc(p,q,r,s):bc(ta,xf(k->o,p,q,ta),tb,xf(k->o,r,s,tb));
    R k->o&Kr?-c:c;
}
I cmp(const V*x,const V*y){                     // keys in order, then (not -u) all bytes, -r still in effect
    struct l*a=(V*)x,*b=(V*)y;I c;
    N(nk,P((c=kc(ks+i,a,b)),c))
    P(u,0)
    c=bc(a->s,a->n,b->s,b->n);R g&Kr?-c:c;
}
I main(I ac,S*av){
    I o,c=0;S on=0,fn;FILE*of=stdout;
    setprog(*av);
    W((o=getopt(ac,av,"cCmo:bdfinrut:k:"))!=-1)S(o,
        C('c',c=1)C('C',c=2)C('m',)C('o',on=optarg)C('u',u=1)C('b',gb=1)
        C('d',g|=Kd)C('f',g|=Kf)C('i',g|=Ki)C('n',g|=Kn)C('r',g|=Kr)
        C('t',if(!*optarg||optarg[1])xusage(U);tc=*optarg)
        C('k',key(optarg)),
        xusage(U))
    av+=optind;fn=*av?*av:"-";
    if(c&&*av&&av[1])xusage(U);
    if(!nk)key("1");                             // no -k: the whole line
    N(nk,if(!ks[i].o)ks[i].o=g;ks[i].b1|=gb;ks[i].b2|=gb)
    if(!*av)rd("-");
    W(*av)rd(*av++);                             // all input is read before -o opens: output may be an input
    ta=mal(mx);tb=mal(mx);
    if(c){
        for(size_t j=1;j<nl;j++){I d=cmp(ls+j-1,ls+j);P(d>0||u&&!d,c<2?xerrx(1,"%s:%zu: %s: %s",fn,j+1,d?"disorder":"duplicate key",ls[j].s):1)}
        R 0;
    }
    qsort(ls,nl,sizeof*ls,cmp);                  // -m: sorting merged input is a merge
    if(on&&!(of=fopen(on,"w")))xerr(2,"%s",on);
    for(size_t j=0;j<nl;j++)if(!u||!j||cmp(ls+j-1,ls+j))fwrite(ls[j].s,1,ls[j].n,of),putc('\n',of);
    P(fflush(of)||ferror(of)||of!=stdout&&fclose(of),xerr(2,"%s",on?on:"stdout"))
    R 0;
}
