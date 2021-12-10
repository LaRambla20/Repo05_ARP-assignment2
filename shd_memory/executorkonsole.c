#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <semaphore.h>

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

  // Debugging variable
  int a;

  char * myfifo = "/tmp/myfifo";
  a=mkfifo(myfifo, 0666);
  if (a==-1){
    printf("Pipe error\n");
    fflush(stdout);
    perror("Pipe");
    fflush(stderr);
  }

  // Shared memory variables
  char * shm_name = "/roundbuffer";

  // Semaphores variables
  const int SIZE = 1024;
  char * sem_name1 = "/semaphore1";
	char * sem_name2 = "/semaphore2";
	char * sem_name3 = "/semaphore3";

  //Open and initialize the semaphores
	// O_CREAT: flag to create a new semaphore if not already existing
	// S_IRUSR and S_IWUSR: macro constant to specify the permissions to the file: respectively: read permissions
	// for the owner and write permissions for the owner
  // 1, SIZE and 0 are the values at which the semaphores (that are nothing more than counters) are initialized
	sem_t * sem1_id = sem_open(sem_name1, O_CREAT, S_IRUSR | S_IWUSR, 1);
	if (sem1_id == SEM_FAILED) {
    	perror("Failed to open semphore");
    	exit(-1);
	}
	sem_t * sem2_id = sem_open(sem_name2, O_CREAT, S_IRUSR | S_IWUSR, SIZE);
	if (sem2_id == SEM_FAILED) {
    	perror("Failed to open semphore");
    	exit(-1);
	}
	sem_t * sem3_id = sem_open(sem_name3, O_CREAT, S_IRUSR | S_IWUSR, 0);
	if (sem3_id == SEM_FAILED) {
    	perror("Failed to open semphore");
    	exit(-1);
	}

  char * arg_list_1[] = { "/usr/bin/konsole",  "-e", "./P1_rnbw", myfifo, shm_name, sem_name1, sem_name2, sem_name3, (char*)NULL }; //argv[] argument that need to be passed to the function "execvp" in order to execute the executable
  //"tobexecuted" in a new konsole with argument 10. Here I'm passing the argv[1] (and argv[2])of the executorkonsole that I suppose to be the IP address (and the port number) of the machine passed from the shell as first argument
  //when running the executorkonsole
  char * arg_list_2[] = { "/usr/bin/konsole",  "-e", "./P2_rnbw", myfifo, shm_name, sem_name1, sem_name2, sem_name3, (char*)NULL };  
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

  sem_close(sem1_id);
	sem_close(sem2_id);
	sem_close(sem3_id);

  printf ("Main program exiting...\n");
  return 0;
}

