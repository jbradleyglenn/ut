// wc [-c|-m] [-lw] [file...]
#include"a.h"
#define U "[-c|-m] [-lw] [file...]"
I f[3],m,rc;                                // fields written: newlines words bytes|characters; -m

V cnt(FILE*fp,UJ*n){                        // add to n: newlines words bytes|characters
    char b[MB_LEN_MAX];I k=0,c,s=1;wchar_t w;size_t z; // b: k bytes read ahead; s: last character was white space
    for(;;){
        if(!k&&(c=getc_unlocked(fp))!=EOF)b[k++]=c;
        if(!k)R;
        W((z=mbrtowc(&w,b,k,&(mbstate_t){0}))==(size_t)-2&&k<(I)MB_CUR_MAX&&(c=getc_unlocked(fp))!=EOF)b[k++]=c;
        if(z>(size_t)k)w='x',z=1;           // a byte that starts no character is one character, not white space
        z+=!z;                              // nul is one byte
        c=!!iswspace(w);n[0]+=w=='\n';n[1]+=s>c;n[2]+=m?1:z;s=c;
        memmove(b,b+z,k-=z);
    }
}
V pr(UJ*n,S s){I o=0;N(3,if(f[i])O(&" %ju"[!o++],n[i]))s?O(" %s\n",s):putchar('\n');} // no operand: no name, no blank
I main(I ac,S*av){
    I o;UJ t[3]={0};S*fs;
    setlocale(LC_ALL,"");
    setprog(*av);
    W((o=getopt(ac,av,"clmw"))!=-1)S(o,C('c',f[2]|=1)C('m',f[2]|=2)C('l',f[0]=1)C('w',f[1]=1),xusage(U))
    if(f[2]==3)xusage(U);
    m=f[2]&2;
    if(!f[0]&&!f[1]&&!f[2])N(3,f[i]=1)
    av+=optind;
    for(fs=*av?av:(S[]){"-",0};*fs;fs++){
        UJ n[3]={0};FILE*fp;
        if(strchr(*fs,'\n')){rc=xwarnx("pathname contains a newline");continue;} // FUTURE DIRECTIONS: an error
        if(!(fp=strcmp(*fs,"-")?fopen(*fs,"r"):stdin)){rc=xwarn("%s",*fs);continue;}
        cnt(fp,n);
        $(ferror(fp),rc=xwarn("%s",*fs))pr(n,*av?*fs:0),t[0]+=n[0],t[1]+=n[1],t[2]+=n[2];
        if(fp==stdin)clearerr(fp);else fclose(fp);
    }
    if(*av&&av[1])pr(t,"total");
    xclose();
    R rc;
}
