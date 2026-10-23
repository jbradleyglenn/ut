// command [-p] command_name [argument...]
// command [-p] [-v|-V] command_name
#include"a.h"
#define U "[-p] [-v|-V] command_name [argument...]"
extern S prog;
S*ks[]={                                          // names the shell knows without a search: reserved words, special built-ins, intrinsic utilities
    (S[]){"!","{","}","case","do","done","elif","else","esac","fi","for","if","in","then","until","while",0},
    (S[]){"break",":","continue",".","eval","exec","exit","export","readonly","return","set","shift","times","trap","unset",0},
    (S[]){"alias","bg","cd","command","fc","fg","getopts","hash","jobs","kill","read","type","ulimit","umask","unalias","wait",0}};
S kd[]={"a reserved word","a special built-in utility","a regular built-in utility"};

V us(V){fprintf(stderr,"usage: %s %s\n",prog,U);exit(127);}  // 127: an error in command itself
I kn(S n){N(3,for(S*l=ks[i];*l;l++)if(!strcmp(*l,n))R i+1)R 0;} // n's kind, 1-based; 0 if none
S dp(V){size_t z=confstr(_CS_PATH,0,0);S s;R z?(confstr(_CS_PATH,s=mal(z),z),s):"/bin:/usr/bin";}
S nx(S*p,S n){                                    // the next prefix at *p applied to n; 0 after the last
    size_t m;S b;
    if(!*p)R 0;
    m=strcspn(*p,":");                            // a zero-length prefix: the current directory
    sprintf(b=mal(m+strlen(n)+2),"%.*s%s%s",(I)m,*p,m&&(*p)[m-1]!='/'?"/":"",n);
    *p=(*p)[m]?*p+m+1:0;
    R b;
}
I ok(S s){struct stat t;R!stat(s,&t)&&S_ISREG(t.st_mode)&&!access(s,X_OK);}
S wh(S n,S p){S b;if(strchr(n,'/'))R ok(n)?n:0;W((b=nx(&p,n))&&!ok(b))free(b);R b;} // pathname of utility n, 0 if none
S ab(S s){                                        // s as an absolute pathname
    char d[PATH_MAX];S b;
    if(*s=='/')R s;
    if(!getcwd(d,sizeof d))xerr(1,"getcwd");
    W(s[0]=='.'&&s[1]=='/')s+=2;
    sprintf(b=mal(strlen(d)+strlen(s)+2),"%s%s%s",d,d[1]?"/":"",s);
    R b;
}
I ex(S f,S*av){                                   // run f, or sh with f as its script if not an executable format; on return, the errno
    S a=*av,s;I e;
    execv(f,av);
    if((e=errno)==ENOEXEC&&(s=wh("sh",dp())))*av=f,av[-1]="sh",execv(s,av-1),e=errno,*av=a;
    R e;
}
I main(I ac,S*av){
    I v=0,p=0,k,e;S n,f,q,ph;
    setprog(*av++);
    for(;*av&&**av=='-'&&av[0][1];av++){          // not getopt: it may permute, and the utility's options are its own
        if(!strcmp(*av,"--")){av++;break;}
        for(q=*av+1;*q;q++)$(*q=='p',p=1)$((*q=='v'||*q=='V')&&(!v||v==*q),v=*q)us();
    }
    if(!(n=*av)||(v&&av[1]))us();
    if(p||!(ph=getenv("PATH")))ph=dp();
    if(v){
        k=kn(n);f=k?n:wh(n,ph);
        if(!f)R v=='V'?xwarnx("%s: not found",n):1; // -v: no output, the status says it
        if(!k&&strchr(f=ab(f),'\n'))xerrx(1,"pathname contains a newline"); // FUTURE DIRECTIONS: an error
        $(v=='v',puts(f))$(k,O("%s is %s\n",n,kd[k-1]))O("%s is %s\n",n,f);
        xclose();R 0;
    }
    k=ENOENT;                                     // arg0: the command name
    $(strchr(n,'/'),k=ex(n,av))if(*n)for(q=ph;(f=nx(&q,n));free(f))if((e=ex(f,av))!=ENOENT&&e!=ENOTDIR)k=e; // like execvp: a file that fails is passed over
    if(k==ENOENT||k==ENOTDIR)R xwarnx("%s: not found",n),127;
    errno=k;R xwarn("%s",n),126;                  // found but not invocable
}
