/**
 * Words. 
 * 
 * The text() function is used for writing words to the screen.
 * The letters can be aligned left, center, or right with the 
 * textAlign() function. 
 */
  
PFont f;
PImage logo;
 
import processing.serial.*;
 
Serial myPort;        // The serial port  

void setup() {
  size(1280, 720);
  
  logo = loadImage("logo.png");
  
  // Create the font
  //printArray(PFont.list());
  f = createFont("Myriad", 24);
  textFont(f);
  
  // myPort = new Serial(this, Serial.list()[0], 9600);
   // don't generate a serialEvent() unless you get a newline character:
   //myPort.bufferUntil('\n');
}

void draw() {
  background(255);
  image(logo,0,0,1575,314);
  textAlign(RIGHT);
  drawType(width * 0.33);
  textAlign(LEFT);
  drawType(width * 0.66);
}

void drawType(float x) {
  fill(0);
  text("ichi", x, 95);
  fill(51);
  text("ni", x, 130);
  fill(204);
  text("san", x, 165);
  fill(255);
  text("shi", x, 210);
}

//void serialEvent (Serial myPort) 
// {
//   // get the ASCII string:
//   String inString = myPort.readStringUntil('\n');
//   String[] str_arr;
//   String str1, str2;
//   
//   if (inString != null) 
//   {
//     // trim off any whitespace:
//     print("original " + inString);
//     str_arr = split(inString,'\t');
//     str1 = str_arr[0];
//     str2 = str_arr[1];
//     println("\t 1 " + str1 + "\t 2 " + str2);
//     // convert to an int and map to the screen height:
//     float inByte1 = float(str1); 
//     inByte1 = map(inByte1, -.1, .1, 0, height/2);
//     float inByte2 = float(str2); 
//     inByte2 = map(inByte2, -.1, .1, 0, height/2);
//     
//     // draw the line:
//     stroke(127,34,255);
//     line(xPos, height, xPos, height - inByte2);
//     line(xPos, height/2, xPos, height/2 - inByte1);
//   
//     // at the edge of the screen, go back to the beginning:
//     if (xPos >= width) 
//     {
//       xPos = 0;
//       background(0); 
//     } 
//     else 
//     {
//       // increment the horizontal position:
//       xPos++;
//     }
//   }
// }
