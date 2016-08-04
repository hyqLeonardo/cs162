#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit ls, lh, lf;
    getrlimit(RLIMIT_STACK, &ls);
    printf("stack size: %ld\n", (long int)ls.rlim_cur);
    getrlimit(RLIMIT_NPROC, &lh);
    printf("process limit: %ld\n", (long int)lh.rlim_cur);
    getrlimit(RLIMIT_NOFILE, &lf);
    printf("max file descriptors: %ld\n", (long int)lf.rlim_cur);

    getrlimit(RLIMIT_AS, &ls);
    printf("max virtual memory: %ld\n", (long int)ls.rlim_cur);
    getrlimit(RLIMIT_CPU, &ls);
    printf("cpu time limit: %ld\n", (long int)ls.rlim_cur);
    return 0;
}
