#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct httpd_conf {
    int MaxChilds;
    int MaxIdleNum;
    int MinIdleNum;
    int StartServers;
    int MaxHistory;
};

int main() {
    struct httpd_conf conf;
    char buff[256];
    char *tok;

    // Open the httpd.conf file for reading
    FILE *fp = fopen("httpd.conf", "r");

    // Read each line of the file and parse the values
	while (fgets(buff, sizeof(buff), fp)) { // read each line 
		tok = strtok(buff, ": ");
		if (strcmp(tok, "MaxChilds") == 0) { // parse the values
			conf.MaxChilds = atoi(strtok(NULL, " \n\0"));
		} 
		else if (strcmp(tok, "MaxIdleNum") == 0) { // parse the values
			conf.MaxIdleNum =  atoi(strtok(NULL, " \n\0"));
		} else if (strcmp(tok, "MinIdleNum") == 0) { // parse the values
			conf.MinIdleNum =  atoi(strtok(NULL, " \n\0"));
		} else if (strcmp(tok, "StartServers") == 0) { // parse the values
			conf.StartServers =  atoi(strtok(NULL, " \n\0"));
		} else if (strcmp(tok, "MaxHistory") == 0) { // parse the values
			conf.MaxHistory =  atoi(strtok(NULL, " \n\0"));
		}
	}

    // Close the file
    fclose(fp);

    // Print the values to verify
    printf("MaxChilds: %d\n", conf.MaxChilds);
    printf("MaxIdleNum: %d\n", conf.MaxIdleNum);
    printf("MinIdleNum: %d\n", conf.MinIdleNum);
    printf("StartServers: %d\n", conf.StartServers);
    printf("MaxHistory: %d\n", conf.MaxHistory);

    return 0;
}