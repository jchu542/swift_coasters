// Handles individual stages of communication between swift and coasters
// By Reed Molbak and Jessica Chung
#ifndef HANDLERS_H
#define HANDLERS_H

#include "client.h"

int handle_configservice(coasters_info *ci, job *j);
int handle_submitjob(coasters_info *ci, job *j);
int handle_jobinfo(coasters_info *ci, job *j);
int handle_jobstatus(coasters_info *ci, job *j);
int handle_get(coasters_info *ci, job *j);
#endif
