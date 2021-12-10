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

void error(char *msg) //function that prints the perror when called
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {

	if (argc < 1) { //if I don't pass the arguments from shell: error
         fprintf(stderr,"ERROR, no named pipe provided\n");
         exit(1);
    }

	//Debugging variables
	int n;

	//Variables for reading and writing from the pipe
	int fd; 
	char buffer[30]; //define a buffer for reading and writing

	char * myfifo=argv[1];
	printf("the pipe's name is: %s\n\n", myfifo);
	fflush(stdout);

	//Variables to evaluate the transfer time in milliseconds
    struct timespec start, stop; //struct of type timespec: composed of two fields: time_t tv_sec; long tv_nsec.
    double exec_time;
	
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
    		n = read(fd,buffer,29); //read from the socket and store in the buffer
    		if (n < 0) error("ERROR reading from socket");

    		if ((buffer[0] == 'n')&&(buffer[1]=='\n')) exit(EXIT_SUCCESS) ; //exit if a q character is detected
			if ((buffer[0] != 'y')||(buffer[1] != '\n')){
				buffer[0] = 'z';
				printf("\nunrecognized character\n");
    			fflush(stdout);
			}
		}
    	printf("\nthe answer is: %sstarting the exchange of data...\n",buffer);
    	fflush(stdout);
		close(fd);

		fd = open(myfifo, O_WRONLY);
		if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      		perror( "clock gettime" );
      		exit( EXIT_FAILURE );
    	}
		for(int i=0;i<bytes;i++){
     		n = write(fd,&data[i],sizeof(data[i])); //write the buffer on the socket
			fsync(fd);
     		if (n < 0) error("ERROR writing to socket");
		}
		if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
		close(fd);
		exec_time = (( stop.tv_sec - start.tv_sec )*1e3) + (( stop.tv_nsec - start.tv_nsec ) / 1e6); 
    	printf( "the transfer time was: %lf\n", exec_time );
		fflush(stdout);
    }
	return 0;
}