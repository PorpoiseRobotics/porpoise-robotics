/* =====================================================================
 * ESPNow_Sender_v4  --  Vehicle telemetry node
 * ---------------------------------------------------------------------
 * WHAT CHANGED FROM v3
 *   The GT-U7 GPS is replaced by an FK-A1 module (u-blox M10 receiver
 *   with a QMC5883L compass on the same little board). Two consequences:
 *     - The GPS serial port now runs at 38400 baud (the FK-A1 default;
 *       the GT-U7 used 9600).
 *     - We gain a real compass heading over I2C. The GPS "course" only
 *       works while the vehicle is MOVING; the compass knows which way
 *       the vehicle is FACING even when parked, so the dashboard arrow
 *       no longer freezes at stops.
 *
 * WHAT THIS BOARD DOES
 *   1. Reads four sensors:
 *        - HC-SR04 ultrasonic rangefinder   (distance to obstacle)
 *        - BMP280                           (temperature / pressure / altitude)
 *        - FK-A1 GPS (u-blox M10)           (position / speed / course)
 *        - QMC5883L compass (on the FK-A1)  (heading, works standing still)
 *   2. Drives 16 NeoPixel LEDs as a "proximity bar":
 *        GREEN  = clear (> 40 in)   YELLOW = caution (20-40 in)
 *        RED    = close (< 20 in)   DIM BLUE = no echo (sensor problem?)
 *   3. Packs everything into one SensorPacket and transmits it to the
 *      receiver board over ESP-NOW once per second.
 *
 * BOARD / IDE SETTINGS
 *   Board:  "ESP32 Dev Module"
 *   Core:   esp32 by Espressif Systems, v3.0.7
 *   Serial monitor: 115200 baud
 *
 * LIBRARIES (Library Manager)
 *   - Adafruit BMP280 Library (also installs Adafruit Unified Sensor)
 *   - Adafruit NeoPixel
 *   - TinyGPSPlus
 *   (The compass needs no library - we talk to it directly over I2C.)
 *
 * WIRING
 *   HC-SR04:  TRIG -> GPIO32,  ECHO -> GPIO25 *through a voltage divider*
 *             (ECHO is a 5 V signal; e.g. 1k from ECHO to GPIO25 and
 *              2k from GPIO25 to GND brings it down to ~3.3 V).
 *   BMP280:   SDA -> GPIO21, SCL -> GPIO22 (I2C, address 0x76 or 0x77)
 *   FK-A1:    6-pin connector, left to right: GND  5V  Rx  Tx  SCL  SDA
 *             GND -> GND,  5V -> 5V
 *             Tx  -> GPIO16 (module transmits, ESP32 receives)
 *             Rx  -> GPIO18 (ESP32 transmits, module receives)
 *             SCL -> GPIO22, SDA -> GPIO21  (compass shares the BMP280's
 *             I2C bus - three devices on two wires is what I2C is for)
 *             The UART is 3.3 V TTL, so NO voltage divider is needed.
 *   NeoPixel: DATA -> GPIO15, 16 LEDs
 *
 * BEFORE UPLOADING
 *   Flash ESPNow_Receiver_v4 first, open its serial monitor, copy the
 *   MAC address it prints, and paste it into PEER_MAC below.
 *
 * HOW TO TELL IT IS WORKING (serial monitor @115200)
 *   - Boot banner + a SELF-TEST report for every sensor.
 *   - One line per transmission showing the values that were sent and
 *     whether the receiver acknowledged the packet (DELIVERED / MISSED).
 *   - A link summary every 10 packets (delivery success rate).
 *   - Bonus hardware check: the FK-A1's blue PPS LED is solid on power
 *     and starts FLASHING once the GPS has a 3D fix.
 * ===================================================================== */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_NeoPixel.h>
#include <TinyGPSPlus.h>

