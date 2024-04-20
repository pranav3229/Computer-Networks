#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port %d\n", PORT);

    while (1) {
        printf("Enter command (put/get/del key value, Bye to exit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Send command to server
        send(client_socket, buffer, strlen(buffer), 0);

        // Receive reply from server
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("Server reply: %s\n", buffer);

        // Check for termination condition
        if (strcmp(buffer, "Goodbye") == 0) {
            close(client_socket);

            return 0;
            break;
        }
    }

    // Close client socket
    close(client_socket);

    return 0;
}
