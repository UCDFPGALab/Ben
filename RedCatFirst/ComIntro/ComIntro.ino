typedef enum {NONE, GOT_DATA, GOT_RUNS, GOT_WAIT, GOT_AGAIN} states;
int data, runs, wait, again;  
states currentState = NONE;
unsigned int currentValue = 0;

void setup() 
{
  Serial.begin(115200);
  getData();
}


void loop() 
{
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
    data = currentValue;
    Serial.print("Y");
    break;
  case GOT_RUNS:
    runs = currentValue;
    Serial.print("Y");
    break;
  case GOT_WAIT:
    wait = currentValue;
    Serial.print("Y");
    break;
  case GOT_AGAIN:
    again = currentValue;
    Serial.print("Y");
    break;
  }
  currentValue = 0;
}

boolean processIncomingByte (const byte c)
{
  if (isdigit(c))
  {
    currentValue = currentValue * 10;
    currentValue = currentValue + 'c' - '0';
    return false;
  }
  else
  {
    handlePreviousState();
    switch (c)
    {
    case 'D':
      currentState = GOT_DATA;
      return false;
      break;
    case 'R':
      currentState = GOT_RUNS;
      return false;
      break;
    case 'W':
      currentState = GOT_WAIT;
      return false;
      break;
    case 'A':
      return false;
      currentState = GOT_AGAIN;
      break;
    case '$':
      return true;
      break;
    default:
      currentState = NONE;
      return false;
      break;
    } 
  } 
}
