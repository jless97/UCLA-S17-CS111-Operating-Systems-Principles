#include <termios.h>
#include <mcrypt.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

struct termios saved_attributes;
int socket_fd, port_num;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void
restore_normal_mode (void)
{
    if ( (tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes)) < 0) {
        fprintf(stderr, "Error restoring terminal mode.\n");
        exit(EXIT_FAILURE);
    }
}

void
set_input_mode (void)
{
    struct termios tty_attr;
    
    if ( !isatty(STDIN_FILENO) )
    {
        fprintf (stderr, "Not a terminal.\n");
        exit(EXIT_FAILURE);
    }
    
    // Get current terminal modes to be restored at end of program execution
    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(restore_normal_mode);
    
    // Set desired terminal attributes as defined in the specification
    tcgetattr (STDIN_FILENO, &tty_attr);
    tty_attr.c_iflag = ISTRIP;
    tty_attr.c_lflag = 0;
    tty_attr.c_oflag = 0;
    tty_attr.c_cc[VMIN] = 1;
    tty_attr.c_cc[VTIME] = 0;
    if ( (tcsetattr (STDIN_FILENO, TCSANOW, &tty_attr)) < 0) {
        fprintf(stderr, "Error setting up current terminal mode.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    set_input_mode();
    int opt;
    char buffer[256];
    ssize_t n;
    struct sockaddr_in server_addr;
    struct hostent* server;
    static struct option long_opts[] =
    {
        {"port", required_argument, 0, 'p'},
        {"log", required_argument, 0, 'l'},
        {"encrypt", no_argument, 0, 'e'}
    };
    while((opt = getopt_long(argc,argv,"p:l:e",long_opts,NULL)) != -1)
    {
        switch(opt)
        {
            case 'p':
                //Grab the port number
                port_num = atoi(optarg);
                break;
            default:
                //Default usage message
                fprintf(stderr, "Usage [le] hostname port_num");
                break;
        }
    }
    
    //Socket initialzation
    //Establish socket file descriptor, for TCP with internet domain
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0) { perror("Error opening socket"); exit(0); }
    server = gethostbyname("localhost");
    if(server == NULL) { fprintf(stderr, "Cannot find host"); exit(0); }
    //Initialize the server address to zero and then correctly assign it
    memset((char*) &server_addr,0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy((char *) &server_addr.sin_addr.s_addr,
           (char*) server->h_addr,
           server->h_length);
    server_addr.sin_port = htons(port_num);
    //Connect to our established server using the socket
    if(connect(socket_fd,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) { perror("Error connecting"); exit(0); }
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(socket_fd,buffer,strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(socket_fd,buffer,255);
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n",buffer);
    return 0;
}
