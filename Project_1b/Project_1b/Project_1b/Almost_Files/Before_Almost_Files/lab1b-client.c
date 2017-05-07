////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
//Project 1b: lab1b-client.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mcrypt.h>

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define ETX 0x03
#define EOT 0x04
#define LF 0x0A
#define CR 0x0D
#define BUFFER_SIZE 1


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Original terminal attributes to be restored upon exit
struct termios saved_attributes;
// For proper <cr> and <lf> handling
char cr_lf_buf[2] = {CR, LF};
// Pipes used with the --shell option
int terminal_to_shell_pipe[2], shell_to_terminal_pipe[2];
// Poll I/O for use with the --shell option
struct pollfd polled_fds[2];
// Process ID is global so it can be used for Poll I/O and the handler
pid_t pid;
// Client variables
int sockfd, portno;
// Log variables: flag if option enabled, and the log file
int log_flag = 0, log_file;
// Encrypt flag option
int encrypt_flag = 0;
MCRYPT td_encrypt, td_decrypt;
int keysize = 16;
char key[16];
char IV[16] = "initial_vector_@";
int key_fd;
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
                /// TODO: ENCRYPTION FUNCTION PROTOTYPES ///
void print_usage(void);
void parser(int argc, char * argv[]);
void init_client(int argc, char *argv[]);
void restore_normal_mode(void);
void set_input_mode(void);
void poll_service_keyboard(void);
void poll_service_socket(void);
void poll_io_handler(void);
void log_write(int nbytes, char *buf, int socket);
char* create_key(char *key_file);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
// I choose to send the bytes one at a time, thus each line consists of 1 byte sent/received
int write_done = 1, read_done = 1;
void
log_write(int nbytes, char *buf, int socket) {
    if (socket) {
        char sent_buf_format[14] = "SENT 1 bytes: ";
        write(log_file, sent_buf_format, 14);
        write(log_file, buf, 1);
        write(log_file, "\n", 1);

    }
    else {
        char received_buf_format[18] = "RECEIVED 1 bytes: ";
        write(log_file, received_buf_format, 18);
        write(log_file, buf, 1);
        write(log_file, "\n", 1);
    }
    //write(log_file, "\n", 1);
}

char* create_key(char *key_file) {
    int keyfd;
    ssize_t nread;
    keyfd = open(key_file, O_RDONLY);
    if (keyfd < 0) {
        fprintf(stderr, "Error opening my.key file.\n");
        exit(EXIT_FAILURE);
    }
    nread = read(keyfd, key, 16);
    if (nread < 0) {
        fprintf(stderr, "Error reading from my.key file.\n");
        exit(EXIT_FAILURE);
    }
    return key;
}

void
print_usage(void) {
    printf("Usage: lab1b-server [hostname] [--port=port#] [--encrypt=filename] \
           [--log=filename] \n");
}

void
parser(int argc, char * argv[]) {
    static struct option long_options[] =
    {
        {"port",      required_argument,    0, 'p'},
        {"encrypt",   required_argument,    0, 'e'},
        {"log",       required_argument,    0, 'l'}
    };
    
    int option;
    while ( (option = getopt_long(argc, argv, "p:e:l", long_options, NULL)) != -1) {
        switch (option) {
            // Port option
            case 'p':
                portno = atoi(optarg);
                break;
            // Encrypt option
            case 'e':
                encrypt_flag = 1;
                create_key("my.key");
                break;
            // Log option
            case 'l':
                log_flag = 1;
                log_file = creat(optarg, 0666);
                if (log_file < 0) {
                    fprintf(stderr, "Error creating log file.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            // Unrecognized argument
            default:
                print_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void
init_client(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error opening socket.\n");
        exit(EXIT_FAILURE);
    }
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr, "Error: no such host.\n");
        exit(EXIT_FAILURE);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if ( (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0 ) {
        fprintf(stderr, "Error connecting.\n");
        exit(EXIT_FAILURE);
    }
}

void
restore_normal_mode (void)
{
    if ( (tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes)) < 0) {
        fprintf(stderr, "Error restoring terminal mode.\n");
        exit(EXIT_FAILURE);
    }
    
    mcrypt_generic_deinit(td_encrypt);
    mcrypt_generic_deinit(td_decrypt);
    mcrypt_module_close(td_encrypt);
    mcrypt_module_close(td_decrypt);
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
int write_done = 1, read_done = 1;
void
log_write(int nbytes, char *buf, int socket) {
    if (socket) {
        char sent_buf_format[14] = "SENT 1 bytes: ";
        write(log_file, sent_buf_format, 14);
        write(log_file, buf, 1);
        write(log_file, "\n", 1);
        
    }
    else {
        char received_buf_format[18] = "RECEIVED 1 bytes: ";
        write(log_file, received_buf_format, 18);
        write(log_file, buf, 1);
        write(log_file, "\n", 1);
    }
    //write(log_file, "\n", 1);
}
// Read from keyboard and write to the STDOUT and server (via socket)
void
poll_service_keyboard(void) {
    char buf[BUFFER_SIZE];
    ssize_t nread, nwrite;
    int i;
    nread = read(STDIN_FILENO, buf, BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading from STDIN.\n");
        exit(EXIT_FAILURE);
    }
    else if (nread == 0) {
        // Automatically restore terminal modes on exit via atexit(restore...)
        exit(EXIT_SUCCESS);
    }
    switch(buf[0]) {
        case CR:
        case LF:
            nwrite = write(STDOUT_FILENO, cr_lf_buf, 2);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing CR & LF to socket.\n");
                exit(EXIT_FAILURE);
            }
            if(encrypt_flag){
                mcrypt_generic(td_encrypt, cr_lf_buf + 1, 1);
            }
            nwrite = write(sockfd, cr_lf_buf + 1, 1);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing CR & LF to STDOUT.\n");
                exit(EXIT_FAILURE);
            }
            if (log_flag == 1) {
                log_write(2, cr_lf_buf, 1);
            }
            break;
        default:
            nwrite = write(STDOUT_FILENO, buf, 1);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing to socket.\n");
                exit(EXIT_FAILURE);
            }
            if(encrypt_flag){
                mcrypt_generic(td_encrypt, buf, 1);
            }
            nwrite = write(sockfd, buf, 1);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing to STDOUT.\n");
                exit(EXIT_FAILURE);
            }
            if (log_flag == 1) {
                log_write(nread, buf, 1);
            }
            break;
    }
}

