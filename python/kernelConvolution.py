import json
import os
import sys
import timeit

from shutil import copyfile
from scipy import misc
import numpy as np
from PIL import Image, ImageDraw
def convoluteFileNoParallel(inputFile, fileOutputLocation,kernel,numThreads, makeGreyScale, shouldSave):
    input_image = Image.open(inputFile)
    input_pixels = input_image.load()

    output_image = Image.new("RGB", input_image.size)
    draw = ImageDraw.Draw(output_image)

    edgeSize = len(kernel) // 2

    if(makeGreyScale):
        for row in range(input_image.height):
            #rowStart = input_image.width * row
            for col in range(input_image.width):
                pixel = input_pixels[col,row]
                acc = pixel[0]
                acc += pixel[1]
                acc += pixel[2]
                acc = int(acc/3)
                input_pixels[col,row] = (acc,acc,acc)
#                 pixel[0] = acc
#                 pixel[1] = acc
#                 pixel[2] = acc


    start = timeit.default_timer()
    for row in range(input_image.height):
        #rowStart = input_image.width * row
        for col in range(input_image.width):
            #pixStart = rowStart + col
            pixel = input_pixels[col,row]

            if row - edgeSize < 0 or row + edgeSize > input_image.height-1 or col-edgeSize < 0 or col+edgeSize > input_image.width-1:
                draw.point((col, row), pixel)
            else:
                acc = [0, 0, 0]
                for rowK in range(len(kernel)):
                    for colK in range(len(kernel)):

                        kPixel = input_pixels[col- edgeSize + colK, row- edgeSize + rowK]
                        acc[0] += kPixel[0] * kernel[rowK][colK]
                        acc[1] += kPixel[1] * kernel[rowK][colK]
                        acc[2] += kPixel[2] * kernel[rowK][colK]

                draw.point((col, row), (int(acc[0]), int(acc[1]), int(acc[2])))

    elapsed = timeit.default_timer() - start

    if shouldSave:
        print("saving..")
        output_image.save(fileOutputLocation)

    return elapsed
#Parrallel Still have to implement. Super simple once the non parrallel stuff works
def convoluteFileParallel(inputFile, fileOutputLocation,kernel,numThreads):
	pool = mp.Pool(processes=numThreads)

# def calculateOne(fileIn,row,col):
# 	#find value

def getDummyKernel(size):
    kernel = []
    for r in range(size):
        kernel.append([])
        for c in range(size):
            kernel[r].append(.11)
    return kernel

def main():

    fileInputLocation = None
    fileOutputLocation = None
    shouldSave = False
    numThreads = 1
    kernelSize = 0
    kernel = None
    shouldWriteToTiming = False
    makeGreyScale = False

    if len(sys.argv) > 1:
        if len(sys.argv) < 5:
            print("Usage: python kernelConvolution.py fileInputLocation [fileOutputLocation|-nosave] numThreads [3|5]")
            return
        else:
            fileInputLocation = sys.argv[1]
            fileOutputLocation = sys.argv[2]
            shouldSave = not (fileOutputLocation == "-nosave")
            numThreads = sys.argv[3]
            kernelSize = sys.argv[4]
            kernel = getDummyKernel(int(kernelSize))
            shouldWriteToTiming = False
            makeGreyScale = False # disable this
    else:
        with open('../transferData/config.json') as f:
            config = json.load(f)

            fileInputLocation = config['fileInputLocation']
            fileOutputLocation = config['pythonOutputLocation']#os.path.join(transferDataDir,"python-" + os.path.basename(inputFile))
            kernel = config['kernel']
            numThreads = config['numThreads']
            makeGreyScale = config['greyScale']
            shouldSave = True
            if not fileInputLocation:
                print("invalid file input location")
                return

    #print(kernel)

    #This copy file function is in place of the filter that has to be written
    #In the implementation, read image from the input file, and write image to output file
    #then replace the 100.01 with the seconds it took to perform the kernel convolution
    #copyfile(inputFile, fileOutputLocation)
    time = convoluteFileNoParallel(fileInputLocation,fileOutputLocation,kernel, numThreads, makeGreyScale, shouldSave)
    print("time: " + str(time))
    #kernel
    if shouldWriteToTiming:
        with open('../transferData/pythonTiming.json', "r+") as pf:
            pythonTiming = json.load(pf)
            pythonTiming['fileOutputLocation'] = fileOutputLocation
            pythonTiming['timing'] = time
            pf.seek(0)
            json.dump(pythonTiming, pf)
            pf.truncate()



main()
