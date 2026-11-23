/* =====================================================================
 * VehicleCamera_v5  --  Find the Target v5, the vehicle's camera
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   Streams live video over WiFi to your station's dashboard. It joins
 *   the field router (or any WiFi network you saved on it), and the
 *   dashboard finds it there by itself.
 *
 * NO PASSWORDS IN THIS FILE
 *   The WiFi name and password are typed into the CAMERA, not into this
 *   code, and the camera saves them in its own memory. So this file can
 *   be shared on GitHub safely, and every station uploads the same file.
 *   Your team's WiFi names and passwords are in the shared OneDrive folder
 *   "Porpoise Robotics Private" -> "WiFi networks.txt".
 *
 * ONE-TIME SETUP (each camera, once) - pick ONE way:
 *   (a) With a phone or laptop:
 *       1. Power the camera. With no WiFi saved, it makes its own WiFi
 *          network called  FTT-Camera-Setup  (or FTT-Camera-3-Setup once
 *          it has a station number).
 *       2. Join that network. A setup page opens by itself (if not, open
 *          a browser and go to  http://192.168.4.1 ).
 *       3. Choose the station number, pick your WiFi, type its password,
 *          press Save. The camera restarts and joins your WiFi.
 *   (b) With the dashboard: plug the camera into the laptop's USB and use
 *       its "Set up a board" button.
 *   (c) With Serial Monitor (115200 baud), type, pressing Enter after each:
 *          station 3
 *          wifiname Your Network Name
 *          wifipass your password
 *          wifisave
 *   It remembers up to 3 networks and joins whichever it can see. If it
 *   cannot join any of them for 30 seconds, it opens the setup network
 *   again for 3 minutes, then tries the WiFi again.
 *
 * BOARD / IDE SETTINGS
 *   Board:            "ESP32 Wrover Module"
 *   Board package:    esp32 by Espressif Systems, 3.3.12
 *   Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
 *   Serial Monitor:   115200 baud
 *   Keep app_httpd.cpp, camera_index.h and camera_pins.h in this folder.
 *   (app_httpd.cpp is the stock Espressif file plus one small addition,
 *   the /whoami page, marked "Find the Target v5".)
 *
 * THE CAMERA IS NOT ON THE PORPOISENET RADIO NETWORK. It only uses WiFi.
 * Set the field router's 2.4 GHz channel to 6 or 11; the radio network
 * uses channel 1, and keeping them apart helps both.
 * ===================================================================== */

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

#define FIRMWARE_VERSION "5.0"

#define CAMERA_MODEL_WROVER_KIT   // Freenove ESP32-WROVER kit
#include "camera_pins.h"

#define MAX_STATIONS          9
#define MAX_NETWORKS          3
#define WIFI_TRY_MS           30000   // give up on WiFi after this long...
#define SETUP_WINDOW_MS       180000  // ...open the setup network this long, then retry
#define STATUS_INTERVAL_MS    30000

// Implemented in app_httpd.cpp (stock Espressif file):
void startCameraServer();
void setupLedFlash(int pin);

Preferences prefs;
WiFiMulti   wifiMulti;
WebServer   setupServer(80);
DNSServer   dns;

uint8_t station = 0;
String  netName[MAX_NETWORKS];
String  netPass[MAX_NETWORKS];
int     netCount = 0;
bool    setupMode = false;
bool    cameraOk  = false;
unsigned long setupStartedMs = 0;
unsigned long lastStatusMs   = 0;

// Called by app_httpd.cpp's /whoami page.
int fttStationNumber() { return station; }

// ========================= Saved settings ============================
void loadSettings() {
  prefs.begin("ftt", false);
  station = prefs.getUChar("station", 0);
  if (station > MAX_STATIONS) station = 0;
  netCount = 0;
  for (int i = 0; i < MAX_NETWORKS; i++) {
    String n = prefs.getString(("n" + String(i)).c_str(), "");
    if (!n.length()) continue;
    netName[netCount] = n;
    netPass[netCount] = prefs.getString(("p" + String(i)).c_str(), "");
    netCount++;
  }
}

void saveNetworks() {
  for (int i = 0; i < MAX_NETWORKS; i++) {
    prefs.putString(("n" + String(i)).c_str(), i < netCount ? netName[i] : String(""));
    prefs.putString(("p" + String(i)).c_str(), i < netCount ? netPass[i] : String(""));
  }
}

