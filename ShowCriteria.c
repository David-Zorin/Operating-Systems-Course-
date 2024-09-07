#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FAILURE -1
#define CRITERIA_LEN 3

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

void read_criteria(char criteria[]) {
	int fd = open_file("Loan/Criteria.txt", O_RDONLY, 0);
	int rbytes = read_file(fd, "Loan/Criteria.txt", criteria, CRITERIA_LEN); //Read the criteria
	criteria[rbytes] = '\0'; //Null-terminate to turn it into a string
	close(fd);
}

void show_criteria() {
	char criteria[CRITERIA_LEN];
	read_criteria(criteria);
	printf("Min Rating %s\n", criteria);
}

int main(int argc, char* argv[]) {
	//This function gets 1 argument: ShowCriteria
	if (argc != 1) {
		fprintf(stderr, "You should insert: [ShowCriteria]\n");
		exit(FAILURE);
	}

	show_criteria();

	return 0;
}