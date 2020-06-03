default: openMP

openMP: kernelConvolution.c cJSON.c
	gcc -Wall -Wextra -g -o $@ kernelConvolution.c cJSON.c -lm

clean:
	-rm -f openMP
