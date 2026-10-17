#include"a.h"
S prog="ut";
V setprog(S a){S p;if(a&&*a)prog=(p=strrchr(a,'/'))?p+1:a;}
I dg(I x,I e,S f,...){
    I s=errno;va_list ap;va_start(ap,f);
    fflush(stdout);fprintf(stderr,"%s: ",prog);
    if(f)vfprintf(stderr,f,ap);
    if(e)fprintf(stderr,"%s%s",f?": ":"",strerror(s));
    fputc('\n',stderr);va_end(ap);
    if(x>=0)exit(x);
    R 1;
}
I xusage(S s){fprintf(stderr,"usage: %s %s\n",prog,s);exit(2);}
V*rea(V*p,size_t n){R(p=realloc(p,n+!n))?p:(xerrx(1,"out of memory"),p);}
S sdp(S s){size_t n=strlen(s)+1;R memcpy(mal(n),s,n);}
/* close stdout, exit 1 on any write error (earlier or at close); call last. not dg: stdout is gone */
V xclose(V){
    I b=ferror(stdout),e;errno=0;
    if(fclose(stdout)||b)e=errno,fprintf(stderr,"%s: stdout: %s\n",prog,e?strerror(e):"write error"),exit(1);
}
