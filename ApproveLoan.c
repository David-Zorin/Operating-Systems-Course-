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
#define ID_LEN 10 //ID contains 9 numbers
#define PASS_LEN 5 //Password contains 4 numbers
#define ANS_LEN 10
#define IN_REQUEST 1
#define IN_APPROVED 2
#define IN_UNAPPROVED 3
#define ACC_WIDTH 11
#define PASS_WIDTH 4
#define BORDER_WIDTH 2
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
		fprintf(stderr, "Cannot read from %s", path);
		perror(NULL);
		exit(FAILURE);
	}
	return rbytes;
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

//Returns TRUE if 2 strings are equal
int equal(char str1[], char str2[]) {
	return !strcmp(str1, str2);
}

//Reads the IDs in User.txt until the given id is found
void find_id(int fd, char path[], char given_id[]) {
	int row_len = ACC_WIDTH + BORDER_WIDTH + PASS_WIDTH + 1; //Columns' width + '\n'
	int spaces = ACC_WIDTH - (ID_LEN - 1); //Number of spaces in account column
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
	int spaces = ACC_WIDTH - (ID_LEN - 1); //Number of spaces in account column
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

//Checks if the user's request in status approved
int is_approved(char id[]) {
	char path[PATH_LEN] = "Loan/Result/Approved/";
	create_path(path, id);
	int fd = open(path, O_WRONLY); //Try opening a request file in WRONLY mode
	if (fd == FAILURE) {
		if (errno == EACCES) { //If the file exists but it's mode is set to read-only
			return TRUE;
		}
		else { //If couldn't open the file for any other reason (this file exists in this path if the function started)
			fprintf(stderr, "Cannot check if the user is approved, while trying to open %s: ", path);
			perror(NULL);
			exit(FAILURE);
		}
	}
	close(fd);
	return FALSE; //The user's request is in status awaiting signature
}

//Reads the details from a request file without its status, and null-terminating it
void read_details(int fd, char path[], char buff[]) {
	int rbytes = read_file(fd, path, buff, BUFF_SIZE); //Read all form
	buff[rbytes] = '\0'; //Null-terminate the end to make it a string
	char* status = strstr(buff, "\nStatus: ");
	int index = status - buff; //Index of '\n'
	buff[index] = '\0'; //Null-terminate the buffer exactly where the new line starts
}

//Shows request's details without its status
void show_details(char id[]) {
	char path[PATH_LEN] = "Loan/Result/Approved/";
	create_path(path, id); //Creates the path for the user's file
	int fd = open_file(path, O_RDONLY, 0);
	char buff[BUFF_SIZE];
	read_details(fd, path, buff);
	printf("%s\n", buff); //Show the details
	close(fd);
}

//Gets user's approval to confirm the loan
void get_approval() {
	int approved = FALSE;
	char answer[ANS_LEN];
	while (!approved) {
		printf("Plz type Approve: ");
		scanf("%s", answer);
		if (equal("Approve", answer)) {
			approved = TRUE;
		}
	}
}

//Gets user's password after approved to confirm the loan
void get_password(char id[]) {
	int correct = FALSE;
	char given_password[PASS_LEN];
	while (!correct) {
		printf("Password: ");
		scanf("%s", given_password);
		if (is_pass(id, given_password)) {
			correct = TRUE;
		}
		else {
			printf("Wrong Password!!!\n");
		}
	}
}

//Removes a file in a given path
void remove_file(char path[]) {
	int id = fork();
	if (id == FAILURE) {
		perror("Failed to fork");
	}
	if (id == 0) { //The child will execute removal
		execlp("rm", "rm", path, NULL);
	}
	wait(NULL); //The parent will wait until removal occured
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

//Changes status to "Approved", changing mode of file to read-only
void approve(char id[]) {
	char path[PATH_LEN] = "Loan/Result/Approved/";
	create_path(path, id); //Creates the path for the user's file
	int fd = open_file(path, O_RDONLY, 0);
	char buff[BUFF_SIZE];
	read_details(fd, path, buff);
	close(fd);
	//Remove the file, create a new one:
	remove_file(path);
	fd = open_file(path, O_WRONLY | O_CREAT, 0444);
	write_string(fd, path, buff);
	write_string(fd, path, "\nStatus: Approved");
	close(fd);
}

int main(int argc, char* argv[]) {
	//This function gets 3 Arguments: ApproveLoan, id and password
	if (argc != 3) {
		fprintf(stderr, "You should write: [ApproveLoan] [id] [password]\n");
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

	//In case the user's request is in "Approved" directory:
	if (has_request(argv[1]) == IN_APPROVED) {
		//In case the user is approved:
		if (is_approved(argv[1])) {
			printf("Sorry! you are already approved!\n");
		}
		//Else, he is awaiting signature:
		else {
			show_details(argv[1]); //Show user's details
			get_approval(); //Ask his approval for the loan
			get_password(argv[1]); //Ask for his password
			approve(argv[1]); //Status changed to "Approved"
			printf("Good Luck!!\n");
		}
	}
	//In case it is not in "Approved" directory:
	else {
		printf("No approved request!\n");
	}

	return 0;
}