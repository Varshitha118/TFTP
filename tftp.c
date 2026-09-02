/* Common file for server & client */
#include "tftp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*----------------------------------------------------------
 * Send file to receiver (GET operation)
 *---------------------------------------------------------*/
void send_file(int sockfd,
               struct sockaddr_in client_addr,
               socklen_t client_len,
               char *filename)
{
    FILE *fp;
    tftp_packet packet;
    tftp_packet ack;

    uint16_t block = 1;
    int bytes_read;

    fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        perror("File open");
        return;
    }

    printf("Sending file : %s\n", filename);

    while (1)
    {
        memset(&packet, 0, sizeof(packet));

        /* Fill DATA packet */

        packet.opcode = htons(DATA);
        packet.body.data_packet.block_number = htons(block);

        bytes_read = fread(packet.body.data_packet.data,
                           1,
                           512,
                           fp);

        /* Send DATA packet */

        if (sendto(sockfd,
                   &packet,
                   4 + bytes_read,
                   0,
                   (struct sockaddr *)&client_addr,
                   client_len) < 0)
        {
            perror("sendto");
            fclose(fp);
            return;
        }

        printf("DATA Block %d Sent (%d Bytes)\n",
               block,
               bytes_read);

        /* Wait for ACK */

        if (recvfrom(sockfd,
                     &ack,
                     sizeof(ack),
                     0,
                     (struct sockaddr *)&client_addr,
                     &client_len) < 0)
        {
            perror("ACK Receive");
            fclose(fp);
            return;
        }

        if (ntohs(ack.opcode) != ACK)
        {
            printf("Invalid ACK received\n");
            fclose(fp);
            return;
        }

        if (ntohs(ack.body.ack_packet.block_number) != block)
        {
            printf("Wrong ACK Block\n");
            fclose(fp);
            return;
        }

        printf("ACK %d Received\n", block);

        /* Last packet */

        if (bytes_read < 512)
            break;

        block++;
    }

    fclose(fp);

    printf("File Transfer Completed.\n");
}

/*----------------------------------------------------------
 * Receive file from sender (PUT operation)
 *---------------------------------------------------------*/
void receive_file(int sockfd,
                  struct sockaddr_in client_addr,
                  socklen_t client_len,
                  char *filename)
{
    FILE *fp;

    tftp_packet packet;
    tftp_packet ack;

    int n;
    uint16_t expected_block = 1;

    fp = fopen(filename, "wb");

    if (fp == NULL)
    {
        perror("File Create");
        return;
    }

    printf("Receiving file : %s\n", filename);

    while (1)
    {
        memset(&packet, 0, sizeof(packet));

        n = recvfrom(sockfd,
                     &packet,
                     sizeof(packet),
                     0,
                     (struct sockaddr *)&client_addr,
                     &client_len);

        if (n < 0)
        {
            perror("recvfrom");
            fclose(fp);
            return;
        }

        if (ntohs(packet.opcode) != DATA)
        {
            printf("Invalid DATA Packet\n");
            fclose(fp);
            return;
        }

        if (ntohs(packet.body.data_packet.block_number) != expected_block)
        {
            printf("Unexpected Block Number\n");
            fclose(fp);
            return;
        }

        fwrite(packet.body.data_packet.data,
               1,
               n - 4,
               fp);

        printf("Received Block %d (%d Bytes)\n",
               expected_block,
               n - 4);

        /* Send ACK */

        memset(&ack, 0, sizeof(ack));

        ack.opcode = htons(ACK);
        ack.body.ack_packet.block_number =
            htons(expected_block);

        if (sendto(sockfd,
                   &ack,
                   sizeof(ack),
                   0,
                   (struct sockaddr *)&client_addr,
                   client_len) < 0)
        {
            perror("sendto");
            fclose(fp);
            return;
        }

        printf("ACK %d Sent\n", expected_block);

        /* Last packet */

        if ((n - 4) < 512)
            break;

        expected_block++;
    }

    fclose(fp);

    printf("File Received Successfully.\n");
}