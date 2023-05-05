#include <signal.h> 
#include <sys/types.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

//volatile sig_atomic_t alarm_flag = 0;
unsigned int alarm_flag = 0;

void signalHandler(int sig)
{
	if (sig== SIGINT)
	{
		printf(" signal: SIGINT \n"); 
		exit(0);
	}	
	if (sig== SIGTSTP)
	{
		printf(" signal: SIGTSTP \n"); 
		exit(0);
	}
	if (sig == SIGCHLD)
	{
		printf("The child process is dead. \n");
		exit(0);
	}
	if (sig == SIGALRM) {
		alarm_flag = 1;
	}
}


int main() {
	signal(SIGCHLD, signalHandler); 
	printf("input Ctrl+C or Ctrl+Z \n\n");

	int status; 
	pid_t pid;
	
	//if ((pid = fork()) == 0) //child process
	//{	
	//	signal(SIGINT, signalHandler); 
	//	signal(SIGTSTP, signalHandler); 
	//	while (1);
	//}
	//else //parent process
	//{
	//	signal(SIGINT, SIG_IGN);
	//	signal(SIGTSTP, SIG_IGN);
	//	waitpid(-1, &status, WNOHANG);
	//	printf("After returning the resource, the parent process also dies. \n");
	//}
	signal(SIGALRM, signalHandler);
	while(1) {

	alarm(3);
	while(!alarm_flag) {
		printf("a");
	}
	alarm_flag = 0;
	printf("b\nb\nb\n");
	break;
	}
}