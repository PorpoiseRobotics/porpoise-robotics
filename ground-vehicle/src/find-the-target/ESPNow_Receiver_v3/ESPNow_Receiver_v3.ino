/* =====================================================================
 * ESPNow_Receiver_v3  --  Base
-station bridge
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   Sits on the base-station computer's USB port, receives SensorPackets
 *   from ESPNow_Sender_v3 over ESP-NOW, and forwards each packet to the
 *   computer as ONE LINE of JSON on the serial port. The base-station
 *   program (base_station.py) reads those lines and updates the web page.
 *
 * SERIAL OUTPUT FORMAT (115200 baud)
 *   - Lines starting with "# " are human-readable status/diagnostics.
 *   - Lines starting with "{"  are machine-readable JSON telemetry.
 *   Both are visible in the Arduino serial monitor, so the same output
 *   works for debugging AND for the web page.
 *
 * BOARD / IDE SETTINGS
 *   Board:  "ESP32 Dev Module"
 *   Core:   esp32 by Espressif Systems, v3.0.7
 *   Serial monitor: 115200 baud
 *   No external libraries required.
 *
 * FIRST-TIME SETUP
 *   Flash this board, open the serial monitor, and copy the MAC address
 *   it prints into PEER_MAC in ESPNow_Sender_v3.ino.
 *
 * HOW TO TELL IT IS WORKING
 *   - Boot banner prints this board's MAC and the listening channel.
 *   - One "# pkt ..." summary + one JSON line per received packet.
 *   - "# WARNING: no packets for Ns" if the link goes quiet.
 * ===================================================================== */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "esp_mac.h"

#define WIFI_CHANNEL     1      // MUST match ESPNow_Sender_v3
#define STALE_WARN_MS    5000   // warn if nothing heard for this long

// ================= Data packet (MUST match sender) ===================
// Keep this struct byte-for-byte identical to ESPNow_Sender_v3.ino.
#define PACKET_VERSION 3

#define FLAG_SONAR_OK 0x01
#define FLAG_BMP_OK   0x02
#define FLAG_GPS_FIX  0x04

typedef struct __attribute__((packed)) {
  uint8_t  version;
  uint8_t  flags;
  uint16_t reserved;
  uint32_t seq;
  uint32_t uptimeMs;
  float    distanceInches;
  float    tempF;
  float    pressureAtm;
  float    bmpAltitudeFeet;
  uint32_t satellites;
  double   latitude;
  double   longitude;
  float    speedMph;
  float    gpsAltitudeFeet;
  float    courseDeg;
} SensorPacket;

// ============================= Globals ===============================
// The receive callback runs in the WiFi task, so it only copies data and
// sets a flag; all printing happens safely in loop().
volatile bool      packetReady = false;
SensorPacket       rxPacket;              // written by callback, read by loop
volatile int8_t    rxRssi      = 0;       // signal strength of last packet
uint32_t           lastSeq     = 0;       // for detecting dropped packets
uint32_t           totalRx     = 0;
uint32_t           totalDropped= 0;
unsigned long      lastRxMs    = 0;
bool               staleWarned = false;

// ======================= Receive callback ============================
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // Reject anything that is not exactly our packet layout.
  if (len != sizeof(SensorPacket)) return;
  if (((const SensorPacket *)data)->version != PACKET_VERSION) return;

  memcpy((void *)&rxPacket, data, sizeof(SensorPacket));
  if (info && info->rx_ctrl) rxRssi = info->rx_ctrl->rssi;  // dBm
  packetReady = true;
}

