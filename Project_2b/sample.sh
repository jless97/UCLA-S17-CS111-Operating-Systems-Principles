# Tests for lab2b_1.png and lab2b_2.png
# Threads: 1, 2, 4, 8, 12, 16, 24
# Iterations: 1000
# Synchronization: m and s
test1and2_threads=(1 2 4 8 12 16 24)

for i in ${test1and2_threads[@]}; do
    ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
    ./lab2_list --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
done

# Tests for lab2b_3.png
# Threads: 1, 4, 8, 12, 16
# Iterations: 1, 2, 4, 8, 16
# Yield: i and d
# Synchronizatoin: none and m and s
# Lists: 4
test3_threads=(1 4 8 12 16)
test3_iterations1=(1 2 4 8 16)
test3_iterations2=(10 20 40 80)

for i in ${test3_threads[@]}; do
    for j in ${test3_iterations1[@]}; do
        ./lab2_list --threads=$i --iterations=$j --yield=id --lists=4 >> lab2b_list.csv
    done
    for j in ${test3_iterations2[@]}; do
        ./lab2_list --threads=$i --iterations=$j --yield=id --lists=4 --sync=m >> lab2b_list.csv
        ./lab2_list --threads=$i --iterations=$j --yield=id --lists=4 --sync=s >> lab2b_list.csv
    done
done

# Tests for lab2b_4.png and lab2b_5.png
# Threads: 1, 2, 4, 8, 12
# Iterations: 1000
# Synchronization: m and s
# Lists: 1, 4, 8, 16
test4and5_threads=(1 2 4 8 12)
test4and5_lists=(4 8 16)

for i in ${test4and5_threads[@]}; do
    for j in ${test4and5_lists[@]}; do
        ./lab2_list --threads=$i --iterations=1000 --lists=$j --sync=m >> lab2b_list.csv
	./lab2_list --threads=$i --iterations=1000 --lists=$j --sync=s >> lab2b_list.csv
    done
done
