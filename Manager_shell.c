#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define FAILURE -1
#define EXIT 1
#define FALSE 0
#define TRUE 1
#define PATH_LEN 20
#define ARG_LEN 20
#define MAX_ARGS 20

//Scans an argument, and notes if there are more to come
int scan_argument(char argument[]) {
    char chr = '\0';
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

//Scans command inserted by the manager
void scan_command(char args[][ARG_LEN]) {
    int arg_num = 0;
    int more_args = TRUE; //At first, assume there are arguments to scan
    while (more_args) {
        more_args = scan_argument(args[arg_num]);
        arg_num++;
    }
}

//Returns TRUE if 2 strings are equal
int equal(char str1[], char str2[]) {
    return !strcmp(str1, str2);
}

//Returns TRUE if the command inserted by the manager is not valid
int not_valid(char command[]) {
    char* valid_commands[10] = { "ShowReqNum", "ShowAReqNum", "ShowUnAReqNum", "CheckRequestList", "ShowReqDetails",
                                "ShowCriteria", "UpdateCriteria", "ApproveReqLoan", "UnApproveReqLoan", "LogOut"};
    for (int i = 0; i < 10; i++) {
        if (equal(valid_commands[i], command)) {
            return FALSE; //Valid command
        }
    }
    return TRUE; //Not valid command
}

//Returns TRUE if this command shows how many files are in a directory
int is_count_cmd(char command[]) {
    char* count_commands[4] = { "ShowReqNum", "ShowAReqNum", "ShowUnAReqNum", "CheckRequestList" };
    for (int i = 0; i < 4; i++) {
        if (equal(count_commands[i], command)) {
            return TRUE; //It's a files count command
        }
    }
    return FALSE; //It's other command
}

//Inserts arguments into pointers
void to_ptrs(char args[][ARG_LEN], char* argv[]) {
    int index = 0;
    while (!equal(args[index], "\0")) {
        argv[index] = args[index];
        index++;
    }
}

//Executes a command
int execute(char* argv[], int* status) {
    char path[PATH_LEN] = "./";
    strcat(path, argv[0]);
    int id = fork();
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
    wait(status); //Manager Shell waits until command executed
}

int main() {

    int files = 0;
    while (TRUE) {
        printf("ManagerShell> ");
        char args[MAX_ARGS][ARG_LEN] = { "\0" };
        scan_command(args); //Scan the command arguments
        if (not_valid(args[0])) {
            printf("Not Supported\n");
            continue;
        }
        char* argv[MAX_ARGS] = { NULL };
        to_ptrs(args, argv); //Insert the arguments into pointers
        execute(argv, &files); //Execute the given command
        if (is_count_cmd(args[0])) {
            printf("%d\n", WEXITSTATUS(files));
        }
    }
}