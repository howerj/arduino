// https://forum.arduino.cc/index.php?topic=72057.0
// https://www.youtube.com/watch?v=-Ul2j6ixbmE
int BITwidth = 5000; //bit width in micros (max tested 500 (not so reliable)
int analogReadTime = 100; //acquistion time of analogRead function (from arduino website, not sure)
float BITstart = 0;
float time = 0;
int BITValue = 0;
int BITsReceived[8];
int ambient = 0;
int ambientIndex = 0;
int possibleInput = 138;
int input = 0;
int BITspacerWidth = 3.5;
int BITspacer = BITwidth * BITspacerWidth;
int BITspacerMiddlePoint = (BITspacer / 2) - (analogReadTime / 2);
int BITmiddlePoint = (BITwidth / 2) - (analogReadTime / 2);
int acquisitionWindow = BITspacer * 1.1;
int presumption = 150;

void setup() {                
pinMode(12, OUTPUT);
for(int i = 0; i < 10; i++) { ambient = analogRead(0) + ambient; }
Serial.begin(115200);
}

// ACTIVE MODE///////////////////////////////////////////////////////////////
void startCOM() {
BITstart = micros();
while((micros() - BITstart) < BITspacer) { digitalWrite(12, HIGH);  }
}

void endCOM() {
BITstart = micros();
while((micros() - BITstart) < BITspacer) { digitalWrite(12, LOW); }
}

void BIT(int b) {
if(b == 1) {  
 BITstart = micros();
 while((micros() - BITstart) < BITwidth) {
  digitalWrite(12, HIGH);
 }
}
if(b == 0) {
 BITstart = micros();
 while((micros() - BITstart) < BITwidth) {
  digitalWrite(12, LOW);
 }
}
}

void printByte(char b) {
int BITcount = 8;
startCOM();
while(BITcount >= 0) {
int value = bitRead(b, BITcount);
if(value <= 1) {BIT(value); BITcount--;}
}
endCOM();
}

// PASSIVE MODE//////////////////////////////////////////////////////////////

void getBYTE() {
 time = micros();
 presumption = (possibleInput - ambient) / 5;
while(analogRead(0) > 150) { }
 if((micros() - time >= BITspacerMiddlePoint) && (micros() - time) <= acquisitionWindow) { possibleInput = analogRead(0); /*Serial.print(" "); Serial.print(possibleInput);*/ }         //non fare niente (da rivedere)
 if((micros() - time) >= BITspacer && (micros() - time) <= acquisitionWindow) {
  if( analogRead(0) <= 150) {
   getBIT();
   time = micros();  //presumibilmente avremo ottenuto il nostro byte
   ambient = 0;
   while(micros() - time <= BITspacerMiddlePoint) { //Nel periodo di 2.5bit LOW a fine comunicazione
    ambient = analogRead(0) + ambient;   //aggiorna la luce ambientale
    ambientIndex++;
   }
  }
  ambient = ambient / ambientIndex;
  /*Serial.print(ambient);
  Serial.print(" ");
  Serial.print(presumption);*/
  ambientIndex = 0;
 }
}


void getBIT() {
int BITcount = 8;
while(BITcount >= 0) {
 time = micros();
 while((micros() - time) <= BITmiddlePoint) {}
  input = analogRead(0);
 while((micros() - time) <= BITwidth) {}
  if(input > 150) { BITsReceived[BITcount] = 1; /*Serial.print(" ' "); Serial.print(analogRead(0)); Serial.print(" ' ");*/}
  if(input <= 150) { BITsReceived[BITcount] = 0; }
  /*Serial.print(" ");
  Serial.print(BITsReceived[BITcount]);*/
 
  BITcount--;
 }
 int somma = ((BITsReceived[7] * pow(2 , 7)) + (BITsReceived[6] * pow(2 , 6)) + (BITsReceived[5] * pow(2 , 5)) +
              (BITsReceived[4] * pow(2 , 4)) + (BITsReceived[3] * pow(2 , 3)) + (BITsReceived[2] * pow(2 , 2)) +
              (BITsReceived[1] * pow(2 , 1)) + (BITsReceived[0] * pow(2 , 0)));
 Serial.print(somma, BYTE);
 Serial.print(" ");
 if(somma == 41) { Serial.println(); }
}
//////////////////////////////////////////////////////////////////////////////

void loop() {
getBYTE();
/*printByte('B');
printByte('I');
printByte('-');
printByte('D');
printByte('I');
printByte('R');
printByte('E');
printByte('C');
printByte('T');
printByte('O');
printByte('N');
printByte('A');
printByte('L');
printByte(' ');
printByte('L');
printByte('E');
printByte('D');
printByte(' ');
printByte('C');
printByte('O');
printByte('M');
printByte(' ');
printByte('B');
printByte('Y');
printByte(' ');
printByte(' ');
printByte('G');
printByte('I');
printByte('O');
printByte('B');
printByte('L');
printByte('U');
printByte('.');
printByte('C');
printByte('O');
printByte('M');
printByte(' ');
printByte(':');
printByte(')');*/
}
