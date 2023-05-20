#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define KEY_NUM 9527
#define MEM_SIZE 1024

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

int cnt;

void *doit1(void*);
void *doit2(void*);

int main(int argc, char **argv) {
	pthread_t tidA, tidB, tidC;
	pthread_create(&tidA, NULL, &doit1, NULL);
	pthread_create(&tidB, NULL, &doit1, NULL);
	pthread_create(&tidC, NULL, &doit2, NULL);

	pthread_join(tidA, NULL);
	pthread_join(tidB, NULL);
	pthread_join(tidC, NULL);

	return 0;
}

void *doit1(void *vptr) {
	int i, val, shm_id;
	void *shm_addr;

	if ((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1) {
		perror("shmget fail\n");
		return 0;
	}
	if ((shm_addr = shmat(shm_id, (void*)0, 0)) == (void*)-1) {
		perror("shmat fail\n");
		return 0;
	}

	for (i = 0; i < 10; i++) {
		pthread_mutex_lock(&counter_mutex);
		val = cnt;
		sleep(1);
		sprintf((char*)shm_addr, "[%d] %d", (int)pthread_self(), val);

		cnt = val+1;
		pthread_mutex_unlock(&counter_mutex);
		sleep(1);
	}return 0;
}

void *doit2(void *vptr) {
	int i, val, shm_id;
	void *shm_addr;
	char prev[32];

	if ((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1) {
		perror("shmget fail\n");
		return 0;
	}
	if ((shm_addr = shmat(shm_id, (void*)0, 0)) == (void*)-1) {
		perror("shmat fail\n");
		return 0;
	}

	memset((void*) prev, 0, 32);
	while (1) {
		if (strcmp(prev, (char*)shm_addr)) {
			printf("%s\n", (char*)shm_addr);
			strcpy(prev, (char *)shm_addr);
		}
		if (strstr((char*)shm_addr, " 19") != NULL) {
			printf("%s\n", (char*)shm_addr);
			break;
		}
	}
	if (shmctl(shm_id, IPC_RMID, 0) == -1) {
		perror ("shmctl fail\n");
	}
	return 0;
}