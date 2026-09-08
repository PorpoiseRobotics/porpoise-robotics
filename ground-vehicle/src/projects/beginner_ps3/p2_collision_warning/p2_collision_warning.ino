/*
  p2_collision_warning.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track)
  TAKE IT FURTHER project - see Lesson 5, "projects that fit on this vehicle"

  WHAT THIS PROGRAM DOES
  ----------------------
  Stops the vehicle before it drives into something.

  An ultrasonic range finder looks straight ahead. Closer than 20 inches:
  stop, flash the whole strip red, hold for three seconds, then let go.
  Reverse still works the whole time - backing away is how you get out.

  This is l5c_drive_with_lights with one thing added. Everything else in
  the file you have already met. Open the two side by side and the
  difference is the project.

  WHAT YOU HAVE TO WIRE UP
  ------------------------
  An HC-SR04 range finder, facing forward, on the top plate.

      HC-SR04 VCC   ->  5 V
      HC-SR04 GND   ->  GND
      HC-SR04 TRIG  ->  GPIO 32
      HC-SR04 ECHO  ->  a divider, then GPIO 35

  TRIG is safe to wire straight across: it is an INPUT to the sensor, and
  3.3 volts is enough to trigger it.

  ECHO is not. It is an OUTPUT from the sensor and it swings to FIVE
  volts, which is more than an ESP32 pin will survive. So:

      ECHO  ---[ 1k ]---+---[ 2k ]---  GND
                        |
                     GPIO 35

      5 V x 2 / (1 + 2)  =  3.3 V

  Lesson 2's divider again, doing the same job for a different reason.
  GPIO 35 is INPUT ONLY, which is all ECHO needs.

  No sensor connected? Nothing ever comes back, the program reads that as
  "nothing is close", and the vehicle drives normally.

  SAFETY
  ------
  Wheels off the ground for the first upload, every time. Motors stop by
  themselves if the controller disconnects.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > esp32 > "ESP32 Dev Module".
  2. Libraries: "PS3 Controller Host" by Jeffrey van Pernis
                "Adafruit NeoPixel" by Adafruit

  CONTROLS
  --------
    Left stick          Drive. Up = forward, down = reverse, left/right = turn.
    SQUARE              All lights on / off
    D-pad UP / DOWN     Headlights bright / dim

  THE IDEA
  --------
  Sound covers about 13,560 inches a second, so a ping takes 74
  microseconds per inch to get there and another 74 to come back. Divide
  the round trip by 148 and you have inches. That is the whole sensor.

  pulseIn() is the ONE blocking call in this program, and it is here
  deliberately. The 12 millisecond timeout is the longest it can ever
  take, it runs once every 60 milliseconds, and the advanced course does
  the same job with an interrupt and blocks for nothing at all. Knowing
  which compromises you have made is the point.

  Note that STOP_INCHES and CLEAR_INCHES are different numbers. Stopping
  at 20 and clearing at 20 would leave the vehicle chattering in and out
  of the warning at exactly 20 inches. Two thresholds with a gap between
  them is called HYSTERESIS, and you will meet it in every controller you
  ever build.

  WHAT TO TRY
  -----------
  1. Hold a book in front of it and watch the inches on the serial
     monitor. Check them against a tape measure.
  2. Point it at something soft - a jumper, a curtain. Ultrasound is bad
     at soft things, and it is worth seeing that yourself.
  3. Point it at a wall at a steep angle. Where did the ping go?
  4. Make STOP_INCHES depend on speed, so the vehicle stops further out
     when it is moving fast. That is what a real one does.
  5. Set CLEAR_INCHES equal to STOP_INCHES and drive at a wall. Watch it
     chatter. Put it back.
  6. Fold this into your copy of the full program.
*/

#include <Ps3Controller.h>
#include <Adafruit_NeoPixel.h>

const char *PS3_MAC_ADDRESS = "02:02:03:04:05:08";

// --- LEDs ---
const int LED_PIN        = 5;
const int LED_COUNT      = 32;
const int LED_BRIGHTNESS = 120;

const int FRONT_FIRST = 0;    // Front left
const int FRONT_LAST  = 15;   // Front right
const int REAR_FIRST  = 16;   // Rear right
const int REAR_LAST   = 31;   // Rear left

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Motors ---
const int FRONT_LEFT_A  = 12, FRONT_LEFT_B  = 13;
const int REAR_LEFT_A   = 18, REAR_LEFT_B   = 19;
const int FRONT_RIGHT_A = 22, FRONT_RIGHT_B = 23;
const int REAR_RIGHT_A  = 16, REAR_RIGHT_B  = 17;

const int MOTOR_PWM_FREQ = 20000;
const int MOTOR_PWM_BITS = 8;
const int MOTOR_MAX      = 255;
const int MOTOR_MIN      = 0;

