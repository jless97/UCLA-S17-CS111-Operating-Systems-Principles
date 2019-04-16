////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
// Project 2b: lab2_list.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <getopt.h>

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define SEC_TO_NSEC 1000000000
#define EXIT_OTHER_FAILURE 2

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/* Programmable Variables */
// Number of threads, number of iterations, number of lists (values are 1 by default)
int nthreads = 1, niterations = 1, nlists = 1;
// Size of the list, and number of list elements
int list_size = 0, nelements = 0;
// Lock-acquisition time counter
long long lock_acquisition_time = 0;
// Char to store the various testnames
char testname[15];
// Synchronization option char variable
char synchronization;
// Yield option variable
int opt_yield = 0;

/* Flags */
// Yield and Synchronizaiton flags
int yield_flag = 0, sync_flag = 0;
// Yield option flags
int insert_flag = 0, delete_flag = 0, lookup_flag = 0;

/* Structs */
// Variables used to get total/avg run time
struct timespec start_time, end_time;

/* Arrarys */
// Thread array
pthread_t *threads;
// Thread ID array
int *thread_args;
// Array to match key to sublist based off of hash function
int *this_list;
// Array of locks (mutex and spin-locks)
pthread_mutex_t *locks;
int *test_and_set_locks;
// Sorted linked list, and set of list elements
SortedList_t *sublists;
SortedListElement_t *elements;

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Print usage upon unrecognized arguments
void print_usage(void);
// Signal handler
void handler(int signum);
// Parse command-line options
void parser(int argc, char *argv[]);
// Get testname
void getTestName(void);
// Initialize list and list elements
void initListAndElements(void);
// Placing keys to specific sublists based off of hash function
void initKeysToSublist(void);
// Initialize locks for the sublists
void initLocksToSublist(void);
// Function to insert, delete, lookup elements within a doubly-linked list
void* listOperations(void *threadID);
// Free dynamic memory
void freeMemory(void);
// Print CSV Record
void printCSVRecord(void);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    printf("Usage: lab2-list [--thread=numthreads] [--iterations=numiterations] [--yield=i/d/l] [--sync=m/s] [--lists=nlists] \n");
}

void
handler(int signum) {
    if (signum == SIGSEGV) {
        fprintf(stderr, "Error: Segmentation fault.\n");
    }
    exit(EXIT_OTHER_FAILURE);
}

