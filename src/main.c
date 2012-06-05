// Creates dummy input resembling what Swift/T would give if it were running
// Shows how the API might be called

#include <stdio.h>
#include <string.h>
#include "client.h"
#include "connection.h"

int main(int argc, char **argv){
  char *hostname = argv[1];
  char *port_a = argv[2];
  int port = atoi(port_a);
  coasters_info *ci;
  job *j;
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
  
  printf("connecting\n");
  coasters_connect(hostname, port, &ci);
  printf("coasters connected\n");
  submit_job(ci, 0001, 2, "passive", &ex, &j);
  printf("submit finished\n");
}
