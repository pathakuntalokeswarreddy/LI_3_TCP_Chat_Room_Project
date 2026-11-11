#ifndef CLIENT_H
#define CLIENT_H
 
#include "common.h"

/*Global State*/
extern int sock_fd;
extern char my_username[NAME_LEN];

/* Function Prototypes */
void display_main_menu();
void *receive_thread_func(void *arg); // Always listening for server updates

void do_register(int sock);
int do_login(int sock);
void handle_chat_loop();

#endif