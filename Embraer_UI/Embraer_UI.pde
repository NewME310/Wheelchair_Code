//ME 310 Embraer UI for wheelchair platform data
  
PFont f;
PImage logo;

String lastHandler = "Handler";
String lastTime = "Time";
 
import processing.serial.*;
 
Serial myPort;        // The serial port  

void setup() {
  size(1280, 720);
  
  logo = loadImage("logo.png");
  
  // Create the font
  //printArray(PFont.list());
  f = createFont("Myriad", 24);
  textFont(f);
  
   myPort = new Serial(this, Serial.list()[0], 9600);
   // don't generate a serialEvent() unless you get a newline character:
   //myPort.bufferUntil('\n');
   print("Initializing");
}

void draw() {
  background(255);
  image(logo,-15,-15,1181,236);
  textAlign(LEFT);
  fill(6,75,160);
  textFont(f,40);
  text("Platform",800,100);
  text("Status",800,150);
  
  //RFID notice
  textAlign(LEFT);
  fill(6,75,160);
  textFont(f,40);
  text("Wheelchair last handled by:",100,300);
  fill(255,0,0);
  textFont(f,44);
  text(lastHandler,100,360);
  fill(6,75,160);
  textFont(f,40);
  text(lastTime,100,420);
//  while(true) ;
//  while(myPort.available() == 0 );
//  drawType(width * 0.33);
//  textAlign(LEFT);
//  drawType(width * 0.66);
}


void serialEvent (Serial myPort) 
 {
   // get the ASCII string:
   String inString = myPort.readStringUntil('\n');

   if (inString != null) 
   {
     print(inString);
     
     if(inString == "Luiz" || inString == "Rodrigo")
     {
       lastHandler = inString;
       lastTime = str(hour()) + ":" + str(minute()) + ":" + str(second()); 
//        lastTime = concat(concat(str(minute()),":"));
     }
    

   }
 }
