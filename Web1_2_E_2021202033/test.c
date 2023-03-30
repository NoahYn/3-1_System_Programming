#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int aflag = 0, bflag = 0;
	char *cvalue = NULL;
//	int i = 0, index = 0; 
	int c= 0;
	opterr = 0;

	while ((c = getopt(argc, argv, "abc:")) != -1) {
		switch(c) {
			case 'a':
				aflag++;
				break;
			case 'b':
				bflag++;
				break;
			case 'c':
				cvalue = optarg;
				break;
			case 'd':
				opterr = 0;
				break;
			case '?':
				printf("Unknow option character\n");
				break;
		}
	}
	printf("aflag = %d\t bflag = %d\t cvalue = %s\n",aflag, bflag, cvalue);
	printf("optind = %d, argc = %d\n", optind, argc);
	if (optind != argc) 
		printf("Non option argument %s\n", *(argv + optind));
	return 0;
}

