#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFERSIZE 512

char* receiveMsgFromServer(int sock_fd) {
    int rbuf = 0;
    int n = read(sock_fd, &rbuf, sizeof(int));
    if(n <= 0) {
        shutdown(sock_fd, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(rbuf*BUFFERSIZE);
    memset(str, 0, rbuf*BUFFERSIZE);
    char *str_p = str;
    int i;
    for(i = 0; i < rbuf; ++i) {
        int n = read(sock_fd, str, BUFFERSIZE);
        str = str+BUFFERSIZE;
    }
    return str_p;
}

void sendMsgToServer(int sock_fd, char *str) {
    int sbuf = (strlen(str)-1)/BUFFERSIZE + 1;
    int n = write(sock_fd, &sbuf, sizeof(int));
    char *msgToSend = (char*)malloc(sbuf*BUFFERSIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < sbuf; ++i) {
        int n = write(sock_fd, msgToSend, BUFFERSIZE);
        msgToSend += BUFFERSIZE;
    }
}
    while(1) {
        msgFromServer = receiveMsgFromServer(sock_fd);
        if(msgFromServer == NULL)
            break;
        printf("%s\n",msgFromServer);
        free(msgFromServer);
    }
    printf("Write end closed by the server.\n");
    shutdown(sock_fd, SHUT_RD);
    printf("Connection closed gracefully.\n");
    return 0;


}
