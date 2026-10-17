// kill [-s signal_name] pid...
// kill -l [status]
#include"a.h"
struct{S n;I v;}sg[]={{"HUP",SIGHUP},{"INT",SIGINT},{"QUIT",SIGQUIT},{"ILL",SIGILL},{"ABRT",SIGABRT},{"BUS",SIGBUS},{"FPE",SIGFPE},{"KILL",SIGKILL},{"USR1",SIGUSR1},{"SEGV",SIGSEGV},{"USR2",SIGUSR2},{"PIPE",SIGPIPE},{"ALRM",SIGALRM},{"TERM",SIGTERM},{"CHLD",SIGCHLD},{"CONT",SIGCONT},{"STOP",SIGSTOP},{"TSTP",SIGTSTP},{"TTIN",SIGTTIN},{"TTOU",SIGTTOU},{"URG",SIGURG},{"WINCH",SIGWINCH}};
#define CT(x) (I)(sizeof(x)/sizeof*(x))
#define A(s) (*av&&!strcmp(*av,s))
I mklong(S s,long*v){S e;*v=strtol(s,&e,10);R e!=s;}
S nam(long v){N(CT(sg),P(sg[i].v==v,sg[i].n))R 0;}
I num(S s){P(!strcmp(s,"0"),0)N(CT(sg),P(!strcasecmp(s,sg[i].n),sg[i].v))R xerrx(1,"%s: unknown signal",s);}
I main(I ac,S*av){
    long v;I s=SIGTERM,r=0;
    setprog(*av++);
    P(A("-l"),                       // 128 is an impl choice, if kill is a builtin, it must match $? building logic
      $(!*++av,N(CT(sg),puts(sg[i].n)))
      $(av[1]||!mklong(*av,&v)||v<0,nyi())
      $(!nam(v-=v>128?128:0),xerrx(1,"%s: unknown signal",*av))
      puts(nam(v));
      xclose();0)
    if(A("-s"))s=av[1]?num(av[1]):nyi(),av+=2;
    $(A("--"),av++)P(*av&&'-'==**av,nyi())
    P(!*av,nyi())
    W(*av)r|=!mklong(*av,&v)||v!=(pid_t)v?xwarnx("%s: invalid pid",*av):kill(v,s)?xwarn("%s",*av):0,av++;
    R r;
}
// POSIX.1-2024 fixes very few signal numbers.
// apart from 0 (the null signal), it gives only seven, and only as part of the optional XSI extension
// using the SIG* macros guarantees this kill is in sync with the systems signal numbers
