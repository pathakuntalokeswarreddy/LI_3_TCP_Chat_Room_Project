#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "../../include/server.h"
 
/* Global State */
client_node_t online_users[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_fd;

void signal_handler(int sig)
{
    printf("\n[INFO] Caught signal %d, shutting down server...\n", sig);
    if (server_fd > 0)
    {
        close(server_fd);
    }
    exit(0);
}

/* Main Function */
int main() 
{
    int new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;

    
    /* Register signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
 

    /* 1. Initialize the online_users array */
    /* We set socket_fd to -1 to indicate an empty slot */
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        online_users[i].socket_fd = -1;
        online_users[i].is_active = 0;
        memset(online_users[i].username, 0, NAME_LEN);
    }
    pthread_mutex_unlock(&clients_mutex);


    /* 2. Create Socket (IPv4, TCP) */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    /* 3. Set Socket Options: Allows immediate restart on the same port */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }


    /* 4. Bind to Port and IP */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }


    /* 5. Start Listening */
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("\n%s======================================%s\n", BLUE, RESET);
    printf("%s   LINUX TCP CHAT SERVER v1.0         %s\n", BOLD, RESET);
    printf("   Port: %d | Max Clients: %d         \n", SERVER_PORT, MAX_CLIENTS);
    printf("%s======================================%s\n", BLUE, RESET);
    printf("%s[SYSTEM]%s Waiting for incoming connections...\n", GREEN, RESET);


    /* 6. Main Accept Loop */
    while (1)
    {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        /* INET_ADDRSTRLEN is a predefined macro provided by the networking header files 
           It specifies the maximum buffer size required to store an IPv4 address in its string representation, 
           including the null terminator.
        */
        char client_ip[INET_ADDRSTRLEN];  
        inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
        printf("%s[INFO] Connection accepted from %s:%d%s\n", YELLOW, client_ip, ntohs(address.sin_port), RESET);

        
        /* 7. Create a dedicated thread for the new client */
        pthread_t tid;
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL)
        {
            perror("Malloc failed");
            close(new_socket);
            continue;
        }

        *client_sock_ptr = new_socket;

        if (pthread_create(&tid, NULL, client_handler, (void*)client_sock_ptr) != 0)
        {
            perror("Thread creation failed");
            free(client_sock_ptr);
            close(new_socket);
        }
        else
        {
            /* Detach the thread so it cleans up automatically upon exit */
            pthread_detach(tid);
        }
    }

    close(server_fd);
    return 0;
}