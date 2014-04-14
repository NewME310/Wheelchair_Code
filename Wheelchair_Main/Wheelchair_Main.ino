/**
*  ME 310 - Team Embraer '13-'14
*  Code for the Wheelchair-side system
*  Accelerometer code adapted from https://www.sparkfun.com/tutorials/240
*  Cliff Bargar, 4/12/14
*/

//********** includes **********
//Add the SPI library so we can communicate with the ADXL345 sensor
//#include <SPI.h>
#include <Wire.h>
#include "ADXL345.h"
//wiring: http://www.inmotion.pt/documentation/others/images/adxl345_to_arduino_uno.png
//SDA pin -> A4 (Uno) or 2 (Leo) (Pull-up resistor to 3.3V)
//SCL pin -> A5 (Uno) or 3 (Leo) (Pull-up resistor to 3.3V)
//CS pin -> 3.3V
//VDD -> 3.3V
//GND -> GND

//********** defines **********
//Free Fall threshold: 300m(g) to 600m(g) recommended
#define FALL_THRESH 0x09 //recommended value                                    <-- needs to be adjusted
//Free Fall duration: minimum time for free fall, 100ms to 350ms recommended
#define FALL_DUR 0x46 //350ms                                                  <-- needs to be adjusted
//Shock threshold: minimum acceleration value set in THRESH_TAP register, 62.5m(g)/LSB
#define SHOCK_THRESH 127 //half of 0xFF?                                         <-- needs to be adjusted
//Shock duration: maximum time that an event exceeds SHOCK_THRESH to qualify as a tap
#define SHOCK_DUR 20 //12.5 ms                                                  <-- needs to be adjusted

#define CS 10 //Assign the Chip Select signal to pin 10.

//interrupt pins on Leonardo:
#define SHOCK_PIN 0 //interrupt for shock on pin 0 (int.2) - INT2 on ADXL345
#define SHOCK_ISR 2
#define FF_PIN 1 //interrupt for free fall on pin 1 (int.3) - INT1 on ADXL345
#define FF_ISR 3

#define DEBUG 1
#define LEO 1
#define debugPrint if(DEBUG) Serial.println

//********** module variables **********
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

//********** functions **********
//Arduino setup()
void setup(){ 
  
  pinMode(13,OUTPUT);
  
  //Create a serial connection to display the data on the terminal.
  Serial.begin(9600);
  
  //delay printing debug statements until Serial line opened
  if(LEO & DEBUG)
    while (!Serial)
    {
      //flashes onboard LED while waiting for serial
      digitalWrite(13,HIGH);
      delay(50);
      digitalWrite(13,LOW);
      delay(50);
    }
  debugPrint("ME 310 Stanford-Embraer Code Initialization Begun");
  
  //initialize accerelometer
  initAccel();
  
  //initialize RFID
  
  //initialize XBee?
 
}

//Arduino main loop()
void loop(){  
  

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
    digitalWrite(13,LOW);
  }
  if(fallFlag == 1)
  {
    Serial.println("Fall experienced!");
    fallFlag = 0;
    adxl.getInterruptSource();
    digitalWrite(13,LOW);
  }

  delay(100); 
}

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
    digitalWrite(13,HIGH);
    //code to read INT_SOURCE (?)
    fallFlag = 1;  
}

//interrupt response for shock
void shock_int_response()
{
    //flash LED
    digitalWrite(13,HIGH);
    //code to read INT_SOURCE (?)
    shockFlag = 1;  
}


