////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
// Project 2a: lab2_add.c

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

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define SEC_TO_NSEC 1000000000

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/* Programmable Variables */
// Number of threads and number of iterations (both values are 1 by default)
int nthreads = 1, niterations = 1;
// Total count of program
long long count = 0;
// Char to store the various testnames
char testname[15];
// Synchronization option char variable
char synchronization;
// Yield option variable
int opt_yield = 0;
// Mutex for synchronization option
pthread_mutex_t lock;
// Spin lock for synchronization option
int test_and_set_lock = 0;

/* Flags */
// Yield and Synchronizaiton flags
int yield_flag = 0, sync_flag = 0;

/* Structs */
// Variables used to calculate total/avg runtime
struct timespec start_time, end_time;

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Print usage upon unrecognized arguments
void print_usage(void);
// Parse command-line options
void parser(int argc, char *argv[]);
// Get testname
void getTestName(void);
// Basic add routine to showcase possible race conditions
void add(long long *pointer, long long value);
// Function to add values with different cases (i.e. locks, synchronization, etc.)
// To shared counter variable
void* addToCounter(void *count);
// Print CSV record
void printCSVRecord(void);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    printf("Usage: lab2-add [--thread=numthreads] [--iterations=numiterations] [--yield] [--sync=m/s/c] \n");
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
    while ( (option = getopt_long(argc, argv, "t:i:s:y", long_options, NULL)) != -1) {
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
                opt_yield = 1;
                yield_flag = 1;
                break;
                // Sync Option
            case 's':
                if (strlen(optarg) == 1) {
                    if (optarg[0] == 'm')
                        synchronization = 'm';
                    else if (optarg[0] == 's')
                        synchronization = 's';
                    else if (optarg[0] == 'c')
                        synchronization = 'c';
                    else {
                        fprintf(stderr, "Error: incorrect sync option.\n");
                        exit(EXIT_FAILURE);
                    }
                    sync_flag = 1;
                }
                else {
                    fprintf(stderr, "Error: sync option should be a single char.\n");
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
getTestName(void) {
    strcat(testname, "add-");
    if (yield_flag == 1)
        strcat(testname, "yield-");
    if (sync_flag == 1) {
        if (synchronization == 'm')
            strcat(testname, "m");
        if (synchronization == 's')
            strcat(testname, "s");
        if (synchronization == 'c')
            strcat(testname, "c");
    }
    else
        strcat(testname, "none");
}

void
add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void*
addToCounter(void *count) {
    int i;
    for (i = 0; i < niterations; i++) {
        if (sync_flag == 1) {
            // Mutex
            if (synchronization == 'm') {
                pthread_mutex_lock(&lock);
                add((long long *)count, 1);
                pthread_mutex_unlock(&lock);
            }
            // Spin Lock
            if (synchronization == 's') {
                while (__sync_lock_test_and_set(&test_and_set_lock, 1));
                add((long long *)count, 1);
                __sync_lock_release(&test_and_set_lock);
            }
            // Compare and Swap
            if (synchronization == 'c') {
                long long oldval, newval;
                do {
                    oldval = *(long long *)count;
                    newval = oldval + 1;
                } while(__sync_val_compare_and_swap((long long *)count, oldval, newval) != oldval);
            }
        }
        else {
            add((long long *)count, 1);
        }
    }
    for (i = 0; i < niterations; i++) {
        if (sync_flag == 1) {
            // Mutex
            if (synchronization == 'm') {
                pthread_mutex_lock(&lock);
                add((long long *)count, -1);
                pthread_mutex_unlock(&lock);
            }
            // Spin Lock
            if (synchronization == 's') {
                while (__sync_lock_test_and_set(&test_and_set_lock, 1));
                add((long long *)count, -1);
                __sync_lock_release(&test_and_set_lock);
            }
            // Compare and Swap
            if (synchronization == 'c') {
                long long oldval, newval;
                do {
                    oldval = *(long long *)count;
                    newval = oldval - 1;
                } while(__sync_val_compare_and_swap((long long *)count, oldval, newval) != oldval);
            }
        }
        else {
            add((long long *)count, -1);
        }
    }
    return NULL;
}

void
printCSVRecord(void) {
    // CSV processing information
    // Name of test
    getTestName();
    // Number of operations
    int noperations = nthreads * niterations * 2;
    // Total run time (ns)
    long long total_runtime = ((end_time.tv_sec - start_time.tv_sec) * SEC_TO_NSEC) + \
    (end_time.tv_nsec - start_time.tv_nsec);
    // Avg. time per operation (ns)
    long long avg_runtime = total_runtime / noperations;
    // Final count
    long long final_count = count;
    
    // Print to STDOUT CSV record
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", testname, nthreads, niterations, \
            noperations, total_runtime, avg_runtime, final_count);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[])
{
    // Parse command-line options
    parser(argc, argv);
    
    // Clock and thread status variables
    int i, clock_status, thread_status;
    
    // Synchronization: If mutex option specified
    if (sync_flag == 1 && synchronization == 'm') {
        pthread_mutex_init(&lock, NULL);
    }
    
    // Create array of threads
    pthread_t *threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t));
    
    // Note the high resolution start time for the run
    clock_status = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (clock_status == -1) {
        fprintf(stderr, "Error with initial clock time.\n");
        exit(EXIT_FAILURE);
    }
    
    // Create required number of threads, and wait for all threads to complete
    for (i = 0; i < nthreads; i++) {
        thread_status = pthread_create(&threads[i], NULL, addToCounter, &count);
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
    
    // Free dynamic arrays
    free(threads);
    
    // Print CSV record
    printCSVRecord();
    
    // If run completes successfully
    exit(EXIT_SUCCESS);
}












