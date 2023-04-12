///////////////////////////////////////////////////////////////////////
// File Name : 2021202033_final_ls.c								 //
// Date : 2023/04/11	 											 //
// Os : Ubuntu 16.04 LTS 64bits 									 //
// Author : Sung Min Yoon 									 	 	 //
// Student ID : 2021202033											 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-3						 //
// Description : This file is source code for Assignment #1-3		 //
///////////////////////////////////////////////////////////////////////

#include <stdio.h> // for printf
#include <stdlib.h> // for malloc, free
#include <dirent.h> // for opendir, readdir, closedir
#include <ctype.h> // for toupper
#include <string.h> // for strlen
#include <unistd.h> // for option
#include <sys/stat.h> // for stat
#include <sys/types.h> 
#include <pwd.h> // for pwduid
#include <grp.h> // for groupgid
#include <time.h> // time
#include <fnmatch.h> // fnmatch

typedef struct s_list {
	char name[256]; // file name
	char fullpath[256]; // absolute path to get stat
	struct stat st; // file stat
	struct s_list *next;
	struct s_list *prev; // use when sorting
} t_list;

void fnmatch2argv(int *argc, char **argv[]); // convert wild card to names matched
void free_list(t_list **head); // free list
int get_list(t_list **head, DIR *dirp, char *path); // make list
int strlscmp(char *s1, char *s2); // compare by the order given by assignment
void sort_list(t_list **head); // sort list
void print_node(char* path, char *name); // print each file
void print_list(t_list **head); // print list

static int aflag = 0; // -a option
static int lflag = 0; // -l option
static int hflag = 0; // -h option
static int rflag = 0; // -r option
static int sflag = 0; // -S option

static int nlink = 0; // padding for -l nlink to align
static int nname = 0; // padding for -l username to align 
static int ngroup = 0; // padding for -l group to align 
static int nsize = 0; // padding for -l size to align

int main(int argc, char *argv[]) {
	DIR *dirp = 0; // directory stream
	t_list *head = 0; // head of list
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
				fprintf(stderr, "%s: invalid option.\n", argv[0]+2); 
				return 0;
		}
	}
	
	if (argc == optind) { // default. open current directory ('.')
		char cwd[256]; // buffer of current working directory 
		getcwd(cwd, 256); // get current working directory
		dirp = opendir(".");

		if (get_list(&head, dirp, cwd) == -1) return 0;	// make list of dirent
		sort_list(&head); // sort list
		
		if (lflag > 0) printf("Directory path: %s\n", cwd);	// print path of current working directory
		print_list(&head); // print list of current working directory
		
		free_list(&head);
		closedir(dirp);
	}

	else  {	// open certain paths
		t_list *curr = 0;
		t_list *temp = 0;
		int ls_cnt = 0; // count number of lists
		int file_entry = 0;
	
		fnmatch2argv(&argc, &argv);
		for (int i = 1; i < argc; i++) { // print error(first) and make list of files(second)
			if (argv[i][0] == '-') continue; // pass option
			
			if ((dirp = opendir(argv[i])) == NULL) { // path is not directory
				if (!(temp = (t_list*)malloc(sizeof(t_list)))) exit(1); // initialize temp
				if (lstat(argv[i], &(temp->st)) == -1) { // path is not exist
					fprintf(stderr, "cannot access '%s': No such file or directory\n", argv[i]); // error : path is not exist
					free(temp);
					continue;
				}
				strcpy(temp->name, argv[i]); 
				strcpy(temp->fullpath, argv[i]);
				temp->next = 0;

				if (head == 0) {	
					if (!(head = (t_list*)malloc(sizeof(t_list)))) exit(1); // exception : bad allocation
					head->next = temp; // link to head
					temp->prev = head;
				}
				else {
					curr->next = temp; // link so on
					temp->prev = curr;
				}
				curr = temp;
			}
			closedir(dirp);
		}

		if (head != 0) { // list is successfully created 
			sort_list(&head); // sort list of files 
			print_list(&head); // print list of files
			free_list(&head);
			file_entry = 1;
			ls_cnt++;
		}
	
		head = 0; curr = 0;
		for (int i = 1; i < argc; i++) {  // make list of directories(third)
			if (argv[i][0] == '-') continue; // pass option
			
			if ((dirp = opendir(argv[i])) == NULL) continue; // pass : flies and errors are already processed 
			if (head == 0) {
				if (!(head = (t_list*)malloc(sizeof(t_list)))) exit(1); // exception : bad allocation
				curr = head;
			}
			else
				curr = temp;
			if (!(temp = (t_list*)malloc(sizeof(t_list)))) exit(1); // exception : bad allocation
			curr->next = temp; // link
			temp->prev = curr; 
			temp->next = 0;
			strcpy(temp->name, argv[i]);
			closedir(dirp);
			ls_cnt++;
		}

		if (head == 0) return 0; // termination : directory isn't exist

		sflag *= -1; rflag *= -1; // dir should be sorted by name
		sort_list(&head); // sort dirs
		sflag *= -1; rflag *= -1; 

		curr = head->next; // node to traverse dir list
		t_list* dir_head = 0; // head of each direcory list
		while (curr) {
			if (lflag > 0 || ls_cnt > 1) {
				if (file_entry++)
					printf("\nDirectory path: %s\n", curr->name); // print Directory path
				else
					printf("Directory path: %s\n", curr->name);
			}
			
			dirp = opendir(curr->name);
			if (get_list(&dir_head, dirp, curr->name) == -1) { // directory is empty
				curr = curr->next; // go to next directory
				continue;
			}
			sort_list(&dir_head); // sort list
			print_list(&dir_head); // print list

			free_list(&dir_head);
			closedir(dirp);
			dir_head = 0;
			curr = curr->next;
		}
		free_list(&head);
	}
	return 0;
}

