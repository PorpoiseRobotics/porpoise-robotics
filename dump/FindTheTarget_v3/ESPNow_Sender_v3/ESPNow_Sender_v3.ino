/* =====================================================================
 * ESPNow_Sender_v3  --  Vehicle telemetry node
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   1. Reads three sensors:
 *        - HC-SR04 ultrasonic rangefinder   (distance to obstacle)
 *        - BMP280                           (temperature / pressure / altitude)
 *        - GT-U7 GPS                        (position / speed / heading)
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
 *
 * WIRING
 *   HC-SR04:  TRIG -> GPIO32,  ECHO -> GPIO25 *through a voltage divider*
 *             (ECHO is a 5 V signal; e.g. 1k from ECHO to GPIO25 and
 *              2k from GPIO25 to GND brings it down to ~3.3 V).
 *   BMP280:   SDA -> GPIO21, SCL -> GPIO22 (I2C, address 0x76 or 0x77)
 *   GT-U7:    GPS TX -> GPIO16, GPS RX -> GPIO18 (UART2, 9600 baud)
 *   NeoPixel: DATA -> GPIO15, 16 LEDs
 *
 * BEFORE UPLOADING
 *   Flash ESPNow_Receiver_v3 first, open its serial monitor, copy the
 *   MAC address it prints, and paste it into PEER_MAC below.
 *
 * HOW TO TELL IT IS WORKING (serial monitor @115200)
 *   - Boot banner + a SELF-TEST report for every sensor.
 *   - One line per transmission showing the values that were sent and
 *     whether the receiver acknowledged the packet (DELIVERED / MISSED).
 *   - A link summary every 10 packets (delivery success rate).
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
#define I2C_SDA     21   // BMP280 data
#define I2C_SCL     22   // BMP280 clock
#define GPS_RX_PIN  16   // ESP32 RX  <- GPS TX
#define GPS_TX_PIN  18   // ESP32 TX  -> GPS RX (unused by GT-U7 but wired)
#define LED_PIN     15   // NeoPixel data in

// ============================= Config ================================
#define NUM_LEDS          16    // full 16-LED strip
#define LED_BRIGHTNESS    8     // 0-255; keep low to save current
#define SEND_INTERVAL_MS  1000  // one packet per second
#define WIFI_CHANNEL      1     // MUST match ESPNow_Receiver_v3
#define STATS_EVERY_N     10    // print a link summary every N packets

// Distance thresholds for the LED bar (inches)
#define DIST_GREEN_IN     40.0f
#define DIST_YELLOW_IN    20.0f

// Receiver MAC address - paste from the receiver's serial output.
uint8_t PEER_MAC[6] = { 0xFC, 0xB4, 0x67, 0xF1, 0xD4, 0xBC };

// ================= Data packet (MUST match receiver) =================
// Any change here must be mirrored byte-for-byte in ESPNow_Receiver_v3,
// otherwise the receiver will reject the packet as malformed.
#define PACKET_VERSION 3

// Bit positions for the .flags field
#define FLAG_SONAR_OK 0x01   // ultrasonic returned an echo this cycle
#define FLAG_BMP_OK   0x02   // BMP280 detected at boot
#define FLAG_GPS_FIX  0x04   // GPS has a valid position fix

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
  float    courseDeg;        // GPS heading, 0-360 (0 until moving)
} SensorPacket;

SensorPacket packet;

// ============================= Globals ===============================
Adafruit_BMP280   bmp;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
TinyGPSPlus       gps;
HardwareSerial    GPSserial(2);          // UART2 for the GT-U7

bool          bmpOk        = false;
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

  // HC-SR04: fire one ping and see if anything echoes.
  float d = readDistanceInches();
  Serial.print(F("[HC-SR04]  "));
  if (d > 0) { Serial.print(F("PASS  (echo at ")); Serial.print(d, 1); Serial.println(F(" in)")); }
  else         Serial.println(F("WARN  (no echo - clear path >5m, or check TRIG=32 ECHO=25 divider)"));

  // GT-U7: is ANY serial data arriving? (A fix can take minutes outdoors,
  // so we only test that the wire is alive here.)
  Serial.print(F("[GT-U7  ]  "));
  unsigned long t0 = millis();
  bool gpsBytes = false;
  while (millis() - t0 < 1500) {
    if (GPSserial.available()) { gpsBytes = true; break; }
  }
  Serial.println(gpsBytes ? F("PASS  (NMEA data flowing; fix may take minutes)")
                          : F("FAIL  (no data - check GPS TX->GPIO16 and power)"));

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
  Serial.println(F(" ESPNow_Sender_v3  (vehicle node)"));
  Serial.println(F("================================="));

  // --- GPIO / buses ---
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Wire.begin(I2C_SDA, I2C_SCL);
  bmpOk = bmp.begin(0x76) || bmp.begin(0x77);
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();
  GPSserial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

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
  Serial.print(F("F  sats="));     Serial.print(packet.satellites);
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
