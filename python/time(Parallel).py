import os

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 1"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 2"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 4"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 8"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 12"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 16"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 20"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 24"
	os.system(stringIn)

for x in range(0,100):
	stringIn = "python kernelConvolution(Parallel).py images/test_"+str(x)+".bmp -nosave numThreads 28"
	os.system(stringIn)