// Newest network goes first. Saving a name that is already there just
// updates its password. The oldest falls off the end when there are 3.
void addNetwork(const String &name, const String &pass) {
  if (!name.length()) return;
  String names[MAX_NETWORKS], passes[MAX_NETWORKS];
  int n = 0;
  names[n] = name; passes[n] = pass; n++;
  for (int i = 0; i < netCount && n < MAX_NETWORKS; i++) {
    if (netName[i] == name) continue;
    names[n] = netName[i]; passes[n] = netPass[i]; n++;
  }
  for (int i = 0; i < n; i++) { netName[i] = names[i]; netPass[i] = passes[i]; }
  netCount = n;
  saveNetworks();
}

String hostName() {
  return station ? "ftt-camera-" + String(station) : String("ftt-camera");
}

// ========================= Serial messages ===========================
void printHello() {
  Serial.print(F("{\"type\":\"hello\",\"board\":\"camera\",\"fw\":\""));
  Serial.print(F(FIRMWARE_VERSION));
  Serial.print(F("\",\"station\":")); Serial.print(station);
  Serial.print(F(",\"wifi\":\""));
  if (setupMode) Serial.print(F("setup"));
  else if (WiFi.status() == WL_CONNECTED) Serial.print(F("connected"));
  else Serial.print(F("connecting"));
  Serial.print(F("\",\"ip\":\""));
  Serial.print(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String(""));
  Serial.print(F("\",\"ssid\":\""));
  // The WiFi it joined, so the dashboard can say "the laptop must be on
  // this one too". Quotes and backslashes are dropped to keep the JSON valid.
  if (WiFi.status() == WL_CONNECTED) {
    String ssid = WiFi.SSID();
    for (unsigned i = 0; i < ssid.length(); i++) {
      char c = ssid[i];
      if (c != '"' && c != '\\' && (uint8_t)c >= 0x20) Serial.print(c);
    }
  }
  Serial.print(F("\",\"networks\":")); Serial.print(netCount);
  Serial.println(F("}"));
}

// ============================ Camera =================================
bool startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println(F("# PSRAM not found - check Tools > Board is \"ESP32 Wrover Module\""));
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("# CAMERA FAILED (0x%x). Check the ribbon cable is pushed in\n", err);
    Serial.println(F("#   and the latch is closed, then press reset."));
    return false;
  }
  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  // QVGA (320x240) keeps several streams on one small router smooth.
  s->set_framesize(s, FRAMESIZE_QVGA);
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif
  Serial.println(F("# Camera: OK"));
  return true;
}

