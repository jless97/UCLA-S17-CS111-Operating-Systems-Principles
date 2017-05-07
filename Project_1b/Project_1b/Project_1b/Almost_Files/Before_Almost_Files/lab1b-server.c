////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
// Project 1b: lab1b-server.c

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
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
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
// For proper <cr> and <lf> handling
char cr_lf_buf[2] = {CR, LF};
// Pipes used with the --shell option
int terminal_to_shell_pipe[2], shell_to_terminal_pipe[2];
// Poll I/O for use with the --shell option
struct pollfd polled_fds[2];
// Process ID is global so it can be used for Poll I/O and the handler
pid_t pid;
// Server variables
int sockfd, newsockfd, portno;
// Encrypt option flag
int encrypt_flag = 0;
MCRYPT td_encrypt, td_decrypt;
int keysize = 16;
char key[16];
char IV[16] = "initial_vector_@";
int key_fd;
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void print_usage(void);
void handler(int signum);
void init_server(int argc, char *argv[], int portno);
void pipe_creation(int fd[2]);
void pipe_child_io_redirection(void);
void pipe_parent_io_redirection(void);
void pipe_socket_io_redirection(void);
void process_creation(void);
void poll_service_client(pid_t pid);
void poll_service_shell(pid_t pid);
void poll_io_handler(pid_t pid);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
check_exit_status(void) {
    if (pid > 0) {
        int wait_status, exit_status, no_opts = 0;
        int signal, status;
        wait_status = waitpid(pid, &exit_status, no_opts);
        if (wait_status == -1) {
            fprintf(stderr, "Error with waitpid.\n");
            exit(EXIT_FAILURE);
        }
        signal = WTERMSIG(exit_status);
        status = WEXITSTATUS(exit_status);
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal, status);
    }
    
    mcrypt_generic_deinit(td_encrypt);
    mcrypt_generic_deinit(td_decrypt);
    mcrypt_module_close(td_encrypt);
    mcrypt_module_close(td_decrypt);
    
    exit(EXIT_SUCCESS);
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
    printf("Usage: lab1b-server [--port=port#] [--encrypt=filename]\n");
}

void
handler(int signum) {
    if (signum == SIGINT) {
        exit(EXIT_SUCCESS);
    }
    if (signum == SIGPIPE) {
        exit(EXIT_SUCCESS);
    }
    if (signum == SIGTERM) {
        exit(EXIT_SUCCESS);
    }
}

