/*
  p3_automatic_lights.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track)
  TAKE IT FURTHER project - see Lesson 5, "projects that fit on this vehicle"

  WHAT THIS PROGRAM DOES
  ----------------------
  Three pieces of lighting that decide for themselves.

    HEADLIGHTS ON DEMAND   The front bar lights only while the vehicle is
                           actually moving. Stop, and it goes dark.
    HAZARD LIGHTS          All four corners flashing amber, on a button,
                           drawn on top of whatever else is lit.
    REVERSING BEEP         A truck beep, from a buzzer, whenever the
                           vehicle is going backwards.

  This is l5c_drive_with_lights with one thing added. Everything else in
  the file you have already met. Open the two side by side and the
  difference is the project.

  WHAT YOU HAVE TO WIRE UP
  ------------------------
  One passive piezo buzzer, on the top plate breadboard.

      buzzer +  ->  GPIO 33
      buzzer -  ->  GND

  It must be a PASSIVE buzzer. An active one has its own oscillator
  inside and only wants on or off; a passive one is a tiny speaker and
  needs a square wave fed to it. We already know how to make a square
  wave - that is Lesson 2's PWM, run at a frequency you can hear instead
  of one you cannot.

  No buzzer? Everything else still works, and the pin sits there driving
  nothing. Set BEEP_WHEN_REVERSING to false to switch it off entirely.

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
    TRIANGLE            Auto headlights on / off
    CIRCLE              Hazard lights on / off

  THE IDEA
  --------
  Every one of these three is the same shape: something the program
  already knows - which way it is moving - drives something the program
  already does. No new sensor, and no new maths.

  The hazards are the interesting one. They are drawn AFTER the driving
  lights, over the top, so the tail lights carry on underneath and the
  corners flash on their own clock. That is exactly how the full program
  runs the KITT scanner over the top of the driving lights.

  The beep is two intervals, not one: 250 ms on, 400 ms off. A beep with
  equal on and off sounds like an alarm; a real reversing beep is short
  and spaced.

  WHAT TO TRY
  -----------
  1. Turn auto headlights off and drive. Which do you prefer, and why?
  2. Make the headlights come on when moving and go off two seconds
     AFTER stopping, instead of immediately. You need one more millis()
     timer.
  3. Change BUZZER_FREQ. Find the pitch your buzzer is loudest at - it
     has a resonant frequency, and it is usually near 2.7 kHz.
  4. Make the hazards flash the two sides alternately instead of all four
     corners together.
  5. Make the beep faster the closer you are to something, using the
     range finder from p2_collision_warning. That is a real reversing
     sensor.
  6. Fold the parts you like into your copy of the full program.
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


// --- Buzzer ----------------------------------------------------------
// See the wiring note at the top. A passive piezo is a tiny speaker, so
// it wants a square wave, which is what PWM is.
const int BUZZER_PIN  = 33;
const int BUZZER_FREQ = 2400;    // About the pitch of a reversing truck
const int BUZZER_BITS = 8;

const bool BEEP_WHEN_REVERSING = true;   // Set false if you have no buzzer

const unsigned long BEEP_ON_MS  = 250;   // A real reversing beep is short
const unsigned long BEEP_OFF_MS = 400;   // and generously spaced

// --- Automatic lighting ----------------------------------------------
const unsigned long HAZARD_BLINK_MS = 400;

// --- State ---
enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE, LIGHTS_LEFT, LIGHTS_RIGHT };

bool         lightsOn       = true;
int          headlightLevel = 80;
LightPattern lightPattern   = LIGHTS_STOPPED;
bool         lightsChanged  = true;

bool squareWasDown = false;
bool wasConnected  = false;


bool          autoHeadlights   = true;    // Front bar only while moving
bool          headlightsLit    = false;
bool          hazardsOn        = false;
bool          hazardBlinkOn    = false;
unsigned long lastHazardBlink  = 0;

bool          beeping          = false;
unsigned long lastBeepChange   = 0;

bool autoButtonWasDown   = false;
bool hazardButtonWasDown = false;

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

    if (headlightsLit) {
      for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {
        strip.setPixelColor(i, headlight);
      }
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
  On or off. The duty is half, which is as loud as a square wave gets - a
  piezo cares about the edges, not the average.
*/
void setBuzzer(bool on) {
  ledcWrite(BUZZER_PIN, on ? 128 : 0);
}

/*
  Hazards: the four corners, amber, blinking together.

  Drawn on TOP of the driving lights rather than instead of them, so the tail
  lights stay lit underneath. Remember the loop - the front bar runs 0 to 15
  left to right, and the rear runs 16 to 31 right to left, so the four corners
  are the two ends of each bar.
*/
void showHazards() {
  uint32_t amber = hazardBlinkOn ? strip.Color(255, 100, 0)
                                 : strip.Color(0, 0, 0);
  for (int i = 0;  i <= 3;  i++) strip.setPixelColor(i, amber);   // Front left
  for (int i = 12; i <= 15; i++) strip.setPixelColor(i, amber);   // Front right
  for (int i = 16; i <= 19; i++) strip.setPixelColor(i, amber);   // Rear right
  for (int i = 28; i <= 31; i++) strip.setPixelColor(i, amber);   // Rear left
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

  ledcAttach(BUZZER_PIN, BUZZER_FREQ, BUZZER_BITS);
  setBuzzer(false);

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

  if (justPressed(Ps3.data.button.triangle, autoButtonWasDown)) {
    autoHeadlights = !autoHeadlights;
    lightsChanged = true;
    Serial.println(autoHeadlights ? "Headlights: AUTO  (on only while moving)"
                                  : "Headlights: ALWAYS ON");
  }

  if (justPressed(Ps3.data.button.circle, hazardButtonWasDown)) {
    hazardsOn = !hazardsOn;
    hazardBlinkOn = hazardsOn;
    lastHazardBlink = millis();
    lightsChanged = true;
    Serial.println(hazardsOn ? "Hazards ON" : "Hazards OFF");
  }

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

  // ---- Headlights only while we are moving, if auto mode is on ----
  bool wantHeadlights = !autoHeadlights || (leftSpeed != 0 || rightSpeed != 0);
  if (wantHeadlights != headlightsLit) {
    headlightsLit = wantHeadlights;
    lightsChanged = true;
  }

  // ---- Reversing beep ----
  if (BEEP_WHEN_REVERSING && lightPattern == LIGHTS_REVERSE) {
    unsigned long holdFor = beeping ? BEEP_ON_MS : BEEP_OFF_MS;
    if (millis() - lastBeepChange >= holdFor) {
      lastBeepChange = millis();
      beeping = !beeping;
      setBuzzer(beeping);
    }
  } else if (beeping) {
    beeping = false;
    setBuzzer(false);
  }

  // ---- Redraw. Hazards go on top, on their own clock. ----
  if (lightsChanged) {
    lightsChanged = false;
    showDrivingLights();
    if (hazardsOn) {
      showHazards();
    }
  }

  if (hazardsOn && millis() - lastHazardBlink >= HAZARD_BLINK_MS) {
    lastHazardBlink = millis();
    hazardBlinkOn = !hazardBlinkOn;
    showDrivingLights();
    showHazards();
  }
}
