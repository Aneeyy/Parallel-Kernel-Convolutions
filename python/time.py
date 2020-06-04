import os

for x in range(0,100):
	stringIn = "python kernelConvolution(Not_Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 3"
	os.system(stringIn)