void
parser(int argc, char * argv[]) {
    static struct option long_options[] =
    {
        {"threads",      required_argument,    0, 't'},
        {"iterations",   required_argument,    0, 'i'},
        {"yield",        required_argument,    0, 'y'},
        {"sync",         required_argument,    0, 's'},
        {"lists",        required_argument,    0, 'l'}
    };
    
    int option, i;
    while ( (option = getopt_long(argc, argv, "t:i:s:y:l:", long_options, NULL)) != -1) {
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
            for (i = 0; optarg[i] != '\0'; i++) {
                if (optarg[i] == 'i') {
                    insert_flag = 1;
                    opt_yield |= INSERT_YIELD;
                }
                else if (optarg[i] == 'd') {
                    delete_flag = 1;
                    opt_yield |= DELETE_YIELD;
                }
                else if (optarg[i] == 'l') {
                    lookup_flag = 1;
                    opt_yield |= LOOKUP_YIELD;
                }
                else {
                    fprintf(stderr, "Error: incorrect yield option.\n");
                    print_usage();
                    exit(EXIT_FAILURE);
                }
            }
            yield_flag = 1;
            break;
            // Sync Option
            case 's':
            if (strlen(optarg) == 1) {
                if (optarg[0] == 'm')
                synchronization = 'm';
                else if (optarg[0] == 's')
                synchronization = 's';
                else {
                    fprintf(stderr, "Error: incorrect sync option.\n");
                    print_usage();
                    exit(EXIT_FAILURE);
                }
                sync_flag = 1;
            }
            else {
                fprintf(stderr, "Error: sync option should be a single char.\n");
                print_usage();
                exit(EXIT_FAILURE);
            }
            break;
            // List Option
            case 'l':
            nlists = atoi(optarg);
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
    // Easiest implementation I could think of
    strcat(testname, "list-");
    if (yield_flag == 1) {
        if (insert_flag == 1 && delete_flag == 0 && lookup_flag == 0)
        strcat(testname, "i-");
        else if (insert_flag == 0 && delete_flag == 1 && lookup_flag == 0)
        strcat(testname, "d-");
        else if (insert_flag == 0 && delete_flag == 0 && lookup_flag == 1)
        strcat(testname, "l-");
        else if (insert_flag == 1 && delete_flag == 1 && lookup_flag == 0)
        strcat(testname, "id-");
        else if (insert_flag == 1 && delete_flag == 0 && lookup_flag == 1)
        strcat(testname, "il-");
        else if (insert_flag == 0 && delete_flag == 1 && lookup_flag == 1)
        strcat(testname, "dl-");
        else
        strcat(testname, "idl-");
    }
    else
    strcat(testname, "none-");
    if (sync_flag == 1) {
        if (synchronization == 'm')
        strcat(testname, "m");
        else if (synchronization == 's')
        strcat(testname, "s");
    }
    else
    strcat(testname, "none");
}

void
initListAndElements(void) {
    int i, j;
    
    // Initialize the empty sublists
    sublists = (SortedList_t *) malloc(sizeof(SortedList_t) * nlists);
    for (i = 0; i < nlists; i++) {
        sublists[i].next = &sublists[i];
        sublists[i].prev = &sublists[i];
        sublists[i].key = NULL;
    }
    // Initialize list elements with random keys
    nelements = nthreads * niterations;
    elements = (SortedListElement_t *) malloc(nelements * sizeof(SortedListElement_t));
    // Variables used for generating random key values
    int key_size;
    char key_element_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    srand((unsigned int) time(NULL));
    for (i = 0; i < nelements; i++) {
        key_size = rand() % 10 + 2;
        char *key = (char *) malloc((key_size + 1) * sizeof(char));
        for (j = 0; j < key_size; j++) {
            key[j] = key_element_set[rand() % strlen(key_element_set)];
        }
        key[key_size] = '\0';
        elements[i].key = key;
    }
}

void
initKeysToSublist(void) {
    int i, hashValue;
    // Array to place the key to a specific sublist based off of simple hash function
    this_list = (int *) malloc(sizeof(int) * nelements);
    for (i = 0; i < nelements; i++) {
        hashValue = i % nlists;
        this_list[i] = hashValue;
    }
}

void
initLocksToSublist(void) {
    int i;
    // Initialize mutex locks
    if (synchronization == 'm') {
        locks = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * nlists);
        for (i = 0; i < nlists; i++) {
            pthread_mutex_init(&locks[i], NULL);
        }
    }
    // Initialize spin locks
    else if (synchronization == 's') {
        test_and_set_locks = (int *) malloc(sizeof(int) * nlists);
        for (i = 0; i < nlists; i++) {
            test_and_set_locks[i] = 0;
        }
    }
}

