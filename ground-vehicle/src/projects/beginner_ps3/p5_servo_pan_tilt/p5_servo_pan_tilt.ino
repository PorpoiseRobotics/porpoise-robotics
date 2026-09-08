/*
  p5_servo_pan_tilt.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track)
  TAKE IT FURTHER project - see Lesson 5, "projects that fit on this vehicle"

  WHAT THIS PROGRAM DOES
  ----------------------
  Aims a two-servo pan and tilt bracket with the RIGHT thumbstick.

  Left and right on the stick pans; up and down tilts. Both servos ease
  toward where you point rather than snapping there, so a camera or a
  range finder on the bracket is not shaken to pieces.

  This is l5c_drive_with_lights with one thing added. Everything else in
  the file you have already met. Open the two side by side and the
  difference is the project.

  WHAT YOU HAVE TO WIRE UP
  ------------------------
  Two hobby servos in a pan and tilt bracket.

  The FOUR SERVO HEADERS ARE ON THE MAIN CONTROL BOARD, under the top
  plate - not on the top plate itself. The bracket goes on the top plate,
  and its leads come down to the board through the cutout.

      pan servo   ->  servo header 1  (GPIO 25)
      tilt servo  ->  servo header 2  (GPIO 26)

  WATCH OUT for which kind of servo you have. A STANDARD servo reads the
  pulse as a POSITION and holds it. A CONTINUOUS ROTATION servo reads the
  same pulse as a SPEED and will just keep turning. They look identical.
  If the bracket never stops moving, that is which one you have.

  Nothing plugged in? The pins pulse away to nobody and the vehicle
  drives as normal.

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
    Right stick         Pan and tilt
    TRIANGLE            Position control / rate control
    CIRCLE              Center both servos

  THE IDEA
  --------
  A servo wants one pulse every 20 milliseconds, and the LENGTH of that
  pulse is the angle: 1.0 ms hard over one way, 1.5 ms centered, 2.0 ms
  hard over the other. The ESP32 does not think in microseconds, it thinks
  in duty counts, so:

      counts = microseconds x 65536 / 20000

  which is the one line of arithmetic in writeServo(). It is done in long
  rather than int because 1500 x 65536 does not fit in an int.

  Two ideas here are worth more than the servos.

  TRAVEL LIMITS. PAN_MIN_US and PAN_MAX_US are narrower than the servo
  can actually go, because a bracket usually cannot go as far as the
  servo can, and a servo held against its own end stop draws current and
  cooks. Find your bracket's real limits and put them here.

  SLEW LIMITING. The target moves as fast as your thumb does; the servo
  is only allowed to move SERVO_STEP_US per update. That turns a flick of
  the stick into a smooth sweep, and it is the same idea as the speed
  ramping in the advanced course.

  TWO KINDS OF CONTROL. Lesson 3 said there are two, and this program has
  both, on a button:

    POSITION control (the default). Where the stick is, is where the
    bracket points. Let go and it springs back to the middle. Good for
    looking around quickly, useless for holding a shot.

    RATE control. The stick says how FAST to move, and the bracket stays
    where you left it. Good for aiming at something and keeping it there,
    and it is how nearly every real camera mount works.

  Neither is right. They are right for different jobs, which is the
  whole point of putting both on one vehicle.

  WHAT TO TRY
  -----------
  1. Set SERVO_STEP_US to 500 and flick the stick. Now set it to 3. Find
     the value you like.
  2. Narrow PAN_MIN_US and PAN_MAX_US until the bracket stops just short
     of its own stops, at both ends.
  3. Switch between position and rate control while aiming at something
     across the room. Which would you want for a camera? For a gripper?
  4. Reverse the tilt axis by swapping TILT_MIN_US and TILT_MAX_US. Which
     way round feels right to you? Aircraft and cameras disagree.
  5. Put a range finder on the bracket, sweep it, and print the distance
     against the angle. That is a scanning sonar.
  6. Make the bracket sweep by itself when nobody has touched the stick
     for five seconds, and hand control back the moment they do.
  7. Fold this into your copy of the full program - which already drives
     four servos, one per stick direction, in a different scheme. Compare
     the two and decide which you prefer.
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


// --- Servos ----------------------------------------------------------
// One pulse every 20 ms; the LENGTH of the pulse is the angle. The
// headers are on the MAIN CONTROL BOARD, under the top plate.
const int SERVO_PWM_FREQ = 50;      // 50 pulses a second is one every 20 ms
const int SERVO_PWM_BITS = 16;      // Plenty of resolution, so the angle is smooth

const int PAN_PIN  = 25;
const int TILT_PIN = 26;

// Travel limits, narrower than the servo can go, because your bracket
// probably cannot go as far as the servo can - and a servo held against a
// stop draws current and cooks. Find your own numbers and put them here.
const int PAN_MIN_US  = 1100;
const int PAN_MAX_US  = 1900;
const int TILT_MIN_US = 1200;
const int TILT_MAX_US = 1800;
const int SERVO_MID_US = 1500;

// How far a servo is allowed to move per update, and how often we update.
// This is what turns a flick of the stick into a sweep.
const int SERVO_STEP_US = 12;
const unsigned long SERVO_UPDATE_MS = 20;

// --- State ---
enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE, LIGHTS_LEFT, LIGHTS_RIGHT };

bool         lightsOn       = true;
int          headlightLevel = 80;
LightPattern lightPattern   = LIGHTS_STOPPED;
bool         lightsChanged  = true;

bool squareWasDown = false;
bool wasConnected  = false;


int panMicros    = SERVO_MID_US;   // Where the servo is now
int tiltMicros   = SERVO_MID_US;
int panTarget    = SERVO_MID_US;   // Where we are asking it to be
int tiltTarget   = SERVO_MID_US;
unsigned long lastServoUpdate = 0;

// false = position control, the stick IS the angle and it springs back.
// true  = rate control, the stick is a speed and the bracket stays put.
bool rateControl = false;

bool modeButtonWasDown   = false;
bool centerButtonWasDown = false;

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
  Sends one servo pulse of the requested length.

  The ESP32 counts duty, not microseconds. With 16 bits a whole 20 ms cycle is
  65536 counts, so counts = microseconds x 65536 / 20000. The math is done in
  long because 1500 x 65536 is far too big for an int.
*/
void writeServo(int target, int microseconds) {
  long duty = (long)microseconds * 65536L / 20000L;
  ledcWrite(target, duty);
}

