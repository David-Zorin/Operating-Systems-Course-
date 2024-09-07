#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define FAILURE -1
#define DIGITS 256

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

//Creates a temporary file that contains the number of files in "Request" directory
void create_temp() {
	int id = fork();
	if (id == FAILURE) { //In case couldn't fork:
		fprintf(stderr, "Cannot fork to create temporary file: ");
		perror(NULL);
	}
	if (id == 0) { //In case this is the child:
		execlp("sh", "sh", "-c", "ls -l Loan/Request | wc -l > Loan/Request/temp.txt", NULL);
		//In case couldn't execute:
		fprintf(stderr, "Cannot execute a program to create temporary file: ");
		perror(NULL);
	}
	wait(NULL); //Parent waits until child finished execution
}

//Reads the temporary file characters which is the number of lines ls -l produced
void read_temp(char lines[]) {
	int fd = open_file("Loan/Request/temp.txt", O_RDONLY, 0);
	char digit;
	int index = 0;
	do {
		read_character(fd, "Loan/Request/temp.txt", &digit);
		if (digit != ' ') {
			lines[index] = digit;
			index++;
		}
	} while (digit != '\n'); //File ends with '\n'
	lines[index] = '\0'; //Null-terminate to turn it into a string
	close(fd);
}

//Prints the number of waiting requests
int get_waiting(char lines[]) {
	return atoi(lines) - 2; //Subtracting "total X" line and "temp.txt" line from the count
}

//Removes the temporary file created
void remove_temp() {
	int id = fork();
	if (id == FAILURE) { //In case couldn't fork:
		fprintf(stderr, "Cannot fork to remove temporary file: ");
		perror(NULL);
	}
	if (id == 0) { //In case this is the child:
		execlp("rm", "rm", "Loan/Request/temp.txt", NULL);
		//In case couldn't execute:
		fprintf(stderr, "Cannot execute a program to remove temporary file: ");
		perror(NULL);
	}
	wait(NULL); //Parent waits until child finished execution
}

int main(int argc, char* argv[]) {
	//This function gets 1 argument: CheckRequestList
	if (argc != 1) {
		fprintf(stderr, "You should insert: [CheckRequestList]\n");
		exit(FAILURE);
	}

	char lines[DIGITS]; //String that represents the number of lines "ls -l" on the directory produced
	create_temp(); //Creating temp.txt, contains the number of lines
	read_temp(lines); //Reading the number of lines
	int waiting = get_waiting(lines); //Number of waiting requests
	remove_temp(); //Removing temp.txt

	return waiting;
}

