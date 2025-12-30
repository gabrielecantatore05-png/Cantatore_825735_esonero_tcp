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
#include <time.h>
#include <ctype.h>
#include "protocol.h"

// --- Password Generation Functions (Requirements) ---

void generate_numeric(int len, char* out) {
    const char charset[] = "0123456789";
    for (int i = 0; i < len; i++) out[i] = charset[rand() % 10];
    out[len] = '\0';
}

void generate_alpha(int len, char* out) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < len; i++) out[i] = charset[rand() % 26];
    out[len] = '\0';
}

void generate_mixed(int len, char* out) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < len; i++) out[i] = charset[rand() % 36];
    out[len] = '\0';
}

void generate_secure(int len, char* out) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
    int size = strlen(charset);
    for (int i = 0; i < len; i++) out[i] = charset[rand() % size];
    out[len] = '\0';
}

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
    printf("%s", errorMessage);
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

#if defined WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
        printf("Error at WSAStartup()\n");
        return 0;
    }
#endif

    // Create welcome socket
    int my_socket;
    my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (my_socket < 0) {
        errorhandler("socket creation failed.\n");
        clearwinsock();
        return -1;
    }

    // Set connection settings
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    sad.sin_port = htons(PROTO_PORT);

    if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        errorhandler("bind() failed.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    // Point 8: Listen with QLEN (5)
    if (listen(my_socket, QLEN) < 0) {
        errorhandler("listen() failed.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    struct sockaddr_in cad;
    int client_socket;
    int client_len;

    printf("Server started. Waiting for connections on %s:%d...\n", SERVER_ADDRESS, PROTO_PORT);

    while (1) {
        client_len = sizeof(cad);
        if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0) {
            errorhandler("accept() failed.\n");
            continue;
        }

        // Point 2: Specific output format required
        printf("New connection from %s:%d\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

        char buffer[BUFFER_SIZE];
        while (1) {
            memset(buffer, '\0', BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

            // Point 7: Check for disconnection or 'q'
            if (bytes_received <= 0 || buffer[0] == 'q') {
                break;
            }

            char type;
            int length;
            char password[MAX_PASS_LEN + 1];

            // Point 4: Parse request "type length"
            if (sscanf(buffer, "%c %d", &type, &length) == 2) {
                // Check requirements (length 6-32)
                if (length >= MIN_PASS_LEN && length <= MAX_PASS_LEN) {
                    switch (type) {
                        case 'n': generate_numeric(length, password); break;
                        case 'a': generate_alpha(length, password); break;
                        case 'm': generate_mixed(length, password); break;
                        case 's': generate_secure(length, password); break;
                        default: strcpy(password, "Error: invalid type"); break;
                    }
                    // Send generated password back to client
                    send(client_socket, password, strlen(password), 0);
                } else {
                    char* msg = "Error: invalid length (6-32)";
                    send(client_socket, msg, strlen(msg), 0);
                }
            }
        }

        closesocket(client_socket);
        printf("Connection closed.\n");
    }

    closesocket(my_socket);
    clearwinsock();
    return 0;
}