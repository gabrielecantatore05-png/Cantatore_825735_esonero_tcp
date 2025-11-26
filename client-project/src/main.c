/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

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
#include <ctype.h>
#include "protocol.h"

void errorhandler(char *errorMessage) {
    printf("%s\n", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {

#if defined WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
		printf("Errore in WSAStartup()\n");
		return 0;
	}
#endif

	int port = SERVER_PORT;
	char ip[32] = DEFAULT_IP;
	char request_string[128] = "";
	int richiesta = 0;

	weather_request_t request;
	memset(&request, 0, sizeof(request));


	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
			strncpy(ip, argv[++i], 31);
		} else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
			port = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
			strncpy(request_string, argv[++i], 127);
			richiesta = 1;
		}
	}

	if (!richiesta) {
		printf("Uso: %s [-s server] [-p port] -r \"tipo città\"\n", argv[0]);
		clearwinsock();
		return -1;
	}


	int i = 0;
	while (request_string[i] == ' ' && request_string[i] != '\0') i++;

	request.type = request_string[i];
	i++;

	while (request_string[i] == ' ' && request_string[i] != '\0') i++;

	strncpy(request.city, &request_string[i], 63);


	int c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		errorhandler("Creazione del socket fallita.\n");
		clearwinsock();
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(ip);
	sad.sin_port = htons(port);

	if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		errorhandler("Connessione fallita.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}


	if(send(c_socket, (char*)&request, sizeof(request), 0) != sizeof(request)) {
		errorhandler("send() ha inviato un numero di byte diverso dall'atteso");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}


	weather_response_t response;
	if ((recv(c_socket, (char*)&response, sizeof(response), 0)) <= 0) {
		errorhandler("recv() fallita o connessione chiusa prematuramente");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}


	printf("Ricevuto risultato dal server ip %s. ", ip);

	if (response.status == 0) {
		request.city[0] = toupper(request.city[0]);
		switch (response.type) {
			case 't': printf("%s: Temperatura = %.1f°C\n", request.city, response.value); break;
			case 'h': printf("%s: Umidità = %.1f%%\n", request.city, response.value); break;
			case 'w': printf("%s: Vento = %.1f km/h\n", request.city, response.value); break;
			case 'p': printf("%s: Pressione = %.1f hPa\n", request.city, response.value); break;
			default:  printf("Tipo sconosciuto ricevuto.\n");
		}
	} else if (response.status == 1) {
		printf("Città non disponibile\n");
	} else if (response.status == 2) {
		printf("Richiesta non valida\n");
	} else {
		printf("Errore sconosciuto\n");
	}

	closesocket(c_socket);

	clearwinsock();
	return 0;
}