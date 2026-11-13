#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "client.h"

void register_client(void)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    printf("Enter username: ");
    fflush(stdout);
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    printf("Enter password: ");
    fflush(stdout);
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';
    snprintf(msg, sizeof(msg),"%s %s %s\n",CMD_REGISTER,username, password);
    send(client_fd, msg, strlen(msg), 0);
}


void login_client(void)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];

    printf("Enter username: ");
    fflush(stdout);
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    printf("Enter password: ");
    fflush(stdout);
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';
    snprintf(msg, sizeof(msg),"%s %s %s\n",CMD_LOGIN,username, password);
    send(client_fd, msg, strlen(msg), 0);
}


void single_chat(void)
{
    char receiver[USERNAME_SIZE];
    char message[BUFFER_SIZE];

    printf("Enter receiver username: ");
    fflush(stdout);
    fgets(receiver, sizeof(receiver), stdin);
    receiver[strcspn(receiver, "\n")] = '\0';
    printf("Enter message: ");
    fflush(stdout);
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';
    snprintf(msg, sizeof(msg),"%s %s %s\n",CMD_SINGLE,receiver, message);
    send(client_fd, msg, strlen(msg), 0);
}


void group_chat(void)
{
    char message[BUFFER_SIZE];
    printf("Enter group message: ");
    fflush(stdout);
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';
    snprintf(msg, sizeof(msg), "%s %s\n",CMD_GROUP,message);
    send(client_fd, msg, strlen(msg), 0);
}
void logout_client(void)
{
    snprintf(msg, sizeof(msg), "%s\n", CMD_LOGOUT);

    send(client_fd, msg, strlen(msg), 0);
}