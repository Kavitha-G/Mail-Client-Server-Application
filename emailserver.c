#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define SIZE 1000

int PORT = 8080;
char* cur_user = NULL;
FILE* fp = NULL;
int cur_num_mails = 0;

char* get_user_list() {
	DIR *dir;
	char* reply;
	struct dirent *dir_entry;
	dir = opendir("MAILSERVER");

	reply = (char*)"";
	char* temp;

	if (dir) {
	    while ((dir_entry = readdir(dir)) != NULL) {
	    	if (strcmp(dir_entry->d_name, ".") == 0) continue;
	    	if (strcmp(dir_entry->d_name, "..") == 0) continue;
	    	
	    	char* userid = strdup(dir_entry->d_name);
	    	userid[strlen(userid) - 4] = '\0';
	    	temp = strdup(reply);
	    	size_t dest_size = snprintf(NULL, 0, "%s%s ", reply, userid);
	    	reply = (char*)malloc(dest_size + 1);
	    	snprintf(reply, dest_size + 1, "%s%s ", temp,  userid);
	    }
	    closedir(dir);
	}
	if (strcmp(reply, "") == 0) {
		reply = (char*)"No users";
		return reply;
	}
	reply[strlen(reply) - 1] = '\0';
	return reply;
}

char* get_filename(char* userid) {
	char* filename;
	size_t dest_size = snprintf(NULL, 0, "MAILSERVER/%s.txt", userid);
	filename = (char*)malloc(dest_size + 1);
	snprintf(filename, dest_size + 1, "MAILSERVER/%s.txt", userid);
	return filename;
}

char* add_user(char* userid) {
	char* reply;
	char* filename = get_filename(userid);
	FILE* fi = fopen(filename, "r");
	if (fi != NULL) {
		reply = (char*)"Userid already present";
	} else {
		fi = fopen(filename, "w");
		reply = (char*)"Added Userid";
	}
	fclose(fi);
	return reply;
}

char* set_cur_user(char* userid) {
	char* filename = get_filename(userid);
	char* reply;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		reply = (char*)"Userid doesn't exist";
		cur_user = NULL;
		return reply;
	}

	char* line = (char*)malloc(SIZE * sizeof(char));
	int num_mails = 0;
	while (fgets(line, SIZE, fp) != NULL) {
		line[strlen(line) - 1] = '\0';
		if (strcmp(line, "###") == 0) num_mails++;
	}
	cur_num_mails = num_mails;
	fclose(fp);

	size_t dest_size = snprintf(NULL, 0, "Userid %s exists, and has %d messages", userid, num_mails);
	reply = (char*)malloc(dest_size + 1);
	snprintf(reply, dest_size + 1, "Userid %s exists, and has %d messages", userid, num_mails);
	
	fp = fopen(filename, "a+");
	cur_user = strdup(userid);
	return reply;
}

char* read_next_message(FILE* fp) {
	char* line = (char*)malloc(SIZE * sizeof(char));
	char* message;
	message = (char*)"";
	while (fgets(line, SIZE, fp) != NULL) {
		line[strlen(line) - 1] = '\0';
		if (strcmp(line, "###") == 0) break;
		char* temp = strdup(message);
		size_t dest_size = snprintf(NULL, 0, "%s%s\n", temp, line);
		message = (char*)malloc(dest_size + 1);
		snprintf(message, dest_size + 1, "%s%s\n", temp, line);
	}
	message[strlen(message) - 1] = '\0';
	return message;
}

int file_is_empty(FILE* fi) {
	fseek (fi, 0, SEEK_END);
	int size = ftell(fi);
    if (size == 0) return 1;
    return 0;
}

char* read_mail() {
	char* reply;
	if (fp == NULL || cur_user == NULL) {
		reply = (char*)"No current user is set";
		return reply;

	}

	unsigned long position = ftell(fp);

	if (feof(fp) != 0 || file_is_empty(fp) != 0) {
		reply = (char*)"No More Mail";
		return reply;
	}

	fseek(fp, position, SEEK_SET);

	reply = read_next_message(fp);
	
	position = ftell(fp);
	fseek(fp, 0, SEEK_END);
	if (ftell(fp) != position) fseek(fp, position, SEEK_SET);
	else fseek(fp, 0, SEEK_SET);

	return reply;
}

