#include <Bluepad32.h>         // Bluetooth game controller library
#include <esp32-hal-ledc.h>    // ESP32 PWM functions
#include <Adafruit_NeoPixel.h> // NeoPixel LED strip library

// ------------------------- NeoPixel settings -------------------------
#define PIN        5           // Data pin for the NeoPixel strip
#define NUMPIXELS 32           // Total number of LEDs in the strip
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// ------------------------- Servo PWM settings -------------------------
#define TIMER_WIDTH 16         // 16-bit PWM resolution for servo outputs

// ------------------------- Servo position values -------------------------
// These values are written directly to the PWM channels that control the servos.
int posOne = 5000;
int posTwo = 6000;
int posThree = 5000; // posThree and posFour are declared but never used
int posFour = 4000;

// ------------------------- Motor control pins -------------------------
// Each motor uses a pair of pins so direction can be controlled by which side gets PWM.
int inOne = 12;
int inTwo = 13;
int inThree = 16;
int inFour = 17;
int inFive = 18;
int inSix = 19;
int inSeven = 22;
int inEight = 23;

// ------------------------- Controller stick values -------------------------
int rX;
int rY;
int lX;
int lY;

// ------------------------- Motor speed values -------------------------
// These are the PWM duty values calculated from the analog stick movement.
int speedY;
int speedX;

// ------------------------- Active controller -------------------------
// This holds the currently connected controller.
ControllerPtr myController = nullptr;

// ------------------------- Previous button states -------------------------
// These are used to detect a new button press only once.
bool prevCross = false;
bool prevSquare = false;
uint8_t prevDpad = 0;

// ------------------------- LEDC PWM channel assignments -------------------------
// The ESP32 PWM system uses channels. Each motor pin and servo output gets one.
const int chOne = 0;
const int chTwo = 1;
const int chThree = 2;
const int chFour = 3;
const int chFive = 4;
const int chSix = 5;
const int chSeven = 6;
const int chEight = 7;
const int chServoOne = 8;
const int chServoTwo = 9;

// ------------------------- Controller connected callback -------------------------
// This runs automatically when a Bluetooth controller connects.
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {
    myController = ctl;
    Serial.println("Controller connected.");
  }
}

// ------------------------- Controller disconnected callback -------------------------
// This runs automatically when the active controller disconnects.
void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    myController = nullptr;
    Serial.println("Controller disconnected.");
  }
}

void setup() {
  Serial.begin(115200);

  // Start the Bluetooth controller system.
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);
  BP32.forgetBluetoothKeys();
  Serial.println("Ready.");

  // ------------------------- NeoPixel initialization -------------------------
  strip.begin();
  strip.setBrightness(255);

  // ------------------------- Motor PWM setup -------------------------
  // Each motor control pin is attached to one PWM channel.
  // Frequency = 300 Hz
  // Resolution = 8 bits
  ledcSetup(chOne, 300, 8);
  ledcAttachPin(inOne, chOne);

  ledcSetup(chTwo, 300, 8);
  ledcAttachPin(inTwo, chTwo);

  ledcSetup(chThree, 300, 8);
  ledcAttachPin(inThree, chThree);

  ledcSetup(chFour, 300, 8);
  ledcAttachPin(inFour, chFour);

  ledcSetup(chFive, 300, 8);
  ledcAttachPin(inFive, chFive);

  ledcSetup(chSix, 300, 8);
  ledcAttachPin(inSix, chSix);

  ledcSetup(chSeven, 300, 8);
  ledcAttachPin(inSeven, chSeven);

  ledcSetup(chEight, 300, 8);
  ledcAttachPin(inEight, chEight);

  // ------------------------- Servo PWM setup -------------------------
  // Servos use 50 Hz PWM with 16-bit resolution.
  ledcSetup(chServoOne, 50, TIMER_WIDTH);
  ledcAttachPin(25, chServoOne);

  ledcSetup(chServoTwo, 50, TIMER_WIDTH);
  ledcAttachPin(26, chServoTwo);

  // Send the starting servo positions.
  ledcWrite(chServoOne, posOne);
  ledcWrite(chServoTwo, posTwo);

  // ------------------------- Startup LED pattern -------------------------
  // This pattern is shown for 2 seconds when the board powers up.
  strip.setBrightness(255);
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

  // Turn the strip off after the startup pattern finishes.
  strip.clear();
  strip.show();
}

