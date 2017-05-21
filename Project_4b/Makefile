.SILENT:

# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS = -lm -lmraa -g
AUX = Makefile README lab4b.c
TARBALL_NAME = lab4b-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab4b lab4b.c

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
	@rm -f lab4b *.tar.gz *.txt

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)

# Smoke-test

tests: invalid_arg_test create_logfile_test

invalid_arg_test:
	./lab4b --bogus &>/dev/null ; \
	if [[ $$? -eq 1 ]]; then \
	echo "Success: ./lab4b returns correct value when given invalid arguments" >> success_log.txt ; \
	else \
	echo "Error: ./lab4b returns incorrect value when given invalid arguments" >> failure_log.txt ; \
	fi

create_logfile_test:
	./lab4b --period=5 --log=log.txt ;&>/dev/null \
	if [[ ! -s log.txt ]]; then \
	echo "Error: ./lab4b doesn't create log file" >> failure_log.txt ; \
	else \
	echo "Success: ./lab4b properly creates log file" >> success_log.txt ; \
	fi