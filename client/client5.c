#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 512
#define FILE_BUFF_SIZE 1024

void send_command(int sockfd, const char *command) {
    char buffer[BUFF_SIZE];
    snprintf(buffer, BUFF_SIZE, "%s", command);
    send(sockfd, buffer, strlen(buffer), 0);

    // recv from sever to check if command is valid
    // clear buffer
    memset(buffer, 0, BUFF_SIZE);
    recv(sockfd, buffer, BUFF_SIZE, 0);
    if (strncmp(buffer, "INVALID", 7) == 0) {
        printf("Invalid command\n");
        return;
    }
    printf("Command sent.\nServer response: %s\n", buffer);
}

// void send_command(int sockfd, const char *command, const char *data) {
//     char buffer[BUFF_SIZE];
//     snprintf(buffer, BUFF_SIZE, "%s %s", command, data);
//     send(sockfd, buffer, strlen(buffer), 0);
// }



void registerAcc( int sockfd, char *username, char *password) {
    send_command(sockfd, "REGISTER");
    char buffer[BUFF_SIZE];
    snprintf(buffer, BUFF_SIZE, "%s %s", username, password);
    send(sockfd, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFF_SIZE);
    recv(sockfd, buffer, BUFF_SIZE, 0);
    printf("%s\n", buffer);

}

void upload_file(int sockfd, const char *filename) {
    // send_command(sockfd, "UPLOAD_FILE", filename);

    FILE *file = fopen(filename, "rb");
    char file_buffer[FILE_BUFF_SIZE];
    int bytes_read;

    if (file != NULL) {
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFF_SIZE, file)) > 0) {
            send(sockfd, file_buffer, bytes_read, 0);
        }
        fclose(file);
    }
}

void download_file(int sockfd, const char *filename) {
    // send_command(sockfd, "DOWNLOAD_FILE", filename);

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

    // Example usage
    // send_command(sockfd, "REGISTER", "tiendz tiens");
    registerAcc(sockfd, "tiendz", "tiens");
    // send_command(sockfd, "LOGIN", "john_doe hashed_password");
    // send_command(sockfd, "CREATE_GROUP", "group1 john_doe");
    // send_command(sockfd, "JOIN_GROUP", "group1 jane_smith");
    // upload_file(sockfd, "example.txt");
    // download_file(sockfd, "example.txt");

    close(sockfd);
    return 0;
}
