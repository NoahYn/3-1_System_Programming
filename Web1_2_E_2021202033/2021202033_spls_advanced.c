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
void print_node(char* path, size_t max_name, size_t max_group);
void print_list(t_list **head);
static int aflag = 0; // -a option
static int lflag = 0; // -l option

int main(int argc, char *argv[]) {
	DIR *dirp; // directory stream
	t_list *head; // head of list
	int opt = 0; // return value of getopt

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

// optind는 옵션아닌 첫번째 index
	printf("%d, %d\n", argc, optind);
	if (argc == optind) // default. open current directory ('.')
		dirp = opendir(".");
	else  {	// open certain directory 
		dirp = opendir(argv[optind]);
		if (dirp == NULL) { // exception : input path are not exist
			print_node(argv[optind], strlen(argv[optind]), strlen(argv[optind]));
		}
	}

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

void print_node(char* path, size_t max_name, size_t max_group) {
	struct stat st;
	int mode;
	struct tm *time;
	char *month[12] = {"Jan", "Fab", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char* name;
	char* group;

	if (lflag > 0) {
		if (lstat(path, &st) == 0) {
			mode = st.st_mode;
			if (S_ISREG(mode))
				printf("-");
			else if (S_ISDIR(mode)) 
				printf("d"); 
			else if (S_ISLNK(mode))
				printf("l");
			else if (S_ISBLK(mode))
				printf("b"); //
			else if (S_ISCHR(mode))
				printf("c");
			else if (S_ISFIFO(mode))
				printf("f"); //
			else if (S_ISSOCK(mode))
				printf("s");
			
			mode %= (1<<10); 	
			for (int i = 8; i >= 0; i--) {
				if (mode>>i == 1) {
					printf("%c", "xwr"[(i)%3]);
					mode -= (1<<i);
				}
				else
					printf("-");
			}
			printf(" %ld", st.st_nlink);
			name = getpwuid(st.st_uid)->pw_name;
			group = getgrgid(st.st_gid)->gr_name;
			printf(" %s", name);
			for (int i = 0; i < max_name - strlen(name); ++i) {
				printf(" ");
			}
			printf(" %s", group);
			for (int i = 0; i < max_group - strlen(group); ++i) {
				printf(" ");
			}
			printf(" %5ld", st.st_size);
			time = localtime(&(st.st_atime));
			printf(" %s %2d %02d:%02d", month[time->tm_mon], time->tm_mday, time->tm_hour, time->tm_min);
			printf(" %s\n", path);
		}
		else
			fprintf(stderr, "cannot access %s: No such file or directory\n", path);
	}
	else 
		printf("%s\n", path); 
}

void print_list(t_list **head) {
	size_t total = 0;
	t_list *temp = (*head)->next; // point to first node of sorted list to print
	struct stat st;
	size_t max_name = 0;
	size_t max_group = 0;
	char* name;
	char* group;

	lflag = 1;
	if (lflag > 0) {
		char cwd[1024]; // path of current working directory	
		getcwd(cwd, 1024);
		printf("Directory path: %s\n", cwd);
		while (temp) {
			if (lstat(temp->path, &st) == 0) {
				total += st.st_blocks/2;
				name = getpwuid(st.st_uid)->pw_name;
				group = getgrgid(st.st_gid)->gr_name;
				max_name = (strlen(name) > max_name ? strlen(name) : max_name);
				max_group = (strlen(group) > max_group ? strlen(group) : max_group);				
			}
			temp = temp->next;
		}
		printf("total %ld\n", total); 
		temp = (*head)->next;
	}
	while (temp) { // print all the node
		print_node(temp->path, max_name, max_group);
		temp = temp->next;
	}
}