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

void upload_file(int sockfd, const char* filename) {
    send(sockfd, "UPLOAD", strlen("UPLOAD"), 0);
    
    FILE *file = fopen(filename, "rb");
    char file_buffer[FILE_BUFF_SIZE];
    int bytes_read;

    if (file != NULL) {
        send(sockfd, filename, strlen(filename), 0); // Send filename
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFF_SIZE, file)) > 0) {
            send(sockfd, file_buffer, bytes_read, 0);
        }
        fclose(file);
    }
}

void download_file(int sockfd, const char* filename) {
    char command[BUFF_SIZE];
    snprintf(command, BUFF_SIZE, "DOWNLOAD %s", filename);
    send(sockfd, command, strlen(command), 0);

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

    // Example usage: Upload and Download a file
    upload_file(sockfd, "example.txt");
    download_file(sockfd, "example.txt");

    close(sockfd);
    return 0;
}
