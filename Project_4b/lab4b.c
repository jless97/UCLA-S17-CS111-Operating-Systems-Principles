////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
// Project 4b: lab4b.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <mraa/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>   /* time_t, struct tm, time, localtime, current_time, previous_time */

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define BUFFER_SIZE 256
#define ENTRY_BUFFER_SIZE 16

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/* Programmable Variables */
// The two sensors used to capture temperature and process button action
mraa_gpio_context button;
mraa_aio_context temperature;
// Temperature sensor conversion variables
const int B = 4275;     // B value of the thermistor
const int R0 = 100000;   // R0 = 100k
// Buffer to read/write from STDIN
char buf[BUFFER_SIZE];
// Buffer for report entry
char report_entry[ENTRY_BUFFER_SIZE];
// Variable to store the current temperature scale value (i.e. Fahrenheit/Celsius)
char temperature_scale;
// Sampling rate (period) (default to 1 sample/sec)
int period_value = 1;
// File descriptor of the log file (if log option provided)
int log_file;

/* Flags */
// Continue execution of program until SIGINT encountered
sig_atomic_t volatile run_flag = 1;
// If log option
int log_flag = 0;
int report_flag = 1;
/* Structs */
// Poll I/O struct
struct pollfd polled_fds[1];
// Time structs to setup sampling interval of temperature sensor
struct timespec current_time, previous_time;

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Print usage upon unrecognized arguments
void print_usage(void);
// Signal handler
void handler(int signum);
// Parse command-line options
void parser(int argc, char *argv[]);
// Initialize sensors
void initSensors(void);
// Shutdown sensors
void shutdownSensors(void);
// Process pressing of the button sensor
void button_handler(void);
// Get current local time
char* getTime(char *entry);
// Get temperature (F/C)
float getTemperature(uint32_t temperature, char temperature_scale);
// Creates report entries
void createReport(float temperature);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    printf("Usage: lab4b [--period=#] [--scale=F/C] [--log=filename]\n");
}

void
handler(int signum) {
    if (signum == SIGINT) {
        run_flag = 0;
    }
    exit(EXIT_FAILURE);
}

