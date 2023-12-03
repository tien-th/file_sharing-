#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 1024

int parse_file_request(const char *request, char *filename, int *port) {
    char command[MAX_BUFFER_SIZE];
    int parsed = sscanf(request, "%s %s %d", command, filename, port);
    
    if (parsed == 3 && strcmp(command, "GET") == 0) {
        return 1;  // Correct format
    } else {
        return 0;  // Incorrect format
    }
}


void send_file(int client_socket, const char *file_path) {
    char buffer[MAX_BUFFER_SIZE];
    FILE *file;

    // Open the requested file for reading
    file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Error opening file for reading");
        return;
    }

    // Read and send the file content
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);
    printf("File sent successfully.\n");
}

void * command_thread(void *arg) {
    int client_socket = *((int*)arg);
    free(arg);

    // Receive the file request from the client
    char file_request[MAX_BUFFER_SIZE];
    recv(client_socket, file_request, sizeof(file_request) - 1, 0);
    printf("Received file request: %s\n", file_request);

    if (strncmp(file_request, "QUIT", 4) == 0) {
        close(client_socket);
        return 0;
    } 
    else if (strncmp(file_request, "GET", 3) ) {
        char filename[MAX_BUFFER_SIZE];
        int data_port;
        if (!parse_file_request(file_request, filename, &data_port)) {
        send(client_socket, "Invalid file request", 21, 0);
        close(client_socket);
        continue;
        }
    }
    
    


    
    // Send the requested file to the client
    send_file(client_socket, file_request);
    // Close the client socket
    close(client_socket);
}


int main() {

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept connections and handle file requests
    while (1) {

        // Kênh lệnh
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        
        pthread_t pid;
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = client_socket;
        pthread_create(&pid, NULL, command_thread, (void*)arg);

        
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
