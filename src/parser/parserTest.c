#include <stdio.h>
#include "parser.h"

int main(int argc, char** argv){
  sites *s;
  processXml(argv[1], &s);
  printf("sites is: %s, %d, %s, %d\n", s->workerManager, s->jobsPerNode, 
	 s->url, s->port);
}
