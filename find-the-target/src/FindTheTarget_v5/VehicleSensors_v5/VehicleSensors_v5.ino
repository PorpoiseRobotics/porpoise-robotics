/* =====================================================================
 * VehicleSensors_v5  --  Find the Target v5, the vehicle's sensor board
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   Reads the range finder, the BMP280, the FK-A1 GPS and its compass
 *   once a second, lights the LED ring, and sends every reading over the
 *   PorpoiseNet radio network. Every station on the field can hear it,
 *   but only YOUR station's dashboard shows it as "my vehicle".
 *
 *   It is the same sensor code as ESPNow_Sender_v4. What changed:
 *     - No MAC address to paste. The board has a STATION NUMBER instead,
 *       saved in its memory, and finds its station radio by itself.
 *     - The first LED is a status light (see LEDS below).
 *     - It blinks when someone sends your station a chat or a map mark.
 *
 * ONE-TIME SETUP (each board, once)
 *   1. Upload this sketch. Every station uploads exactly the same file.
 *   2. Give the board its station number. Either:
 *        - plug it into the laptop and use the dashboard's "Set up a
 *          board" page, or
 *        - open Serial Monitor (115200 baud), type   station 3   and
 *          press Enter.
 *      The number is saved, so you never do this again for this board.
 *
 * LEDS (16 NeoPixels)
 *   At power-up : the ring lights N LEDs in your station's color, where
 *                 N is the station number. (Blinking orange = no station
 *                 number yet: do step 2 above.)
 *   Then        : green = path clear, yellow = getting close, red = very
 *                 close, dim blue = range finder hears no echo.
 *   LED 0       : status light. Your station color = talking to your
 *                 station radio. Blinking = cannot hear the station radio.
 *   Three white flashes = a new chat or map mark arrived at your station.
 *
 * BOARD / IDE SETTINGS
 *   Board:          "ESP32 Dev Module"
 *   Board package:  esp32 by Espressif Systems, 3.3.12
 *   Libraries:      Adafruit BMP280, Adafruit NeoPixel, TinyGPSPlus
 *                   (Tools > Manage Libraries). PorpoiseNet.h is already
 *                   in this folder; nothing to install for it.
 *   Serial Monitor: 115200 baud. Type  help  for the commands.
 *
 * WIRING  (same as v4 and the Rev 0.5 sensor board)
 *   HC-SR04   TRIG -> GPIO32, ECHO -> GPIO25 THROUGH A DIVIDER (5 V echo)
 *   BMP280    SDA -> GPIO21, SCL -> GPIO22
 *   FK-A1     Tx -> GPIO16, Rx -> GPIO18, SCL -> GPIO22, SDA -> GPIO21, 5V
 *   NeoPixel  data -> GPIO13
 * ===================================================================== */

#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_NeoPixel.h>
#include <TinyGPSPlus.h>
#include "PorpoiseNet.h"        // the copy in THIS folder

#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 0)
#error "Update the esp32 board package to 3.3.12: Tools > Board > Boards Manager"
#endif

#define FIRMWARE_VERSION "5.0"

// ====================== Network (same on EVERY board) ================
// These two numbers must match on every station radio and every vehicle
// on the field, or the boards cannot hear each other. Leave them alone
// unless two Find the Target games are running side by side.
#define FTT_NET_ID   50     // "Find the Target" network label
#define FTT_CHANNEL  1      // radio channel; set the field router to 6 or 11

// ============================== Pins =================================
#define TRIG_PIN    32
#define ECHO_PIN    25
#define I2C_SDA     21
#define I2C_SCL     22
#define GPS_RX_PIN  16      // ESP32 RX  <- FK-A1 Tx
#define GPS_TX_PIN  18      // ESP32 TX  -> FK-A1 Rx
#define LED_PIN     13

// ============================= Config ================================
#define NUM_LEDS          16
#define LED_BRIGHTNESS    8      // 0-255; keep low to save battery
#define SEND_INTERVAL_MS  1000   // one reading per second
#define GPS_BAUD          38400  // FK-A1 factory default
#define STATION_LOST_MS   10000  // no word from the station radio this long = "lost"

// Turn the vehicle to face a known direction (use a phone compass),
// compare with the dashboard's Heading, and put the difference here.
#define HEADING_OFFSET_DEG  0.0f

// Range finder colors (inches)
#define DIST_GREEN_IN     40.0f
#define DIST_YELLOW_IN    20.0f

