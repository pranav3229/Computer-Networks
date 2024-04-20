#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAXPENDING 5
#define BUFFERSIZE 32

int main() {
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("Error creating the socket");
        exit(0);
    }

    printf("Server Socket Created\n");

    struct sockaddr_in serverAddress, clientAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");

    int temp = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (temp < 0) {
        printf("Error while binding\n");
        exit(0);
    }
    printf("Binding successful\n");

    int temp1 = listen(serverSocket, MAXPENDING);
    if (temp1 < 0) {
        printf("Error in listen");
        exit(0);
    }
    printf("Now Listening\n");

    srand(time(NULL)); // Seed for random number generation

    char msg[BUFFERSIZE];
    int clientLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
    if (clientSocket < 0) {
        printf("Error accepting client connection\n");
        exit(0);
    }
    printf("Handling Client %s\n", inet_ntoa(clientAddress.sin_addr));

    // Generate a random number between 1 and 6 for the client to guess
    int randomNumber = (rand() % 6) + 1;
    snprintf(msg, sizeof(msg), "Guess a number between 1 and 6:");

    int bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg)) {
        printf("Error while sending message to client\n");
        exit(0);
    }

    int guessedNumber;
    int temp2 = recv(clientSocket, &guessedNumber, sizeof(guessedNumber), 0);
    if (temp2 < 0) {
        printf("Error receiving data from client\n");
        exit(0);
    }

    if (guessedNumber == randomNumber) {
        snprintf(msg, sizeof(msg), "Correct! You guessed the right number.");
    } else {
        snprintf(msg, sizeof(msg), "Wrong! The number was %d.", randomNumber);
    }

    bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg)) {
        printf("Error while sending message to client\n");
    }

    close(serverSocket);
    close(clientSocket);

    return 0;
}
