#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT 5550
#define BUFF_SIZE 512
#define FILE_BUFF_SIZE 1024
#define MAX_CLIENTS 100


void send_message(int client_socket, const char *message) {
    send(client_socket, message, strlen(message), 0);
}
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
void handle_user_registration(int client_socket) {
    char buffer[BUFF_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFF_SIZE, 0);
    if (bytes_received <= 0) return;

    char username[50], password[50];
    sscanf(buffer, "%s %s", username, password);

    if (check_username_existence(username)) {
        // Username already exists, send a message to the client
        send_message(client_socket, "Username already exists");
    } else {
        // Username does not exist, append buffer to file
        FILE *file = fopen("users.txt", "a");
        if (file != NULL) {
            fputs(buffer, file);
            fputs("\n", file);
            fclose(file);

            // Send success message to the client
            send_message(client_socket, "Registration successful");
        } else {
            // Error opening the file
            send_message(client_socket, "Error during registration");
        }
    }
}

// void handle_user_registration(int client_socket) {
//     // Implement user registration logic
//     // Read user data from socket and save to 'users.txt'

//     char buffer[BUFF_SIZE];
//     int bytes_received = recv(client_socket, buffer, BUFF_SIZE, 0);
//     if (bytes_received <= 0) return;

//     char username[50], password[50];
//     sscanf(buffer, "%s %s", username, password);

//     FILE *file = fopen("users.txt", "a");
//     if (file != NULL) {
//         // check if username already exists in file
        

//         fputs(buffer, file);
//         fputs("\n", file);
//         fclose(file);
//     }
// }

void handle_user_login(int client_socket) {
    // Implement user login logic
    // Check credentials against 'users.txt'
    char buffer[BUFF_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFF_SIZE, 0);
    if (bytes_received <= 0) return;

    FILE *file = fopen("users.txt", "r");
    char line[BUFF_SIZE];
    int found = 0;

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, buffer, strlen(buffer)) == 0) {
                found = 1;
                break;
            }
        }
        fclose(file);
    }
    // Send login status to client
    // For example: send(client_socket, found ? "SUCCESS" : "FAIL", ...)

}

void handle_group_creation(int client_socket) {
    // Implement group creation logic
    // Save group data to 'groups.txt' and 'group_members.txt'
    char buffer[BUFF_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFF_SIZE, 0);
    if (bytes_received <= 0) return;

    FILE *file = fopen("groups.txt", "a");
    if (file != NULL) {
        fputs(buffer, file);
        fputs("\n", file);
        fclose(file);
    }

    // Additional logic to update group_members.txt
    // ...

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

void process_client_request(int client_socket) {
    char command[BUFF_SIZE];
    int bytes_read = recv(client_socket, command, BUFF_SIZE - 1, 0);
    if (bytes_read <= 0) {
        printf("Client disconnected\n");

        return;
    }

    command[bytes_read] = '\0';

    printf("Received command: %s\n", command);
    printf("Processing command...\n");

    if (strcmp(command, "REGISTER") == 0) {
        char *msg = "Pls give me username and password";
        send(client_socket, msg, strlen(msg) , 0);
        handle_user_registration(client_socket);
    } else if (strcmp(command, "LOGIN") == 0) {
        handle_user_login(client_socket);
    } else if (strcmp(command, "CREATE_GROUP") == 0) {
        handle_group_creation(client_socket);
    } else if (strcmp(command, "JOIN_GROUP") == 0) {
        handle_join_group(client_socket);
    } else if (strcmp(command, "UPLOAD_FILE") == 0) {
        handle_file_upload(client_socket);
    } else if (strcmp(command, "DOWNLOAD_FILE") == 0) {
        handle_file_download(client_socket);
    }else {
        // send "INVALID" to client
        send(client_socket, "INVALID", 7, 0);
    }
    // Add other commands as necessary
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

        if (fork() == 0) {
            close(server_fd);
            process_client_request(client_socket);
            close(client_socket);
            exit(0);
        }
        close(client_socket);
    }
    close(server_fd);
    return 0;
}