void fnmatch2argv(int *argc, char ***argv_src) {
	DIR *dirp = 0; // directory stream
	struct dirent *dir;
	t_list *head = (t_list*)malloc(sizeof(t_list)); // list to store file path
	t_list *curr = head; 
	char **argv = *argv_src; // source of argv
	char **argv_dst; // destination of argv
	int path_len; // strlen of path
	char *name; // name after path
	int cnt = 0; // size of new argv
	
	for (int i = 1; i < *argc; i++) { // find wildcard and convert to argv
		if (!fnmatch("*[?]*", argv[i], 0) || !fnmatch("*[*]*", argv[i], 0) || !fnmatch("*[[]*[]]*", argv[i], 0)) { // find wildcard (?, *, [seq])
			name = strrchr(argv[i], '/'); 
			if (name == 0) { // '/' is not founded
				dirp = opendir(".");
				while ((dir = readdir(dirp)) != NULL) { // read from dirent until all file is readed
					if (dir->d_name[0] == '.') continue; // skip hidden file without aflag
					if (!fnmatch(argv[i], dir->d_name, 0)) {
						t_list *temp = (t_list*)malloc(sizeof(t_list));
						strcpy(temp->name, dir->d_name);
						temp->next = 0;
						curr->next = temp;
						curr = temp;
						cnt++;
					}
				}
				cnt--; // except argv[i]
				closedir(dirp);
			}
			else {
				path_len = (int)(++name - argv[i]);
				strncpy(head->name, argv[i], path_len);
				dirp = opendir(head->name);
				while ((dir = readdir(dirp)) != NULL) { // read from dirent until all file is readed
					if (dir->d_name[0] == '.') continue; // skip hidden file without aflag
					if (!fnmatch(name, dir->d_name, 0)) {
						t_list *temp = (t_list*)malloc(sizeof(t_list));
						strcpy(temp->name, head->name);
						strcpy(temp->name + strlen(head->name), dir->d_name);
						temp->next = 0;
						curr->next = temp;
						curr = temp;
						cnt++;
					}
				}
				cnt--; // except argv[i]
				closedir(dirp);
			}
		}
		else {
			t_list *temp = (t_list*)malloc(sizeof(t_list));
			strcpy(temp->name, argv[i]);
			strcpy(temp->fullpath, argv[i]);
			temp->next = 0;
			curr->next = temp;
			curr = temp;
		}
	}

	*argc += cnt;
	argv_dst = (char **)malloc(sizeof(char *) * (*argc + 1));
	curr = head->next;
	int i = 1;
	while (curr) {
		argv_dst[i] = (char *)malloc(sizeof(char) * (strlen(curr->name) + 1));
		strcpy(argv_dst[i], curr->name);
		curr = curr->next;
		i++;
	}
	argv_dst[i] = 0;
	*argv_src = argv_dst;
}


