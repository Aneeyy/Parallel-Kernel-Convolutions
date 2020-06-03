default: openMP

openMP: kernelConvolution.c cJSON.c
	gcc -Wall -Wextra -g -O3 -fopenmp -o $@ kernelConvolution.c cJSON.c -lm

clean:
	-rm -f openMP
