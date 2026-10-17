// sort [-m] [-o output] [-bdfinru] [-t char] [-k keydef]... [file...]
// sort -c|-C [-bdfinru] [-t char] [-k keydef] [file]
#include"a.h"
#define U "[-cCmu] [-o output] [-bdfinr] [-t char] [-k keydef]... [file...]"
#define DG(c) ((c)>='0'&&(c)<='9')
#define SG(x) (((x)>0)-((x)<0))
typedef wchar_t*WS;
enum{Kd=1,Kf=2,Ki=4,Kn=8,Kr=16};                 // order bits, in "dfinr" order
struct l{S s;WS w;size_t n;}*ls;size_t nl;       // line: bytes, wide chars, wide length
struct k{I f1,c1,f2,c2,o,b1,b2;}*ks;I nk;        // key: start field.char; end field.char (f2 0: end of line, c2 0: end of field); order bits; -b at start, end
I g,gb,u;wchar_t tc,ts,dp;WS ta,tb;              // global order bits, global -b, -u; -t char, thousands sep, radix; key buffers

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
        ls[nl++]=(struct l){s,0,n};s=0;m=0;
    }
    if(ferror(f))xerr(2,"%s",nm);
    if(f!=stdin)fclose(f);
    free(s);
}
WS bl(WS p,WS e){W(p<e&&iswblank(*p))p++;R p;}
WS fld(WS p,WS e){if(tc){W(p<e&&*p!=tc)p++;R p;}p=bl(p,e);W(p<e&&!iswblank(*p))p++;R p;} // to end of field at p
WS skp(WS p,WS e,I n){N(n,p=fld(p,e);if(tc&&p<e)p++)R p;}                               // to start of field n+1
WS adv(WS p,WS e,I n){R p+MN((ptrdiff_t)n,e-p);}
V rng(struct k*k,struct l*a,WS*p,WS*q){         // key k of line a is [*p,*q)
    WS e=a->w+a->n,s=skp(a->w,e,k->f1-1),t=e;
    if(k->b1)s=bl(s,e);
    s=adv(s,e,k->c1-1);
    if(k->f2)t=skp(a->w,e,k->f2-1),t=k->c2?adv(k->b2?bl(t,e):t,e,k->c2):fld(t,e);
    *p=s;*q=MX(s,t);
}
V xf(I o,WS p,WS e,WS t){                       // key with -d -i dropped, -f folded
    W(p<e){wchar_t c=*p++;if(!(o&Kd)||iswblank(c)||iswalnum(c))if(!(o&Ki)||iswprint(c))*t++=o&Kf?towupper(c):c;}
    *t=0;
}
I nm(WS p,WS e,WS t,I*ni,I*nf){                 // numeric string at p: int digits (no leading 0s) then fraction digits (no trailing 0s) to t; sign, 0 if zero
    I s=1,n=0;p=bl(p,e);
    if(p<e&&*p=='-')s=-1,p++;
    W(p<e&&(*p=='0'||ts&&*p==ts))p++;
    W(p<e&&(DG(*p)||ts&&*p==ts))if(*p++!=ts)t[n++]=p[-1];
    *ni=n;
    if(p<e&&*p==dp)for(p++;p<e&&DG(*p);)t[n++]=*p++;
    W(n>*ni&&t[n-1]=='0')n--;
    *nf=n-*ni;R n?s:0;
}
I nc(WS p,WS q,WS r,WS s){
    I i,j,f,h,a=nm(p,q,ta,&i,&f),b=nm(r,s,tb,&j,&h),c;
    P(a!=b,a<b?-1:1)
    c=i!=j?i-j:wmemcmp(ta,tb,i);
    if(!c)c=wmemcmp(ta+i,tb+j,MN(f,h));
    if(!c)c=f-h;                                 // no trailing 0s: the longer fraction is bigger
    R a*SG(c);
}
I kc(struct k*k,struct l*a,struct l*b){
    WS p,q,r,s;I c;rng(k,a,&p,&q);rng(k,b,&r,&s);
    c=k->o&Kn?nc(p,q,r,s):(xf(k->o,p,q,ta),xf(k->o,r,s,tb),SG(wcscoll(ta,tb)));
    R k->o&Kr?-c:c;
}
I cmp(const V*x,const V*y){                     // keys in order, then (not -u) all bytes, -r still in effect
    struct l*a=(V*)x,*b=(V*)y;I c;
    N(nk,P((c=kc(ks+i,a,b)),c))
    P(u,0)
    c=SG(strcmp(a->s,b->s));R g&Kr?-c:c;
}
I main(I ac,S*av){
    I o,c=0;S on=0,fn;size_t mx=0;FILE*of=stdout;struct lconv*lc;
    setlocale(LC_ALL,"");
    setprog(*av);
    lc=localeconv();
    if(mbtowc(&dp,lc->decimal_point,MB_LEN_MAX)<=0)dp='.';
    if(mbtowc(&ts,lc->thousands_sep,MB_LEN_MAX)<=0)ts=0;
    W((o=getopt(ac,av,"cCmo:bdfinrut:k:"))!=-1)S(o,
        C('c',c=1)C('C',c=2)C('m',)C('o',on=optarg)C('u',u=1)C('b',gb=1)
        C('d',g|=Kd)C('f',g|=Kf)C('i',g|=Ki)C('n',g|=Kn)C('r',g|=Kr)
        C('t',if(mbtowc(&tc,optarg,MB_LEN_MAX)!=(I)strlen(optarg)||!tc)xusage(U))
        C('k',key(optarg)),
        xusage(U))
    av+=optind;fn=*av?*av:"-";
    if(c&&*av&&av[1])xusage(U);
    if(!nk)key("1");                             // no -k: the whole line
    N(nk,if(!ks[i].o)ks[i].o=g;ks[i].b1|=gb;ks[i].b2|=gb)
    if(!*av)rd("-");
    W(*av)rd(*av++);                             // all input is read before -o opens: output may be an input
    for(size_t j=0;j<nl;j++){
        struct l*a=ls+j;size_t r;a->w=mal((a->n+1)*sizeof*a->w);
        $((r=mbstowcs(a->w,a->s,a->n+1))!=(size_t)-1,a->n=r)N(a->n,a->w[i]=(C)a->s[i])a->w[a->n]=0; // invalid: bytewise
        mx=MX(mx,a->n);
    }
    ta=mal((mx+1)*sizeof*ta);tb=mal((mx+1)*sizeof*tb);
    if(c){
        for(size_t j=1;j<nl;j++){I d=cmp(ls+j-1,ls+j);P(d>0||u&&!d,c<2?xerrx(1,"%s:%zu: %s: %s",fn,j+1,d?"disorder":"duplicate key",ls[j].s):1)}
        R 0;
    }
    qsort(ls,nl,sizeof*ls,cmp);                  // -m: sorting merged input is a merge
    if(on&&!(of=fopen(on,"w")))xerr(2,"%s",on);
    for(size_t j=0;j<nl;j++)if(!u||!j||cmp(ls+j-1,ls+j))fputs(ls[j].s,of),putc('\n',of);
    P(fflush(of)||ferror(of)||of!=stdout&&fclose(of),xerr(2,"%s",on?on:"stdout"))
    R 0;
}
