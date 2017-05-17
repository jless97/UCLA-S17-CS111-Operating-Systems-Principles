#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. avg lock-wait time (ns)
#
# output:
#	lab2b_1.png ... throughput vs. # threads for mutex/spin-lock sync list ops
#	lab2b_2.png ... mean time per mutex wait and mean time per op for mutex list ops
#	lab2b_3.png ... successful iterations vs. threads for each sync method (m/s)
#	lab2b_4.png ... throughput vs. # threads for mutex sync partitioned lists
#	lab2b_5.png ... throughput vs. # threads for spin-lock sync partitioned lists
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#

# general plot parameters
set terminal png
set datafile separator ","

################################################################################
set title "Scalability-1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:16]
set ylabel "Throughput (Operations/sec)"
set logscale y 10
set yrange [10000:1e6]
set output 'lab2b_1.png'
set key top

# grep out non-yield results
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'list ins/lookup/delete w/mutex' with linespoints lc rgb 'orange', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'list ins/lookup/delete w/spin' with linespoints lc rgb 'purple'

################################################################################
set title "Scalability-2: Per-operation Times for Mutex-Protected List Operations"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:32]
set ylabel "mean time/opertion (ns)"
set logscale y 10
set yrange [10:1e7]
set output 'lab2b_2.png'
set key left top
# grep out non-yield, mutex-sync results
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
     	title 'wait for lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
     	title 'completion time' with linespoints lc rgb 'orange'

################################################################################
set title "Scalability-3: Correct Synchronization of Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:16]
set ylabel "Successful Iterations"
set logscale y 10
set yrange [1:100]
set output 'lab2b_3.png'
set key left top

# grep out yield, yield and mutex-sync, and yield and spin-lock-sync results
plot \
     "< grep 'list-id-none' lab2b_list.csv" using ($2):($3) \
     	title 'unprotected' with points lc rgb 'red', \
     "< grep 'list-id-m' lab2b_list.csv"  using ($2):($3) \
     	title 'Mutex' with points lc rgb 'green', \
     "< grep 'list-id-s' lab2b_list.csv" using ($2):($3) \
     	title 'Spin-Lock' with points lc rgb 'blue',

################################################################################
set title "Scalability-4: Throughut of Mutex-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.5:16]
set logscale x 2
set ylabel "Throughput (operations/sec)"
set logscale y 10
set yrange [10000:1e7]
set output 'lab2b_4.png'

# grep out non-yield mutex-sync results
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=1' with linespoints lc rgb 'purple', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=4' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=16' with linespoints lc rgb 'orange'

################################################################################
set title "Scalability-5: Throughut of Spin-Lock-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.5:16]
set logscale x 2
set ylabel "Throughput (operations/sec)"
set logscale y 10
set yrange [10000:1e7]
set output 'lab2b_5.png'

# grep out non-yield mutex-sync results
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=1' with linespoints lc rgb 'purple', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=4' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1e9/($7)) \
     	title 'lists=16' with linespoints lc rgb 'orange'
