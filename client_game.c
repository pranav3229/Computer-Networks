#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 32

int main() {
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("Error opening a socket\n");
        exit(0);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("172.27.143.13");

    int c = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (c < 0) {
        printf("Error while establishing connection\n");
        exit(0);
    }

    char recvBuffer[BUFSIZE];
    int bytesRecvd = recv(sock, recvBuffer, BUFSIZE-1, 0);
    if (bytesRecvd < 0) {
        printf("Error while receiving data from server\n");
        exit(0);
    }
    recvBuffer[bytesRecvd] = '\0';
    printf("%s\n", recvBuffer);

    int guessedNumber;
    printf("Enter your guess: ");
    scanf("%d", &guessedNumber);

    int bytesSent = send(sock, &guessedNumber, sizeof(guessedNumber), 0);
    if (bytesSent != sizeof(guessedNumber)) {
        printf("Error while sending the guess to server\n");
        exit(0);
    }

    bytesRecvd = recv(sock, recvBuffer, BUFSIZE-1, 0);
    if (bytesRecvd < 0) {
        printf("Error while receiving data from server\n");
        exit(0);
    }
    recvBuffer[bytesRecvd] = '\0';
    printf("%s\n", recvBuffer);

    close(sock);

    return 0;
}
