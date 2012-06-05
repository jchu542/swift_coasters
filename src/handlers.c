// Handles individual stages of communication between swift and coasters
// By Reed Molbak and Jessica Chung

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "handlers.h"
#include "client.h"
#include "connection.h"

// Globals to be used temporarily
char *output;
char sendbuf[PACKET_MAX+1];
char recvbuf[PACKET_MAX+1];
header_packet hp;

int job_to_string(job_data *job, char **out);


// Send a configservice message, the first part of the submit job
// protocol.
int handle_configservice(coasters_info *ci, job *j){
  int jobs = j->jpn; // Get this from somewhere
  char *manager = "passive"; // Get this from somewhere
  char cs[] = "CONFIGSERVICE";
  send_with_retry(ci->sockfd, cs, strlen(cs), ci->tag, 0, MAX_RETRIES);

  char msg[PACKET_MAX + 1];
  sprintf(msg, "jobsPerNode=%d", j->jpn);
  send_with_retry(ci->sockfd, msg, strlen(msg), ci->tag, 0, MAX_RETRIES);

  // param workerManager
  sprintf(msg, "workerManager=%s", manager);
  send_with_retry(ci->sockfd, msg, strlen(msg), ci->tag, FIN_FLAG, MAX_RETRIES);
  ci->tag++;
}

int handle_submitjob(coasters_info *ci, job *j){
  int sent = 0;
  char sj[] = "SUBMITJOB";
  sent = send_with_retry(ci->sockfd, sj, strlen(sj), ci->tag, 0, MAX_RETRIES);

  char *msg; 
  job_to_string(j->data, &msg);

  char buf[PACKET_MAX];
  long len = PACKET_MAX;
  int res = ezcompress(buf, &len, msg, strlen(msg));
  sent = send_with_retry(ci->sockfd, buf, strlen(buf), ci->tag, FIN_FLAG, MAX_RETRIES);
  return sent;
}

int handle_jobinfo(coasters_info *ci, job *j){
  // Assuming all is well, record the log method
  receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); // outpuT
  output = "STDIN";
  if(!memcmp(recvbuf, output, 6))
    j->output = stdin;
  else{
    output = "STDOUT";
    if(!memcmp(recvbuf, output, 6))
      j->output = stdout;
    else{
      output = "STDERR";
      if(!memcmp(recvbuf, output, 6))
	j->output = stderr;
    }
  }

  // Print out next two things (log statements)
  receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); // 2 lines to print
  fprintf(j->output, "%s\n", recvbuf);
  send_with_retry(ci->sockfd, "OK", 2, hp.tag, FIN_FLAG | REPLY_FLAG, MAX_RETRIES);

  receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); // 2 lines to print
  fprintf(j->output, "%s\n", recvbuf);

  return 0;
}

int handle_jobstatus(coasters_info *ci, job *j){
  receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); // urn

  receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); // the actual job status (2 parts)
  j->job_status = atoi(recvbuf);

  receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES); 
  j->exit_code = atoi(recvbuf);

  return 0;
}

int handle_get(coasters_info *ci, job *j){
  int len;

  // Get the path of the requested file
  len = receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES);
  j->src = calloc(len + 1, 1);
  strncpy(j->src, recvbuf, len + 1);
  len = receive_with_retry(ci->sockfd, &hp, recvbuf, MAX_RETRIES);
  j->dest = calloc(len + 1, 1);
  strncpy(j->dest, recvbuf, len + 1);
  
  
  // Load src file
  // Since this is a reply, send with the sender's tag
  j->file_contents = calloc(1, sizeof(char*));
  len = load_file(j->src, j->file_contents);
  memcpy(sendbuf, &len, sizeof(int));
  memset(sendbuf + sizeof(int), 0, 8-sizeof(int));
  send_with_retry(ci->sockfd, sendbuf, 8, hp.tag, REPLY_FLAG, MAX_RETRIES);
  send_with_retry(ci->sockfd, *(j->file_contents), len, hp.tag, 
		  REPLY_FLAG | FIN_FLAG, MAX_RETRIES);
  
}

int job_to_string(job_data *jd, char **string){
  job_data ex = *jd;
  int len = 218;
  len += strlen(ex.identity) 
      + strlen(ex.executable) 
      + strlen(ex.directory) 
      + strlen(ex.stagein) 
      + strlen(ex.stageout[0]) 
      + strlen(ex.stageout[1]) 
      + strlen(ex.stageout[2]) 
      + strlen(ex.cleanup) 
      + strlen(ex.contact) 
      + strlen(ex.provider) 
      + strlen(ex.job_manager);
  int i;
  for(i=0; i<21; i++){
    if(ex.args[i])
      len+= strlen(ex.args[i]);
  }

  if(ex.batch) len+= strlen("true");
  else len+= strlen("false");
  char *out = calloc(sizeof(char), len);
  strcpy(out, "identity=");
  strcat(out, ex.identity);
  strcat(out, "\nexecutable=");
  strcat(out, ex.executable);
  strcat(out, "\ndirectory=");
  strcat(out, ex.directory);
  strcat(out, "\nbatch=");;
  if(ex.batch) strcat(out, "true");
  else strcat(out, "false");
  for(i=0; i<21; i++){
    strcat(out, "\narg=");
    if(ex.args[i]){
      strcat(out, ex.args[i]);
    }
  }
  strcat(out, "\nstagein=");
  strcat(out, ex.stagein);
  for(i=0; i<3; i++){
    strcat(out, "\nstageout=");
    if(ex.args[i]){
      strcat(out, ex.stageout[i]);
    }
  }
  strcat(out, "\ncleanup=");
  strcat(out, ex.cleanup);
  strcat(out, "\ncontact=");
  strcat(out, ex.contact);
  strcat(out, "\nprovider=");
  strcat(out, ex.provider);
  strcat(out, "\njm=");
  strcat(out, ex.job_manager);
  *string = out;
  
  return strlen(out);
}
