#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Function that prints the perror when called
void error(char *msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    // If the master doesn't pass all the necessary arguments it prints an error
    if (argc < 3){
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(EXIT_FAILURE);
    }

    // Define all the variables and the structs necessary to the socket communication
    int fd_sock, port_n;
    struct sockaddr_in server_addr;
    struct hostent *server;

    // Define a buffer where save utility information such as commands or time values
    char buff[80];

    // Initialize a buffer where to insert the read data
  	char *data;
  	int bytes= 10*1024*1024;          // 1mb
  	data= (char *) malloc(bytes);
    int n;          // debugging variable to keep track of the number of written/read bytes

    // Define all the variables and structs necessary to keep track of the time
    struct timespec ending_time;
    uint64_t tf;

    // Create the socket
    fd_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_sock < 0){
        error("socket() failed");
    }

    // Get the IPv4 and the port number of the server
    server= gethostbyname(argv[1]);
    if (server == NULL){
        fprintf(stderr,"ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }
    port_n= atoi(argv[2]);

    // Initialize the server_addr struct by setting it at zero
    bzero((char *) &server_addr, sizeof(server_addr));
    // Fill the sockaddr_in struct
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_n);

    // Connect to the server
    if (connect(fd_sock,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
      error("connect() error");
    }

    int new_experiment= 1;
    int iterations= 0;

    printf("\nCONSUMER PROCESS\n");
    fflush(stdout);

    while(1){
      if(new_experiment == 1){
        // Print a simple user interface
        printf("\nStarting a new experiment...\n");
        printf("  - enter the number of times to repeat the test to procede;\n");
        printf("  - enter 'q' to quit;\n\n");
        fflush(stdout);

        while(1){
          // Get the command from stdin
          printf("New command: ");
      	  fflush(stdout);
      		fgets(buff, 79, stdin);

          // Send the input to the server
      	  n = write(fd_sock, buff, strlen(buff));
      	  if (n < 0){
            error("write() on the socket failed");
          }

          // Filter the input
  		    if ((buff[0] == 'q') && (buff[1] == '\n')){
            close(fd_sock);
            exit(EXIT_SUCCESS);
          }
          else if(isdigit(buff[0]) && (buff[1] == '\n')){
            // One-digit number has been insert
            sscanf(buff, "%d", &iterations);
            break;
          }
          else if(isdigit(buff[0]) && isdigit(buff[1]) && (buff[2] == '\n')){
            // Two-digit number has been insert
            sscanf(buff, "%d", &iterations);
            break;
          }
          printf("Unknown command, please try again.\n\n");
          fflush(stdout);
          // Clear the buffer at every cycle
          bzero(buff, 80);
  		  }

        new_experiment= 0;
      }

      printf("Receiving data...\n");
      fflush(stdout);

      // Read the data sent by the producer one at the time
      for(int i=0; i<bytes; i++){
        n= read(fd_sock, &data[i], sizeof(data[i]));
        if (n < 0){
          error("read() from the socket failed.");
        }

        //printf("%c", data[i]);
        //fflush(stdout);
      }

      // Get the time instant in which the consumer finishes to read the data
      if(clock_gettime(CLOCK_REALTIME, &ending_time) == -1 ){
        error( "clock_gettime() failed" );
		  }
      tf= ending_time.tv_sec * 1e3 + ending_time.tv_nsec/1e6;  // ms

      // Clear both the data and utility buffers
      bzero(data, bytes);
      bzero(buff, 80);

      // Send the final time to the producer
      sprintf(buff, "%lu", tf);
      n= write(fd_sock, &buff, sizeof(buff));
      if (n < 0){
        error("write() on the socket failed.");
      }

      iterations--;
      if(iterations == 0){
        printf("Experiment ended.\n");
        fflush(stdout);
        new_experiment= 1;
      }

    }
    return 0;
}