void
parser(int argc, char * argv[]) {
    static struct option long_options[] =
      {
        {"period",      optional_argument,    0, 'p'},
        {"scale",       required_argument,    0, 's'},
        {"log",         required_argument,    0, 'l'},
      };
    
    int option;
    while ( (option = getopt_long(argc, argv, "ps:l:", long_options, NULL)) != -1) {
        switch (option) {
                // Period option
            case 'p':
                period_value = atoi(optarg);
                if (period_value < 0) {
                    fprintf(stderr, "Error: period must be positive value.\n");
                    exit(EXIT_FAILURE);
                }
                break;
                // Scale option
        case 's':
            if (strlen(optarg) == 1) {
                if (optarg[0] == 'F')
                    temperature_scale = 'F';
                else if (optarg[0] == 'C')
                    temperature_scale = 'C';
                else {
                    fprintf(stderr, "Error: incorrect scale option.\n");
                    print_usage();
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "Error: scale option should be a single char\n");
                print_usage();
                exit(EXIT_FAILURE);
            }
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
initSensors(void) {
    // Initialize mraa
    mraa_init();
    
    // Set button sensor to digital input 3
    button = mraa_gpio_init(3);
    // Set temperature sensor to analog input 0
    temperature = mraa_aio_init(0);
    
    // Indicating the button is a general-purpose input
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    // Register the button, so that when pressed, it will be properly serviced
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, (void *)&button_handler, NULL);
}

void
shutdownSensors(void) {
    mraa_gpio_close(button);
    mraa_aio_close(temperature);
}

void
button_handler(void) {
    // Logs final sample with the time and SHUTDOWN string (instead of data values)
    memset(report_entry, 0, ENTRY_BUFFER_SIZE);
    strcpy(report_entry, getTime(report_entry));
    strcat(report_entry, " SHUTDOWN\n");
    fprintf(stdout, "%s", report_entry);
    if (log_flag == 1) {
        // Append shutdown string to logfile
        write(log_file, report_entry, strlen(report_entry));
    }
    exit(EXIT_SUCCESS);
}

char*
getTime(char *entry) {
    // Get current time
    time_t rawtime;
    struct tm *time_info;
    
    time(&rawtime);
    time_info = localtime(&rawtime);
    
    snprintf(entry, 9, "%02d:%02d:%02d", time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
    return entry;
}

float
getTemperature(uint32_t temperature, char temperature_scale) {
    float R, celsius, fahrenheit;
    
    R = (1023.0/temperature - 1.0) * R0;
    celsius = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
    
    if (temperature_scale == 'F') {
        fahrenheit = (1.8 * celsius) + 32;
        return fahrenheit;
    }
    else {
        return celsius;
    }
}


void
createReport(float temperature) {
    // Clear out report entry buffer
    memset(report_entry, 0, ENTRY_BUFFER_SIZE);
    
    // Append current local time to report entry
    strcpy(report_entry, getTime(report_entry));
    
    // Writes report entry to STDOUT (and also logfile if specified)
    fprintf(stdout, "%s %.1f\n", report_entry, temperature);
    if (log_flag == 1) {
        // Append entry to logfile
        write(log_file, report_entry, strlen(report_entry));
        dprintf(log_file, " %.1f\n", temperature);
    }
}

// Read from keyboard and write to the STDOUT (or logfile if specified)
// TODO: still just a skeleton
//void
//poll_service_keyboard(void) {
//  memset(buf, 0, BUFFER_SIZE);
//  ssize_t nread, nwrite;
//  int i;
//  nread = read(STDIN_FILENO, buf, BUFFER_SIZE);
//  if (nread < 0) {
//    fprintf(stderr, "Error reading from STDIN.\n");
//    exit(EXIT_FAILURE);
//  }
//  else if (nread == 0) {
//    // Automatically restore terminal modes on exit via atexit(restore...)
//    exit(EXIT_SUCCESS);
//  }
//  // Process the commands
//  if (strcmp(buf, "OFF") == 0) {
//    button_handler();
//  }
//  else if (strcmp(buf, "STOP") == 0) {
//    if (report_flag == 0) {
//      // log receipt of command
//    }
//    report_flag = 0;
//  }
//  else if (strcmp(buf, "START") == 0) {
//    if (report_flag == 1) {
//      // log receipt of command
//    }
//    report_flag = 1;
//  }
//  else if (strcmp(buf, "SCALE=F") == 0) {
//    temperature_scale = 'F';
//  }
//  else if (strcmp(buf, "SCALE=C") == 0) {
//    temperature_scale = 'C';
//  }
//  // TODO: edge case
//  else if (strcmp(buf, "PERIOD= ") == 0) {
//        
//  }
//  memset(buf, 0, BUFFER_SIZE);
//}
//
//void
//poll_io_handler(void) {
//  // Input received from the keyboard
//  polled_fds[0].fd = STDIN_FILENO;
//  polled_fds[0].events = POLLIN | POLLHUP | POLLERR;
//    
//  int nwrite, poll_status, num_fds = 1, timeout = 0;
//  while(1) {
//    poll_status = poll(polled_fds, num_fds, timeout);
//    if (poll_status == -1) {
//      fprintf(stderr, "Error with poll I/O.\n");
//      exit(EXIT_FAILURE);
//    }
//    else {
//      //Check for events on keyboard
//      if (polled_fds[0].revents & POLLIN) {
//	poll_service_keyboard();
//      }
//      // Check for errors on keyboard
//      if (polled_fds[0].revents & (POLLHUP | POLLERR)) {
//	exit(EXIT_SUCCESS);
//      }
//    }
//  }
//}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[])
{
    // Register SIGINT (^C) to handler
    signal(SIGINT, handler);
    
    // Parse command-line options
    parser(argc, argv);
    
    // Initialize the button and temperature sensors
    initSensors();
    
    // Initialize time struct variable
    int clock_status;
    clock_status = clock_gettime(CLOCK_MONOTONIC, &previous_time);
    if (clock_status == -1) {
        fprintf(stderr, "Error with initial previous clock time.\n");
        exit(EXIT_FAILURE);
    }
    
    // Variables to hold voltage and temperature values
    uint32_t voltage_value;
    float temperature_value;
    memset(report_entry, 0, ENTRY_BUFFER_SIZE);
    while (run_flag) {
        // Polling for input commands (OFF, STOP, START, SCALE=F/C, PERIOD=seconds)
        //poll_io_handler();
        
        // Sampling the temperature readings from temperature sensor at given intervals
        clock_status = clock_gettime(CLOCK_MONOTONIC, &current_time);
        if (clock_status == -1) {
            fprintf(stderr, "Error with current clock time.\n");
            exit(EXIT_FAILURE);
        }
        if (current_time.tv_sec >= (previous_time.tv_sec + period_value)) {
            // Reset previous sample time to the current sample time
            previous_time.tv_sec = current_time.tv_sec;
            
            // Read in from the temperature sensor and convert to appropriate scale
            voltage_value = mraa_aio_read(temperature);
            temperature_value = getTemperature(voltage_value, temperature_scale);
        
            // Create the report (to STDOUT and log if specified)
            createReport(temperature_value);
        
        }
    }
    
    // Shutdown the button and temperature sensors
    shutdownSensors();
    
    // If no errors encountered, success
    exit(EXIT_SUCCESS);
}
