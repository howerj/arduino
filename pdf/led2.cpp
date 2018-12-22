// https://forum.arduino.cc/index.php?topic=72057.0
///LED COM
///BIDIRECTIONAL LED COMMUNICATION
///INVENTED BY GIOVANNI BLU MITOLO 2011
//          W W W . G I O B L U . C O M

////Bidirectional ANALOG LED communication by Giovanni Blu Mitolo is licensed
////under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//Based on a work at www.gioblu.com.
//int inputPIN = 12;        // + LED in analog 0 - GND LED in GND Arduino
int inputPIN = A0;        // + LED in analog 0 - GND LED in GND Arduino
int ENDstringValue = 41; //Decimal value of the carachter received that means END of stream / package ( in this case " ) " )
int BYTEvalue = 0;
int BITwidth = 2500; //bit width in micros (max tested 500) now 2500 60cm good
// 1000millis / (( (1.5bit  * 2500microseconds ) + (8bit * 2500microseconds) + (1.5bit  * 2500microseconds) ) / 1000conversion to millis) = 36bytes/s
int analogReadTime = 100; //acquistion time of analogRead function (from arduino website, not sure)
float time = 0;
int BITValue = 0;
int BITsReceived[8];
int ambient = 0;
int ambientIndex = 0;
int possibleInput = 138;
int input = 0;
int BITspacerWidth = 2.5; //1.5
int BITspacer = BITwidth * BITspacerWidth;
int BITspacerMiddlePoint = (BITspacer / 2) - (analogReadTime / 2);
int BITmiddlePoint = (BITwidth / 2) - (analogReadTime / 2);
int presumption = 150;

void setup() {               
pinMode(inputPIN, INPUT);
for(int i = 0; i < 10; i++) { ambient = analogRead(0) + ambient; }
Serial.begin(115200);
}

// ACTIVE MODE///////////////////////////////////////////////////////////////
void startCOM() { time = micros(); while((micros() - time) < BITspacer) {  pinMode(inputPIN, OUTPUT); digitalWrite(inputPIN, HIGH); } }
void endCOM() { time = micros(); while((micros() - time) < BITspacer) {  pinMode(inputPIN, OUTPUT);  digitalWrite(inputPIN, LOW); } }

void BIT(int b) {
if(b == 1) { time = micros(); while((micros() - time) < BITwidth) {  pinMode(inputPIN, OUTPUT);  digitalWrite(inputPIN, HIGH); } }
if(b == 0) { time = micros(); while((micros() - time) < BITwidth) {  pinMode(inputPIN, OUTPUT);  digitalWrite(inputPIN, LOW); } }
}

void printByte(char b) {
int BITcount = 8;
startCOM();
while(BITcount >= 0) { int value = bitRead(b, BITcount); if(value <= 1) {BIT(value); BITcount--;} }
endCOM();
}
// PASSIVE MODE//////////////////////////////////////////////////////////////
void getBYTE() {
  time = micros();
  pinMode(inputPIN, INPUT);
while(analogRead(inputPIN) > 150) { }
  if((micros() - time) >= BITspacer) {
   if( analogRead(inputPIN) <= 150) {
   getBIT();
   while((micros() - time) < BITspacer) { }
   }
  }
}


void getBIT() {
int BITcount = 8;
while(BITcount >= 0) {
  time = micros();
  while((micros() - time) <= BITmiddlePoint) {}
   input = analogRead(inputPIN);
  while((micros() - time) <= BITwidth) {}
   if(input > 150) { BITsReceived[BITcount] = 1; }
   if(input <= 150) { BITsReceived[BITcount] = 0; }
   BITcount--;
  }
  BYTEvalue = ((BITsReceived[7] * pow(2 , 7)) + (BITsReceived[6] * pow(2 , 6)) + (BITsReceived[5] * pow(2 , 5)) +
               (BITsReceived[4] * pow(2 , 4)) + (BITsReceived[3] * pow(2 , 3)) + (BITsReceived[2] * pow(2 , 2)) +
               (BITsReceived[1] * pow(2 , 1)) + (BITsReceived[0] * pow(2 , 0)));
if(BYTEvalue == ENDstringValue) { Serial.println(); }
  Serial.print(BYTEvalue, BYTE);
  Serial.print(" ");
}

//////////////////////////////////////////////////////////////////////////////

void loop() {
getBYTE();
//printByte('B');
//printByte('2');
//printByte('1');
//printByte(')');
}
