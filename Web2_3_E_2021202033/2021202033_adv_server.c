///////////////////////////////////////////////////////////////////////
// File Name : 2021202033_adv_server.c							 	 //
// Date : 2023/05/10	 											 //
// Os : Ubuntu 16.04 LTS 64bits 									 //
// Author : Sung Min Yoon 									 	 	 //
// Student ID : 2021202033											 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-3						 //
// Description : This file is source code for Assignment #2-3		 //
///////////////////////////////////////////////////////////////////////

#define _GNU_SOURCE // FNM_CASEFOLD option

#include <fcntl.h>
#include <stdio.h> // for printf
#include <string.h> // for strlen
#include <stdlib.h> // for malloc, free
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h> // for socket
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <signal.h>
#include <unistd.h> // for option
#include <dirent.h> // for opendir, readdir, closedir
#include <ctype.h> // for toupper
#include <sys/stat.h> // for stat
#include <pwd.h> // for pwduid
#include <grp.h> // for groupgid
#include <time.h> // time
#include <fnmatch.h> // fnmatch

#define URL_LEN 256
#define BUFSIZE 1024
#define PORT 40000

typedef struct s_list {
	char name[256]; // file name
	char fullpath[256]; // absolute path to get stat
	struct stat st; // file stat
	struct s_list *next;
	struct s_list *prev; // use when sorting
} t_list;

typedef struct s_client { // information of client
	int No; // N-th connected client
	int pid; // pid of response server
	int port; // port number
	char IP[20]; // IP address
	char time[50]; // time when client and server connected
} t_client;

void fnmatch2argv(int *argc, char **argv[]); // convert wild card to names matched
void free_list(t_list **head); // free list
int get_list(t_list **head, DIR *dirp, char *path); // make list
int strlscmp(char *s1, char *s2); // compare by the order given by assignment
void sort_list(t_list **head); // sort list
void print_node(char* path, char *name); // print each file
void print_list(t_list **head); // print list

static int aflag = 0; // -a option
static int cli_sd; 
static unsigned int cwd_len = 0;
static char response_message[BUFSIZE*BUFSIZE*BUFSIZE] = {0,};
static char content_type[20] = {0,};
static int No = 0; // n-th connected client
struct s_client cli_history[10]; // recent client connection history
static unsigned int alarm_flag = 0;

///////////////////////////////////////////////////////////////////////
// sigHandler														 //
// ================================================================= //
// Input: int -> signal number		 								 //
// Output: void														 //
// Purpose: handling signal											 //
///////////////////////////////////////////////////////////////////////

void sigHandler(int sig) {
	if (sig == SIGALRM) { // timer is over!
		puts("========= Connection History ==================================="); // print recently connected client list
		printf("Number of request(s) : %d\n", No); // print number of client connected
		printf("No.\tIP\t\tPID\tPORT\tTIME\n");
		for (int i = 10; i >= 1; i--) { // print in recent time order
			if (cli_history[(No+i)%10].No == 0) continue;
			printf("%d\t%s\t%d\t%d\t%s", cli_history[(No+i)%10].No, cli_history[(No+i)%10].IP, cli_history[(No+i)%10].pid, cli_history[(No+i)%10].port, cli_history[(No+i)%10].time);
		}
		printf("\n");
		alarm_flag = 1; // loop flag == 1 until timer is over
	}
	else if (sig == SIGCHLD) { // child dies
		int stat;
		wait(&stat); // get termination status from child that is number n-th connected 
		stat >>= 8; // make bit lower
		puts("====== Disconnected Client ======"); // print information of disconneted client
		printf("IP : %s\nPort : %d\n", cli_history[stat%10].IP, cli_history[stat%10].port);
		puts("=================================\n");	
	}
}

