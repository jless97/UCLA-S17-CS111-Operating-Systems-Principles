.SILENT:

# NAME: Jason Less
# EMAIL: jaless1997@gmail.com
# ID: 404-640-158

CC = gcc
CFLAGS = -pthread -g
AUX = Makefile README lab2_list.c SortedList.* *.png *.gp *.csv sample.sh profile.out
TARBALL_NAME = lab2b-404640158.tar.gz

build: lab2_list

lab2_list:
	@$(CC) $(CFLAGS) -o lab2_list SortedList.c lab2_list.c

tests: build
	rm -rf *.csv
	chmod +x sample.sh
	./sample.sh 2>/dev/null

graphs: tests
	rm -rf *.png
	chmod +x lab2_list.gp
	gnuplot ./lab2_list.gp >/dev/null

profile: build Makefile
	rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/grad/patnaiku/lib/libprofiler.so.0 CPUPROFILE=raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s >/dev/null 2>&1
	pprof --text ./lab2_list ./raw.gperf > profile.out 2>/dev/null
	pprof --list=listOperations ./lab2_list ./raw.gperf >> profile.out 2>/dev/null
	rm -f ./raw.gperf

clean:
	@rm -f lab2_list *.png *.csv *.tar.gz *.out

dist: graphs profile
	@tar -czf $(TARBALL_NAME) $(AUX)