// ============================== Pins =================================
#define TRIG_PIN    32   // HC-SR04 trigger (3.3 V output is fine)
#define ECHO_PIN    25   // HC-SR04 echo, MUST come through a divider
#define I2C_SDA     21   // BMP280 + QMC5883L data
#define I2C_SCL     22   // BMP280 + QMC5883L clock
#define GPS_RX_PIN  16   // ESP32 RX  <- FK-A1 Tx
#define GPS_TX_PIN  18   // ESP32 TX  -> FK-A1 Rx
#define LED_PIN     15   // NeoPixel data in

// ============================= Config ================================
#define NUM_LEDS          16    // full 16-LED strip
#define LED_BRIGHTNESS    8     // 0-255; keep low to save current
#define SEND_INTERVAL_MS  1000  // one packet per second
#define WIFI_CHANNEL      1     // MUST match ESPNow_Receiver_v4
#define STATS_EVERY_N     10    // print a link summary every N packets

#define GPS_BAUD          38400 // FK-A1 factory default (GT-U7 was 9600)

// Turn the vehicle so it faces a known direction (use a phone compass),
// compare with the Heading value on the dashboard, and put the
// difference here. Corrects for how the module is mounted on the chassis.
#define HEADING_OFFSET_DEG  0.0f

// Distance thresholds for the LED bar (inches)
#define DIST_GREEN_IN     40.0f
#define DIST_YELLOW_IN    20.0f

// Receiver MAC address - paste from the receiver's serial output.
uint8_t PEER_MAC[6] = { 0xFC, 0xB4, 0x67, 0xF1, 0xD4, 0xBC };

// ================= Data packet (MUST match receiver) =================
// Any change here must be mirrored byte-for-byte in ESPNow_Receiver_v4,
// otherwise the receiver will reject the packet as malformed.
#define PACKET_VERSION 4

// Bit positions for the .flags field
#define FLAG_SONAR_OK 0x01   // ultrasonic returned an echo this cycle
#define FLAG_BMP_OK   0x02   // BMP280 detected at boot
#define FLAG_GPS_FIX  0x04   // GPS has a valid position fix
#define FLAG_MAG_OK   0x08   // QMC5883L compass gave a reading this cycle

typedef struct __attribute__((packed)) {
  uint8_t  version;          // packet layout version (= PACKET_VERSION)
  uint8_t  flags;            // sensor health bits, see FLAG_* above
  uint16_t reserved;         // padding / future use, always 0
  uint32_t seq;              // packet counter, lets receiver spot drops
  uint32_t uptimeMs;         // sender millis(), proves sender is alive
  float    distanceInches;   // HC-SR04 range (0 when no echo)
  float    tempF;            // BMP280 temperature
  float    pressureAtm;      // BMP280 pressure
  float    bmpAltitudeFeet;  // BMP280 barometric altitude
  uint32_t satellites;       // GPS satellites in use
  double   latitude;         // GPS position (0 until first fix)
  double   longitude;
  float    speedMph;         // GPS ground speed
  float    gpsAltitudeFeet;  // GPS altitude
  float    courseDeg;        // GPS direction of TRAVEL, 0-360 (needs motion)
  float    headingDeg;       // compass direction the vehicle FACES, 0-360
} SensorPacket;

SensorPacket packet;

// ============================= Globals ===============================
Adafruit_BMP280   bmp;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
TinyGPSPlus       gps;
HardwareSerial    GPSserial(2);          // UART2 for the FK-A1

bool          bmpOk        = false;
bool          magOk        = false;
unsigned long lastSend     = 0;
int           lastColorCat = -99;        // forces first LED write

// Delivery statistics (updated from the ESP-NOW send callback)
volatile uint32_t txDelivered = 0;       // receiver ACKed
volatile uint32_t txMissed    = 0;       // no ACK (receiver off / out of range)
volatile int8_t   lastTxOk    = -1;      // -1 unknown, 0 missed, 1 delivered

// ============================= Sensors ===============================

