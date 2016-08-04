#include <stdio.h>

int main() {
	int i;
	int a = 2147483647;
	printf("%d\n", a);

	for (i = 0; i < 10; i++)
		printf("%d\n", a+i);

	return 0;
}