const int turnMax = (MOTOR_MAX * 3) / 4;

// --- Thumbsticks ---
const int STICK_MAX      = 127;
const int STICK_DEADZONE = 20;


// --- Range finder ----------------------------------------------------
// See the wiring note at the top. ECHO goes through a divider; TRIG does
// not need one.
const int TRIG_PIN = 32;
const int ECHO_PIN = 35;

const float STOP_INCHES  = 20.0f;   // Closer than this and we stop
const float CLEAR_INCHES = 26.0f;   // Further than this before we let go

const unsigned long PING_EVERY_MS   = 60;
const unsigned long STOP_HOLD_MS    = 3000;
const unsigned long ECHO_TIMEOUT_US = 12000;   // About 6 feet, there and back
const unsigned long WARN_BLINK_MS   = 150;

// --- State ---
enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE, LIGHTS_LEFT, LIGHTS_RIGHT };

bool         lightsOn       = true;
int          headlightLevel = 80;
LightPattern lightPattern   = LIGHTS_STOPPED;
bool         lightsChanged  = true;

bool squareWasDown = false;
bool wasConnected  = false;


enum RangeState { RANGE_CLEAR, RANGE_STOPPED, RANGE_BACKING_OFF };

RangeState    rangeState     = RANGE_CLEAR;
float         lastInches     = 999.0f;
unsigned long lastPing       = 0;
unsigned long stoppedAt      = 0;
unsigned long lastWarnBlink  = 0;
bool          warnBlinkOn    = false;

bool justPressed(bool isDown, bool &wasDown) {
  bool isNewPress = isDown && !wasDown;
  wasDown = isDown;
  return isNewPress;
}

int stickToSpeed(int stickValue, int maxSpeed) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);
  size = constrain(size, MOTOR_MIN, maxSpeed);
  return (stickValue > 0) ? size : -size;
}

void setMotor(int pinA, int pinB, int speed) {
  speed = constrain(speed, -MOTOR_MAX, MOTOR_MAX);
  if (speed >= 0) {
    ledcWrite(pinA, speed);
    ledcWrite(pinB, 0);
  } else {
    ledcWrite(pinA, 0);
    ledcWrite(pinB, -speed);
  }
}

void drive(int leftSpeed, int rightSpeed) {
  setMotor(FRONT_LEFT_A,  FRONT_LEFT_B,  leftSpeed);
  setMotor(REAR_LEFT_A,   REAR_LEFT_B,   leftSpeed);
  setMotor(FRONT_RIGHT_A, FRONT_RIGHT_B, rightSpeed);
  setMotor(REAR_RIGHT_A,  REAR_RIGHT_B,  rightSpeed);
}

/*
  Draws the driving lights: white at the front, red at the back, brighter red
  when stopped, white at the back in reverse, and amber down whichever side we
  are turning toward.
*/
void showDrivingLights() {
  strip.clear();

  if (lightsOn) {
    uint32_t headlight    = strip.Color(headlightLevel, headlightLevel, headlightLevel);
    uint32_t tailLight    = strip.Color(60, 0, 0);
    uint32_t brakeLight   = strip.Color(180, 0, 0);
    uint32_t reverseLight = strip.Color(200, 200, 200);
    uint32_t amber        = strip.Color(255, 100, 0);

    for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {
      strip.setPixelColor(i, headlight);
    }

    uint32_t rearColor = tailLight;
    if (lightPattern == LIGHTS_STOPPED) rearColor = brakeLight;
    if (lightPattern == LIGHTS_REVERSE) rearColor = reverseLight;
    for (int i = REAR_FIRST; i <= REAR_LAST; i++) {
      strip.setPixelColor(i, rearColor);
    }

    // Remember the loop: 0-7 and 24-31 are LEFT, 8-15 and 16-23 are RIGHT.
    if (lightPattern == LIGHTS_LEFT) {
      for (int i = 0;  i <= 7;  i++) strip.setPixelColor(i, amber);
      for (int i = 24; i <= 31; i++) strip.setPixelColor(i, amber);
    } else if (lightPattern == LIGHTS_RIGHT) {
      for (int i = 8;  i <= 15; i++) strip.setPixelColor(i, amber);
      for (int i = 16; i <= 23; i++) strip.setPixelColor(i, amber);
    }
  }

  strip.show();
}

/*
  Slow green blink while we wait for a controller. No delay() in here - we
  check the clock instead, so the rest of the program keeps running.
*/
void showWaitingLights() {
  static unsigned long lastBlink = 0;
  static bool blinkOn = false;

  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    blinkOn = !blinkOn;
    strip.fill(blinkOn ? strip.Color(0, 60, 0) : strip.Color(0, 0, 0));
    strip.show();
  }
}


