#ifndef COMMON_H
#define COMMON_H

#define PORT 6333
#define MAX_CLIENTS 10
#define BUFFER_SIZE 100
#define USER_FILE "users.txt"
#define USERNAME_SIZE 30
#define PASSWORD_SIZE 30

#define CMD_REGISTER "REGISTER"
#define CMD_LOGIN    "LOGIN"
#define CMD_SINGLE   "SINGLE"
#define CMD_GROUP    "GROUP"
#define CMD_LOGOUT   "LOGOUT"

typedef struct
{
    char username[USERNAME_SIZE];
    int socket_fd;
    int online;
} User;

#endif