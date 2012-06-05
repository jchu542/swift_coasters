// client.h -- Primary API to connect to coasters service, and send jobs
// By Reed Molbak and Jessica Chung

#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>

#define MAX_RETRIES 2
#define REPLY_FLAG 1
#define FIN_FLAG 2

// Return codes
#define OK 0
#define CONNECTION_ERROR -1
#define RECV_ERROR -2
#define CHECKSUM_FAIL -3
#define COASTERS_ERROR -4
#define FILE_ERROR -5

// Exit codes (Improvised)
#define OK 0
#define SUBMIT_SEND_ERROR 1
#define SUBMIT_RESPONSE_ERROR 2
#define STATUS_RESPONSE_ERROR 3

// State of the client/coasters communication
typedef enum state {
  SUBMIT_JOB,
  JOB_INFO,
  JOB_STATUS,
  GET,
  COMPLETED,
  LOOP,
  ERROR
} state;

typedef struct job_data {
  char *identity;               // ID number
  char *executable;           //
  char *directory;            //
  int batch;                  // "true" or "false"
  char *args[21];             // string of cmd arguments given by Swift
  char *stagein;
  char *stageout[3];          
  char *cleanup;              // NULL
  char *contact;
  char *provider;             // NULL
  char *job_manager;          // NULL
} job_data;


// Information about the job to be given to coasters
// Should be populated by Swift/T
typedef struct job {
  int id;                 // Job id       
  int jpn;                // # of jobs to run per node
  char *manager;          // Passively or actively managed
  FILE *output;           // Output file (stdin, stdout or stderr, usually)
  int worker;         // Worker dealing with job
  int job_status;
  int exit_code;
  job_data *data;
  char *src;          // Source of job file
  char *dest;         // Dest of job file
  char **file_contents;
} job;

// Information about the coaster service to be connected to
typedef struct coasters_info {
  char *hostname;
  int port;
  int sockfd;
  int tag;    // Tag of current communication with coasters
  state mode;    // State of current communication with coasters
  char *client_chan_id;
  char *coaster_chan_id;
} coasters_info;

int coasters_connect(char *hostname, int port, coasters_info **coasters);
int submit_job(coasters_info *ci, int id, int jpn, char* manager, job_data *jd, job **j);

#endif