int main() {
	struct sockaddr_in srv_addr, cli_addr; // address of server and client
	int sd; // socket descripter

	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) { // open socket
		perror("Server : Can't open stream socket\n");
		exit(1);
	}	

	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // to prevent bind err
	setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)); // to keep connection alive until download completed 

	memset(&srv_addr, 0, sizeof(srv_addr)); // initailize
	srv_addr.sin_family = AF_INET; // INET
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // make host address to network address
	srv_addr.sin_port = htons(PORT); // port

	if (bind(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr))<0) { // bind server and socket
		perror("Server : Can't bind local address\n");
		exit(1);
	}

	listen(sd, 5); // listen client
	signal(SIGALRM, sigHandler); // call signal handler to ready
	signal(SIGCHLD, sigHandler);
	while (1) {
		alarm(10); // set signal timer 10 seconds
		while (!alarm_flag) {
			pid_t pid;
			struct in_addr inet_cli_addr;
			char buf[BUFSIZE] = {0,}; // buffer to read request 
			char url[URL_LEN] = {0,}; // url string
			char cwd[URL_LEN] = {0,}; // buffer of current working directory 
			getcwd(cwd, URL_LEN); // get current working directory
			cwd_len = strlen(cwd); // length of current directory
			char response_header[BUFSIZE] = {0,}; 
			char method[20] = {0,}; // method from request
			char *tok = NULL; 
			int status = 200; // response status
			unsigned int len; 
			size_t response_len = 0;

			len = sizeof(cli_addr);
			cli_sd = accept(sd, (struct sockaddr*)&cli_addr, &len); // accept client
			if (cli_sd < 0) { // accept error
				perror("Server : accept failed\n");
				exit(1);
			}

			FILE* fs = fopen("accessible.usr", "r"); // open accessible.usr file
			int access = 0; // accessible flag
			inet_cli_addr.s_addr = cli_addr.sin_addr.s_addr; // cli inet addr initialize
				
			while(fgets(buf, BUFSIZE, fs)) { // read one line
				buf[strcspn(buf, "\n")] = '\0'; // replace \n to \0 if exist
				if (fnmatch(buf, inet_ntoa(inet_cli_addr), 0) == 0) { // match accessible or not
					access = 1;
					break;
				}
			}

			if (access == 0) { // client has no access
				strcpy(content_type, "text/html"); // html file
				sprintf(response_message, // no permission message
					"<h1>Access denied!</h1>"
					"<h2>Your IP : %s</h2>"
					"You have no permission to access this web server<br/>"
					"HTTP 403.6 - Forbidden: IP address reject", inet_ntoa(inet_cli_addr));

				sprintf(response_header, // response header
					"HTTP/1.1 403.6 OK\r\n"
					"Server:2023 simple web server\r\n"
					"Connection: keep-alive\r\n"
					"Content-type: %s\r\n"
					"Content-length:%lu\r\n\r\n", content_type, strlen(response_message));

				write(cli_sd, response_header, strlen(response_header)); // write header
				write(cli_sd, response_message, strlen(response_message));	// write reponse
				memset(response_message, 0, sizeof(response_message)); // reset
									
				close(cli_sd);
			}
			else { // clinet has access right. 				
				read(cli_sd, buf, BUFSIZE); // read request from client
				tok = strtok(buf, " ");
				strcpy(method, tok); // tok method
				if (strcmp(method, "GET") == 0) {
					tok = strtok(0, " "); // tok url
					if (strstr(tok, "favicon.ico")) continue; // skip favicon
					strcpy(url, tok); 
					if (strcmp(url, "/") == 0) { // root dir
						aflag = 0;
					}
					else { // other
						aflag = 1;
					}
					strcat(cwd, url); 
				}
				puts("========= New Client ============"); // connected message
				printf("IP : %s\nPort : %d\n", inet_ntoa(inet_cli_addr), cli_addr.sin_port); // clinet address
				puts("=================================\n");

				++No;
				pid = fork();
				if (pid > 0) { // parent process
				// make connection history 
					time_t t; 
					time(&t); // get connected time
					cli_history[No%10].No = No;
					strcpy(cli_history[No%10].IP, inet_ntoa(inet_cli_addr));
					strcpy(cli_history[No%10].time, ctime(&t)); // store time in string format 
					cli_history[No%10].pid = pid;
					cli_history[No%10].port = cli_addr.sin_port;
					close(cli_sd);
				}
 				else if (pid == 0) { // child process
					t_list *head = 0; // head of list
					DIR *dirp;
					int fd = 0;

					if ((dirp = opendir(cwd)) == NULL) { // not dir
						t_list *temp = 0;
						if (!(temp = (t_list*)malloc(sizeof(t_list)))) exit(1); // initialize temp
						if (lstat(cwd, &(temp->st)) == -1) { // 404 not found
							status = 404;
							strcpy(content_type, "text/html"); // directory html file 
							sprintf(response_message, 
								"<h1>Not Found</h1>"
								"The request URL %s was not found on this server<br/>"
								"HTTP %d - Not Page Found", cwd, status);
							response_len = strlen(response_message);
						}
						else { // file
							fd = open(cwd, O_RDONLY); // open file
							if (fnmatch("*.jpeg", cwd, FNM_CASEFOLD) == 0 || fnmatch("*.jpg", cwd, FNM_CASEFOLD) == 0 || fnmatch("*.png", cwd, FNM_CASEFOLD) == 0) { // image process
								strcpy(content_type, "image/*"); // image file
							}
							else {
								strcpy(content_type, "text/plain"); // source code or text file
							}
							response_len = temp->st.st_size; // get size of file
						}
						free(temp);
					}
					else {
						strcpy(content_type, "text/html"); // directory html file
						if (get_list(&head, dirp, cwd) == -1) return 0;	// make list of dirent
						if (aflag == 0) { // root
							sprintf(response_message, // Welcome
								"<h1>Welcome to System Programming Http</h1>"
								"<b>Directory path : %s <br/></b> ", cwd);
						}
						else {
							sprintf(response_message, 
								"<h1>System Programming Http</h1>"
								"<b>Directory path : %s <br/></b> ", cwd);			
						}
						sort_list(&head); // sort list
						print_list(&head); // print list of current working directory
						free_list(&head); // free
						response_len = strlen(response_message);
					}
					closedir(dirp); // close
					sprintf(response_header, // response header
						"HTTP/1.1 %d OK\r\n"
						"Server:2023 simple web server\r\n"
						"Connection: keep-alive\r\n"
						"Content-type: %s\r\n"
						"Content-length:%lu\r\n\r\n", status, content_type, response_len);

					write(cli_sd, response_header, strlen(response_header)); // write header
					if (fd) { // file
						int num_read;
						while((num_read = read(fd, response_message, BUFSIZE)) > 0) {
							write(cli_sd, response_message, num_read);
						}
					}
					else
						write(cli_sd, response_message, response_len);	// write reponse
					memset(response_message, 0, sizeof(response_message)); // reset
									
					close(fd);
					close(cli_sd);
					exit(No);
				}
			}
		}
		alarm_flag = 0; // reset the flag
	}
	return 0;
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

	while (curr) { // free node by node from head to tail
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
//		  path -> path to earn stat									 //
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

		strcpy(temp->name, dir->d_name);
		strcpy(temp->fullpath, path);

		if (temp->fullpath[strlen(temp->fullpath)-1] != '/')
			strcat(temp->fullpath, "/");
		strcat(temp->fullpath, temp->name);

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
	size_t size; // flag for using while condition (size of list)
	do {
		size = 0;
		temp = (*head)->next; // point to first component
		if (temp) { // setting for comparing
			min_path= temp->name;
			s_min = temp;
		}
		while (temp) {
			if (temp->next) { // comparing minimum with next node's name
				if (strlscmp(min_path, temp->next->name) > 0) { // select when next name is smaller
					min_path= temp->next->name; // min_path
					s_min = temp->next; // s_min
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

	// print the file information (-l option)
	int mode; // file type and permission
	struct tm *tm; // time
	const char *month[12] = {"Jan", "Fab", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; // month array
	int xflag = 0; // flag whether the file is executable file or not
	char format;
	char buf[BUFSIZE];

	if (S_ISDIR(st.st_mode)) sprintf(buf, "<tr style=\"color:blue\""); // print as blue
	else if (S_ISLNK(st.st_mode)) sprintf(buf, "<tr style=\"color:green\""); // print as green
	else sprintf(buf, "<tr style=\"color:red\""); // red
	strcat(response_message, buf);

	sprintf(buf, "<tr><td> <a href=\"%s\">%s</a> </td>", path+cwd_len, name);
	strcat(response_message, buf);

	mode = st.st_mode; // print file format
	if (S_ISREG(mode))	// regular file
		format = '-'; 
	else if (S_ISDIR(mode)) // directory
		format = 'd'; 
	else if (S_ISLNK(mode)) // symbolic link
		format = 'l';
	else if (S_ISBLK(mode)) // block special file
		format = 'b'; 
	else if (S_ISCHR(mode)) // character special file
		format = 'c';
	else if (S_ISFIFO(mode)) // fifo
		format = 'p'; 
	else if (S_ISSOCK(mode)) // socket
		format = 's';
	sprintf(buf, "<td>%c", format); // print Permission
	strcat(response_message, buf);
	
	mode %= (1<<9); // print file permission
	for (int i = 8; i >= 0; i--) {
		if (mode>>i == 1) { // check bit by bit
			if (i%3 == 0) {
				xflag++;
				if (i == 0 && st.st_mode & __S_ISVTX) sprintf(buf, "t"); // sticky bit -> t
				else if (st.st_mode & S_ISUID || st.st_mode & S_ISGID) sprintf(buf, "s"); // setuserid, setgroupid -> s
				else sprintf(buf, "x"); // no special bits -> x
			}
			else sprintf(buf, "%c", "xwr"[i%3]); // permission -> r,w,x
			mode -= (1<<i); // get next bit
		}
		else {
			if (i == 0 && st.st_mode & __S_ISVTX) sprintf(buf, "T"); // sticky bit -> T(no execute permission + t = T)
			else if (i%3 == 0 && (st.st_mode & S_ISUID || st.st_mode & S_ISGID)) sprintf(buf, "S"); // setuserid, setgroupid -> S(no execution permission)
			else sprintf(buf, "-"); // no permission
		}
		strcat(response_message, buf);
	}
	sprintf(buf, "</td><td>%ld", st.st_nlink); // print the number of hard links
	strcat(response_message, buf);

	sprintf(buf, "</td><td>%s", getpwuid(st.st_uid)->pw_name); // print the author
	strcat(response_message, buf);

	sprintf(buf, "</td><td>%s", getgrgid(st.st_gid)->gr_name); // print the group name
	strcat(response_message, buf);

	sprintf(buf, "</td><td>%lu", st.st_size); // print the size of file (default)
	strcat(response_message, buf);

	time_t now = time(0); // now
	int this_year = localtime(&now)->tm_year; // this_year
	tm = localtime(&(st.st_mtime)); // time when the file modified
	sprintf(buf, "</fd><td>%s %2d", month[tm->tm_mon], tm->tm_mday); // print the time in certain format
	strcat(response_message, buf);
	if (tm->tm_year != this_year) sprintf(buf, " %d", tm->tm_year + 1900); // print year instead of hour when the file is modified not this year
	else sprintf(buf, " %02d:%02d", tm->tm_hour, tm->tm_min); // print the time(hour : minute)
	strcat(response_message, buf);

	sprintf(buf, "</td></tr>");
	strcat(response_message, buf); 
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
	char buf[BUFSIZE] = {0, };

	while (temp) {
		st = temp->st;
		total += st.st_blocks/2; // count total block size
		temp = temp->next;
	}

	sprintf(buf, "<b>total %ld<br/></b>", total);  // print total size of dir
	strcat(response_message, buf);

	sprintf(buf, "<table border=\"1\"> <tr><b> <th>Name</th> <th>Permission</th> <th>Link</th> <th>Owner</th> <th>Group</th> <th>Size</th> <th>Last Modified</th> </b></tr>"); // table header l version
	strcat(response_message, buf);

	temp = (*head)->next; // rewind temp
	while (temp) { // print all the node
		print_node(temp->fullpath, temp->name); // print each file
		memset(full_path, '\0', 256); // reset full_path
		temp = temp->next; 
	}
	sprintf(buf, "</table><br/>");
	strcat(response_message, buf);
}