// Fire one ultrasonic ping. Returns distance in inches, or -1 if no
// echo came back within the ~5 m timeout (nothing in range, or wiring).
float readDistanceInches() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000UL);  // 30 ms ~ 5 m
  if (dur == 0) return -1.0f;
  return dur / 148.0f;   // round-trip microseconds -> inches
}

// Copy BMP280 readings into the packet (zeros if the chip is absent).
void readBmp() {
  if (!bmpOk) {
    packet.tempF = packet.pressureAtm = packet.bmpAltitudeFeet = 0;
    return;
  }
  packet.tempF           = bmp.readTemperature() * 9.0f / 5.0f + 32.0f;
  packet.pressureAtm     = bmp.readPressure() / 101325.0f;         // Pa -> atm
  packet.bmpAltitudeFeet = bmp.readAltitude(1013.25f) * 3.28084f;  // m -> ft
}

// The GPS streams NMEA text continuously; feed every byte into the
// parser as often as possible so sentences are never truncated.
void feedGps() {
  while (GPSserial.available()) gps.encode(GPSserial.read());
}

// Copy the latest parsed GPS values into the packet.
void readGps() {
  bool fix = gps.location.isValid();
  packet.satellites      = gps.satellites.isValid() ? gps.satellites.value() : 0;
  packet.latitude        = fix ? gps.location.lat() : 0.0;
  packet.longitude       = fix ? gps.location.lng() : 0.0;
  packet.speedMph        = gps.speed.isValid()  ? gps.speed.mph()   : 0.0f;
  packet.gpsAltitudeFeet = gps.altitude.isValid() ? gps.altitude.feet() : 0.0f;
  packet.courseDeg       = gps.course.isValid() ? gps.course.deg()  : 0.0f;
  if (fix) packet.flags |= FLAG_GPS_FIX;
}

// ================= QMC5883L compass (no library) =====================
// The compass measures the Earth's magnetic field along its X, Y and Z
// axes. With the module lying flat, the angle of the field in the X-Y
// plane IS the compass heading. We talk to the chip with plain I2C
// register reads/writes - a nice peek at what libraries do under the hood.
#define QMC_ADDR        0x0D   // fixed I2C address
#define QMC_REG_DATA    0x00   // X lo, X hi, Y lo, Y hi, Z lo, Z hi
#define QMC_REG_STATUS  0x06   // bit0 = data ready
#define QMC_REG_CTRL1   0x09   // mode / data rate / range / oversampling
#define QMC_REG_CTRL2   0x0A   // 0x80 = soft reset
#define QMC_REG_PERIOD  0x0B   // set/reset period, datasheet says write 1

bool qmcWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(QMC_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

// Returns true if the chip answered on the bus and was configured.
bool qmcBegin() {
  if (!qmcWrite(QMC_REG_CTRL2, 0x80)) return false;  // soft reset
  delay(10);
  qmcWrite(QMC_REG_PERIOD, 0x01);
  // CTRL1 0x05 = oversample x512, range +/-2 gauss, 50 Hz, continuous mode
  return qmcWrite(QMC_REG_CTRL1, 0x05);
}

// Latest heading in degrees 0-360 (0 = magnetic north), or -1 on error.
// Assumes the module sits flat; tilting the vehicle tilts the reading.
float readHeadingDeg() {
  if (!magOk) return -1.0f;

  Wire.beginTransmission(QMC_ADDR);
  Wire.write(QMC_REG_DATA);
  if (Wire.endTransmission(false) != 0) return -1.0f;
  if (Wire.requestFrom((uint8_t)QMC_ADDR, (uint8_t)6) != 6) return -1.0f;

  int16_t x = Wire.read() | (Wire.read() << 8);   // low byte first
  int16_t y = Wire.read() | (Wire.read() << 8);
  (void)(Wire.read() | (Wire.read() << 8));       // Z: read but unused

  // atan2 gives the field angle in radians; convert to 0-360 compass
  // degrees and apply the mounting correction. (This is MAGNETIC north;
  // true north differs by your local declination, ~11 deg E in San Diego -
  // fine to ignore for finding a target on a soccer field.)
  float deg = atan2f((float)y, (float)x) * 180.0f / PI + HEADING_OFFSET_DEG;
  while (deg < 0)      deg += 360.0f;
  while (deg >= 360.0f) deg -= 360.0f;
  return deg;
}

// =============================== LEDs ================================
// Category: 0 green (clear), 1 yellow (caution), 2 red (close),
//           -1 no echo -> dim blue so the team can SEE a sensor problem.
int colorCategory(float inches) {
  if (inches < 0)               return -1;
  if (inches > DIST_GREEN_IN)   return 0;
  if (inches >= DIST_YELLOW_IN) return 1;
  return 2;
}

void updateLeds(float inches) {
  int cat = colorCategory(inches);
  if (cat == lastColorCat) return;   // no change -> skip the strip write
  lastColorCat = cat;

  uint32_t color;
  switch (cat) {
    case 0:  color = strip.Color(0, 255, 0);    break;  // green: clear
    case 1:  color = strip.Color(255, 255, 0);  break;  // yellow: caution
    case 2:  color = strip.Color(255, 0, 0);    break;  // red: close
    default: color = strip.Color(0, 0, 40);     break;  // dim blue: no echo
  }
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, color);
  strip.show();
}

