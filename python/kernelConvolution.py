import json
import os
import sys
import timeit

from shutil import copyfile
from scipy import misc
import numpy as np
from PIL import Image
from scipy import signal as sg
def convoluteFileNoParallel(inputFile, fileOutputLocation,kernel,numThreads, makeGreyScale, shouldSave):
    input_image = Image.open(inputFile)
    # input_pixels = input_image
    # input_image = misc.imread(inputFile, False, mode='L')
    # input_pixels = input_image.load()
    # kernel = np.asarray(kernel)
    # output_image = Image.new("RGB", input_image.size)
    # draw = ImageDraw.Draw(output_image)
    
    # edgeSize = len(kernel) // 2
    # Numpy Code
    input_image = np.asarray(input_image)
    # output_image = np.zeros((input_pixels.shape[0],input_pixels.shape[1],input_pixels.shape[2]))

    if(makeGreyScale):
        for row in range(input_image.height):
            #rowStart = input_image.width * row
            for col in range(input_image.width):
                pixel = input_pixels[row][col]
                acc = pixel[0]
                acc += pixel[1]
                acc += pixel[2]
                acc = int(acc/3)
                input_pixels[row][col] = np.asarray([acc,acc,acc])
                # pixel[0] = acc
                # pixel[1] = acc
                # pixel[2] = acc

    output_image = []
    start = timeit.default_timer()
    # print(np.asarray(input_image).shape)
    # print(np.asarray(kernel).shape)
    for index in range(3):
        piece = sg.convolve2d(input_image[:,:,index],np.asarray(kernel),boundary='symm', mode='same')
        output_image.append(piece)
    output_image = np.stack(output_image, axis=2).astype("uint8")
    # for row in range(input_image.height):
    #     #rowStart = input_image.width * row
    #     print(row)
    #     for col in range(input_image.width):
    #         #pixStart = rowStart + col
    #         pixel = input_pixels[row][col]

    #         if row - edgeSize < 0 or row + edgeSize > input_image.height-1 or col-edgeSize < 0 or col+edgeSize > input_image.width-1:
    #             # draw.point((row, col), tuple(pixel))
    #             output_image[row][col] = pixel
    #         else:
    #             acc = np.zeros(3)
    #             for rowK in range(len(kernel)):
    #                 for colK in range(len(kernel)):

    #                     kPixel = input_pixels[ row- edgeSize + rowK][col- edgeSize + colK]
    #                     acc = np.sum(np.multiply(kPixel,kernel[rowK][colK]))
    #                     # acc[0] += kPixel[0] * kernel[rowK][colK]
    #                     # acc[1] += kPixel[1] * kernel[rowK][colK]
    #                     # acc[2] += kPixel[2] * kernel[rowK][colK]

    #             # draw.point((col, row), (int(acc[0]), int(acc[1]), int(acc[2])))
    #             output_image[row][col] = acc

    elapsed = timeit.default_timer() - start
    output_image = Image.fromarray(output_image)
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
