/*
 * protocol.h
 * Application protocol shared between client and server.
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Default connection parameters (Point 1 & Requirements)
#define SERVER_PORT 27015
#define SERVER_ADDRESS "127.0.0.1"

// Buffer and Queue settings (Point 8)
#define BUFFER_SIZE 512
#define QLEN 5

// Password length constraints (Requirements)
#define MIN_PASS_LEN 6
#define MAX_PASS_LEN 32

// Required generation functions (Requirements)
void generate_numeric(int len, char* out);
void generate_alpha(int len, char* out);
void generate_mixed(int len, char* out);
void generate_secure(int len, char* out);

#endif /* PROTOCOL_H_ */