#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include"server.h"

void*handle_client(void*arg)
{
    int client_fd=*(int*)arg;
    free(arg);
    
    // char response[BUFFER_SIZE];
    int bytes_received;
    char buffer[BUFFER_SIZE];
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char response[BUFFER_SIZE];
    int logged_in=0;
    printf("Thread started\n");
    
    while(1)
    {
       
        memset(buffer,0,sizeof(buffer));
        bytes_received=recv(client_fd,buffer,sizeof(buffer)-1,0);
        if(bytes_received==0)
        {
            printf("Client disconnected\n");
            break;
        }
        if(bytes_received==-1)
        {
            perror("recv");
            break;
        }
        buffer[bytes_received]='\0';
        printf("Client: %s",buffer);
        if(strncmp(buffer,CMD_REGISTER,strlen(CMD_REGISTER))==0)
        {
            if(sscanf(buffer+9,"%29s %29s",username,password)==2)
            {
                int res=register_user(username,password);
                if(res==1)
                {
                    
                    strcpy(response,"Registered successfully\n");
                    
                }
                else if(res==0)
                {
                    strcpy(response,"Duplicate user-name\n");
                }
                else
                {
                    strcpy(response,"Registration failed\n");
                }
                send(client_fd,response,strlen(response),0);
            }
            else
            {
                strcpy(response,"Invalid registration format\n");
                send(client_fd,response,strlen(response),0);
            }
        }
        else if(strncmp(buffer,CMD_LOGIN, strlen(CMD_LOGIN))==0)
        {
            if(sscanf(buffer + 6,"%29s %29s",username,password) == 2)
            {
                int res=login_user(username, password);
                if(res==1)
                {
                    if(is_user_online(username))
                    {
                        strcpy(response, "User already logged in\n");
                        send(client_fd,response,strlen(response),0);
                        continue;
                    }
                    add_online_user(username, client_fd);
                    logged_in = 1;
                    update_user_status(username, client_fd, "ONLINE");
                    strcpy(response, "Logged in successfully\n");
                    send(client_fd,response,strlen(response),0);
                    send_online_users(client_fd);
                    notify_user_joined(username, client_fd);
                    continue;
                }
                else if(res==0)
                {
                    strcpy(response, "Password not matching\n");
                }
                else if(res==-2)
                {
                    strcpy(response, "User-name not found\n");
                }
                else
                {
                    strcpy(response, "Login failed\n");
                }
                send(client_fd, response, strlen(response), 0);
            }
            else
            {
                strcpy(response, "Invalid login format\n");
                send(client_fd, response, strlen(response), 0);
            }
        }
        else if(strncmp(buffer,CMD_SINGLE,strlen(CMD_SINGLE))==0)
        {
            char receiver[USERNAME_SIZE];
            char message[BUFFER_SIZE];
            if(sscanf(buffer + 7,"%29s %[^\n]",receiver,message) == 2)
            {
                if(logged_in)
                {
                    send_private_message(receiver,username,message);
                }
            }
        }
        else if (strncmp(buffer,CMD_GROUP,strlen(CMD_GROUP))==0)
        {
            char message[BUFFER_SIZE];
            if(sscanf(buffer + 6,"%[^\n]",message)==1)
            {
                if (logged_in)
                {
                    send_group_message(username, message);
                }
            }
        }
        else if(strncmp(buffer, CMD_LOGOUT,strlen(CMD_LOGOUT))==0)
        {
            if (logged_in)
            {
                notify_user_left(username, client_fd);
                remove_online_user(client_fd);
                update_user_status(username, -1, "OFFLINE");
                strcpy(response, "Logged out successfully\n");
                send(client_fd,response,strlen(response),0);
                logged_in = 0;
            }
            else
            {
                strcpy(response, "Couldn't logged out\n");
                send(client_fd,response,strlen(response),0);
            }
        }

        else
        {
            broadcast_message(buffer,client_fd);

        }
        
    }
    if(logged_in)
    {
        notify_user_left(username,client_fd);
        remove_online_user(client_fd);
    }
    remove_client(client_fd);
    close(client_fd);
    return NULL;
}
int main()
{
    int server_fd;
    int client_fd;
    int bytes_rec;
    int opt=1;

    char buffer[100];
    char res[100];

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);
    pthread_t thread_id;
    server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==-1)
    {
        perror("socket");
        return 1;
    }
    printf("Socket created\n");
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(PORT);
    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }
    printf("Bind successfull\n");
    if(listen(server_fd,5)==-1)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }
    printf("Server listening on port %d\n",PORT);
    
    while(1)
    {
        client_fd=accept(server_fd,NULL,NULL);
        if(client_fd==-1)
        {
            perror("accept");
            close(server_fd);
            return 1;
        }
        printf("Client connected\n");
        pthread_mutex_lock(&client_mutex);
        if(client_count>=MAX_CLIENTS)
        {
            pthread_mutex_unlock(&client_mutex);
            printf("Maximum clients reached\n");
            close(client_fd);
            continue;
        }
        client_fds[client_count]=client_fd;
        client_count++;
        pthread_mutex_unlock(&client_mutex);
        int*new_client=malloc(sizeof(int));
        if(new_client==NULL)
        {
            perror("malloc");
            close(client_fd);
            continue;
        }
        *new_client=client_fd;
        if(pthread_create(&thread_id,NULL,handle_client,new_client)!=0)
        {
            perror("pthread_create");
            free(new_client);
            close(client_fd);
            continue;
        }
        
        pthread_detach(thread_id);
        
    }
    close(server_fd);
    return 0;

}
   
    
