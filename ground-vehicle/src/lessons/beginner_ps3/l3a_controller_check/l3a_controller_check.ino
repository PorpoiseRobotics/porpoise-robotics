/*
  l3a_controller_check.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Connects to your PS3 controller and prints every button press, every button
  release, and every thumbstick movement to the Serial Monitor. No motors, no
  servos, no LEDs - just the controller. It is completely safe with the vehicle
  on the bench.

  Use it to answer the question "what number does the controller actually send
  when I do that?" before you try to make the vehicle react to it.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Library: "PS3 Controller Host" by Jeffrey van Pernis
       Tools > Manage Libraries. Searching for "Ps3Controller" finds nothing -
       the display name is the longer one above.

  PAIRING
  -------
  A PS3 controller only ever talks to ONE Bluetooth address, and it has to be
  told that address over a USB cable using SixaxisPairTool on a Windows PC.
  Whatever address you write into the controller has to match PS3_MAC_ADDRESS
  below. Any address works as long as the two agree - but give every vehicle in
  the room a different one, or two vehicles will answer the same controller.

  The first pair of digits must be an EVEN number. This is a quirk of the
  Bluetooth stack, and using an odd one is a very confusing way to spend an
  afternoon.

  WHAT TO TRY
  -----------
  1. Press every button in turn and match it to the name that prints.
  2. Push the left stick fully UP. Is the number positive or negative? This
     surprises everybody, and it is why the driving code has a minus sign in it.
  3. Let go of the sticks and leave the controller alone. Does it print zero,
     or does it drift by one or two? That drift is what a DEADZONE is for.
  4. Push a stick diagonally. What are x and y doing at the same time?
  5. Squeeze L2 slowly. It is not just on or off - watch the number climb.
*/

#include <Ps3Controller.h>

// This has to match the address written into your controller.
const char *PS3_MAC_ADDRESS = "02:02:03:04:05:08";

// Remembers what each button was doing last time round, so we can print only
// when something CHANGES rather than thousands of times a second.
bool wasCross = false, wasCircle = false, wasSquare = false, wasTriangle = false;
bool wasUp = false, wasDown = false, wasLeft = false, wasRight = false;
bool wasL1 = false, wasR1 = false, wasL2 = false, wasR2 = false;
bool wasL3 = false, wasR3 = false;
bool wasSelect = false, wasStart = false, wasPs = false;

int lastLx = 0, lastLy = 0, lastRx = 0, lastRy = 0;
bool wasConnected = false;

/*
  Prints one line when a button goes down and one when it comes back up.
  The "&" on wasDown means this function is handed the actual variable, not a
  copy, so the value it stores is remembered for next time.
*/
void reportButton(const char *name, bool isDown, bool &wasDown) {
  if (isDown && !wasDown) {
    Serial.print("PRESSED   ");
    Serial.println(name);
  } else if (!isDown && wasDown) {
    Serial.print("released  ");
    Serial.println(name);
  }
  wasDown = isDown;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Ps3.begin(PS3_MAC_ADDRESS);

  Serial.println();
  Serial.println("=== Controller check ===");
  Serial.print("This program is listening as ");
  Serial.println(PS3_MAC_ADDRESS);
  Serial.println("Press the PS button on your controller now.");
}

void loop() {
  if (!Ps3.isConnected()) {
    if (wasConnected) {
      Serial.println("Controller disconnected.");
      wasConnected = false;
    }
    delay(200);
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    Ps3.setPlayer(1);          // Lights up player LED 1 on the controller
    Serial.println("Controller connected. Start pressing things.");
  }

  // --- The four face buttons ---
  reportButton("CROSS",    Ps3.data.button.cross,    wasCross);
  reportButton("CIRCLE",   Ps3.data.button.circle,   wasCircle);
  reportButton("SQUARE",   Ps3.data.button.square,   wasSquare);
  reportButton("TRIANGLE", Ps3.data.button.triangle, wasTriangle);

  // --- The D-pad ---
  reportButton("D-PAD UP",    Ps3.data.button.up,    wasUp);
  reportButton("D-PAD DOWN",  Ps3.data.button.down,  wasDown);
  reportButton("D-PAD LEFT",  Ps3.data.button.left,  wasLeft);
  reportButton("D-PAD RIGHT", Ps3.data.button.right, wasRight);

  // --- Shoulders and triggers ---
  reportButton("L1", Ps3.data.button.l1, wasL1);
  reportButton("R1", Ps3.data.button.r1, wasR1);
  reportButton("L2", Ps3.data.button.l2, wasL2);
  reportButton("R2", Ps3.data.button.r2, wasR2);

  // --- Clicking the sticks in ---
  reportButton("L3 (left stick click)",  Ps3.data.button.l3, wasL3);
  reportButton("R3 (right stick click)", Ps3.data.button.r3, wasR3);

  // --- The middle three ---
  reportButton("SELECT", Ps3.data.button.select, wasSelect);
  reportButton("START",  Ps3.data.button.start,  wasStart);
  reportButton("PS",     Ps3.data.button.ps,     wasPs);

  // --- Thumbsticks ---
  // Each axis reports a whole number from -128 to +127. Print only when
  // something has moved by more than 4, or the Serial Monitor scrolls forever.
  int lx = Ps3.data.analog.stick.lx;
  int ly = Ps3.data.analog.stick.ly;
  int rx = Ps3.data.analog.stick.rx;
  int ry = Ps3.data.analog.stick.ry;

  if (abs(lx - lastLx) > 4 || abs(ly - lastLy) > 4) {
    Serial.print("LEFT stick    x = ");  Serial.print(lx);
    Serial.print("\t y = ");             Serial.println(ly);
    lastLx = lx;
    lastLy = ly;
  }

  if (abs(rx - lastRx) > 4 || abs(ry - lastRy) > 4) {
    Serial.print("RIGHT stick   x = ");  Serial.print(rx);
    Serial.print("\t y = ");             Serial.println(ry);
    lastRx = rx;
    lastRy = ry;
  }

  // --- The triggers are not just on or off ---
  // L2 and R2 also report HOW HARD they are squeezed, 0 to 255.
  if (Ps3.data.button.l2) {
    Serial.print("L2 pressure: ");
    Serial.println(Ps3.data.analog.button.l2);
  }
  if (Ps3.data.button.r2) {
    Serial.print("R2 pressure: ");
    Serial.println(Ps3.data.analog.button.r2);
  }

  delay(20);   // Slow the loop down a little so the output stays readable
}
