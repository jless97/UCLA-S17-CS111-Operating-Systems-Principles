//Name: Jason Less
//Email: jaless1997@gmail.com
//ID: 404-640-158

//Project 0: lab0.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>

void print_usage()
{
    printf("Usage: lab0 [sc] -i input_file -o output_file\n");
}

void handler(int signum)
{
    if (signum == SIGSEGV)
    {
        fprintf(stderr, "Error: Segmentation fault\n");
        exit(4);
    }
}

//Copies STDIN to STDOUT: 1 byte at a time
void STDIN_TO_STDOUT(int ifd, int ofd)
{
    char *buf = (char *)malloc(sizeof(char));
    ssize_t state;
    while ( (state = read(ifd, buf, 1)) > 0)
    {
        write(ofd, buf, 1);
    }
}

int main(int argc, char * argv[])
{
    static struct option long_options[] =
    {
        {"input",    required_argument,    0, 'i'},
        {"output",   required_argument,    0, 'o'},
        {"segfault", no_argument,          0, 's'},
        {"catch",    no_argument,          0, 'c'},
    };
    
    int option;
    int ifd, ofd;
    char *input_file, *output_file = NULL;
    int input_file_flag = 0, output_file_flag = 0;
    int segfault_flag = 0, catch_segfault_flag = 0;
    char *force_segfault = NULL;
    
    while ( (option = getopt_long(argc, argv, "i:o:sc", long_options, NULL)) != -1)
    {
        switch (option)
        {
            //Input option
            case 'i':
                input_file_flag = 1;
                input_file = optarg;
                break;
            //Output option
            case 'o':
                output_file_flag = 1;
                output_file = optarg;
                break;
            //Segfault option, set the segfault flag
            case 's':
                segfault_flag = 1;
                break;
            //Catch option, set the catch segfault flag
            case 'c':
                catch_segfault_flag = 1;
                break;
            //Unrecognized argument
            default:
                print_usage();
                exit(1);
        }
    }
    
    //Input file provided to be STDIN
    if (input_file_flag == 1)
    {
        ifd = open(input_file, O_RDONLY);
        if (ifd >= 0)
        {
            close(0);
            dup(ifd);
            close(ifd);
        }
        else
        {
            fprintf(stderr, "Error opening file: %s\n", strerror(errno));
            exit(2);
        }
    }
    
    //Output file provided to be STDOUT
    if (output_file_flag == 1)
    {
        ofd = creat(output_file, 0666);
        if (ofd >= 0)
        {
            close(1);
            dup(ofd);
            close(ofd);
        }
        else
        {
            fprintf(stderr, "Error creating file: %s\n", strerror(errno));
            exit(3);
        }
    }
    
    //Register the SIGSEGV handler
    if (catch_segfault_flag == 1)
    {
        signal(SIGSEGV, handler);
    }
    
    //Force the segfault by storing through nullptr
    if (segfault_flag == 1)
    {
        *force_segfault = 's';
    }
    
    //Main purpose of program: Copy STDIN to STDOUT
    STDIN_TO_STDOUT(STDIN_FILENO, STDOUT_FILENO);
    
    //Exit success: No errors encountered
    exit(0);
}
