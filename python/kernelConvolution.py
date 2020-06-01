import json
import os
from shutil import copyfile

with open('../transferData/config.json') as f:
    config = json.load(f)

inputFile = config['fileInputLocation']
if inputFile:

    transferDataDir = os.path.join(os.path.dirname(os.getcwd()),'transferData')
    fileOutputLocation = config['pythonOutputLocation']#os.path.join(transferDataDir,"python-" + os.path.basename(inputFile))
    kernel = config['kernel']

    print(kernel)

    #This copy file function is in place of the filter that has to be written
    #In the implementation, read image from the input file, and write image to output file
    #then replace the 100.01 with the seconds it took to perform the kernel convolution
    copyfile(inputFile, fileOutputLocation)


    with open('../transferData/pythonTiming.json', "r+") as pf:
        pythonTiming = json.load(pf)
        pythonTiming['fileOutputLocation'] = fileOutputLocation
        pythonTiming['timing'] = 100.01
        pf.seek(0)
        json.dump(pythonTiming, pf)
        pf.truncate()




