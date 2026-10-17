// kill [-s signal_name] pid...
// kill -l [status]
#include"a.h"
struct{char*n;int v;}sigs[]={{"HUP",SIGHUP},{"INT",SIGINT},{"QUIT",SIGQUIT},{"ILL",SIGILL},{"ABRT",SIGABRT},{"BUS",SIGBUS},{"FPE",SIGFPE},{"KILL",SIGKILL},{"USR1",SIGUSR1},{"SEGV",SIGSEGV},{"USR2",SIGUSR2},{"PIPE",SIGPIPE},{"ALRM",SIGALRM},{"TERM",SIGTERM},{"CHLD",SIGCHLD},{"CONT",SIGCONT},{"STOP",SIGSTOP},{"TSTP",SIGTSTP},{"TTIN",SIGTTIN},{"TTOU",SIGTTOU},{"URG",SIGURG},{"WINCH",SIGWINCH}};
#define CT(x) (int)(sizeof(x)/sizeof*(x))
int mklong(char*s,long*v){char*e;*v=strtol(s,&e,10);return e!=s;}
char*nam(long v){for(int i=0;i<CT(sigs);i++)if(sigs[i].v==v)return sigs[i].n;return NULL;}
int num(char*s){if(!strcmp(s,"0"))return 0; for(int i=0;i<CT(sigs);i++)if(!strcasecmp(s,sigs[i].n))return sigs[i].v; xerrx(1,"%s: unknown signal",s); return 0;}
int main(int ac,char**av){
    long v; int s=SIGTERM, r=0;
    setprog(*av++);
    if(*av&&!strcmp(*av,"-l")){(!*++av)?({for(int i=0;i<CT(sigs);i++)puts(sigs[i].n);}):(av[1]||!mklong(*av,&v)||v<0)?nyi(): (v>128&&(v-=128),nam(v))?puts(nam(v)):xerrx(1,"%s: unknown signal",*av),xclose();return 0;} // 128 is an impl choice, if kill is a builtin, it must match $? building logic
    if(*av&&!strcmp(*av,"-s"))(!av[1])?nyi():(s=num(av[1]),av+=2);
    if(*av&&!strcmp(*av,"--"))av++;
    else if(*av&&'-'==**av)nyi();
    if(!*av)nyi();
    for(;*av;av++)r|=(!mklong(*av,&v)||v!=(pid_t)v)?(xwarnx("%s: invalid pid",*av),1):kill(v,s)?(xwarn("%s",*av),1):0;
    return r;
}
// POSIX.1-2024 fixes very few signal numbers.
// apart from 0 (the null signal), it gives only seven, and only as part of the optional XSI extension
// using the SIG* macros guarantees this kill is in sync with the systems signal numbers