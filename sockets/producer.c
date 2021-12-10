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
	if(argc < 2){
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
  }

  // Define all the variables and the structs necessary to the socket communication
  int fd_wlcm_sock, fd_sock, client_size, read_size, port_n;
  struct sockaddr_in server_addr, client_addr;

  // Define all the variables and structs necessary to keep track of the time
  struct timespec starting_time;
  uint64_t ti, tf, average_elapsed_time;
  uint64_t sum= 0;
  uint64_t *elapsed_time;

  // Create the socket
  fd_wlcm_sock= socket(AF_INET, SOCK_STREAM, 0);
  if (fd_wlcm_sock == -1){
    perror("socket() failed");
  }

	// Initialize the server_addr struct by setting it at zero
  bzero((char *) &server_addr, sizeof(server_addr));
  // Retrieve the port number from argv[]
  port_n = atoi(argv[1]);

  // Fill the sockaddr_in struct
  server_addr.sin_family= AF_INET;
  server_addr.sin_addr.s_addr= INADDR_ANY;
  server_addr.sin_port= htons(port_n);

  // Bind the socket to the server address
  if (bind(fd_wlcm_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
    error("bind() failed");
  }

  // Listen on the socket
  int queue= 3;
  listen(fd_wlcm_sock, queue);

  // Accept a new incoming connection by a client
  client_size= sizeof(client_addr);
  fd_sock= accept(fd_wlcm_sock, (struct sockaddr *) &client_addr, &client_size);
  if (fd_sock < 0){
    error("accept() failed");
  }

	close(fd_wlcm_sock);

  // Initialize a buffer where to insert the read data
	char *data;
	int bytes= 10*1024*1024;  // 1mb
	data= (char *) malloc(bytes);
  bzero(data, bytes);
  int n;          // debugging variable to keep track of the number of written/read bytes

  // Initialize a smaller buffer where to exchange utility info
  char buff[80];
  bzero(buff, 80);  // clear the buffer

  // Flag to identify the first time that the infinite loop is executed
  int new_experiment= 1;
  int tot_iterations= 0;
  int it= 0;

  printf("\nPRODUCER PROCESS\n");
  fflush(stdout);

	while(1){
    // Wait the consumer to ask for exchanging data
    if(new_experiment== 1){
      // Print a simple user interface
      printf("\nWaiting a new experiment to be runned...\n\n");

      while(1){
        // Read the user input from the socket
      	n= read(fd_sock, buff, 79);
      	if (n < 0){
          error("read() from socket failed");
        }

        // Filter the input
        if ((buff[0] == 'q') && (buff[1] == '\n')){
          close(fd_sock);
          exit(EXIT_SUCCESS);
        }
        else if(isdigit(buff[0]) && (buff[1] == '\n')){
          // One-digit number has been insert
          sscanf(buff, "%d", &tot_iterations);
          elapsed_time= (uint64_t *) malloc(tot_iterations);
          break;
        }
        else if(isdigit(buff[0]) && isdigit(buff[1]) && (buff[2] == '\n')){
          // Two-digit number has been insert
          sscanf(buff, "%d", &tot_iterations);
          elapsed_time= (uint64_t *) malloc(tot_iterations);
          break;
        }

        // Clear the buffer at every cycle
        bzero(buff, 80);
  		}

      new_experiment= 0;
    }

    printf("Sending data...\n");
    fflush(stdout);

    // Fill the data buffer with random characters from 'A' to 'Z'
    for(int i=0; i<(bytes-1); i++){
      data[i] = (rand() % (90-65)) + 65;  // 'A'= 65 and 'Z'= 95
      //printf("%c", data[i]);
      //fflush(stdout);
    }

    // Get the time instant in which the producer starts to send the data
    if(clock_gettime(CLOCK_REALTIME, &starting_time) == -1){
			error("clock_gettime() failed");
		}

    // Send the data to the consumer one byte at the time
		for (int i=0; i< bytes; i++){
     		n= write(fd_sock, &data[i], sizeof(data[i]));
     		if(n < 0){
          perror("read() on socket failed");
        }
		}

    // Clear the data buffer
    bzero(data, bytes);

    // Get the time instant in which the consumer finishes to read all the data
    n= read(fd_sock, buff, sizeof(buff));
    if(n < 0){
      perror("read() failed");
      break;
    }

    // Compute the enlapsed time
    ti= starting_time.tv_sec * 1e3 + starting_time.tv_nsec/1e6;  // ms
    printf("Starting time: %lu\n", ti);
    sscanf(buff, "%lu", &tf);
    printf("Ending time: %lu\n", tf);

    elapsed_time[it]= tf - ti;      // elapsed time in ms
    sum= sum + elapsed_time[it];

    it++;
    if(it == tot_iterations){
      // Compute the average
      average_elapsed_time= sum/(tot_iterations+1);
      printf("\nElapsed time on average: %lu ms.\n\n", average_elapsed_time);
      fflush(stdout);

      // Clear all for a new execution
      it= 0;
      sum= 0;
      bzero(elapsed_time, tot_iterations);
      new_experiment= 1;
    }
  }

  return 0;
}
