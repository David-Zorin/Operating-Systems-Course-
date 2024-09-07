#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define FAILURE -1
#define EXIT 1
#define FALSE 0
#define TRUE 1
#define RATING_LEN 3
#define PATH_LEN 20
#define ARG_LEN 20
#define ACC_WIDTH 11
#define PASS_WIDTH 4
#define BORDER_WIDTH 2
#define MAX_ARGS 20

//Creates a directory
void create_dir(char path[]) {
    if (mkdir(path, 0777) == FAILURE) {
        fprintf(stderr, "Cannot create %s: ", path);
        perror(NULL);
        exit(FAILURE);
    }
}

//Creates all directories: "Loan", "Request", "Result", "Approved", "Unapproved"
void create_directories(){
    create_dir("Loan");
    create_dir("Loan/Request");
    create_dir("Loan/Result");
    create_dir("Loan/Result/Approved");
    create_dir("Loan/Result/Unapproved");
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

//Checks if User.txt exists, therefore assuming all directories and files already exist
int exist() {
    int fd = open("Loan/User.txt", O_RDONLY); //Try opening User.txt
    if (fd == -1) {
        if (errno == ENOENT) { //If the file does not exist
            return FALSE;
        }
        else { //If couldn't open the file for any other reason
            fprintf(stderr, "Cannot check if files and directories exist, while trying to open User.txt: ");
            perror(NULL);
            exit(FAILURE);
        }
    }
    return TRUE; //Files and directories exists
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

//Writes a new line in a text file
void write_newline(int fd, char path[]) {
    write_string(fd, path, "\n");
}

//Writes spaces a given number of times to a text file
void write_spaces(int fd, char path[], int times) {
    for (int i = 0; i < times; i++) {
        write_string(fd, path, " ");
    }
}

//Writes to a field in a table's column
void write_col(int fd, char path[], int col_width, char string[]) {
    int spaces = col_width - strlen(string);
    write_spaces(fd, path, spaces / 2);
    write_string(fd, path, string);
    write_spaces(fd, path, spaces - (spaces / 2));
}

//Writes a new row in a table to a text file
void write_row(int fd, char path[], int col_num, int col_width[], char* strings[]) {
    for (int col = 0; col < col_num; col++) {
        write_col(fd, path, col_width[col], strings[col]);
        if(col != col_num - 1) write_spaces(fd, path, BORDER_WIDTH);
    }
    write_newline(fd, path);
}

void create_users() {
    int fd = open_file("Loan/User.txt", O_WRONLY | O_CREAT, 0777);
    int col_num = 2; //Number of columns
    int col_width[2] = { ACC_WIDTH, PASS_WIDTH }; //Columns' width
    char* titles[2] = { "Account", "Pass"}; //Title 1: "Account", Title 2: "Pass"
    write_row(fd, "Loan/Users.txt", col_num, col_width, titles);
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

//Creates the credit rating
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

//Creates Criteria.txt
void create_criteria() {
    int fd = open_file("Loan/Criteria.txt", O_WRONLY | O_CREAT, 0777);
    char rating[RATING_LEN];
    get_rating(5, rating);
    write_string(fd, "Loan/Criteria.txt", rating);
    close(fd);
}

//Creates Users.txt and Criteria.txt
void create_files() {
    create_users();
    create_criteria();
}

//Scans an argument, and notes if there are more to come
int scan_argument(char argument[]) {
    char chr ='\0';
    int index = 0;
    while (chr != ' ' && chr != '\n') {
        scanf("%c", &chr);
        if (chr != ' ' && chr != '\n') {
            argument[index] = chr;
            index++;
        }
    }
    argument[index] = '\0'; //Null-terminate the argument to turn it into a string
    
    //In case that was the last argument:
    if (chr == '\n')
        return FALSE;
    
    //There are probably more arguments:
    return TRUE;
}

//Scans command inserted by the user
void scan_command(char args[][ARG_LEN]) {
    int arg_num = 0;
    int more_args = TRUE; //At first, assume there are arguments to scan
    while(more_args) {
        more_args = scan_argument(args[arg_num]);
        arg_num++;
    }
}

//Returns TRUE if 2 strings are equal
int equal(char str1[], char str2[]) {
    return !strcmp(str1, str2);
}

//Returns TRUE if the command inserted by the user is not valid
int not_valid(char command[]) {
    char* valid_commands[5] = { "GetLoan", "CheckStatus", "ApproveLoan", "LoginM", "exit" };
    for (int i = 0; i < 5; i++) {
        if (equal(valid_commands[i], command)) {
            return FALSE; //Valid command
        }
    }
    return TRUE; //Not valid command
}

//Inserts arguments into pointers
void to_ptrs(char args[][ARG_LEN], char* argv[]) {
    int index = 0;
    while(!equal(args[index], "\0")){
        argv[index] = args[index];
        index++;
    }
}

//Executes a command
int execute(char* argv[], int* status) {
    char path[PATH_LEN] = "./";
    strcat(path, argv[0]);
    pid_t id = fork();
    if (id == FAILURE) {
        perror("Cannot fork while trying to execute a command");
    }
    //In case this is the child:
    if (id == 0) {
        execv(path, argv);
        //In case cannot execute:
        fprintf(stderr, "Cannot execute %s: ", argv[0]);
        perror(NULL);
    }
    wait(status); //Main Shell waits until command executed
}

int main(){

    if (!exist()) { //In case the directories and files do not exist:
        create_directories(); //Creates "Loan", "Request", "Approved" and "Unapproved" directories
        create_files(); //Creates User.txt and Criteria.txt
    }

    int status = 0;
    while(WEXITSTATUS(status) != EXIT) {
        printf("MainShell> ");
        char args[MAX_ARGS][ARG_LEN] = { "\0" };
        scan_command(args); //Scan the command arguments
        if (not_valid(args[0])) {
            printf("Not Supported\n");
            continue;
        }
        char* argv[MAX_ARGS] = { NULL };
        to_ptrs(args, argv); //Insert the arguments into pointers
        execute(argv, &status); //Execute the given command
    }
    
    return 0;
}
