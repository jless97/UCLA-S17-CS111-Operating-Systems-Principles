////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
// Project 1b: lab1b-client.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <getopt.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Number of threads and number of iterations (both values are 1 by default)
int nthreads = 1, niterations = 1;


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Print usage upon unrecognized arguments 
void print_usage(void);
// Parse command-line options
void parser(int argc, char *argv[]);
// Basic add routine to showcase possible race conditions
void add(long long *pointer, long long value);
// Function to add values with different cases (i.e. locks, synchronization, etc.)
// To shared counter variable
void* addToCounter(void *counter);
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    printf("Usage: lab2-add [--thread=numthreads] [--iterations=numiterations] \
           [--yield] [--sync=m/s/c] \n");
}

void
parser(int argc, char * argv[]) {
    static struct option long_options[] =
    {
        {"threads",      required_argument,    0, 't'},
        {"iterations",   required_argument,    0, 'i'},
        {"yield",        no_argument,          0, 'y'},
        {"sync",         required_argument,    0, 's'}
    };
    
    int option;
    while ( (option = getopt_long(argc, argv, "p:e:l", long_options, NULL)) != -1) {
        switch (option) {
            // Threads option
            case 't':
                nthreads = atoi(optarg);
                break;
            // Iterations option
            case 'i':
                niterations = atoi(optarg);
                break;
            // Yield Option
            case 'y':
                break;
            // Sync Option
            case 's':
                break;
            // Unrecognized argument
            default:
                print_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
}

void* addToCounter(void *counter) {
    int i;
    for (i = 0; i < niterations; i++) {
        add((long long *)counter, 1);
    }
    for (i = 0; i < niterations; i++) {
        add((long long *)counter, -1);
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[])
{
    // Parse command-line options
    parser(argc, argv);
    
    long long counter = 0;
    int i, clock_status, thread_status;
    timespec start_time, end_time;
    // Note the high resolution start time for the run
    clock_status = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (clock_status == -1) {
        fprintf(stderr, "Error with initial clock time.\n");
        exit(EXIT_FAILURE);
    }
    
    // Create required number of threads, and wait for all threads to complete
    // Call function to add to counter
    pthread_t *threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t));
    for (i = 0; i < nthreads; i++) {
        thread_status = pthread_create(&threads[i], NULL, addToCounter, &counter);
        if (thread_status) {
            fprintf(stderr, "Error with creating threads.\n");
            exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < nthreads; i++) {
        thread_status = pthread_join(threads[i], NULL);
        if (thread_status) {
            fprintf(stderr, "Error with joining threads.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Note the high resolution ending time for the run
    clock_status = clock_gettime(CLOCK_MONOTONIC, &end_time);
    if (clock_status == -1) {
        fprintf(stderr, "Error with final clock time.\n");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}












