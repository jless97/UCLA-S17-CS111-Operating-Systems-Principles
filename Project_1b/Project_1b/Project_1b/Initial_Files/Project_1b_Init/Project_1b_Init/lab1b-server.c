/* A simple server in the internet domain using TCP
 The port number is passed as an argument */

#include <stdio.h>
#include <string.h>
#include <mcrypt.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <netinet/in.h>

int socket_fd, newsocket_fd;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int opt = 0, port_num, client_len;
    struct sockaddr_in server_addr, client_addr;
    static struct option long_opts[] =
    {
        {"port", required_argument, 0, 'p'},
        {"encrypt", no_argument, 0, 'e'}
    };
    
    while((opt = getopt_long(argc, argv, "p:e", long_opts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'p':
                //Grab port number
                port_num = atoi(optarg);
                break;
            default:
                //Usage message
                fprintf(stderr, "Usage [e] port_number");
                break;
        }
    }
    //Set up socket connection
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0) { perror("Error opening socket"); exit(1); }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }
    //Listen for data over the socket
    listen(socket_fd, 5);
    client_len = sizeof(client_addr);
    //Block while accepting data input as a stream
    newsocket_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_len);
    if(newsocket_fd < 0) { perror("Error accepting the socket"); exit(1); }
}
