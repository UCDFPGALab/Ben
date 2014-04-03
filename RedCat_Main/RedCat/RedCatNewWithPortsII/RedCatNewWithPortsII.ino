//Various pins on the RedCat chip - Note: Ground and Vcc not included

const int CS = 52; // Chip select
const int OE = 2; // Output enable''
const int WR = 53; // Write enable
const int LED = 22; // Testing LED
float startTime;

const int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 4;
const char TERMINATION = '$';
const int ADDRESS_MASK = 1022;
const int DATA_MASK = 30;
const int DELAY_AFTER_ADDRESS = 5 //in milliseconds
const bool VERBOSE = true;

typedef enum {NONE, GOT_DATA, GOT_NUM_RUNS, GOT_DELAY, GOT_REPEAT_READS} states;
states currentState = NONE;
unsigned int currentValue = 0;
byte output1, output2, output3;
const char SET_CODES[] = {'a', 'b', 'c', 'd'};

// Data and address integers
int correctData = 170;
int dataInt;
int addressSteps = 0;
long addressInt = 0;

double holdAmount = 0.1;
boolean writing = true;
boolean correct = true;

int divider = 0;
int numRuns = 1;           // Number of runs 
int currentRun = 0;

int delayAfterWrite = 10;  // Delay after writing (in milliseconds)
int readsAfterFailure = 5; // Number of reads if there is a bad read

int netFalseReads = 0;
int i = 0;

