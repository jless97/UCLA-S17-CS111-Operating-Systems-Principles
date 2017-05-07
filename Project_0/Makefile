# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

.SILENT:

CC = gcc
CFLAGS = -g
AUX = Makefile README lab0.c backtrace.png breakpoint.png
TARBALL_NAME = lab0-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab0 lab0.c

check: default tests
	if [ -s failure_log.txt ] ; then \
	echo "Failure: One or more test cases failed" ; \
	cat failure_log.txt ; \
	else \
	echo "Success: All test cases succeeded" ; \
	cat success_log.txt ; \
	fi ;
	rm -f failure_log.txt success_log.txt
clean:
	@rm -f lab0 *.o *.txt *.tar.gz

dist:
	@tar -czf $(TARBALL_NAME) $(AUX)

# Smoke-test

tests: invalid_arg_test input_to_output_test input_option_test output_option_test input_and_output_option_test catch_segfault_test

invalid_arg_test:
	./lab0 --bogus  &>/dev/null ; \
	if [[ $$? -eq 1 ]] ; then \
	echo "Success: ./lab0 returns correct value when given invalid arguments" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value when given invalid arguments" >> failure_log.txt ; \
	fi
input_to_output_test:
	echo "Testing" > input_file.txt
	./lab0 < input_file.txt > output_file.txt
	cmp input_file.txt output_file.txt &>/dev/null
	if [[ $$? -eq 0 ]] ; then \
	echo "Success: ./lab0 returns correct value in the default case (when no options provided)" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value in the default case (when no options provided)" >> failure_log.txt ; \
	fi ;
	rm -f input_file.txt output_file.txt
input_option_test:
	echo "Testing" > input_file.txt &>/dev/null ; \
	./lab0 --input=input_file.txt ; \
	if [[ $$? -eq 0 ]] ; then \
	echo "Success: ./lab0 returns correct value when input option provided" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value when input option provided" >> failure_log.txt ; \
	fi ;
	rm -f input_file.txt
	./lab0 --input=nonexistent_file.txt &>/dev/null ; \
	if [[ $$? -eq 2 ]] ; then \
	echo "Success: ./lab0 returns correct value when it can't open input file" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value when it can't open input file" >> failure_log.txt ; \
	fi ;
	rm -f nonexistent_file.txt
output_option_test:
	touch cant_write_output_file.txt
	chmod -w cant_write_output_file.txt
	./lab0 --output=cant_write_output_file.txt &>/dev/null ; \
	if [[ $$? -eq 3 ]] ; then \
	echo "Success: ./lab0 returns correct value when it can't create output file" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value when it can't create output file" >> failure_log.txt ; \
	fi ;
	rm -f cant_write_output_file.txt
input_and_output_option_test:
	echo "Testing" > input_file.txt &>/dev/null ; \
	./lab0 --input=input_file.txt --output=output_file.txt ; \
	cmp input_file.txt output_file.txt &>/dev/null
	if [[ $$? -eq 0 ]] ; then \
	echo "Success: ./lab0 returns correct value when both input and output options provided" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value when both input and output options provided" >> failure_log.txt ; \
	fi ;
	rm -f input_file.txt output_file.txt
catch_segfault_test:
	./lab0 --catch --segfault &>/dev/null ; \
	if [[ $$? -eq 4 ]] ; then \
	echo "Success: ./lab0 returns correct value when catching segfault" >> success_log.txt ; \
	else \
	echo "Error: ./lab0 returns incorrect value when catching segfault" >> failure_log.txt ; \
	fi
