#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 1024

void receive_file(int server_socket, const char *file_path) {
    char buffer[MAX_BUFFER_SIZE];
    FILE *file;

    // Open the file for writing
    file = fopen(file_path, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Receive and write the file content
    int bytes_received;
    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
    printf("File received successfully.\n");
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port %d\n", PORT);


    

    // Send a file request to the server
    const char *file_request = "GET sample.txt 9999";  // Replace with the file you want to request
    send(client_socket, file_request, strlen(file_request), 0);

    // Receive the requested file from the server
    receive_file(client_socket, file_request);

    // Close the client socket
    close(client_socket);

    return 0;
}


void request(int sockfd, const char *filename, int data_port) {
    char request[MAX_BUFFER_SIZE];
    sprintf(request, "GET %s %d", filename, data_port);
    send(sockfd, request, strlen(request), 0);
}

