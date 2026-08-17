
#include <Ps3Controller.h>
#include <esp32-hal-ledc.h>
#include <Adafruit_NeoPixel.h>
#define PIN        5 // Our LEDs are hardwired to pin 18
#define NUMPIXELS 32 //We have 32 oneopixel lights. Four 8 LED bars
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define COUNT_LOW 1638
#define COUNT_HIGH 7864
#define TIMER_WIDTH 16

int posOne = 5000;
int posTwo = 6000;
int posThree = 5000;
int posFour = 4000;
int inOne = 12;
int inTwo = 13;
int inThree = 16;
int inFour = 17;
int inFive = 18;
int inSix = 19;
int inSeven = 22;
int inEight = 23;
int rX;
int rY;
int lX;
int lY;

void setup()
{
    Serial.begin(115200);
 //   Ps3.attach(notify);
    Ps3.begin("02:02:03:04:05:25");
    Serial.println("Ready.");

    strip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.setBrightness(255); // Full brightness

//Motor PWM Channels
    ledcAttach(inOne, 300, 8); //we set up PWM channel 1, frequency of 30,000 Hz, 8 bit resolution
    ledcAttach(inTwo, 300, 8); 
    ledcAttach(inThree, 300, 8); 
    ledcAttach(inFour, 300, 8);
    ledcAttach(inFive, 300, 8); 
    ledcAttach(inSix, 300, 8); 
    ledcAttach(inSeven, 300, 8); 
    ledcAttach(inEight, 300, 8); 

    //Set up Servos
    ledcAttach(25, 50, TIMER_WIDTH); // channel 15, 50 Hz, 16-bit width
   // ledcAttachPin(25, 9);   // GPIO 25 assigned to channel 15
   // ledcAttachPin(27, 9);   // GPIO 27 assigned to channel 15
    ledcAttach(26, 50, TIMER_WIDTH); // channel 14, 50 Hz, 16-bit width
   // ledcAttachPin(26, 10);   // GPIO 26 assigned to channel 14
   // ledcAttachPin(14, 10);   // GPIO 14 assigned to channel 14
    ledcWrite(25,posOne);    
    ledcWrite(26,posTwo);    

 strip.setBrightness(255); // Max Brightness
 strip.clear();
 strip.setPixelColor(0, strip.Color(200, 200, 0));   
 strip.setPixelColor(1, strip.Color(0, 125, 125));
 strip.setPixelColor(2, strip.Color(0, 255, 255));
 strip.setPixelColor(3, strip.Color(0, 255, 255));
 strip.setPixelColor(4, strip.Color(0, 125, 125));   
 strip.setPixelColor(5, strip.Color(200, 200, 0));
 strip.setPixelColor(6, strip.Color(200, 200, 0));
 strip.setPixelColor(7, strip.Color(255, 0, 0));
 strip.setPixelColor(8, strip.Color(0, 255, 255));
 strip.setPixelColor(9, strip.Color(0, 255, 255));   
 strip.setPixelColor(10, strip.Color(255, 0, 0));
 strip.setPixelColor(11, strip.Color(200, 200, 0));
 strip.setPixelColor(12, strip.Color(200, 200, 0));   
 strip.setPixelColor(13, strip.Color(0, 125, 125));
 strip.setPixelColor(14, strip.Color(0, 255, 255));
 strip.setPixelColor(15, strip.Color(0, 255, 255));
 strip.setPixelColor(16, strip.Color(0, 125, 125));   
 strip.setPixelColor(17, strip.Color(200, 200, 0));
 strip.setPixelColor(18, strip.Color(200, 200, 0));
 strip.setPixelColor(19, strip.Color(255, 0, 0));
 strip.setPixelColor(20, strip.Color(0, 255, 255));
 strip.setPixelColor(21, strip.Color(0, 255, 255));   
 strip.setPixelColor(22, strip.Color(255, 0, 0));
 strip.setPixelColor(23, strip.Color(200, 200, 0));
 strip.setPixelColor(24, strip.Color(200, 200, 0));   
 strip.setPixelColor(25, strip.Color(0, 125, 125));
 strip.setPixelColor(26, strip.Color(0, 255, 255));
 strip.setPixelColor(27, strip.Color(0, 255, 255));
 strip.setPixelColor(28, strip.Color(0, 125, 125));   
 strip.setPixelColor(29, strip.Color(200, 200, 0));
 strip.setPixelColor(30, strip.Color(200, 200, 0));
 strip.setPixelColor(31, strip.Color(255, 0, 0));
 strip.show(); 
 delay(2000); 
 //strip.setBrightness(0); // Turn off LEDs
 strip.clear();
 strip.show(); 
}

