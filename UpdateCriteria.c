#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define FAILURE -1
#define RATING_LEN 3

//Converts a digit to a character
char* to_char(int digit) {
	switch (digit) {
	case 1:
		return "1";
	case 2:
		return "2";
	case 3:
		return "3";
	case 4:
		return "4";
	case 5:
		return "5";
	case 6:
		return "6";
	case 7:
		return "7";
	case 8:
		return "8";
	case 9:
		return "9";
	}
}

//Creates the new credit rating
void get_rating(int min_rating, char new_rating[]) {
	//Randomize a number between minimal rating given to 10:
	srand(time(NULL));
	int rating = rand() % (10 - min_rating + 1) + min_rating;
	int digit = rating % 10;
	//In case the right digit is '0', it's 10:
	if (digit == 0) {
		strcpy(new_rating, "10");
		return;
	}
	//Else, it's a number less than 10:
	strcpy(new_rating, to_char(digit));
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
	wait(NULL); //Parent waits until removal executed
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

void update_criteria(char new_rating[]) {
	remove_file("Loan/Criteria.txt"); //Removes the criteria file with the old credit rating
	int fd = open_file("Loan/Criteria.txt", O_WRONLY | O_CREAT, 0777); //Creates a new one
	write_string(fd, "Loan/Criteria.txt", new_rating); //Writes the rating into it
	close(fd);
}

int main(int argc, char* argv[]) {
	//This function gets 2 argument: UpdateCriteria, Minimal Rating
	if (argc != 2 || atoi(argv[1]) < 1 || atoi(argv[1]) > 10) {
		fprintf(stderr, "Wrong Parameters!\n");
		exit(FAILURE);
	}

	char new_rating[RATING_LEN];
	get_rating(atoi(argv[1]), new_rating); //Get a new rating using the new minimum
	update_criteria(new_rating);

	return 0;
}