// ========================= Setup hotspot =============================
// A tiny web page served by the camera's own WiFi network. Big text and
// big buttons, because it is usually filled in on a phone.
const char SETUP_PAGE_TOP[] PROGMEM = R"HTML(<!doctype html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Camera setup</title><style>
body{font-family:system-ui,sans-serif;max-width:480px;margin:0 auto;padding:16px;font-size:20px;background:#f4f6fb;color:#111}
h1{font-size:28px}label{display:block;margin-top:18px;font-weight:700}
select,input{width:100%;font-size:22px;padding:12px;margin-top:6px;box-sizing:border-box;border-radius:10px;border:2px solid #889}
button{width:100%;font-size:24px;padding:16px;margin-top:24px;border:0;border-radius:12px;background:#1f6feb;color:#fff;font-weight:700}
.note{font-size:16px;color:#444;margin-top:18px}
</style></head><body><h1>&#128247; Camera setup</h1><form method="POST" action="/save">)HTML";

const char SETUP_PAGE_BOTTOM[] PROGMEM = R"HTML(
<label for="pass">WiFi password</label><input id="pass" name="pass" type="text" autocomplete="off">
<button type="submit">Save</button></form>
<p class="note">The password is in the OneDrive folder "Porpoise Robotics Private",
file "WiFi networks.txt". The camera saves it; you will not need to type it again.</p>
</body></html>)HTML";

void handleSetupPage() {
  String h = FPSTR(SETUP_PAGE_TOP);
  h += F("<label for=\"st\">Station number</label><select id=\"st\" name=\"station\">");
  for (int i = 1; i <= MAX_STATIONS; i++) {
    h += "<option value=\"" + String(i) + "\"" + (i == station ? " selected" : "") + ">Station " + String(i) + "</option>";
  }
  h += F("</select><label for=\"name\">WiFi network</label><select id=\"name\" name=\"name\">");
  // List the networks the camera can see right now, strongest first.
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    if (!s.length()) continue;
    s.replace("&", "&amp;"); s.replace("<", "&lt;"); s.replace("\"", "&quot;");
    h += "<option>" + s + "</option>";
  }
  WiFi.scanDelete();
  h += F("<option value=\"\">(type it below instead)</option></select>"
         "<label for=\"other\">...or type the network name</label>"
         "<input id=\"other\" name=\"other\" autocomplete=\"off\">");
  h += FPSTR(SETUP_PAGE_BOTTOM);
  setupServer.send(200, "text/html", h);
}

void handleSetupSave() {
  long st = setupServer.arg("station").toInt();
  String name = setupServer.arg("other");
  name.trim();
  if (!name.length()) name = setupServer.arg("name");
  String pass = setupServer.arg("pass");
  if (st >= 1 && st <= MAX_STATIONS) { station = (uint8_t)st; prefs.putUChar("station", station); }
  if (name.length()) addNetwork(name, pass);
  String h = F("<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
               "<body style='font-family:system-ui;font-size:24px;padding:20px'>"
               "<h1>&#9989; Saved!</h1><p>The camera is restarting and will join <b>");
  name.replace("<", "&lt;");
  h += name + F("</b>.</p><p>You can close this page and go back to your normal WiFi.</p>");
  setupServer.send(200, "text/html", h);
  Serial.println(F("# Setup saved from the setup page. Restarting..."));
  delay(1500);
  ESP.restart();
}

void startSetupMode() {
  setupMode = true;
  setupStartedMs = millis();
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);   // AP for the page, STA so the scan works
  String apName = station ? "FTT-Camera-" + String(station) + "-Setup" : String("FTT-Camera-Setup");
  WiFi.softAP(apName.c_str());   // open network: it only exists during setup
  delay(200);
  // Answer every web address with our own page, so phones pop it up.
  dns.start(53, "*", WiFi.softAPIP());
  setupServer.on("/", HTTP_GET, handleSetupPage);
  setupServer.on("/save", HTTP_POST, handleSetupSave);
  setupServer.onNotFound([]() {
    setupServer.sendHeader("Location", "http://192.168.4.1/", true);
    setupServer.send(302, "text/plain", "");
  });
  setupServer.begin();
  Serial.println(F("#"));
  Serial.println(F("# >>> SETUP MODE <<<"));
  Serial.print  (F("#   With a phone or laptop, join the WiFi network  ")); Serial.println(apName);
  Serial.println(F("#   then open  http://192.168.4.1  if the page does not open by itself."));
  Serial.println(F("#   (Or type: station 3 / wifiname ... / wifipass ... / wifisave)"));
  Serial.println(F("#"));
  printHello();
}

// ============================= WiFi ==================================
bool connectWifi() {
  if (netCount == 0) return false;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostName().c_str());
  WiFi.setSleep(false);   // sleep off = smoother video
  for (int i = 0; i < netCount; i++) wifiMulti.addAP(netName[i].c_str(), netPass[i].c_str());
  Serial.println(F("# Joining WiFi. Saved networks:"));
  for (int i = 0; i < netCount; i++) { Serial.print(F("#   - ")); Serial.println(netName[i]); }
  unsigned long t0 = millis();
  while (millis() - t0 < WIFI_TRY_MS) {
    if (wifiMulti.run(5000) == WL_CONNECTED) return true;
    readSerial();
  }
  Serial.println(F("# Could not join any saved network. Networks the camera can see:"));
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) Serial.printf("#   %-28s %4d dBm\n", WiFi.SSID(i).c_str(), (int)WiFi.RSSI(i));
  WiFi.scanDelete();
  Serial.println(F("#   (Only 2.4 GHz networks work. Check the spelling and the password.)"));
  return false;
}

