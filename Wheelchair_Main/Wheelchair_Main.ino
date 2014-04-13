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
//SDA pin -> Analog 4 (Pull-up resistor to 3.3V)
//SCL pin -> Analog 5 (Pull-up resistor to 3.3V)
//CS pin -> 3.3V
//VDD -> 3.3V
//GND -> GND

//********** defines **********
#define CS 10 //Assign the Chip Select signal to pin 10.

#define DEBUG 1
#define LEO 0
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
      digitalWrite(13,HIGH);
      delay(50);
      digitalWrite(13,LOW);
      delay(50);
    }
  debugPrint("ME 310 Stanford-Embraer Code Initialization Begun");
  
  //initialize accerelometer
  initAccel();
 
}

//Arduino main loop()
void loop(){  
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
  
//      Serial.println("SINGLE");
//      Serial.print(x);
//      Serial.print(',');
//      Serial.print(y);
//      Serial.print(',');
//      Serial.println(z);

  if(loopCount == 100)
  {
   loopCount = 0; 
      Serial.print(x, DEC);
      Serial.print(',');
      Serial.print(y, DEC);
      Serial.print(',');
      Serial.println(z, DEC);
  }


  loopCount++;
  delay(10); 
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
  
  
  debugPrint("Accelerometer Initialized");
}

void tap(void){
  //Clear the interrupts on the ADXL345
  //readRegister(INT_SOURCE, 1, values); 
  if(values[0] & (1<<5))tapType=2;
  else tapType=1;;
}

