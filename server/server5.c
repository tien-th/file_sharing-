#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h> 

#define PORT 9998
#define BUFF_SIZE 512
#define FILE_BUFF_SIZE 1024
#define MAX_CLIENTS 100

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;


int check_username_existence(const char *username) {
    FILE *file = fopen("users.txt", "r");
    if (file != NULL) {
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), file) != NULL) {
            char stored_username[50];
            sscanf(line, "%s", stored_username);

            if (strcmp(username, stored_username) == 0) {
                fclose(file);
                return 1; // Username exists
            }
        }
        fclose(file);
    }
    return 0; // Username does not exist
}


// utils.c
void send_message(int client_socket, const char *message) {
    send(client_socket, message, strlen(message), 0);
}

void handle_user_registration(int client_socket, char *command) {
    // Implement user registration logic
    // Read user data from socket and save to 'users.txt'
    char REG[10] = {0};
    char username[20] = {0};
    char password[20] = {0};
    sscanf(command, "%s %s %s", REG, username, password);

    if (check_username_existence(username)) {
        // Username already exists, send a message to the client
        send_message(client_socket, "-1");
    } else {
        // Username does not exist, append buffer to file
        FILE *file = fopen("users.txt", "a");
        if (file != NULL) {
            fputs(username, file);
            fputs(" ", file); // Add a space between username and password
            fputs(password, file);
            fputs("\n", file);
            fclose(file);

            // Send success message to the client
            send_message(client_socket, username );
        } else {
            // Error opening the file
            send_message(client_socket, "0");
        }
    }

}

void handle_user_login(int client_socket, char *command) {
    // Implement user login logic
    
    // Send success or failure response to the client

    char LOG[10] = {0};
    char username[20];
    char password[20];
    sscanf(command, "%s %s %s", LOG, username, password);   
    // Check credentials against 'users.txt' 
    FILE *file = fopen("users.txt", "r");
    if (file != NULL) {
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), file) != NULL) {
            char stored_username[20];
            char stored_password[20];
            sscanf(line, "%s %s", stored_username, stored_password);

            if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0) {
                fclose(file);
                send_message(client_socket, username);
                return;
            }
        }
        fclose(file);
    }
    send_message(client_socket, "-1");
}

void handle_group_creation(int client_socket, char *command) {
    // Implement group creation logic
    // Save group data to 'groups.txt' and 'group_members.txt'

    char CREATE_GROUP[20] = {0};
    char group_name[20] = {0};
    char username[20] = {0};
    sscanf(command, "%s %s %s", CREATE_GROUP, username, group_name);

    FILE *file = fopen("groups.txt", "a");
    if (file != NULL) {
        fputs(group_name, file);
        fputs(" ", file);
        fputs(username, file);
        fputs("\n", file);
        fclose(file);
    }

    file = fopen("group_members.txt", "a");
    if (file != NULL) {
        fputs(group_name, file);
        fputs(" ", file);
        fputs(username, file);
        fputs("\n", file);
        fclose(file);
    }

}

void handle_join_group(int client_socket) {
    // Implement logic for a user to join a group
    // Update 'group_members.txt'
    char buffer[BUFF_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFF_SIZE, 0);
    if (bytes_received <= 0) return;

    FILE *file = fopen("group_members.txt", "a");
    if (file != NULL) {
        fputs(buffer, file);
        fputs("\n", file);
        fclose(file);
    }

}

void handle_file_upload(int client_socket) {
    // Implement file upload logic
    // Save files to the respective group folder
    char filename[BUFF_SIZE];
    recv(client_socket, filename, BUFF_SIZE, 0);

    FILE *file = fopen(filename, "wb");
    char file_buffer[FILE_BUFF_SIZE];

    if (file != NULL) {
        int bytes_received;
        while ((bytes_received = recv(client_socket, file_buffer, FILE_BUFF_SIZE, 0)) > 0) {
            fwrite(file_buffer, sizeof(char), bytes_received, file);
        }
        fclose(file);
    }

    // Additional logic to update files_info.txt
    // ...

}

void handle_file_download(int client_socket) {
    // Implement file download logic
    // Send files from the respective group folder
    char filename[BUFF_SIZE];
    recv(client_socket, filename, BUFF_SIZE, 0);

    FILE *file = fopen(filename, "rb");
    char file_buffer[FILE_BUFF_SIZE];

    if (file != NULL) {
        int bytes_read;
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFF_SIZE, file)) > 0) {
            send(client_socket, file_buffer, bytes_read, 0);
        }
        fclose(file);
    }
}

void * process_client_request(void *arg) {
    int client_socket = *((int*)arg);
    free(arg);

    char command[BUFF_SIZE] = {0};

    while(1) {
        memset(command, 0, BUFF_SIZE);
        
        printf("Waiting for command...\n");
        
        int bytes_read = recv(client_socket, command, BUFF_SIZE - 1, 0);
        if (bytes_read <= 0) return 0;

        // command[bytes_read] = '\0';

        printf("Received command: %s\n", command);

        if (strncmp(command, "REG", 3) == 0) {
            handle_user_registration(client_socket, command);
        } else if (strncmp(command, "LOGIN", 5) == 0) {
            handle_user_login(client_socket, command);
        } else if (strncmp(command, "CREATE_GROUP", 12) == 0) {
            handle_group_creation(client_socket, command);
        } else if (strcmp(command, "JOIN_GROUP") == 0) {
            handle_join_group(client_socket);
        } else if (strcmp(command, "UPLOAD_FILE") == 0) {
            handle_file_upload(client_socket);
        } else if (strcmp(command, "DOWNLOAD_FILE") == 0) {
            handle_file_download(client_socket);
        }
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Socket setup...
    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Bind the socket to the specified port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for client connections
    listen(server_fd, MAX_CLIENTS);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        printf("\nNew connection\n");

        pthread_t pid;
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = client_socket;
        pthread_create(&pid, NULL, process_client_request, (void*)arg);
    }
    close(server_fd);
    return 0;
}


// Kiểm soát quyền truy cập ??? 
// Xử lý file có kích thước lớn bất kỳ ??? 
// Liệt kê danh sách nhóm 
// Liệt kê danh sách thành viên trong nhóm
// Yêu cầu tham gia một nhóm và phê duyệt
// Thoát khỏi một nhóm
// Mời tham gia một nhóm và phê duỵet, Xóa thành viên khỏi nhóm
// Liệt kê nội dung thư mục 
//  Ghi log 
