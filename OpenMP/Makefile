default: openMP

openMP: dummy.c cJSON.c
	gcc -O3 -Wall -Wextra -o $@ dummy.c cJSON.c

clean:
	-rm -f matmult
	-rm -f matmult_omp
