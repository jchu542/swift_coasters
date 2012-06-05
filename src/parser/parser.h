#ifndef PARSER_H
#define PARSER_H

// Xml labels and attributes
#define EXECUTION_TAG "execution"
#define PROFILE_TAG "profile"
#define MANAGER_ATTR "workerManager"
#define JOBS_ATTR "jobsPerNode"
#define URL_LABEL "url"
#define KEY_LABEL "key"

typedef struct sites{
  char *workerManager;
  int jobsPerNode;
  char *url;
  int port;
} sites;

int processXml(char *filename, sites **_sites);
int freeSites(sites *s);
#endif
