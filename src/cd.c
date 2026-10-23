// cd [-L|-P] [-e] [directory]   cd [-L|-P] [-e] -
// a separate process: the caller's working directory is unchanged, but the result (stdout, status) is cd's
#include"a.h"
#define U "[-L|-P] [-e] [directory | -]"
I ph,ee,x=1;                                     // -P, -e, error status: >1 with both

I isd(S d){struct stat s;R!stat(d,&s)&&S_ISDIR(s.st_mode);}
S cat(S a,size_t m,S b){                         // a's first m bytes, a slash unless a ends in one, b
    size_t n=strlen(b);I s=!m||a[m-1]!='/';S r=mal(m+s+n+1);
    memcpy(r,a,m);r[m]='/';memcpy(r+m+s,b,n+1);R r;
}
S gcwd(V){                                       // getcwd of any length; 0 if it can't be determined
    S b=0;for(size_t n=256;;n*=2){b=rea(b,n);if(getcwd(b,n))R b;if(errno!=ERANGE)R free(b),(S)0;}
}
S pwd(V){                                        // PWD, if it's an absolute pathname of the working directory
    S p=getenv("PWD");struct stat a,b;
    P(p&&*p=='/'&&!stat(p,&a)&&!stat(".",&b)&&a.st_dev==b.st_dev&&a.st_ino==b.st_ino,p)
    R gcwd();
}
S canon(S p){                                    // step 8: drop dot, resolve dot-dot lexically; 0 on error
    size_t l=strspn(p,"/"),k=0,r,m,s;S o=mal(strlen(p)+1);
    if(l==2)o[k++]='/';if(l)o[k++]='/';          // two leading slashes are implementation-defined: keep them
    for(r=k,p+=l;*p;p+=m,p+=strspn(p,"/")){
        m=strcspn(p,"/");
        if(m==1&&*p=='.')continue;
        for(s=k;s>r&&o[s-1]!='/';s--);           // the preceding component is o[s..k)
        if(m==2&&!strncmp(p,"..",2)&&k>r&&!(k-s==2&&!strncmp(o+s,"..",2))){
            o[k]=0;if(!isd(o))R xwarn("%s",o),free(o),(S)0;
            k=s>r?s-1:r;continue;
        }
        if(k>r)o[k++]='/';
        memcpy(o+k,p,m);k+=m;
    }
    o[k]=0;R o;
}
I main(I ac,S*av){
    I o,pr=0,L;S d,c,w,e,old,nw;size_t m;
    setprog(*av);
    W((o=getopt(ac,av,"LPe"))!=-1)S(o,C('L',ph=0)C('P',ph=1)C('e',ee=1),xusage(U))
    av+=optind;
    if(*av&&av[1])xusage(U);
    x=ph&&ee?2:1;
    if(!(d=*av)){if(!(d=getenv("HOME"))||!*d)xerrx(x,"HOME not set");} // step 1-2
    else if(!*d)xerrx(x,"empty directory operand");
    else if(!strcmp(d,"-")){if(!(d=getenv("OLDPWD"))||!*d)xerrx(x,"OLDPWD not set");pr=1;} // cd "$OLDPWD" && pwd
    c=d;
    if(*d!='/'&&!(*d=='.'&&(!d[1]||d[1]=='/'||(d[1]=='.'&&(!d[2]||d[2]=='/')))) // steps 3-5
       &&(e=getenv("CDPATH")))
        for(;*e;e+=m+!!e[m]){                    // a null entry: dot, same as the operand alone (step 6)
            m=strcspn(e,":");
            if(!m)continue;
            if(isd(w=cat(e,m,d))){c=w;pr=1;break;}
            free(w);
        }
    old=pwd();
    L=!ph&&(old||*c=='/');                       // no working directory to prefix: physically, like -P
    if(L){                                       // steps 7-9
        if(*c!='/')c=cat(old,strlen(old),c);
        if(!(c=canon(c)))R x;
        m=old?strlen(old):0;w=c;
        if(strlen(c)>=PATH_MAX&&strlen(d)<PATH_MAX&&m>1&&!strncmp(c,old,m)&&c[m]=='/')w=c+m+1;
    }else w=c;
    if(chdir(w))R xwarn("%s",d),x;               // step 10
    nw=L?c:gcwd();
    if(old)setenv("OLDPWD",old,1);
    if(!nw){if(ph&&ee)R xwarnx("cannot determine the working directory"),1;unsetenv("PWD");R 0;}
    setenv("PWD",nw,1);
    if(pr)puts(nw);
    xclose();R 0;
}
