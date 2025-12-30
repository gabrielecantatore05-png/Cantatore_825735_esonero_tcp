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
#include "protocol.h"

#define NO_ERROR 0

// Implementation of required generation functions
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

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

#if defined WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return -1;
    }
#endif

    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket < 0) {
        printf("Socket creation failed.\n");
        clearwinsock();
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    if (bind(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Bind failed.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    if (listen(my_socket, QLEN) < 0) {
        printf("Listen failed.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    printf("Server listening on %s:%d\n", SERVER_ADDRESS, SERVER_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int client_socket = accept(my_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket >= 0) {
            // Point 2: Specific output format
            printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            char buffer[BUFFER_SIZE];
            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                
                // Point 7: Handle disconnection or 'q'
                if (bytes <= 0 || buffer[0] == 'q') break;

                char type;
                int length;
                char password[MAX_PASS_LEN + 1];

                // Point 4: Parse request and generate
                if (sscanf(buffer, "%c %d", &type, &length) == 2) {
                    if (length >= MIN_PASS_LEN && length <= MAX_PASS_LEN) {
                        switch (type) {
                            case 'n': generate_numeric(length, password); break;
                            case 'a': generate_alpha(length, password); break;
                            case 'm': generate_mixed(length, password); break;
                            case 's': generate_secure(length, password); break;
                            default: strcpy(password, "Invalid type"); break;
                        }
                        send(client_socket, password, strlen(password), 0);
                    } else {
                        char* err = "Length must be between 6 and 32";
                        send(client_socket, err, strlen(err), 0);
                    }
                }
            }
            closesocket(client_socket);
        }
    }
    return 0;
}