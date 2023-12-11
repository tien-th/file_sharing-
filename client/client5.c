#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 9989
#define BUFF_SIZE 512
#define FILE_BUFF_SIZE 1024
#define MAX_GROUP 100

char USERNAME[20];

// utils.c
long get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return -1;  // Return -1 to indicate an error
    }

    // Seek to the end of the file
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(file);
        return -1;
    }

    // Get the current position, which corresponds to the file size
    long size = ftell(file);
    if (size == -1) {
        perror("Error getting file size");
        fclose(file);
        return -1;
    }

    fclose(file);
    return size;
}

// utils.c
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}



void send_command(int sockfd, const char *command, const char *data) {
    char buffer[BUFF_SIZE];
    snprintf(buffer, BUFF_SIZE, "%s %s", command, data);
    send(sockfd, buffer, strlen(buffer), 0);
}


void create_group(int sockfd, const char *command,const char *username ,const char *data) {
    char buffer[BUFF_SIZE];
    snprintf(buffer, BUFF_SIZE, "%s %s %s", command, username, data);
    send(sockfd, buffer, strlen(buffer), 0);
}

char * registerAcc(int sockfd, const char * data) {
    // send_command(sockfd, "REG", "john_doe hashed_password");
    
    send_command(sockfd, "REG", data);
    // receive response from server amd print it
    char username[20];
    int r = recv(sockfd, username, BUFF_SIZE, 0);
    if (r > 0) {
        username[r] = '\0';
        if (strcmp(username, "-1") == 0) {
            printf("Username already exists\n");
            return "-1";
        }
        else if (strcmp(username, "0") == 0) {
            printf("Registration failed\n");
            return "-1";
        }
        else {
            printf("Registration successful. Welcome %s\n", username);
            char * tmp = (char *)calloc(20, sizeof(char));
            // copy
            strcpy(tmp, username);

            return tmp;
        }
    }
    else {
        printf("Error receiving server's response\n");
    }
}


char * login(int sockfd, const char * data) {
    // send_command(sockfd, "LOGIN", "john_doe hashed_password");
    send_command(sockfd, "LOGIN", data);
    // receive response from server amd print it
    char username[20];
    int r = recv(sockfd, username, BUFF_SIZE, 0);
    if (r > 0) {
        username[r] = '\0';
        if (strcmp(username, "-1") == 0) {
            printf("Invalid username or password\n");
            return "-1";
        }
        else if (strcmp(username, "0") == 0) {
            printf("Login failed\n");
            return "-1";
        }
        else {
            printf("Login successful. Welcome %s\n", username);
            char * tmp = (char *)calloc(20, sizeof(char));
            // copy
            strcpy(tmp, username);

            return tmp;
        }
    }
    else {
        printf("Error receiving server's response\n");
    }
}

void upload_file(int sockfd, const char *filename) {
    FILE *file = fopen(filename, "rb");
    char file_buffer[FILE_BUFF_SIZE];
    int bytes_read;

    if (file != NULL) {
        // Get the file size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Send the file size to the server
        send(sockfd, &file_size, sizeof(file_size), 0);

        // Send the file content
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFF_SIZE, file)) > 0) {
            send(sockfd, file_buffer, bytes_read, 0);
            memset(file_buffer, 0, sizeof(file_buffer));
        }

        fclose(file);
    }
}

void download_file(int sockfd, const char *filename) {
    send_command(sockfd, "DOWNLOAD_FILE", filename);

    FILE *file = fopen(filename, "wb");
    char file_buffer[FILE_BUFF_SIZE];
    int bytes_received;

    if (file != NULL) {
        while ((bytes_received = recv(sockfd, file_buffer, FILE_BUFF_SIZE, 0)) > 0) {
            fwrite(file_buffer, sizeof(char), bytes_received, file);
        }
        fclose(file);
    }
}

void join_group(int sockfd, const char *username, const char *groupname) {
    send_command(sockfd, "JOIN_GROUP", groupname);
    // TODO
}

void leave_group(int sockfd, const char *username, const char *groupname) {
    send_command(sockfd, "LEAVE_GROUP", groupname);
    // TODO
}

