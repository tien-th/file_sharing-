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

void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void process_file_upload(int client_socket, char* filename) {
    FILE *file = fopen(filename, "wb");
    char file_buffer[FILE_BUFF_SIZE];
    int bytes_received;

    if (file != NULL) {
        while ((bytes_received = recv(client_socket, file_buffer, FILE_BUFF_SIZE, 0)) > 0) {
            fwrite(file_buffer, sizeof(char), bytes_received, file);
            if (bytes_received < FILE_BUFF_SIZE) break; // End of file
        }
        fclose(file);

        // Update files_info.txt
        // You need to implement update_files_info(filename, additional_info);
    } else {
        perror("Error opening file");
    }
}

void process_client_request(int client_socket) {
    char buffer[BUFF_SIZE];
    int bytes_read = recv(client_socket, buffer, BUFF_SIZE - 1, 0);
    if (bytes_read <= 0) return;

    buffer[bytes_read] = '\0';
    // Parse the command from the client
    // For example, let's assume the command is "UPLOAD <filename>"
    if (strncmp(buffer, "UPLOAD ", 7) == 0) {
        char* filename = buffer + 7;
        process_file_upload(client_socket, filename);
    }

    // Add other command processing (e.g., LOGIN, LOGOUT, etc.)
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

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
