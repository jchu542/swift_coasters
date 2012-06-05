// Handles connecting to and sending messages to and from coaster service
// By Reed Molbak and Jessica Chung
#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include <stdlib.h>

// Max size of a packet that can be sent
#define PACKET_MAX 1000

// Contents of a packet to be sent out
typedef struct header_packet {
  uint32_t tag;
  uint32_t flags;
  uint32_t len;
  uint32_t hsum;
  uint32_t csum;
  //char *msg;
} header_packet;

int get_connection(char *name, int port);
int pack_and_send(int sockfd, char *msg, int size, int tag, uint32_t flags);
int send_with_retry(int sockfd, char *msg, int size, int tag, uint32_t flags, int retries);
int receive_data(int sockfd, header_packet *hp, char *buf);
int receive_with_retry(int sockfd, header_packet *hp, char *buf, int retries);
#endif