// ============================= ESP-NOW ===============================
// Called by the WiFi stack after every transmission. "SUCCESS" means the
// receiver's radio acknowledged the frame - a real end-to-end check.
// (Core 3.0.x callback signature; 3.1+ changes the first parameter.)
void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) { txDelivered++; lastTxOk = 1; }
  else                                { txMissed++;    lastTxOk = 0; }
}

// ============================ Self-test ==============================
// Runs once at boot and prints a PASS/FAIL line per subsystem so the
// team can verify wiring before the vehicle drives away.
void runSelfTest() {
  Serial.println(F("---- SELF-TEST ----"));

  // BMP280: did it answer on I2C?
  Serial.print(F("[BMP280 ]  "));
  Serial.println(bmpOk ? F("PASS  (found on I2C)")
                       : F("FAIL  (check SDA=21 SCL=22, addr 0x76/0x77)"));

  // QMC5883L: did the compass answer on the same I2C bus?
  Serial.print(F("[COMPASS]  "));
  if (magOk) {
    float h = readHeadingDeg();
    if (h >= 0) { Serial.print(F("PASS  (heading ")); Serial.print(h, 0); Serial.println(F(" deg)")); }
    else          Serial.println(F("WARN  (found, but no reading yet)"));
  } else {
    Serial.println(F("FAIL  (no answer at 0x0D - check FK-A1 SCL->22 SDA->21)"));
  }

  // HC-SR04: fire one ping and see if anything echoes.
  float d = readDistanceInches();
  Serial.print(F("[HC-SR04]  "));
  if (d > 0) { Serial.print(F("PASS  (echo at ")); Serial.print(d, 1); Serial.println(F(" in)")); }
  else         Serial.println(F("WARN  (no echo - clear path >5m, or check TRIG=32 ECHO=25 divider)"));

  // FK-A1 GPS: is ANY serial data arriving? (A fix can take ~30 s or more
  // outdoors, so we only test that the wire is alive here. Tip: the
  // module's blue PPS LED flashes once it has a 3D fix.)
  Serial.print(F("[FK-A1  ]  "));
  unsigned long t0 = millis();
  bool gpsBytes = false;
  while (millis() - t0 < 1500) {
    if (GPSserial.available()) { gpsBytes = true; break; }
  }
  Serial.println(gpsBytes ? F("PASS  (NMEA data flowing; fix may take ~30 s outdoors)")
                          : F("FAIL  (no data - check FK-A1 Tx->GPIO16, power, 38400 baud)"));

  // NeoPixels: brief white flash proves data line + power.
  Serial.println(F("[LEDs   ]  flashing white - confirm visually"));
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(40, 40, 40));
  strip.show();
  delay(400);
  strip.clear();
  strip.show();

  Serial.println(F("-------------------"));
}

