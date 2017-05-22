.SILENT:

# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS = -lm -lmraa -g
AUX = Makefile README lab4b.c makecheck.sh
TARBALL_NAME = lab4b-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab4b lab4b.c

check: clean default
	@chmod +x makecheck.sh
	@bash ./makecheck.sh lab4b

clean:
	@rm -f lab4b *.tar.gz *.txt

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)