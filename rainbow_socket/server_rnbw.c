#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void error(char *msg) //function that prints the perror when called
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {

	if (argc < 2) { //if I don't pass the arguments from shell: error
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
    }

	
	int sockfd, newsockfd, portno, clilen; //define two socket descriptors, the port number, the 
    struct sockaddr_in serv_addr, cli_addr; //define the struct serv_addr of type sockaddr_in to store the server address - define the struct cli_addr of type sockaddr_in to store the client address
    int n; //define the number of written/read bytes
	char buffer[30];

	//Variables to evaluate the transfer time in milliseconds
    struct timespec start, stop; //struct of type timespec: composed of two fields: time_t tv_sec; long tv_nsec.
    double exec_time;


	sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the socket and return the socket descriptor
    if (sockfd < 0) 
    	error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); // void bzero(void *s, size_t n); the bzero() function erases the data in the n bytes of the memory starting at the location pointed to by s, by writing zeros (bytes containing '\0') to that area.
    portno = atoi(argv[1]); //retrieve the port number from the argv[]
    serv_addr.sin_family = AF_INET; //store the socket family in the field of the struct
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);  //The htons function takes a 16-bit number in host byte order and returns a 16-bit number in network byte order used in TCP/IP networks. Here I store the port number in the struct
    if (bind(sockfd, (struct sockaddr *) &serv_addr, //bind the socket to the server address
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,5); //many clients can try to connect to a server. Each one is served one by one. Until the current connection is closed the other clients wait in a queue (5 is the maximum number of clients that can wait in the queue) - initialize the socket
    clilen = sizeof(cli_addr); //retrieve the length in byte of the client address
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //accept conncet() requests from the client side
    if (newsockfd < 0) 
        error("ERROR on accept");
	close(sockfd);
	
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
		bzero(buffer, 30); //clear the buffer
		while (buffer[0] != 'y'){
			bzero(buffer, 30); //clear the buffer
    		n = read(newsockfd,buffer,29); //read from the socket and store in the buffer
    		if (n < 0) error("ERROR reading from socket");

    		if ((buffer[0] == 'n')&&(buffer[1]=='\n')){
				close(newsockfd);
				exit(EXIT_SUCCESS); //exit if a q character is detected
			}
			if ((buffer[0] != 'y')||(buffer[1] != '\n')){
				buffer[0] = 'z';
				printf("\nunrecognized character\n");
    			fflush(stdout);
			}
		}
    	printf("\nthe answer is: %sstarting the exchange of data...\n",buffer);
    	fflush(stdout);

		if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      		perror( "clock gettime" );
      		exit( EXIT_FAILURE );
    	}
		for(int i=0;i<bytes;i++){
     		n = write(newsockfd,&data[i],sizeof(data[i])); //write the buffer on the socket
     		if (n < 0) error("ERROR writing to socket");
		}
		if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
		exec_time = (( stop.tv_sec - start.tv_sec )*1e3) + (( stop.tv_nsec - start.tv_nsec ) / 1e6); 
    	printf( "the transfer time was: %lf\n", exec_time );
		fflush(stdout);
    }

	return 0;
}