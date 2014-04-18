//Various pins on the RedCat chip - Note: Ground and Vcc not included

const byte CS = 52; // Chip select
const byte OE = 2; // Output enable''
const byte WR = 53; // Write enable
const byte LED = 22; // Testing LED
//float startTime;

const int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 5;
const char TERMINATION = '$';
const int ADDRESS_MASK = 1022;
const int DATA_MASK = 30;

const byte DELAY_AFTER_ADDRESS = 0; //in microseconds, 10 is more than enough, but probably depends on cable setup
const bool VERBOSE = true;

typedef enum {NONE, GOT_DATA, GOT_NUM_RUNS, GOT_DELAY, GOT_REPEAT_READS, GOT_MODE} states;
typedef enum {ALL0, ALL1, ALTERNATING, NORMAL, RANDOM} mode;

states currentState = NONE;
mode currentMode = NORMAL;

int currentValue = 0;
byte output1, output2, output3;
const char SET_CODES[] = {'a', 'b', 'c', 'd', 'e'};

// Data and address integers
int correctData = 170;

int divider = 0;
int numRuns = 1; // Number of runs

int delayAfterWrite = 10; // Delay after writing (in milliseconds)
int readsAfterFailure = 5; // Number of reads if there is a bad read

int i = 0;

void getData() {
  int mod = 0;
  bool done = false;
  while (!done) {
    if (Serial.available()) {
      done = processIncomingByte (Serial.read());
    }
  }

  correctData = messedData(correctData);
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
    case GOT_MODE:
       if(currentValue==0) {
         currentMode = ALL0;
       }
       else if(currentValue==1) {
         currentMode = ALL1;
       }
       else if (currentValue == 2) {
         currentMode = ALTERNATING;
       }
       else if (currentValue == 3) {
         currentMode = NORMAL;
       }
       else if (currentValue == 4) {
         currentMode = RANDOM;
       }
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
 E = mode
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
      case 'E':
        currentState = GOT_MODE;
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
  while (Serial.read() != 'A');
}



// Read a byte of data given an address and an array of data
int readData(const long& address) {
  int data = 0;
  
  digitalWrite(CS, HIGH);
  digitalWrite(OE, HIGH);
  
  REG_PIOC_ODSR = address;

  digitalWrite(CS, LOW);
  digitalWrite(OE, LOW);

  data = REG_PIOD_PDSR & 0b01111001111;

  digitalWrite(OE, HIGH);
  digitalWrite(CS, HIGH);
  return data;
}



// Read a byte of data given an address and an array of data
int readDataAddr(const long& address) {
  //function assumes that OE and CS have already been drawn low

  REG_PIOC_ODSR = address;

  delayMicroseconds(1);

  return (REG_PIOD_PDSR & 0b01111001111);
}



// Write a byte of data
void writeData(const long& address, const int& data) {
  REG_PIOC_ODSR = address;
  
  REG_PIOD_ODSR = data;

  digitalWrite(CS, LOW);
  digitalWrite(WR, LOW);
  
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
}



