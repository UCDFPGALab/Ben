import serial
import sys
import numpy
from time import sleep
import random
from pyqtgraph import QtCore, QtGui
from pyqtgraph import multiprocess as mp
import pyqtgraph as pg

import time

port = "/dev/" + sys.argv[1];

try:
    ser = serial.Serial(port, 115200, timeout=3)
except IOError:
    print port, ' CANNOT BE OPENED. TERMINATING SCRIPT';
    sys.exit();

aFile = open('logFile', 'w')
holder = False
WIDTH = 4.40
LENGTH = 3.00



def getNumberInput(message, minimum, maximum, floatFlag, characterCode):
    validInput = False
    ser.flush()
    ser.flushInput()
    ser.flushOutput()
    keepSending = True;
    confirmationDelay = 1
    while validInput == False:
        userInput = raw_input(message)
        try:
            if floatFlag == 0:
                int(userInput)
            else:
                float(userInput)
                
            if minimum != maximum and (int(userInput) < minimum or int(userInput) > maximum):
                raise ValueError


            userInput = characterCode + userInput;
            sys.stdout.write("TEST: " + str(userInput))
            ser.write(str(userInput))
            # Keep resending user-input data until confirmation or time-out
            while(keepSending):
                if confirmationDelay > 3000:
                    sys.stdout.write("COULD NOT CONFIRM WITH ARDUINO - ASSUME DEFAULT VALUE USED\n")
                    keepSending = False;
                else:
                    confirmationDelay = 2 * confirmationDelay

                if characterCode != ' ' and not(waitForConfirmation(confirmationDelay)):
                    ser.flush()
                    ser.flushInput()
                    ser.flushOutput()
                    ser.write(str(userInput))
                else:
                    keepSending = False;

            validInput = True
        except ValueError:
            sys.stdout.write("BAD INPUT\n")



    if floatFlag == 0:
        return int(userInput[1:len(userInput) - 1])
    else:
        return float(userInput[1:len(userInput) - 1])



def waitForConfirmation(confirmationDelay):
    sleep(confirmationDelay / 1000.0)
    inByte = ser.readline()
    sys.stdout.write("CONFIRMATION OUTPUT: " + str(inByte) + "\n")
    if inByte == "Y\n":
       ser.flushOutput
       ser.flushInput
       sys.stdout.write("TESTING: " + str(confirmationDelay/1000.0) + "\n")
       return True
    else:
        return False    






holderString = "INPUT DATA VALUE: "
getNumberInput(holderString, 0, 255, 0, 'D')

holderString = "INPUT RADIATION VALUE (pA): "
radiationDose = getNumberInput(holderString, 0, 0, 1, ' ')

holderString = "INPUT NUMBER OF RUNS: "
getNumberInput(holderString, 0, 0, 0, 'R')

holderString = "INPUT DELAY BETWEEN READ AND WRITE CYCLES (ms): "
getNumberInput(holderString, 0, 1000000, 0, 'W')

holderString = "INPUT NUMBER OF TIMES TO RE-READ ADDRESS AFTER FAILURE: "
getNumberInput(holderString, 0, 100, 0, 'A')

ser.write("$")

while True:
    line = ser.readline()
    if len(line) != 0:
        sys.stdout.write(line)
##        if line[len(line) - 1] == 'U':
##            totalUsets = totalUpsets + 1
##        elif line[len(line) - 1] == 'L':
##            totalLatches = totalLatches + 1
##        else:
##            sys.stdout.write("SOMETHING WENT WRONG\n")



