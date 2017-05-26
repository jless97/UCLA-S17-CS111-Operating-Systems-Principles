.SILENT:

# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

# NAME: Yun Xu
# EMAIL: 
# ID:

CC = gcc
CFLAGS = -g
AUX = Makefile README lab3a.c 
TARBALL_NAME = lab3a-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab3a lab3a.c

clean:
	@rm -f lab3a *.tar.gz *.txt

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)