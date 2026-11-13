#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include"client.h"

char msg[BUFFER_SIZE];
int client_fd;

void*receive_messages(void*arg)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    while(1)
    {
        memset(buffer,0,sizeof(buffer));
        bytes_received=recv(client_fd,buffer,sizeof(buffer)-1,0);
        if(bytes_received<=0)
        {
            printf("\nServer dissconnected\n");
            exit(0);
        }
        buffer[bytes_received]='\0';
        printf("\n%s\n",buffer);
        fflush(stdout);
    }
    return NULL;
}

int main()
{
    //int bytes_rec;
    struct sockaddr_in server_addr;
    pthread_t receive_thread;
    client_fd=socket(AF_INET,SOCK_STREAM,0);
    if(client_fd==-1)
    {
        perror("socket");
        return 1;
    }
    printf("Client socket created\n");
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    server_addr.sin_port=htons(6333);
    if (connect(client_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))==-1)
    {
        perror("connect");
        close(client_fd);
        return 1;
    }
    printf("Connected to server\n");
    if(pthread_create(&receive_thread,NULL,receive_messages,NULL)!=0)
    {
        perror("pthread_create");
        close(client_fd);
        return 1;
    }
    while(1)
    {
        int choice;
        char username[30];
        char password[30];
        printf("\n1. Register\n");
        printf("2. Login\n");
        printf("3. Single Chat\n");
        printf("4. Group Chat\n");
        printf("5. Logout\n");
        printf("Enter choice: ");
        fflush(stdout);

        scanf("%d", &choice);
        getchar();
        if(choice==1)
        {
            register_client();
        }
        else if(choice==2)
        {
            login_client();
        }
        else if (choice == 3)
        {
            single_chat();
        }
        else if (choice == 4)
        {
            group_chat();
        }
        else if (choice==5)
        {
            logout_client();
        }
        else
        {
           printf("Invalid choice\n");
        }
    }
    close(client_fd);
    return 0;
}