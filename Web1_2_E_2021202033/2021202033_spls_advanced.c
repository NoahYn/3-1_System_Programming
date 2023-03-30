///////////////////////////////////////////////////////////////////////
// File Name : 2021202033_spls_advanced.c							 //
// Date : 2023/03/25	 											 //
// Os : Ubuntu 16.04 LTS 64bits 									 //
// Author : Sung Min Yoon 									 	 	 //
// Student ID : 2021202033											 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-2						 //
// Description : This file is source code for Assignment #1-2		 //
///////////////////////////////////////////////////////////////////////

#include <stdio.h> // for printf
#include <stdlib.h> // for malloc
#include <dirent.h> // for opendir, readdir, closedir
#include <ctype.h> // for toupper
#include <string.h> // for strlen
#include <unistd.h> // for option
#include <sys/stat.h>
#include <sys/types.h> 
#include <pwd.h>
#include <grp.h>
#include <time.h>

typedef struct s_list {
	char *path; // file name
	char *compare; // to dismiss alphabet lower & upper case
	struct s_list *next;
	struct s_list *prev; // will be used in sorting
} t_list;

void get_list(t_list **head, DIR *dirp);
void sort_list(t_list **head);
void print_list(t_list **head);
static int aflag = 0; // -a option
static int lflag = 0; // -l option

int main(int argc, char *argv[]) {
	DIR *dirp; // directory stream
	t_list *head; // head of list
	char cwd[1024]; // path of current working directory	
	int opt = 0; // return value of getopt
	opterr = 0;


	while ((opt = getopt(argc, argv, ":al")) != -1) {
		switch(opt) {
			case 'a': // -a option 
				aflag++;
				break;
			case 'l': // -l option
				lflag++;
				break;
			case '?': // other undefined options
				fprintf(stderr, "%s: invalid option\nTry 'ls --help' for more information.\n", argv[0]+2);
				break;
		}
	}

	if (argc == optind) // default. open current directory ('.')
		dirp = opendir(".");
	else if (argc == optind+1) // one input. open certain directory 
		dirp = opendir(argv[1]);
	if (dirp == NULL) { // exception : input path are not dir or not exist
		fprintf(stderr, "cannot access '%s' : No such directory\n", argv[1]);
		exit(1);
	}

	getcwd(cwd, 1024);
	printf("Directory path: %s\n", cwd);

	printf("a = %d, l = %d\n", aflag, lflag);
	get_list(&head, dirp); // make list of dirent
	sort_list(&head); // sort list
	print_list(&head); // print list
	closedir(dirp);
	return 0;
}

///////////////////////////////////////////////////////////////////////
// get_list															 //
// ================================================================= //
// Input: t_list** -> pointer of head, 								 //
// 		  DIR* -> directory stream for getting directory entry info  //
// Output: void 													 //
// Purpose: Make linked list which contain name of files in certain  //
// 			directory to print out 									 //
///////////////////////////////////////////////////////////////////////

void get_list(t_list **list, DIR *dirp) {
	t_list *temp = 0; // temporary node
	t_list *head = 0; // head
	struct dirent *dir; // directory entry

	while ((dir = readdir(dirp)) != NULL) { // read from dirent until all file is readed
		if (!aflag && dir->d_name[0] == '.') continue; // skip hidden file without aflag
		if (head == 0) { // initialize head
			head = (t_list *)malloc(sizeof(t_list));
			if (!head) // exception : bad allocation
				exit(1);
			head->path = 0; 
			head->compare = 0;
			head->prev = 0;
			*list = head;
		}
		else
			head = temp;
		temp = (t_list *)malloc(sizeof(t_list)); // initialize temp
		if (!temp) // exception : bad allocation
			exit(1);
		head->next = temp;
		temp->prev = head;
		temp->path = dir->d_name;
		temp->next = 0;
		temp->compare = (char *)malloc(sizeof(char)*strlen(temp->path)); // upper version of name to compare without u/lcase
		if (!temp->compare) // exception : bad allocatoin
			exit(1);
		if (aflag && temp->path[0] == '.') { // hidden file
			for (size_t i = 0; i < strlen(temp->path); i++) // toupper and copy 
				temp->compare[i] = toupper(temp->path[i+1]); 		
		}
		else {
			for (size_t i = 0; i < strlen(temp->path); i++) // toupper and copy 
				temp->compare[i] = toupper(temp->path[i]); 
		}
	}
}