/*
  One ping, and how far away the nearest thing in front of us is, in inches.

  A 10 microsecond pulse on TRIG starts the ping. The sensor then holds ECHO
  high for exactly as long as the sound was in the air, and pulseIn() measures
  that. 148 microseconds per inch, because the sound has to get there AND back.

  A timeout returns 0, which means nothing came back at all - so we report a
  long way, not a short one. Getting that backwards makes a vehicle that
  slams to a halt the moment the sensor is unplugged.
*/
float pingInches() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long echoMicros = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (echoMicros == 0) {
    return 999.0f;
  }
  return echoMicros / 148.0f;
}

/*
  The collision warning: the whole strip, red, blinking fast.
*/
void showCollisionWarning() {
  strip.fill(warnBlinkOn ? strip.Color(255, 0, 0) : strip.Color(0, 0, 0));
  strip.show();
}

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();

  ledcAttach(FRONT_LEFT_A,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_LEFT_B,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_LEFT_A,   MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_LEFT_B,   MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_RIGHT_A, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_RIGHT_B, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_RIGHT_A,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_RIGHT_B,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  drive(0, 0);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Ps3.begin(PS3_MAC_ADDRESS);
  Serial.println("Drive with lights. Waiting for the controller...");
}

void loop() {
  // ---- No controller? Stop everything and wait. ----
  if (!Ps3.isConnected()) {
    if (wasConnected) {
      Serial.println("Controller disconnected. Motors stopped.");
      wasConnected = false;
    }
    drive(0, 0);
    showWaitingLights();
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    lightsChanged = true;
    Ps3.setPlayer(1);
    Serial.println("Controller connected.");
  }

  // ---- Buttons ----
  if (justPressed(Ps3.data.button.square, squareWasDown)) {
    lightsOn = !lightsOn;
    lightsChanged = true;
    Serial.println(lightsOn ? "Lights ON" : "Lights OFF");
  }

  if (Ps3.data.button.up && headlightLevel != 255) {
    headlightLevel = 255;
    lightsChanged = true;
  }
  if (Ps3.data.button.down && headlightLevel != 80) {
    headlightLevel = 80;
    lightsChanged = true;
  }

  // ---- Drive ----
  int leftStickX = Ps3.data.analog.stick.lx;
  int leftStickY = Ps3.data.analog.stick.ly;

  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);
  int turn    = stickToSpeed(leftStickX, turnMax);

  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);
  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);
  // ---- Ping, and decide whether we are allowed to go forward ----
  if (millis() - lastPing >= PING_EVERY_MS) {
    lastPing = millis();
    lastInches = pingInches();

    if (rangeState == RANGE_CLEAR && lastInches < STOP_INCHES) {
      rangeState = RANGE_STOPPED;
      stoppedAt = millis();
      lightsChanged = true;
      Serial.print("Something at ");
      Serial.print(lastInches, 1);
      Serial.println(" in. Stopping.");
    }
  }

  if (rangeState == RANGE_STOPPED && millis() - stoppedAt >= STOP_HOLD_MS) {
    rangeState = RANGE_BACKING_OFF;
    lightsChanged = true;
    Serial.println("Three seconds up. Reverse away, or steer around it.");
  }

  if (rangeState == RANGE_BACKING_OFF && lastInches > CLEAR_INCHES) {
    rangeState = RANGE_CLEAR;
    lightsChanged = true;
    Serial.println("Clear.");
  }

  // While the warning is up, forward is refused and reverse is not. A vehicle
  // you cannot back out of a corner is worse than one that hits the wall.
  if (rangeState == RANGE_STOPPED) {
    leftSpeed = 0;
    rightSpeed = 0;
  } else if (rangeState == RANGE_BACKING_OFF) {
    if (leftSpeed > 0)  leftSpeed = 0;
    if (rightSpeed > 0) rightSpeed = 0;
  }

  drive(leftSpeed, rightSpeed);

  // ---- Which lighting picture matches what we are doing? ----
  LightPattern newPattern = LIGHTS_STOPPED;
  if (forward > 0)      newPattern = LIGHTS_FORWARD;
  else if (forward < 0) newPattern = LIGHTS_REVERSE;
  else if (turn > 0)    newPattern = LIGHTS_RIGHT;
  else if (turn < 0)    newPattern = LIGHTS_LEFT;

  if (newPattern != lightPattern) {
    lightPattern = newPattern;
    lightsChanged = true;
  }

  // ---- Redraw. A collision warning takes the whole strip. ----
  if (rangeState == RANGE_STOPPED) {
    if (millis() - lastWarnBlink >= WARN_BLINK_MS) {
      lastWarnBlink = millis();
      warnBlinkOn = !warnBlinkOn;
      showCollisionWarning();
    }
    lightsChanged = true;
  } else if (lightsChanged) {
    lightsChanged = false;
    showDrivingLights();
  }
}
