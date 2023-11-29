#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 512



int main (){

    char response[BUFF_SIZE] = "CREATE_GROUP"; 

    // print len of response
    printf("%ld\n", strlen(response));


    
    char *token = strtok(response, " ");
    char *token1 = strtok(NULL, " ");

    // printf("%s\n", token1);
    char respon[BUFF_SIZE] = {0};
    fflush(stdin);
    fgets(respon, BUFF_SIZE, stdin);
    printf("%s\n", respon);
    return 0 ; 
}