void group_function(int sockfd, const char *username) {
    char group_list[(MAX_GROUP + 1) * 20] = {0};
    send_command(sockfd, "YOUR_GROUPS", USERNAME); 
    recv(sockfd, group_list, sizeof(group_list), 0);
    printf("List of your groups: \n%s\n", group_list);

    while(1)
    {    
        printf("What do you want ?: \n");
        printf("1. Send file to group (command: UPLOAD group_name file_path)\n");
        printf("2. Download file from group (command: DOWNLOAD_FILE group_name file_name)\n");
        printf("3. Show group members (command: SHOW_MEMBERS group_name)\n");
        printf("4. Leave group (command: LEAVE_GROUP group_name)\n");
        printf("5. Exit (command: exit) \n");

        char command[BUFF_SIZE] = {0};
        // clear_input_buffer(); 
        fflush(stdin); 
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        if (strcmp(command, "exit") == 0) {
            break;
        }
        else if (strncmp(command, "UPLOAD", 5) == 0 ) {
            send_command(sockfd, command , username); 
            char ack[BUFF_SIZE] = {0};
            recv(sockfd, ack, sizeof(ack), 0);
            if (strcmp(ack, "1") == 0) {
                printf("Start upload\n");
                char filename[BUFF_SIZE] = {0};
                sscanf(command, "%*s %*s %s", filename);
                long file_size = get_file_size(filename);
                upload_file(sockfd, filename);
                printf("Upload successful\n");
            }
            else {
                printf("Upload failed\n");
            }

        }
    }

}


int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to the server");
        close(sockfd);
        return 1;
    }

    char command[BUFF_SIZE] = {0};
    // menu for user select 
    while (1) {
        printf("\n---------------WELCOME---------------\n") ;
        printf("1. Register (command: username password)\n");
        printf("2. Login (command: LOGIN username password)\n");
        printf("0. Exit\n");

        printf("\nYou select: ");

        int select; 
        if (scanf("%d", &select) == 1) {
            printf("You entered: %d\n", select);
        }


        switch (select)
        {
        case 1: 
            printf("Enter (username password) for register: \n");
            clear_input_buffer();
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';
            
            strcpy(USERNAME, registerAcc(sockfd, command));
            break;
        
        case 2:
            printf("Enter (username password) for login: \n");
            clear_input_buffer();
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';

            // char *tmp = login(sockfd, command);
            strcpy(USERNAME, login(sockfd, command));
            break;
        case 0:
            printf("Exit\n");
            close(sockfd);
            return 0;
        }

        //  if USERNAME is not empty and not -1 then go to menu that have other function
        if (strcmp(USERNAME, "") != 0 && strcmp(USERNAME, "-1") != 0) {
            printf("\n---------------MENU---------------\n") ;
            printf("1. Create group (command: CREATE_GROUP group_name)\n");
            printf("2. Join group (command: JOIN_GROUP group_name)\n");
            printf("3. Your group\n");
            int select; 
            if (scanf("%d", &select) != 1) {
                printf("STDIN ERROR" );
            }
            switch (select)
            {
                case 1:
                {
                    printf("\nEnter group name you want to create: \n");

                    char group_name[20] = {0};
                    clear_input_buffer();
                    fgets(group_name, sizeof(group_name), stdin);
                    group_name[strcspn(group_name, "\n")] = '\0';
                    create_group(sockfd, "CREATE_GROUP", USERNAME ,group_name);
                    break;
                }
                case 2:
                {
                    
                    char group_list[(MAX_GROUP + 1) * 20] = {0};
                    send_command(sockfd, "NOT_JOINED_GROUP", USERNAME); 
                    recv(sockfd, group_list, sizeof(group_list), 0);
                    printf("List of groups you can join: \n%s\n", group_list);

                    printf("Enter group name you want to join: \n");

                    char group_name[20] = {0};
                    clear_input_buffer();
                    fgets(group_name, sizeof(group_name), stdin);
                    group_name[strcspn(group_name, "\n")] = '\0';
                    join_group(sockfd, USERNAME, group_name);
                    break;

                }
                case 3:
                {
                    group_function(sockfd, USERNAME);
                }

                default:
                    break;
            }
        }

    }
    close(sockfd);
    return 0;
}
