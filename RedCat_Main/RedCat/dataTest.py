import sys
import numpy


oldVal = 0;
outputFile = open('dataDifference.txt', 'w');

def is_int(s):
    try:
        int(s)
        return True
    except ValueError:
        return False

with open('data.txt') as f:
    for line in f:
	if is_int(line):
		currentVal = int(line);
		if currentVal < oldVal:
			oldVal = 0;
			holder = str(currentVal - oldVal) + "*\n";
		else:
			holder = str(currentVal - oldVal) + "\n";

		outputFile.write(holder);		
		oldVal = currentVal;	

        