char* delete_mail() {
	char* reply;
	if (fp == NULL || cur_user == NULL) {
		reply = (char*)"No current user is set";
		return reply;

	} 

	unsigned long position = ftell(fp);

	if (feof(fp) || file_is_empty(fp) != 0) {
		reply = (char*)"No More Mail";
		return reply;
	}
	
	fseek(fp, position, SEEK_SET);
	
	char* message_to_delete = read_next_message(fp);
	
	rewind(fp);

	FILE* new_fp;
	new_fp = fopen("MAILSERVER/TempSpool.txt", "w+");

	char* filename = get_filename(cur_user);
	
	int num_mails = cur_num_mails;
	while (num_mails--) {
		char* next_message = read_next_message(fp);
		if (strcmp(next_message, message_to_delete) == 0) {
			continue;
		}
		fprintf(new_fp, "%s\n###\n", next_message);
	}
	fclose(fp);
	fclose(new_fp);

	remove(filename);
	rename("MAILSERVER/TempSpool.txt", filename);
	cur_num_mails--;

	fp = fopen(filename, "a+");
	reply = (char *)"Message Deleted";
	
	fseek(fp, 0, SEEK_END);
	if (ftell(fp) == position) fseek(fp, 0, SEEK_SET);
	else fseek(fp, position, SEEK_SET);
	return reply;
}

int valid(char* userid) {
	char* filename = get_filename(userid);
	FILE* fin = fopen(filename, "r");
	if (fin == NULL) return 0;
	return 1;
}

char* send_message(char* userid, char* message) {
	char* reply;
	if (valid(userid) == 0) {
		reply = (char*)"User doesn't exist. Try sending to valid user.";
		return reply;
	} else if (strcmp(userid, cur_user) == 0) {
		reply = (char*)"Cannot send message to yourself";
		return reply;
	}
	char* filename = get_filename(userid);
	FILE* fout;
	fout = fopen(filename, "a+");
	time_t t = time(NULL);
    struct tm *tm = localtime(&t);
	fprintf(fout, "From: %s\nTo: %s\nDate: %sSubject: Hello\n%s\n###\n", cur_user, userid, asctime(tm), message);
	fclose(fout);
	reply = (char*)"Done";
	return reply;
}

char* done_cur_user() {
	char* reply;
	cur_user = NULL;
	if (fp != NULL) fclose(fp);
	fp = NULL;
	reply = (char*)"Done";
	return reply;
}

char* execute_command(char* command) {
	char* reply;
	if (strcmp(command, "LSTU") == 0) {
		reply = get_user_list();
	} else if (strncmp(command, "ADDU", 4) == 0) {
		char* userid = command + 5;
		reply = add_user(userid);
	} else if (strncmp(command, "USER", 4) == 0) {
		char* userid = command + 5;
		reply = set_cur_user(userid);
	} else if (strcmp(command, "READM") == 0) {
		reply = read_mail();
	} else if (strcmp(command, "DELM") == 0) {
		reply = delete_mail();
	} else if (strncmp(command, "SEND", 4) == 0) {
		char* userid = command + 5;
		int i = 0;
		while (userid[i] != ' ') i++;
		userid[i] = '\0';
		char* message = userid + i + 1;
		message[strlen(message) - 3] = '\0';
		reply = send_message(userid, message);
	} else if (strcmp(command, "DONEU") == 0) {
		reply = done_cur_user();
	} else {
		reply = (char*)"Invalid.";
	}
	return reply;
}

void setup_server_directory() {
	struct stat st = {0};
	if (stat("MAILSERVER", &st) == -1) {
	    mkdir("MAILSERVER", 0700);
	} else {
		DIR *dir;
		struct dirent *dir_entry;
		dir = opendir("MAILSERVER");
	
		if (dir) {
		    while ((dir_entry = readdir(dir)) != NULL) {
		    	if (strcmp(dir_entry->d_name, ".") == 0) continue;
		    	if (strcmp(dir_entry->d_name, "..") == 0) continue;
		 		char* filename = get_filename(dir_entry->d_name);
		 		filename[strlen(filename) - 4] = '\0';
		 		remove(filename);   	
		    }
		    closedir(dir);
		}
	}
}

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		printf("Format: ./emailserver <port_no>\n");
		exit(0);
	}

	PORT = atoi(argv[1]);
	int socket_desc, sock;
	char client_request[SIZE];
	char* server_response = (char*)"response";

	// Create socket file descriptor.
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		fprintf(stderr, "Failed to create socket.");
		exit(1);
	}

	struct sockaddr_in socket_address; 
	socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(PORT);

	// Binding the socket to the address and port number.
	if (bind(socket_desc, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0) {
		fprintf(stderr, "Failed to bind.");
		exit(1);	
	}

	// Listening on the socket waiting for a client.
	if (listen(socket_desc, 10)) { // TODO: check this backlog value.
		fprintf(stderr, "Failed while listening.");
		exit(1);
	}

	// Accept the connection.
	int addrlen = sizeof(socket_address);
	if ((sock = accept(socket_desc, (struct sockaddr *)&socket_address, (socklen_t*)&addrlen)) < 0) {
		fprintf(stderr, "Failed while accepting.");
		exit(1);	
	}

	// Setting up server directory.
	setup_server_directory();

	while (1) {
		int delim = read(sock, client_request, SIZE);
		*(client_request + delim) = '\0';
		
		if (strcmp(client_request, "QUIT") == 0) break;
		else server_response = execute_command(client_request);

		send(sock, server_response, strlen(server_response), 0);	

	}
	return 0;
}