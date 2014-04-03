//Various pins on the RedCat chip - Note: Ground and Vcc not included

const byte CS = 52; // Chip select
const byte OE = 2; // Output enable''
const byte WR = 53; // Write enable
const byte LED = 22; // Testing LED
float startTime;

const unsigned int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 4;
const char TERMINATION = '$';
const int ADDRESS_MASK = 1022;
const int DATA_MASK = 30;

const byte DELAY_AFTER_ADDRESS = 5; //in milliseconds
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
bool writing = true;

int divider = 0;
int numRuns = 1;           // Number of runs 
int currentRun = 0;

int delayAfterWrite = 10;  // Delay after writing (in milliseconds)
int readsAfterFailure = 5; // Number of reads if there is a bad read

int i = 0;

void setup()
{
  Serial.begin(115200);
  establishContact();
  int mod = 0;
  // Set modes of IO
  pinMode(CS, OUTPUT);
  pinMode(OE,OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(LED, OUTPUT);
  
  digitalWrite(LED, HIGH);
  digitalWrite(OE, HIGH);
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
  //These pins are to be set high after all operations to reduce safeguard overhead

  // Address pins
  for(i = 33; i < 52; i++) {
    pinMode(i, OUTPUT);
    if(i == 41) {
      i = 44;
    }
  }
  
  // Initialize serial connection
  getData();

  // Using PD0 through PD9 without PD4 and PD5. Thus, need to split the data at PD4 and PD5; 
  mod = correctData & 15; // Get the last 4 digits
  correctData >>= 4;  // Bit shift up to where 0's should start
  correctData <<= 6;  // Add the two extra zeros
  correctData += mod; // Re-add mod
  //startTime  = micros(); //is this line necessary?
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
  
  if(currentRun < numRuns) { // Check to make sure the correct number of runs are being done
    if(addressSteps < NUM_ADDRESSES) { // Loop through all addresses then stop
      if(writing) {  // Write and then read
      
        if(addressSteps == 0) { //starting write cycle
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
        
        ++addressSteps;
      }
      else {
      
        if(addressSteps == 0) { // Reset all inputs to outputs
          pinMode(11, INPUT);
          pinMode(12, INPUT);
          for(i = 25; i < 31; i++) {
            pinMode(i, INPUT);
          }
        }
        // Used to see if output is consistent with initial input
        dataInt = readData(addressInt);
        
        // Print out the address and received data if bad data read
        if(!check(correctData, dataInt)) {
          
          if (VERBOSE){
            Serial.print("Address:\t");
            Serial.println(addressSteps);
            Serial.print("Correct data:\t");
            Serial.println(correctData);
            Serial.print("Read data:\t");
            Serial.println(dataInt);
            Serial.print("XOR of the data:\t");
            Serial.println(toBinary(dataInt ^ correctData, 10));
            Serial.println();
          }
          else {
            Serial.println(addressSteps);
            Serial.println(correctData);
            Serial.println(dataInt);
            Serial.println();
          }

          // If the read-back data is incorrect reread n times, n = readsAfterFailure
          reread(addressSteps, readsAfterFailure);
        }
        
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
        
        if (VERBOSE) {
          Serial.println("Done with current run\n");
        }
        else {
          Serial.println("--\n");
        }
        
        currentRun = currentRun + 1;
        writing = true;
        addressInt = 0;
        addressSteps = 0;
      }
    }
  }
}



// Checks that the data is correct by comparing the two data arrays
bool check(const int& oldData, const int& data) {
  return (oldData == data);
}



void getData() {
  bool done = false;
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
bool processIncomingByte (const byte& c) {
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
   bool done = false;
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
void writeData(const int& address,const int& data) {
  REG_PIOC_ODSR = address;
  REG_PIOD_ODSR = data;
  
  digitalWrite(CS, LOW);
  digitalWrite(WR, LOW);
  
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
}



String toBinary(const unsigned int& val, const int& length) { 
  String holder = "";
  unsigned int mask = 1 << length - 1;
  
  for(i = 0; i < length; i++) {
     if((val & mask) == 0) {
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
  int reads[times];
  int netFalseReads = 0;
  
  for(i = 0; i < times; i++) {
    reads[i] = readData(address);
    if(!check(correctData, reads[i])) {
      netFalseReads = netFalseReads + 1;
    }
  }
  
  if (VERBOSE) {
    Serial.print("Total number of invalid reads: ");
    Serial.println(netFalseReads + 1);
    Serial.print("Read back: ");
    for(i = 0; i < times; i++){
      Serial.print(reads[i]);
      Serial.print("\t");
    }
    Serial.println();
  }
  else {
    Serial.println(netFalseReads + 1);
  }
}


