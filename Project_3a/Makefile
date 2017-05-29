.SILENT:

# NAME: Jason Less & Yun Xu
# EMAIL: jaless1997@gmail.com & x_one_u@yahoo.com
# ID: 404-640-158 & 304635157

CC = gcc
CFLAGS = -g
AUX = Makefile README lab3a.c ext2_fs.h
TARBALL_NAME = lab3a-404640158.tar.gz

default:
	@$(CC) $(CFLAGS) -o lab3a lab3a.c -lm

clean:
	@rm -f lab3a *.tar.gz *.txt

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)