#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include<pthread.h>
extern User online_users[MAX_CLIENTS];
extern int online_count;

extern int client_fds[MAX_CLIENTS];
extern int client_count;

extern pthread_mutex_t client_mutex;
extern pthread_mutex_t user_mutex;

void *handle_client(void *arg);
void broadcast_message(char *message, int sender_fd);
void remove_client(int client_fd);

int register_user(char *username, char *password);
int login_user(char *username, char *password);

void update_user_status(char *username, int socket_fd, char *status);
void add_online_user(char *username, int client_fd);
void remove_online_user(int client_fd);
int is_user_online(char *username);

void trim_spaces(char *str);
void send_online_users(int client_fd);
void notify_user_joined(char *username, int sender_fd);
void notify_user_left(char *username, int sender_fd);

int get_user_socket(char *username);
void send_private_message(char *receiver,char *sender,char *message);

void send_group_message(char *sender, char *message);

#endif