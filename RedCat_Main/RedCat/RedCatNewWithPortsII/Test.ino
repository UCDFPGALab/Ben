//Various pins on the RedCat chip - Note: Ground and Vcc not included

const byte CS = 52; // Chip select
const byte OE = 2; // Output enable''
const byte WR = 53; // Write enable
const byte LED = 22; // Testing LED
//const byte POWER = 23; // Power pin
const byte VCC = 23;

const int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 5;
const char TERMINATION = '$';
const int ADDRESS_MASK = 1022;
const int DATA_MASK = 30;

const byte DELAY_AFTER_ADDRESS = 0; //in microseconds, 10 is more than enough, but probably depends on cable setup
const bool VERBOSE = true;

typedef enum {NONE, GOT_DATA, GOT_NUM_RUNS, GOT_DELAY, GOT_REPEAT_READS, GOT_MODE, GOT_POWER_CYCLES} states;
typedef enum {ALL0, ALL1, ALTERNATING, NORMAL, RANDOM} mode;

states currentState = NONE;
mode currentMode = NORMAL;
int currentValue = 0;
byte output1, output2, output3;
const char SET_CODES[] = {'a', 'b', 'c', 'd', 'e'};

// Data and address integers
int correctData = 170;
int numPowerCycles = 5;
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
    case GOT_POWER_CYCLES:
      numPowerCycles = currentValue;
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
 F = power cycle
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
      case 'F':
        currentState = GOT_POWER_CYCLES;
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
  delayMicroseconds(10);
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

 /*
  digitalRead(30);  // Least significant bit
  digitalRead(12);
  digitalRead(11);
  digitalRead(29);
  digitalRead(28);
  digitalRead(27);
  digitalRead(26);
  digitalRead(25); // Most Significant Bit
 */
 
 /*
  Serial.print("ADDRESS: ");
  Serial.print((address & 32768) ? 1 : 0);
  Serial.print((address & 16384) ? 1 : 0);
  Serial.print((address & 8192) ? 1 : 0);
  Serial.print((address & 4096) ? 1 : 0);
  Serial.print((address & 2048) ? 1 : 0);
  Serial.print((address & 1024) ? 1 : 0);
  Serial.print((address & 512) ? 1 : 0);
  Serial.print((address & 256) ? 1 : 0);
  Serial.print((address & 128) ? 1 : 0);
  Serial.print((address & 64) ? 1 : 0);
  Serial.print((address & 32) ? 1 : 0);
  Serial.print((address & 16) ? 1 : 0);
  Serial.print((address & 8) ? 1 : 0);
  Serial.print((address & 4) ? 1 : 0);
  Serial.print((address & 2) ? 1 : 0);
  Serial.println((address & 1) ? 1 : 0);
  */
 
  
  digitalWrite(45, (address & 1)); //A0 = least significant bit
  digitalWrite(46, (address & 2));
  digitalWrite(47, (address & 4));
  digitalWrite(48, (address & 8));
  digitalWrite(49, (address & 16));
  digitalWrite(50, (address & 32));
  digitalWrite(51, (address & 64));
  digitalWrite(41, (address & 128));
  digitalWrite(40, (address & 256));
  digitalWrite(39, (address & 512));
  digitalWrite(38, (address & 1024));
  digitalWrite(37, (address & 2048));
  digitalWrite(36, (address & 4096));
  digitalWrite(35, (address & 8192));
  digitalWrite(34, (address & 16384));
  digitalWrite(33, (address & 32768));

  
  
  int holder = 0;
  int value;
  
  value = 128 * digitalRead(25);
  value = value + 64 * digitalRead(26);
  value = value + 32 * digitalRead(27);
  value = value + 16 * digitalRead(28);
  value = value + 8 * digitalRead(29);
  value = value + 4 * digitalRead(11);
  value = value + 2 * digitalRead(12);
  value = value + digitalRead(30);
  //Serial.print("VALUE: ");
  //Serial.println(messedData(value));
  //delay(1000);
  return messedData(value);
  
  //REG_PIOC_ODSR = address;
  //Serial.print("READING ADDRESS ");
  //Serial.println(address);
  //delayMicroseconds(100);
  //delay(3000);
  //return (REG_PIOD_PDSR & 0b01111001111);
}



