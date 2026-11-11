#ifndef SERVER_H
#define SERVER_H
 
#include "common.h"
#include <pthread.h>

/* Track online users in memory */
typedef struct {
    int socket_fd;
    char username[NAME_LEN];
    int is_active;
} client_node_t;

/* Global State */
extern client_node_t online_users[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;
extern int server_fd;

/* Function Prototypes */
void *client_handler(void *arg);
void handle_broadcast(int sender_fd, chat_packet_t *pkt);
void handle_private_chat(int sender_fd, chat_packet_t *pkt);
void handle_logout(int client_fd);

/* Database helpers */
int user_exists(const char *user);
int validate_user(const char *user, const char *pass);
int save_user(const char *user, const char *pass);

#endif