// ===================== Station numbers and colors ====================
// Node numbers on the network: station radio = S, this board = 10 + S.
// The same colors are used on the dashboard, so a child can match
// "the blue vehicle" to "station 2" at a glance.
#define MAX_STATIONS 9
const uint8_t STATION_RGB[MAX_STATIONS + 1][3] = {
  {255, 120, 0},    // 0 = not set yet (orange)
  {230, 30, 30},    // 1 red
  {30, 90, 255},    // 2 blue
  {20, 200, 60},    // 3 green
  {255, 200, 0},    // 4 yellow
  {160, 60, 230},   // 5 purple
  {255, 110, 20},   // 6 orange
  {0, 200, 220},    // 7 cyan
  {255, 80, 170},   // 8 pink
  {200, 200, 200},  // 9 white
};

// ============================= Globals ===============================
Adafruit_BMP280   bmp;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
TinyGPSPlus       gps;
HardwareSerial    GPSserial(2);
Preferences       prefs;

uint8_t  station     = 0;      // 0 = not set yet
bool     meshUp      = false;
bool     bmpOk       = false;
bool     magOk       = false;
unsigned long lastSend          = 0;
unsigned long lastHeardStation  = 0;   // last message from our station radio
unsigned long flashUntil        = 0;   // "new message" flashes run until then
unsigned long identifyUntil     = 0;   // "which one am I?" flashing runs until then
uint32_t readingsSent = 0;
float    lastInches   = -1;

uint32_t stationColor(uint8_t s) {
  if (s > MAX_STATIONS) s = 0;
  return strip.Color(STATION_RGB[s][0], STATION_RGB[s][1], STATION_RGB[s][2]);
}

// ============================= Sensors ===============================
// One ultrasonic ping. Inches, or -1 if no echo came back (~5 m timeout).
float readDistanceInches() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (dur == 0) return -1.0f;
  return dur / 148.0f;      // round-trip microseconds -> inches
}

// The GPS streams text all the time; feed every byte to the parser.
void feedGps() {
  while (GPSserial.available()) gps.encode(GPSserial.read());
}

// ---- QMC5883L compass, plain I2C, no library (same as v4) ----
#define QMC_ADDR        0x0D
#define QMC_REG_DATA    0x00
#define QMC_REG_CTRL1   0x09
#define QMC_REG_CTRL2   0x0A
#define QMC_REG_PERIOD  0x0B

bool qmcWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(QMC_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool qmcBegin() {
  if (!qmcWrite(QMC_REG_CTRL2, 0x80)) return false;   // soft reset
  delay(10);
  qmcWrite(QMC_REG_PERIOD, 0x01);
  return qmcWrite(QMC_REG_CTRL1, 0x05);   // x512 oversample, 2 gauss, 50 Hz, continuous
}

// Heading 0-360 (0 = magnetic north), or -1 on error. Module must sit flat.
float readHeadingDeg() {
  if (!magOk) return -1.0f;
  Wire.beginTransmission(QMC_ADDR);
  Wire.write(QMC_REG_DATA);
  if (Wire.endTransmission(false) != 0) return -1.0f;
  if (Wire.requestFrom((uint8_t)QMC_ADDR, (uint8_t)6) != 6) return -1.0f;
  // One read per statement, so the bytes cannot swap order (see v4).
  uint8_t xl = Wire.read(), xh = Wire.read();
  uint8_t yl = Wire.read(), yh = Wire.read();
  Wire.read(); Wire.read();                      // Z: unused
  int16_t x = (int16_t)(xl | (xh << 8));
  int16_t y = (int16_t)(yl | (yh << 8));
  // The chip is upside down on the FK-A1 (antenna up), so Y is mirrored.
  float deg = atan2f(-(float)y, (float)x) * 180.0f / PI + HEADING_OFFSET_DEG;
  while (deg < 0)       deg += 360.0f;
  while (deg >= 360.0f) deg -= 360.0f;
  return deg;
}

// =============================== LEDs ================================
// Redrawn about 20 times a second, so the animations stay smooth. Order
// of importance: "which one am I" > new-message flash > range colors.
void drawLeds() {
  unsigned long now = millis();
  strip.clear();

  if (station == 0) {
    // No station number: slow orange blink on every LED.
    if ((now / 500) % 2 == 0)
      for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, stationColor(0));
    strip.show();
    return;
  }

  if (now < identifyUntil) {
    // Whole ring flashes the station color.
    if ((now / 250) % 2 == 0)
      for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, stationColor(station));
    strip.show();
    return;
  }

  if (now < flashUntil) {
    // Three white flashes: "look at the dashboard, a message came in".
    if ((now / 250) % 2 == 0)
      for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    return;
  }

  // Range finder colors on LEDs 1-15.
  uint32_t c;
  if (lastInches < 0)                  c = strip.Color(0, 0, 40);     // no echo
  else if (lastInches > DIST_GREEN_IN) c = strip.Color(0, 255, 0);    // clear
  else if (lastInches >= DIST_YELLOW_IN) c = strip.Color(255, 255, 0);// caution
  else                                 c = strip.Color(255, 0, 0);    // close
  for (int i = 1; i < NUM_LEDS; i++) strip.setPixelColor(i, c);

  // LED 0: status light. Steady = we hear our station radio.
  bool heard = lastHeardStation && (now - lastHeardStation) < STATION_LOST_MS;
  if (heard || (now / 400) % 2 == 0) strip.setPixelColor(0, stationColor(station));
  strip.show();
}

