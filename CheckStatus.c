#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define FAILURE -1
#define FALSE 0
#define TRUE 1
#define PATH_LEN 40
#define ID_LEN 10 //ID contains 9 numbers
#define PASS_LEN 5 //Password contains 4 numbers
#define LAST_LINE_LEN 30
#define IN_REQUEST 1
#define IN_APPROVED 2
#define IN_UNAPPROVED 3
#define ACC_WIDTH 11
#define PASS_WIDTH 4
#define BORDER_WIDTH 2
#define APPROVED 1
#define AWAITING 2

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
	if (fd == -1) {
		if (errno == ENOENT) { //If the file does not exist
			return FALSE;
		}
		else { //If couldn't open the file for any other reason
			fprintf(stderr, "Cannot check if the user has a request, while trying to open %s.txt", id);
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

//Returns TRUE if 2 strings are equal
int equal(char str1[], char str2[]) {
	return !strcmp(str1, str2);
}

//Reads the IDs in User.txt until the given id is found
void find_id(int fd, char path[], char given_id[]) {
	int row_len = ACC_WIDTH + BORDER_WIDTH + PASS_WIDTH + 1; //Columns' width + '\n'
	int spaces = ACC_WIDTH - (ID_LEN -1); //Number of spaces in account column
	int offset = seek_file(fd, path, row_len, SEEK_SET); //Skip columns' titles
	int found = FALSE; //ID not found yet
	while (!found) {
		seek_file(fd, path, spaces / 2, SEEK_CUR); //Skip the first spaces
		char id[ID_LEN];
		read_file(fd, path, id, ID_LEN - 1);
		id[ID_LEN - 1] = '\0'; //Null-terminate the string
		found = equal(id, given_id); //Will be TRUE when a user id in this row is the given id
		if (!found) {
			seek_file(fd, path, row_len - ((spaces / 2) + (ID_LEN - 1)), SEEK_CUR); //Skip to the next user row
		}
	}
}

//Reads the password of a given id
void find_pass(int fd, char path[], char given_id[], char password[]) {
	int spaces = ACC_WIDTH - (ID_LEN -1); //Number of spaces in account column
	find_id(fd, path, given_id);
	//Skip last spaces and the border, get the position of the password
	seek_file(fd, path, BORDER_WIDTH + (spaces - (spaces / 2)), SEEK_CUR);
	read_file(fd, path, password, PASS_LEN - 1); //Read the password
}

//Returns TRUE if the given password is the user's password
int is_pass(char id[], char given_password[]) {
	char path[PATH_LEN] = "Loan/User.txt";
	char password[PASS_LEN];
	int fd = open_file(path, O_RDONLY, 0);
	find_pass(fd, path, id, password);
	close(fd);
	return equal(password, given_password);
}

int get_status(char id[]) {
	int status = AWAITING; //At first, assume it's 'Awaiting'
	char path[PATH_LEN] = "Loan/Result/Approved/";
	create_path(path, id);
	int fd = open_file(path, O_RDONLY, 0);
	seek_lastln(fd, path);
	char line[LAST_LINE_LEN];
	int rbytes = read_file(fd, path, line, LAST_LINE_LEN - 1); //Read the last line
	line[rbytes] = '\0'; //Null-terminate the string
	if (equal(line, "Status: Approved")) {
		status = APPROVED;
	}
	close(fd);
	return status;
}

int main(int argc, char* argv[]) {
	//This function gets 3 Arguments: CheckStatus, id and password
	if (argc != 3) {
		fprintf(stderr, "You should write: [CheckStatus] [id] [password]\n");
		exit(FAILURE);
	}

	//In case this ID does not have a request:
	if (!has_request(argv[1])) {
		printf("Sorry! you do not have a request\n");
		return 0;
	}


	//In case the user inserted wrong password:
	if (!is_pass(argv[1], argv[2])) {
		printf("Wrong Pass!!!\n");
		return 0;
	}

	//Check status and print it:
	if (has_request(argv[1]) == IN_REQUEST) { printf("Waiting\n"); }
	else if (has_request(argv[1]) == IN_UNAPPROVED) { printf("Unapproved\n"); }
	else { //The request is in "Approved" directory
		int status = get_status(argv[1]);
		if (status == APPROVED) { printf("Approved\n"); }
		else { printf("Awaiting Signature\n"); }
	}

	return 0;
}
