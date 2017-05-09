.SILENT:

# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS = -pthread
AUX = Makefile README lab2_add.c lab2_list.c SortedList.h SortedList.c *.png *.gp *.csv sample.sh
TARBALL_NAME = lab2a-404640158.tar.gz

build: lab2_add lab2_list

lab2_add:
	@$(CC) $(CFLAGS) -o lab2_add lab2_add.c

lab2_list:
	@$(CC) $(CFLAGS) -o lab2_list SortedList.c lab2_list.c

tests: build
	rm -rf *.csv
	chmod +x sample.sh
	./sample.sh 2>/dev/null

graphs: build tests
	rm -rf *.png
	chmod +x lab2_add.gp
	chmod +x lab2_list.gp
	gnuplot ./lab2_add.gp 2>/dev/null
	gnuplot ./lab2_list.gp 2>/dev/null

clean:
	@rm -f lab2_add lab2_list *.png *.csv *.tar.gz

dist: build tests graphs
	@tar -czf $(TARBALL_NAME) $(AUX)
