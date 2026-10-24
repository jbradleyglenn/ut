// pw [-L|-P]
#include"a.h"
#define U "[-L|-P]"
I ok(S p){                                       // -L: PWD, if absolute, with no dot or dot-dot, and the working directory
    struct stat a,b;
    if(!p||*p!='/'||stat(p,&a)||stat(".",&b)||a.st_dev!=b.st_dev||a.st_ino!=b.st_ino)R 0;
    for(;;p+=strcspn(p,"/")){
        p+=strspn(p,"/");if(!*p)R 1;
        if(*p=='.'&&(!p[1]||p[1]=='/'||(p[1]=='.'&&(!p[2]||p[2]=='/'))))R 0;
    }
}
S gcwd(V){                                       // getcwd of any length; 0 if it can't be determined
    S b=0;for(size_t n=256;;n*=2){b=rea(b,n);if(getcwd(b,n))R b;if(errno!=ERANGE)R free(b),(S)0;}
}
I main(I ac,S*av){
    I o,ph=0;S p;
    setprog(*av);
    W((o=getopt(ac,av,"LP"))!=-1)S(o,C('L',ph=0)C('P',ph=1),xusage(U))
    if(optind<ac)xusage(U);
    p=getenv("PWD");
    if(ph||!ok(p))if(!(p=gcwd()))xerr(1,"getcwd");
    puts(p);xclose();R 0;
}
