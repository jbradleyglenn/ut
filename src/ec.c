// no flags, no \ interp (posix, not xsi)
#include"a.h"
I main(I ac,S*av){
    setprog(*av);
    N(ac-1,O(&" %s"[!i],av[i+1]))
    putchar('\n');
    xclose();
}
