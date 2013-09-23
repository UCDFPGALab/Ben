
//Various pins on the RedCat chip - Note: Ground and Vcc not included

const int CS = 52; // Chip select
const int OE = 2; // Output enable''
const int WR = 53; // Write enable
float startTime;
//const int FIRST_ADDRESS = 22;
//const int FIRST_IO = 40;

const int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 4;
const char TERMINATION = '$';
const int ADDRESS_MASK = 1022;
const int DATA_MASK = 30;

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

int numRuns = 1;           // Number of runs 
int currentRun = 0;

int delayAfterWrite = 10;  // Delay after writing (in milliseconds) 
int readsAfterFailure = 5; // Number of reads if there is a bad read

// Delay in microseconds
int delayAmount = 1;

int netFalseReads = 0;

void setup()
{

  int mod = 0;
  // Set modes of IO
  pinMode(CS, OUTPUT);
  pinMode(OE,OUTPUT);
  pinMode(WR, OUTPUT);
  // Address pins
  for(int i = 33; i < 52; i++)
  {
    pinMode(i, OUTPUT);
    if(i == 41)
    {
      i = 44;
    }
  }
  
  digitalWrite(CS, HIGH);
  // 0 = input
  // 1 = output
  
  // Initialize serial connection
  Serial.begin(115200);
  randomSeed(analogRead(5));
  establishContact();
  getData();
  Serial.print("DATA: ");
  Serial.println(correctData);
  mod = correctData & 15; // Get the last 4 digits
  correctData >>= 4;  // Bit shift up to where 0's should start
  correctData <<= 6;  // Add the two extra zeros
  correctData += mod; // Re-add mod
/*
  Serial.print("NEW DATA: ");
  Serial.println(correctData);
  Serial.print("NUMBER OF RUNS: ");
  Serial.println(numRuns);
  Serial.print("DELAY AFTER WRITING (ms): ");
  Serial.println(delayAfterWrite);
  Serial.print("READS AFTER FAILURE: ");
  Serial.println(readsAfterFailure);
*/  
  startTime  = micros();
}


