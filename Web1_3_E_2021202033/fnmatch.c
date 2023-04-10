#include <stdio.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <string.h>
#include <ctype.h>

int strlscmp(char *s1, char *s2) {
	int i1 = 0;
	int i2 = 0;

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
	if (s1[i1] || s2[i2]) return ((unsigned char)s1[i1] - (unsigned char)s2[i2]); // length is different
	
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

int main(int argc, char **argv) {
/*
	if (argc < 3) {
		printf("argc < 3 \n");
		return 0;
	}
	printf("%s and %s is ", argv[2], argv[1]);
	if (!fnmatch(argv[2], argv[1], 0))
		printf("matching\n");
	else
		printf("not matching\n");
	return 0;
*/
}

			//if (!fnmatch(argv[i], cwd, FNM_PATHNAME)) {
			//	strcpy(argv[i], ".");
			//}

			// ./spls ~ '*' 순서가 *먼저나오는 이유
			// 
/*
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
*/
int get_list(t_list **list, DIR *dirp, char *path) {
	t_list *temp = 0; // temporary node
	t_list *head = 0; // head
	struct dirent *dir; // directory entry

	while ((dir = readdir(dirp)) != NULL) { // read from dirent until all file is readed
		if (!aflag && dir->d_name[0] == '.') continue; // skip hidden file without aflag
		if (head == 0) { // initialize head
			head = (t_list *)malloc(sizeof(t_list));
			if (!head) // exception : bad allocation
				exit(1);
			*list = head;
		}
		else
			head = temp;
		temp = (t_list *)malloc(sizeof(t_list)); // initialize temp
		if (!temp) // exception : bad allocation
			exit(1);
		head->next = temp;
		temp->prev = head;

		strcpy(temp->path, dir->d_name); // get file name
		strcpy(temp->fullpath, path); // make absolute path to get stat
		temp->fullpath[strlen(path)] = '/';
		strcpy(temp->fullpath + strlen(path) + 1, temp->path);
		
		lstat(temp->fullpath, &temp->st); // get stat
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
*/