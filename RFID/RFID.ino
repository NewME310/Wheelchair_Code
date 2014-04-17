/*
  RFID Eval 13.56MHz Shield example sketch v10
 
 Aaron Weiss, aaron at sparkfun dot com
 OSHW license: http://freedomdefined.org/OSHW
 
 works with 13.56MHz MiFare 1k tags
 
 Based on hardware v13:
 D7 -> RFID RX
 D8 -> RFID TX
 D9 -> XBee TX
 D10 -> XBee RX
 
 Note: RFID Reset attached to D13 (aka status LED)
 
 Note: be sure include the SoftwareSerial lib, http://arduiniana.org/libraries/newsoftserial/
 
 Usage: Sketch prints 'Start' and waits for a tag. When a tag is in range, the shield reads the tag,
 blinks the 'Found' LED and prints the serial number of the tag to the serial port
 and the XBee port.
 
 06/04/2013 - Modified for compatibility with Arudino 1.0. Seb Madgwick.
 a
 */
#include <SoftwareSerial.h>

SoftwareSerial rfid(7, 8);
SoftwareSerial xbee(10, 9);

//last 2 digits of their serial number
const int ALBERT=0x9A;
const int BOB=0xD0;
const int CHARLES=0x5A;
const int DIANA=0xC0;
const int ELTON=0x00;

#define INDICATOR_PIN         13   //Pin for indicator LED
#define INDICATOR_DURATION    700  //duration of indicator
unsigned long indicator_time = -1;

//Prototypes
//void check_for_notag(void);
void halt(void);
void parse(void);
void print_serial(void);
void read_serial(void);
void seek(void);
void set_flag(void);

//Global var
int flag = 0;
int Str1[11];

//INIT
void setup()
{
  Serial.begin(19200);
  xbee.begin(9600);
  rfid.begin(19200);
  while(!Serial){
    // set the data rate for the SoftwareSerial ports
    ;
  }

  pinMode(INDICATOR_PIN,OUTPUT);
  digitalWrite(INDICATOR_PIN,LOW);

  delay(10);
  halt();
  Serial.println("Start");
}

//MAIN
void loop()
{
  read_serial();
  checkIndicator();
}

//void check_for_notag()
//{
//  seek();
//  delay(10);
//  parse();
//  set_flag();
//
//  if(flag = 1){
//    seek();
//    delay(10);
//    parse();
//  }
//}

void halt()
{
  //Halt tag
  rfid.write((uint8_t)255);
  rfid.write((uint8_t)0);
  rfid.write((uint8_t)1);
  rfid.write((uint8_t)147);
  rfid.write((uint8_t)148);
}

void parse()
{
  //Serial.println("parse");
  while(rfid.available()){
    //Serial.println("available");
    if(rfid.read() == 255){
      for(int i=1;i<11;i++){
        Str1[i]= rfid.read();
      }
    }
  }
}

void print_serial()
{
  if(flag == 1){
    indicatorOn();
    //print to serial port
//    Serial.print(Str1[8], HEX);
//    Serial.print(Str1[7], HEX);
//    Serial.print(Str1[6], HEX);
//    Serial.print(Str1[5], HEX);
//    Serial.println();
    //tagid=Str1[8]+Str1[7]+Str1[6]+Str1[5]
    print_handler();
 
    //print to XBee module
    xbee.print(Str1[8], HEX);
    xbee.print(Str1[7], HEX);
    xbee.print(Str1[6], HEX);
    xbee.print(Str1[5], HEX);
    xbee.println();
    delay(100);
    //check_for_notag();
  }
}

void print_handler(){
  //Serial.println("handler");
  if (Str1[5]==ALBERT){
    Serial.println("Albert is now handling your wheelchair");
  } else if (Str1[5]==BOB){
    Serial.println("Bob is now handling your wheelchair");
  } else if (Str1[5]==CHARLES){
    Serial.println("Charles is now handling your wheelchair");
  } else if (Str1[5]==DIANA){
    Serial.println("Diana is now handling your wheelchair");
  } else if (Str1[5]==ELTON){
    Serial.println("Elton is now handling your wheelchair");
  } else {
    //do nothing?
  }
  //delay(1000); //so there arent repeat alerts
    
//  switch (Str1[8], HEX) {
//  case ALBERT:
//    Serial.println("Albert is now handling your wheelchair");
//    break;
//  case BOB:
//    Serial.println("Bob is now handling your wheelchair");
//    break;
//  case CHARLES:
//    Serial.println("Charles is now handling your wheelchair");
//    break;
//  case DIANA:
//    Serial.println("Diana is now handling your wheelchair");
//    break;
//  case ELTON:
//    Serial.println("Elton is now handling your wheelchair");
////    break;
  
}



void read_serial()
{
  seek();
  delay(10);
  parse();
  set_flag();
  print_serial();
  delay(100);
}

void seek()
{
  //search for RFID tag

  //Serial.println("seeking");

  rfid.write((uint8_t)255);
  rfid.write((uint8_t)0);
  rfid.write((uint8_t)1);
  rfid.write((uint8_t)130);
  rfid.write((uint8_t)131);
  delay(10);
}

void set_flag()
{
  if(Str1[2] == 6){
    flag++;
  }
  if(Str1[2] == 2){
    flag = 0;
  }
}

void indicatorOn()
{
  indicator_time = millis();
  digitalWrite(INDICATOR_PIN,HIGH);
}

//determine if indicator light should be turned off
void checkIndicator()
{
  if((unsigned long)(millis() - indicator_time) > INDICATOR_DURATION && indicator_time != -1)
    digitalWrite(INDICATOR_PIN,LOW);
}