void loop() {
  // Update controller data.
  BP32.update();

  // Only run the control logic if a controller is connected.
  if (myController != nullptr) {

    // ------------------------- Read analog sticks -------------------------
    // Left stick controls vehicle movement.
    // Right stick controls the two servos.
    lX = myController->axisX();
    lY = myController->axisY();
    rX = myController->axisRX();
    rY = myController->axisRY();

    // ------------------------- Read buttons -------------------------
    // a() is used as Cross
    // x() is used as Square
    bool crossNow = myController->a();
    bool squareNow = myController->x();
    uint8_t dpadNow = myController->dpad();

    // ------------------------- Convert stick movement into PWM speed -------------------------
    // The first 25 counts are treated as a dead zone.
    // The rest of the stick range is mapped to 0-255.
    speedY = constrain(map(abs(lY), 75, 512, 0, 255), 0, 255);
    speedX = constrain(map(abs(lX), 25, 512, 0, 125), 0, 255);

    // ------------------------- Vehicle movement -------------------------
    // The left stick chooses one movement direction at a time.

    if (lY < -75) {
      // Move forward
      ledcWrite(chOne, speedY);
      ledcWrite(chTwo, 0);
      ledcWrite(chThree, speedY);
      ledcWrite(chFour, 0);
      ledcWrite(chFive, speedY);
      ledcWrite(chSix, 0);
      ledcWrite(chSeven, speedY);
      ledcWrite(chEight, 0);
      Serial.println("Forward!");
    }
    else if (lY > 75) {
      // Show reverse LED pattern
      strip.setBrightness(255);
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

      // Move backward
      ledcWrite(chOne, 0);
      ledcWrite(chTwo, speedY);
      ledcWrite(chThree, 0);
      ledcWrite(chFour, speedY);
      ledcWrite(chFive, 0);
      ledcWrite(chSix, speedY);
      ledcWrite(chSeven, 0);
      ledcWrite(chEight, speedY);
      Serial.println("       Backward!");

      // Turn off the reverse LED pattern
      strip.setBrightness(50);
      strip.clear();
      strip.show();
    }
    else if (lX > 25) {
      // Show right-turn LED pattern
      strip.setBrightness(255);
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

      // Turn right
      ledcWrite(chOne, speedX);
      ledcWrite(chTwo, 0);
      ledcWrite(chThree, 0);
      ledcWrite(chFour, speedX);
      ledcWrite(chFive, speedX);
      ledcWrite(chSix, 0);
      ledcWrite(chSeven, 0);
      ledcWrite(chEight, speedX);
      Serial.println("                Right!");

      // Turn off the right-turn LED pattern
      strip.setBrightness(50);
      strip.clear();
      strip.show();
    }
    else if (lX < -25) {
      // Show left-turn LED pattern
      strip.setBrightness(255);
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

      // Turn left
      ledcWrite(chOne, 0);
      ledcWrite(chTwo, speedX);
      ledcWrite(chThree, speedX);
      ledcWrite(chFour, 0);
      ledcWrite(chFive, 0);
      ledcWrite(chSix, speedX);
      ledcWrite(chSeven, speedX);
      ledcWrite(chEight, 0);
      Serial.println("                     Left!");

      // Turn off the left-turn LED pattern
      strip.setBrightness(50);
      strip.clear();
      strip.show();
    }
    else if (squareNow && !prevSquare) {
      // Square button clears the LEDs and stops the drive motors
      Serial.println("        Square button depressed");
      strip.setBrightness(50);
      strip.clear();
      strip.show();

      ledcWrite(chOne, 0);
      ledcWrite(chTwo, 0);
      ledcWrite(chThree, 0);
      ledcWrite(chFour, 0);
      ledcWrite(chFive, 0);
      ledcWrite(chSix, 0);
      ledcWrite(chSeven, 0);
      ledcWrite(chEight, 0);
    }
    else {
      // No movement command is active, so stop the drive motors
      ledcWrite(chOne, 0);
      ledcWrite(chTwo, 0);
      ledcWrite(chThree, 0);
      ledcWrite(chFour, 0);
      ledcWrite(chFive, 0);
      ledcWrite(chSix, 0);
      ledcWrite(chSeven, 0);
      ledcWrite(chEight, 0);
    }

    // ------------------------- Servo control -------------------------
    // Right stick X moves servo 1.
    // Right stick Y moves servo 2.

    if (rX < -25 && posOne < 7000) {
      ledcWrite(chServoOne, posOne);
      posOne += 1;
    }
    else if (rX > 25 && posOne > 1500) {
      ledcWrite(chServoOne, posOne);
      posOne -= 1;
    }

    if (rY < -25 && posTwo < 7000) {
      ledcWrite(chServoTwo, posTwo);
      posTwo += 1;
    }
    else if (rY > 25 && posTwo > 1500) {
      ledcWrite(chServoTwo, posTwo);
      posTwo -= 1;
    }

    // ------------------------- Cross button pattern -------------------------
    // This pattern is shown once when Cross is pressed.
    if (crossNow && !prevCross) {
      Serial.println("        Cross button depressed");
      strip.setBrightness(125);
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

    // ------------------------- D-pad left pattern -------------------------
    if (dpadNow == 0x08 && prevDpad != 0x08) {
      Serial.println("Started pressing the left button");
      strip.setBrightness(125);
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

    // ------------------------- D-pad right pattern -------------------------
    if (dpadNow == 0x04 && prevDpad != 0x04) {
      Serial.println("Started pressing the right button");
      strip.setBrightness(125);
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

    // ------------------------- D-pad down pattern -------------------------
    if (dpadNow == 0x02 && prevDpad != 0x02) {
      Serial.println("Started pressing the down button");
      strip.setBrightness(125);
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

    // ------------------------- D-pad up pattern -------------------------
    if (dpadNow == 0x01 && prevDpad != 0x01) {
      Serial.println("Started pressing the up button");
      strip.setBrightness(225);
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

    // Save the current button states for edge detection next loop.
    prevCross = crossNow;
    prevSquare = squareNow;
    prevDpad = dpadNow;
  }
  else {
    // If no controller is connected, make sure the drive motors are off.
    ledcWrite(chOne, 0);
    ledcWrite(chTwo, 0);
    ledcWrite(chThree, 0);
    ledcWrite(chFour, 0);
    ledcWrite(chFive, 0);
    ledcWrite(chSix, 0);
    ledcWrite(chSeven, 0);
    ledcWrite(chEight, 0);

    // Reset stored button states.
    prevCross = false;
    prevSquare = false;
    prevDpad = 0;
  }
}