# NAME: Jason Less
# Email: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CDEBUG = -g
CFLAGS = $(CDEBUG)

AUX = Makefile README lab0.c backtrace.png breakpoint.png
MAIN = lab0
SRCS_C = lab0.c
STUDENT_ID = 404640158
TARBALL_NAME = $(MAIN)-$(STUDENT_ID)

default:
    @$(CC) $(CFLAGS) -o $(Main) $(SRCS_C)

check: default tests

clean:
    @rm -f $(MAIN) *.o *.txt *.tar.gz

dist:
    @tar -czf $(TARBALL_NAME) $(AUX)

# Smoke-test

tests: invalid_arg_test input_to_output_test input_option_test output_option_test input_and_output_option_test catch_segfault_test

invalid_arg_test:
    echo "Test to see if ./$(MAIN) detects and reports invalid arguments" ;
    ./($MAIN) --fake_arg &>/dev/null
    if [ $$? -ne 1 ]
    then
        echo "Error: ./$(MAIN) doesn't handle invalid arguments correctly."
    fi
input_to_output_test:
    echo "Test to see if ./$(MAIN) handles the primary function correctly (i.e. STDIN to STDOUT)"
    echo "Testing" > input_file.txt
    ./$(MAIN) < input_file.txt > output_file.txt
    cmp input_file.txt output_file.txt &>/dev/null
    if [ $$? -ne 0 ]
    then
        echo "Error: ./$(MAIN) doesn't handle copy of input to output correctly: Files contain different content"
    fi
    rm -f input_file.txt output_file.txt
input_option_test:
    echo "Test to see if ./$(MAIN) correctly handles when invalid input option provided"
    ./$(MAIN) --input=_nonexistent_input_file.txt &>/dev/null
    if [ $$? -ne 2 ]
    then
        echo "Error: ./$(MAIN) doesn't handle when unable to open specified input file correctly"
    fi
    rm -f nonexistent_input_file.txt
output_option_test:
    echo "Test to see if ./$(MAIN) correctly handles when invalid output option provided"
    echo "Testing" > input_file.txt
    touch output_file.txt
    chmod -w output_file.txt
    ./$(MAIN) --output=output_file.txt < input_file.txt &>/dev/null
    if [ $$? -ne 3]
    then
        echo "Error: ./$(MAIN) doesn't handle when unable to create specified output file correctly"
    fi
    rm -f input_file.txt output_file.txt
input_and_output_option_test:
    echo "Testing" > input_file.txt
    ./$(MAIN) --input=input_file.txt --output=output_file.txt &>/dev/null
    if [ $$? -ne 0 ]
    then
        echo "Error: ./$(MAIN) doesn't handle when both input and output options provided correctly"
    fi
    rm -f input_file.txt output_file.txt
catch_segfault_test:
    echo "Test to see if ./$(MAIN) causes and catches segfaults correctly"
    ./$(MAIN) --catch --segfault &>/dev/null
    if [ $$? -ne 4 ]
    then
        echo "Error: ./$(MAIN) doesn't handle catching of segfaults correctly"
    fi
