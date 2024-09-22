#include <sys/socket.h>  
#include <netinet/in.h>   
#include <arpa/inet.h>    
#include <unistd.h>       
#include <string.h>       
#include <stdio.h>        
#include <stdlib.h>       

#define SERV_PORT 8080    
#define MAXLINE 4096      

void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: tcpcli <IPaddress>\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    str_cli(stdin, sockfd);  // Send data to server and handle echo

    exit(0);
}

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        write(sockfd, sendline, strlen(sendline));  // Send data to server

        if (read(sockfd, recvline, MAXLINE) == 0) {
            fprintf(stderr, "str_cli: server terminated prematurely\n");
            exit(1);
        }

        fputs(recvline, stdout);  // Print echo from server
        bzero(recvline, MAXLINE);  // Clear the buffer after printing
    }
}
