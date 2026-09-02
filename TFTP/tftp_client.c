#include "tftp.h"
#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    char command[256];

    tftp_client_t client;

    memset(&client,0,sizeof(client));

    while(1)
    {
        printf("tftp> ");

        fgets(command,sizeof(command),stdin);

        command[strcspn(command,"\n")] = '\0';

        process_command(&client,command);
    }

    return 0;
}

// Function to process commands
void process_command(tftp_client_t *client, char *command)
{
    char cmd[20];
    char arg[100];

    int count = sscanf(command,"%s %s",cmd,arg);

    if(count <= 0)
        return;

    if(strcmp(cmd,"connect")==0)
    {
        connect_to_server(client,arg,PORT);
    }
    else if(strcmp(cmd,"put")==0)
    {
        put_file(client,arg);
    }
    else if(strcmp(cmd,"get")==0)
    {
        get_file(client,arg);
    }
    else if(strcmp(cmd,"exit")==0)
    {
        disconnect(client);
        exit(0);
    }
    else
    {
        printf("Unknown Command\n");
    }
}

// This function is to initialize socket with given server IP, no packets sent to server in this function
void connect_to_server(tftp_client_t *client,char *ip,int port)
{
    client->sockfd = socket(AF_INET,SOCK_DGRAM,0);

    if(client->sockfd < 0)
    {
        perror("socket");
        return;
    }

    struct timeval tv;

    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;

    setsockopt(client->sockfd,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &tv,
               sizeof(tv));

    memset(&client->server_addr,0,sizeof(client->server_addr));

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);

    inet_pton(AF_INET,
              ip,
              &client->server_addr.sin_addr);

    client->server_len = sizeof(client->server_addr);

    strcpy(client->server_ip,ip);

    printf("Connected to %s:%d\n",ip,port);
}

void put_file(tftp_client_t *client, char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if(fp == NULL)
    {
        printf("File not found.\n");
        return;
    }

    fclose(fp);

    send_request(client->sockfd,
                 client->server_addr,
                 filename,
                 WRQ);

    receive_request(client->sockfd,
                    client->server_addr,
                    filename,
                    WRQ);

    send_file(client->sockfd,
              client->server_addr,
              client->server_len,
              filename);
}

void get_file(tftp_client_t *client, char *filename)
{
    send_request(client->sockfd,
                 client->server_addr,
                 filename,
                 RRQ);

    receive_request(client->sockfd,
                    client->server_addr,
                    filename,
                    RRQ);

    receive_file(client->sockfd,
                 client->server_addr,
                 client->server_len,
                 filename);
}

void disconnect(tftp_client_t *client)
{
    if(client->sockfd > 0)
    {
        close(client->sockfd);
        client->sockfd = -1;
    }

    printf("Disconnected from server.\n");
}
void send_request(int sockfd,
                  struct sockaddr_in server_addr,
                  char *filename,
                  int opcode)
{
    tftp_packet packet;

    memset(&packet, 0, sizeof(packet));

    packet.opcode = htons(opcode);

    strcpy(packet.body.request.filename, filename);
    strcpy(packet.body.request.mode, "octet");

    sendto(sockfd,
           &packet,
           sizeof(packet),
           0,
           (struct sockaddr *)&server_addr,
           sizeof(server_addr));
}

void receive_request(int sockfd,
                     struct sockaddr_in server_addr,
                     char *filename,
                     int opcode)
{
    tftp_packet packet;

    socklen_t len = sizeof(server_addr);

    if(recvfrom(sockfd,
                &packet,
                sizeof(packet),
                0,
                (struct sockaddr *)&server_addr,
                &len) < 0)
    {
        perror("recvfrom");
        return;
    }

    if(ntohs(packet.opcode) == ACK)
    {
        printf("Server acknowledged request.\n");
    }
    else if(ntohs(packet.opcode) == ERROR)
    {
        printf("Server Error : %s\n",
               packet.body.error_packet.error_msg);
    }
}