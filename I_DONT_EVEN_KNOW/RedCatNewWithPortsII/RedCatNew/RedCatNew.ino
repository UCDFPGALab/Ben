//Various pins on the RedCat chip - Note: Ground and Vcc not included

const int CS = 51; // Chip select
const int OE = 52; // Output enable''
const int WR = 53; // Write enable
const int FIRST_ADDRESS = 22;
const int FIRST_IO = 40;

const int NUM_ADDRESSES = 64000;

const int NUM_SET_CODES = 4;

const char TERMINATION = '$';


typedef enum {NONE, GOT_DATA, GOT_NUM_RUNS, GOT_DELAY, GOT_REPEAT_READS} states;
states currentState = NONE;
unsigned int currentValue = 0;


const char SET_CODES[] = {'a', 'b', 'c', 'd'};

// Data and address integers
int dataInt = 170;
int addressInt = 0;
int data[8];
int address[16];
int oldData[8];
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

  // Set modes of IO
  pinMode(CS, OUTPUT);
  pinMode(OE,OUTPUT);
  pinMode(WR, OUTPUT);
  
  for(int i = FIRST_ADDRESS; i < FIRST_ADDRESS + 16; i++)
  {
    pinMode(FIRST_ADDRESS, OUTPUT);
  }
    
  
  digitalWrite(CS, HIGH);
  // 0 = input
  // 1 = output
  
  /* Requirement: PC30 has to be output
     For addresses: Use PC1, PC2,  PC3,  PC4,  PC5,  PC6,  PC7,  PC8
                        PC9, PC12, PC13, PC14, PC15, PC16, PC17, PC18
     All other addresses are irrelevant. Also, least significant digit
     is on the right
  */
  
  //DDRC = DDRC | 0b0111111111111111111111111111111;
 
   
  
  // Initialize serial connection
  Serial.begin(115200);
  randomSeed(analogRead(5));
  establishContact();
  getData();
  Serial.print("DATA: ");
  Serial.println(dataInt);
  Serial.print("NUMBER OF RUNS: ");
  Serial.println(numRuns);
  Serial.print("DELAY AFTER WRITING (ms): ");
  Serial.print(delayAfterWrite);
  Serial.print("READS AFTER FAILURE: ");
  Serial.println(readsAfterFailure);
}

void loop()
{
  // Check to make sure the correct number of runs are being done
  if(currentRun < numRuns)
  {
    // Loop through all addresses then stop
    if(addressInt < NUM_ADDRESSES)
    {
      // Convert address to binary array
      convertToBinaryArray(16, addressInt, address);
      convertToBinaryArray(8, dataInt, data);
  
      // Write and then read
      if(writing)
      {
        if(addressInt == 0)
        {
          for(int i = 0; i < 8; i++)
          {
             pinMode(i + FIRST_IO, OUTPUT); 
          }
        }
        writeData(address, data);
        /* PD5 must be input
           Using pins: PD0, PD1, PD2, PD3
                       PD6, PD7, PD8, PD9
        */
        //DDRD = DDRD | B111111001111
        //writeData(address, data);
        addressInt = addressInt + 1;
      }
      else
      {
        // Reset all inputs to outputs
        if(addressInt == 0)
        {
          Serial.print("SET");
          Serial.print("\n");
          for(int i = 0; i < 8; i++)
          {
            pinMode(i + FIRST_IO, INPUT);
          }
        }
        // Used to see if output is consistent with initial input
        correct = true;
        // Latch data
        for(int i = 0; i < 8; i++)
        {
          oldData[i] = data[i];
        }
    
        // Read an address
        readData(address, data);
        delay(2000);
        // Compare the output
       
        // Check if the data is correct 
        correct = check(oldData, data);


       // Print out the address and received data if bad data read
        if(correct == false)
        { 
         // If the read-back data is incorrect read the address more
         for(int i = 0; i < readsAfterFailure; i++)
         {
           readData(address, data);
           correct = check(oldData, data);
           if(correct == false)
           {
             netFalseReads = netFalseReads + 1;
           }
         }
       
         // Append a 'U' for single event upset and 'L' for latch up  
         Serial.print(addressInt);
         Serial.print (" ");
         if(netFalseReads < readsAfterFailure)
         {
           Serial.print(netFalseReads);
           Serial.print("U\n");
         }
         else
         {
           Serial.print(netFalseReads);
           Serial.print("L\n");
         }
         netFalseReads = 0;
        }
        else
        {
          Serial.print(addressInt);
          Serial.print("\n");
        }
        //increment the address
        addressInt = addressInt + 1;
        correct = true;
      }
    }
    // Way to tell if finished running through addresses
    else if(addressInt == NUM_ADDRESSES)
    {
      if(writing)
      {
        writing = false;
        addressInt = 0;
        delay(delayAfterWrite);
      }
      else
      {
        Serial.print("DONE\n");
        currentRun = currentRun + 1;
        addressInt = 0;
      }
    }
  }
}

// Checks that the data is correct by comparing the two data arrays
boolean check(int oldData[], int data[])
{
  boolean check = true;
  for(int i = 0; i < 8; i++)
  {
    //int temp = random(0, 10000);
    //if(temp < 5 || addressInt == 0)
    //{
    //  data[i] = !data[i];
    //} 
    if(oldData[i] != data[i])
    {
      check = false; 
    }
  }
  Serial.print("\n");
  return check;
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
    dataInt = currentValue;
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
  
// Converts an integer to a binary array
void convertToBinaryArray(int length, int number, int output[])
{
  // For some reason, string conversion is not working, so make
  // method for binary conversion
 
  String aString = toBinary(unsigned(number), length);
  for(int i = 0; i < length; i++)
  {
    output[i] = (aString.charAt(i)) - '0';  
  }  
}


// Read a byte of data given an address and an array of data
void readData(int address[], int dataArray[])
{
  digitalWrite(WR, HIGH);
  for(int i = 0; i < 16; i++)
  {
    digitalWrite(i + FIRST_ADDRESS, address[i]);
  }
  digitalWrite(CS, LOW);
  digitalWrite(OE, LOW);
  //delayMicroseconds(1);
  Serial.print("READ: ");
  for(int i = 0; i < 8; i++)
  {
    dataArray[i] = digitalRead(i + FIRST_IO);
    Serial.print(digitalRead(i + FIRST_IO));
  }
  Serial.print("\n");
  Serial.print(analogRead(A0)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A1)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A2)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A3)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A4)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A5)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A8)*3.3/1024.0);
  Serial.print(" ");
  Serial.print(analogRead(A9)*3.3/1024.0);
  Serial.print("\n");
  digitalWrite(OE, HIGH);
  digitalWrite(CS, HIGH);
 }

// Write a byte of data
void writeData(int address[], int data[])
{
  digitalWrite(OE, HIGH);
  for(int i = 0; i < 16; i++)
  {
    digitalWrite(i + FIRST_ADDRESS, address[i]);
  }
  for(int i = 0; i < 8; i++)
  {
    digitalWrite(i + FIRST_IO, data[i]);
  }
  digitalWrite(CS, LOW);
  digitalWrite(WR, LOW);
  //delayMicroseconds(1);
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
}