// Write a byte of data
void writeData(const long& address, const int& data) {
  //REG_PIOC_ODSR = address;
  //Serial.print("WRITING DATA: ");
  //Serial.println(data);
  //delay(1000);
  //REG_PIOD_ODSR = data;
    
  digitalWrite(45, (address & 1)); //A0 = least significant bit
  digitalWrite(46, (address & 2));
  digitalWrite(47, (address & 4));
  digitalWrite(48, (address & 8));
  digitalWrite(49, (address & 16));
  digitalWrite(50, (address & 32));
  digitalWrite(51, (address & 64));
  digitalWrite(41, (address & 128));
  digitalWrite(40, (address & 256));
  digitalWrite(39, (address & 512));
  digitalWrite(38, (address & 1024));
  digitalWrite(37, (address & 2048));
  digitalWrite(36, (address & 4096));
  digitalWrite(35, (address & 8192));
  digitalWrite(34, (address & 16384));
  digitalWrite(33, (address & 32768));
 
 /* 
  Serial.print("ADDRESS: ");
  Serial.print((address & 32768) ? 1 : 0);
  Serial.print((address & 16384) ? 1 : 0);
  Serial.print((address & 8192) ? 1 : 0);
  Serial.print((address & 4096) ? 1 : 0);
  Serial.print((address & 2048) ? 1 : 0);
  Serial.print((address & 1024) ? 1 : 0);
  Serial.print((address & 512) ? 1 : 0);
  Serial.print((address & 256) ? 1 : 0);
  Serial.print((address & 128) ? 1 : 0);
  Serial.print((address & 64) ? 1 : 0);
  Serial.print((address & 32) ? 1 : 0);
  Serial.print((address & 16) ? 1 : 0);
  Serial.print((address & 8) ? 1 : 0);
  Serial.print((address & 4) ? 1 : 0);
  Serial.print((address & 2) ? 1 : 0);
  Serial.println((address & 1) ? 1 : 0);
  delay(2000);
  */
  /*
  Serial.print("WRITTEN DATA: ");
  Serial.print((data & 512) ? 1 : 0);
  Serial.print((data & 256) ? 1 : 0);
  Serial.print((data & 128) ? 1 : 0);
  Serial.print((data & 64) ? 1 : 0);
  Serial.print('0');
  Serial.print('0');
  Serial.print((data & 8) ? 1 : 0);
  Serial.print((data & 4) ? 1 : 0);
  Serial.print((data & 2) ? 1 : 0);
  Serial.println((data & 1) ? 1 : 0);
  delay(2000);
  */
  //Serial.print("WRITTEN DATA: ");
  //Serial.println(data);
  
  digitalWrite(30, data & 1);
  digitalWrite(12, data & 2);
  digitalWrite(11, data & 4);
  digitalWrite(29, data & 8);
  digitalWrite(28, data & 64);
  digitalWrite(27, data & 128);
  digitalWrite(26, data & 256);
  digitalWrite(25, data & 512);

  
  
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
    //(*fread)(resetaddr);
    reads[i] = (*fread)(address);
    if (correctData != reads[i]) {
      netFalseReads++;
    }
  }

  if (VERBOSE) {
    /*
    Serial.println(" ");
    Serial.println(" ");
    Serial.print("CURRENT MODE: ");
    Serial.println(currentMode);
    Serial.print("ADDRESS: ");
    Serial.println(toBinary(address, 16));
    Serial.print("CORRECT DATA: ");
    Serial.print(correctData);
    Serial.print("\\\\");
    Serial.println(toBinary(correctData, 10));
    
    Serial.print("Total number of invalid reads: ");
    Serial.println(netFalseReads + 1);
    Serial.print("Read back: ");
    for (int i = 0; i < times; i++) {
      Serial.print(reads[i]);
      Serial.print("\\\\");
      Serial.print(correctData);
      Serial.print("\\\\");
      Serial.print(toBinary((~(reads[i] & correctData)), 10));
      Serial.print('e');
      Serial.println(i);
      //Serial.print("\t");
    }
    Serial.println();
    Serial.println();
    */
  }
  else {
    Serial.print("BR ");
    Serial.println(netFalseReads + 1, HEX);
    Serial.print("NR ");
    for (i = 0; i < times; i++) {
      Serial.print(reads[i], HEX);
      Serial.print("\t");
    }
    Serial.println();
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
    //correctData = correctData ^ 0b1111001111;
    //correctData = messedData(addr%256);  //-- Silly; don't do anything
    correctData = messedData(addr%256);
    //correctData = messedData(121);
    //Serial.print("CORRECT DATA: ");
    //Serial.println(correctData);
    //delay(2000);
  }
  else if (currentMode == RANDOM) {
    correctData = messedData(random(256));
  }
}



