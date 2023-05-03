#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define URL_LEN 256
#define BUFSIZE 1024
#define PORT 40000

int main() {
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, client_fd;
	int len, len_out;
	int opt = 1;

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server : can't open stream socket\n");
		exit(1);
	}

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server : Can't bind local address\n");
		exit(1);
	}

	listen(socket_fd, 5);
		char buf[BUFSIZE] = {0, };
	while (1) {
		struct in_addr inet_client_address;
		char response_header[BUFSIZE] = {0,};
		char response_message[BUFSIZE] = {0, };
		char url[URL_LEN] = {0,};
		char method[20] = {0,};
		char *tok = NULL;

		len = sizeof(client_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
		if (client_fd < 0) {
			printf("Server : accept failed\n");
			exit(1);
		}
		inet_client_address.s_addr = client_addr.sin_addr.s_addr;
		printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		read(client_fd, buf, BUFSIZE);
		puts("====================================");
		printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		puts(buf);
		puts("====================================");

		tok = strtok(buf, " ");
		strcpy(method, tok);
		if (strcmp(method, "GET") == 0) {
			tok = strtok(0, " ");
			strcpy(url, tok);
		}
		sprintf(response_message, 
			"<h1>RESPONSE</h1><br>"
			"Hello %s:%d<br>"
			"%s", inet_ntoa(inet_client_address), client_addr.sin_port, url);
		sprintf(response_header,
			"HTTP/1.0 200 OK\r\n"
			"Server:2023 simple web server\r\n"
			"Content-length:%lu\r\n"
			"Content-type:text/html\r\n\r\n", strlen(response_message));
			
			write(client_fd, response_header, strlen(response_header));
			write(client_fd, response_message, strlen(response_message));

			sprintf(response_message, 
			"<h1>second</h1><br>"
			"Hello %s:%d<br>"
			"%s", inet_ntoa(inet_client_address), client_addr.sin_port, url);

		sprintf(response_header,
			"HTTP/1.0 200 OK\r\n"
			"Server:2023 simple web server\r\n"
			"Content-length:%lu\r\n"
			"Content-type:text/html\r\n\r\n", strlen(response_message));

			write(client_fd, response_header, strlen(response_header));
			write(client_fd, response_message, strlen(response_message));

			printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			close(client_fd);
	}
}