// Read from the server (via socket) and write it to STDOUT
void
poll_service_socket(void) {
    char buf[BUFFER_SIZE];
    ssize_t nread, nwrite;
    int i, j;
    nread = read(sockfd, buf, BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading (socket).\n");
        exit(EXIT_FAILURE);
    }
    else if (nread == 0) {
        exit(EXIT_SUCCESS);
    }
    if (log_flag == 1) {
        if (buf[0] != CR) {
            log_write(nread, buf, 0);
        }
    }
    if(encrypt_flag){
        mdecrypt_generic(td_decrypt, buf, 1);
    }
    switch(buf[0]) {
        //case CR:
        case LF:
            nwrite = write(STDOUT_FILENO, cr_lf_buf, 2);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing CR & LF to STDOUT.\n");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            nwrite = write(STDOUT_FILENO, buf, 1);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing to STDOUT.\n");
                exit(EXIT_FAILURE);
            }
            break;
    }
}

void
poll_io_handler(void) {
    // Input received from the keyboard
    polled_fds[0].fd = STDIN_FILENO;
    // Input received from the socket
    polled_fds[1].fd = sockfd;
    polled_fds[0].events = POLLIN | POLLHUP | POLLERR;
    polled_fds[1].events = POLLIN | POLLHUP | POLLERR;
    
    int poll_status, num_fds = 2, timeout = 0;
    while(1) {
        poll_status = poll(polled_fds, num_fds, timeout);
        if (poll_status == -1) {
            fprintf(stderr, "Error with poll I/O.\n");
            exit(EXIT_FAILURE);
        }
        else {
            //Check for events on keyboard
            if (polled_fds[0].revents & POLLIN) {
                if (log_flag == 1) {
                    write_done = 1;
                    if (read_done) {
                        read_done = 0;
                        write(log_file, "\n", 0);
                    }
                }
                poll_service_keyboard();
            }
            // Check for events on the socket
            if (polled_fds[1].revents & POLLIN) {
                read_done = 1;
                if (log_flag == 1) {
                    if (write_done) {
                        write_done = 0;
                        write(log_file, "\n", 1);
                    }
                }
                poll_service_socket();
            }
            // Check for errors on keyboard
            if (polled_fds[0].revents & (POLLHUP | POLLERR)) {
                exit(EXIT_SUCCESS);
            }
            // Check for errors from socket
            if (polled_fds[1].revents & (POLLHUP | POLLERR)) {
                exit(EXIT_SUCCESS);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char * argv[])
{
    // Parse command-line options
    parser(argc, argv);
    
    // Save normal terminal mode to restore upon exit, and configure desired terminal mode for process execution
    set_input_mode();
    
    // Set up client
    init_client(argc, argv);
    
    //Initialize encryption
    if (encrypt_flag == 1) {
        int mcrypt_status, mcrypt_size, i;
        //IV = "initial_vector_@";
        td_encrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL);
        if (td_encrypt == MCRYPT_FAILED) {
            fprintf(stderr, "Error opening mcrypt module: td_encrypt.\n");
            exit(EXIT_FAILURE);
        }
        td_decrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL);
        if (td_decrypt == MCRYPT_FAILED) {
            fprintf(stderr, "Error opening mcrypt module: td_decrypt.\n");
            exit(EXIT_FAILURE);
        }
        mcrypt_status = mcrypt_generic_init(td_encrypt, key, keysize, IV);
        if (mcrypt_status < 0) {
            fprintf(stderr, "Error with mcrypt init: td_encrypt.\n");
            exit(EXIT_FAILURE);
        }
        mcrypt_status = mcrypt_generic_init(td_decrypt, key, keysize, IV);
        if (mcrypt_status < 0) {
            fprintf(stderr, "Error with mcrypt init: td_decrypt.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    poll_io_handler();
    
    exit(EXIT_SUCCESS);
}