/*
  Turns a thumbstick reading into a pulse length between two limits.
  A centered stick gives a centered servo.
*/
int stickToMicroseconds(int stickValue, int minUs, int maxUs) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return (minUs + maxUs) / 2;
  }
  int value = constrain(stickValue, -STICK_MAX, STICK_MAX);
  return map(value, -STICK_MAX, STICK_MAX, minUs, maxUs);
}

/*
  Moves one servo at most `step` microseconds toward where it is wanted.

  Returns where it now is. Calling this on a clock rather than every pass of
  loop() is what makes the speed of the sweep predictable - loop() runs at
  whatever rate it happens to run at, and 20 ms is 20 ms.
*/
int easeToward(int now, int target, int step) {
  if (target > now) return min(now + step, target);
  if (target < now) return max(now - step, target);
  return now;
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

  ledcAttach(PAN_PIN,  SERVO_PWM_FREQ, SERVO_PWM_BITS);
  ledcAttach(TILT_PIN, SERVO_PWM_FREQ, SERVO_PWM_BITS);
  writeServo(PAN_PIN, panMicros);
  writeServo(TILT_PIN, tiltMicros);

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

  if (justPressed(Ps3.data.button.triangle, modeButtonWasDown)) {
    rateControl = !rateControl;
    Serial.println(rateControl ? "Servos: RATE control  (the bracket stays put)"
                               : "Servos: POSITION control  (springs back)");
  }

  if (justPressed(Ps3.data.button.circle, centerButtonWasDown)) {
    panTarget  = SERVO_MID_US;
    tiltTarget = SERVO_MID_US;
    Serial.println("Servos centered.");
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

  // ---- Aim the bracket with the right stick ----
  int stickPan  = Ps3.data.analog.stick.rx;
  int stickTilt = Ps3.data.analog.stick.ry;

  // All of this is on a 20 ms clock, and that is not a detail. loop() runs at
  // whatever rate it happens to run at - tens of thousands of times a second -
  // so anything that moves "a bit each time round" moves instantly. 20 ms is
  // 20 ms, which is what makes the speed of the sweep a number you can set.
  if (millis() - lastServoUpdate >= SERVO_UPDATE_MS) {
    lastServoUpdate = millis();

    if (rateControl) {
      // The stick is a SPEED. Nudge the target while it is held over, and
      // leave it exactly where it was when the stick is let go.
      if (abs(stickPan) >= STICK_DEADZONE) {
        panTarget += (stickPan > 0) ? SERVO_STEP_US : -SERVO_STEP_US;
      }
      if (abs(stickTilt) >= STICK_DEADZONE) {
        tiltTarget += (stickTilt > 0) ? SERVO_STEP_US : -SERVO_STEP_US;
      }
      panTarget  = constrain(panTarget,  PAN_MIN_US,  PAN_MAX_US);
      tiltTarget = constrain(tiltTarget, TILT_MIN_US, TILT_MAX_US);
    } else {
      // The stick IS the angle, so letting go springs the bracket back.
      panTarget  = stickToMicroseconds(stickPan,  PAN_MIN_US,  PAN_MAX_US);
      tiltTarget = stickToMicroseconds(stickTilt, TILT_MIN_US, TILT_MAX_US);
    }

    // The target can still jump as fast as a thumb. The servo may not.
    panMicros  = easeToward(panMicros,  panTarget,  SERVO_STEP_US);
    tiltMicros = easeToward(tiltMicros, tiltTarget, SERVO_STEP_US);
    writeServo(PAN_PIN, panMicros);
    writeServo(TILT_PIN, tiltMicros);
  }

  // ---- Redraw, but only if something actually changed ----
  if (lightsChanged) {
    lightsChanged = false;
    showDrivingLights();
  }
}