// At power-up: light N LEDs in the station color, so a child can check
// "this is the vehicle for station 3" before driving off.
void showStationNumber() {
  strip.clear();
  for (int i = 0; i < station && i < NUM_LEDS; i++) strip.setPixelColor(i, stationColor(station));
  strip.show();
  delay(3000);
}

// ============================ Self-test ==============================
void runSelfTest() {
  Serial.println(F("# ---- SELF-TEST ----"));
  Serial.print(F("# [BMP280 ]  "));
  Serial.println(bmpOk ? F("PASS  (found on I2C)")
                       : F("FAIL  (check SDA=21 SCL=22, addr 0x76/0x77)"));
  Serial.print(F("# [COMPASS]  "));
  if (magOk) {
    float h = readHeadingDeg();
    if (h >= 0) { Serial.print(F("PASS  (heading ")); Serial.print(h, 0); Serial.println(F(" deg)")); }
    else          Serial.println(F("WARN  (found, but no reading yet)"));
  } else {
    Serial.println(F("FAIL  (no answer at 0x0D - check FK-A1 SCL->22 SDA->21)"));
  }
  float d = readDistanceInches();
  Serial.print(F("# [HC-SR04]  "));
  if (d > 0) { Serial.print(F("PASS  (echo at ")); Serial.print(d, 1); Serial.println(F(" in)")); }
  else         Serial.println(F("WARN  (no echo - clear path >5m, or check TRIG=32 ECHO=25 divider)"));
  Serial.print(F("# [FK-A1  ]  "));
  unsigned long t0 = millis();
  bool gpsBytes = false;
  while (millis() - t0 < 1500) {
    if (GPSserial.available()) { gpsBytes = true; break; }
  }
  Serial.println(gpsBytes ? F("PASS  (GPS data flowing; a fix takes ~30 s outdoors)")
                          : F("FAIL  (no data - check FK-A1 Tx->GPIO16, power, 38400 baud)"));
  Serial.println(F("# [LEDs   ]  flashing white - check they all light"));
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(40, 40, 40));
  strip.show();
  delay(400);
  strip.clear();
  strip.show();
  Serial.println(F("# -------------------"));
}

// ========================== Sending readings =========================
void sendReading() {
  PnTelemetryFull t = {};
  float inches = readDistanceInches();
  lastInches = inches;
  if (inches >= 0) t.health |= PN_TFULL_RANGE_OK;
  t.rangeInches = inches;

  if (bmpOk) {
    t.health         |= PN_TFULL_BMP_OK;
    t.tempF           = bmp.readTemperature() * 9.0f / 5.0f + 32.0f;
    t.pressureAtm     = bmp.readPressure() / 101325.0f;
    t.bmpAltitudeFeet = bmp.readAltitude(1013.25f) * 3.28084f;
  }

  bool fix = gps.location.isValid();
  if (fix) t.health |= PN_TFULL_GPS_FIX;
  t.satellites      = gps.satellites.isValid() ? (uint8_t)min((uint32_t)255, gps.satellites.value()) : 0;
  t.latitude        = fix ? gps.location.lat() : 0.0;
  t.longitude       = fix ? gps.location.lng() : 0.0;
  t.speedMph        = gps.speed.isValid()    ? gps.speed.mph()     : 0.0f;
  t.gpsAltitudeFeet = gps.altitude.isValid() ? gps.altitude.feet() : 0.0f;
  t.courseDeg       = gps.course.isValid()   ? gps.course.deg()    : 0.0f;

  float heading = readHeadingDeg();
  if (heading >= 0) t.health |= PN_TFULL_COMPASS_OK;
  t.headingDeg  = heading;
  t.battPercent = 255;          // not measured on this board

  // Broadcast (dest 0): every station can show every vehicle on its map.
  PorpoiseNet.sendTelemetryFull(t);
  readingsSent++;

  // One short line a person can read in Serial Monitor.
  Serial.print(F("# #"));          Serial.print(readingsSent);
  Serial.print(F("  range="));
  if (inches >= 0) { Serial.print(inches, 1); Serial.print(F("in")); } else Serial.print(F("none"));
  Serial.print(F("  hdg="));       Serial.print(heading, 0);
  Serial.print(F("  sats="));      Serial.print(t.satellites);
  Serial.print(F("  fix="));       Serial.print(fix ? F("YES") : F("no"));
  Serial.print(F("  station radio: "));
  bool heard = lastHeardStation && (millis() - lastHeardStation) < STATION_LOST_MS;
  Serial.println(heard ? F("heard") : F("NOT HEARD (powered? in range?)"));
}

