#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

int job_to_string(job_data *job, char **out);

int main(){
  job_data ex;
  ex.identity="1338695764465";
  ex.executable = "/bin/bash";
  ex.directory ="/stage/swift-coasters/swift_coasters/swift_dir/swiftwork/hello-20120602-2256-qs5e4kxg-i-echo-ics4c8sk";
  ex.batch = 0;
  ex.args[0] = "_swiftwrap.staging";
  ex.args[1] = "-e";
  ex.args[2] = "/bin/echo";
  ex.args[3] = "-out";
  ex.args[4] = "hello.txt";
  ex.args[5] = "-err";
  ex.args[6] = "stderr.txt";
  ex.args[7] = "-i";
  ex.args[8] = "-d";
  ex.args[9] = NULL;
  ex.args[10] = "-if";
  ex.args[11] = NULL;
  ex.args[12] = "-of";
  ex.args[13] = "hello.txt";
  ex.args[14] = "-k";
  ex.args[15] = "-cdmfile";
  ex.args[16] = NULL;
  ex.args[17] = "-status";
  ex.args[18] = "provider";
  ex.args[19] = "-a";
  ex.args[20] = "Hello, world!";
  ex.stagein = "proxy://localhost//stage/swift-coasters/swift-0.93/cog/modules/swift/dist/swift-svn/bin/../libexec/_swiftwrap.staging\\n_swiftwrap.staging\\n10";
  ex.stageout[0] = "wrapper.log\\nproxy://localhost/hello-20120602-2256-qs5e4kxg.d/echo-ics4c8sk.info\\n2";
  ex.stageout[1] = "./hello.txt\\nproxy://localhost/hello.txt\\n10";
  ex.stageout[2] = "wrapper.error\\nproxy://localhost/hello-20120602-2256-qs5e4kxg.d/echo-ics4c8sk.error\\n2";
  ex.cleanup = ".";
  ex.contact = "http://ursa.cs.uchicago.edu:12000";
  ex.provider = "coaster-persistent";
  ex.job_manager = "";
  
  char *msg= "identity=1338695764465\nexecutable=/bin/bash\ndirectory=/stage/swift-coasters/swift_coasters/swift_dir/swiftwork/hello-20120602-2256-qs5e4kxg-i-echo-ics4c8sk\nbatch=false\narg=_swiftwrap.staging\narg=-e\narg=/bin/echo\narg=-out\narg=hello.txt\narg=-err\narg=stderr.txt\narg=-i\narg=-d\narg=\narg=-if\narg=\narg=-of\narg=hello.txt\narg=-k\narg=-cdmfile\narg=\narg=-status\narg=provider\narg=-a\narg=Hello, world!\nstagein=proxy://localhost//stage/swift-coasters/swift-0.93/cog/modules/swift/dist/swift-svn/bin/../libexec/_swiftwrap.staging\\n_swiftwrap.staging\\n10\nstageout=wrapper.log\\nproxy://localhost/hello-20120602-2256-qs5e4kxg.d/echo-ics4c8sk.info\\n2\nstageout=./hello.txt\\nproxy://localhost/hello.txt\\n10\nstageout=wrapper.error\\nproxy://localhost/hello-20120602-2256-qs5e4kxg.d/echo-ics4c8sk.error\\n2\ncleanup=.\ncontact=http://ursa.cs.uchicago.edu:12000\nprovider=coaster-persistent\njm=";
  
  
  char *measure="identity=\nexecutable=\ndirectory=\nbatch=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\narg=\n arg=\n arg=\narg=\nstagein=\nstageout=\nstageout=\n stageout=\ncleanup=\ncontact=\nprovider=\njm=";
  printf("measure %d\n", strlen(measure));
  
  char *out;
  job_to_string(&ex, &out);
  
  printf("%d == %d\n", strlen(out), strlen(msg));
  int problem = strncmp(out, msg, strlen(out));
  if( problem == 0) printf("so far, so good\n");
  else printf("fucked @ %d\n", problem);
  
}

int job_to_string(job_data *j, char **string){
  job_data ex = *j;
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
    printf("%d ", i);
    if(ex.args[i])
      len+= strlen(ex.args[i]);
  }
  printf("\n");
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
  //printf("[%d] %s\n", strlen(out), out);
  *string = out;
}