void*
listOperations(void *threadID) {
    SortedListElement_t *element;
    int i, clock_status, delete_status;
    struct timespec before_mutex, after_mutex, before_lock, after_lock;
    long temp_time = 0;
    
    // Insert elements into the list
    for (i = *(int *)threadID; i < nelements; i += nthreads) {
        if (sync_flag == 1) {
            // Mutex
            if (synchronization == 'm') {
                
                // Note the high resolution time before acquiring mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &before_mutex);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with initial clock time.\n");
                    exit(EXIT_FAILURE);
                }
                pthread_mutex_lock(&locks[this_list[i]]);
                // Note the high resolution time after releasing mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &after_mutex);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with final clock time.\n");
                    exit(EXIT_FAILURE);
                }
                // Lock-acquisition (mutex-insert) time
                temp_time = ((after_mutex.tv_sec - before_mutex.tv_sec) * SEC_TO_NSEC) + (after_mutex.tv_nsec - before_mutex.tv_nsec);
                lock_acquisition_time += temp_time;
                
                SortedList_insert(&sublists[this_list[i]], &elements[i]);
                pthread_mutex_unlock(&locks[this_list[i]]);
            }
            // Spin Lock
            if (synchronization == 's') {
                // Note the high resolution time before acquiring mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &before_lock);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with initial clock time.\n");
                    exit(EXIT_FAILURE);
                }
                while (__sync_lock_test_and_set(&test_and_set_locks[this_list[i]], 1));
                // Note the high resolution time after releasing mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &after_lock);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with final clock time.\n");
                    exit(EXIT_FAILURE);
                }
                // Lock-acquisition (mutex-insert) time
                temp_time = ((after_lock.tv_sec - before_lock.tv_sec) * SEC_TO_NSEC) + (after_lock.tv_nsec - before_lock.tv_nsec);
                lock_acquisition_time += temp_time;
                
                
                SortedList_insert(&sublists[this_list[i]], &elements[i]);
                __sync_lock_release(&test_and_set_locks[this_list[i]]);
            }
        }
        else {
            SortedList_insert(&sublists[this_list[i]], &elements[i]);
        }
    }
    
    // Gets length of lists
    if (sync_flag == 1) {
        if (synchronization == 'm') {
            for (i = 0; i < nlists; i++) {
                // Note the high resolution time before acquiring mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &before_mutex);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with initial clock time.\n");
                    exit(EXIT_FAILURE);
                }
                pthread_mutex_lock(&locks[i]);
                // Note the high resolution time after releasing mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &after_mutex);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with final clock time.\n");
                    exit(EXIT_FAILURE);
                }
                // Lock-acquisition (mutex-insert) time
                temp_time = ((after_mutex.tv_sec - before_mutex.tv_sec) * SEC_TO_NSEC) + (after_mutex.tv_nsec - before_mutex.tv_nsec);
                lock_acquisition_time += temp_time;
                
                list_size += SortedList_length(&sublists[i]);
                pthread_mutex_unlock(&locks[i]);
            }
        }
        else if (synchronization == 's') {
            for (i = 0; i < nlists; i++) {
                // Note the high resolution time before acquiring mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &before_lock);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with initial clock time.\n");
                    exit(EXIT_FAILURE);
                }
                while (__sync_lock_test_and_set(&test_and_set_locks[i], 1));
                // Note the high resolution time after releasing mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &after_lock);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with final clock time.\n");
                    exit(EXIT_FAILURE);
                }
                // Lock-acquisition (mutex-insert) time
                temp_time = ((after_lock.tv_sec - before_lock.tv_sec) * SEC_TO_NSEC) + (after_lock.tv_nsec - before_lock.tv_nsec);
                lock_acquisition_time += temp_time;
                
                list_size += SortedList_length(&sublists[i]);
                __sync_lock_release(&test_and_set_locks[i]);
            }
        }
        else {
            // Set the length of the list
            for (i = 0; i < nlists; i++) {
                list_size += SortedList_length(&sublists[i]);
            }
        }
    }
    
    // Lookup and delete elements from the list
    for (i = *(int *)threadID; i < nelements; i += nthreads) {
        if (sync_flag == 1) {
            // Mutex
            if (synchronization == 'm') {
                // Note the high resolution time before acquiring mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &before_mutex);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with initial clock time.\n");
                    exit(EXIT_FAILURE);
                }
                pthread_mutex_lock(&locks[this_list[i]]);
                // Note the high resolution time after releasing mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &after_mutex);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with final clock time.\n");
                    exit(EXIT_FAILURE);
                }
                // Lock-acquisition (mutex-insert) time
                temp_time = ((after_mutex.tv_sec - before_mutex.tv_sec) * SEC_TO_NSEC) + (after_mutex.tv_nsec - before_mutex.tv_nsec);
                lock_acquisition_time += temp_time;
                
                element = SortedList_lookup(&sublists[this_list[i]], elements[i].key);
                if (element == NULL) {
                    fprintf(stderr, "Error: can't find key that was inserted.\n");
                    exit(EXIT_OTHER_FAILURE);
                }
                delete_status = SortedList_delete(element);
                if (delete_status == 1) {
                    fprintf(stderr, "Error: delete failed.\n");
                    exit(EXIT_OTHER_FAILURE);
                }
                pthread_mutex_unlock(&locks[this_list[i]]);
            }
            // Spin Lock
            if (synchronization == 's') {
                // Note the high resolution time before acquiring mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &before_lock);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with initial clock time.\n");
                    exit(EXIT_FAILURE);
                }
                while (__sync_lock_test_and_set(&test_and_set_locks[this_list[i]], 1));
                // Note the high resolution time after releasing mutex lock
                clock_status = clock_gettime(CLOCK_MONOTONIC, &after_lock);
                if (clock_status == -1) {
                    fprintf(stderr, "Error with final clock time.\n");
                    exit(EXIT_FAILURE);
                }
                // Lock-acquisition (mutex-insert) time
                temp_time = ((after_lock.tv_sec - before_lock.tv_sec) * SEC_TO_NSEC) + (after_lock.tv_nsec - before_lock.tv_nsec);
                lock_acquisition_time += temp_time;
                
                
                element = SortedList_lookup(&sublists[this_list[i]], elements[i].key);
                if (element == NULL) {
                    fprintf(stderr, "Error: can't find key that was inserted.\n");
                    exit(EXIT_OTHER_FAILURE);
                }
                delete_status = SortedList_delete(element);
                if (delete_status == 1) {
                    fprintf(stderr, "Error: delete failed.\n");
                    exit(EXIT_OTHER_FAILURE);
                }
                __sync_lock_release(&test_and_set_locks[this_list[i]]);
            }
        }
        else {
            element = SortedList_lookup(&sublists[this_list[i]], elements[i].key);
            if (element == NULL) {
                fprintf(stderr, "Error: can't find key that was inserted.\n");
                exit(EXIT_OTHER_FAILURE);
            }
            delete_status = SortedList_delete(element);
            if (delete_status == 1) {
                fprintf(stderr, "Error: delete failed.\n");
                exit(EXIT_OTHER_FAILURE);
            }
        }
    }
    return NULL;
}