///////////////////////////////////////////////////////////////////////
// sort_n_print														 //
// ================================================================= //
// Input: t_list** -> pointer of head								 //
// Output: void 													 //
// Purpose: Sorting linked list(using selection sort) and print out	 //
///////////////////////////////////////////////////////////////////////

void sort_list(t_list **head) {
	t_list *temp; // temporary node
	t_list *sorted = (t_list*)malloc(sizeof(t_list)); // new head of sorted list
	t_list *s_temp = sorted; // temporary node for sorted list
	t_list *s_min; // select minimum(most left component)
	char *min; 
	size_t size; // flag for using while condition
	do {
		size = 0;
		temp = (*head)->next; // point to first component
		if (temp) { // setting for comparing
			min = temp->compare;
			s_min = temp;
		}
		while (temp) {
			if (temp->next) { // comparing minimum with next node's name
				if (strcmp(min, temp->next->compare) > 0) { // select when next name is smaller
					min = temp->next->compare;
					s_min = temp->next;
				}
			}
			temp = temp->next; // move to next node
			size++;
		}
		s_min->prev->next = s_min->next; // disconnect from ordinary list
		if (s_min->next)
			s_min->next->prev = s_min->prev;
		s_min->next = 0; // reinitialize for sorted list 
		s_min->prev = s_temp;
		s_temp->next = s_min;
		s_temp = s_min;
	} while(size > 1); // finish when all the node in list move to sorted list
	*head = sorted;
}

void print_list(t_list **head) {
	size_t total = 0;
	t_list *temp = (*head)->next; // point to first node of sorted list to print
	struct stat *st;
	struct passwd *pw;
	char *name;
	struct group *gr;
	struct tm *tm;
	int mode;

//	printf("total : %d\n"); // 1kB
	while (temp) { // print all the node
		if (lflag > 0) {
			if (stat(temp->path, st) == 0) {
				if ((st->st_mode)>>12 == 04) // 04(8진수) dir
					printf("d");
				else if ((st->st_mode)>>12 == 010) // 010(8진수) regular file
					printf("-"); 
				else if ((st->st_mode)>>12 == 012)
					printf("l");

				mode = st->st_mode%(1<<10); // 배열하고 for문으로 9개 압축
				if (mode>>8 == 1) {
					printf("r");
					mode -= (1<<8);
				}
				else
					printf("-");
	
				if (mode>>7 == 1) {
					printf("w");
					mode -= (1<<7);
				}
				else
					printf("-");
		
				if (mode>>6 == 1) {
					printf("x"); 
					mode -= (1<<6);
				}
				else
					printf("-");

				if (mode>>5 == 1) {
					mode -= (1<<5);
					printf("r");
				}
				else
					printf("-");

				if (mode>>4 == 1) {
					printf("w");
					mode -= (1<<4);
				}
				else
					printf("-");

				if (mode>>3 == 1) {
					printf("x");
					mode -= (1<<3);
				}
				else
					printf("-");

				if (mode>>2 == 1) {
					mode -= (1<<2);
					printf("r");
				}
				else
					printf("-");

				if (mode>>1 == 1) {
					printf("w");
					mode -= (1<<1);
				}
				else
					printf("-");

				if (mode == 1) 
					printf("x");
				else
					printf("-");


//				pw = getpwuid(st->st_uid);
//				name = pw->pw_name;
//				printf("%s\t", name);
			}
			// rwx rwx rwx
			// 111 111 111
			printf("\t%o", st->st_mode);
			printf("\t%s\n", temp->path);
//			printf("%s\t%d\t%ld\t%d\t%ld\t%ld\n", temp->path, st->st_mode, st->st_nlink, st->st_uid, st->st_size, st->st_blksize);
		}
		// format, mode, nlink, uid(1000), gid, byte(st_size), date, name 

		else {
			printf("%s\n", temp->path); 
		}
		temp = temp->next;
	}

}