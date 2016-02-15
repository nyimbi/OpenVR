#!/usr/bin/python
#This picks templates in a dir andcompresses the first and second fingers onboth hands

from multiprocessing import Pool
import os
import sys
import subprocess

try:
    import lz4
except ImportError as err:
    print("Fatal ImportError  module lz4 not found ...try going to http://pypi.python.org/pypi/lz4 ")
    print("For Ubuntu, solve by- sudo apt-get install python-setuptools python-dev")
    print("sudo easy_install lz4")
    sys.exit();
 
'''
Created on Dec 22, 2012

@author: dftaiwo
'''
if os.path.exists("/Users/dftaiwo"):
    rootDir = "/Users/dftaiwo/Documents/INITS/openvr/data"
elif os.path.exists("/opt/openvr"):
    rootDir = "/opt/openvr"
else:
    print("Unable to find a valid rootDir path")
    sys.exit()   

sourceDir = "{0}/stored_minutiae".format(rootDir)
outputDir = "{0}/sl_compressed_minutiae".format(rootDir)
templatesSourceList = "{0}/templates.lst".format(rootDir)
errorFile = "{0}/error.log".format(rootDir)
maxThreads = 500
c = 0
def createDir(dirToCheck):
    if not os.path.exists(dirToCheck):
        print("{0} does not exist. Creating it".format(dirToCheck))
        return os.makedirs(dirToCheck)
    return True

def listDirFiles(dirPath):
    filesInDir = []
    if not os.path.exists(dirPath):
        print("{0} does not exist!".format(dirPath))
        return filesInDir
    try:    
        filesInDir = os.listdir(dirPath)
    finally:
        return filesInDir

def copyDirStructure(srcRoot, dstRoot):
        print("Copying Directory Structure Using Rsync From {0} to {1}.\n This may take a while".format(srcRoot, dstRoot))
        copyCmd = "rsync -a -f'+ */' -f'- *' {0}/ {1}".format(srcRoot, dstRoot)
        #
        p = subprocess.Popen(copyCmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            print line,
        retval = p.wait()
        
    

def readDataFile(filePath, readAsLines=True):
    try:
        f = open(filePath, "r")
        if readAsLines:
            rawFileData = f.readlines()
        else:
            rawFileData = f.read()
        f.close()
        return rawFileData
    except IOError as err:
        print("Fatal I/O error: {0} . Unable to find file {1}".format(err, filePath))
        sys.exit()
    except:
        print("My o My! Unexpected error:", sys.exc_info()[0])
        raise

def clearErrorLog():
        aFp = open(errorFile, "w")
        aFp.write("");
        aFp.close()


def logError(errorMessage):
        aFp = open(errorFile, "a")
        aFp.write(errorMessage);
        aFp.write("\n");
        aFp.close()


def compressFile(srcFile, dstFile):
        if not os.path.exists(srcFile):
            logError("NotFound:{0}".format(srcFile))
            return
#        print("Compressing to {0}".format(dstFile))
        sourceContents = readDataFile(srcFile, False)
        compressed_data = lz4.dumps(sourceContents)
        try:
            cfile = open(dstFile, 'w')
            cfile.write(compressed_data)
            cfile.close()
        except IOError as err:
            logError ("WriteError:{0}", dstFile)    

def processSourceLine(srcLine):
        srcLineParts = srcLine.split(",")
        templateId = srcLineParts[0]
        regVin = srcLineParts[1]
        templateName = srcLineParts[2]
        fingerNumber = srcLineParts[3]
        srcFile = "{0}/{1}/{2}.xyt".format(sourceDir, regVin, templateName)
        dstFile = "{0}/{1}/{2}.xyt".format(outputDir, regVin, templateName)
        compressFile(srcFile, dstFile)
        logFile= "{0}/logs/{1}.log".format(rootDir,os.getpid())
        aFp = open(logFile, "a")
        aFp.write(srcFile);
        aFp.write("\n");
        aFp.close()
#        print(incrementCompressed())
    
if __name__ == '__main__':
    
    if not os.path.exists(sourceDir):
        print("Unexpected error: {0} not found".format(sourceDir))
        sys.exit();
        
    if(createDir(outputDir) == False):
        print("Unexpected error: {0} not found and unable to create".format(outputDir))
        sys.exit();
    
    clearErrorLog()
    
    fingerprintTemplates = readDataFile(templatesSourceList, True)
    print("Found {0} templates in {1}".format(len(fingerprintTemplates), templatesSourceList))

#    copyDirStructure(sourceDir, outputDir)
#    processSourceLine(fingerprintTemplates[0])
    print("Now Compressing Data!")
    pool = Pool(processes=maxThreads)
    x = pool.map(processSourceLine, fingerprintTemplates)           
    print("Done Compressing Data!")
    errorFileContents = readDataFile(errorFile, True)
    print("Errors: {0}".format(len(errorFileContents)))


    
