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
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 1) { //if I don't pass the arguments from shell: error
         fprintf(stderr,"ERROR, no named pipe provided\n");
         exit(1);
    }

    //Debugging variables
	int n;

	//Variables for reading and writing from the pipe
	int fd; 
	char buffer[30]; //define a buffer for reading and writing
    char read_char;

	char * myfifo=argv[1];
	printf("the pipe's name is: %s\n\n", myfifo);
	fflush(stdout);

    //Variables to evaluate the readng time in milliseconds
    struct timespec start, stop; //struct of type timespec: composed of two fields: time_t tv_sec; long tv_nsec.
    double exec_time;
    
    while(1){
        fd = open(myfifo, O_WRONLY);
        bzero(buffer,30); //clear the buffer
        while (buffer[0] != 'y'){
            bzero(buffer,30); //clear the buffer
            printf("enter 'y' to allow the exchange of data, 'n' to exit: ");
    	    fflush(stdout);
    		fgets(buffer,29,stdin); //store the string entering from stdin in the buffer, \n included
    	    n = write(fd,buffer,strlen(buffer)); //write the buffer on the socket
            fsync(fd);
    	    if (n < 0) error("ERROR writing to socket");
		
		    if ((buffer[0] == 'n')&&(buffer[1] == '\n')){
                unlink(myfifo);
                exit(EXIT_SUCCESS) ; //exit if a q character is detected
            }
			if (buffer[1] != '\n'){
				buffer[0]='z';
			}
		}

        close(fd);

        read_char = 'z'; //kinda empty the read_char variable
        fd = open(myfifo, O_RDONLY);
        if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      		perror( "clock gettime" );
      		exit( EXIT_FAILURE );
    	}
        while(read_char != 'b'){
            n = read(fd,&read_char,sizeof(read_char)); //read from the socket and store in the buffer
            if (n < 0) error("ERROR reading from socket");
            
            printf("%c",read_char); //print the content of the buffer
            fflush(stdout);
        }
        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
        close(fd);
        exec_time = (( stop.tv_sec - start.tv_sec )*1e3) + (( stop.tv_nsec - start.tv_nsec ) / 1e6); 
    	printf( "\nthe reading time was: %lf\n", exec_time );
		fflush(stdout);
    }
    return 0;
}
