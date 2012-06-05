// client.c -- Primary API to connect to coasters service, and send jobs
// By Reed Molbak and Jessica Chung
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include <uuid/uuid.h>

#include "easyzlib.h" // Taken from www.firstobject.com/easy-zlib-c++-xml-compression.htm
#include "client.h"
#include "connection.h"
#include "handlers.h"

int chan_config(coasters_info *ci);
int load_file(char *getfile, char **contents);
void free_job(job *j);

// Opens a connection to coaster service. Must be done first.
// Output parameter coasters contains info about this connection.
int coasters_connect(char *hostname, int port, coasters_info **coasters){
  *coasters = malloc(sizeof(coasters_info));
  coasters_info *ci = *coasters;
  ci->hostname = hostname;
  ci->port = port;
  ci->sockfd = get_connection(hostname, port);

  if(ci->sockfd < 0){
    fprintf(stderr, "connection error: could not connect to coaster service\n");
    return CONNECTION_ERROR;
  }
  if(chan_config(ci) > 0) return 0;
  else return CONNECTION_ERROR;
}

// Configuring a channel
// Verify the things we need to send
int chan_config(coasters_info *ci)
{
  //srand(time(NULL));
  //uint32_t tag = rand() % 10000;
  int tag = 1;
  ci->tag = 1;

  // first send a CHANNELCONFIG
  char *msg = "CHANNELCONFIG";
  int sent = send_with_retry(ci->sockfd, msg, strlen(msg), tag, 0, MAX_RETRIES);

  // Should we be setting keepalive?
  msg = " keepalive(120), reconnect";
  sent = send_with_retry(ci->sockfd, msg, strlen(msg), tag, 0, MAX_RETRIES); 

  send_with_retry(ci->sockfd, NULL, 0, tag, 0, MAX_RETRIES);
  /*
    char newmsg[36];
    uuid_t chan_id;
    uuid_generate_random(chan_id);
    uuid_unparse(chan_id, msg);
    char uuid[27] = "u";
    strncpy(&uuid[1], msg, 20);
    strncat(uuid, "--8000", 6);*/
  char *newmsg = "ub8a652a-13743a87f30--8000\0";
  ci->client_chan_id = newmsg;
  sent = send_with_retry(ci->sockfd, newmsg, strlen(newmsg), tag, 0, MAX_RETRIES);
  
  //send FIN packet
  sent = send_with_retry(ci->sockfd, NULL, 0, tag, FIN_FLAG, MAX_RETRIES);
	
  	
  char recvbuf[PACKET_MAX + 1]; // +1 for null terminator

  // Catch reply with channel id of coasters
  header_packet hp;
  int len = receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES);
  ci->tag = hp.tag + 1;
  ci->coaster_chan_id = calloc(1, len + 1);  // For null terminator
  memcpy(ci->coaster_chan_id, recvbuf, len + 1);

  return sent;
}

