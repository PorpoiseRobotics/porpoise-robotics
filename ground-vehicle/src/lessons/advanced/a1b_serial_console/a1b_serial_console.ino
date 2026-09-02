/*
  a1b_serial_console.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 1

  WHAT THIS PROGRAM DOES
  ----------------------
  Gives the ESP32 a command line. Open the Serial Monitor at 115200 baud, type
  "help", and press Enter. Nothing moves.

  This is the pattern behind Console.ino in Pathfinder_Op_Program12, where it
  is how you pair a controller, dump diagnostics and retune the deadzones at
  the track without a laptop full of toolchains. A console turns a compiled
  binary into something you can interrogate.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  COMMANDS
  --------
    help            list the commands
    status          show the current values
    echo <text>     print whatever you typed back
    set rate <n>    change the blink rate, in milliseconds
    set scale <f>   change a float, to show the parsing difference
    reset           put everything back to its compiled-in defaults

  THE THREE PROBLEMS A CONSOLE HAS TO SOLVE
  -----------------------------------------
  1. READING WITHOUT BLOCKING. Serial.readStringUntil() waits for a newline,
     and while it waits the vehicle is not driving. Instead we take whatever
     bytes have arrived, one pass at a time, and only act once we see the end
     of a line. Look at readSerialLine() - it returns immediately every time.

  2. LINE ENDINGS. The Serial Monitor sends whatever its dropdown is set to:
     nothing, LF, CR, or both. A parser that only handles one of those will
     look broken to whoever set theirs differently. This one accepts all four.

  3. PARSING. A command is a verb and some arguments. startsWith() and
     substring() are enough for a handful of commands and are far easier to
     read than anything cleverer. toInt() and toFloat() return 0 for anything
     they cannot parse, so a value that matters gets range checked rather than
     trusted - the same defensive habit loadSettings() uses in Op 12 when it
     reads a brand new EEPROM full of 0xFF.

  WHAT TO TRY
  -----------
  1. Try every command. Then type something that is not a command.
  2. Type "set rate abc". What happens, and why does the range check matter?
  3. Change your Serial Monitor line ending setting to each of its four
     options in turn. Does the console still work?
  4. Add a command of your own: "toggle" that flips a bool.
  5. Open Console.ino in Op Program 12 and find the same three problems being
     solved there.
*/

// --- Compiled-in defaults -------------------------------------------
const unsigned long DEFAULT_RATE_MS = 500;
const float         DEFAULT_SCALE   = 1.0f;

const unsigned long RATE_MIN = 20, RATE_MAX = 5000;
const float         SCALE_MIN = 0.2f, SCALE_MAX = 1.2f;

const int LED_PIN = 2;

// --- Live values ----------------------------------------------------
unsigned long blink_rate_ms = DEFAULT_RATE_MS;
float         scale_factor  = DEFAULT_SCALE;

unsigned long last_blink = 0;
bool led_is_on = false;

// A place to build up the line as its characters arrive.
String input_line = "";

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);

  Serial.println();
  Serial.println(F("=== Pathfinder serial console ==="));
  Serial.println(F("Type 'help' and press Enter."));
  printPrompt();
}

void loop() {
  unsigned long now = millis();

  // The console never stops the blinker, and the blinker never stops the
  // console. That is the whole point.
  handleSerial();

  if (now - last_blink >= blink_rate_ms) {
    last_blink = now;
    led_is_on = !led_is_on;
    digitalWrite(LED_PIN, led_is_on ? HIGH : LOW);
  }
}

// ===================================================================
// READING
// ===================================================================

/*
  Takes whatever characters have arrived since last time and adds them to
  input_line. When a line ending shows up, hands the finished line to
  runCommand() and starts a fresh one.

  Returns immediately either way, so loop() keeps running at full speed.
*/
void handleSerial() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    // Accept CR, LF, or both, so every Serial Monitor setting works.
    if (c == '\n' || c == '\r') {
      if (input_line.length() > 0) {
        Serial.println();          // Echo the newline the user pressed
        runCommand(input_line);
        input_line = "";
        printPrompt();
      }
      continue;
    }

    // Backspace, so a typo is fixable.
    if (c == 8 || c == 127) {
      if (input_line.length() > 0) {
        input_line.remove(input_line.length() - 1);
        Serial.print(F("\b \b"));
      }
      continue;
    }

    // Ignore anything that is not printable, and cap the length so a stuck
    // sender cannot grow the string until the ESP32 runs out of memory.
    if (c >= 32 && c < 127 && input_line.length() < 80) {
      input_line += c;
      Serial.print(c);             // Echo, so the user can see what they typed
    }
  }
}

void printPrompt() {
  Serial.print(F("> "));
}

// ===================================================================
// PARSING
// ===================================================================

void runCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "help") {
    printHelp();

  } else if (line == "status") {
    printStatus();

  } else if (line == "reset") {
    blink_rate_ms = DEFAULT_RATE_MS;
    scale_factor  = DEFAULT_SCALE;
    Serial.println(F("Back to defaults."));

  } else if (line.startsWith("echo ")) {
    Serial.println(line.substring(5));

  } else if (line.startsWith("set rate ")) {
    // toInt() returns 0 for anything it cannot parse, so the range check below
    // is doing double duty: it catches both nonsense and out-of-range values.
    long value = line.substring(9).toInt();
    if (value >= (long)RATE_MIN && value <= (long)RATE_MAX) {
      blink_rate_ms = (unsigned long)value;
      Serial.printf("rate = %lu ms\n", blink_rate_ms);
    } else {
      Serial.printf("rate must be %lu to %lu\n", RATE_MIN, RATE_MAX);
    }

  } else if (line.startsWith("set scale ")) {
    float value = line.substring(10).toFloat();
    if (value >= SCALE_MIN && value <= SCALE_MAX) {
      scale_factor = value;
      Serial.printf("scale = %.2f\n", scale_factor);
    } else {
      Serial.printf("scale must be %.1f to %.1f\n", SCALE_MIN, SCALE_MAX);
    }

  } else {
    Serial.print(F("Unknown command: "));
    Serial.println(line);
    Serial.println(F("Type 'help'."));
  }
}

// ===================================================================
// OUTPUT
// ===================================================================

void printHelp() {
  Serial.println(F("  help            this list"));
  Serial.println(F("  status          show the current values"));
  Serial.println(F("  echo <text>     print it back"));
  Serial.println(F("  set rate <n>    blink rate in ms, 20 to 5000"));
  Serial.println(F("  set scale <f>   a float, 0.2 to 1.2"));
  Serial.println(F("  reset           back to compiled-in defaults"));
}

void printStatus() {
  Serial.printf("rate  = %lu ms\n", blink_rate_ms);
  Serial.printf("scale = %.2f\n",   scale_factor);
  Serial.printf("up    = %lu s\n",  millis() / 1000);
}