String toBinary(const int& val, const int& length) {
  String holder = "";
  int mask = 1 << length - 1;

  for (i = 0; i < length; i++) {
    if ((val & mask) == 0) {
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



void reread(const long& address, const int& times, int (*fread)(const long&)) {
  // If the read-back data is incorrect read the address more
  int reads[times];
  int netFalseReads = 0;
  long resetaddr = incrementAddress(address);

  for (i = 0; i < times; i++) {
    (*fread)(resetaddr);
    reads[i] = (*fread)(address);
    if (correctData != reads[i]) {
      netFalseReads++;
    }
  }

  if (VERBOSE) {
    Serial.print("CURRENT MODE: ");
    Serial.println(currentMode);
    Serial.print("Total number of invalid reads: ");
    Serial.println(netFalseReads + 1);
    Serial.print("Read back: ");
    for (i = 0; i < times; i++) {
      Serial.print(reads[i]);
      Serial.print("\t");
    }
    Serial.println();
  }
  else {
    Serial.println(netFalseReads + 1);
  }
}


long incrementAddress(const long& address) {
  // Strange addition is necessary to ensure that dedicated pins do not get set. In this case, pins 33 through 41 are being used and then pins 45 through 51. Thus, when the following situation is met:
  // 0000000 00 111111111 0 -> 0000001 00 000000000 0
  if ((address & ADDRESS_MASK) == (ADDRESS_MASK)) {
    return (address + 3074);
  }
  else {
    return (address + 2);
  }
}

int messedData(const int& data) {
  int mod;
  int ret;
  ret = data;
  // Using PD0 through PD9 without PD4 and PD5. Thus, need to split the data at PD4 and PD5;
  mod = ret & 15; // Get the last 4 digits
  ret >>= 4; // Bit shift up to where 0's should start
  ret <<= 6; // Add the two extra zeros
  ret += mod; // Re-add mod
  return ret;
}


void setCorrectData(const int& addr){
  if(currentMode == ALL0) {
    correctData = messedData(0);
  }
  else if (currentMode == ALL1) {
    correctData = messedData(255);
  }
  else if (currentMode == ALTERNATING) {
    correctData = correctData ^ 0b1111001111;
  }
  else if (currentMode == NORMAL) {
    correctData = messedData(addr%256);
  }
  else if (currentMode == RANDOM) {
    correctData = messedData(random(256));
  }
}



void setup()
{
  Serial.begin(115200);
  // Set modes of IO
  pinMode(CS, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, HIGH);
  digitalWrite(OE, HIGH);
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
  //These pins are to be set high after all operations to reduce safeguard overhead

  // Address pins
  for (i = 33; i < 52; i++) {
    pinMode(i, OUTPUT);
    if (i == 41) {
      i = 44;
    }
  }
  //startTime = micros();
}



void loop() {
  establishContact();
  getData();

  for (int rn = 0; rn < numRuns; rn++) { //main loop for the tests
    int addr = 0;
    long addressInt = 0;
    int dataInt = 0;
    int seed = analogRead(0);

    // Current behavior is read all addresses, then write all the addresses.
    // However, this can be changed to write and address, then read it right away, and so on.

    // REG_PIOD_OWER = 0x3CF;
    // Set pins to outputs
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    for (i = 25; i < 31; i++) {
      pinMode(i, OUTPUT);
    }

    correctData = 0;
    randomSeed(seed);

    for (addr = 0; addr < NUM_ADDRESSES; addr++) { //write loop
      setCorrectData(addr);
            
      writeData(addressInt, correctData);
      addressInt = incrementAddress(addressInt);
    }

    // Set pins to inputs
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    for (i = 25; i < 31; i++) {
      pinMode(i, INPUT);
    }
    addressInt = 0; //reset the address between our run and read cycles back to 0
    correctData = 0;
    randomSeed(seed);
    
    //Bring the necessary pins down for the read cycle
    digitalWrite(CS, LOW);
    digitalWrite(OE, LOW);
    
    for (addr = 0; addr < NUM_ADDRESSES; addr++) { //read loop
      setCorrectData(addr);
      readDataAddr(addressInt);
      dataInt = readDataAddr(addressInt);

      // Print out the address and received data if bad data read
      if (correctData != dataInt) {
        if (VERBOSE) {
          Serial.print("\nAddress:\t");
          Serial.println(addr);
          Serial.print("Correct data:\t");
          Serial.println(correctData);
          Serial.print("Read data:\t");
          Serial.println(dataInt);
          Serial.print("Difference of the data:\t");
          Serial.println(correctData - dataInt); //toBinary(dataInt ^ correctData, 10)
          //Serial.print("Time after start (microseconds):\t");
          //Serial.println((micros()-startTime));
        }
        else {
          Serial.println(addr);
          Serial.println(correctData);
          Serial.println(dataInt);
        }
        // If the read-back data is incorrect reread n times, n = readsAfterFailure
        reread(addressInt, readsAfterFailure, readDataAddr);
      }
      addressInt = incrementAddress(addressInt);
    }
    
    //Bring the necessary pins up at the end of the read cycle
    digitalWrite(OE, HIGH);
    digitalWrite(CS, HIGH);

    if (VERBOSE) {
      Serial.print("Done with run ");
      Serial.print(rn + 1);
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
