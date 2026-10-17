// cat [-u] [file...]
#include"a.h"
I main(I ac,S*av){
    I o,fd,rc=0;char b[1<<16];ssize_t n,w;S*fs;
    setprog(*av);
    W((o=getopt(ac,av,"u"))!=-1)if(o!='u')xusage("[-u] [file...]"); // -u: always so, each read is written at once
    for(fs=av[optind]?av+optind:(S[]){"-",0};*fs;fs++){
        if((fd=strcmp(*fs,"-")?open(*fs,O_RDONLY):0)<0){rc=xwarn("%s",*fs);continue;} // keep going
        W((n=read(fd,b,sizeof b))>0)for(S p=b;n;p+=w,n-=w)if((w=write(1,p,n))<0)xerr(1,"stdout");
        if(n<0)rc=xwarn("%s",*fs);
        if(fd)close(fd);                    // - is not closed: a later - reads on
    }
    R rc;
}
