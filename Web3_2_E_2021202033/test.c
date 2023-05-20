#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 12345

typedef struct node {
    int data;
    struct node* next;
} node_t;

int main() {
    // 공유 메모리 생성
    int shmid = shmget(SHM_KEY, sizeof(node_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // 링크드 리스트 생성
    node_t* head = malloc(sizeof(node_t));
    head->data = 0;
    head->next = NULL;
    node_t* curr = head;
    for (int i = 1; i < 10; i++) {
        node_t* new_node = malloc(sizeof(node_t));
        new_node->data = i;
        new_node->next = NULL;
        curr->next = new_node;
        curr = new_node;
		printf("p%d ", curr->data);
    }
	printf("\n");

	curr = head;
    while (curr != NULL) {
        printf("t%d ", curr->data);
        curr = curr->next;
    }
	printf("\n");

    // 자식 프로세스 생성
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  // 자식 프로세스
        // 공유 메모리 접근
        node_t* shared_mem = shmat(shmid, NULL, 0);
        if (shared_mem == (void*) -1) {
            perror("shmat");
            exit(1);
        }

        // 링크드 리스트 출력
        node_t* curr = shared_mem;
        while (curr != NULL) {
            printf("%d ", curr->data);
            curr = curr->next;
        }
        printf("\n");

        // 공유 메모리 해제
        if (shmdt(shared_mem) == -1) {
            perror("shmdt");
            exit(1);
        }
    } else {  // 부모 프로세스
        // 공유 메모리 접근
        node_t* shared_mem = shmat(shmid, NULL, 0);
        if (shared_mem == (void*) -1) {
            perror("shmat");
            exit(1);
        }
  // 링크드 리스트 출력
        node_t* curr = shared_mem;
        while (curr != NULL) {
            printf("%d ", curr->data);
            curr = curr->next;
        }
        printf("\n");

        // 공유 메모리 해제
        if (shmdt(shared_mem) == -1) {
            perror("shmdt");
            exit(1);
        }
	}
}