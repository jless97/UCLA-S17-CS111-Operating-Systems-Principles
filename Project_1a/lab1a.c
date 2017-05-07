#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 1024

#define ETX 0x03
#define EOT 0x04
#define LF 0x0A
#define CR 0x0D

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
// Indicator of the --shell option to be used when restoring terminal mode
int shell_flag = 0;

void print_usage(void);
void parser(int argc, char * argv[]);
void handler(int signum);
void restore_normal_mode(void);
void set_input_mode(void);
void stdin_to_stdout(int ifd, int ofd);
void pipe_creation(int fd[2]);
void pipe_child_io_redirection(void);
void pipe_parent_io_redirection(void);
void process_creation(void);
void poll_io_handler(pid_t pid);

int
main (int argc, char * argv[])
{
    // Parse command-line options
    parser(argc, argv);
    
    // Save normal terminal mode to restore upon exit, and configure desired terminal mode for process execution
    set_input_mode();
    
    // Extention of program to support the --shell option
    if (shell_flag == 1) {
        process_creation();
    }
    
    // Non-canonical, no echo, full duplex terminal I/O
    stdin_to_stdout(STDIN_FILENO, STDOUT_FILENO);
    
    return EXIT_SUCCESS;
}

void
print_usage(void) {
    printf("Usage: lab1 [shell]\n");
}

void
parser(int argc, char * argv[]) {
    static struct option long_options[] =
    {
        {"shell", no_argument, 0, 's'}
    };
    
    int option;
    while ( (option = getopt_long(argc, argv, "s", long_options, NULL)) != -1) {
        switch (option) {
            // Shell option
            case 's':
                shell_flag = 1;
                signal(SIGINT, handler);
                signal(SIGPIPE, handler);
                break;
            // Unrecognized argument
            case '?':
                print_usage();
                exit(EXIT_FAILURE);
                break;
            // Normal program mode (i.e. no shell option)
            default:
                break;
        }
    }
}

void
handler(int signum) {
    if (signum == SIGINT) {
        exit(EXIT_SUCCESS);
    }
    if (signum == SIGPIPE) {
        exit(EXIT_SUCCESS);
    }
}

void
restore_normal_mode (void)
{
    if ( (tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes)) < 0) {
        fprintf(stderr, "Error restoring terminal mode.\n");
        exit(EXIT_FAILURE);
    }
    // Printing out shell's exit status when program shuts down
    if (shell_flag == 1) {
        int wait_status, exit_status, no_opts = 0;
        int signal, status;
        wait_status = waitpid(pid, &exit_status, no_opts);
        if (wait_status == -1) {
            fprintf(stderr, "Error with waitpid.\n");
            exit(EXIT_FAILURE);
        }
        // Normal exit status
        if ( WIFEXITED(exit_status) ) {
            signal = WTERMSIG(exit_status);
            status = WEXITSTATUS(exit_status);
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal, status);
        }
        // Exit status due to specific signals
        else if ( WIFSIGNALED(exit_status) ) {
            signal = WTERMSIG(exit_status);
            status = WEXITSTATUS(exit_status);
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal, status);
        }
        else {
            fprintf(stderr, "SHELL EXIT SIGNAL=NONE STATUS=NORMAL\n");
        }

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

// Normal program exeuction (i.e. no shell option)
void
stdin_to_stdout(int ifd, int ofd)
{
    char buf[BUFFER_SIZE];
    ssize_t nread, nwrite;
    int i, end_of_file = 0;
    while (!end_of_file) {
        nread = read(ifd, buf, BUFFER_SIZE);
        if (nread < 0) {
            fprintf(stderr, "Error reading from STDIN.\n");
            exit(EXIT_FAILURE);
        }
        
        for (i = 0; i < nread; i++) {
            if (buf[i] == EOT) {
                end_of_file = 1;
                break;
            }
            else {
                switch(buf[i]) {
                    case CR:
                    case LF:
                        nwrite = write(ofd, cr_lf_buf, 2);
                        if (nwrite < 0) {
                            fprintf(stderr, "Error writing CR & LF using larger read to STDOUT.\n");
                            exit(EXIT_FAILURE);
                        }
                        i++;
                        break;
                    case EOT:
                        end_of_file = 1;
                        break;
                    default:
                        nwrite = write(ofd, buf + i, 1);
                        if (nwrite < 0) {
                            fprintf(stderr, "Error writing larger read to STDOUT.\n");
                            exit(EXIT_FAILURE);
                        }
                        break;
                }
            }
        }
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
        pipe_parent_io_redirection();
        poll_io_handler(pid);
    }
}

void
poll_service_shell(pid_t pid) {
    char buf[BUFFER_SIZE];
    ssize_t nread, nwrite;
    int i;
    nread = read(polled_fds[1].fd, buf, BUFFER_SIZE);
    if (nread == -1) {
        fprintf(stderr, "Error reading (shell).\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nread; i++) {
        switch(buf[i]) {
            case LF:
                nwrite = write(STDOUT_FILENO, cr_lf_buf, 2);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing CR & LF using larger read to STDOUT.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                nwrite = write(STDOUT_FILENO, buf + i, 1);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing larger read to STDOUT.\n");
                    exit(EXIT_FAILURE);
                }
                break;
        }
    }
}

void
poll_service_keyboard(pid_t pid) {
    char buf[BUFFER_SIZE];
    ssize_t nread, nwrite;
    int i, kill_status;
    nread = read(STDIN_FILENO, buf, BUFFER_SIZE);
    if (nread == -1) {
        fprintf(stderr, "Error reading (keyboard).\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nread; i++) {
        switch(buf[i]) {
            case CR:
            case LF:
                nwrite = write(STDOUT_FILENO, cr_lf_buf, 2);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing CR & LF using larger read to STDOUT.\n");
                    exit(EXIT_FAILURE);
                }
                nwrite = write(terminal_to_shell_pipe[1], &cr_lf_buf[1], 1);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing CR & LF (keyboard).\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case ETX:
                kill_status = kill(pid, SIGINT);
                if (kill_status == -1) {
                    fprintf(stderr, "Error with killing shell process.\n");
                }
                break;
            case EOT:
                close(terminal_to_shell_pipe[1]);
                break;
            default:
                nwrite = write(STDOUT_FILENO, buf + i, 1);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing larger read to STDOUT.\n");
                    exit(EXIT_FAILURE);
                }
                nwrite = write(terminal_to_shell_pipe[1], buf + i, 1);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing (keyboard).\n");
                    exit(EXIT_FAILURE);
                }
                break;
        }
    }
}

void
poll_io_handler(pid_t pid) {
    polled_fds[0].fd = STDIN_FILENO;
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
            // Check for events on the shell
            if (polled_fds[1].revents & POLLIN) {
                poll_service_shell(pid);
            }
            // Check for errors from shell
            if (polled_fds[1].revents & (POLLHUP | POLLERR)) {
                exit(EXIT_SUCCESS);
            }
            //Check for events on keyboard
            if (polled_fds[0].revents & POLLIN) {
                poll_service_keyboard(pid);
            }
            // Check for errors on keyboard
            if (polled_fds[0].revents & (POLLHUP | POLLERR)) {
                break;
            }
        }
    }
    
}





