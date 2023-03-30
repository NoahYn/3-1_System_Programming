///////////////////////////////////////////////////////////////////////
// File Name : simple_ls.c 											 //
// Date : 2023/03/25	 											 //
// Os : Ubuntu 16.04 LTS 64bits 									 //
// Author : Sung Min Yoon 									 	 	 //
// Student ID : 2021202033											 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-1 (proxy server) 		 //
// Description : This file is source code for Assignment #1-1		 //
///////////////////////////////////////////////////////////////////////

#include <stdio.h> // for printf
#include <stdlib.h> // for malloc
#include <dirent.h> // for opendir, readdir, closedir
#include <ctype.h> // for toupper
#include <string.h> // for strlen

typedef struct s_list {
	char *path; // file name
	char *upper; // to dismiss alphabet lower & upper case
	struct s_list *next;
	struct s_list *prev; // will be used in sorting
} t_list;

void get_list(t_list **head, DIR *dirp);
void sort_n_print(t_list **head);

int main(int argc, char *argv[]) {
	DIR *dirp; // directory stream
	t_list *head; // head of list

	if (argc == 1) // default. open current directory ('.')
		dirp = opendir(".");
	else if (argc == 2) // one input. open certain directory 
		dirp = opendir(argv[1]);
	else { // exception : two or more input 
		fprintf(stderr, "%s: only one directory path can be processed\n", argv[0] + 2);
		exit(1);
	}
	if (dirp == NULL) { // exception : input path are not dir or not exist
		fprintf(stderr, "%s: cannot access '%s' : No such directory\n", argv[0] + 2, argv[1]);
		exit(1);
	}
	get_list(&head, dirp); // make list of dirent
	sort_n_print(&head); // sort and print
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
		if (dir->d_name[0] == '.') continue; // skip hidden file
		if (head == 0) { // initialize head
			head = (t_list *)malloc(sizeof(t_list));
			if (!head) // exception : bad allocation
				exit(1);
			head->path = 0; 
			head->upper = 0;
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
		temp->upper = (char *)malloc(sizeof(char)*strlen(temp->path)); // upper version of name to compare without u/lcase
		if (!temp->upper) // exception : bad allocatoin
			exit(1);
		for (size_t i = 0; i < strlen(temp->path); i++) // toupper and copy 
			temp->upper[i] = toupper(temp->path[i]); 
	}
}

///////////////////////////////////////////////////////////////////////
// sort_n_print														 //
// ================================================================= //
// Input: t_list** -> pointer of head								 //
// Output: void 													 //
// Purpose: Sorting linked list(using selection sort) and print out	 //
///////////////////////////////////////////////////////////////////////

void sort_n_print(t_list **head) {
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
			min = temp->upper;
			s_min = temp;
		}
		while (temp) {
			if (temp->next) { // comparing minimum with next node's name
				if (strcmp(min, temp->next->upper) > 0) { // select when next name is smaller
					min = temp->next->upper;
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
	s_temp = sorted->next; // point to first node of sorted list to print
	while (s_temp) { // print all the node
		printf("%s\n", s_temp->path); 
		s_temp = s_temp->next;
	}
}
