.SILENT:

# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS = -lm -lmraa -lssl -lcrypto -g
AUX = Makefile README lab4c_tcp.c lab4c_tls.c 
TARBALL_NAME = lab4c-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab4c_tcp lab4c_tcp.c
	@$(CC) $(CFLAGS) -o lab4c_tls lab4c_tls.c

clean:
	@rm -f lab4c_tcp lab4c_tls *.tar.gz *.txt

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)
