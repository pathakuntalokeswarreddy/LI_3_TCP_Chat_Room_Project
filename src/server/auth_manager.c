#include "../../include/server.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h> // For flock


/* Validate user credentials*/
int validate_user(const char *user, const char *pass)
{
    int fd = open(USER_FILE, O_RDONLY);
    if (fd == -1)
        return 0; // File doesn't exist yet

    char buffer[4096];

    char file_user[NAME_LEN], file_pass[PASS_LEN];

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if(bytes_read <= 0)
        return 0; // Error reading file or file is empty

    buffer[bytes_read] = '\0'; // Null-terminate the buffer

    char *line = strtok(buffer, "\n");
    while (line != NULL)
    {
        if (sscanf(line, "%31[^:]:%31s", file_user, file_pass) == 2)
        {
            if (strcmp(file_user, user) == 0 && strcmp(file_pass, pass) == 0 )
                return 1; // User validated successfully
        }
        line = strtok(NULL, "\n");
    }
    return 0; // No match found
}



/* Check if user exists*/
int user_exists(const char *user) 
{
    int fd = open(USER_FILE, O_RDONLY);
    if (fd == -1) 
        return 0;

    char buffer[4096];
    char file_user[NAME_LEN], file_pass[PASS_LEN];

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read <= 0) 
        return 0;

    buffer[bytes_read] = '\0';

    char *line = strtok(buffer, "\n");
    while (line != NULL) 
    {
        if (sscanf(line, "%31[^:]:%31s", file_user, file_pass) == 2) 
        {
            if (strcmp(file_user, user) == 0) 
                return 1;
        }
        line = strtok(NULL, "\n");
    }
    return 0;
}


/* Save user credentials*/
int save_user(const char *user, const char *pass)
{ 
    // First check if user already exists
    if (user_exists(user)) 
    {
        return 0; // User already exists
    }

    int fd = open(USER_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("Error opening user file for writing");
        return 0; // Error opening file
    }

    if (flock(fd, LOCK_EX) == -1)
    {
        perror("Lock failed");
        close(fd);
        return 0;
    }

    char entry[128];
    int len = snprintf(entry, sizeof(entry), "%s:%s\n", user, pass);

    if (write(fd, entry, len) != len)
    {
        perror("Error writing to user file");
        close(fd);
        return 0; // Error writing to file
    }

    flock(fd, LOCK_UN);
    close(fd);
    return 1; // User saved successfully
}