///////////////////////////////////////////////////////////////////////
// free_list														 //
// ================================================================= //
// Input: t_list** -> pointer of head, 								 //
// Output: void														 //
// Purpose: free memory of linked list								 //
///////////////////////////////////////////////////////////////////////

void free_list(t_list **head) {
	t_list *temp;
	t_list *curr = *head; // node to traverse list 

	while (curr) {
		temp = curr->next;  
		free(curr);  
		curr = temp; 
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

int get_list(t_list **list, DIR *dirp, char *path) {
	t_list *temp = 0; // temporary node
	t_list *head = 0; // head
	struct dirent *dir; // directory entry

	while ((dir = readdir(dirp)) != NULL) { // read from dirent until all file is readed
		if (!aflag && dir->d_name[0] == '.') continue; // skip hidden file without aflag
		if (head == 0) { // initialize head
			if (!(head = (t_list *)malloc(sizeof(t_list)))) exit(1); // exception : bad allocation
			*list = head;
		}
		else
			head = temp;
		if (!(temp = (t_list *)malloc(sizeof(t_list)))) exit(1); // initialize temp
		head->next = temp; // link
		temp->prev = head;

		strcpy(temp->name, dir->d_name); // get file name
		strcpy(temp->fullpath, path); // make absolute path to get stat
		temp->fullpath[strlen(path)] = '/';
		strcpy(temp->fullpath + strlen(path) + 1, temp->name);
		
		lstat(temp->fullpath, &temp->st); // get stat
		temp->next = 0;
	}
	if (head == 0) // there's nothing to make list
		return -1; 
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////
// strlscmp															 //
// ================================================================= //
// Input: char *s1, *s2 -> strings to compare each other  			 //
// Output: int (difference between s1 and s2)						 //
// Purpose: compare two string to sort 								 //
///////////////////////////////////////////////////////////////////////

int strlscmp(char *s1, char *s2) {
	int i1 = 0; // index of s1
	int i2 = 0; // index of s2

	if (s1[i1] == '.') i1 += 1; // skip : '.' is just flag of hidden file
	if (s2[i2] == '.') i2 += 1;

	while (s1[i1] && s2[i2]) {
		if (s1[i1] == '\'' || s1[i1] == '\"') i1 += 1; // skip : ' and " are just flag of string 
		if (s2[i2] == '\'' || s2[i2] == '\"') i2 += 1; 
		if (s1[i1] == '\\') i1 += 1; // skip : \ is flag of escape sequence
		if (s2[i2] == '\\') i2 += 1; // skip : \ is flag of escape sequence

		if (toupper(s1[i1]) != toupper(s2[i2])) // compare between alphabet -> follow alphabet order / other character follow ascii order 
			return ((unsigned char)toupper(s1[i1]) - (unsigned char)toupper(s2[i2])); // compare with toupper
		i1++; i2++;
	}
	if (s1[i1] || s2[i2]) return ((unsigned char)s1[i1] - (unsigned char)s2[i2]); // two lengths are different
	
	// s1 and s2 are identical with toupper (using alphabet order)
	// we should compare without toupper(ex. ABCD and abcd -> ABCD first)
	i1 = 0; i2 = 0;

	if (s1[i1] == '.') i1 += 1; // skip : '.' is just flag of hidden file
	if (s2[i2] == '.') i2 += 1;

	while (s1[i1] && s2[i2]) {
		if (s1[i1] == '\'' || s1[i1] == '\"') i1 += 1; // skip : ' and " are just flag of string 
		if (s2[i2] == '\'' || s2[i2] == '\"') i2 += 1; 
		if (s1[i1] == '\\') i1 += 1; // skip : \ is flag of escape sequence
		if (s2[i2] == '\\') i2 += 1; // skip : \ is flag of escape sequence

		if (s1[i1] != s2[i2]) { // same alphabet -> ascii order : just compare
			return ((unsigned char)s1[i1] - (unsigned char)s2[i2]);
		}
		i1++; i2++;
	}
	
	return ((unsigned char)s1[i1] - (unsigned char)s2[i2]);
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
	char *min_path;
	ssize_t min_size;
	size_t size; // flag for using while condition (size of list)
	do {
		size = 0;
		temp = (*head)->next; // point to first component
		if (temp) { // setting for comparing
			if (sflag > 0) min_size = temp->st.st_size;
			min_path= temp->name;
			s_min = temp;
		}
		while (temp) {
			if (temp->next) { // comparing minimum with next node's name
				if (sflag > 0) { // sflag
					if (rflag > 0) { // rflag
						if (min_size > temp->next->st.st_size) { // select when next size is smaller
							min_size = temp->next->st.st_size; // get next size
							min_path = temp->next->name; // get next path
							s_min = temp->next; // get next node
						}
						else if (min_size == temp->next->st.st_size) { // if file size is same -> sort by file name
							if (strlscmp(min_path, temp->next->name) < 0) { // select when next name is bigger
								min_path = temp->next->name; // max_path
								s_min = temp->next; // s_max
							}
						}
					}
					else { // rflag == 0
						if (min_size < temp->next->st.st_size) { // select when next size is bigger
							min_size = temp->next->st.st_size; // max_size
							min_path = temp->next->name; // max_path
							s_min = temp->next; // s_max
						}
						else if (min_size == temp->next->st.st_size) { // if file size is same -> sort by file name
							if (strlscmp(min_path, temp->next->name) > 0) { // select when next name is smaller
								min_path = temp->next->name; // min_path
								s_min = temp->next; // s_min
							}
						}
					}
				}
				else { // sflag == 0, default
					if (rflag > 0) { // rflag  
						if (strlscmp(min_path, temp->next->name) < 0) { // select when next name is bigger
							min_path = temp->next->name; // max_path
							s_min = temp->next; // s_max
						}
						
					}
					else { // rflag == 0, default
						if (strlscmp(min_path, temp->next->name) > 0) { // select when next name is smaller
							min_path= temp->next->name; // min_path
							s_min = temp->next; // s_min
						}
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
		s_temp->next = s_min; // link to sorted list
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
	int mode; // file type and permission
	struct tm *tm; // time
	const char *month[12] = {"Jan", "Fab", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; // month array
	int xflag = 0; // flag whether the file is executable file or not

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
	
	mode %= (1<<9); // print file permission
	for (int i = 8; i >= 0; i--) {
		if (mode>>i == 1) { // check bit by bit
			if (i%3 == 0) {
				xflag++;
				if (i == 0 && st.st_mode & __S_ISVTX) printf("t"); // sticky bit -> t
				else if (st.st_mode & S_ISUID || st.st_mode & S_ISGID) printf("s"); // setuserid, setgroupid -> s
				else printf("x"); // no special bits -> x
			}
			else printf("%c", "xwr"[i%3]); // permission -> r,w,x
			mode -= (1<<i); // get next bit
		}
		else {
			if (i == 0 && st.st_mode & __S_ISVTX) printf("T"); // sticky bit -> T(no execute permission + t = T)
			else if (i%3 == 0 && (st.st_mode & S_ISUID || st.st_mode & S_ISGID)) printf("S"); // setuserid, setgroupid -> S(no execution permission)
			else printf("-"); // no permission
		}
	}

	if (nlink == 0) printf(" %ld", st.st_nlink); // print the number of hard links
	else printf(" %*ld", nlink, st.st_nlink); // print the number of hard links aligned version

	if (nname == 0 && ngroup == 0) printf(" %s %s", getpwuid(st.st_uid)->pw_name, getgrgid(st.st_gid)->gr_name); // print the author and group name
	else printf(" %-*s %-*s", nname, getpwuid(st.st_uid)->pw_name, ngroup, getgrgid(st.st_gid)->gr_name); // print the author and group name aligned version

	if (hflag == 0) {
		if (nsize == 0) printf(" %5lu", st.st_size); // print the size of file
		else printf(" %*lu", nsize, st.st_size); // print the size of file aligned version
	}
	else { // print size as human readable form
		size_t size = st.st_size;
		if (size >> 10 == 0) printf(" %4ld", size); // size < 1024 -> just print
		else { // size >= 1024 -> parsing
			int i;
			for (i = 0; i < 5; i++) { // next unit
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
				else size /= 1024; // next prefix
			}
		}
	}

	time_t now = time(0); // now
	int this_year = localtime(&now)->tm_year; // this_year
	tm = localtime(&(st.st_mtime)); // time when the file modified
	printf(" %s %2d", month[tm->tm_mon], tm->tm_mday); // print the time in certain format
	if (tm->tm_year != this_year) printf("  %d", tm->tm_year + 1900); // print year instead of hour when the file is modified not this year
	else printf(" %02d:%02d", tm->tm_hour, tm->tm_min); // print the time(hour : minute)

	if (S_ISLNK(st.st_mode)) { // symbolic link file
		printf(" \033[96m\033[1m%s\033[0m ->", name); // set color as bold bright cyan and reset (can see in man console_codes(4))
		memset(name, 0, 255);
		readlink(path, name, 255); // get the name linked by symlink
		if (stat(name, &st) == -1) return; // get stat of file linked by symlink
		xflag = 0;
		if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) xflag++; // if the file is execute file(for color)
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

void print_list(t_list **head) {
	t_list *temp = (*head)->next; // point to first node of sorted list to print
	size_t total = 0; // total block size of directory
	struct stat st; // stat
	char full_path[256]; // absolute path to get stat

	if (lflag > 0) { // count and print total size of dir
		while (temp) {
			st = temp->st;
			total += st.st_blocks/2; // count total block size
			if ((size_t)nname < strlen(getpwuid(st.st_uid)->pw_name)) nname = strlen(getpwuid(st.st_uid)->pw_name); // set nname
			if ((size_t)ngroup < strlen(getgrgid(st.st_gid)->gr_name)) ngroup = strlen(getgrgid(st.st_gid)->gr_name); // set ngroup
			int ntemp = st.st_nlink; // temp nlink 
			int numlen = 1; 
			while (ntemp) if ((ntemp /= 10) > 0) numlen++; // count length of nlink
			if (nlink < numlen) nlink = numlen; // get max nlink 
			ntemp = st.st_size; // temp nsize
			numlen = 1;
			while (ntemp) if ((ntemp /= 10) > 0) numlen++; // count length of nsize
			if (nsize < numlen) nsize = numlen; // get max nsize
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
					else total /= 1024; // next prefix
				}
			}
		}

		temp = (*head)->next; // rewind temp
	}
	while (temp) { // print all the node
		print_node(temp->fullpath, temp->name); // print each file
		memset(full_path, '\0', 256); // reset full_path
		temp = temp->next; 
	}
	nlink = 0; nname = 0; ngroup = 0; nsize = 0; // reset nlink, nname, ngroup
}