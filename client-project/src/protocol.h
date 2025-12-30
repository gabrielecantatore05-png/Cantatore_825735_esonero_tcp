/*
 * protocol.h
 * Shared application parameters
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PROTO_PORT 27015
#define SERVER_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 512
#define QLEN 5

#define MIN_PASS_LEN 6
#define MAX_PASS_LEN 32

// Generation functions prototypes
void generate_numeric(int len, char* out);
void generate_alpha(int len, char* out);
void generate_mixed(int len, char* out);
void generate_secure(int len, char* out);

#endif