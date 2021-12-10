#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

//FUNCTION THAT IS CALLED TO FORK AND EXEC A PROCESS
int spawn(const char * program, char ** arg_list) {
  pid_t child_pid = fork();
  if (child_pid != 0)
    return child_pid;
  else {
    execvp (program, arg_list);
    perror("exec failed");
    return 1;
  }
}

int main(int argc, char *argv[]) {


  char * arg_list_1[] = { "/usr/bin/konsole",  "-e", "./producer", argv[2], (char*)NULL }; //argv[] argument that need to be passed to the function "execvp" in order to execute the executable
  //"tobexecuted" in a new konsole with argument 10. Here I'm passing the argv[1] (and argv[2])of the executorkonsole that I suppose to be the IP address (and the port number) of the machine passed from the shell as first argument
  //when running the executorkonsole
  char * arg_list_2[] = { "/usr/bin/konsole",  "-e", "./consumer", argv[1], argv[2], (char*)NULL };  
  int pid1;
  int pid2;


  //RUN THE TWO PROCESSES IN TWO DIFFERENT KONSOLES AND STORE THEIR PIDS IN TWO DIFFERENT VARIABLES
  pid1 = spawn("/usr/bin/konsole", arg_list_1);
  printf("1st konsole pid = %d\n", pid1);
  fflush(stdout);
  sleep(1);
  pid2 = spawn("/usr/bin/konsole", arg_list_2);
  printf("2nd konsole pid = %d\n", pid2);
    fflush(stdout);

  //EXIT
  printf ("Main program exiting...\n");
  return 0;
}
