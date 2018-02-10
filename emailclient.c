#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 1000

int PORT = 8080;
char* cur_user = NULL;
char* get_client_request(char* request) {
	char* client_request;
	if (strcmp(request, "Listusers") == 0) {
		client_request = (char*)"LSTU";
	} else if (strncmp(request, "Adduser", 7) == 0) {
		char* userid = request + 8;
		size_t dest_size = snprintf(NULL, 0, "ADDU %s", userid);
		client_request = (char*)malloc(dest_size + 1);
		snprintf(client_request, dest_size + 1, "ADDU %s", userid);
	} else if (strncmp(request, "SetUser", 7) == 0) {
		char* userid = request + 8;
		size_t dest_size = snprintf(NULL, 0, "USER %s", userid);
		client_request = (char*)malloc(dest_size + 1);
		snprintf(client_request, dest_size + 1, "USER %s", userid);	
		cur_user = strdup(userid);
	} else if (strcmp(request, "Read") == 0) {
		client_request = (char *)"READM";
	} else if (strcmp(request, "Delete") == 0) {
		client_request = (char *)"DELM";
	} else if (strncmp(request, "Send", 4) == 0) {
		printf("Type Message: ");
		char* message = (char*)malloc(SIZE * sizeof(int));
		fgets(message, SIZE, stdin);
		message[strlen(message) - 1] = '\0';
		char* receiver_id = request + 5;
		size_t dest_size = snprintf(NULL, 0, "SEND %s %s", receiver_id, message);
		client_request = (char*)malloc(dest_size + 1);
		snprintf(client_request, dest_size + 1, "SEND %s %s", receiver_id, message);	
	} else if (strcmp(request, "Done") == 0) {
		client_request = (char *)"DONEU";
		cur_user = NULL;
	} else if (strcmp(request, "Quit") == 0) {
		client_request = (char*)"QUIT";
	} else {
		client_request = (char*)"";
	}
	return client_request;
}

int main(int argc, char const *argv[])
{
	if (argc != 3) {
		printf("Format: ./emailclient <machine> <port_no>\n");
		exit(0);
	}

	PORT = atoi(argv[2]);
	int socket_desc;
	char server_response[SIZE];
	char* client_request = (char*)"response";

	// Create socket file descriptor.
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		fprintf(stderr, "Failed to create socket.");
		exit(1);
	}

	struct sockaddr_in server_address; 
	memset(&server_address, '0', sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	      
	if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
	    fprintf(stderr, "Invalid address.");
	    exit(1);
	}
	
	// Connect to the server.
	if (connect(socket_desc, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		fprintf(stderr, "Failed while connecting.");
		exit(1);
	}


	char* request = (char*)malloc(100 * sizeof(int));
	while (1) {
		if (cur_user != NULL) printf("Sub-Prompt-%s>", cur_user);
		else printf("Main-Prompt>");

		fgets(request, 100, stdin);
		request[strlen(request) - 1] = '\0';
		client_request = get_client_request(request);
		send(socket_desc, client_request, strlen(client_request), 0);

		if (strcmp(request, "Quit") == 0) break;

		int delim = read(socket_desc, server_response, SIZE);
		*(server_response + delim) = '\0';
		if (strcmp(server_response, "Done") != 0) printf("%s\n", server_response);
	}
	return 0;
}