default: openMP

openMP: kernelConvolution.c cJSON.c
	gcc -O3 -Wall -Wextra -o $@ kernelConvolution.c cJSON.c

clean:
	-rm -f matmult
	-rm -f matmult_omp
