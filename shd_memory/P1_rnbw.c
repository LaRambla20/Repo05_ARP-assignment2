#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h> 
#include <signal.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <semaphore.h>
//sprintf scrivo variabili su stringa, sscanf scrivo stringa su variabili

// compile with: gcc P1_rnbw.c -lrt -pthread -o P1_rnbw

void error(char *msg) //function that prints the perror when called
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {

	printf("the passed arguments are: %d\n", argc);
	fflush(stdout);

	if (argc < 6) { //if I don't pass the arguments from shell: error
        fprintf(stderr,"ERROR, not enough arguments provided\n");
        exit(1);
    }

	//Debugging variables
	int n;

	//Variables to evaluate the transfer time in milliseconds
    struct timespec start, stop; //struct of type timespec: composed of two fields: time_t tv_sec; long tv_nsec.
    double exec_time;

	//Variables for reading and writing from the pipe
	int fd; 
	char buffer[30]; //define a buffer for reading and writing

	char * myfifo=argv[1];
	printf("the pipe's name is: %s\n\n", myfifo);
	fflush(stdout);



	//Variables to define the shared memory segment and the semaphores
	const char * shm_name = argv[2];
	printf("the shared memory object's name is: %s\n\n", shm_name);
	fflush(stdout);

	const char * protection_sem = argv[3]; // semaphore that locks and unlocks for protecting the shared memory segment
	const char * empty_cells_sem = argv[4]; // semaphore that blocks (counter = 0) when the number of empty cells is 0 (aka when the buffer is full)
	const char * full_cells_sem= argv[5];  // semaphore that blocks (counter = 0) when the number of full cells is 0 (aka when the buffer is empty)
	printf("the semaphores' names are: %s, %s, %s\n\n", protection_sem, empty_cells_sem, full_cells_sem);
	fflush(stdout);

	const int SIZE = 1024; //the round buffer should be 1KB large

	//Auxiliary variables
	int i, shm_fd;
	int j = 0;
	void * ptr;

	//Define the shared memory segment
	shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	if (shm_fd == 1) {
		printf("Shared memory segment failed\n");
		fflush(stdout);
		exit(1);
	}
	
	//Limit the dimension of the shared memeory segment to SIZE bytes
	ftruncate(shm_fd, SIZE);

	//Connect to the shared memory segment 
	//Establishes a memory-mapped file containing the shared-memory object. It also returns a pointer to the 
	//memory-mapped file that is used for accessing the shared-memory object
	//Our memory buffer will be readable and writable -> protection: PROT_READ | PROT_WRITE;
	//The buffer will be shared (meaning other processes can access it) -> visibility: MAP_SHARED
	ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
 	if (ptr == MAP_FAILED) {
 		printf("Map failed\n");
		fflush(stdout);
 		return 1;
	}
	char * char_ptr = (char *)ptr;

	// Open the semaphores
	// O_CREAT: flag to create a new semaphore if not already existing
	// S_IRUSR and S_IWUSR: macro constant to specify the permissions to the file: respectively: read permissions
	// for the owner and write permissions for the owner
	sem_t * protection_sem_id = sem_open(protection_sem, O_CREAT, S_IRUSR | S_IWUSR);
	if (protection_sem_id == SEM_FAILED) {
    	perror("Failed to open semphore");
    	exit(-1);
	}
	sem_t * empty_cells_sem_id = sem_open(empty_cells_sem, O_CREAT, S_IRUSR | S_IWUSR);
	if (empty_cells_sem_id == SEM_FAILED) {
    	perror("Failed to open semphore");
    	exit(-1);
	}
	sem_t * full_cells_sem_id = sem_open(full_cells_sem, O_CREAT, S_IRUSR | S_IWUSR);
	if (full_cells_sem_id == SEM_FAILED) {
    	perror("Failed to open semphore");
    	exit(-1);
	}

	//Generate the 1MB vector to be transferred
	char *data; //char array because char size is exactly 1 byte (8 bit)
	int bytes = (1024*1024);
	data = (char *) malloc(bytes);
	for(int i=0;i<(bytes-1);i++){
		data[i] = 'a'; //(char) rand()
		printf("%c",data[i]);
		fflush(stdout);
	}
	data[bytes-1] = 'b';
	printf("%c\n", data[bytes-1]);
	fflush(stdout);



	while(1){
		fd = open(myfifo, O_RDONLY);
		bzero(buffer, 30); //clear the buffer
		while (buffer[0] != 'y'){
			bzero(buffer, 30); //clear the buffer
    		n = read(fd,buffer,29); //read from the pipe and store in the buffer
    		if (n < 0) error("ERROR reading from socket");

    		if ((buffer[0] == 'n')&&(buffer[1]=='\n')){
				close(fd);

				munmap(ptr, SIZE);

				sem_close(protection_sem_id);
				sem_close(empty_cells_sem_id);
				sem_close(full_cells_sem_id);

				unlink(myfifo);

				if (shm_unlink(shm_name) == 1) {
                    printf("Error removing %s\n", shm_name);
                    fflush(stdout);
                    exit(1);
                }

				sem_unlink(protection_sem);
				sem_unlink(empty_cells_sem);
				sem_unlink(full_cells_sem);

				exit(EXIT_SUCCESS) ; //exit if a n character is detected
			}
			if ((buffer[0] != 'y')||(buffer[1] != '\n')){
				buffer[0] = 'z';
				printf("\nunrecognized character\n");
    			fflush(stdout);
			}
		}
    	printf("\nthe answer is: %sstarting the exchange of data...\n",buffer);
    	fflush(stdout);
		close(fd);
	

		if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      		perror( "clock gettime" );
      		exit( EXIT_FAILURE );
    	}
	
		for(int i=0;i<bytes;i++){
			sem_wait(empty_cells_sem_id);
			sem_wait(protection_sem_id);
			char_ptr[j] = data[i];
			j = (j + sizeof(data[i]))%SIZE;
			sem_post(protection_sem_id);
			printf("\x1b[31m" "|%d|" "\x1b[0m", j);
			fflush(stdout);
			sem_post(full_cells_sem_id);
		}
		
		if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
		exec_time = (( stop.tv_sec - start.tv_sec )*1e3) + (( stop.tv_nsec - start.tv_nsec ) / 1e6); 
    	printf( "\nthe transfer time was: %lf\n", exec_time );
		fflush(stdout);
    }
	return 0;
}