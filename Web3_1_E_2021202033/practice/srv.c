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
#define NCHLD 5
static char buf[BUFSIZE+1];
static pid_t pids[NCHLD];

pid_t child_make(int i, int socketfd, int addrlen);
void child_main(int i, int socketfd, int addrlen);

int main() {
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, addrlen, i, opt=1;
	char haddr[] = "127.0.0.1";

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Can't create socket.\n");
		return -1;
	}

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(buf, 0, sizeof(buf));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(haddr);
	server_addr.sin_port = htons(PORT);

	if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Can't bind local address.\n");
		return -1;
	}

	listen(socket_fd, 5);
	addrlen = sizeof(client_addr);

	for (i = 0; i < NCHLD; i++) {
		pids[i] = child_make(i, socket_fd, addrlen);
	}
	for (;;)
		pause();
	return 0;
}

pid_t child_make(int i, int socketfd, int addrlen) {
	pid_t pid;
	if ((pid = fork()) > 0)
		return(pid);

	child_main(i, socketfd, addrlen);
}

void child_main(int i, int socketfd, int addrlen) {
	int client_fd, len_out;
	char buf[BUFSIZE];
	socklen_t clilen;
	struct sockaddr *client_addr;

	client_addr = (struct sockaddr *)malloc(addrlen);
	printf("child %ld starting \n", (long)getpid());
	while (1) {
		clilen = sizeof(client_addr);
		client_fd = accept(socketfd, (struct sockaddr*)&client_addr, &clilen);
		if (client_fd < 0) {
			perror("Server : accept failed\n");
			exit(1);
		}
		int len;
		while((len = read(client_fd, buf, BUFSIZE)) > 0) {
			buf[len] = 0;
			printf("%s\n", buf);
			printf("child %d processing request\n", getpid());
			write(client_fd, buf, len);
		}
		close(client_fd);
	}
	close(socketfd);
}