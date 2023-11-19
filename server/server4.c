#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5550
#define BUFF_SIZE 512
#define FILE_BUFF_SIZE 1024
#define MAX_CLIENTS 100

void handle_file_upload(int client_socket) {
    char filename[BUFF_SIZE];
    int bytes_received = recv(client_socket, filename, BUFF_SIZE, 0);
    if (bytes_received <= 0) return;
    
    FILE *file = fopen(filename, "wb");
    char file_buffer[FILE_BUFF_SIZE];
    
    while ((bytes_received = recv(client_socket, file_buffer, FILE_BUFF_SIZE, 0)) > 0) {
        fwrite(file_buffer, sizeof(char), bytes_received, file);
    }
    
    fclose(file);
}

void handle_file_download(int client_socket, const char* filename) {
    FILE *file = fopen(filename, "rb");
    char file_buffer[FILE_BUFF_SIZE];
    int bytes_read;

    if (file != NULL) {
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFF_SIZE, file)) > 0) {
            send(client_socket, file_buffer, bytes_read, 0);
        }
        fclose(file);
    }
}

void process_client_request(int client_socket) {
    char command[BUFF_SIZE];
    int bytes_read = recv(client_socket, command, BUFF_SIZE - 1, 0);
    if (bytes_read <= 0) return;

    command[bytes_read] = '\0';

    if (strncmp(command, "UPLOAD", 6) == 0) {
        handle_file_upload(client_socket);
    } else if (strncmp(command, "DOWNLOAD", 8) == 0) {
        handle_file_download(client_socket, command + 9);
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

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
    // Socket setup omitted for brevity...

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

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
