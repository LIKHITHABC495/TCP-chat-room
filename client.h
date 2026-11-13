#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

extern int client_fd;
extern char msg[BUFFER_SIZE];

void *receive_messages(void *arg);

void register_client(void);
void login_client(void);
void single_chat(void);
void group_chat(void);
void logout_client(void);

#endif