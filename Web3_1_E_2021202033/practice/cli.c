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

int main() {
	int socket_fd, len;
	struct sockaddr_in server_addr;
	char haddr[] = "127.0.0.1";
	char buf[BUFSIZE];

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Can't create socket.\n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(haddr);
	server_addr.sin_port = htons(PORT);

	if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Can't connect.\n");
		return -1;
	}

	write(STDOUT_FILENO, "> ", 2);
	while((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		if (write(socket_fd, buf, strlen(buf)) > 0) {
			if ((len = read(socket_fd, buf, sizeof(buf))) >0) {
				write(STDOUT_FILENO, buf, len);
				memset(buf, 0, sizeof(buf));
			}
		}
		write(STDOUT_FILENO, "> ", 2);
	}
	close(socket_fd);
	return 0;
}