// ============================ JSON output ============================
// One compact JSON object per line. Keys match what base_station.py and
// dashboard.html expect - change them together if you rename anything.
void printJson(const SensorPacket &p, int8_t rssi) {
  Serial.print(F("{\"seq\":"));          Serial.print(p.seq);
  Serial.print(F(",\"uptime_ms\":"));    Serial.print(p.uptimeMs);
  Serial.print(F(",\"sonar_ok\":"));     Serial.print((p.flags & FLAG_SONAR_OK) ? 1 : 0);
  Serial.print(F(",\"bmp_ok\":"));       Serial.print((p.flags & FLAG_BMP_OK)   ? 1 : 0);
  Serial.print(F(",\"gps_fix\":"));      Serial.print((p.flags & FLAG_GPS_FIX)  ? 1 : 0);
  Serial.print(F(",\"distance_in\":"));  Serial.print(p.distanceInches, 1);
  Serial.print(F(",\"temp_f\":"));       Serial.print(p.tempF, 1);
  Serial.print(F(",\"pressure_atm\":")); Serial.print(p.pressureAtm, 4);
  Serial.print(F(",\"bmp_alt_ft\":"));   Serial.print(p.bmpAltitudeFeet, 1);
  Serial.print(F(",\"sats\":"));         Serial.print(p.satellites);
  Serial.print(F(",\"lat\":"));          Serial.print(p.latitude, 6);
  Serial.print(F(",\"lon\":"));          Serial.print(p.longitude, 6);
  Serial.print(F(",\"speed_mph\":"));    Serial.print(p.speedMph, 1);
  Serial.print(F(",\"gps_alt_ft\":"));   Serial.print(p.gpsAltitudeFeet, 1);
  Serial.print(F(",\"course_deg\":"));   Serial.print(p.courseDeg, 1);
  Serial.print(F(",\"rssi\":"));         Serial.print(rssi);
  Serial.println(F("}"));
}

// ============================== Setup ================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("# ==================================="));
  Serial.println(F("#  ESPNow_Receiver_v3  (base station)"));
  Serial.println(F("# ==================================="));

  // Station mode, not associated with any AP, pinned to our channel.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  // Read the MAC straight from efuse - valid even before WiFi fully starts.
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(F("# Receiver MAC: "));
  Serial.println(macStr);
  Serial.println(F("# Paste that MAC into PEER_MAC in ESPNow_Sender_v3.ino"));
  Serial.print(F("# Listening on WiFi channel "));
  Serial.println(WIFI_CHANNEL);

  if (esp_now_init() != ESP_OK) {
    Serial.println(F("# FATAL: ESP-NOW init failed - resetting in 5 s."));
    delay(5000);
    ESP.restart();
  }
  esp_now_register_recv_cb(onRecv);
  Serial.println(F("# Ready - waiting for packets..."));
}

// =============================== Loop ================================
void loop() {
  // ---- Forward any packet that arrived since the last pass ----
  if (packetReady) {
    packetReady = false;

    // Take a local copy so the callback can't overwrite mid-print.
    SensorPacket p;
    noInterrupts();          // brief critical section around the copy
    memcpy(&p, (const void *)&rxPacket, sizeof(p));
    int8_t rssi = rxRssi;
    interrupts();

    totalRx++;
    // Sequence-gap check: tells the team if packets are being lost in
    // the air even though the link is "up".
    if (lastSeq != 0 && p.seq > lastSeq + 1) {
      uint32_t gap = p.seq - lastSeq - 1;
      totalDropped += gap;
      Serial.print(F("# NOTE: missed "));
      Serial.print(gap);
      Serial.println(F(" packet(s)"));
    }
    lastSeq  = p.seq;
    lastRxMs = millis();
    staleWarned = false;

    // Human-readable one-liner...
    Serial.print(F("# pkt "));    Serial.print(p.seq);
    Serial.print(F("  rssi "));   Serial.print(rssi);
    Serial.print(F(" dBm  rx ")); Serial.print(totalRx);
    Serial.print(F("  lost "));   Serial.println(totalDropped);
    // ...followed by the JSON line the base station parses.
    printJson(p, rssi);
  }

  // ---- Link watchdog: shout if the sender goes quiet ----
  if (lastRxMs != 0 && !staleWarned && millis() - lastRxMs > STALE_WARN_MS) {
    staleWarned = true;
    Serial.print(F("# WARNING: no packets for "));
    Serial.print((millis() - lastRxMs) / 1000);
    Serial.println(F("s - sender powered? in range? same channel?"));
  }

  delay(5);   // yield; everything interesting happens in the callback
}
