/*
  * threadHello.c -- Simple multi-threaded program.
  *
  * Compile with
  *    > gcc -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS threadHello.c -c -o threadHello.o
  *    > gcc -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS sthread.c -c -o sthread.o
  *    > gcc -lpthread threadHello.o sthread.o -o threadHello
  * Run with
  *    > ./threadHello
  */
#include <stdio.h>
#include "sthread.h"

static void go(int n);

#define NTHREADS 10
static sthread_t threads[NTHREADS];

int main(int argc, char **argv)
{
  int ii;

  for(ii = 0; ii < NTHREADS; ii++){
    sthread_create(&(threads[ii]), &go, ii);
  }
  for(ii = 0; ii < NTHREADS; ii++){
    long ret = sthread_join(threads[ii]);
    printf("Thread %d returned %ld\n", ii, ret);
  }
  printf("Main thread done.\n");
  return 0;
}

void go(int n)
{
  printf("Hello from thread %d\n", n);
  sthread_exit(100 + n);
  // Not reached
}

