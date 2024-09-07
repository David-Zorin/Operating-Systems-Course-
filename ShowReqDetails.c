#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define FAILURE -1
#define FALSE 0
#define TRUE 1
#define PATH_LEN 40
#define IN_REQUEST 1
#define IN_APPROVED 2
#define IN_UNAPPROVED 3
#define BUFF_SIZE 256

//Concatanate given number of strings
void cat(char* string[], int strings_num) {
	for (int i = 1; i < strings_num; i++) {
		strcat(string[0], string[i]);
	}
}

//Creates the path for a user's file
void create_path(char path[], char id[]) {
	char* string[] = { path, id, ".txt" };
	cat(string, 3);
}

//Checks if a user has a request in a given path with his given id
int has_req_in(char id[], char path[]) {
	create_path(path, id); //Create the path for the request file
	int fd = open(path, O_RDONLY); //Try opening a request file for this user
	if (fd == FAILURE) {
		if (errno == ENOENT) { //If the file does not exist
			return FALSE;
		}
		else { //If couldn't open the file for any other reason
			fprintf(stderr, "Cannot check if the user has a request, while trying to open %s.txt: ", id);
			perror(NULL);
			exit(FAILURE);
		}
	}
	close(fd);
	return TRUE; //The file exists, therefore his user has a request
}

//Checks if a user has a request with his given id, returns a number that represents the directory
int has_request(char id[]) {
	char path_request[PATH_LEN] = "Loan/Request/";
	if (has_req_in(id, path_request)) { return IN_REQUEST; }
	char path_approved[PATH_LEN] = "Loan/Result/Approved/";
	if (has_req_in(id, path_approved)) { return IN_APPROVED; }
	char path_unapproved[PATH_LEN] = "Loan/Result/Unapproved/";
	if (has_req_in(id, path_unapproved)) { return IN_UNAPPROVED; }
	return FALSE; //Does not have a request
}

//Reads a file into a buffer
int read_file(int fd, char path[], char buffer[], int buff_size) {
	int rbytes = read(fd, buffer, buff_size);
	if (rbytes == FAILURE) {
		fprintf(stderr, "Cannot read from %s", path);
		perror(NULL);
		exit(FAILURE);
	}
	return rbytes;
}

//Opens a file, prints an error if occurs
int open_file(char path[], int flags, int mode) {
	int fd = open(path, flags, mode);
	if (fd == FAILURE) {
		fprintf(stderr, "Cannot open file %s: ", path);
		perror(NULL);
		exit(FAILURE);
	}
	return fd;
}

//Reads the details from a request file without its status, and null-terminating it
void read_details(int fd, char path[], char buff[]) {
	int rbytes = read_file(fd, path, buff, BUFF_SIZE); //Read all form
	buff[rbytes] = '\0'; //Null-terminate the end to turn it into a string
	char* status = strstr(buff, "\nStatus: ");
	int index = status - buff; //Index of '\n'
	buff[index] = '\0'; //Null-terminate the buffer exactly where the new line starts
}

//Shows request's details without its status
void show_details(char id[], char path[]) {
	create_path(path, id); //Creates the path for the user's file
	int fd = open_file(path, O_RDONLY, 0);
	char buff[BUFF_SIZE];
	read_details(fd, path, buff);
	printf("%s\n", buff); //Show the details
	close(fd);
}

int main(int argc, char* argv[]) {
	//This function gets 2 arguments: ShowReqDetails, id
	if (argc != 2) {
		fprintf(stderr, "You should write: [ShowReqDetails] [id]\n");
		exit(FAILURE);
	}

	//In case this ID does not have a request:
	if (!has_request(argv[1])) {
		printf("Sorry! this user does not exist\n");
		return 0;
	}

	if (has_request(argv[1]) == IN_REQUEST) {
		char path[PATH_LEN] = "Loan/Request/";
		show_details(argv[1], path);
	}
	else if (has_request(argv[1]) == IN_APPROVED) {
		char path[PATH_LEN] = "Loan/Result/Approved/";
		show_details(argv[1], path);
	}
	else {
		char path[PATH_LEN] = "Loan/Result/Unapproved/";
		show_details(argv[1], path);
	}


	return 0;
}