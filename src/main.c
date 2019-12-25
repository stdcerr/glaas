#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define OK          0
#define ERROR       -1
#define DFLT_PRT    8080
#define SIZE        1024
#define BACKLOG     10
#define BUFSIZE        SIZE


void report(struct sockaddr_in *serverAddress);


//------------------------------------------------------------------
void setHttpHeader(char httpHeader[])
{
    // File object to return
    FILE *htmlData = fopen("index.html", "r");
    if (htmlData == NULL){
        perror("failed to open index.html");
        exit (EXIT_FAILURE);
    }

    char line[100];
    char responseData[8000];
    while (fgets(line, 100, htmlData) != 0) {
        strcat(responseData, line);
    }
    strcat(httpHeader, responseData);
}
//------------------------------------------------------------------

int main (int argc, char *argv[]) {
    int clientSocket;
    uint32_t port;
    char httpHeader[8000] = "HTTP/1.1 200 OK\r\n\n";
    char buf[BUFSIZE]; /* message buffer */
    fd_set readfds;

    port = (argc>1)?strtol(argv[1],NULL,10):DFLT_PRT;
    if (port == LONG_MIN ||
        port == LONG_MAX)
        port = DFLT_PRT;
    if (errno)
        exit(EXIT_FAILURE);

    printf("listening port set to %u\n",port);

    //serverSocket setup
    int serverSocket = socket (
            AF_INET,
            SOCK_STREAM,
            0
        );

    //local address structure
    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port= htonl(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK/*ANY*/);

    if (bind (
         serverSocket,
         (struct sockaddr*)&srv_addr,
        sizeof(srv_addr)
    )!=0) {
        perror("bind operation failed");
        exit (EXIT_FAILURE);
    }

    int listening = listen(serverSocket, BACKLOG);
    if (listening <0) {
        printf("Error: The server is not listenig.\n");
        return listening;
    }
    report(&srv_addr);
    FD_ZERO(&readfds);          /* initialize the fd set */
    FD_SET(serverSocket, &readfds); /* add socket fd */
    FD_SET(0, &readfds);        /* add stdin fd (0) */
    if (select(serverSocket+1, &readfds, 0, 0, 0) < 0) {
      perror("ERROR in select");
      exit (EXIT_FAILURE);
    }
    printf("recv conn req!\n");

    /* if the user has entered a command, process it */
    if (FD_ISSET(0, &readfds)) {
      fgets(buf, BUFSIZE, stdin);
      switch (buf[0]) {
          default:
              printf("buf[0]: 0x%c\n",buf[0]);
      }
    setHttpHeader(httpHeader);
    int clientSocket;
    }

    while(1) {
    clientSocket = accept(serverSocket, NULL, NULL);
    send(clientSocket, httpHeader, sizeof(httpHeader),0);
    close(clientSocket);
    }

return OK;

}
//------------------------------------------------------------------

void report(struct sockaddr_in *serverAddress)
{
    char hostBuffer[INET6_ADDRSTRLEN];
    char serviceBuffer[NI_MAXSERV]; // defined in `<netdb.h>`
    socklen_t addr_len = sizeof(*serverAddress);
    int err = getnameinfo(
        (struct sockaddr *) serverAddress,
        addr_len,
        hostBuffer,
        sizeof(hostBuffer),
        serviceBuffer,
        sizeof(serviceBuffer),
        NI_NUMERICHOST|NI_NUMERICSERV
    );
    if (err != 0) {
        printf("It's not working!!\n");
        exit (EXIT_FAILURE);
    }
    printf("\n\n\tServer listening on http://%s:%s\n", hostBuffer, serviceBuffer);
}
//------------------------------------------------------------------