void
init_server(int argc, char *argv[], int portno) {
    int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error opening socket.\n");
        exit(EXIT_FAILURE);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if ( (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0 ) {
        fprintf(stderr, "Error on binding.\n");
        exit(EXIT_FAILURE);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        fprintf(stderr, "Error on accepting.\n");
        exit(EXIT_FAILURE);
    }
}

void
pipe_creation(int fd[2]) {
    int pipe_status = pipe(fd);
    if (pipe_status == -1) {
        fprintf(stderr, "Error creating pipe.\n");
        exit(EXIT_FAILURE);
    }
}

void
pipe_child_io_redirection(void) {
    // Shell process doesn't need to read
    // Redirection so that shell can write to stdin of terminal
    close(terminal_to_shell_pipe[1]);
    dup2(terminal_to_shell_pipe[0], 0);
    
    // Terminal doesn't need to write
    // Redirection so that terminal can read from stdout and stderr of shell
    close(shell_to_terminal_pipe[0]);
    dup2(shell_to_terminal_pipe[1] , 1);
    dup2(shell_to_terminal_pipe[1] , 2);
    
    // Close the now redundant file descriptors that were just duplicated
    close(terminal_to_shell_pipe[0]);
    close(shell_to_terminal_pipe[1]);
}

void pipe_parent_io_redirection(void) {
    // Close pipes not required by parent terminal process
    close(terminal_to_shell_pipe[0]);
    close(shell_to_terminal_pipe[1]);
}

/////// TODO: RESOLVE THIS ///////
//void
//pipe_socket_io_redirection(void) {
//    dup2(newsockfd, 0);
//    dup2(newsockfd, 1);
//    dup2(newsockfd, 2);
//    close(newsockfd);
//}

void
process_creation(void) {
    int exec_failure;
    
    pipe_creation(terminal_to_shell_pipe);
    pipe_creation(shell_to_terminal_pipe);
    
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: fork failed.\n");
        exit(EXIT_FAILURE);
    }
    // Child Process
    else if (pid == 0) {
        pipe_child_io_redirection();
        exec_failure = execvp("/bin/bash", NULL);
        if (exec_failure == 1) {
            fprintf(stderr, "Error: exec failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    // Parent Process
    else {
        //Initialize encryption
        if (encrypt_flag == 1) {
            int mcrypt_status, mcrypt_size, i;
            //IV = "initial_vector_@";
            read(key_fd, key, 16);
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
        pipe_parent_io_redirection();

        poll_io_handler(pid);
    }
}

// Read from client (via socket) and write to the shell
void
poll_service_client(pid_t pid) {
    char buf[BUFFER_SIZE];
    ssize_t nread, nwrite;
    int kill_status;
    nread = read(newsockfd, buf, BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading (keyboard).\n");
        exit(EXIT_FAILURE);
    }
    else if (nread == 0) {
        kill_status = kill(pid, SIGTERM);
        if (kill_status == -1) {
            fprintf(stderr, "Error with SIGTERM handling.\n");
            exit(EXIT_FAILURE);
        }
    }
    if(encrypt_flag){
        mdecrypt_generic(td_decrypt, buf, 1);
    }
    switch(buf[0]) {
        case ETX:
            kill_status = kill(pid, SIGINT);
            if (kill_status == -1) {
                fprintf(stderr, "Error with killing shell process.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case EOT:
            close(terminal_to_shell_pipe[1]);
            break;
        default:
            nwrite = write(terminal_to_shell_pipe[1], buf, 1);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing (keyboard).\n");
                exit(EXIT_FAILURE);
            }
            break;
    }
}

// Read from shell, and write to the client (via socket)
void
poll_service_shell(pid_t pid) {
    char buf[BUFFER_SIZE];
    int kill_status;
    ssize_t nread, nwrite;
    nread = read(shell_to_terminal_pipe[0], buf, BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading (shell).\n");
        exit(EXIT_FAILURE);
    }
    if (encrypt_flag == 1) {
        mcrypt_generic(td_encrypt, buf, 1);
    }
    switch(buf[0]) {
        default:
            nwrite = write(newsockfd, buf, 1);
            if (nwrite < 0) {
                fprintf(stderr, "Error writing larger read to STDOUT.\n");
                exit(EXIT_FAILURE);
            }
            break;
    }
}

void
poll_io_handler(pid_t pid) {
    // Input received from the network socket
    polled_fds[0].fd = newsockfd;
    // Input received from the shell pipes (receives both STDOUT and STDERR)
    polled_fds[1].fd = shell_to_terminal_pipe[0];

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
            //Check for events on socket
            if (polled_fds[0].revents & POLLIN) {
                poll_service_client(pid);
            }
            // Check for errors on keyboard
//            if (polled_fds[0].revents & (POLLHUP | POLLERR)) {
//                exit(EXIT_SUCCESS);
//            }
            // Check for events on the shell
            if (polled_fds[1].revents & POLLIN) {
                poll_service_shell(pid);
            }
            // Check for errors from socket
            if (polled_fds[0].revents & POLLERR) {
                exit(EXIT_SUCCESS);
            }
            // Check for errors from shell
            if (polled_fds[1].revents & POLLHUP) {
                close(shell_to_terminal_pipe[0]);
                exit(EXIT_SUCCESS);
            }
            if (polled_fds[1].revents & POLLERR) {
                exit(EXIT_SUCCESS);
            }
        }
    }
    
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main(int argc, char *argv[]) {
    // Parse command-line options 
    static struct option long_options[] =
    {
        {"port",      required_argument,    0, 'p'},
        {"encrypt",   required_argument,    0, 'e'},
    };
    
    int option;
    while ( (option = getopt_long(argc, argv, "p:e", long_options, NULL)) != -1) {
        switch (option) {
                // Port option
            case 'p':
                signal(SIGINT, handler);
                signal(SIGPIPE, handler);
                signal(SIGTERM, handler);
                portno = atoi(optarg);
                break;
                // Encrypt option
            case 'e':
                encrypt_flag = 1;
                //create_key("my.key");
                key_fd = open(optarg, O_RDONLY);
                break;
                // Unrecognized argument
            default:
                print_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
    
    // Upon exit of program, harvest the exit status
    atexit(check_exit_status);
    
    // Initialize server program
    init_server(argc, argv, portno);
    
    // Create child process
    process_creation();
    
    return EXIT_SUCCESS;
}




