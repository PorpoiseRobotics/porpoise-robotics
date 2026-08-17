/* =====================================================================
 * CameraWebServer_v3  --  Freenove ESP32-WROVER camera node
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   Streams live MJPEG video over WiFi. The base-station dashboard embeds
 *   the stream at:   http://<camera-ip>:81/stream
 *   The camera's own tuning page (stock Espressif UI) lives at:
 *                    http://<camera-ip>/
 *
 * BOARD / IDE SETTINGS
 *   Board:            "ESP32 Wrover Module"
 *   Core:             esp32 by Espressif Systems, v3.0.7
 *   Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
 *   Serial monitor:   115200 baud
 *   Sketch folder must also contain: app_httpd.cpp, camera_index.h,
 *   camera_pins.h (these are the stock Espressif files, unchanged).
 *
 * WIFI: EDIT THE NETWORKS[] LIST BELOW
 *   Add every network the camera might use (home, school, hotspot...).
 *   On boot the camera scans and joins the strongest one it knows.
 *   If it can't join any, it prints a scan of what it CAN see so you
 *   can spot typos, then keeps retrying.
 *
 * HOW TO TELL IT IS WORKING (serial monitor @115200)
 *   - Boot banner, camera-init PASS/FAIL, PSRAM check.
 *   - "WiFi connected" with SSID, IP, and the two URLs above.
 *   - A status line every 30 s (SSID / IP / signal strength).
 *   - Automatic reconnect messages if WiFi drops.
 * ===================================================================== */

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiMulti.h>

// ===================
// Select camera model  (Freenove ESP32-WROVER kit = WROVER_KIT)
// ===================
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE  // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_CAMS3_UNIT  // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ======================================================================
// WIFI NETWORK LIST - add or remove rows as needed.
// The camera joins the strongest network in this list that it can see.
// ======================================================================
struct WifiCred { const char *ssid; const char *password; };

const WifiCred NETWORKS[] = {
  { "Kiln Guest",     "kilnguests"     },
  { "SpectrumSetup-D8",     "violetocean004"     },
  { "Porpoise",     "*9293kpb"     },
  // { "YourHomeWiFi",   "yourpassword"   },
  // { "PhoneHotspot",   "hotspotpass"    },
};
const int NUM_NETWORKS = sizeof(NETWORKS) / sizeof(NETWORKS[0]);

#define WIFI_CONNECT_TIMEOUT_MS  15000   // per connection attempt
#define STATUS_INTERVAL_MS       30000   // periodic health line

// Implemented in app_httpd.cpp (stock Espressif file):
void startCameraServer();
void setupLedFlash(int pin);

WiFiMulti wifiMulti;
unsigned long lastStatusMs = 0;
bool wasConnected = false;

// ======================================================================
// Print every network the radio can currently see. Called when we fail
// to connect, so a typo in NETWORKS[] is obvious ("my SSID isn't listed
// / is listed with a different name").
// ======================================================================
void printWifiScan() {
  Serial.println("Scanning for visible networks...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("  (none found - antenna attached? 2.4 GHz network?)");
    return;
  }
  for (int i = 0; i < n; i++) {
    Serial.printf("  %2d: %-28s  %4d dBm  ch %d\n",
                  i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
  }
  WiFi.scanDelete();
}

// ======================================================================
// Block until WiFi is connected. Retries forever, printing diagnostics
// each round so the serial monitor always explains what is happening.
// ======================================================================
void connectWifi() {
  Serial.printf("Trying %d known network(s):\n", NUM_NETWORKS);
  for (int i = 0; i < NUM_NETWORKS; i++) {
    Serial.printf("  - %s\n", NETWORKS[i].ssid);
  }

  while (true) {
    if (wifiMulti.run(WIFI_CONNECT_TIMEOUT_MS) == WL_CONNECTED) {
      Serial.println();
      Serial.println("WiFi connected");
      Serial.printf("  SSID:    %s\n", WiFi.SSID().c_str());
      Serial.printf("  IP:      %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("  Signal:  %d dBm\n", WiFi.RSSI());
      return;
    }
    Serial.println("Could not join any known network.");
    printWifiScan();
    Serial.println("Retrying in 5 s...");
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(300);
  Serial.println();
  Serial.println("=================================");
  Serial.println(" CameraWebServer_v3 (camera node)");
  Serial.println("=================================");

  // ---------------- Camera configuration (stock Espressif) -----------
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
  config.frame_size = FRAMESIZE_UXGA;      // trimmed later if no PSRAM
  config.pixel_format = PIXFORMAT_JPEG;    // JPEG = streaming mode
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;                // lower number = better quality
  config.fb_count = 1;

  // With PSRAM we can afford double-buffering and better JPEG quality;
  // without it, shrink the frame and keep the buffer in internal RAM.
  if (psramFound()) {
    Serial.println("PSRAM: found (full resolution available)");
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println("PSRAM: NOT found - check board selection (need Wrover)");
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // ---------------- Camera init: the #1 failure point ----------------
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("FATAL: camera init failed (0x%x). Check the ribbon\n", err);
    Serial.println("cable seating and the camera model #define. Reset to retry.");
    return;   // nothing else can work without the camera
  }
  Serial.println("Camera init: PASS");

  sensor_t *s = esp_camera_sensor_get();
  // OV3660 modules ship flipped and over-saturated; correct that here.
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  // Start at QVGA for a fast, smooth stream; raise it from the web UI.
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  // ---------------- WiFi ----------------
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);                 // sleep off = smoother streaming
  for (int i = 0; i < NUM_NETWORKS; i++) {
    wifiMulti.addAP(NETWORKS[i].ssid, NETWORKS[i].password);
  }
  connectWifi();

  // ---------------- HTTP servers ----------------
  startCameraServer();                  // port 80 (UI) + 81 (stream)

  Serial.println();
  Serial.println("Camera ready!");
  Serial.printf("  Control page:  http://%s/\n", WiFi.localIP().toString().c_str());
  Serial.printf("  Video stream:  http://%s:81/stream\n", WiFi.localIP().toString().c_str());
  Serial.println("Paste the stream URL into the dashboard's Camera settings.");
}

void loop() {
  // The web server runs in its own tasks; the loop just watches WiFi
  // health and reports it so the serial monitor is never silent.
  bool connected = (WiFi.status() == WL_CONNECTED);

  if (!connected && wasConnected) {
    Serial.println("WiFi DROPPED - reconnecting...");
  }
  if (!connected) {
    // wifiMulti.run() re-scans and rejoins the best known network.
    if (wifiMulti.run(WIFI_CONNECT_TIMEOUT_MS) == WL_CONNECTED) {
      Serial.printf("WiFi reconnected: %s  IP %s\n",
                    WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      Serial.printf("  Video stream:  http://%s:81/stream\n",
                    WiFi.localIP().toString().c_str());
    }
  } else if (millis() - lastStatusMs > STATUS_INTERVAL_MS) {
    lastStatusMs = millis();
    Serial.printf("STATUS  ssid=%s  ip=%s  rssi=%d dBm  uptime=%lus\n",
                  WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
                  WiFi.RSSI(), millis() / 1000);
  }
  wasConnected = connected;
  delay(500);
}
