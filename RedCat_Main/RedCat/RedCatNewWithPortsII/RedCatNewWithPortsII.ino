//Various pins on the RedCat chip - Note: Ground and Vcc not included

const byte CS = 52; // Chip select
const byte OE = 2; // Output enable''
const byte WR = 53; // Write enable
const byte LED = 22; // Testing LED
//float startTime;

const unsigned int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 4;
const char TERMINATION = '$';
const int ADDRESS_MASK = 1022;
const int DATA_MASK = 30;

const byte DELAY_AFTER_ADDRESS = 10; //in microseconds, 10 is more than enough, but probably depends on cable setup
const bool VERBOSE = true;

typedef enum {NONE, GOT_DATA, GOT_NUM_RUNS, GOT_DELAY, GOT_REPEAT_READS} states;
states currentState = NONE;
unsigned int currentValue = 0;
byte output1, output2, output3;
const char SET_CODES[] = {'a', 'b', 'c', 'd'};

// Data and address integers
int correctData = 170;

int divider = 0;
int numRuns = 1;           // Number of runs

int delayAfterWrite = 10;  // Delay after writing (in milliseconds)
int readsAfterFailure = 5; // Number of reads if there is a bad read

int i = 0;

void setup()
{
  Serial.begin(115200);
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
  //startTime  = micros();
}



void loop() {
  establishContact();
  getData();

  for(int rn = 0; rn < numRuns; rn++) { //main loop for the tests    
    int addr = 0;
    long addressInt = 0;
    int dataInt = 0;
    
    // Current behavior is read all addresses, then write all the addresses.
    // However, this can be changed to write and address, then read it right away, and so on.
    
    // REG_PIOD_OWER = 0x3CF;
    // Set pins to outputs
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    for(i = 25; i < 31; i++) {
      pinMode(i, OUTPUT);
    }
    
    for(addr = 0; addr < NUM_ADDRESSES; addr++) { //write loop
      writeData(addressInt, correctData);
      incrementAddress(addressInt);
    }
    
    // Set pins to inputs
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    for(i = 25; i < 31; i++) {
      pinMode(i, INPUT);
    }
    addressInt = 0; //reset the address between our run and read cycles back to 0
    
    for(addr = 0; addr < NUM_ADDRESSES; addr++) { //read loop
      dataInt = readData(addressInt);
      
      // Print out the address and received data if bad data read
      if(!check(correctData, dataInt)) {
        if (VERBOSE){
          Serial.print("Address:\t");
          Serial.println(addr);
          Serial.print("Correct data:\t");
          Serial.println(correctData);
          Serial.print("Read data:\t");
          Serial.println(dataInt);
          Serial.print("XOR of the data:\t");
          Serial.println(toBinary(dataInt ^ correctData, 10));
          //Serial.print("Time after start (microseconds):\t");
          //Serial.println((micros()-startTime));
        }
        else {
          Serial.println(addr);
          Serial.println(correctData);
          Serial.println(dataInt);
        }
        // If the read-back data is incorrect reread n times, n = readsAfterFailure
        reread(addressInt, readsAfterFailure);
      }
      incrementAddress(addressInt);
    }
    
    if (VERBOSE) {
      Serial.print("Done with run ");
      Serial.print(rn+1);
      Serial.print('/');
      Serial.println(numRuns);
    }
    else {
      Serial.println("--");
    }
  }
  
  if (VERBOSE) {
    Serial.println("Done with all runs, ready for more data\n");
  }
  else {
    Serial.println("----\n");
  }
}



// Checks that the data is correct by comparing the two data arrays
bool check(const int& oldData, const int& data) {
  return (oldData == data);
}



void getData() {
  int mod = 0;
  bool done = false;
  while(!done) {
    if (Serial.available()) {
      done = processIncomingByte (Serial.read());
    }
  }
  
  // Using PD0 through PD9 without PD4 and PD5. Thus, need to split the data at PD4 and PD5; 
  mod = correctData & 15; // Get the last 4 digits
  correctData >>= 4;  // Bit shift up to where 0's should start
  correctData <<= 6;  // Add the two extra zeros
  correctData += mod; // Re-add mod
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
void writeData(const int& address, const int& data) {
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
      netFalseReads++;
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


void incrementAddress(long& address){
  // Strange addition is necessary to ensure that dedicated pins do not get set. In this case, pins 33 through 41 are being used and then pins 45 through 51. Thus, when the following situation is met:
  // 0000000 0000 111111111 0 -> 0000001 0000 000000000 0 
  if((address & ADDRESS_MASK) < (ADDRESS_MASK - 1)) {
    address += 2;
  }
  else {
    address += 3074;
  }
}
