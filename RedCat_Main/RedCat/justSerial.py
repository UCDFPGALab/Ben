import serial
import sys
import numpy
from time import sleep
import random
#from pyqtgraph import QtCore, QtGui
#from pyqtgraph import multiprocess as mp
#import pyqtgraph as pg
import time
import datetime

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


def is_number(s):
  try:
      float(s)
      return True
  except ValueError:
      return False
      
def user_inputs():
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
  
def send_inputs():
  dataInt = getNumberInput(holderString, 0, 255, 0, 'A')
  getNumberInput(holderString, 0, 0, 0, 'B')
  radiationDose = getNumberInput(holderString, 0, 0, 1, ' ')
  getNumberInput(holderString, 0, 1000000, 1, 'C')
  getNumberInput(holderString, 0, 100, 1, 'D')
    
    
    
port = sys.argv[1];
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

serialEvent(ser)               

user_inputs();

ser.write("$")
ser.flushInput()
sys.stdout.write('STUFF STILL IN BUFFER\n');
sys.stdout.write(ser.readline());
ser.flushOutput()
initTime = time.time()

line = "    "
        
while (line != "$\n"):
    line = ser.readline()
    if len(line) > 0: #and is_number(line):
    	sys.stdout.write(line);
    outputFile.write(line);
    outputFile.write('\n');

ser.close()