void setup()
{
  Serial.begin(115200);
  //randomSeed(analogRead(5));
  establishContact();
  int mod = 0;
  // Set modes of IO
  pinMode(CS, OUTPUT);
  pinMode(OE,OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Address pins
  for(i = 33; i < 52; i++) {
    pinMode(i, OUTPUT);
    if(i == 41) {
      i = 44;
    }
  }
  
  digitalWrite(CS, HIGH); //HIGH = output
  
  // Initialize serial connection
  getData();

  // Using PD0 through PD9 without PD4 and PD5. Thus, need to split the data at PD4 and PD5; 
  mod = correctData & 15; // Get the last 4 digits
  correctData >>= 4;  // Bit shift up to where 0's should start
  correctData <<= 6;  // Add the two extra zeros
  correctData += mod; // Re-add mod
  startTime  = micros();
}



void loop()
{
  
  divider = (divider + 1) % 25000;
  if(divider > 12500) {
    digitalWrite(LED, HIGH);
  }
  else {
    digitalWrite(LED, LOW);
  }
  
  // Check to make sure the correct number of runs are being done
  if(currentRun < numRuns) {
    // Loop through all addresses then stop
    if(addressSteps < NUM_ADDRESSES) {
      // Write and then read
      if(writing) {
        if(addressSteps == 0) {
          //REG_PIOD_OWER = 0x3CF;
          // Set pins to outputs
          pinMode(11, OUTPUT);
          pinMode(12, OUTPUT);
          for(i = 25; i < 31; i++) {
            pinMode(i, OUTPUT);
          }
        }
        writeData(addressInt, correctData);
       
        // Strange addition is necessary to ensure that dedicated pins do not get set. In this case, pins 33 through 41 are being used and then pins 45 through 51. Thus, when the following situation is met:
        // 0000000 0000 111111111 0 -> 0000001 0000 000000000 0 
        if ((addressInt & ADDRESS_MASK) < (ADDRESS_MASK - 1)) {
           addressInt = addressInt + 2;
        }
        else {
          addressInt = addressInt + 3074;
        }
        
        addressSteps = addressSteps + 1;
      }
      else {
        // Reset all inputs to outputs
        if(addressSteps == 0) {
          pinMode(11, INPUT);
          pinMode(12, INPUT);
          for(i = 25; i < 31; i++) {
            pinMode(i, INPUT);
          }
        }
        // Used to see if output is consistent with initial input
        correct = true;
        
        dataInt = readData(addressInt);
        
        // Check if the data is correct 
        correct = check(correctData, dataInt);
		
        // Print out the address and received data if bad data read
        if(correct == false) {
          Serial.print("THIS IS ADDRESS: ");
          Serial.println(addressSteps);
          Serial.println(toBinary(dataInt ^ correctData, 10));
          Serial.println(" ");
          Serial.println(correctData);
          Serial.println(dataInt);

          // If the read-back data is incorrect read the address more
          
          for(i = 0; i < readsAfterFailure; i++) {
            dataInt = readData(addressInt);
            correct = check(correctData, dataInt);
            if(correct == false) {
              netFalseReads = netFalseReads + 1;
              Serial.print("BAD READ NUMBER: ");
              Serial.println(netFalseReads);
            }
          }
        }
        netFalseReads = 0;
      
        //increment the address
        addressSteps = addressSteps + 1;
        if((addressInt & ADDRESS_MASK) < (ADDRESS_MASK - 1)) {
          addressInt = addressInt + 2;
        }
        else {
          addressInt = addressInt + 3074;
        }
      }
    }
    
    // Way to tell if finished running through addresses
    else if(addressSteps == NUM_ADDRESSES) {
      if(writing) {
        writing = false;
        addressSteps = 0;
        addressInt = 0;
        delay(delayAfterWrite);
      }
      else {
        Serial.println("DONE\n");
        currentRun = currentRun + 1;
        writing = true;
        addressInt = 0;
        addressSteps = 0;
      }
    }
  }
}



// Checks that the data is correct by comparing the two data arrays
boolean check(int oldData, int data) {
  return (oldData == data);
}



void getData() {
  boolean done = false;
  while(!done) {
     divider = (divider + 1) % 100000;
     if(divider < 50000) {
        digitalWrite(LED, HIGH);
     }
     else {
        digitalWrite(LED, LOW);
     }

    if (Serial.available()) {
      done = processIncomingByte (Serial.read());
    }
  }
}



void handlePreviousState() {
  switch (currentState) {
    case GOT_DATA:
      correctData = currentValue;
      Serial.println("Y");
      break;
    case GOT_NUM_RUNS:
      numRuns = currentValue;
      Serial.println("Y");
      break;
    case GOT_DELAY:
      delayAfterWrite = currentValue;
      Serial.println("Y");
      break;
    case GOT_REPEAT_READS:
      readsAfterFailure = currentValue;
      Serial.println("Y");
      break;
  }
  currentValue = 0;
}



/* Character codes:
      A = data value
      B = number of runs
      C = delay between runs
      D = number of times to re-read address after failure
*/
boolean processIncomingByte (const byte c) {
  if (isdigit(c)) {
    currentValue = currentValue * 10;
    currentValue = currentValue + c - '0';
    return false;
  }
  else {
    handlePreviousState();
    switch (c) {
      case 'A':
        currentState = GOT_DATA;
        return false;
      case 'B':
        currentState = GOT_NUM_RUNS;
        return false;
      case 'C':
        currentState = GOT_DELAY;
        return false;
      case 'D':
        currentState = GOT_REPEAT_READS;
        return false;
      case TERMINATION:
        return true;
      default:
        currentState = NONE;
        return false;
    } 
  } 
}
 
 
 
void establishContact() {
   char holder[3];
   boolean done = false;
   char input = ' ';
   
   do {
     Serial.print('$');
     Serial.print('\n');
     delay(10);
     digitalWrite(LED, HIGH);
   }
   while(Serial.read() != 'A');
}
 

 
// Read a byte of data given an address and an array of data
int readData(const int& address) {
  int data = 0;
  int holder = 0;
  
  digitalWrite(OE, HIGH);
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH); //safeguard
  
  REG_PIOC_ODSR = address;
  delayMicroseconds(DELAY_AFTER_ADDRESS); //necessary to wait for register to set
  
  digitalWrite(CS, LOW);
  digitalWrite(OE, LOW);
  
  data = (PIOD->PIO_PDSR & (0b1111 << 6)) + (PIOD->PIO_PDSR & 0b1111);
  
  digitalWrite(OE, HIGH);
  digitalWrite(CS, HIGH);
  return data;
 }

 
 
// Write a byte of data
void writeData(int address, int data) {
  digitalWrite(OE, HIGH);
 
  REG_PIOC_ODSR = address;
  REG_PIOD_ODSR = data;
  
  digitalWrite(CS, LOW);
  digitalWrite(WR, LOW);
  
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
}

String toBinary(unsigned int val, int length) { 
  String holder = "";
  unsigned int mask = 1 << length - 1;
  
  for(i = 0; i < length; i++) {
     if( (val & mask) == 0) {
       holder += "0";
     }
     else {
       holder += "1"; 
     }
     // Bitshift the mask over one character
     mask >>= 1;
  }
  return holder;   
}



void reread(const long& address, const int& times) {
  // If the read-back data is incorrect read the address more
  int reads[
  for(i = 0; i < times; i++) {
    dataInt = readData(address);
    correct = check(correctData, dataInt);
    if(correct == false) {
      netFalseReads = netFalseReads + 1;
      Serial.print("BAD READ NUMBER: ");
      Serial.println(netFalseReads);
    }
  }
}