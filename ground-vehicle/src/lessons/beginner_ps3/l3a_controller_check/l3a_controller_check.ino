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
  1. Board package: "esp32" by Espressif Systems, VERSION 3.3.12
       Tools > Board > esp32 > "ESP32 Dev Module".
  2. Library: "Fork of PS3 Controller Host" by Jeffrey van Pernis
       Tools > Manage Libraries, and search for "PS3 Controller Host". Two
       come up. Install the one whose name starts "Fork of".
       If the original "PS3 Controller Host" is installed, REMOVE it first.
       It does not compile on board package 3.3, and with both installed
       the IDE picks the old one and fails with an error that mentions
       esp_base_mac_addr_set.

  BEFORE YOU UPLOAD
  -----------------
  Set PS3_MAC_ADDRESS, just below the #include lines, to YOUR controller's
  address: the number on its sticker goes in place of the 00 at the end.
  You do this in every program that uses the controller. Left at 00, the
  controller will not connect.

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
// Keep this line. It tells the ESP32 to keep its Bluetooth memory - without
// it the board frees that memory at power-on and no controller can connect.
#include "esp32-hal-alloc-bt-classic-mem.h"

// ======================== CHANGE THIS BEFORE YOU UPLOAD ========================
// Your controller only talks to ONE address, and this has to be it. Replace
// the 00 at the end with the number on YOUR controller's sticker - controller
// 7 is "02:02:03:04:05:07". Every vehicle in the room needs a different number,
// and left at 00 nothing will connect.
// ==============================================================================
const char *PS3_MAC_ADDRESS = "02:02:03:04:05:00";

// Remembers what each button was doing last time round, so we can print only
// when something CHANGES rather than thousands of times a second.
bool wasCross = false, wasCircle = false, wasSquare = false, wasTriangle = false;
bool wasUp = false, wasDown = false, wasLeft = false, wasRight = false;
bool wasL1 = false, wasR1 = false, wasL2 = false, wasR2 = false;
bool wasL3 = false, wasR3 = false;
bool wasSelect = false, wasStart = false, wasPs = false;

int lastLx = 0, lastLy = 0, lastRx = 0, lastRy = 0;
bool wasConnected = false;

// Sticks and triggers move all the time. Printing every change would scroll
// faster than anybody can read, so they are printed at most once every
// PRINT_MS milliseconds. Make it 20 and see for yourself.
const unsigned long PRINT_MS = 250;
unsigned long lastPrint = 0;

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

  if (!Ps3.begin(PS3_MAC_ADDRESS)) {
    Serial.println("Bluetooth did not start. Is the esp32-hal-alloc-bt-classic-mem.h line still at the top?");
  }

  Serial.println();
  Serial.println("=== Controller check ===");
  Serial.print("This program is listening as ");
  Serial.print(PS3_MAC_ADDRESS);
  Serial.println("  - it must match the number on your controller.");
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

  // --- Thumbsticks and triggers, at most once every PRINT_MS ---
  if (millis() - lastPrint >= PRINT_MS) {
    lastPrint = millis();

    // Each axis reports a whole number from -128 to +127. Print only when
    // a stick has moved by more than 4 since the last line about it.
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

    // The triggers are not just on or off. L2 and R2 also report HOW HARD
    // they are squeezed, 0 to 255.
    if (Ps3.data.button.l2) {
      Serial.print("L2 pressure: ");
      Serial.println(Ps3.data.analog.button.l2);
    }
    if (Ps3.data.button.r2) {
      Serial.print("R2 pressure: ");
      Serial.println(Ps3.data.analog.button.r2);
    }
  }

  delay(20);   // A short pause before looking at the controller again
}
