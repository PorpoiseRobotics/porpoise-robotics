/*
  l3a_controller_check.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Two jobs at once:

    1. It prints the BLUETOOTH ADDRESS of whatever controller connects to it,
       formatted as a line you can paste straight into the other programs. You
       need that address for every sketch from here on, and for
       pathfinder_nintendoswitch.ino itself.

    2. It prints every button press, every button release and every thumbstick
       movement, so you can find out what number the controller actually sends
       when you do a thing.

  No motors, no servos, no LEDs. Safe with the vehicle on the bench.

  This is a trimmed-down version of the pathfinder_find_controller tool that
  already lives in this repository. That one is for setting up a whole box of
  controllers; this one is for understanding what a controller sends.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none. Bluepad32 arrives with the board package.

  THIS SKETCH ACCEPTS ANY CONTROLLER, ON PURPOSE
  ----------------------------------------------
  Every other program locks itself to ONE controller. This one cannot, because
  its whole job is to tell you the address of a controller you do not know yet.

  In a classroom with several vehicles running this at once, controllers will
  connect to whichever board they reach first. If the address that prints is
  not the controller in your hands, switch the other boards off, or take turns.

  HOW TO CONNECT A SWITCH CONTROLLER
  ----------------------------------
  Hold the small round SYNC button down until the lights on the controller
  start running back and forth. It will connect within a few seconds.

  WHAT TO TRY
  -----------
  1. Connect your controller, copy the MY_CONTROLLER line that prints, and keep
     it somewhere safe. You will paste it into the next sketch.
  2. Press every button and match it to the name that prints. Pay attention to
     the face buttons - see the note in the code about why the letters do not
     match what is printed on the pad.
  3. Push the left stick fully UP. Is the number positive or negative? This
     surprises everybody, and it is why the driving code has a minus sign in it.
  4. Let go of the sticks and leave the controller alone. Does it print zero,
     or does it drift by one or two? That drift is what a DEADZONE is for.
  5. Compare the stick numbers with what a friend on the PS3 track sees. Theirs
     run to 127, yours run to 511. Why does that matter?
*/

#include <Bluepad32.h>

ControllerPtr myController = nullptr;

// Remembers what each button was doing last time round, so we can print only
// when something CHANGES rather than thousands of times a second.
bool wasA = false, wasB = false, wasX = false, wasY = false;
bool wasL1 = false, wasR1 = false, wasL2 = false, wasR2 = false;
bool wasThumbL = false, wasThumbR = false;
bool wasStart = false, wasBack = false, wasHome = false;
bool wasUp = false, wasDown = false, wasLeft = false, wasRight = false;

int lastLx = 0, lastLy = 0, lastRx = 0, lastRy = 0;

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

/*
  Bluepad32 calls this by itself whenever a controller connects. We never call
  it ourselves - it is a "callback", a function we hand over for someone else
  to call at the right moment.
*/
void onConnectedController(ControllerPtr controller) {
  if (myController != nullptr) {
    Serial.println("A second controller tried to connect. Refused.");
    controller->disconnect();
    return;
  }

  myController = controller;
  ControllerProperties properties = controller->getProperties();

  Serial.println();
  Serial.print("Connected: ");
  Serial.println(controller->getModelName());

  Serial.println();
  Serial.println("Copy this line into the top of the other Lesson 3 sketches:");
  Serial.println();
  Serial.print("const uint8_t MY_CONTROLLER[6] = { ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", properties.btaddr[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println(" };");
  Serial.println();
  Serial.println("Now start pressing things.");
}

void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
    Serial.println("Controller disconnected.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);      // We only want gamepads, not virtual mice
  BP32.forgetBluetoothKeys();           // Start clean so any pad can connect
  BP32.enableNewBluetoothConnections(true);

  Serial.println();
  Serial.println("=== Controller check ===");
  Serial.println("Hold the SYNC button on your controller until its lights run.");
}

void loop() {
  // Ask Bluepad32 for fresh data. This is also where it calls the two
  // callbacks above for us.
  BP32.update();

  if (myController == nullptr || !myController->isConnected()) {
    delay(100);
    return;
  }

  // --- The four face buttons ---
  // Bluepad32 names these by POSITION, Xbox style, not by the letter printed
  // on your controller:
  //        a() = bottom     b() = right     x() = left     y() = top
  // Most Switch style pads print  bottom = B, right = A, left = Y, top = X.
  // So y() below is the button marked X on the pad. Press them and see.
  reportButton("A - bottom face button", myController->a(), wasA);
  reportButton("B - right face button",  myController->b(), wasB);
  reportButton("X - left face button",   myController->x(), wasX);
  reportButton("Y - top face button",    myController->y(), wasY);

  // --- Shoulders and triggers ---
  reportButton("L  (left shoulder)",  myController->l1(), wasL1);
  reportButton("R  (right shoulder)", myController->r1(), wasR1);
  reportButton("ZL (left trigger)",   myController->l2(), wasL2);
  reportButton("ZR (right trigger)",  myController->r2(), wasR2);

  // --- Clicking the sticks in ---
  reportButton("left stick click",  myController->thumbL(), wasThumbL);
  reportButton("right stick click", myController->thumbR(), wasThumbR);

  // --- The small ones in the middle ---
  reportButton("PLUS / start", myController->miscStart(), wasStart);
  reportButton("MINUS / back", myController->miscBack(),  wasBack);
  reportButton("HOME",         myController->miscHome(),  wasHome);

  // --- D-pad ---
  // The D-pad arrives as one number with a bit set per direction, so we test
  // each direction with "&". That is how you can press up and left together.
  uint8_t dpad = myController->dpad();
  reportButton("D-PAD UP",    dpad & DPAD_UP,    wasUp);
  reportButton("D-PAD DOWN",  dpad & DPAD_DOWN,  wasDown);
  reportButton("D-PAD LEFT",  dpad & DPAD_LEFT,  wasLeft);
  reportButton("D-PAD RIGHT", dpad & DPAD_RIGHT, wasRight);

  // --- Thumbsticks ---
  // Bluepad32 reports every stick axis as -512 to +511, whatever brand of
  // controller you plug in. Print only when something has moved by more than
  // 15, or the Serial Monitor scrolls forever.
  int lx = myController->axisX();
  int ly = myController->axisY();
  int rx = myController->axisRX();
  int ry = myController->axisRY();

  if (abs(lx - lastLx) > 15 || abs(ly - lastLy) > 15) {
    Serial.print("LEFT stick    x = ");  Serial.print(lx);
    Serial.print("\t y = ");             Serial.println(ly);
    lastLx = lx;
    lastLy = ly;
  }

  if (abs(rx - lastRx) > 15 || abs(ry - lastRy) > 15) {
    Serial.print("RIGHT stick   x = ");  Serial.print(rx);
    Serial.print("\t y = ");             Serial.println(ry);
    lastRx = rx;
    lastRy = ry;
  }

  delay(20);   // Slow the loop down a little so the output stays readable
}