// Accepts a job, and loads the job parameters into a job struct.
// Job will be sent to the coaster service in ci.
// Proceeds to execute the job, and returns when done. Swift/T should
// probably run this in a separate thread. Given the implementation of standard
// swift, forking the thread seems to be swift's job
int submit_job(coasters_info *ci, int id, int jobsPerNode, char *manager, job_data *jd, 
	       job **_job){
  *_job = calloc(1, sizeof(job));
  job *j = *_job;
  j->id = id;
  j->jpn = jobsPerNode;
  j->manager = calloc(1, 10);
  j->output = stderr;
  j->data = jd;
  strncpy(j->manager, manager, 10);

  //uint32_t tag = rand() % 10000;

  char recvbuf[PACKET_MAX + 1]; // Leave room to add a null-terminator

  handle_configservice(ci, j);
  ci->mode = SUBMIT_JOB;

  // Wait for recv's. Depending on the mode, do something
  header_packet hp;
  int len;
  char sendbuf[PACKET_MAX + 1];
  char *reply;
  char *output;
  do{
    len = receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES);
    
    if(!strncmp("JOBSTATUS", recvbuf, PACKET_MAX))
      ci->mode = JOB_STATUS;
    if(!strncmp("GET", recvbuf, PACKET_MAX))
      ci->mode = GET;
    

    switch(ci->mode){
    case SUBMIT_JOB:
      if (strcmp(recvbuf, "OK") == 0){
	int submit_success = handle_submitjob(ci, j);
	if(submit_success < 0){
	  j->exit_code = SUBMIT_SEND_ERROR;
	  ci->mode = ERROR;
	  continue;
	}
      }
      else{
	// Unexpected response
	break;
      }
      
      ci->mode = JOB_INFO;
      break;
      
    case JOB_INFO:
      // First response should be rlog
      reply = "RLOG";
      if(strncmp(reply, recvbuf, PACKET_MAX)){
	// Unexpected response
	break;
      }
      handle_jobinfo(ci, j);
       
      ci->tag++;
      ci->mode = LOOP;
      break;

    case JOB_STATUS:
      if(strcmp(recvbuf, "JOBSTATUS")){
	// Unexpected response
	break;
      }
       
      handle_jobstatus(ci, j);
       
      // If the exit code is not zero, something is wrong
      // Todo: check this logic
      if(j->exit_code){
	ci->mode = ERROR;
      }
      else
	ci->mode = LOOP;
      break;

    case LOOP:
      // Here, accept anything. Look for specific messages to get out
      reply = "workerid=";
      if(!memcmp(reply, recvbuf, 9)){
	j->worker = atoi(reply + 9);
	receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); // Get some thing whose
                                                                   // purpose is unknown
	send_with_retry(ci->sockfd, "OK", 2, hp.tag, REPLY_FLAG | FIN_FLAG, MAX_RETRIES);
      }
      
      ci->mode = LOOP;
      break;

    case GET:
      if(strcmp(recvbuf, "GET")){
	// Unexpected response
	break;
      }
      handle_get(ci, j); 
      ci->mode = LOOP;
      break;

    case ERROR:
      fprintf(j->output, "Error: %d\n", j->exit_code);
      fprintf(j->output, "%s\n", recvbuf);
      free_job(j);
      return COASTERS_ERROR;

    case COMPLETED:
      fprintf(j->output, "Job completed\n");
      free_job(j);
      return OK;
    }
  } while (len > 0);
}

void free_job(job *j){
  if(j->manager) free(j->manager);
  // output not freed because it is stdout or some other thing we didn't allocate
  // data is the responsibility of swift to free (swift creates the data struct)
  if(j->src) free(j->src);
  if(j->dest) free(j->dest);
  if(j->file_contents){
    if(*(j->file_contents)) free(*(j->file_contents));
    free(j->file_contents);
  }
  free(j);
}
int load_file(char *getfile, char** contents){
  size_t len;
  
  // make a copy of the original file name because strtok kills things
  char original[strlen(getfile) + 1];
  strcpy(original, getfile);
  char *filename;
  filename = strtok(getfile, ":/");
  if(strcmp(filename, "file") == 0){
    filename = strtok(NULL, ":/");
    if(strcmp(filename, "localhost") == 0)
      filename = strtok(NULL, " ");
    else filename = NULL; // should go make a connection with something that is not localhost
                          // For the purposes of this project, this should not happen
  }
  else filename = original;
  
  if(filename) {
    fprintf(stderr, "opening file %s\n", filename);
    FILE *fp = fopen(filename, "r");
    if (fp != NULL) {
      printf("seeking\n");
      if (fseek(fp, 0L, SEEK_END) == 0) {
	long bufsize = ftell(fp);
	if (bufsize == -1) {  }
	printf("about to malloc %d\n", bufsize);
	printf("contents is %d\n", contents);
	printf("and pointer is %d\n", *contents);
	*contents = calloc(sizeof(char), (bufsize + 1));
	printf("malloc'd\n");
	if (fseek(fp, 0L, SEEK_SET) == 0) {  }
	
	len = fread(*contents, sizeof(char), bufsize, fp);
	if (len == 0) {
	  fputs("Error reading file", stderr);
	} else {
	  printf("placing null\n");
	  (*contents)[len] = '\0'; 
	}
      }
      fclose(fp);
    }
    fprintf(stderr,"%s\n", *contents);
    return len;
  }
  else return FILE_ERROR;
}
