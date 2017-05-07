# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS =
AUX = Makefile README lab1a.c
TARBALL_NAME = lab1a-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab1a lab1a.c

clean:
	@rm -f lab1a *.o *.tar.gz *.txt

dist:
	@tar -czf $(TARBALL_NAME) $(AUX)
