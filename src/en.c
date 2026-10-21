// env [-i] [name=value]... [utility [argument...]]
#include"a.h"
#define U "[-i] [name=value]... [utility [argument...]]"
extern S*environ;
I main(I ac,S*av){
    S*v,q;I x=0,n=0,i;
    setprog(*av++);
    for(;*av&&**av=='-';av++){                   // not getopt: it may permute, and the utility's options are its own
        if(!strcmp(*av,"--")){av++;break;}
        if(!av[0][1])x=1;                        // "-": unspecified; historically -i
        for(q=*av+1;*q;q++)if(*q=='i')x=1;else xusage(U);
    }
    if(!x)W(environ[n])n++;
    v=memcpy(mal((n+ac)*sizeof*v),environ,n*sizeof*v);
    for(;*av&&(q=strchr(*av,'='));av++){         // replace name, else append
        for(i=0;i<n&&strncmp(v[i],*av,q-*av+1);i++);
        v[i]=*av;n+=i==n;
    }
    v[n]=0;environ=v;                            // so execvp searches the new PATH
    if(!*av){for(;*v;v++)puts(*v);xclose();R 0;}
    execvp(*av,av);
    i=errno==ENOENT||errno==ENOTDIR?127:126;     // not found, or found but not invocable
    R xwarn("%s",*av),i;
}
