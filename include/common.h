#ifndef COMMON_H
#define COMMON_H

/* ANSI Color Codes */
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"

/* Network Configuration */
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6333
#define MAX_CLIENTS 10

/* Database Configuration */
#define USER_FILE "data/users.txt"

/* Buffer Sizes */
#define MAX_MSG_LEN 256
#define NAME_LEN 32
#define PASS_LEN 32

/* Message Types to identify the intent of the packet */
typedef enum {
    REQ_REGISTER,   // New user registration
    REQ_LOGIN,      // User login
    REQ_LOGOUT,     // User logout
    MSG_GROUP,      // Broadcast to all
    MSG_PRIVATE,    // Unicast to one
    LIST_ONLINE,    // Request/Receive online user list
    RES_SUCCESS,    // Server generic ACK
    RES_FAILURE     // Server generic NACK
} msg_type_t;
 
/* The primary data structure for socket communication */
typedef struct {
    msg_type_t type;
    char username[NAME_LEN];
    char password[PASS_LEN];
    char target[NAME_LEN];     // Used as 'receiver' for private messages
    char payload[MAX_MSG_LEN];  // The actual text message
} chat_packet_t;

#endif