void loop()
{
  if(Ps3.isConnected()){

 lX =(Ps3.data.analog.stick.lx);
 lY =(Ps3.data.analog.stick.ly);
 rX =(Ps3.data.analog.stick.rx);
 rY =(Ps3.data.analog.stick.ry);

 if(lY < -5){
  analogWrite(inOne, (abs(lY)+255));
  analogWrite(inTwo, 0);
  analogWrite(inThree, (abs(lY)+255));
  analogWrite(inFour, 0);
  analogWrite(inFive, (abs(lY)+255));
  analogWrite(inSix, 0);
  analogWrite(inSeven, (abs(lY)+255));
  analogWrite(inEight, 0);
  Serial.println("Forward!");
 }
 else if(lY > 5){
  strip.setBrightness(255); // Max Brightness
  strip.setPixelColor(16, strip.Color(200, 0, 0)); 
  strip.setPixelColor(17, strip.Color(200, 0, 0));
  strip.setPixelColor(18, strip.Color(200, 0, 0)); 
  strip.setPixelColor(19, strip.Color(200, 0, 0));
  strip.setPixelColor(20, strip.Color(200, 0, 0)); 
  strip.setPixelColor(21, strip.Color(200, 0, 0));  
  strip.setPixelColor(22, strip.Color(200, 0, 0)); 
  strip.setPixelColor(23, strip.Color(200, 0, 0));
  strip.setPixelColor(24, strip.Color(200, 0, 0)); 
  strip.setPixelColor(25, strip.Color(200, 0, 0));
  strip.setPixelColor(26, strip.Color(200, 0, 0)); 
  strip.setPixelColor(27, strip.Color(200, 0, 0));  
  strip.setPixelColor(28, strip.Color(200, 0, 0)); 
  strip.setPixelColor(29, strip.Color(200, 0, 0));
  strip.setPixelColor(30, strip.Color(200, 0, 0)); 
  strip.setPixelColor(31, strip.Color(200, 0, 0)); 
  strip.show();  
  analogWrite(inOne, 0);
  analogWrite(inTwo, (abs(lY)+255));
  analogWrite(inThree, 0);
  analogWrite(inFour, (abs(lY)+255));
  analogWrite(inFive, 0);
  analogWrite(inSix, (abs(lY)+255));
  analogWrite(inSeven, 0);
  analogWrite(inEight, (abs(lY)+255));
  Serial.println("       Backward!");
  strip.setBrightness(50); // Turn off LEDs
  strip.clear();
  strip.show();
 }

//  lX =(Ps3.data.analog.stick.lx);
 else if(lX > 5){
    strip.setBrightness(255); // Max Brightness
  strip.setPixelColor(8, strip.Color(200, 200, 0)); 
  strip.setPixelColor(9, strip.Color(200, 200, 0)); 
  strip.setPixelColor(10, strip.Color(200, 200, 0));
  strip.setPixelColor(11, strip.Color(200, 200, 0)); 
  strip.setPixelColor(12, strip.Color(200, 200, 0));
  strip.setPixelColor(13, strip.Color(200, 200, 0)); 
  strip.setPixelColor(14, strip.Color(200, 200, 0));  
  strip.setPixelColor(15, strip.Color(200, 200, 0));
  strip.setPixelColor(16, strip.Color(200, 200, 0)); 
  strip.setPixelColor(17, strip.Color(200, 200, 0)); 
  strip.setPixelColor(18, strip.Color(200, 200, 0));
  strip.setPixelColor(19, strip.Color(200, 200, 0)); 
  strip.setPixelColor(20, strip.Color(200, 200, 0));
  strip.setPixelColor(21, strip.Color(200, 200, 0)); 
  strip.setPixelColor(22, strip.Color(200, 200, 0));  
  strip.setPixelColor(23, strip.Color(200, 200, 0));  
  strip.show();
  analogWrite(inOne, (abs(lX)+255));
  analogWrite(inTwo, 0);
  analogWrite(inThree, 0);
  analogWrite(inFour, (abs(lX)+255));
  analogWrite(inFive, (abs(lX)+255));
  analogWrite(inSix, 0);
  analogWrite(inSeven, 0);
  analogWrite(inEight, (abs(lX)+255));
  Serial.println("                Right!");
  strip.setBrightness(50); // Turn off LEDs
  strip.clear();
  strip.show();
 }
 else if(lX < -5){
   strip.setBrightness(255); // Max Brightness
  strip.setPixelColor(0, strip.Color(200, 200, 0)); 
  strip.setPixelColor(1, strip.Color(200, 200, 0));
  strip.setPixelColor(2, strip.Color(200, 200, 0)); 
  strip.setPixelColor(3, strip.Color(200, 200, 0));
  strip.setPixelColor(4, strip.Color(200, 200, 0)); 
  strip.setPixelColor(5, strip.Color(200, 200, 0));  
  strip.setPixelColor(6, strip.Color(200, 200, 0)); 
  strip.setPixelColor(7, strip.Color(200, 200, 0));
  strip.setPixelColor(24, strip.Color(200, 200, 0)); 
  strip.setPixelColor(25, strip.Color(200, 200, 0));
  strip.setPixelColor(26, strip.Color(200, 200, 0)); 
  strip.setPixelColor(27, strip.Color(200, 200, 0));
  strip.setPixelColor(28, strip.Color(200, 200, 0)); 
  strip.setPixelColor(29, strip.Color(200, 200, 0));  
  strip.setPixelColor(30, strip.Color(200, 200, 0)); 
  strip.setPixelColor(31, strip.Color(200, 200, 0));  
  strip.show(); 
  analogWrite(inOne, 0);
  analogWrite(inTwo, (abs(lX)+255));
  analogWrite(inThree, (abs(lX)+255));
  analogWrite(inFour, 0);
  analogWrite(inFive, 0);
  analogWrite(inSix, (abs(lX)+255));
  analogWrite(inSeven, (abs(lX)+255));
  analogWrite(inEight, 0);
  Serial.println("                     Left!");
  strip.setBrightness(50); // Turn off LEDs
  strip.clear();
  strip.show();
 }
 
 // Turn off LEDs       
  else if( Ps3.event.button_down.square ){
    Serial.println("        Square button depressed");
    strip.setBrightness(50); // Turn off LEDs
    strip.clear();
    strip.show();
    }
// Servo Control
 rX =(Ps3.data.analog.stick.rx);
 if(rX < -5 && posOne < 7000){
  ledcWrite(25,posOne);       
  posOne+=1;
 }
 else if(rX > 5 && posOne > 1500){
  ledcWrite(25,posOne);       
  posOne-=1;
 }
  rY =(Ps3.data.analog.stick.ry);
 if(rY < -5 && posTwo < 7000){
  ledcWrite(26,posTwo);       
  posTwo+=1;
 }
 else if(rY > 5 && posTwo > 1500){
  ledcWrite(26,posTwo);       
  posTwo-=1;
 }

 else {
  analogWrite(inOne, 0);
  analogWrite(inTwo, 0);
  analogWrite(inThree, 0);
  analogWrite(inFour, 0);
  analogWrite(inFive, 0);
  analogWrite(inSix, 0);
  analogWrite(inSeven, 0);
  analogWrite(inEight, 0);
 }
 while( Ps3.event.button_down.cross ){
  Serial.println("        Cross button depressed");
  strip.setBrightness(125); // Turn on LEDs
  strip.clear();
  strip.setPixelColor(0, strip.Color(200, 200, 0));
  strip.setPixelColor(1, strip.Color(0, 100, 100));
  strip.setPixelColor(2, strip.Color(0, 255, 255));
  strip.setPixelColor(3, strip.Color(0, 255, 255));
  strip.setPixelColor(4, strip.Color(0, 100, 100));
  strip.setPixelColor(5, strip.Color(200, 200, 0));
  strip.setPixelColor(6, strip.Color(200, 200, 0));
  strip.setPixelColor(7, strip.Color(255, 0, 0));
  strip.setPixelColor(8, strip.Color(0, 255, 255));
  strip.setPixelColor(9, strip.Color(0, 255, 255));
  strip.setPixelColor(10, strip.Color(255, 0, 0));
  strip.setPixelColor(11, strip.Color(200, 200, 0));
  strip.setPixelColor(12, strip.Color(200, 200, 0));   
 strip.setPixelColor(13, strip.Color(0, 125, 125));
 strip.setPixelColor(14, strip.Color(0, 255, 255));
 strip.setPixelColor(15, strip.Color(0, 255, 255));
 strip.setPixelColor(16, strip.Color(0, 125, 125));   
 strip.setPixelColor(17, strip.Color(200, 200, 0));
 strip.setPixelColor(18, strip.Color(200, 200, 0));
 strip.setPixelColor(19, strip.Color(255, 0, 0));
 strip.setPixelColor(20, strip.Color(0, 255, 255));
 strip.setPixelColor(21, strip.Color(0, 255, 255));   
 strip.setPixelColor(22, strip.Color(255, 0, 0));
 strip.setPixelColor(23, strip.Color(200, 200, 0));
 strip.setPixelColor(24, strip.Color(200, 200, 0));   
 strip.setPixelColor(25, strip.Color(0, 125, 125));
 strip.setPixelColor(26, strip.Color(0, 255, 255));
 strip.setPixelColor(27, strip.Color(0, 255, 255));
 strip.setPixelColor(28, strip.Color(0, 125, 125));   
 strip.setPixelColor(29, strip.Color(200, 200, 0));
 strip.setPixelColor(30, strip.Color(200, 200, 0));
 strip.setPixelColor(31, strip.Color(255, 0, 0));
  strip.show();
  delay(2000);
  }
     //--------------- Digital D-pad button events --------------
    while( Ps3.event.button_down.left ){
        Serial.println("Started pressing the left button");
        strip.setBrightness(125); // Turn on LEDs
        strip.clear();
        strip.setPixelColor(0, strip.Color(255, 255, 255));
        strip.setPixelColor(1, strip.Color(255, 255, 255));
        strip.setPixelColor(2, strip.Color(255, 255, 255));
        strip.setPixelColor(3, strip.Color(255, 255, 255));
        strip.setPixelColor(4, strip.Color(255, 255, 255));
        strip.setPixelColor(5, strip.Color(255, 255, 255));
        strip.setPixelColor(6, strip.Color(255, 255, 255));
        strip.setPixelColor(7, strip.Color(255, 255, 255));
        strip.setPixelColor(24, strip.Color(255, 255, 255)); 
        strip.setPixelColor(25, strip.Color(255, 255, 255));
        strip.setPixelColor(26, strip.Color(255, 255, 255)); 
        strip.setPixelColor(27, strip.Color(255, 255, 255));
        strip.setPixelColor(28, strip.Color(255, 255, 255)); 
        strip.setPixelColor(29, strip.Color(255, 255, 255));  
        strip.setPixelColor(30, strip.Color(255, 255, 255)); 
        strip.setPixelColor(31, strip.Color(255, 255, 255));         
        strip.show();
        delay(2000);
    }
  
    
    while( Ps3.event.button_up.right ){
       Serial.println("Started pressing the right button");
        strip.setBrightness(125); // Turn on LEDs
        strip.clear();
        strip.setPixelColor(8, strip.Color(255, 255, 255));
        strip.setPixelColor(9, strip.Color(255, 255, 255));
        strip.setPixelColor(10, strip.Color(255, 255, 255));
        strip.setPixelColor(11, strip.Color(255, 255, 255));
        strip.setPixelColor(12, strip.Color(255, 255, 255));
        strip.setPixelColor(13, strip.Color(255, 255, 255));
        strip.setPixelColor(14, strip.Color(255, 255, 255));
        strip.setPixelColor(15, strip.Color(255, 255, 255));
        strip.setPixelColor(16, strip.Color(255, 255, 255)); 
        strip.setPixelColor(17, strip.Color(255, 255, 255)); 
        strip.setPixelColor(18, strip.Color(255, 255, 255));
        strip.setPixelColor(19, strip.Color(255, 255, 255)); 
        strip.setPixelColor(20, strip.Color(255, 255, 255));
        strip.setPixelColor(21, strip.Color(255, 255, 255)); 
        strip.setPixelColor(22, strip.Color(255, 255, 255));  
        strip.setPixelColor(23, strip.Color(255, 255, 255));              
        strip.show();
        delay(2000);
    } 
     while( Ps3.event.button_down.down ){
        Serial.println("Started pressing the down button");
        strip.setBrightness(125); // Turn on LEDs
        strip.clear();
        strip.setPixelColor(0, strip.Color(255, 255, 255));
        strip.setPixelColor(1, strip.Color(255, 255, 255));
        strip.setPixelColor(2, strip.Color(255, 255, 255));
        strip.setPixelColor(3, strip.Color(255, 255, 255));
        strip.setPixelColor(4, strip.Color(255, 255, 255));
        strip.setPixelColor(5, strip.Color(255, 255, 255));
        strip.setPixelColor(6, strip.Color(255, 255, 255));
        strip.setPixelColor(7, strip.Color(255, 255, 255));
        strip.setPixelColor(8, strip.Color(255, 255, 255));
        strip.setPixelColor(9, strip.Color(255, 255, 255));
        strip.setPixelColor(10, strip.Color(255, 255, 255));
        strip.setPixelColor(11, strip.Color(255, 255, 255));
        strip.setPixelColor(12, strip.Color(255, 255, 255));
        strip.setPixelColor(13, strip.Color(255, 255, 255));
        strip.setPixelColor(14, strip.Color(255, 255, 255));
        strip.setPixelColor(15, strip.Color(255, 255, 255));
        strip.show();
        delay(2000);
    } 
  while( Ps3.event.button_down.up ){
        Serial.println("Started pressing the up button");
        strip.setBrightness(225); // Turn on LEDs
        strip.clear();
        strip.setPixelColor(0, strip.Color(255, 255, 255));
        strip.setPixelColor(1, strip.Color(255, 255, 255));
        strip.setPixelColor(2, strip.Color(255, 255, 255));
        strip.setPixelColor(3, strip.Color(255, 255, 255));
        strip.setPixelColor(4, strip.Color(255, 255, 255));
        strip.setPixelColor(5, strip.Color(255, 255, 255));
        strip.setPixelColor(6, strip.Color(255, 255, 255));
        strip.setPixelColor(7, strip.Color(255, 255, 255));
        strip.setPixelColor(8, strip.Color(255, 255, 255));
        strip.setPixelColor(9, strip.Color(255, 255, 255));
        strip.setPixelColor(10, strip.Color(255, 255, 255));
        strip.setPixelColor(11, strip.Color(255, 255, 255));
        strip.setPixelColor(12, strip.Color(255, 255, 255));
        strip.setPixelColor(13, strip.Color(255, 255, 255));
        strip.setPixelColor(14, strip.Color(255, 255, 255));
        strip.setPixelColor(15, strip.Color(255, 255, 255));
        strip.show();
        delay(2000);
    }        
 }
}
