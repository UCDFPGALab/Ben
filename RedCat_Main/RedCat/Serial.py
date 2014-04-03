import serial
import sys
import numpy
from time import sleep
import random
from pyqtgraph import QtCore, QtGui
from pyqtgraph import multiprocess as mp
import pyqtgraph as pg
import time
import datetime


port = "/dev/" + sys.argv[1];
outputFile = open('data3.txt', 'w');

try:
    ser = serial.Serial(port, 115200, timeout=1)
except IOError:
    print port, ' CANNOT BE OPENED. TERMINATING SCRIPT';
    sys.exit();

aFile = open('logFile', 'w')
holder = False
WIDTH = 4.40
LENGTH = 3.00


def serialEvent(serialInput):
    global holder
    global initialized
    sys.stdout.write('WAITING...\n')
    while holder == False:
        inByte = serialInput.read()
        sys.stdout.write(inByte)
        sys.stdout.write('% \n')
        #if len(inByte) > 0:
	if inByte == "$":
            serialInput.flushOutput()
            serialInput.flushInput()
            serialInput.write('A') # Hard set to 'A' right now
            holder = True;
            sys.stdout.write("FOUND ARDUINO\n")


def waitForConfirmation(confirmationDelay):
    sleep(confirmationDelay / 1000.0)
    inByte = ser.read()
    sys.stdout.write(inByte);
    sys.stdout.write('\n');
    if inByte == 'Y':
       ser.flushOutput
       ser.flushInput
       return True
    else:
        return False    


# Get numeric input and confirm with the Arduino (very slow but very safe)

# Arguments:
#   message:        String to display along with asking for user input
#   minimum:        Minimum value for user input
#   maximum:        Maximum value for user input - if minimum equals maximum, there is no range
#   floatFlag:      Floating point variable or  not
#   characterCode:  Way for the Arduino to know what input is coming in
#                   Code system:
#                      A = data value
#                      B = number of runs
#                      C = delay between runs
#                      D = number of times to re-read address after failure
#                      (space) = no character code - only PC needs to know value                    
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



            # Appended to user input - a character code, the length of the string, and a termination character
            userInput = characterCode + userInput

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
                    if characterCode != ' ':
                        sys.stdout.write("SET VALUE ON ARDUINO\n")
                    else:
                        sys.stdout.write("SET VALUE LOCALLY\n")
                    keepSending = False;

            validInput = True
        except ValueError:
            sys.stdout.write("BAD INPUT\n")


    if len(userInput) < 3:
        if floatFlag == 0:
            return int(userInput[1])
        else:
            return float(userInput[1])
    else:
        if floatFlag == 0:
            return int(userInput[1:len(userInput) - 1])
        else:
            return float(userInput[1:len(userInput) - 1])

            



def getStringInput(message):
    validInput = False;
    ser.flush();
    ser.flushInput();
    ser.flushOutput();
    while validInput == False:
            userInput = raw_input(message)
            userInput = userInput.upper()
            if userInput == "YES" or userInput == "NO" or userInput == "Y" or userInput == "N":
                validInput = True;
            else:
                sys.stdout.write("BAD INPUT\n")
    return userInput.upper()



serialEvent(ser)

               

holderString = "INPUT DATA VALUE: "
dataInt = getNumberInput(holderString, 0, 255, 0, 'A')

holderString = "INPUT NUMBER OF RUNS: "
getNumberInput(holderString, 0, 0, 0, 'B')

holderString = "INPUT RADIATION VALUE (pA): "
radiationDose = getNumberInput(holderString, 0, 0, 1, ' ')


holderString = "INPUT DELAY BETWEEN READ AND WRITE CYCLES (ms): "
getNumberInput(holderString, 0, 1000000, 1, 'C')

holderString = "INPUT NUMBER OF TIMES TO RE-READ ADDRESS AFTER FAILURE: "
getNumberInput(holderString, 0, 100, 1, 'D')

ser.write("$")
ser.flushInput()
sys.stdout.write('STUFF STILL IN BUFFER\n');
sys.stdout.write(ser.readline());
ser.flushOutput()
initTime = time.time()


##cycleFlag = 0 if cycling[0] == 'N' else cycleFlag = 1;


##radRate = (radiationDose * pow(10, -12) / 4.39 / pow(10, -11))
##radRate = 1
##print radRate

#initialTime = time.time();
#totalDosage = list()
#totalUpsets = 0
#totalLatches = 0


# Need to collect data and plot simultaneously - Utilize multiprocessing to accomplish this
# by creating a separate graphing process
#app = pg.mkQApp()
#proc = mp.QtProcess()
#remotepg = proc._import('pyqtgraph')
##eventUpsets = 0
##latches = 0
##eventUpsetRate = proc.transfer([])
##latchRate = proc.transfer([])

#app = QtGui.QApplication([])
#win = remotepg.GraphicsWindow(title="CHIP TESTER V0.0")
#win.resize(1600,900)
#win.setWindowTitle('Central Command')

#p1 = win.addPlot(title = "Dose versus Time")
#p2 = win.addPlot(title="Upset rate versus Integrated Dose")
#p3 = win.addPlot(title = "Latch rate versus Integrated Dose")
#win.nextRow()
#p4 = win.addPlot(title = "Latch rate versus Current")
#p5 = win.addPlot(title = "Latch rate versus Time")
#p6 = win.addPlot(title = "Display of upsets")



#superClock = time.mktime(time.gmtime())
#initClock = superClock
#currentTime = 0
#latchRate = proc.transfer([]);
#upsetRate = proc.transfer([]);
#timing = proc.transfer([]);
#currentDose = proc.transfer([]);
#totalDose = proc.transfer([]);
#upsetHolder = 0
#latchHolder = 0

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

while True:
#    sys.stdout.write('READING...\n');
    line = ser.readline()
    if len(line) > 0: #and is_number(line):
    	sys.stdout.write(line);
    	sys.stdout.write('\n\n');
	outputFile.write(line);
	outputFile.write('\n');
    
#
#    currentTime = time.time()
#
#    if len(line) != 0:
#        sys.stdout.write(line)
#        #if "EC" in line:
#        #    aFile.write(str(int(line[line.index("EC") + 2:]) - (currentTime - initTime)) + "\n")
#        aFile.write(line)
        #aFile.write("\n" + str(currentTime - initTime) + "\n")
        
        #if(line[1].isdigit()):
        #    upsetHolder = upsetHolder + 1
        #if(line[0].isdigit()):
        #    latchHolder = latchHolder + 1
#    if time.mktime(time.gmtime()) - superClock > 1:
#        superClock = time.mktime(time.gmtime())
#        latchRate.extend([latchHolder])
#        upsetRate.extend([upsetHolder])
#        timing.extend([(superClock - initClock)])
#        currentDose.extend([radiationDose])
#        totalDose.extend([sum(currentDose) + radiationDose])
#        upsetHolder = 0
#        latchHolder = 0
#        #p1.setData(x=timing, y=rate, _callSync='off')
#        p1.plot(timing, totalDose, pen=(200,200,200), symbolBrush=(255,0,0))
#        p2.plot(totalDose, upsetRate, pen=(200,200,200), symbolBrush=(255,0,0))
#        p3.plot(totalDose, latchRate, pen=(200,200,200), symbolBrush=(255,0,0))
##        if line[len(line) - 1] == 'U':
##            totalUsets = totalUpsets + 1
##        elif line[len(line) - 1] == 'L':
##            totalLatches = totalLatches + 1
##        else:
##            sys.stdout.write("SOMETHING WENT WRONG\n")
        
        



ser.close()


