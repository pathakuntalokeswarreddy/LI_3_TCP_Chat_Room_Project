#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../include/client.h"
 
int sock_fd;
char my_username[NAME_LEN];

int main() 
{
    struct sockaddr_in serv_addr;
    int choice;

    // 1. Create Socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) 
    {
        perror("Invalid address");
        close(sock_fd);
        return -1;
    }

    // 2. Connect to Server
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("Connection Failed");
        close(sock_fd);
        return -1;
    }

    printf("\n--- Welcome to the TCP Chat Room ---\n");

    // 3. Pre-Login Menu Loop
    int logged_in = 0;
    while (!logged_in)
    { 
        display_main_menu();
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear buffer
            continue;
        }
        getchar(); // Clean newline

        if (choice == 1)
        {
            do_register(sock_fd);
        }
        else if (choice == 2)
        {
            if (do_login(sock_fd))
            {
                logged_in = 1;
            }
        }
        else if (choice == 3)
        {
            printf("Exiting...\n");
            close(sock_fd);
            return 0;
        }
        else
        {
            printf("Invalid choice. Try again.\n");
        }
    }

    // 4. Start Background Thread to receive messages
    pthread_t recv_tid;

    if (pthread_create(&recv_tid, NULL, receive_thread_func, NULL) != 0) {
        perror("Could not create receive thread");
        close(sock_fd);
        return -1;
    }

    // Detach so the thread cleans up its own resources
    pthread_detach(recv_tid);

    // 5. Post-Login Chat Loop
    handle_chat_loop();

    // 6. Final Cleanup
    printf("Logging out and closing connection...\n");
    close(sock_fd);
    return 0;
}