#include "tftp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

void handle_client(int sockfd,
                   struct sockaddr_in client_addr,
                   socklen_t client_len,
                   tftp_packet *packet);

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    tftp_packet packet;

    /* Create UDP Socket */

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Set Timeout */

    struct timeval tv;

    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;

    if(setsockopt(sockfd,
                  SOL_SOCKET,
                  SO_RCVTIMEO,
                  &tv,
                  sizeof(tv)) < 0)
    {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* Server Address */

    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* Bind */

    if(bind(sockfd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("\n=====================================\n");
    printf("       TFTP SERVER STARTED\n");
    printf("Listening on Port %d\n",PORT);
    printf("=====================================\n");

    client_len = sizeof(client_addr);

    while(1)
    {
        memset(&packet,0,sizeof(packet));

        int n = recvfrom(sockfd,
                         &packet,
                         sizeof(packet),
                         0,
                         (struct sockaddr *)&client_addr,
                         &client_len);

        if(n < 0)
        {
            perror("recvfrom");
            continue;
        }

        handle_client(sockfd,
                      client_addr,
                      client_len,
                      &packet);
    }

    close(sockfd);

    return 0;
}

/*----------------------------------------------------*/
/* Handle Client Request                              */
/*----------------------------------------------------*/

void handle_client(int sockfd,
                   struct sockaddr_in client_addr,
                   socklen_t client_len,
                   tftp_packet *packet)
{
    uint16_t opcode;

    opcode = ntohs(packet->opcode);

    switch(opcode)
    {
        case RRQ:
        {
            printf("\nRead Request Received\n");
            printf("Filename : %s\n",
                    packet->body.request.filename);

            FILE *fp;

            fp = fopen(packet->body.request.filename,"rb");

            if(fp == NULL)
            {
                tftp_packet err;

                memset(&err,0,sizeof(err));

                err.opcode = htons(ERROR);
                err.body.error_packet.error_code = htons(1);

                strcpy(err.body.error_packet.error_msg,
                       "File Not Found");

                sendto(sockfd,
                       &err,
                       sizeof(err),
                       0,
                       (struct sockaddr *)&client_addr,
                       client_len);

                printf("File Not Found\n");

                return;
            }

            fclose(fp);

            tftp_packet ack;

            memset(&ack,0,sizeof(ack));

            ack.opcode = htons(ACK);
            ack.body.ack_packet.block_number = htons(0);

            sendto(sockfd,
                   &ack,
                   sizeof(ack),
                   0,
                   (struct sockaddr *)&client_addr,
                   client_len);

            send_file(sockfd,
                      client_addr,
                      client_len,
                      packet->body.request.filename);

            break;
        }

        case WRQ:
        {
            printf("\nWrite Request Received\n");
            printf("Filename : %s\n",
                    packet->body.request.filename);

            tftp_packet ack;

            memset(&ack,0,sizeof(ack));

            ack.opcode = htons(ACK);
            ack.body.ack_packet.block_number = htons(0);

            sendto(sockfd,
                   &ack,
                   sizeof(ack),
                   0,
                   (struct sockaddr *)&client_addr,
                   client_len);

            receive_file(sockfd,
                         client_addr,
                         client_len,
                         packet->body.request.filename);

            break;
        }

        default:
        {
            printf("Unknown Request\n");

            tftp_packet err;

            memset(&err,0,sizeof(err));

            err.opcode = htons(ERROR);
            err.body.error_packet.error_code = htons(4);

            strcpy(err.body.error_packet.error_msg,
                   "Illegal TFTP Operation");

            sendto(sockfd,
                   &err,
                   sizeof(err),
                   0,
                   (struct sockaddr *)&client_addr,
                   client_len);

            break;
        }
    }
}