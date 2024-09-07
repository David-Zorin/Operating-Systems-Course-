#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

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

//Reads a file into a buffer
int read_file(int fd, char path[], char buffer[], int buff_size) {
	int rbytes = read(fd, buffer, buff_size);
	if (rbytes == FAILURE) {
		fprintf(stderr, "Cannot read from %s: ", path);
		perror(NULL);
		exit(FAILURE);
	}
	return rbytes;
}

//Reads one character from a file
int read_character(int fd, char path[], char* chr) {
	return read_file(fd, path, chr, 1); //Returns how many bytes read
}

//Finds a position in the file
int seek_file(int fd, char path[], int offset, int whence) {
	int oset = lseek(fd, offset, whence);
	if (oset == FAILURE) {
		fprintf(stderr, "Cannot seek in file %s: ", path);
		perror(NULL);
		exit(FAILURE);
	}
	return oset;
}

//Position of the last character in a file
int end_of_file(int fd, char path[]) {
	int offset = seek_file(fd, path, -1, SEEK_END);
	return offset;
}

//Finds the position the last line starts
void seek_lastln(int fd, char path[]) {
	int offset = end_of_file(fd, path); //Gets the position of the last character
	int rbytes;
	char chr;
	do {
		offset = seek_file(fd, path, offset - 1, SEEK_SET); //Move position backwards
		rbytes = read_character(fd, path, &chr); //Read the character
	} while (rbytes > 0 && chr != '\n');
}

//Writes a string to a text file
int write_string(int fd, char path[], char string[]) {
	int wbytes = write(fd, string, strlen(string));
	if (wbytes == FAILURE) {
		fprintf(stderr, "Cannot write to %s: ", path);
		perror(NULL);
		exit(FAILURE);
	}
	return wbytes;
}

//Moves a file from source to dest
void move_file(char source[], char dest[]) {
	int id = fork();
	if (id == FAILURE) {
		perror("Failed to fork");
	}
	if (id == 0) { //The child executes the transference
		execlp("mv", "mv", source, dest, NULL);
	}
	wait(NULL); //Parent waits until transference executed
}

//Changes the status from "Waiting!" to "Awaiting signature" and moves the file to "Approved" directory
void change_status(char id[]) {
	char path[PATH_LEN] = "Loan/Request/";
	create_path(path, id);
	int fd = open_file(path, O_RDWR, 0);
	seek_lastln(fd, path);
	write_string(fd, path, "Status: Awaiting signature");
	close(fd);
	move_file(path, "Loan/Result/Approved");
}

int main(int argc, char* argv[]) {
	//This function gets 2 arguments: ApproveReqLoan, id
	if (argc != 2) {
		fprintf(stderr, "You should insert: [ApproveReqLoan] [id]\n");
		exit(FAILURE);
	}

	if (!has_request(argv[1])) {
		printf("Request Not Found!\n");
	}
	else if (has_request(argv[1]) == IN_APPROVED) {
		printf("Request Already Approved!\n");
	}
	else if (has_request(argv[1]) == IN_UNAPPROVED) {
		printf("Request Already Unapproved!\n");
	}
	else {
		change_status(argv[1]);
		printf("Request Approved.\n");
	}
	
	return 0;
}