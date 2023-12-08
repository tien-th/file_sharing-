#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 512



int main (){

    char response[BUFF_SIZE] = "CREATE_GROUP"; 

    // print len of response
    // printf("%ld\n", strlen(response));


    
    // char *token = strtok(response, " ");
    // char *token1 = strtok(NULL, " ");

    // // printf("%s\n", token1);
    // char respon[BUFF_SIZE] = {0};
    // fflush(stdin);
    // fgets(respon, BUFF_SIZE, stdin);
    // printf("%s\n", respon);

    printf("\n--------------------------------\n");
    char command[BUFF_SIZE] = "CREATE_GROUP ffffff username";
    char username[20] = {0};
    sscanf(command, "%*s %*s %s", username);  
    printf("%s\n", username);

    printf("\n--------------------------------\n");
    // char filename[] = "/mnt/disk3/tien_aiot/file_sharing-/ssssd/das/server/server5.c";
    char filename[] = "server5.c";

    // Find the last occurrence of '/' in the path
    char *last_slash = strrchr(filename, '/');
    char extracted_filename[256];
    if (last_slash != NULL) {
        // Extract the filename after the last '/'
          // Adjust the size as needed
        strcpy(extracted_filename, last_slash + 1);

        // Print the extracted filename
        printf("Extracted Filename: %s\n", extracted_filename);
    } else {
        // Handle the case where there is no '/'
        printf("Invalid file path\n");
    }

    return 0 ; 
}