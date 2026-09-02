/*
  a5a_i2c_scan.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 5

  WHAT THIS PROGRAM DOES
  ----------------------
  Walks every address on the I2C bus and reports which ones answer. On a Gen 3
  vehicle you should find the INA219 current sensor at 0x40. On a Gen 2 you
  should find nothing, and that is the correct answer. Nothing moves.

  This is the first thing to run when a sensor is not working, before you
  suspect anything in your own code.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none. Wire ships with the core.

  COMMANDS (115200 baud)
  ----------------------
    scan            sweep the bus once
    watch           scan every two seconds until you type 'stop'
    stop
    pins <sda> <scl>  move the bus and try again
    speed <hz>      bus clock, 100000 or 400000

  HOW I2C ADDRESSING WORKS
  ------------------------
  Two wires, SDA and SCL, shared by every device. Each device answers to a
  7-bit address, so there are 128 of them, and 0x00-0x07 and 0x78-0x7F are
  reserved. That leaves 0x08 to 0x77 to scan.

  To test an address you start a transmission to it and end it immediately
  without sending any data. If a device is there it pulls SDA low to
  acknowledge, and endTransmission() returns 0. If nothing is there, nobody
  acknowledges and you get 2.

        0  acknowledged - something is there
        1  the data was too long for the buffer
        2  no acknowledge for the address
        3  no acknowledge for the data
        4  some other error
        5  timeout

  That is exactly what is_ina219_present() does in Op Program 12, and it is how
  the program works out which generation of vehicle it is running on. One
  binary, two vehicles, decided at boot by whether anybody answers at 0x40.

  THE PULL-UP RESISTORS
  ---------------------
  I2C devices can only ever pull a line DOWN. Something has to pull it back up,
  and that is a pair of resistors to 3.3 V, usually 4.7k. Without them both
  lines sit low and the scan finds nothing at all. On the Pathfinder they are
  on the control board, so this only matters when you add your own sensor on
  the breadboard - and a scan that returns absolutely nothing, on every
  address, is the classic symptom of a missing pull-up.

  WHAT TO TRY
  -----------
  1. Scan a Gen 3 vehicle and a Gen 2 vehicle. What is the difference?
  2. Type "watch", then unplug the sensor. How quickly does it notice?
  3. Try "pins 21 22", the ESP32 default. Does the sensor still answer?
  4. Add a second I2C sensor on the breadboard and find its address.
*/

#include <Wire.h>

// The Pathfinder control board puts I2C here, not on the ESP32 defaults.
int sda_pin = 32;
int scl_pin = 33;
long bus_speed = 100000;

bool watching = false;
unsigned long last_scan = 0;

String input_line = "";

/*
  A few addresses worth naming, so a hit means something without a datasheet.
*/
const char *knownDevice(uint8_t address) {
  switch (address) {
    case 0x40: return "INA219 current sensor (or default INA219 address)";
    case 0x41: case 0x44: case 0x45: return "INA219 with address jumpers set";
    case 0x23: return "BH1750 light sensor";
    case 0x77: return "BMP180 / BMP280 pressure sensor";
    case 0x76: return "BMP280 / BME280, alternate address";
    case 0x68: return "MPU6050 IMU or DS3231 clock";
    case 0x3C: return "SSD1306 OLED display";
    default:   return nullptr;
  }
}

void scanBus() {
  Serial.printf("Scanning SDA %d, SCL %d, at %ld Hz...\n", sda_pin, scl_pin, bus_speed);

  int found = 0;
  for (uint8_t address = 0x08; address <= 0x77; address++) {
    Wire.beginTransmission(address);
    uint8_t result = Wire.endTransmission();

    if (result == 0) {
      found++;
      Serial.printf("  0x%02X  answered", address);
      const char *name = knownDevice(address);
      if (name != nullptr) {
        Serial.print("   -   ");
        Serial.print(name);
      }
      Serial.println();

    } else if (result != 2) {
      // Anything other than "no acknowledge" is worth mentioning.
      Serial.printf("  0x%02X  error %d\n", address, result);
    }
  }

  if (found == 0) {
    Serial.println(F("  nothing answered."));
    Serial.println(F("  On a Gen 2 vehicle that is correct - there is no sensor."));
    Serial.println(F("  On a Gen 3, check the pins above, and check the pull-ups."));
  } else {
    Serial.printf("  %d device%s found.\n", found, found == 1 ? "" : "s");
  }
}

void restartBus() {
  Wire.end();
  Wire.begin(sda_pin, scl_pin);
  Wire.setClock(bus_speed);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(sda_pin, scl_pin);
  Wire.setClock(bus_speed);

  Serial.println();
  Serial.println(F("=== I2C bus scan ==="));
  scanBus();
  Serial.println(F("Type 'help'."));
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (input_line.length() > 0) {
        runCommand(input_line);
        input_line = "";
      }
    } else if (c >= 32 && c < 127 && input_line.length() < 40) {
      input_line += c;
    }
  }

  if (watching && millis() - last_scan >= 2000) {
    last_scan = millis();
    scanBus();
  }
}

void runCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "help") {
    Serial.println(F("  scan                 sweep once"));
    Serial.println(F("  watch                sweep every two seconds"));
    Serial.println(F("  stop"));
    Serial.println(F("  pins <sda> <scl>     move the bus"));
    Serial.println(F("  speed <hz>           100000 or 400000"));

  } else if (line == "scan") {
    scanBus();

  } else if (line == "watch") {
    watching = true;
    last_scan = 0;
    Serial.println(F("watching. Type 'stop'."));

  } else if (line == "stop") {
    watching = false;
    Serial.println(F("stopped"));

  } else if (line.startsWith("pins ")) {
    String rest = line.substring(5);
    rest.trim();
    int space = rest.indexOf(' ');
    if (space < 0) {
      Serial.println(F("Use: pins <sda> <scl>"));
      return;
    }
    int sda = rest.substring(0, space).toInt();
    int scl = rest.substring(space + 1).toInt();
    if (sda < 0 || sda > 39 || scl < 0 || scl > 39) {
      Serial.println(F("Pins must be 0 to 39."));
      return;
    }
    // GPIO 34-39 are input-only on this chip, so they cannot drive a bus.
    if (sda >= 34 || scl >= 34) {
      Serial.println(F("GPIO 34-39 are input-only and cannot drive I2C."));
      return;
    }
    sda_pin = sda;
    scl_pin = scl;
    restartBus();
    scanBus();

  } else if (line.startsWith("speed ")) {
    long value = line.substring(6).toInt();
    if (value != 100000 && value != 400000) {
      Serial.println(F("Use 100000 or 400000."));
      return;
    }
    bus_speed = value;
    restartBus();
    scanBus();

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
