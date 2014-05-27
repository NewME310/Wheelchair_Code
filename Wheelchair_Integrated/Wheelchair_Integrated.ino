/**
*  ME 310 - Team Embraer '13-'14
*  Code for the Wheelchair-side system: integrated RFID and accelerometer
*  Accelerometer code adapted from https://www.sparkfun.com/tutorials/240
*  RFID code adapted from SparkFun code by Maria Barrera:
  RFID Eval 13.56MHz Shield example sketch v10
 
 Aaron Weiss, aaron at sparkfun dot com
 OSHW license: http://freedomdefined.org/OSHW
 
 works with 13.56MHz MiFare 1k tags 
*  Cliff Bargar, 4/25/14
*/

//wiring
//Accelerometer http://www.inmotion.pt/documentation/others/images/adxl345_to_arduino_uno.png
//SDA pin -> A4 (Uno) (Pull-up resistor to 3.3V)
//SCL pin -> A5 (Uno) (Pull-up resistor to 3.3V)
//CS pin -> 3.3V
//VDD -> 3.3V
//GND -> GND
//RFID
// Based on hardware v13:
// D7 -> RFID RX
// D8 -> RFID TX
// D9 -> XBee TX
// D10 -> XBee RX


//********** includes **********
//Add the SPI library so we can communicate with the ADXL345 sensor
//#include <SPI.h>
#include <Wire.h>
#include "ADXL345.h"
#include <SoftwareSerial.h>


//********** defines **********
//***Accel***
//Free Fall threshold: 300m(g) to 600m(g) recommended
#define FALL_THRESH 0x09 //recommended value                                    <-- needs to be adjusted
//Free Fall duration: minimum time for free fall, 100ms to 350ms recommended
#define FALL_DUR 0x14 //350ms                                                  <-- needs to be adjusted
//Shock threshold: minimum acceleration value set in THRESH_TAP register, 62.5m(g)/LSB
#define SHOCK_THRESH 0xE0 //arbitrary                                           <-- needs to be adjusted
//Shock duration: maximum time that an event exceeds SHOCK_THRESH to qualify as a tap
#define SHOCK_DUR 20 //12.5 ms                                                  <-- needs to be adjusted

#define CS 10 //Assign the Chip Select signal to pin 10.

//interrupt pins on Uno:
#define SHOCK_PIN 2 //interrupt for shock on pin 2 (int.0) - INT2 on ADXL345
#define SHOCK_ISR 0
#define FF_PIN 3 //interrupt for free fall on pin 3 (int.1) - INT1 on ADXL345
#define FF_ISR 1

#define DEBUG 1
#define debugPrint if(DEBUG) Serial.println

//***RFID***
#define INDICATOR_PIN         13   //Pin for indicator LED
#define INDICATOR_DURATION    700  //duration of indicator

//last 2 digits of their serial number
#define ALBERT 0x9A
#define BOB 0xD0
#define CHARLES 0x5A
#define DIANA 0xC0
#define ELTON 0x00

//********** module variables **********
//***Accel***
//This buffer will hold values read from the ADXL345 registers.
int values[3];
char output[20];

int loopCount = 0;
ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

//These variables will be used to hold the x,y and z axis accelerometer values.
int x,y,z;
double xg, yg, zg;
char tapType=0;

boolean shockFlag = 0;
boolean fallFlag = 0;

//***RFID***
SoftwareSerial rfid(7, 8);
SoftwareSerial xbee(10, 9);
unsigned long indicator_time = -1;
int flag = 0;
int Str1[11];

//Prototypes
//***Accel***
void initAccel(void);
void tap(void);
void ff_int_response(void);
void shock_int_response(void);
//***RFID***
//void check_for_notag(void);
void halt(void);
void parse(void);
void print_serial(void);
void read_serial(void);
void seek(void);
void set_flag(void);

//********** functions **********
//Arduino setup()
void setup(){ 
  
  pinMode(13,OUTPUT);
  
  //Create a serial connection to display the data on the terminal.
  Serial.begin(9600);
  
  //RFID init
  xbee.begin(9600);
  rfid.begin(19200);
  while(!Serial){
    // set the data rate for the SoftwareSerial ports
    ;
  }
  pinMode(INDICATOR_PIN,OUTPUT);
  digitalWrite(INDICATOR_PIN,LOW);

  delay(10);
  //halt();
  
  debugPrint("ME 310 Stanford-Embraer Code Initialization Begun");
  
  //initialize accerelometer
  initAccel();
  
  //initialize XBee?
 
}

