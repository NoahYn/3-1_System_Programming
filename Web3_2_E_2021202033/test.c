
#include <fcntl.h>
#include <stdio.h> // for printf
#include <string.h> // for strlen
#include <stdlib.h> // for malloc, free
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h> // for socket
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h> // for option
#include <dirent.h> // for opendir, readdir, closedir
#include <ctype.h> // for toupper
#include <sys/stat.h> // for stat
#include <pwd.h> // for pwduid
#include <grp.h> // for groupgid
#include <time.h> // time
#include <fnmatch.h> // fnmatch

int main() {
	int pid = fork();

	if (pid == 0) {
		//child
		sleep(1);
		exit(1);
	}
	else if (pid > 0) // parent
	{
//		sleep(1);
		printf("kill = %d\n", kill(pid, SIGTERM));
		int status;
		wait(&status);
		/* code */
	}
	
}