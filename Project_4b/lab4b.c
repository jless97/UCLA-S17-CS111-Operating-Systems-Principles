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
#i#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <mraa/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
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
// Variables to hold voltage and temperature values
uint32_t voltage_value;
float temperature_value;
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
// Sample temperature from the temperature sensor and poll for input from keyboard
void sample_temperature_poll_input_handler(void);
// Poll input from the keyboard
void poll_service_keyboard(void);

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
        shutdownSensors();
    }
    exit(EXIT_SUCCESS);
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
                //log_file = creat(optarg, 0666);
                log_file=open(optarg, O_CREAT|O_NONBLOCK|O_APPEND|O_WRONLY,0666);
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
    shutdownSensors();
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

void
poll_service_keyboard(void) {
    // Clear out buffer before each poll
    memset(buf, 0, BUFFER_SIZE);
    ssize_t nread, nwrite;

    // Process the commands
    int i;
    nread = read(STDIN_FILENO, buf, BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading from STDIN.\n");
        exit(EXIT_FAILURE);
    }
    else if (nread == 0) {
        shutdownSensors();
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nread; i++) {
        switch(buf[i]) {
        case 'O':
        if (buf[i]=='O' && buf[i+1]=='F' && buf[i+2]=='F' && buf[i+3]=='\n') {
            if (log_flag == 1) {
                nwrite = write(log_file, "OFF\n", 4);
            }
            i+=4;
            button_handler();
        }
        case 'S':
        if (buf[i]=='S' && buf[i+1]=='T' && buf[i+2]=='O' && buf[i+3]=='P' && buf[i+4]=='\n') {
            if (report_flag == 0) {
                fprintf(stdout, "Program is already not processing reports.\n");
            }
            if (log_flag == 1) {
                nwrite = write(log_file, "STOP\n", 5);
            }
            i+=4;
            report_flag = 0;
        }
        else if (buf[i]=='S' && buf[i+1]=='T' && buf[i+2]=='A' && buf[i+3]=='R' && buf[i+4]=='T' && buf[i+5]=='\n') {
            if (report_flag == 1) {
                fprintf(stdout, "Program is already processing reports.\n");
            }
            if (log_flag == 1) {
                nwrite = write(log_file, "START\n", 6);
            }
            i+=5;
            report_flag = 1;
        }
        else if (buf[i]=='S' && buf[i+1]=='C' && buf[i+2]=='A' && buf[i+3]=='L' && buf[i+4]=='E' && buf[i+5]=='=' && buf[i+6]=='F' && buf[i+7]=='\n') {
            temperature_scale = 'F';
            if (log_flag == 1) {
                nwrite = write(log_file, "SCALE=F\n", 8);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing SCALE=F to log file.\n");
                    exit(EXIT_FAILURE);
                }
            }
            i+=6;
        }
        else if (buf[i]=='S' && buf[i+1]=='C' && buf[i+2]=='A' && buf[i+3]=='L' && buf[i+4]=='E' && buf[i+5]=='=' && buf[i+6]=='C' && buf[i+7]=='\n') {
            temperature_scale = 'C';
            if (log_flag == 1) {
                nwrite = write(log_file, "SCALE=C\n", 8);
                if (nwrite < 0) {
                    fprintf(stderr, "Error writing SCALE=C to log file.\n");
                    exit(EXIT_FAILURE);
                }
            }
            i+=6;
        }
        case 'P':
        if (buf[i]=='P' && buf[i+1]=='E' && buf[i+2]=='R' && buf[i+3]=='I' && buf[i+4]=='O' && buf[i+5]=='D' && buf[i+6]=='=' && isdigit(buf[i+7])) {
            int single_digit_flag = 1;
            int pow = 10;
            int new_period_value = buf[i+7] - 48;
            
            // If the new period value is a single digit
            if (buf[i+8] == '\n') {
                i+=7;
            }
            else {
                int count = 0;
                int j = 1;
                // If new period value is multiple digits long
                while (isdigit(buf[i+7+j])) {
                    new_period_value = new_period_value * pow + buf[7 + j] - 48;
                    j++;
                    count++;
                }
                i+=count;
            }
            if (log_flag == 1) {
                dprintf(log_file, "PERIOD=%d\n", new_period_value);
            }
            period_value = new_period_value;
        }
        }
    }
    
    // Reset buffer before next poll
    memset(buf, 0, BUFFER_SIZE);
}

void
sample_temperature_poll_input_handler(void) {
    // Input received from the keyboard
    polled_fds[0].fd = STDIN_FILENO;
    polled_fds[0].events = POLLIN | POLLHUP | POLLERR;
    
    int nwrite, clock_status, poll_status, num_fds = 1, timeout = 0;
    while(1) {
        // Poll for input from the keyboard
        poll_status = poll(polled_fds, num_fds, timeout);
        if (poll_status == -1) {
            fprintf(stderr, "Error with poll I/O.\n");
            exit(EXIT_FAILURE);
        }
        else {
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
                if (report_flag == 1) {
                    createReport(temperature_value);
                }
            }
            //Check for events on keyboard
            if (polled_fds[0].revents & POLLIN) {
                poll_service_keyboard();
            }
            // Check for errors on keyboard
            if (polled_fds[0].revents & (POLLHUP | POLLERR)) {
                exit(EXIT_SUCCESS);
            }
            // Sampling the temperature readings from temperature sensor at given intervals
            else {
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
                    if (report_flag == 1) {
                        createReport(temperature_value);
                    }
                }
            }
        }
    }
}

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
    
    // Initialize previous time struct variable
    previous_time.tv_sec = -1;
    
    memset(report_entry, 0, ENTRY_BUFFER_SIZE);
    
    // Sample temperature from temperature sensor
    // Poll input from keyboard for commands
    sample_temperature_poll_input_handler();
    
    // Shutdown the button and temperature sensors
    shutdownSensors();
    
    // If no errors encountered, success
    exit(EXIT_SUCCESS);
}