// ============================== Setup ================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("================================="));
  Serial.println(F(" ESPNow_Sender_v4  (vehicle node)"));
  Serial.println(F("================================="));

  // --- GPIO / buses ---
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

  // --- Radio: station mode, no AP, fixed channel shared with receiver ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // lower power draw and RF noise

  Serial.print(F("This sender's MAC:  "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("Sending to peer:    "));
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          PEER_MAC[0], PEER_MAC[1], PEER_MAC[2],
          PEER_MAC[3], PEER_MAC[4], PEER_MAC[5]);
  Serial.println(macStr);

  if (esp_now_init() != ESP_OK) {
    Serial.println(F("FATAL: ESP-NOW init failed - resetting in 5 s."));
    delay(5000);
    ESP.restart();
  }
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, PEER_MAC, 6);
  peer.channel = WIFI_CHANNEL;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println(F("FATAL: could not add peer - check PEER_MAC."));
  }

  packet.version = PACKET_VERSION;
  Serial.println(F("Setup complete - transmitting every 1 s."));
}

// =============================== Loop ================================
void loop() {
  feedGps();   // keep the NMEA parser fed between transmissions

  if (millis() - lastSend < SEND_INTERVAL_MS) return;
  lastSend = millis();

  // ---- Gather one snapshot of every sensor ----
  packet.flags = 0;
  float inches = readDistanceInches();
  if (inches >= 0) packet.flags |= FLAG_SONAR_OK;
  packet.distanceInches = (inches < 0) ? 0.0f : inches;
  if (bmpOk) packet.flags |= FLAG_BMP_OK;
  readBmp();
  readGps();
  float heading = readHeadingDeg();
  if (heading >= 0) packet.flags |= FLAG_MAG_OK;
  packet.headingDeg = (heading < 0) ? 0.0f : heading;
  updateLeds(inches);

  packet.seq++;
  packet.uptimeMs = millis();

  // ---- Transmit ----
  esp_err_t err = esp_now_send(PEER_MAC, (uint8_t *)&packet, sizeof(packet));

  // ---- One human-readable status line per packet ----
  // (lastTxOk reflects the PREVIOUS packet's ACK; the callback for this
  //  one usually arrives a few ms after esp_now_send returns.)
  Serial.print(F("#"));            Serial.print(packet.seq);
  Serial.print(F("  dist="));      Serial.print(packet.distanceInches, 1);
  Serial.print(F("in  temp="));    Serial.print(packet.tempF, 1);
  Serial.print(F("F  hdg="));      Serial.print(packet.headingDeg, 0);
  Serial.print(F("  sats="));      Serial.print(packet.satellites);
  Serial.print(F("  fix="));       Serial.print((packet.flags & FLAG_GPS_FIX) ? F("YES") : F("no"));
  Serial.print(F("  lat="));       Serial.print(packet.latitude, 6);
  Serial.print(F("  lon="));       Serial.print(packet.longitude, 6);
  Serial.print(F("  tx="));
  if      (err != ESP_OK)  Serial.println(F("QUEUE-FAIL"));
  else if (lastTxOk == 1)  Serial.println(F("DELIVERED"));
  else if (lastTxOk == 0)  Serial.println(F("MISSED (receiver on? in range? same channel?)"));
  else                     Serial.println(F("..."));

  // ---- Link summary every N packets ----
  if (packet.seq % STATS_EVERY_N == 0) {
    uint32_t total = txDelivered + txMissed;
    Serial.print(F("LINK: "));
    Serial.print(txDelivered); Serial.print(F("/")); Serial.print(total);
    Serial.print(F(" delivered ("));
    Serial.print(total ? (100.0f * txDelivered / total) : 0.0f, 0);
    Serial.println(F("%)"));
  }
}
