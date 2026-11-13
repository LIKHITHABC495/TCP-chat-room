#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "server.h"
User online_users[MAX_CLIENTS];
int online_count=0;

int client_fds[MAX_CLIENTS];
int client_count=0;

pthread_mutex_t client_mutex=PTHREAD_MUTEX_INITIALIZER;;
pthread_mutex_t user_mutex=PTHREAD_MUTEX_INITIALIZER;;
void broadcast_message(char*message,int sender_fd)
{
    pthread_mutex_lock(&client_mutex);
    for(int i=0;i<client_count;i++)
    {
        if(client_fds[i]!=sender_fd)
        {
            send(client_fds[i],message,strlen(message),0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}
void trim_spaces(char *str)
{
    char *start = str;
    char *end;

    while(*start == ' '||*start == '\t')
    {
        start++;
    }

    if(start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }

    end=str+strlen(str)-1;

    while(end >= str&&(*end == ' ' ||*end == '\t'||*end == '\n'||*end == '\r'))
    {
        *end = '\0';
        end--;
    }
}

void remove_client(int client_fd)
{
    pthread_mutex_lock(&client_mutex);
    for(int i=0;i<client_count;i++)
    {
        if(client_fds[i]==client_fd)
        {
            for(int j=i;j<client_count-1;j++)
            {
                client_fds[j]=client_fds[j+1];
            }
            client_count--;
            break;

        }
        
    }
    pthread_mutex_unlock(&client_mutex);
}
int register_user(char *username, char *password)
{
    FILE *fp;

    char stored_username[USERNAME_SIZE];
    char stored_password[PASSWORD_SIZE];
    char status[20];

    int socket_fd;

    fp = fopen(USER_FILE, "a+");

    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    rewind(fp);

    while (fscanf(fp," %29[^|] | %29[^|] | %19[^|] | %d",stored_username,stored_password,status,&socket_fd) == 4)
    {
        trim_spaces(stored_username);
        trim_spaces(stored_password);
        trim_spaces(status);

        if (strcmp(username, stored_username) == 0)
        {
            fclose(fp);
            return 0;
        }
    }

    fprintf(fp, "%s | %s | OFFLINE | -1\n",username,password);

    fclose(fp);

    return 1;
}
int login_user(char *username, char *password)
{
    FILE *fp;

    char stored_username[USERNAME_SIZE];
    char stored_password[PASSWORD_SIZE];
    char status[20];

    int socket_fd;

    fp = fopen(USER_FILE, "r");

    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    while (fscanf(fp," %29[^|] | %29[^|] | %19[^|] | %d",stored_username,stored_password,status,&socket_fd) == 4)
    {
        trim_spaces(stored_username);
        trim_spaces(stored_password);
        trim_spaces(status);

        if (strcmp(username, stored_username) == 0)
        {
            if (strcmp(password, stored_password) == 0)
            {
                fclose(fp);
                return 1;
            }
            else
            {
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);

    return -2;
}
void update_user_status(char *username, int socket_fd, char *status)
{
    FILE *fp;
    FILE *temp_fp;

    char stored_username[USERNAME_SIZE];
    char stored_password[PASSWORD_SIZE];
    char stored_status[20];

    int stored_socket_fd;

    fp = fopen(USER_FILE, "r");
    temp_fp = fopen("users_temp.txt", "w");

    if (fp == NULL || temp_fp == NULL)
    {
        perror("fopen");
        
        if (fp != NULL)
            fclose(fp);

        if (temp_fp != NULL)
            fclose(temp_fp);

        return;
    }

    while (fscanf(fp,
                  " %29[^|] | %29[^|] | %19[^|] | %d",
                  stored_username,
                  stored_password,
                  stored_status,
                  &stored_socket_fd) == 4)
    {
        trim_spaces(stored_username);
        trim_spaces(stored_password);
        trim_spaces(stored_status);

        if (strcmp(username, stored_username) == 0)
        {
            fprintf(temp_fp,"%s | %s | %s | %d\n",stored_username,stored_password,status,socket_fd);
        }
        else
        {
            fprintf(temp_fp,"%s | %s | %s | %d\n",stored_username,stored_password,stored_status,stored_socket_fd);
        }
    }

    fclose(fp);
    fclose(temp_fp);

    remove(USER_FILE);
    rename("users_temp.txt", USER_FILE);
}

void add_online_user(char *username,int client_fd)
{
    pthread_mutex_lock(&user_mutex);

    strcpy(online_users[online_count].username, username);
    online_users[online_count].socket_fd=client_fd;
    online_users[online_count].online=1;
    online_count++;
    pthread_mutex_unlock(&user_mutex);
}
int is_user_online(char *username)
{
    int found = 0;
    pthread_mutex_lock(&user_mutex);
    for(int i=0;i<online_count;i++)
    {
        if (online_users[i].online &&
            strcmp(online_users[i].username, username) == 0)
        {
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&user_mutex);
    return found;
}
void send_private_message(char *receiver,char *sender,char *message)
{
    int receiver_fd;
    char response[BUFFER_SIZE];
    receiver_fd=get_user_socket(receiver);
    if(receiver_fd == -1)
    {
        return;
    }
    snprintf(response,sizeof(response),"%s: %s",sender,message);
    send(receiver_fd,response,strlen(response),0);
}
void send_group_message(char *sender, char *message)
{
    char response[BUFFER_SIZE];
    snprintf(response,sizeof(response),"%s: %s",sender,message);
    pthread_mutex_lock(&user_mutex);
    for(int i=0;i<online_count;i++)
    {
        if(online_users[i].online)
        {
            send(online_users[i].socket_fd,response,strlen(response),0);
        }
    }
    pthread_mutex_unlock(&user_mutex);
}
int get_user_socket(char *username)
{
    int socket_fd=-1;
    pthread_mutex_lock(&user_mutex);
    for(int i=0;i<online_count;i++)
    {
        if(online_users[i].online &&strcmp(online_users[i].username, username) == 0)
        {
            socket_fd=online_users[i].socket_fd;
            break;
        }
    }
    pthread_mutex_unlock(&user_mutex);
    return socket_fd;
}
void notify_user_joined(char *username,int sender_fd)
{
    char message[BUFFER_SIZE];
    snprintf(message,sizeof(message),"%s joined the chat\n",username);
    pthread_mutex_lock(&user_mutex);
    for(int i=0;i<online_count;i++)
    {
        if(online_users[i].online &&
            online_users[i].socket_fd != sender_fd)
        {
            send(online_users[i].socket_fd,message,strlen(message),0);
        }
    }
    pthread_mutex_unlock(&user_mutex);
}

void send_online_users(int client_fd)
{
    char response[BUFFER_SIZE];
    int offset = 0;
    pthread_mutex_lock(&user_mutex);
    offset += snprintf(response + offset,sizeof(response) - offset,"Online users:\n");
    for (int i = 0; i < online_count; i++)
    {
        if (online_users[i].online)
        {
            offset += snprintf(response + offset,sizeof(response) - offset,"%s\n",online_users[i].username);
        }
    }
    pthread_mutex_unlock(&user_mutex);
    send(client_fd, response, strlen(response), 0);
}
void notify_user_left(char *username,int sender_fd)
{
    char message[BUFFER_SIZE];

    snprintf(message,sizeof(message),"%s left the chat\n",username);

    pthread_mutex_lock(&user_mutex);

    for(int i=0;i<online_count;i++)
    {
        if(online_users[i].online&&online_users[i].socket_fd!=sender_fd)
        {
            send(online_users[i].socket_fd,message,strlen(message),0);
        }
    }
    pthread_mutex_unlock(&user_mutex);
}
void logout_user(int client_fd)
{
    pthread_mutex_lock(&user_mutex);
    for(int i=0;i<online_count;i++)
    {
        if(online_users[i].socket_fd==client_fd)
        {
            online_users[i].online = 0;
            break;
        }
    }
    pthread_mutex_unlock(&user_mutex);
}
void remove_online_user(int client_fd)
{
    pthread_mutex_lock(&user_mutex);

    for (int i = 0; i < online_count; i++)
    {
        if (online_users[i].socket_fd == client_fd)
        {
            for (int j = i; j < online_count - 1; j++)
            {
                online_users[j] = online_users[j + 1];
            }

            online_count--;
            break;
        }
    }

    pthread_mutex_unlock(&user_mutex);
}
