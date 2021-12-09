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
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 3) { //if I don't pass the arguments from shell: error
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }


    int sockfd, portno, n; //define socket descriptor, the port number, the number of written/read bytes
    struct sockaddr_in serv_addr; //define the struct serv_addr of type sockaddr_in to store the server address
    struct hostent *server; //define the struct of the server of type hostent-pointer
    char buffer[30]; //define a buffer for reading and writing
    char read_char; //define the read_char variable

    //Variables to evaluate the readng time in milliseconds
    struct timespec start, stop; //struct of type timespec: composed of two fields: time_t tv_sec; long tv_nsec.
    double exec_time;

    portno = atoi(argv[2]); //retrieve the port number from the argv[]
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the socket and return the socket descriptor
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]); //address the host with the IP passed via shell
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr)); // void bzero(void *s, size_t n); the bzero() function erases the data in the n bytes of the memory starting at the location pointed to by s, by writing zeros (bytes containing '\0') to that area.
    serv_addr.sin_family = AF_INET; //store the socket family in the field of the struct
    bcopy((char *)server->h_addr, // void bcopy(const void *src, void *dest, size_t n); The bcopy() function copies n bytes from src to dest.
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno); //The htons function takes a 16-bit number in host byte order and returns a 16-bit number in network byte order used in TCP/IP networks. Here I store the port number in the struct
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) //connect to the server passing the socket descriptor, and the serv_addr struct of type sockaddr_in
        error("ERROR connecting");
    
    while(1){
        bzero(buffer,30); //clear the buffer
        while (buffer[0] != 'y'){
            bzero(buffer,30); //clear the buffer
            printf("enter 'y' to allow the exchange of data, 'n' to exit: ");
    	    fflush(stdout);
    		fgets(buffer,29,stdin); //store the string entering from stdin in the buffer, \n included
    	    n = write(sockfd,buffer,strlen(buffer)); //write the buffer on the socket
    	    if (n < 0) error("ERROR writing to socket");
		
		    if ((buffer[0] == 'n')&&(buffer[1] == '\n')){ 
                close(sockfd);
                exit(EXIT_SUCCESS); //exit if a q character is detected
            }
			if (buffer[1] != '\n'){
				buffer[0]='z';
			}
		}

        read_char = 'z'; //kinda empty the read_char variable
        if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      		perror( "clock gettime" );
      		exit( EXIT_FAILURE );
    	}
        while(read_char != 'b'){
            n = read(sockfd,&read_char,sizeof(read_char)); //read from the socket and store in the buffer
            if (n < 0) error("ERROR reading from socket");
            
            printf("%c",read_char); //print the content of the buffer
            fflush(stdout);
        }
        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
        exec_time = (( stop.tv_sec - start.tv_sec )*1e3) + (( stop.tv_nsec - start.tv_nsec ) / 1e6); 
    	printf( "\nthe reading time was: %lf\n", exec_time );
		fflush(stdout);
    }
    return 0;
}