//Arduino main loop()
void loop(){  
  
  //RFID
  read_serial();
  checkIndicator();


  //ACCEL
  
  //Reading data:  
  //Reading 6 bytes of data starting at register DATAX0 will retrieve the x,y and z acceleration values from the ADXL345.
  //The results of the read operation will get stored to the values[] buffer.
  adxl.readAccel(&values[0], &values[1], &values[2]);
  //The ADXL345 gives 10-bit acceleration values, but they are stored as bytes (8-bits). To get the full value, two bytes must be combined for each axis.
  //The X value is stored in values[0] and values[1].
  x = values[0];
  //The Y value is stored in values[2] and values[3].
  y = values[1];
  //The Z value is stored in values[4] and values[5].
  z = values[2];
    
  Serial.print(x, DEC);
  Serial.print(',');
  Serial.print(y, DEC);
  Serial.print(',');
  Serial.println(z, DEC);
    
  //Serial.println(sqrt(x*x + y*y + z*z));

  if(shockFlag == 1)
  {
    Serial.println("Shock experienced!");
    shockFlag = 0;
    adxl.getInterruptSource();
    //digitalWrite(13,LOW);
  }
  if(fallFlag == 1)
  {
    Serial.println("Fall experienced!");
    fallFlag = 0;
    adxl.getInterruptSource();
    //digitalWrite(13,LOW);
  }

  delay(100); 
}

//Accel helper functions
void initAccel()
{
   //Set up the Chip Select pin to be an output from the Arduino.
  pinMode(CS, OUTPUT);
  //Before communication starts, the Chip Select pin needs to be set high.
  digitalWrite(CS, HIGH);
  
  adxl.powerOn();
  adxl.setRangeSetting(8); //16g range
  //set rate to 3200Hz
  adxl.setRate(1600); //sets rate bits of BW_RATE register to 0x0F
  
  //disable all interrupts
  adxl.writeTo(ADXL345_INT_ENABLE, 0x00);
    
  //setup free fall interrupt:
  //set ff threshold
  adxl.setFreeFallThreshold(FALL_THRESH);
  //set ff duration
  adxl.setFreeFallDuration(FALL_DUR);
  
  //setup single-tap (shock) interrupt:
  //set tap threshold
  adxl.setTapThreshold(SHOCK_THRESH);
  //set tap duration
  adxl.setTapDuration(SHOCK_DUR);
  //set X direction
  adxl.setTapDetectionOnX(HIGH);
  //set Y direction
  adxl.setTapDetectionOnY(HIGH);
  //set Z direction
  adxl.setTapDetectionOnZ(HIGH);
  
  //Map Free Fall to pin 1 (clear --> INT1)
  adxl.setInterruptMapping(ADXL345_INT_FREE_FALL_BIT, ADXL345_INT1_PIN);
  //Map Shock to pin 2 (set --> INT22)
  adxl.setInterruptMapping(ADXL345_INT_SINGLE_TAP_BIT, ADXL345_INT2_PIN);
  
  
  //set free-fall bit
  adxl.setRegisterBit(ADXL345_INT_ENABLE, ADXL345_INT_FREE_FALL_BIT, HIGH);
  //set single-tap bit (shock)
  adxl.setRegisterBit(ADXL345_INT_ENABLE, ADXL345_INT_SINGLE_TAP_BIT, HIGH);
  
  //attach interrupts on rising edges
  attachInterrupt(SHOCK_ISR,shock_int_response,RISING);
  attachInterrupt(FF_ISR,ff_int_response,RISING);
  
  debugPrint("Accelerometer Initialized");
}

void tap(void){
  //Clear the interrupts on the ADXL345
  //readRegister(INT_SOURCE, 1, values); 
  if(values[0] & (1<<5))tapType=2;
  else tapType=1;;
}

//interrupt response for free fall
void ff_int_response()
{
    //flash LED
    //digitalWrite(13,HIGH);
    //code to read INT_SOURCE (?)
    fallFlag = 1;  
    indicatorOn();
}

//interrupt response for shock
void shock_int_response()
{
    //flash LED
    //digitalWrite(13,HIGH);
    //code to read INT_SOURCE (?)
    shockFlag = 1;   
    indicatorOn();
}

//RFID helper functions
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
//    print_handler();
 
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

void print_handler()
{
  if (Str1[5]==ALBERT)
  {
    Serial.println("Albert is now handling your wheelchair");
  } 
  else if (Str1[5]==BOB)
  {
    Serial.println("Bob is now handling your wheelchair");
  } 
  else if (Str1[5]==CHARLES)
  {
    Serial.println("Charles is now handling your wheelchair");
  } 
  else if (Str1[5]==DIANA)
  {
    Serial.println("Diana is now handling your wheelchair");
  } 
  else if (Str1[5]==ELTON)
  {
    Serial.println("Elton is now handling your wheelchair");
  } 
  else 
  {
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

