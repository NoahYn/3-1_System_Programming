///////////////////////////////////////////////////////////////////////
// File Name : 2021202033_spls_advanced.c							 //
// Date : 2023/04/02	 											 //
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
#include <sys/stat.h> // for stat
#include <sys/types.h> 
#include <pwd.h> // for pwduid
#include <grp.h> // for groupgid
#include <time.h> // time

#define K 1024
typedef struct s_list {
	char path[256]; // file name
	char compare[256]; // to dismiss alphabet lower & upper case, hidden file
	struct s_list *next;
	struct s_list *prev; // use when sorting
} t_list;

int get_list(t_list **head, DIR *dirp);
void sort_list(t_list **head);
void print_node(char* path, char *name);
void print_list(t_list **head, char *path);
void exception(char *path);

static int aflag = 0; // -a option
static int lflag = 0; // -l option
static int hflag = 0; // -h option
static int rflag = 0; // -r option
static int sflag = 0; // -S option

static int nlink = 0; // padding for -l nlink to align
static int nname = 0; // padding for -l username to align 
static int ngroup = 0; // padding for -l group to align 

int main(int argc, char *argv[]) {
	DIR *dirp; // directory stream
	t_list *head; // head of list
	int opt = 0; // return value of getopt

	while ((opt = getopt(argc, argv, ":alhrS")) != -1) { // option handling
		switch(opt) {
			case 'a': // -a option 
				aflag++;
				break;
			case 'l': // -l option
				lflag++;
				break;
			case 'h': // -h option
				hflag++;
				break;
			case 'r': // -r option
				rflag++;
				break;
			case 'S': // -S option
				sflag++;
				break;
			case '?': // other undefined options
				fprintf(stderr, "%s: invalid option.\nonly a and l options are allowed.\n", argv[0]+2);
				return 0;
		}
	}

	if (argc == optind) { // default. open current directory ('.')
		dirp = opendir(".");
		get_list(&head, dirp); // make list of dirent
		sort_list(&head); // sort list
		if (lflag > 0) { // print path of current working directory	
			char cwd[256];
			getcwd(cwd, 256);
			printf("Directory path: %s\n", cwd);
		}
		print_list(&head, 0); // print list
		closedir(dirp);
	}
	else  {	// open certain path
		for (int i = 1; i < argc; i++) { // for print error first
			if (argv[i][0] == '-') continue; // pass option
			exception(argv[i]);
		}
		for (int i = 1; i < argc; i++) { 
			if (argv[i][0] == '-') continue; // pass option
			dirp = opendir(argv[i]);
			if (dirp == NULL) { // exception : input path are not directory
				nlink = 0; nname = 0; ngroup = 0; // not necessary to align (just one thing to print)
				print_node(argv[i], argv[i]); // print certain file
				continue;
			}
			if (get_list(&head, dirp) == -1) // make list of dirent
				continue; 
			sort_list(&head); // sort list
			if (lflag > 0)
				printf("Directory path: %s\n", argv[i]); // print Directory path
			print_list(&head, argv[i]); // print list
			closedir(dirp);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////
// exception														 //
// ================================================================= //
// Input: char* -> path string to check which is exist or not	     //
// Output: void 													 //
// Purpose: error print when path is not exist						 //
///////////////////////////////////////////////////////////////////////

void exception(char *path) {
	DIR *dirp;
	struct stat st;

	dirp = opendir(path);
	if (dirp == NULL) {
		if (lstat(path, &st) == -1) { // exception : path is not exist
			fprintf(stderr, "cannot access %s: No such file or directory\n", path);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////
// get_list															 //
// ================================================================= //
// Input: t_list** -> pointer of head, 								 //
// 		  DIR* -> directory stream for getting directory entry info  //
// Output: int // success 0 fail -1									 //
// Purpose: Make linked list which contain name of files in certain  //
// 			directory to print out 									 //
///////////////////////////////////////////////////////////////////////

int get_list(t_list **list, DIR *dirp) {
	t_list *temp = 0; // temporary node
	t_list *head = 0; // head
	struct dirent *dir; // directory entry

	while ((dir = readdir(dirp)) != NULL) { // read from dirent until all file is readed
		if (!aflag && dir->d_name[0] == '.') continue; // skip hidden file without aflag
		if (head == 0) { // initialize head
			head = (t_list *)malloc(sizeof(t_list));
			if (!head) // exception : bad allocation
				exit(1);
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
		strcpy(temp->path, dir->d_name);
		temp->next = 0;
		if (aflag && temp->path[0] == '.') { // hidden file, to dismiss '.' when comparing 
			for (size_t i = 0; i < strlen(temp->path); i++) // toupper and copy 
				temp->compare[i] = toupper(temp->path[i+1]); 		
		}
		else {
			for (size_t i = 0; i < strlen(temp->path); i++) // toupper and copy 
				temp->compare[i] = toupper(temp->path[i]); 
		}
	}
	if (head == 0)
		return -1;
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////
// sort_list														 //
// ================================================================= //
// Input: t_list** -> pointer of head								 //
// Output: void 													 //
// Purpose: Sort linked list(using selection sort) 					 //
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
				if (rflag == 0) { 
					if (strcmp(min, temp->next->compare) > 0) { // select when next name is smaller
						min = temp->next->compare;
						s_min = temp->next;
					}
				}
				else { // rflag 
					if (strcmp(min, temp->next->compare) < 0) { // select when next name is bigger
						min = temp->next->compare;
						s_min = temp->next;
					}
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

///////////////////////////////////////////////////////////////////////
// print_node														 //
// ================================================================= //
// Input: char* path -> full path of file							 //
//		  char* name -> name of file								 //
// Output: void 													 //
// Purpose: Print file information									 //
///////////////////////////////////////////////////////////////////////

void print_node(char* path, char* name) {
	struct stat st;
	if (lstat(path, &st) == -1) { // exception : path is not exist
		return;
	}
	if (lflag == 0) { // print the file name
		printf("%s\n", name); 
		return ;
	}

	// print the file information (-l option)
	int mode;
	struct tm *tm;
	const char *month[12] = {"Jan", "Fab", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; // month array
	int xflag = 0;

	mode = st.st_mode; // print file format
	if (S_ISREG(mode))	// regular file
		printf("-"); 
	else if (S_ISDIR(mode)) // directory
		printf("d"); 
	else if (S_ISLNK(mode)) // symbolic link
		printf("l");
	else if (S_ISBLK(mode)) // block special file
		printf("b"); 
	else if (S_ISCHR(mode)) // character special file
		printf("c");
	else if (S_ISFIFO(mode)) // fifo
		printf("p"); 
	else if (S_ISSOCK(mode)) // socket
		printf("s");
	
	mode %= (1<<10); // print file permission
	for (int i = 8; i >= 0; i--) {
		if (mode>>i == 1) { // check bit by bit
			printf("%c", "xwr"[i%3]); // permission
			if (i%3 == 0) xflag++;
			mode -= (1<<i);
		}
		else
			printf("-"); // permission x
	}

	if (nlink == 0) printf(" %ld", st.st_nlink); // print the number of hard links
	else printf(" %*ld", nlink, st.st_nlink); // print the number of hard links aligned version

	if (nname == 0 && ngroup == 0) printf(" %s %s", getpwuid(st.st_uid)->pw_name, getgrgid(st.st_gid)->gr_name); // print the author and group name
	else printf(" %-*s %-*s", nname, getpwuid(st.st_uid)->pw_name, ngroup, getgrgid(st.st_gid)->gr_name); // print the author and group name aligned version

	if (hflag == 0)
		printf("\t%5lu", st.st_size); // print the size of file
	else {
		size_t size = st.st_size;
		if (size >> 10 == 0) printf(" %4ld", size); // size < 1024 -> just print
		else { // size >= 1024 -> parsing
			int i;
			for (i = 0; i < 5; i++) { // one loop moves one prefix
				float temp = (float)size / 1024; 
				if (temp < 10) { // print as n.n form
					temp *= 10; 
					if (size % 1024) temp += 1; // if there's remain -> ceil operation
					printf(" %1d.%d%c", (int)temp/10, (int)temp%10, "KMGTP"[i]); // print n.nx
					break;
				}
				else if (temp < 1024) {
					if (size % 1024) temp += 1; // if remain -> ceil operation
					printf(" %3d%c", (int)temp, "KMGTP"[i]); // print nx
					break;
				}
				else 
					size /= 1024; // next prefix
			}
		}
	}

	time_t now = time(0);
	int this_year = localtime(&now)->tm_year;
	tm = localtime(&(st.st_mtime));
	printf(" %s %2d", month[tm->tm_mon], tm->tm_mday); // print the access time in certain format
	if (tm->tm_year != this_year) printf("  %d", tm->tm_year + 1900);
	else printf(" %02d:%02d", tm->tm_hour, tm->tm_min); 

	if (S_ISLNK(st.st_mode)) { // symbolic link file
		printf(" \033[96m\033[1m%s \033[0m ->", name); // set color as bold bright cyan and reset (can see in man console_codes(4))
		readlink(path, name, 254); // get the name linked by symlink
		xflag = 0;
		if (stat(name, &st) == -1) return; // exception : path is not exist
		if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) xflag++; // if the file is execute file
	}
	if (S_ISDIR(st.st_mode)) printf(" \033[34m\033[1m%s\033[0m\n", name); // print the dir name(bold blue)
	else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) printf(" \033[33m\033[1m%s\033[0m\n", name); // print the charcter/block special file name(bold brown(yellow))
	else if (S_ISFIFO(st.st_mode)) printf(" \033[33m\033[1m\033[2m%s\033[0m\n", name); // print the fifo name(bold half-bright brown)
	else if (S_ISSOCK(st.st_mode)) printf(" \033[35m\033[1m%s\033[0m\n", name); // print the socket name (bold magenta)
	else if (xflag) printf(" \033[92m\033[1m%s\033[0m\n", name); // print the execute file name(bold bright green)
	else printf(" %s\n", name); // print regular file
}

///////////////////////////////////////////////////////////////////////
// print_list														 //
// ================================================================= //
// Input: t_list** -> pointer of head								 //
// Output: void 													 //
// Purpose: Print linked list										 //
///////////////////////////////////////////////////////////////////////

void print_list(t_list **head, char *path) {
	t_list *temp = (*head)->next; // point to first node of sorted list to print
	size_t total = 0;
	struct stat st;
	char full_path[256];

	if (lflag > 0) { // count and print total size of dir
		while (temp) {
			if (path) { // make absolute path to get info
				strcpy(full_path, path);
				full_path[strlen(path)] = '/';
				strcpy(full_path + strlen(path) + 1, temp->path);
			}
			else // use relative path
				strcpy(full_path, temp->path);
			if (lstat(full_path, &st) == 0) {
				total += st.st_blocks/2; // count total block size
				if (nname < strlen(getpwuid(st.st_uid)->pw_name)) nname = strlen(getpwuid(st.st_uid)->pw_name); // set nname
				if (ngroup < strlen(getgrgid(st.st_gid)->gr_name)) ngroup = strlen(getgrgid(st.st_gid)->gr_name); // set ngroup
				int temp = st.st_nlink; 
				int numlen = 1; // count length of nlink
				while (temp) {
					temp /= 10;
					if (temp > 0) numlen++;
				}
				if (nlink < numlen) nlink = numlen; // set nlink
			}
			temp = temp->next;
		}
		if (hflag == 0)	printf("total %ld\n", total);  // print total size of dir
		else { // hflag
			if (total >> 10 == 0) printf("total %ldK\n", total); // size < 1024 -> just print with K
			else { // size >= 1024 -> parsing
				int i;
				for (i = 0; i < 4; i++) { // one loop moves one prefix
					float temp = (float)total / 1024;
					if (temp < 10) { // print as n.n form
						temp *= 10; 
						if (total % 1024) temp += 1; // if there's remain -> ceil operation
						printf("total %1d.%d%c\n", (int)temp/10, (int)temp%10, "MGTP"[i]); // print n.nx
						break;
					}
					else if (temp < 1024) {
						if (total % 1024) temp += 1; // if remain -> ceil operation
						printf("total %3d%c\n", (int)temp, "MGTP"[i]); // print nx
						break;
					}
					else 
						total /= 1024; // next prefix
				}
			}
		}
		temp = (*head)->next;
	}
	while (temp) { // print all the node
		if (path) {
			strcpy(full_path, path); // make absolute path to get info
			full_path[strlen(path)] = '/';
			strcpy(full_path + strlen(path) + 1, temp->path);
		}
		else // use relative path
			strcpy(full_path, temp->path);
		print_node(full_path, temp->path); // print each file
		memset(full_path, '\0', 256); // reset full_path
		temp = temp->next; 
	}
}