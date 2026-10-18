// mkdir [-p] [-m mode] dir...
#include"a.h"
#define U "[-p] [-m mode] dir..."
I um,rc;                                         // file mode creation mask

I mode(S s){                                     // chmod's mode operand, symbolic relative to a=rwx; -1 if invalid
    S e,wh="ugoa",pm="rwxXs";I m=0777,w,b,o;      // X: the file is a directory, so x; no t: XSI
    if(isdigit((C)*s)){long v=strtol(s,&e,8);R*e||v>07777?-1:v;}
    do{
        for(w=0;*s&&(e=strchr(wh,*s));s++)w|=(I[]){04700,02070,07,06777}[e-wh];
        if(!*s||!strchr("+-=",*s))R -1;          // a clause has at least one action
        W(*s&&strchr("+-=",*s)){
            o=*s++;b=0;
            $(*s&&(e=strchr(wh,*s))&&*s!='a',b=(m>>(6-3*(e-wh))&7)*0111;s++)       // permcopy: u g o as they are now
            for(;*s&&(e=strchr(pm,*s));s++)b|=(I[]){0444,0222,0111,0111,06000}[e-pm];
            b&=w?w:06777&~um;                    // no who: all of them, but not the bits in the mask
            if(o!='+')m&=~(o=='-'?b:w?w:07777);
            if(o!='-')m|=b;
        }
    }W(*s==','&&++s);
    R*s?-1:m;
}
I isd(S d){struct stat s;R!stat(d,&s)&&S_ISDIR(s.st_mode);}
I mk(S d,I m,I ok){I e;P(!mkdir(d,m),1)e=errno;P(ok&&isd(d),0)errno=e;R -1;} // 1 made, 0 there already (ok), -1 error
I pre(S d){                                      // -p: each missing directory of the path prefix
    for(S q=d;(q=strchr(q,'/'));q++){
        I r;char c=q[1];
        if(q==d||q[-1]=='/'||!q[strspn(q,"/")])continue; // a slash that ends a component, and another follows
        q[1]=0;r=mk(d,0,1);                      // with its trailing slash
        if(r>0&&chmod(d,(S_IWUSR|S_IXUSR|~um)&0777))r=-1;
        q[1]=c;
        if(r<0){*q=0;xwarn("%s",d);*q='/';R 1;}
    }
    R 0;
}
I main(I ac,S*av){
    I o,p=0,m=-1,r;
    setprog(*av);
    um=umask(0);umask(um);
    W((o=getopt(ac,av,"pm:"))!=-1)S(o,C('p',p=1)C('m',if((m=mode(optarg))<0)xerrx(2,"%s: invalid mode",optarg)),xusage(U))
    av+=optind;
    if(!*av)xusage(U);
    for(;*av;av++){
        if(strchr(*av,'\n')){rc=xwarnx("pathname contains a newline");continue;} // FUTURE DIRECTIONS: an error
        if(p&&pre(*av)){rc=1;continue;}
        r=mk(*av,m<0?0777:m&0777,p);             // with -m: never less restrictive than mode, then exactly mode
        if(r<0||(r>0&&m>=0&&chmod(*av,m)))rc=xwarn("%s",*av);
    }
    R rc;
}
