#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

// Function to handle client requests
void handle_client(int client_socket) {
    // Implement logic to handle client requests
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Bind the socket to the specified port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    // Listen for client connections
    listen(server_fd, 3);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        // Handle client in a separate thread or process
        handle_client(new_socket);
    }

    return 0;
}
