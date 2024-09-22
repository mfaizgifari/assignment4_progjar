#include <sys/socket.h>  
#include <sys/wait.h>
#include <netinet/in.h>   
#include <unistd.h>       
#include <string.h>       
#include <stdio.h>        
#include <stdlib.h>       
#include <errno.h>        
#include <signal.h>       

#define SERV_PORT 8080    
#define LISTENQ 10        
#define MAXLINE 4096      

void str_echo(int sockfd);
void sig_child(int signo);

int main(int argc, char **argv) {
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    servaddr.sin_port = htons(SERV_PORT);          

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    // Handle child processes when they terminate
    signal(SIGCHLD, sig_child);

    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

        if (connfd < 0) {
            if (errno == EINTR) {
                continue;  // Retry accept on interrupt
            } else {
                perror("accept error");
                exit(1);
            }
        }

        if ((childpid = fork()) == 0) {  // Child process
            close(listenfd);             // Close listening socket
            str_echo(connfd);            // Handle client request
            exit(0);
        }

        close(connfd);  // Parent closes connected socket
    }
}

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];

    while (1) {
        n = read(sockfd, buf, MAXLINE);

        if (n > 0) {
            buf[n] = '\0';  // Ensure null-terminated string
            printf("Received: %s\n", buf);  // Print message received from client
            write(sockfd, buf, n);  // Echo back to client
        } else if (n < 0 && errno == EINTR) {
            continue;  // Retry if interrupted by signal
        } else if (n < 0) {
            perror("str_echo: read error");
            break;
        } else {
            break;  // End when client disconnects
        }
    }
}

// Handle terminated child processes to prevent zombies
void sig_child(int signo) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("Child %d terminated\n", pid);
    }
    return;
}