void loop()
{
  // Check to make sure the correct number of runs are being done
  if(currentRun < numRuns)
  {
    // Loop through all addresses then stop
    if(addressSteps < NUM_ADDRESSES)
    {
      // Write and then read
      if(writing)
      {
        if(addressSteps == 0)
        {
          
          //REG_PIOD_OWER = 0x3CF;

          pinMode(11, OUTPUT);
          pinMode(12, OUTPUT);
          for(int i = 25; i < 31; i++)
          {
            pinMode(i, OUTPUT);
          }
          
        }
        //addressInt = reverseAddress(addressInt);
        //Serial.print("EC");
        //Serial.println(micros());
        writeData(addressInt, correctData);
       
        /* PD5 must be input
           Using pins: PD0, PD1, PD2, PD3
                       PD6, PD7, PD8, PD9
        */
        //DDRD = DDRD | B111111001111
        //writeData(address, data);
        if((addressInt & ADDRESS_MASK) < (ADDRESS_MASK - 1))
        {
           addressInt = addressInt + 2;
        }
        else
        {
          addressInt = addressInt + 3074;
        }
        //addressInt = reverseBits16(addressInt);
        //addressInt = reverseAddress(addressInt);
        //Serial.println(addressInt);
        addressSteps = addressSteps + 1;
      }
      else
      {
        // Reset all inputs to outputs
        if(addressSteps == 0)
        {
          //Serial.print("SET");
          //Serial.print("\n");
          
          //REG_PIOD_OWER = 0x410;
          //REG_PIOD_OWER = 0x1;
          
          pinMode(11, INPUT);
          pinMode(12, INPUT);
          for(int i = 25; i < 31; i++)
          {
            pinMode(i, INPUT);
          }
          
        }
        // Used to see if output is consistent with initial input
        correct = true;
        // Latch data
        //oldData = dataInt;
        // Read an address
        //Serial.println(toBinary((addressInt), 19));
        
        dataInt = readData(addressInt);
        // Compare the output
       
        // Check if the data is correct 
        correct = check(correctData, dataInt);
        //correct = check(oldData, 0);
        /*
        if(random(10) < 5)
        {
           correct = false;
        }
        */
        //correct = false;
       // Print out the address and received data if bad data read
        if(correct == false)
        { 
          Serial.print(correctData);
          Serial.print(" WAS READ AS ");
          Serial.println(dataInt);
          Serial.print(toBinary(dataInt, 10));
          Serial.print(" WAS READ AS: ");
          Serial.println(toBinary(correctData, 10));
          Serial.print("READING FROM ADDRESS: ");
          Serial.print(addressInt);
          Serial.print("\\");
          Serial.println(toBinary(addressInt, 19));
          Serial.print("THIS IS ADDRESS ");
          Serial.println(addressSteps); 
         // If the read-back data is incorrect read the address more
         for(int i = 0; i < readsAfterFailure; i++)
         {
           dataInt = readData(addressInt);
           correct = check(correctData, dataInt);
           if(correct == false)
           {
             netFalseReads = netFalseReads + 1;
           }
         }
         // Append a 'U' for single event upset and 'L' for latch up  
         //Serial.print(addressSteps);
         //Serial.print (" ");
         //Serial.print(addressSteps);
         //Serial.print("  ");
         //Serial.println(netFalseReads);
         //output1 = addressSteps >> 8;
         //output2 = addressSteps & 0xFF;
         //output3 = netFalseReads;
         //Serial.print(byte(output1));
         //Serial.print(byte(output2));
         //Serial.print(byte(output3));
         //Serial.print("EC");
         //Serial.println(micros());
        }
        /*
        else
        {
          Serial.print(addressSteps);
          Serial.println(" GOOD!");
          Serial.print("\n");
        }
        */
        netFalseReads = 0;
        //increment the address
        addressSteps = addressSteps + 1;
        if((addressInt & ADDRESS_MASK) < (ADDRESS_MASK - 1))
        {
          addressInt = addressInt + 2;
        }
        else
        {
          addressInt = addressInt + 3074;
        }
      }
     }
    // Way to tell if finished running through addresses
    else if(addressSteps == NUM_ADDRESSES)
    {
      if(writing)
      {
        writing = false;
        addressSteps = 0;
        addressInt = 0;
        delay(delayAfterWrite);
      }
      else
      {
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
boolean check(int oldData, int data)
{
  return (oldData == data);
}

void getData()
{
  boolean done = false;
  while(!done)
  {
    if (Serial.available())
    {
      done = processIncomingByte (Serial.read());
    }
  }
}

void handlePreviousState()
{
  switch (currentState)
  {
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


boolean processIncomingByte (const byte c)
{
  if (isdigit(c))
  {
    currentValue = currentValue * 10;
    currentValue = currentValue + c - '0';
    return false;
  }
  else
  {
    handlePreviousState();
    switch (c)
    {
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
  
void establishContact()
{
   do
   {
     Serial.print('$');
     delay(10);
   }
   while(Serial.available() <= 0);   
   Serial.read();
}
  
// Read a byte of data given an address and an array of data
int readData(int address)
{
  int data = 0;
  int holder = 0;
  //Serial.print("READ TEST: ");
  //Serial.println(toBinary(address, 19));
  digitalWrite(WR, HIGH);
  delayMicroseconds(1);
  REG_PIOC_ODSR = address;
  delayMicroseconds(1);
  digitalWrite(CS, LOW);
  delayMicroseconds(1);
  digitalWrite(OE, LOW);
  //delayMicroseconds(1);
  /*
  Serial.print("ADDRESS: ");
  Serial.println(reverseAddress(address));
  Serial.print("READ: ");
  */
  //data = PIOD->PIO_PDSR & 1023;

  data = (PIOD->PIO_PDSR & (0b1111 << 6)) + (PIOD->PIO_PDSR & 0b1111);;
  /*
  if(data != 975)
  {
    Serial.print("TEST READ: ");
    Serial.println(toBinary(data, 10));
  }
  */
  delayMicroseconds(1);
  //data = PIOD->PIO_PDSR;
  /*
  data = REG_PIOD_PDSR;
  Serial.print(analogRead(A0) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A1) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A2) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A3) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A4) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A5) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A6) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A8) * 3.3 / 1023.0);
  Serial.print(" ");  
  Serial.print("\n");
  Serial.print("TEST: ");
   */
  //data = REG_PIOD_PDSR; 
  /*
  Serial.println(toBinary(data, 32));  
  Serial.print("ANOTHER TEST: ");
  Serial.print((PIOD->PIO_PDSR & (0b1111 << 6)));
  Serial.print(" ");
  Serial.print((PIOD->PIO_PDSR & 0b1111));
  Serial.println(" ");
  */
  /*
  Serial.print(!!(PIOD->PIO_PDSR & (1<<7)));
  Serial.print(" ");
  Serial.print(!!(PIOD->PIO_PDSR & (1<<6)));
  Serial.print(" ");
  Serial.print(!!(PIOD->PIO_PDSR & (1<<3)));
  Serial.print(" ");
  Serial.print(!!(PIOD->PIO_PDSR & (1<<2)));
  Serial.print(" ");
  Serial.print(!!(PIOD->PIO_PDSR & (1<<1)));
  Serial.print(" ");
  Serial.print(!!(PIOD->PIO_PDSR & 1));
  Serial.println(" ");
  */
  
  digitalWrite(OE, HIGH);
  delayMicroseconds(1);
  digitalWrite(CS, HIGH);
  delayMicroseconds(1);
  return data;
 }

// Write a byte of data
void writeData(int address, int data)
{
  digitalWrite(OE, HIGH);
  delayMicroseconds(1);
  REG_PIOC_ODSR = address;
  delayMicroseconds(1);
  //REG_PIOD_ODSR = data;
  //REG_PIOD_ODSR = 0x0710;
   /*
   pinMode(11, INPUT);
   pinMode(12, INPUT);
   for(int i = 25; i < 31; i++)
   {
     pinMode(i, INPUT);
   }
   */
  REG_PIOD_ODSR = data;
   
  /*
  Serial.print(analogRead(A0) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A1) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A2) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A3) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A4) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A5) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A6) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print(analogRead(A8) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.println(toBinary(data, 10));  
  Serial.print("\n");
  */
  
  delayMicroseconds(1);
  digitalWrite(CS, LOW);
  delayMicroseconds(1);
  digitalWrite(WR, LOW);
  delayMicroseconds(1);
  digitalWrite(CS, HIGH);
  delayMicroseconds(1);
  digitalWrite(WR, HIGH);
  delayMicroseconds(1);
}

String toBinary(unsigned int val, int length)
{ 
  String holder = "";
  unsigned int mask = 1 << length - 1;
  
  for(int i = 0; i < length; i++)
  {
     if( (val & mask) == 0)
     {
       holder += "0";
     }
     else
     {
       holder += "1"; 
     }
     // Bitshift the mask over one character
     mask >>= 1;
  }
  return holder;   
}



