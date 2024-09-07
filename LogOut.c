#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define FAILURE -1

int main(int argc, char* argv[]){
    //This function gets 1 argument: exit
    if(argc != 1){
        fprintf(stderr, "To logout, you should insert: [LogOut]\n");
        exit(FAILURE);
    }

    int parent_pid = getppid();
    if (kill(parent_pid, SIGKILL) == FAILURE) {
        perror("Kill Manager_Shell failed");
        exit(FAILURE);
    }

    return 0;
}