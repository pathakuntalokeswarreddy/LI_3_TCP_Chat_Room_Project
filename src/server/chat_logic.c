#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../../include/server.h"
#include <sys/socket.h>

void handle_list_users(int client_fd);
void notify_all(const char *username, const char *status, int sender_fd);
void perform_cleanup(const char *user, int fd);


void perform_cleanup(const char *user, int fd)
{
    printf("[INFO] %s logging out.\n", user);
    notify_all(user, "left", fd);
    handle_logout(fd);
    close(fd);
}


/* Helper to notify everyone when someone joins or leaves */
void notify_all(const char *username, const char *status, int sender_fd) 
{
    chat_packet_t notification;
    notification.type = MSG_GROUP;
    strcpy(notification.username, "SERVER");
    snprintf(notification.payload, sizeof(notification.payload), "--- %s has %s the chat ---", username, status);

    handle_broadcast(sender_fd, &notification);
}

/* Client Handler Thread */
void *client_handler(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg); 
    
    chat_packet_t pkt;
    int authenticated = 0;
    char current_user[NAME_LEN] = {0};

    /* 1. Authentication Phase */
    while (!authenticated)
    {
        if (recv(client_fd, &pkt, sizeof(chat_packet_t), 0) <= 0)
        {
            close(client_fd);
            return NULL;
        }
  
        if (pkt.type == REQ_REGISTER)
        {
            if (save_user(pkt.username, pkt.password))
            {
                pkt.type = RES_SUCCESS;
                strcpy(pkt.payload, "Registration Successful! Please Login.");
            }
            else
            {
                pkt.type = RES_FAILURE;
                strcpy(pkt.payload, "User already exists or DB Error.");
            }
            send(client_fd, &pkt, sizeof(chat_packet_t), 0);
        } 
        else if (pkt.type == REQ_LOGIN)
        {
            if (validate_user(pkt.username, pkt.password))
            {
                int check_flag=0;
                                
                /* Check if user is already in Login */
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if(strcmp(online_users[i].username,pkt.username)==0 && 
                              online_users[i].is_active)
                    {
                        pkt.type = RES_FAILURE;
                        strcpy(pkt.payload, "User already Logged in.");
                        send(client_fd, &pkt, sizeof(chat_packet_t), 0);
                        check_flag = 1;
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);

                if(check_flag)
                    continue;

                authenticated = 1;
                strncpy(current_user, pkt.username, NAME_LEN);

                /* Add to Online Array */
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (online_users[i].socket_fd == -1)
                    {
                        online_users[i].socket_fd = client_fd;
                        strncpy(online_users[i].username, current_user, NAME_LEN);
                        online_users[i].is_active = 1;
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);

                pkt.type = RES_SUCCESS;
                strcpy(pkt.payload, "Login Successful!");
                send(client_fd, &pkt, sizeof(chat_packet_t), 0);
                
                printf("%s[INFO] %s logged in.%s\n", GREEN, current_user, RESET);
                notify_all(current_user, "joined", client_fd);
            }
            else
            {
                pkt.type = RES_FAILURE;
                strcpy(pkt.payload, "Invalid Username or Password.");
                send(client_fd, &pkt, sizeof(chat_packet_t), 0);
            }
        }
    }

    /* 2. Chatting Phase */
    while (1)
    {
        if (recv(client_fd, &pkt, sizeof(chat_packet_t), 0) <= 0)
        {
            perform_cleanup(current_user, client_fd);
            return NULL;
        }

        switch (pkt.type)
        {
            case MSG_GROUP:
                handle_broadcast(client_fd, &pkt);
                break;

            case MSG_PRIVATE:
                handle_private_chat(client_fd, &pkt);
                break;

            case LIST_ONLINE:
                handle_list_users(client_fd);
                break;

            case REQ_LOGOUT:
                perform_cleanup(current_user, client_fd);
                return NULL;

            default:
                break;
        }
    }  
}

/* Logic for Group Chat */
void handle_broadcast(int sender_fd, chat_packet_t *pkt)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // Send to everyone EXCEPT the sender
        if (online_users[i].socket_fd != -1 && online_users[i].socket_fd != sender_fd) 
        {
            if (send(online_users[i].socket_fd, pkt, sizeof(chat_packet_t), 0) <= 0)
            {
                online_users[i].socket_fd = -1; // Mark as disconnected
                online_users[i].is_active = 0;
                memset(online_users[i].username, 0, NAME_LEN);
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Logic for Single Chat */
void handle_private_chat(int sender_fd, chat_packet_t *pkt)
{
    int found = 0;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (online_users[i].is_active && strcmp(online_users[i].username, pkt->target) == 0)
        {
            if (send(online_users[i].socket_fd, pkt, sizeof(chat_packet_t), 0) <= 0)
            {
                online_users[i].socket_fd = -1;
                online_users[i].is_active = 0;
                memset(online_users[i].username, 0, NAME_LEN);
            }
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (!found) {
        chat_packet_t error_pkt;
        error_pkt.type = RES_FAILURE;
        strcpy(error_pkt.username, "SERVER");
        snprintf(error_pkt.payload, sizeof(error_pkt.payload), "User %s is not online.", pkt->target);
        send(sender_fd, &error_pkt, sizeof(chat_packet_t), 0);
    }
}

/* Logic to send list of online users to a client */
void handle_list_users(int client_fd) 
{
    chat_packet_t list_pkt;
    list_pkt.type = LIST_ONLINE;
    strcpy(list_pkt.username, "SERVER");

    strcpy(list_pkt.payload, "Online: ");

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        if (online_users[i].socket_fd != -1) 
        {
            strcat(list_pkt.payload, online_users[i].username);
            strcat(list_pkt.payload, ", ");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    send(client_fd, &list_pkt, sizeof(chat_packet_t), 0);
}

/* Logic for Logout */
void handle_logout(int client_fd)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (online_users[i].socket_fd == client_fd)
        {
            online_users[i].socket_fd = -1;
            online_users[i].is_active = 0;
            memset(online_users[i].username, 0, NAME_LEN);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}