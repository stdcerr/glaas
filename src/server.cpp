#include <stdio.h> 
#include <string.h> //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <ctype.h>
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
	
#define TRUE 1 
#define FALSE 0 
#define DFLTPORT 8888 
#define MAX_CLIENTS 30
//--------------------------------------------------------------------------------
bool isNumber(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++) {
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}	
//--------------------------------------------------------------------------------
int main(int argc , char *argv[]) 
{
unsigned int portno;
	//check command line args
	if (argc >0 && 
	    isNumber(argv[1])) {
	    unsigned int portno = atoi(argv[1]);
        printf("Listening on portno %u\n",portno);
	} else {
		portno = DFLTPORT;
	}
	int opt = TRUE; 
	int sockfd , addrlen , new_socket , client_socket[MAX_CLIENTS], activity, i , valread , sd; 
	int max_sd; 
	struct sockaddr_in address; 
		
	char buffer[1025]; //data buffer of 1K 
		
	//set of socket descriptors 
	fd_set readfds; 
		
	//a message 
	char message[50];
    strcpy(message,"Welcome, you are connected to the GLaas server\r\n"); 
	
	//initialise all client_socket[] to 0 so not checked 
	for (i = 0; i < MAX_CLIENTS; i++) { 
		client_socket[i] = 0; 
	} 
		
	//create a master socket 
	if( (sockfd = socket(AF_INET , SOCK_STREAM , 0)) == 0) { 
        perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
		sizeof(opt)) < 0 ) { 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( portno ); 
		
	//bind the socket to localhost at the selected portno
	if (bind(sockfd, (struct sockaddr *)&address, sizeof(address))<0) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Listener on portno %u \n", ntohs(portno)); 
		
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(sockfd, 3) < 0) { 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
		
	//accept the incoming connection 
	addrlen = sizeof(address); 
	puts("Waiting for connections ..."); 
		
	while(TRUE) { 
		//clear the socket set 
		FD_ZERO(&readfds); 
	
		//add master socket to set 
		FD_SET(sockfd, &readfds); 
		max_sd = sockfd; 
			
		//add child sockets to set 
		for ( i = 0 ; i < MAX_CLIENTS ; i++) 
		{ 
			//socket descriptor 
			sd = client_socket[i]; 
				
			//if valid socket descriptor then add to read list 
			if(sd > 0) 
				FD_SET( sd , &readfds); 
				
			//highest file descriptor number, need it for the select function 
			if(sd > max_sd) 
				max_sd = sd; 
		} 
	
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
        printf ("Got select!\n");
	
		if ((activity < 0) && (errno!=EINTR)) { 
			printf("select error"); 
		} 
			
		//handle incoming connection 
		if (FD_ISSET(sockfd, &readfds)) { 
			if ((new_socket = accept(sockfd, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n" ,
                   new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
		
			//send new connection greeting message 
			if( send(new_socket, message, strlen(message), 0) != strlen(message) ) { 
				perror("send"); 
			} 
				
			puts("Welcome message sent successfully"); 
				
			//add new socket to array of sockets 
			for (i = 0; i < MAX_CLIENTS; i++) { 
				//if position is empty 
				if( client_socket[i] == 0 ) { 
					client_socket[i] = new_socket; 
					printf("Adding to list of sockets as %d\n" , i); 
						
					break; 
				} 
			} 
		} 
			
		//else its some IO operation on some other socket 
		for (i = 0; i < MAX_CLIENTS; i++) { 
			sd = client_socket[i];
            printf("DBG checking client %d\n",i); 
				
			if (FD_ISSET( sd , &readfds)) { 
				//Check if it was for closing , and also read the 
				//incoming message 
				if ((valread = read( sd , buffer, 1024)) == 0) { 
                    printf("DBG: client %d closed the connection %d\n",i,sd); 
					//Somebody disconnected , get his details and print 
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen); 
					printf("Host disconnected , ip %s , port %d \n" , 
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
						
					//Close the socket and mark as 0 in list for reuse 
					close( sd ); 
					client_socket[i] = 0; 
				} else { 
                    printf("DBG: data read from client %d at sd %d\n",i,sd); 
					//set the string terminating NULL byte on the end 
					//of the data read 
					buffer[valread] = '\0'; 
					send(sd , buffer , strlen(buffer) , 0 ); 
					//print incoming data to stdout
					printf("%s\n",buffer);
				} 
			} 
		} 
	} 
		
	return 0; 
} 
