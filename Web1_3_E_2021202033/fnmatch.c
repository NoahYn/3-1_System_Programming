#include <stdio.h>
#include <stdlib.h>
#include <fnmatch.h>

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("argc < 3 \n");
		return 0;
	}
	printf("%s and %s is ", argv[2], argv[1]);
	if (!fnmatch(argv[2], argv[1], 0))
		printf("matching\n");
	else
		printf("not matching\n");
	return 0;
}