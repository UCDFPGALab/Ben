int holder = 0;

void setup() 
{
         
  /* 1 is output, 0 is input
     PC0 =  unconnected
     PC3 =  unconnected
     PC10 = unconnected
     PC11 = unconnected
     PC20 = unconnected
     PC26 = unconnected
     PC28 = unconnected
     PC29 = unconnected
     PC30 = LED "RX"
  */
  // Use for data - PC12 through PC19 (digital pins 44 through 51)  
  // 11111 11111 11111 11111 11111 11111 11111 1
  //REG_PIOC_OWER = 0xFFFFFFFFE;
  
  
  pinMode(44, OUTPUT);
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
  pinMode(47, OUTPUT);
  pinMode(48, OUTPUT);
  pinMode(49, OUTPUT);
  pinMode(50, OUTPUT);
  pinMode(51, OUTPUT);
  
  
  // 00000 00000 00101 01010 00000 00000 0
  REG_PIOC_ODSR = 479232;
  digitalWrite(44, LOW);
  Serial.begin(9600);
  //REG_PIOD_OWSR = (unsigned short)(65535);
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
  Serial.print(analogRead(A7) * 3.3 / 1023.0);
  Serial.print(" ");
  Serial.print("\n");
  pinMode(44, INPUT);
  pinMode(45, INPUT);
  pinMode(46, INPUT);
  pinMode(47, INPUT);
  pinMode(48, INPUT);
  pinMode(49, INPUT);
  pinMode(50, INPUT);
  pinMode(51, INPUT);
  int holder = REG_PIOC_PDSR;
  Serial.print("TEST: ");
  Serial.print(holder);
  Serial.print("\n"); 
}

void loop() 
{
   holder = REG_PIOC_PDSR;
   Serial.print("TEST: ");
   Serial.print(holder);
   Serial.print("\n");
   delay(1000);
   
}
