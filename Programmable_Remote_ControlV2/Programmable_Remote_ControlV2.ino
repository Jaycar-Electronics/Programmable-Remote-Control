#include <EEPROM.h>
#include <IRremote.h>
IRrecv irrecv(2);         //IR receiver on this pin
IRsend irsend;            //defaults to D3 on Uno
#define MATRIX1 10
#define MATRIX2 9
#define MATRIX3 8
#define MATRIX4 7
#define MATRIX5 6
#define MATRIX6 5
#define MATRIX7 4
//number of bytes per EEPROM record
#define BPRECORD 10

char keys[]="0123456789*#";     //key index for EEPROM lookup

void setup() {  
  char a;
  Serial.begin(115200);
  a=checkmatrix();
  if(a){learnkey(a);}         //wait for code and save to EEPROM if button pressed on reset
}

void loop() {
  char a;
  int d;
  a=checkmatrix();
  if(a){                        //key pressed, send code
    pinMode(13,OUTPUT);
    digitalWrite(13,HIGH);                //turn LED on
    Serial.println(a);
    sendcode(a);
    digitalWrite(13,LOW);                 //turn LED off
  }
  if(Serial.available()){       //use serial monitor to control learning
    d=Serial.read();
    if(index(keys,d)>=0){learnkey(d);}
  }
}

char checkmatrix(){                   //returns ASCII code of button pressed on button down, otherwise null
  static char lastkey=0;              //null aka no key- use ASCII encoding here
  char key=0;                         //assume nothing pressed
  pinMode(MATRIX1,INPUT_PULLUP);
  pinMode(MATRIX2,INPUT_PULLUP);
  pinMode(MATRIX3,INPUT_PULLUP);
  pinMode(MATRIX4,INPUT_PULLUP);
  pinMode(MATRIX5,INPUT_PULLUP);
  pinMode(MATRIX6,INPUT_PULLUP);
  pinMode(MATRIX7,INPUT_PULLUP);

  pinMode(MATRIX1,OUTPUT);            //scan middle column
  digitalWrite(MATRIX1,LOW);
  delay(1);
  if(!digitalRead(MATRIX2)){key='2';}
  if(!digitalRead(MATRIX7)){key='5';}
  if(!digitalRead(MATRIX6)){key='8';}
  if(!digitalRead(MATRIX4)){key='0';}
  pinMode(MATRIX1,INPUT_PULLUP);

  pinMode(MATRIX3,OUTPUT);            //scan left column
  digitalWrite(MATRIX3,LOW);
  delay(1);
  if(!digitalRead(MATRIX2)){key='1';}
  if(!digitalRead(MATRIX7)){key='4';}
  if(!digitalRead(MATRIX6)){key='7';}
  if(!digitalRead(MATRIX4)){key='*';}
  pinMode(MATRIX3,INPUT_PULLUP);

  pinMode(MATRIX5,OUTPUT);            //scan right column
  digitalWrite(MATRIX5,LOW);
  delay(1);
  if(!digitalRead(MATRIX2)){key='3';}
  if(!digitalRead(MATRIX7)){key='6';}
  if(!digitalRead(MATRIX6)){key='9';}
  if(!digitalRead(MATRIX4)){key='#';}
  pinMode(MATRIX5,INPUT_PULLUP);

  if((!lastkey)&&key){                //on key down
    lastkey=key;
    return key;
  }
  lastkey=key;
  return 0;
}

void learnkey(char a){  
  int eadd;
  eadd=index(keys,a)*BPRECORD;
  Serial.println(eadd);
  if(eadd<0){return;}                    //validate
  decode_results results;                // Somewhere to store the results
  Serial.print("Learning:");
  Serial.println(a);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);                //turn LED on
  irrecv.enableIRIn();                  //Start the receiver
  irrecv.resume();                      //Prepare for the next value
  while(!irrecv.decode(&results)){}     //wait until we get a code
  Serial.print("Type  :0x");
  Serial.println((unsigned int)results.decode_type,HEX);
  Serial.print("Adress:0x");
  Serial.println((unsigned int)results.address,HEX);
  Serial.print("Data  :0x");
  Serial.println((unsigned long)results.value,HEX);
  Serial.print("Bits  :0x");
  Serial.println((unsigned int)results.bits,HEX);
  Serial.println();
  EEPROM.put(eadd,(unsigned int)results.decode_type);
  EEPROM.put(eadd+2,(unsigned int)results.address);
  EEPROM.put(eadd+4,(unsigned long)results.value);
  EEPROM.put(eadd+8,(unsigned int)results.bits);  
  digitalWrite(13,LOW);                 //turn LED off
}

int index(char* a,char b){      //find b in a
  for(int i=0;i<strlen(a);i++){
    if(a[i]==b){return i;}
  }
  return -1;
}

void sendcode(char k){        //send code based on key
  int eadd;
  unsigned int t;             //data for sendIR function
  unsigned int a;
  unsigned long d;
  unsigned int b;
  eadd=index(keys,k)*BPRECORD;
  Serial.println(eadd);
  if(eadd<0){return;}                    //validate
  EEPROM.get(eadd,t);
  EEPROM.get(eadd+2,a);
  EEPROM.get(eadd+4,d);
  EEPROM.get(eadd+8,b);
  Serial.print("Type  :0x");
  Serial.println(t,HEX);
  Serial.print("Adress:0x");
  Serial.println(a,HEX);
  Serial.print("Data  :0x");
  Serial.println(d,HEX);
  Serial.print("Bits  :0x");
  Serial.println(b,HEX);
  sendIR(t,a,d,b);
}

void sendIR(unsigned int t, unsigned int a, unsigned long d, unsigned int b){       //send data d length b bits of type t to address a
  switch(t){
    case(RC5):          irsend.sendRC5(d,b);break;
    case(RC6):          irsend.sendRC6(d,b);break;
    case(NEC):          irsend.sendNEC(d,b);break;
    case(SONY):         irsend.sendSony(d,b);break;
    case(PANASONIC):    irsend.sendPanasonic(a,d);break;
    case(JVC):          irsend.sendJVC(d,b,0);break;
    case(SAMSUNG):      irsend.sendSAMSUNG(d,b);break;
    case(WHYNTER):      irsend.sendWhynter(d,b);break;
  }
}

