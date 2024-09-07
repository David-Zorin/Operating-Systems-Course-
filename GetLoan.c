#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define FALSE 0
#define TRUE 1
#define FAILURE -1
#define PASS_LEN 5
#define RATE_LEN 6
#define PATH_LEN 30
#define IN_REQUEST 1
#define IN_APPROVED 2
#define IN_UNAPPROVED 3
#define ACC_WIDTH 11
#define PASS_WIDTH 4
#define BORDER_WIDTH 2
#define NAME_LEN 15
#define AGE_LEN 4

//Concatanate given number of strings
void cat(char* string[], int strings_num) {
    for (int i = 1; i < strings_num; i++) {
        strcat(string[0], string[i]);
    }
}

//Creates the path for a user's file
void create_path(char path[], char id[]) {
    char* string[] = { path, id, ".txt"};
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

//Requests user's password for creation
void get_password(char id[], char password[]) {
    printf("User: %s\n", id);
    printf("Password: ");
    scanf("%s", password);
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

//Writes spaces a given number of times to a text file
void write_spaces(int fd, char path[], int times) {
    for (int i = 0; i < times; i++) {
        write_string(fd, path, " ");
    }
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

//Writes to a field in a table's column
void write_col(int fd, char path[], int col_width, char string[]) {
    int spaces = col_width - strlen(string);
    write_spaces(fd, path, spaces / 2);
    write_string(fd, path, string);
    write_spaces(fd, path, spaces - (spaces / 2));
}

//Writes a new line in a text file
void write_newline(int fd, char path[]) {
    write_string(fd, path, "\n");
}

//Writes a new row in a table to a text file
void write_row(int fd, char path[], int col_num, int col_width[], char* strings[]) {
    for (int col = 0; col < col_num; col++) {
        write_col(fd, path, col_width[col], strings[col]);
        if(col != col_num - 1) write_spaces(fd, path, BORDER_WIDTH);
    }
    write_newline(fd, path);
}

//Adds the user's id and password into User.txt file
void create_user(char id[], char password[]) {
    int fd = open_file("Loan/User.txt", O_WRONLY | O_APPEND, 0);
    int col_num = 2; //Number of columns
    int col_width[2] = { ACC_WIDTH, PASS_WIDTH }; //Columns' width
    char* strings[2] = { id, password }; //Column 1: <id>, Column 2: <password>
    write_row(fd, "Loan/User.txt", col_num, col_width, strings);
    printf("User Created\n");
    close(fd);
}

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

//Randoms a number between 1 to 10, converts it to a string
void get_rating(char rating[]) {
    //Randomize a number between 1-10:
    srand(time(NULL));
    int number = rand() % 10 + 1;
    int digit = number % 10; //Calculate the right digit
    //In case the right digit is '0', it's 10:
    if (digit == 0) {
        strcpy(rating, "10");
        return;
    }
    //Else, it's a number less than 10:
    strcpy(rating, to_char(digit));
}

void get_details(char name[], char surname[], char age[], char rating[]) {
    printf("Insert Details:\n");
    printf("Name: ");
    scanf("%s", name);
    printf("SurName: ");
    scanf("%s", surname);
    printf("Age: ");
    scanf("%s", age);
    get_rating(rating);
}

//Changes the rating string to be out of 10
void outof_ten(char rating[]) {
    strcat(rating, "/10");
}

//Writes a form line
void write_formln(int fd, char path[], char field[], char info[], int add_line) {
    write_string(fd, path, field);
    write_string(fd, path, info);
    if (add_line) { write_newline(fd, path); };
}

//Creates the request's form file
void create_request(char id[], char name[], char surname[], char age[], char sum[], char payments[], char rating[]) {
    char path[PATH_LEN] = "Loan/Request/";
    create_path(path, id); //Create the path for the request file
    outof_ten(rating);
    int fd = open_file(path, O_WRONLY | O_APPEND | O_CREAT, 0777);
    write_formln(fd, path, "Name: ", name, TRUE);
    write_formln(fd, path, "SurName: ", surname, TRUE);
    write_formln(fd, path, "Age: ", age, TRUE);
    write_formln(fd, path, "Sum: ", sum, TRUE);
    write_formln(fd, path, "Payments: ", payments, TRUE);
    write_formln(fd, path, "Rating: ", rating, TRUE);
    write_formln(fd, path, "Status: ", "Waiting!", FALSE);
    close(fd);
    printf("Request Saved!\n");
}

int main(int argc, char* argv[]){
    //This function gets 4 argument: GetLoan, id, loan sum, number of payments
    if(argc != 4){
        fprintf(stderr, "You should insert: [GetLoan] [id] [loan sum] [number of payments]\n");
        exit(FAILURE);
    }
    
    //In case this user already has a request:
    if(has_request(argv[1])){
        printf("Sorry! you have a request.\n");
        return 0;
    }

    //Else, get his details and create his request:
    char password[PASS_LEN], name[NAME_LEN], surname[NAME_LEN], age[AGE_LEN], rating[RATE_LEN];
    get_password(argv[1], password);
    create_user(argv[1], password);
    get_details(name, surname, age, rating);
    create_request(argv[1], name, surname, age, argv[2], argv[3], rating);
    
    return 0;
}