// ========================== Network messages =========================
void onNetworkMessage(const PnMessage &msg, int8_t rssi) {
  // Anything at all from our own station radio proves the link is up.
  if (msg.origin == station) lastHeardStation = millis();

  PnCommand c;
  if (!PorpoiseNet.asCommand(msg, c)) return;
  // Only our own station radio may make us blink.
  if (msg.origin == station && c.command == PN_CMD_NOTIFY)   flashUntil    = millis() + 1500;
  if (c.command == PN_CMD_IDENTIFY && msg.dest == PorpoiseNet.id()) identifyUntil = millis() + 3000;
}

// =========================== Serial commands =========================
void printHello() {
  // One JSON line so the dashboard's "Set up a board" page knows what
  // is plugged in. People can ignore it.
  Serial.print(F("{\"type\":\"hello\",\"board\":\"vehicle-sensors\",\"fw\":\""));
  Serial.print(F(FIRMWARE_VERSION));
  Serial.print(F("\",\"station\":")); Serial.print(station);
  Serial.print(F(",\"node\":"));      Serial.print(station ? 10 + station : 0);
  Serial.print(F(",\"net\":"));       Serial.print(FTT_NET_ID);
  Serial.print(F(",\"ch\":"));        Serial.print(FTT_CHANNEL);
  Serial.println(F("}"));
}

void printHelp() {
  Serial.println(F("# Commands (type one and press Enter):"));
  Serial.println(F("#   station 3   set this vehicle's station number (1-9) and restart"));
  Serial.println(F("#   info        show what this board is"));
  Serial.println(F("#   selftest    check every sensor again"));
  Serial.println(F("#   roster      list the boards this one can hear"));
}

void handleCommand(String line) {
  line.trim();
  String low = line; low.toLowerCase();
  if (low.startsWith("station")) {
    long n = line.substring(7).toInt();
    if (n < 1 || n > MAX_STATIONS) {
      Serial.println(F("# Station number must be 1 to 9, e.g.  station 3"));
      Serial.println(F("{\"type\":\"error\",\"text\":\"station must be 1-9\"}"));
      return;
    }
    prefs.putUChar("station", (uint8_t)n);
    Serial.print(F("# Saved: this is now the vehicle for station ")); Serial.println(n);
    Serial.println(F("# Restarting..."));
    Serial.println(F("{\"type\":\"saved\",\"what\":\"station\"}"));
    delay(300);
    ESP.restart();
  } else if (low == "info")     { printHello();
  } else if (low == "selftest") { runSelfTest();
  } else if (low == "roster")   { if (meshUp) PorpoiseNet.printRoster(); else Serial.println(F("# Not on the network: no station number yet."));
  } else if (low.length())      { printHelp(); }
}

void readSerial() {
  static String line;
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\r') continue;
    if (ch == '\n') { handleCommand(line); line = ""; }
    else if (line.length() < 200) line += ch;
  }
}

// ============================== Setup ================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("# ======================================"));
  Serial.println(F("#  VehicleSensors_v5  (Find the Target)"));
  Serial.println(F("# ======================================"));

  prefs.begin("ftt", false);           // the board's small saved-settings area
  station = prefs.getUChar("station", 0);
  if (station > MAX_STATIONS) station = 0;

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Wire.begin(I2C_SDA, I2C_SCL);
  bmpOk = bmp.begin(0x76) || bmp.begin(0x77);
  magOk = qmcBegin();
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();
  GPSserial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  runSelfTest();
  printHello();

  if (station == 0) {
    Serial.println(F("#"));
    Serial.println(F("# >>> This vehicle has no station number yet. <<<"));
    Serial.println(F("# >>> Type   station 3   (your number) and press Enter. <<<"));
    Serial.println(F("#"));
    return;                            // stay off the network until it has one
  }

  showStationNumber();
  if (!PorpoiseNet.begin(10 + station, PN_ROLE_VEHICLE, FTT_CHANNEL, FTT_NET_ID)) {
    Serial.println(F("# Radio failed to start. Unplug and plug back in."));
    return;
  }
  PorpoiseNet.onMessage(onNetworkMessage);
  meshUp = true;
  Serial.print(F("# Ready: vehicle for station ")); Serial.print(station);
  Serial.println(F(", sending a reading every second."));
}

// =============================== Loop ================================
void loop() {
  feedGps();
  readSerial();
  if (meshUp) {
    PorpoiseNet.loop();
    if (millis() - lastSend >= SEND_INTERVAL_MS) {
      lastSend = millis();
      sendReading();
    }
  }
  static unsigned long lastDraw = 0;
  if (millis() - lastDraw >= 50) { lastDraw = millis(); drawLeds(); }
}
