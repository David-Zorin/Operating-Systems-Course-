#include <stdio.h>
#include <stdlib.h>

#define FAILURE -1
#define EXIT 1

int main(int argc, char* argv[]){
    //This function gets 1 argument: exit
    if(argc != 1){
        fprintf(stderr, "You should insert: [exit]\n");
        exit(FAILURE);
    }

    printf("Goodbye...\n");

    return EXIT;
}