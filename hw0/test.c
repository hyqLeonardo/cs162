#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef unsigned char *pointer;

void show_bytes(pointer start, size_t len) {
	size_t i;
	for (i = 0; i < len; i++)
		printf("%p\t0x%.2x\n", start+i, start[i]);
	printf("\n");
}

int fib(int a) {
	if (a == 0 || a == 1) {
		return 1;
	}
	else { 
		return fib(a-1) + a;
	}
}

int main() {
	// int a = 15213;
	// printf("int a = 15213;\n");
	// show_bytes((pointer) &a, sizeof(int));

	int result = fib(5);
	printf("%d\n", result);

  	return 0;
}
