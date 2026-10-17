// no flags, no \ interp (posix, not xsi)
#include"a.h"
int main(int ac,char**av) {
    setprog(*av++);
    for(;*av;av++){fputs(*av,stdout);if(av[1])putchar(' ');}
    putchar('\n');
    xclose();
}
