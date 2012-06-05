// Handles connecting to and communicating with coasters

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "connection.h"
#include "client.h"

int pack_header(int len, int tag, uint32_t flags, header_packet *p);
int verify(header_packet *recvd);

// Loads host info from a hostname
// Returns a socket
int get_connection(char *name, int port){

  /* get the host info */ 
  struct addrinfo *destinfo;
  struct addrinfo hints;

  char str_port[6];
  sprintf(str_port, "%d", port);
  int gotinfo = getaddrinfo(name, str_port, NULL, &destinfo);
  if(gotinfo != 0){
    return CONNECTION_ERROR;
  }
  int sockfd;


  struct addrinfo *iter;
  // Iterate through all addrs until we get a good one
  for(iter = destinfo; iter != NULL; iter = iter->ai_next) {
    if((sockfd = socket(iter->ai_family, iter->ai_socktype,
			iter->ai_protocol)) == -1)
      continue;

    if(connect(sockfd, iter->ai_addr, iter->ai_addrlen) == -1){
      close(sockfd);
      continue;
    }
    // If we get this far, we have a good addrinfo
    break;
  }


  if(iter == NULL){
    return CONNECTION_ERROR;
  }

  freeaddrinfo(destinfo);

  return sockfd;





}



// The message should be packed into little endian format
// Based on the strace, each command is preceded by 20 bytes
// of this header
// Followed by the actual message
int pack_header(int len, int tag, uint32_t flags, header_packet *p)
{
  p->tag = tag;
  p->flags = flags;
  p->len = len;
  p->hsum = p->tag ^ p->flags ^ p->len;
  p->csum = 0; // Not doing body checksums yet (not required)
  return 0;  
}

int pack_and_send(int sockfd, char *msg, int size, int tag, uint32_t flags)
{
  int sent=0;
  header_packet p;
  pack_header(size, tag, flags, &p);
  sent += send(sockfd, &p, sizeof(header_packet), 0);
  if(size > 0)
    sent += send(sockfd, msg, size, 0);
  return sent;
}

// Sends a string through the socket, retrying if it fails
int send_with_retry(int sockfd, char *msg, int size, int tag, uint32_t flags, int retries){
  int sent = 0;
  int i;
  for(i = 0; i < retries; i++){
    sent = pack_and_send(sockfd, msg, size, tag, flags);
    if(sent >= size + sizeof(header_packet)) break;
  }
  return sent;
}


int verify(header_packet *recvd){
  uint32_t calc = recvd->tag ^ recvd->flags ^ recvd->len;
  if (calc == recvd->hsum) return 0;
  else {
    fprintf(stderr, "hsum error!\n");
    return CHECKSUM_FAIL;
  }
}

int receive_data(int sd, header_packet *hp, char *buf){
  int len = recv(sd, hp, sizeof(header_packet), 0);
  if(len < sizeof(header_packet)) return RECV_ERROR;

  int retval = verify(hp);
  if (retval < 0) return retval;

  len = recv(sd, buf, hp->len, 0);
  buf[len] = '\0'; // So all strings are null-terminated
  return len;
}

int receive_with_retry(int sd, header_packet *hp, char *buf, int retries){
  int recvd;
  int i;
  for(i = 0; i < retries; i++){
    recvd = receive_data(sd, hp, buf);
    if(recvd >= 0) break;
  }
  return recvd;
}
