// printf format [argument...]
#include"a.h"
S*ar;I na,bs,k,hi,rc;                    // arguments, how many; first argument of this pass, unnumbered count, highest used; exit status
V done(V){xclose();exit(rc);}
I esc(S*p,I x){                          // *p just past a backslash: the byte, or -1 for %b \c; x: %b rules
    S s=*p,q,E="\\abfnrtv";I c=0,n=0;
    P(x&&*s=='c',-1)
    P(*s&&(q=strchr(E,*s)),*p=s+1,"\\\a\b\f\n\r\t\v"[q-E])
    s+=x&&*s=='0';                       // %b: \0ddd
    W(n<3&&*s>='0'&&*s<='7')c=c*8+*s++-'0',n++;
    P(s==*p,'\\')                        // anything else: the backslash is literal
    *p=s;R c&255;
}
V chk(S a,S e){
    $(e==a,rc=xwarnx("%s: expected numeric value",a))
    $(*e,rc=xwarnx("%s: not completely converted",a))
    if(errno)rc=xwarn("%s",a);
}
J ival(S a,I u){                         // missing is 0; 'c is the character's value (rest ignored, as in the spec's "+3); else a C integer constant
    S e;J v;wchar_t w;mbstate_t st={0};
    P(!a,0)
    P(*a=='\''||*a=='"',mbrtowc(&w,a+1,MB_LEN_MAX,&st)>MB_LEN_MAX?(C)a[1]:w)
    errno=0;v=u?(J)strtoumax(a,&e,0):strtoimax(a,&e,0);
    chk(a,e);R v;
}
long double fval(S a){S e;long double v;P(!a,0)errno=0;v=strtold(a,&e);chk(a,e);R v;}
V cv(S*pp){                              // *pp at a %: write one conversion, step past it
    S p=*pp+1,f,e,a,s,t;long n=strtol(p,&e,10);I w=0,pr=-1,c,x=0;size_t m,d;
    $(isdigit((C)*p)&&*e=='$'&&n>0,p=e+1;if(n>na-bs)xerrx(1,"%%%ld$: missing argument",n))n=++k;
    hi=MX(hi,(I)n);a=bs+n<=na?ar[bs+n-1]:0;
    f=p;p+=strspn(p,"-+ #0");
    if(isdigit((C)*p))w=strtol(p,&p,10);
    if(*p=='.')pr=isdigit((C)*++p)?strtol(p,&p,10):0;
    if(!(c=*p)||!strchr("diouxXcsbaAeEfFgG",c))xerrx(1,"%s: invalid conversion",*pp);
    *pp=p+1;
    char fm[p-f+4];sprintf(fm,"%%%.*s%s%c",(I)(p-f),f,strchr("di",c)||strchr("ouxX",c)?"j":"L",c);
    $(strchr("bcs",c),                   // by hand, not printf: %b can write null bytes
      t=s=a?a:"";m=c=='c'?!!*s:strlen(s);
      if(c=='b')for(t=mal(m),m=0;*s&&!x;)(d=*s=='\\'?(s++,esc(&s,1)):(C)*s++)==(size_t)-1?x=1:(t[m++]=d);
      m=pr<0?m:MN(m,(size_t)pr);d=w>(I)m?w-m:0;
      I l=!!memchr(f,'-',p-f);O("%*s",l?0:(I)d,"");fwrite(t,1,m,stdout);O("%*s",l?(I)d:0,"");
      if(x)done())
    $(strchr("di",c),O(fm,ival(a,0)))
    $(strchr("ouxX",c),O(fm,(UJ)ival(a,1)))
    O(fm,fval(a));
}
I main(I ac,S*av){
    setlocale(LC_ALL,"");
    setprog(*av++);
    if(*av&&!strcmp(*av,"--"))av++;      // no options, but -- is still discarded
    if(!*av)xusage("format [argument...]");
    ar=av+1;W(ar[na])na++;
    do{                                  // reuse the format until the arguments run out
        k=hi=0;
        for(S p=*av;*p;)$(*p=='\\',p++;putchar(esc(&p,0)))$(*p!='%'||p[1]=='%',putchar(*p);p+=1+(*p=='%'))cv(&p);
        bs+=hi;
    }W(hi&&bs<na);
    done();
}
