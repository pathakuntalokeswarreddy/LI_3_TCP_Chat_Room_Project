#include <stdio.h>
#include <stdlib.h>      // For exit()
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>  // For send() and recv()
#include <pthread.h>
#include "../../include/client.h"

/* Display the main menu */
void display_main_menu() 
{
    printf("\n%s%s--- MAIN MENU ---%s\n", BOLD, BLUE, RESET);
    printf("%s1.%s Register\n", YELLOW, RESET);
    printf("%s2.%s Login\n", YELLOW, RESET);
    printf("%s3.%s Exit\n", YELLOW, RESET);
}

/* Register a new user */
void do_register(int sock)
{ 
    chat_packet_t pkt;
    pkt.type = REQ_REGISTER;

    printf("Enter new username: ");
    scanf("%31s", pkt.username);
    getchar(); // Clear newline

    printf("Enter new password: ");
    scanf("%31s", pkt.password);
    getchar(); // Clear newline
    
    if (send(sock, &pkt, sizeof(pkt), 0) <= 0)
    {
        perror("Send failed");
        return;
    }

    if (recv(sock, &pkt, sizeof(pkt), 0) <= 0)
    {
        perror("Receive failed");
        return;
    }

    printf("[SERVER] %s\n", pkt.payload);
}

/* Login a user */
int do_login(int sock) 
{
    chat_packet_t pkt;
    pkt.type = REQ_LOGIN;

    printf("Username: ");
    scanf("%31s", pkt.username);
    getchar(); // Clear newline

    printf("Password: ");
    scanf("%31s", pkt.password);
    getchar(); // Clear newline

    if (send(sock, &pkt, sizeof(pkt), 0) <= 0)
    {
        perror("Send failed");
        return 0;
    }
    if (recv(sock, &pkt, sizeof(pkt), 0) <= 0)
    {
        perror("Receive failed");
        return 0;
    }
    
    if (pkt.type == RES_SUCCESS) 
    {
        printf("[SUCCESS] %s\n", pkt.payload);
        strcpy(my_username, pkt.username);
        return 1;
    }

    printf("[ERROR] %s\n", pkt.payload);
    return 0;
}


/* Receive thread function */
void *receive_thread_func(void *arg) 
{
    chat_packet_t pkt;
    while (recv(sock_fd, &pkt, sizeof(pkt), 0) > 0)
    {
        if (pkt.type == MSG_GROUP)  
        {
            printf("\n%s[GROUP] %s: %s%s\n", CYAN, pkt.username, pkt.payload, RESET);
        } 
        else if (pkt.type == MSG_PRIVATE)
        {
            printf("\n%s[PRIVATE] %s: %s%s\n", MAGENTA, pkt.username, pkt.payload, RESET);
        }
        else if (pkt.type == LIST_ONLINE)
        {
            printf("\n%s[SYSTEM] %s%s\n", YELLOW, pkt.payload, RESET);
        }
        else if (pkt.type == RES_FAILURE)
        {
            printf("\n%s[ERROR] %s%s\n", RED, pkt.payload, RESET);
        }
    
        fflush(stdout);
    }

    printf("\n%s[SYSTEM] Disconnected from server.%s\n", RED, RESET);
    close(sock_fd);
    pthread_exit(NULL);
    return NULL;
}


/* Main chat loop */
void handle_chat_loop() 
{
    chat_packet_t pkt;
    int chat_choice;

    while(1)
    {
        printf("\n%s%s--- CHAT OPTIONS ---%s\n", BOLD, BLUE, RESET);
        printf("%s1. Group Chat.%s\n", MAGENTA, RESET);
        printf("%s2. Private Chat.%s\n", MAGENTA, RESET);
        printf("%s3. See Online Users%s\n", MAGENTA, RESET);
        printf("%s4. Logout%s\n", MAGENTA, RESET);
        printf("%sEnter Choice:%s ", BOLD, RESET);

        if (scanf("%d", &chat_choice) != 1)
        {
            while (getchar() != '\n'); // Clear buffer
            continue;
        }
        getchar(); // Clear newline

        if (chat_choice == 4) 
        {
            pkt.type = REQ_LOGOUT;
            if (send(sock_fd, &pkt, sizeof(pkt), 0) <= 0)
                perror("Send failed");

            break;
        }

        if (chat_choice == 3) 
        {
            pkt.type = LIST_ONLINE;
            if (send(sock_fd, &pkt, sizeof(pkt), 0) <= 0)
                perror("Send failed");
            
            usleep(100000);  // Wait for receive thread to print the list
            continue; 
        }

        
        if (chat_choice == 1)
            pkt.type = MSG_GROUP;
        else
            pkt.type = MSG_PRIVATE;
        

        strcpy(pkt.username, my_username);

        if (chat_choice == 2)
        {
            printf("Enter Target Username: ");
            scanf("%31s", pkt.target);
            getchar();
        }
        else
        {
            strcpy(pkt.target, "ALL");
        }

        printf("Enter Message: ");
        fgets(pkt.payload, MAX_MSG_LEN, stdin);
        pkt.payload[strcspn(pkt.payload, "\n")] = '\0'; 

        if (send(sock_fd, &pkt, sizeof(pkt), 0) <= 0)
        {
            perror("Send failed");
            break;
        }
        usleep(100000); // Small delay to allow the server to process
    }
}