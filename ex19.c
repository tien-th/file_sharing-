#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>


#define PORT 8000
#define MAX_CLIENTS 1000
int client_sockets[MAX_CLIENTS];


void *client_handler(void *socket) {
    int sock = *(int*)socket;
    char buffer[1024];


    while (1) {
        int read_size = recv(sock, buffer, sizeof(buffer), 0);
        if (read_size <= 0) {
            break;
        }

        
        buffer[read_size] = '\0';

        // Send the message to other clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != 0 && client_sockets[i] != sock) {
                send(client_sockets[i], buffer, read_size + 1, 0); 
            }
        }
    }

    close(sock);
    return 0;
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_fd, MAX_CLIENTS);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = new_socket;
                break;
            }
        }
        pthread_create(&thread_id, NULL, client_handler, (void*)&new_socket);
    }

    return 0;
}