void
freeMemory(void) {
    free(sublists);
    free(elements);
    free(thread_args);
    free(threads);
    free(this_list);
    free(locks);
    free(test_and_set_locks);
}

void
printCSVRecord(void) {
    // CSV processing information
    // Name of test
    getTestName();
    // Number of operations
    long long noperations;
    if (yield_flag == 0)
    noperations = nthreads * niterations;
    else
    noperations = nthreads * niterations * 3;
    // Total run time (ns)
    long long total_runtime = ((end_time.tv_sec - start_time.tv_sec) * SEC_TO_NSEC) + \
    (end_time.tv_nsec - start_time.tv_nsec);
    // Avg. time per operation (ns)
    long long avg_runtime = total_runtime / noperations;
    // Total lock-acquisition time
    long long total_locktime = lock_acquisition_time / noperations;
    
    // Print to STDOUT CSV record
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", testname, nthreads, niterations, \
            nlists, noperations, total_runtime, avg_runtime, total_locktime);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[])
{
    // Register SIGSEGV to handler
    signal(SIGSEGV, handler);
    
    // Parse command-line options
    parser(argc, argv);
    
    // Clock and thread status variables
    int i, clock_status, thread_status;
    
    // Initialize list and list elements
    initListAndElements();
    
    // Initialize keys to sublists
    initKeysToSublist();
    
    // Initialize locks to sublists
    if (sync_flag == 1)
    initLocksToSublist();
    
    // Create array of threads
    threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t));
    thread_args = (int *) malloc(nthreads * sizeof(int));
    
    // Note the high resolution start time for the run
    clock_status = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (clock_status == -1) {
        fprintf(stderr, "Error with initial clock time.\n");
        exit(EXIT_FAILURE);
    }
    
    // Create required number of threads, and wait for all threads to complete
    for (i = 0; i < nthreads; i++) {
        thread_args[i] = i;
        thread_status = pthread_create(&threads[i], NULL, listOperations, &thread_args[i]);
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
    
    // Get list size (enumerating through all sublists)
    list_size = 0;
    for (i = 0; i < nlists; i++) {
        list_size += SortedList_length(&sublists[i]);
    }
    
    // Free dynamic arrays
    freeMemory();
    
    // Final list size (should be 0) (if not 0, then the list was corrupted)
    if (list_size != 0) {
        fprintf(stderr, "Error: Final list size not correct. Corrupted list.\n");
        exit(EXIT_OTHER_FAILURE);
    }
    // Final list size is 0, so print CSV record
    else {
        printCSVRecord();
    }
    
    // If list size is 0, success
    exit(EXIT_SUCCESS);
}
