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
#define ACC_LEN 20
#define PASS_LEN 20

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

//Skips a line in a text file
void skip_line(int fd, char path[], int end, int* offset) {
	char chr = '\0';
	while (chr != '\n' && *offset != end) {
		*offset += read_character(fd, path, &chr);
	}
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

//End of file offset (SHOULD BE USED BEFORE MOVING THE OFFSET FROM THE BEGINNING!)
int eof_offset(int fd, char path[]) {
	int offset = seek_file(fd, path, 0, SEEK_END);
	seek_file(fd, path, 0, SEEK_SET); //Turn the offset back to the beginning of the file
	return offset;
}

//Skips spaces in a text file
void skip_spaces(int fd, char path[], int* offset) {
	char chr = ' ';
	while (chr == ' ') {
		read_character(fd, path, &chr);
	}
	*offset = seek_file(fd, path, -1, SEEK_CUR); //Set the offset back to the the first character which is not a space
}

//Reads a string from a text file
void read_string(int fd, char path[], int* offset, char string[]) {
	char chr = '\0';
	int index = 0;
	while (chr != ' ' && chr != '\n') {
		*offset += read_character(fd, path, &chr);
		string[index] = chr;
		index++;
	}
	string[index - 1] = '\0';
}

//Returns TRUE if 2 strings are equal
int equal(char str1[], char str2[]) {
	return !strcmp(str1, str2);
}

//Returns offset if the account found in the file, else FAILURE
int find_account(int fd, char path[], char given_account[]) {
	char account[ACC_LEN];
	int offset = 0;
	int end = eof_offset(fd, path);
	skip_line(fd, path, end, &offset); //Skips the titles line
	do {
		skip_spaces(fd, path, &offset); //Skip spaces until reached the account
		read_string(fd, path, &offset, account); //Read the account
		if (equal(account, given_account)) { //If it's equal to the given account
			return offset;
		}
		skip_line(fd, path, end, &offset); //If it's not, skip to the next line of account
	} while (offset != end); //While not getting to the end of the file
	return FAILURE; //Account not found
}

int account_exists(char given_account[]) {
	int fd = open_file("Manager_pass.txt", O_RDONLY, 0);
	if (find_account(fd, "Manager_pass.txt", given_account) == FAILURE) { //In case cannot find this account
		close(fd);
		return FALSE;
	}
	close(fd);
	return TRUE; //Account exists
}

//Reads the pass from the file
void read_pass(int fd, char path[], char account[], char password[]) {
	int offset = find_account(fd, path, account); //Sets the offset right after the last character of account
	skip_spaces(fd, path, &offset); //Skips spaces until reached to the password
	read_string(fd, path, &offset, password);
}

//Returns TRUE if the password given is correct
int is_pass(char account[], char given_password[]) {
	int fd = open_file("Manager_pass.txt", O_RDONLY, 0);
	char password[PASS_LEN];
	read_pass(fd, "Manager_pass.txt", account, password);
	return equal(password, given_password);
}

int main(int argc, char* argv[]) {
	//This function gets 3 arguments: LoginM, account and password
	if (argc != 3) {
		fprintf(stderr, "You should insert: [LoginM] [Account] [Password]\n");
		exit(FAILURE);
	}

	if (!account_exists(argv[1])) {
		printf("This account does not exist!\n");
		return 0;
	}

	if (!is_pass(argv[1], argv[2])) {
		printf("Wrong Password!!!\n");
		return 0;
	}

	int id = fork();
	if (id == FAILURE) {
		perror("Cannot fork to execute ManagerShell");
	}
	if (id == 0) {
		execl("Manager_shell", "Manager_shell", NULL);
		//In case of an error:
		perror("Cannot execute ManagerShell");
		exit(FAILURE);
	}
	wait(NULL); //LoginM dies after ManagerShell dies

	return 0;
	
}