void announceReady() {
  String ip = WiFi.localIP().toString();
  MDNS.begin(hostName().c_str());
  MDNS.addService("http", "tcp", 80);
  Serial.println(F("#"));
  Serial.print(F("# Camera ready for station ")); Serial.println(station);
  Serial.print(F("#   WiFi:          ")); Serial.println(WiFi.SSID());
  Serial.print(F("#   Video stream:  http://")); Serial.print(ip); Serial.println(F(":81/stream"));
  Serial.print(F("#   Settings page: http://")); Serial.print(ip); Serial.println(F("/"));
  Serial.println(F("#   The dashboard finds this camera by itself."));
  Serial.println(F("#"));
  printHello();
}

// ========================= Serial commands ===========================
String pendingName, pendingPass;

void handleCommand(String line) {
  line.trim();
  String low = line; low.toLowerCase();
  if (low.startsWith("station")) {
    long n = line.substring(7).toInt();
    if (n < 1 || n > MAX_STATIONS) { Serial.println(F("{\"type\":\"error\",\"text\":\"station must be 1-9\"}")); return; }
    station = (uint8_t)n;
    prefs.putUChar("station", station);
    Serial.print(F("# Saved station ")); Serial.println(station);
    Serial.println(F("{\"type\":\"saved\",\"what\":\"station\"}"));
  } else if (low.startsWith("wifiname")) {
    pendingName = line.substring(8); pendingName.trim();
    Serial.print(F("# WiFi name: ")); Serial.println(pendingName);
    Serial.println(F("# Now type   wifipass <the password>   then   wifisave"));
  } else if (low.startsWith("wifipass")) {
    pendingPass = line.substring(8); pendingPass.trim();
    Serial.println(F("# Password noted (not shown). Type   wifisave   to save and restart."));
  } else if (low == "wifisave") {
    if (!pendingName.length()) { Serial.println(F("{\"type\":\"error\",\"text\":\"type wifiname first\"}")); return; }
    addNetwork(pendingName, pendingPass);
    Serial.println(F("# Saved. Restarting to join it..."));
    Serial.println(F("{\"type\":\"saved\",\"what\":\"wifi\"}"));
    delay(300);
    ESP.restart();
  } else if (low == "wificlear") {
    netCount = 0; saveNetworks();
    Serial.println(F("# All saved WiFi networks forgotten."));
    Serial.println(F("{\"type\":\"saved\",\"what\":\"wifi\"}"));
  } else if (low == "wifi") {
    Serial.println(F("# Saved WiFi networks (passwords are never shown):"));
    for (int i = 0; i < netCount; i++) { Serial.print(F("#   ")); Serial.println(netName[i]); }
    if (!netCount) Serial.println(F("#   (none)"));
  } else if (low == "setup") {
    if (!setupMode) startSetupMode();
  } else if (low == "restart") {
    ESP.restart();
  } else if (low == "info") {
    printHello();
  } else if (low.length()) {
    Serial.println(F("# Commands: station 3 | wifiname <name> | wifipass <password> | wifisave"));
    Serial.println(F("#           wifi (list) | wificlear | setup | info | restart"));
  }
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
  Serial.println(F("# ====================================="));
  Serial.println(F("#  VehicleCamera_v5  (Find the Target)"));
  Serial.println(F("# ====================================="));
  loadSettings();
  cameraOk = startCamera();

  if (station == 0 || netCount == 0) {
    Serial.println(F("# This camera needs a station number and a WiFi network."));
    startSetupMode();
    return;
  }
  if (connectWifi()) {
    if (cameraOk) startCameraServer();   // port 80 (settings + /whoami) and 81 (stream)
    announceReady();
  } else {
    startSetupMode();
  }
}

// =============================== Loop ================================
void loop() {
  readSerial();

  if (setupMode) {
    dns.processNextRequest();
    setupServer.handleClient();
    // Nobody set it up? The router may have been off. Try the WiFi again.
    if (netCount > 0 && millis() - setupStartedMs > SETUP_WINDOW_MS) {
      Serial.println(F("# Setup time is over - restarting to try the WiFi again."));
      delay(200);
      ESP.restart();
    }
    delay(2);
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("# WiFi dropped - reconnecting..."));
    if (wifiMulti.run(10000) == WL_CONNECTED) announceReady();
  } else if (millis() - lastStatusMs > STATUS_INTERVAL_MS) {
    lastStatusMs = millis();
    Serial.printf("# STATUS  station=%d  wifi=%s  ip=%s  signal=%d dBm\n", station,
                  WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
    printHello();
  }
  delay(100);
}
