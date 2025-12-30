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

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

int main(int argc, char *argv[]) {
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != 0) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	// Create client socket
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		errorhandler("Socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	// Set connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(SERVER_ADDRESS); // IP defined in protocol.h
	sad.sin_port = htons(PROTO_PORT);              // Port defined in protocol.h

	// Establish connection automatically (Point 1)
	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("Failed to connect.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	char input_buffer[BUFFER_SIZE];
	char response_buffer[BUFFER_SIZE];

	// Communication loop
	while (1) {
		// Get request from user (Point 3 & 6)
		printf("Enter request (type length, e.g., 'n 8') or 'q' to quit: ");
		
		// Use fgets to read the whole line in a single read (Requirement)
		if (fgets(input_buffer, BUFFER_SIZE, stdin) == NULL) break;
		
		// Remove newline character
		input_buffer[strcspn(input_buffer, "\n")] = 0;

		// Check for quit command (Point 7)
		if (strcmp(input_buffer, "q") == 0) {
			send(c_socket, "q", 1, 0);
			break;
		}

		// Send request to server (Point 4)
		if (send(c_socket, input_buffer, strlen(input_buffer), 0) <= 0) {
			errorhandler("send() failed.\n");
			break;
		}

		// Receive generated password from server (Point 5)
		memset(response_buffer, '\0', BUFFER_SIZE);
		if (recv(c_socket, response_buffer, BUFFER_SIZE - 1, 0) <= 0) {
			errorhandler("recv() failed or connection closed.\n");
			break;
		}

		// Display the password on standard output
		printf("Generated password: %s\n", response_buffer);
	}

	// Close and cleanup
	closesocket(c_socket);
	printf("Client terminated.\n");
	clearwinsock();
	return 0;
} // main end.