void setup() {
  Serial.begin(115200);
  // Set modes of IO
  pinMode(CS, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(VCC, OUTPUT);
  
  pinMode(LED, OUTPUT);
  
  digitalWrite(LED, HIGH);
  digitalWrite(VCC, HIGH);
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
}



void loop() {
  establishContact();
  getData();
  Serial.print("CORRECT DATA: ");
  Serial.println(correctData);
  delay(3000);

  for (int pwrc = 0; pwrc < numPowerCycles; pwrc++) {
    digitalWrite(VCC, HIGH);
    
    for (int rn = 0; rn < numRuns; rn++) { //main loop for the tests
      int addr = 0;
      long addressInt = 0;
      int dataInt = 0;
      int seed = analogRead(0);

      pinMode(11, OUTPUT);
      pinMode(12, OUTPUT);
      for (i = 25; i < 31; i++) {
        pinMode(i, OUTPUT);
      }

      correctData = 0;
      randomSeed(seed);
/*
      for (addr = 0; addr < NUM_ADDRESSES; addr++) { //write loop
        setCorrectData(addr);
        //Serial.print("RECEIVED DATA: ");
        //Serial.println(correctData);
        writeData(addr, correctData);
        //writeData(addressInt, correctData);
        //addressInt = incrementAddress(addressInt);
      }

      // Set pins to inputs
      pinMode(11, INPUT);
      pinMode(12, INPUT);
      for (i = 25; i < 31; i++) {
        pinMode(i, INPUT);
      }
*/      
      addressInt = 0; //reset the address between our run and read cycles back to 0
      correctData = 0;
      randomSeed(seed);
      int badReads = 0;
      //Bring the necessary pins down for the read cycle
      digitalWrite(CS, LOW);
      digitalWrite(OE, LOW);
      
      for (addr = 0; addr < NUM_ADDRESSES; addr++) { //read loop
        setCorrectData(addr);
        //Serial.print("CORRECT DATA: ");
        //Serial.println(correctData);
        //dataInt = readDataAddr(addressInt);
        addressInt = (addressInt + 1);
 
        //correctData = addressInt % 2 == 1 ? 0 : messedData(255);
        //Serial.print("CORRECT DATA: ");
        //Serial.println(correctData);
        
        //Serial.print("ADDRESS: ");
        //Serial.println(addressInt);
        digitalWrite(OE, HIGH);
        digitalWrite(CS, HIGH);
        digitalWrite(WR, HIGH);
        pinMode(11, OUTPUT);
        pinMode(12, OUTPUT);
        for (i = 25; i < 31; i++) {
          pinMode(i, OUTPUT);
        }
        
        writeData(addressInt, correctData);
        
        digitalWrite(CS, LOW);
        digitalWrite(OE, LOW);
        //delay(1000);
        pinMode(11, INPUT);
        pinMode(12, INPUT);
        for (i = 25; i < 31; i++) {
          pinMode(i, INPUT);
        }
        //readDataAddr(addr);
        dataInt = readDataAddr(addressInt);
        //delayMicroseconds(30);
        //dataInt = readData(addressInt);
        //Serial.print("TEST: ");
        //Serial.println(dataInt);
        // Print out the address and received data if bad data read
        if (correctData != dataInt) {
          //badReads = badReads + 1;
          //Serial.print(addr);
          //Serial.print(": ");
          Serial.print(addressInt);
          Serial.print(": ");
          Serial.print(correctData);
          Serial.print("    ");
          Serial.print(dataInt);
          Serial.print("    ");
          Serial.println(toBinary(correctData ^ dataInt, 10)) ;
          /*
          if (VERBOSE) {
            Serial.print("\nAddress:\t");
            Serial.println(addr);
            Serial.print("Correct data:\t");
            Serial.println(correctData);
            Serial.print("Read data:\t");
            Serial.println(dataInt);
            Serial.print("Difference of the data:\t");
            Serial.println(correctData - dataInt); //toBinary(dataInt ^ correctData, 10)
          }
          else {
            Serial.print("AD ");
            Serial.println(addr,HEX);
            Serial.print("CD ");
            Serial.println(correctData, HEX);
            Serial.print("DI ");
            Serial.println(dataInt, HEX);
          }
          */
          // If the read-back data is incorrect reread n times, n = readsAfterFailure
          //reread(addressInt, readsAfterFailure, readDataAddr);
          //delay(1000);
          reread(addressInt, readsAfterFailure, readDataAddr);
        }
        //else
        //{
         // Serial.print("ADDRESS NUMBER: ");
         // Serial.println(addr);
 
        //}
        //addressInt = addressInt + 1;
        //addressInt = incrementAddress(addressInt);
      }
      
      
      //Bring the necessary pins up at the end of the read cycle
      digitalWrite(OE, HIGH);
      digitalWrite(CS, HIGH);

      if (VERBOSE) {
        Serial.print("Done with run ");
        Serial.print(rn + 1);
        Serial.print('/');
        Serial.println(numRuns);
        Serial.print("NUMBER OF BAD ADDRESSES: ");
        Serial.println(badReads);
      }
      else {
        Serial.println("--");
      }
    }
    if (VERBOSE) {
      Serial.print("Done with all runs, power cycle number ");
      Serial.print(pwrc+1);
      Serial.print(" of ");
      Serial.println(numPowerCycles);
    }
    else {
      Serial.println("----\n");
    }
    
    digitalWrite(VCC, LOW);
    delay(2000);
    digitalWrite(VCC, HIGH);
  }
}
