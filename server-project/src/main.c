/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>

typedef int socklen_t;
#define strcasecmp _stricmp
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"


#ifndef NO_ERROR
#define NO_ERROR 0
#endif

static const char *SUPPORTED_CITIES[] = {
	"Bari", "Roma", "Milano", "Napoli", "Torino",
	"Palermo", "Genova", "Bologna", "Firenze", "Venezia"
};

void errorhandler(char *errorMessage) {
    printf("%s\n", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

float random_float(float min, float max) {
	float scale = rand() / (float) RAND_MAX;
	return min + scale * (max - min);
}


void valida(weather_request_t *req, weather_response_t *resp) {

	if(req->type != 't' && req->type != 'h' && req->type != 'w' && req->type != 'p') {
		resp->status = 2;
		return;
	}


	int flag = 1;
	for (int i = 0; i < 10; i++) {
		if(strcasecmp(req->city, SUPPORTED_CITIES[i]) == 0) {
			flag = 0;
			break;
		}
	}

	if(flag == 1) {
		resp->status = 1;
	} else {
		resp->status = 0;
	}
}

float get_temperature(void) { return random_float(-10.0, 40.0); }
float get_humidity(void)    { return random_float(20.0, 100.0); }
float get_wind(void)        { return random_float(0.0, 100.0); }
float get_pressure(void)    { return random_float(950.0, 1050.0); }

int main(int argc, char *argv[]) {
	#if defined WIN32
		WSADATA wsa_data;
		if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
			printf("Error at WSAStartup()\n");
			return 0;
		}
	#endif

	int port = SERVER_PORT;
	if (argc > 2 && strcmp(argv[1], "-p") == 0) {
		port = atoi(argv[2]);
	}

	int my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = htonl(INADDR_ANY);
	sad.sin_port = htons(port);

	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	if (listen(my_socket, QUEUE_SIZE) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	struct sockaddr_in cad;
	int client_socket;
	int client_len;

	srand(time(NULL));
	printf("Server in ascolto sulla porta %d...\n", port);

	while (1) {
		client_len = sizeof(cad);

		if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, (socklen_t*)&client_len)) < 0) {
			errorhandler("accept() failed.\n");
			continue;
		}

		weather_request_t request;
		if (recv(client_socket, (char*)&request, sizeof(request), 0) <= 0) {
			closesocket(client_socket);
			continue;
		}

		printf("Richiesta '%c %s' dal client ip %s\n", request.type, request.city, inet_ntoa(cad.sin_addr));

		weather_response_t response;
		valida(&request, &response);

		if(response.status == 0) {
			switch (request.type) {
				case 't': response.value = get_temperature(); break;
				case 'h': response.value = get_humidity(); break;
				case 'w': response.value = get_wind(); break;
				case 'p': response.value = get_pressure(); break;
			}
			response.type = request.type;
		} else {
			response.type = '\0';
			response.value = 0.0;
		}

		if (send(client_socket, (char*)&response, sizeof(response), 0) != sizeof(response)) {
			errorhandler("send() failed");
		}

		closesocket(client_socket);
	}

	closesocket(my_socket);
	clearwinsock();
	return 0;
}
