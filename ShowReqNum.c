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

//Creates a temporary file that contains the number of lines in User.txt
void create_temp(){
	int id = fork();
	if (id == FAILURE) { //In case couldn't fork:
		fprintf(stderr, "Cannot fork to create temporary file: ");
		perror(NULL);
	}
	if (id == 0) { //In case this is the child:
		execlp("sh", "sh", "-c", "wc -l Loan/User.txt > Loan/temp.txt", NULL);
		//In case couldn't execute:
		fprintf(stderr, "Cannot execute a program to create temporary file: ");
		perror(NULL);
	}
	wait(NULL); //Parent waits until child finished execution
}

//Reads the temporary file first characters which is the number of lines in User.txt
void read_temp(char lines[]) {
	int fd = open_file("Loan/temp.txt", O_RDONLY, 0);
	char digit;
	int index = 0;
	do {
		read_character(fd, "Loan/temp.txt", &digit);
		if (digit != ' ') {
			lines[index] = digit;
			index++;
		}
	} while (digit != ' ');
	lines[index] = '\0'; //Null-terminate to turn it into a string
	close(fd);
}

//Returns the number of requests
int get_requests(char lines[]) {
	return atoi(lines) - 1; //The number of requests is the number of lines in User.txt without the titles row
}

//Removes the temporary file created
void remove_temp() {
	int id = fork();
	if (id == FAILURE) { //In case couldn't fork:
		fprintf(stderr, "Cannot fork to remove temporary file: ");
		perror(NULL);
	}
	if (id == 0) { //In case this is the child:
		execlp("rm", "rm", "Loan/temp.txt", NULL);
		//In case couldn't execute:
		fprintf(stderr, "Cannot execute a program to remove temporary file: ");
		perror(NULL);
	}
	wait(NULL); //Parent waits until child finished execution
}

int main(int argc, char* argv[]) {
	//This function gets 1 argument: ShowReqNum
	if (argc != 1) {
		fprintf(stderr, "You should insert: [ShowReqNum]\n");
		exit(FAILURE);
	}

	char lines[DIGITS]; //String that represents the number of lines in User.txt
	create_temp(); //Creating temp.txt, contains the number of lines
	read_temp(lines); //Reading the number of lines
	int requests = get_requests(lines); //Number of requests
	remove_temp(); //Removing temp.txt

	return requests;
}
