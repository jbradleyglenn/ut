// ls [-ikqrs] [-ln] [-A|-a] [-C|-m|-x|-1] [-F|-p] [-H|-L] [-R|-d] [-S|-f|-t] [-c|-u] [file...]
#include"a.h"
#define U "[-ikqrs] [-ln] [-A|-a] [-C|-m|-x|-1] [-F|-p] [-H|-L] [-R|-d] [-S|-f|-t] [-c|-u] [file...]"
#define HALFYR 15778476                          // six months: half a Gregorian year, in seconds
struct e{S nm,pa;struct stat st;};               // name to write, path to it, its information
struct an{dev_t d;ino_t i;struct an*up;};        // a directory being listed and its parent: for -R cycles
char op[128];I a,f,p,h,so,tm,hd,wr,rc,cols=80;   // flags given; last of -A|-a, -l|-C|-m|-x|-1, -F|-p, -H|-L, -S|-t, -c|-u; headers, written yet
I nf,wi,wb,wl,wu,wg,wz;time_t now;               // operand symlinks not followed; field widths of the listing

I nw(UJ x){R snprintf(0,0,"%ju",x);}
UJ bk(struct stat*s){R op['k']?((UJ)s->st_blocks+1)/2:(UJ)s->st_blocks;} // st_blocks: 512-byte units
time_t ts(struct stat*s){R tm=='c'?s->st_ctime:tm=='u'?s->st_atime:s->st_mtime;}
I cmp(const V*x,const V*y){
    struct e*l=(V*)x,*r=(V*)y;time_t s=ts(&l->st),t=ts(&r->st);I c=0;
    if(so=='S')c=(l->st.st_size<r->st.st_size)-(l->st.st_size>r->st.st_size);  // biggest first
    if(so=='t')c=(s<t)-(s>t);                   // newest first
    if(!c)c=strcmp(l->nm,r->nm);               // the POSIX locale's collation: bytes in order
    R op['r']?-c:c;
}
V pn(S s){for(;*s;s++)putchar(op['q']&&!isprint((C)*s)?'?':*s);}
I ind(mode_t m){R !p?0:S_ISDIR(m)?'/':p=='p'?0:S_ISLNK(m)?'@':S_ISFIFO(m)?'|':S_ISSOCK(m)?'=':S_ISREG(m)&&m&0111?'*':0;}
I cw(struct e*e){R(op['i']?wi+1:0)+(op['s']?wb+1:0)+(I)strlen(e->nm)+!!ind(e->st.st_mode);} // width of an entry outside -l
V pre(struct stat*s){if(op['i'])O("%*ju ",wi,(UJ)s->st_ino);if(op['s'])O("%*ju ",wb,bk(s));} // [inode ][blocks ]
V nm(S s,mode_t m){I c=ind(m);pn(s);if(c)putchar(c);}                                   // name[indicator]
V ent(struct e*e){pre(&e->st);nm(e->nm,e->st.st_mode);}
S own(struct stat*s,I g,S b){                    // owner (g: group) name, or its number
    struct passwd*u;struct group*r;
    if(!op['n']&&!g&&(u=getpwuid(s->st_uid)))R u->pw_name;
    if(!op['n']&&g&&(r=getgrgid(s->st_gid)))R r->gr_name;
    sprintf(b,"%ju",g?(UJ)s->st_gid:(UJ)s->st_uid);R b;
}
S sz(struct stat*s,S b){S_ISCHR(s->st_mode)||S_ISBLK(s->st_mode)?sprintf(b,"%ju",(UJ)s->st_rdev):sprintf(b,"%jd",(J)s->st_size);R b;} // device info: st_rdev
V md(mode_t m,S b){                              // file mode; no alternate access method flag
    *b=S_ISDIR(m)?'d':S_ISBLK(m)?'b':S_ISCHR(m)?'c':S_ISLNK(m)?'l':S_ISFIFO(m)?'p':S_ISSOCK(m)?'s':S_ISREG(m)?'-':'?';
    N(3,I x=m&0100>>3*i,s=m&(I[]){S_ISUID,S_ISGID,S_ISVTX}[i];
        b[1+3*i]=m&0400>>3*i?'r':'-';b[2+3*i]=m&0200>>3*i?'w':'-';
        b[3+3*i]=s?"sst"[i]^(x?0:32):x?'x':'-')  // set-ID or sticky: lowercase if executable, uppercase if not
    b[10]=0;
}
V lg(struct e*e){
    struct stat*s=&e->st,t;char m[11],u[24],g[24],z[48],d[32],l[PATH_MAX];time_t w=ts(s);ssize_t k;
    md(s->st_mode,m);
    strftime(d,sizeof d,w>now||now-w>HALFYR?"%b %e  %Y":"%b %e %H:%M",localtime(&w));
    pre(s);
    O("%s %*ju %-*s %-*s %*s %s ",m,wl,(UJ)s->st_nlink,wu,own(s,0,u),wg,own(s,1,g),wz,sz(s,z),d);
    $(S_ISLNK(s->st_mode)&&(k=readlink(e->pa,l,sizeof l-1))>=0,l[k]=0;pn(e->nm);O(" -> ");nm(l,stat(e->pa,&t)?0:t.st_mode)) // dangling: no indicator, not an error
    nm(e->nm,s->st_mode);
    putchar('\n');
}
V show(struct e*v,size_t n,I dir){               // one list of files: of a directory, or the non-directory operands
    UJ t=0;I l=f=='l',w=0,r,c;char b[48];
    wi=wb=wl=wu=wg=wz=0;
    for(size_t j=0;j<n;j++){
        struct stat*s=&v[j].st;
        t+=s->st_blocks;wi=MX(wi,nw(s->st_ino));wb=MX(wb,nw(bk(s)));
        if(l)wl=MX(wl,nw(s->st_nlink)),wu=MX(wu,(I)strlen(own(s,0,b))),wg=MX(wg,(I)strlen(own(s,1,b))),wz=MX(wz,(I)strlen(sz(s,b)));
    }
    if(dir&&(l||op['s']))O("total %ju\n",op['k']?(t+1)/2:t),wr=1;
    wr|=n>0;
    $(l,for(size_t j=0;j<n;j++)lg(v+j))
    $(f=='1',for(size_t j=0;j<n;j++)ent(v+j),putchar('\n'))
    $(f=='m',for(size_t j=0,x=0,y;j<n;j++){      // x: column; room for the next entry and its comma, or a newline
        y=cw(v+j);
        if(j)putchar(','),x++,x+1+y+(j+1<n)>(size_t)cols?putchar('\n'),x=0:(putchar(' '),x++);
        ent(v+j);x+=y;
        if(j+1==n)putchar('\n');
    })
    {                                            // -C down the columns, -x across: constant width, 2 blanks apart
        for(size_t j=0;j<n;j++)w=MX(w,cw(v+j));
        w+=2;c=MX((cols+2)/w,1);r=(n+c-1)/c;if(f=='C'&&r)c=(n+r-1)/r;
        N(r,for(I k=0;k<c;k++){
            size_t j=f=='C'?k*r+i:i*c+k,nx=f=='C'?j+r:j+1;
            if(j>=n)break;
            ent(v+j);if(k+1<c&&nx<n)O("%*s",w-cw(v+j),"");
        }putchar('\n'))
    }
}
I gs(S pa,struct stat*s,I top){                  // information for pa: of a symlink, or of what it references
    struct stat t;
    if(lstat(pa,s))R xwarn("%s",pa);
    if(S_ISLNK(s->st_mode)&&!stat(pa,&t)&&(h=='L'||(top&&(h||(!nf&&S_ISDIR(t.st_mode))))))*s=t;
    R 0;
}
I dot(S s){R*s=='.'&&(!s[1]||(s[1]=='.'&&!s[2]));} // . or ..
V dls(S pa,struct stat*ds,struct an*up){         // list directory pa, then with -R its subdirectories
    DIR*dp;struct dirent*de;struct e*v=0;size_t n=0,c=0,l=strlen(pa);struct an me={ds->st_dev,ds->st_ino,up};
    for(struct an*x=up;x;x=x->up)P(x->d==me.d&&x->i==me.i,(V)(rc=xwarnx("%s: directory causes a cycle",pa)))
    if(hd){if(wr)putchar('\n');pn(pa);O(":\n");wr=1;}
    P(!(dp=opendir(pa)),(V)(rc=xwarn("%s",pa)))
    W((errno=0,de=readdir(dp))){
        S s=de->d_name;
        if(*s=='.'&&(!a||(a=='A'&&dot(s))))continue;
        if(!op['q']&&strchr(s,'\n')){rc=xwarnx("pathname contains a newline");continue;} // FUTURE DIRECTIONS: an error
        if(n==c)v=rea(v,(c=c*2+16)*sizeof*v);
        v[n].pa=mal(l+strlen(s)+2);sprintf(v[n].pa,"%s%s%s",pa,l&&pa[l-1]=='/'?"":"/",s);
        v[n].nm=v[n].pa+strlen(v[n].pa)-strlen(s);
        if(gs(v[n].pa,&v[n].st,0))rc=1,free(v[n].pa);else n++;
    }
    if(errno)rc=xwarn("%s",pa);
    closedir(dp);
    if(!op['f'])qsort(v,n,sizeof*v,cmp);
    show(v,n,1);
    for(size_t j=0;j<n;j++){if(op['R']&&S_ISDIR(v[j].st.st_mode)&&!dot(v[j].nm))dls(v[j].pa,&v[j].st,&me);free(v[j].pa);}
    free(v);
}
I main(I ac,S*av){
    I o;size_t n=0,m=0;struct e*fv,*dv;S c=getenv("COLUMNS");
    setprog(*av);
    W((o=getopt(ac,av,"ikqrslnAaCmx1FpHLRdSftcu"))!=-1){
        if(o=='?')xusage(U);
        op[o]=1;                                 // in each set of options that exclude each other, the last wins
        if(strchr("Aa",o))a=o;
        if(strchr("lnCmx",o))f=o=='n'?'l':o;
        if(o=='1')f=op['l']||op['n']?'l':o;      // -1 overrides -C -m -x, and so brings back -l
        if(strchr("Fp",o))p=o;
        if(strchr("HL",o))h=o;
        if(strchr("St",o)||(strchr("cu",o)&&op['t']))so=o=='S'?o:'t'; // with -t, -c and -u exclude -S
        if(strchr("cu",o))tm=o;
    }
    if(isatty(1))op['q']=1,f=f?f:'C';           // to a terminal: implementation-defined
    if(!f)f='1';
    if(op['f'])a='a';
    nf=op['d']||op['F']||op['l']||op['n'];
    if(c&&atoi(c)>0)cols=atoi(c);
    now=time(0);
    av+=optind;if(!*av)av=(S[]){".",0};
    hd=op['R']||av[1];
    fv=mal(ac*sizeof*fv);dv=mal(ac*sizeof*dv);
    for(;*av;av++){
        struct e e={*av,*av};
        if(!op['q']&&strchr(*av,'\n')){rc=xwarnx("pathname contains a newline");continue;}
        if(gs(*av,&e.st,1)){rc=1;continue;}
        $(S_ISDIR(e.st.st_mode)&&!op['d'],dv[m++]=e)fv[n++]=e;
    }
    if(!op['f'])qsort(fv,n,sizeof*fv,cmp),qsort(dv,m,sizeof*dv,cmp); // non-directories first
    show(fv,n,0);
    for(size_t j=0;j<m;j++)dls(dv[j].pa,&dv[j].st,0);
    xclose();
    R rc;
}
