# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS = -lmcrypt
AUX = Makefile README lab1b-server.c lab1b-client.c my.key
TARBALL_NAME = lab1b-404640158.tar.gz

default: server client

server:
	@$(CC) $(CFLAGS) -o lab1b-server lab1b-server.c

client:
	@$(CC) $(CFLAGS) -o lab1b-client lab1b-client.c

clean:
	@rm -f server client *.o *.tar.gz *.txt

dist:
	@tar -czf $(TARBALL_NAME) $(AUX)
