#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
#if defined WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) return -1;
#endif

    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    // Point 1: Automatic connection
    if (connect(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed.\n");
        clearwinsock();
        return -1;
    }

    char input[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    while (1) {
        // Point 3 & 6: Single read from standard input (e.g., "n 8")
        printf("Enter request (n/a/m/s length) or 'q' to quit: ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        // Point 7: Quit condition
        if (strcmp(input, "q") == 0) {
            send(my_socket, "q", 1, 0);
            break;
        }

        // Point 4: Send request
        send(my_socket, input, strlen(input), 0);

        // Point 5: Read and display password
        memset(password, 0, BUFFER_SIZE);
        if (recv(my_socket, password, BUFFER_SIZE - 1, 0) > 0) {
            printf("Password: %s\n", password);
        } else {
            break;
        }
    }

    closesocket(my_socket);
    printf("Client terminated.\n"); // Point 7 requirement
    clearwinsock();
    return 0;
}