#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#define BUFFERSIZE 512
#define EXIT -1


void sendMsgtoClient(int clientFD, char *str) {
    int sbuf = (strlen(str)-1)/BUFFERSIZE + 1;
    int n = write(clientFD, &sbuf, sizeof(int));
    char *msgToSend = (char*)malloc(sbuf*BUFFERSIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < sbuf; ++i) {
        int n = write(clientFD, msgToSend, BUFFERSIZE);
        msgToSend += BUFFERSIZE;
    }
}

char* recieveMsgFromClient(int clientFD) {
    int rbuf = 0;
    int n = read(clientFD, &rbuf, sizeof(int));
    if(n <= 0) {
        shutdown(clientFD, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(rbuf*BUFFERSIZE);
    memset(str, 0, rbuf*BUFFERSIZE);
    char *str_p = str;
    int i;
    for(i = 0; i < rbuf; ++i) {
        int n = read(clientFD, str, BUFFERSIZE);
        str = str+BUFFERSIZE;
    }
    return str_p;
}




void closeclient(int client_fd,char *str)
{
	sendMsgtoClient(client_fd, str);
    shutdown(client_fd, SHUT_RDWR);
}



int main(int argc,char **argv)
{
	int sock_fd,client_fd,port_no;
	struct sockaddr_in serv_addr, cli_addr; socklen_t clisize;


	memset((void*)&serv_addr, 0, sizeof(serv_addr));
  if (argc != 3) {
  printf("Usage: ./prog host port\n");
  exit(1);
  }
	port_no=atoi(argv[2]);

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_port = htons(port_no);         //set the port number
	serv_addr.sin_family = AF_INET;             //set domain
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    printf("Error on binding.\n");
	    exit(EXIT_FAILURE);
	}
	int reuse=1;
	listen(sock_fd, 5);
	clisize = sizeof(struct sockaddr_in);

	while(1) {
	    //blocking call
	    memset(&cli_addr, 0, sizeof(cli_addr));
	    if((client_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &clisize)) < 0) {
	        printf("Error on accept.\n");
	        exit(EXIT_FAILURE);
	    }

	    switch(fork()) {
	        case -1:
	            printf("Error in fork.\n");
	            break;
	        case 0: {
	            close(sock_fd);
	            talkToClient(client_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        default:
	            close(client_fd);
	            break;
	    }
	}


}
