/* =====================================================================
 * PorpoiseNet_CameraNode  --  the camera, as a node on the same network
 * ---------------------------------------------------------------------
 * THE IDEA IN ONE PARAGRAPH
 *   This board runs the SAME PorpoiseNet code as the rovers in the Find
 *   the Target game. It is not a separate system with a bridge bolted
 *   on; it is another node. When a motion sensor two hundred feet away
 *   trips, this camera hears the alert directly over ESP-NOW - no
 *   router, no server, no cabling - takes a picture within about a
 *   quarter of a second, and broadcasts back what it saved. Everyone on
 *   the network, including the base station, sees both halves.
 *
 *   The exact same thing happens if a rover announces a target: the
 *   camera photographs it. One network, two projects, one code base.
 *
 * WHAT TRIGGERS A PICTURE
 *   - PN_ALERT      a sensor node tripped                 (security)
 *   - PN_TARGET     a rover found the target              (the game)
 *   - PN_CMD_SNAPSHOT  someone typed "snapshot" at the base station
 *   - the local PIR pin, if one is wired to this board
 *
 * WHERE THE PICTURE GOES - AND WHERE IT NEVER GOES
 *   NEVER over ESP-NOW. An ESP-NOW packet holds 250 bytes; a small JPEG
 *   is 15,000. It is not a matter of being patient - the protocol simply
 *   cannot carry it, and chopping an image into 60 unencrypted broadcast
 *   fragments would be a bad idea even if it could.
 *   What travels over ESP-NOW is the EVENT ("zone 3, motion, 14:32") and
 *   the RECEIPT ("saved n21_000007.jpg, 18 kB"). The image itself goes:
 *     - to an SD card in this board, and/or
 *     - over ordinary WiFi to a server you control (USE_WIFI_UPLOAD).
 *   That split is the whole architecture. Small facts on the mesh, big
 *   data on real infrastructure.
 *
 * *** THE CHANNEL TRAP - READ THIS BEFORE YOU DEBUG ANYTHING ***
 *   An ESP32 has ONE radio and can only be on ONE WiFi channel. If this
 *   board joins a router, the ROUTER picks the channel, and this board
 *   stops hearing the rest of the mesh - silently, with no error
 *   anywhere. Every "the camera worked on the bench and died in the
 *   field" story is this.
 *   With USE_WIFI_UPLOAD 1, this sketch connects to WiFi FIRST, then
 *   starts PorpoiseNet with PN_CHANNEL_FOLLOW_WIFI, which reads the
 *   channel the router handed out and PRINTS IT IN CAPITALS. Put that
 *   number in MY_CHANNEL on every other board, or lock the router to a
 *   fixed channel and use that everywhere. Full explanation in
 *   PorpoiseNet.h, STEP 6.
 *
 * BOARD / IDE SETTINGS
 *   Board:            "AI Thinker ESP32-CAM"  (or "ESP32 Wrover Module"
 *                     for the Freenove kit - change CAMERA_MODEL below)
 *   Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
 *   PSRAM:            Enabled
 *   Serial monitor:   115200 baud
 *   This folder must also contain camera_pins.h (it does) and
 *   PorpoiseNet.h, unless PorpoiseNet is installed as a library.
 *
 * FLASHING AN AI-THINKER ESP32-CAM
 *   It has no USB port. Use an FTDI adapter at 5V, and jumper GPIO 0 to
 *   GND before pressing reset to enter flash mode. REMOVE THAT JUMPER
 *   afterwards or it will never run your program.
 *
 * PIN BUDGET ON THE AI-THINKER BOARD - it is nearly full
 *   The camera uses most of the GPIO. What is left, and the catches:
 *     GPIO 0    camera clock AND the flash-mode jumper. Do not use.
 *     GPIO 4    the very bright white LED. Also SD data line 1 in 4-bit
 *               mode, which is why this sketch uses SD in 1-BIT mode.
 *     GPIO 12   free in 1-bit SD mode, but it is a boot strapping pin -
 *               if it is held HIGH at power-on the board will not start.
 *               Safe for a PIR (idle LOW), risky for a switch to 3.3 V.
 *     GPIO 13   free in 1-bit SD mode. This is the PIR pin below.
 *     GPIO 16   free if PSRAM is not using it - it usually is. Avoid.
 *   If you need more pins than this, that is the signal to use a second
 *   cheap ESP32 as the sensor node instead of overloading the camera.
 *
 * PRIVACY - NOT OPTIONAL, NOT A CODE PROBLEM
 *   This is a camera that will point at a place where students are. The
 *   questions in homeland-security-camera/README.md - who owns the
 *   footage, who may look at it, how long it is kept, what the school
 *   has agreed to IN WRITING - are not answered by any line of this
 *   file, and must be settled before anything is mounted on a wall.
 *   Two things the code does do:
 *     - it stores a zone NUMBER, never a room name or a person's name,
 *     - it never puts an image on the radio, so nothing recognisable is
 *       broadcast over an unencrypted link.
 *   Those help. They are not consent.
 * ===================================================================== */

#if __has_include(<PorpoiseNet.h>)
  #include <PorpoiseNet.h>
#else
  #include "PorpoiseNet.h"
#endif

#include "esp_camera.h"

// ======================================================================
// ================  THE THREE LINES YOU MUST GET RIGHT  ================
// ======================================================================
#define MY_NODE_ID   21   // cameras are 20-29 by convention
#define MY_NET_ID     7   // SAME on every board on this site
#define MY_CHANNEL    1   // SAME on every board - IGNORED if USE_WIFI_UPLOAD
                          // is 1, because the router picks the channel then
// ======================================================================

// ---- Which camera board? Exactly one of these. ----
#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_WROVER_KIT     // the Freenove kit used elsewhere in this repo
#include "camera_pins.h"

// ---- Options ----
#define USE_SD_CARD       1    // save JPEGs to the microSD slot
#define USE_WIFI_UPLOAD   0    // ALSO push them to a server (read the channel trap)
#define USE_LOCAL_PIR     0    // a PIR wired to this board as well
#define USE_FLASH_LED     0    // fire the white LED when capturing. Very bright.
                               // Also: it announces the camera's position to
                               // anyone watching, which may be the opposite
                               // of what a security camera wants.

#define PIR_PIN          13    // see the pin budget note above
#define FLASH_LED_PIN     4

#define ROSTER_EVERY_MS      15000
#define CAPTURE_COOLDOWN_MS   3000   // ignore repeat triggers for this long
#define FRAME_SIZE       FRAMESIZE_SVGA   // 800x600, about 20-40 kB per frame
#define JPEG_QUALITY     12               // 10 best ... 63 worst

#if USE_SD_CARD
  #include "FS.h"
  #include "SD_MMC.h"
#endif

#if USE_WIFI_UPLOAD
  #include <WiFi.h>
  #include <HTTPClient.h>
  // Real values live in secrets.h, which .gitignore blocks from ever
  // being committed. Copy secrets.example.h to secrets.h and fill it in.
  // THIS REPOSITORY IS PUBLIC: a password typed into this file instead
  // would be readable by everyone, forever, even after it is deleted.
  #include "secrets.h"
#endif

// ============================== State =================================
bool          armed          = true;    // DISARM stops it reacting to alerts
uint32_t      captureCount   = 0;
uint32_t      failCount      = 0;
unsigned long lastCaptureMs  = 0;
unsigned long lastRoster     = 0;
bool          sdReady        = false;

// ============================ The camera ==============================
bool startCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAME_SIZE;
  config.jpeg_quality = JPEG_QUALITY;
  config.fb_count     = 1;
  config.grab_mode    = CAMERA_GRAB_LATEST;

  // Without PSRAM the chip cannot hold a large frame. Drop the size
  // rather than failing, so a cheaper board still works, just smaller.
  if (!psramFound()) {
    Serial.println(F("# no PSRAM found - falling back to a smaller frame"));
    config.frame_size   = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_location  = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.print(F("# CAMERA FAILED TO START, error 0x")); Serial.println(err, HEX);
    Serial.println(F("#   Usual causes: wrong board selected in the IDE,"));
    Serial.println(F("#   the ribbon connector not fully latched, or a USB"));
    Serial.println(F("#   port that cannot supply enough current. The camera"));
    Serial.println(F("#   pulls a lot the instant it powers up."));
    return false;
  }
  Serial.println(F("# camera ready"));
  return true;
}

// ============================ The SD card =============================
bool startSdCard() {
#if USE_SD_CARD
  // 1-bit mode (the "true" argument). Slower than 4-bit, but it frees
  // GPIO 4, 12 and 13 - which is the difference between having pins for
  // a sensor and not having any.
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println(F("# no SD card - captures will not be saved."));
    Serial.println(F("#   Card inserted? Formatted FAT32? 32 GB or smaller?"));
    return false;
  }
  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println(F("# SD slot responded but no card is in it."));
    return false;
  }
  Serial.print(F("# SD card ready, "));
  Serial.print((uint32_t)(SD_MMC.cardSize() / (1024 * 1024)));
  Serial.println(F(" MB"));
  return true;
#else
  return false;
#endif
}

// ====================== capture and file it ===========================
// Returns the PnEvidence describing what happened, ready to broadcast.
PnEvidence captureFor(uint16_t aboutOrigin, uint32_t aboutSeq, const char *why) {
  PnEvidence ev = {};
  ev.aboutOrigin = aboutOrigin;
  ev.aboutSeq    = aboutSeq;
  ev.result      = 0;

  Serial.print(F("# CAPTURE triggered by ")); Serial.println(why);

#if USE_FLASH_LED
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(80);                       // let the sensor's exposure settle
#endif

  camera_fb_t *fb = esp_camera_fb_get();

#if USE_FLASH_LED
  digitalWrite(FLASH_LED_PIN, LOW);
#endif

  if (!fb) {
    failCount++;
    Serial.println(F("# capture FAILED - no frame came back from the sensor"));
    pnCopyStr(ev.name, sizeof(ev.name), "capture-failed");
    return ev;
  }

  captureCount++;
  ev.bytes = fb->len;

  // File name: node number and a counter. Deliberately contains no
  // location, no room name and nothing about who or what is in it.
  char filename[32];
  snprintf(filename, sizeof(filename), "n%u_%06u.jpg",
           (unsigned)MY_NODE_ID, (unsigned)captureCount);
  pnCopyStr(ev.name, sizeof(ev.name), filename);

#if USE_SD_CARD
  if (sdReady) {
    char path[40];
    snprintf(path, sizeof(path), "/%s", filename);
    File f = SD_MMC.open(path, FILE_WRITE);
    if (f) {
      f.write(fb->buf, fb->len);
      f.close();
      ev.result |= 1;
      Serial.print(F("# saved "));  Serial.print(path);
      Serial.print(F("  "));        Serial.print(fb->len);
      Serial.println(F(" bytes"));
    } else {
      Serial.println(F("# could not open the file for writing - card full?"));
    }
  }
#endif

#if USE_WIFI_UPLOAD
  // Ordinary HTTP over ordinary WiFi. This is the ONLY path the image
  // itself ever takes off this board.
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(UPLOAD_URL);
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("X-Node-Id", String(MY_NODE_ID));
    http.addHeader("X-Image-Name", filename);
    int code = http.POST(fb->buf, fb->len);
    if (code > 0 && code < 300) {
      ev.result |= 2;
      Serial.print(F("# uploaded, server said ")); Serial.println(code);
    } else {
      Serial.print(F("# upload failed, code ")); Serial.println(code);
    }
    http.end();
  } else {
    Serial.println(F("# upload skipped - WiFi is not connected"));
  }
#endif

  esp_camera_fb_return(fb);   // hand the buffer back or the next call fails
  return ev;
}

// Capture, then tell the network what happened. The receipt is what
// makes this two-way: the sensor that tripped learns that its alert
// actually resulted in a picture.
void captureAndReport(uint16_t aboutOrigin, uint32_t aboutSeq, const char *why) {
  if (!armed) {
    Serial.println(F("# trigger ignored - this camera is DISARMED"));
    return;
  }
  if (millis() - lastCaptureMs < CAPTURE_COOLDOWN_MS) {
    Serial.println(F("# trigger ignored - still in the cooldown window"));
    return;
  }
  lastCaptureMs = millis();

  PnEvidence ev = captureFor(aboutOrigin, aboutSeq, why);
  PorpoiseNet.sendEvidence(ev);
}

// ====================== messages from the network =====================
void onNetworkMessage(const PnMessage &msg, int8_t rssi) {
  (void)rssi;
  switch (msg.type) {

    // ---------- a perimeter sensor tripped: photograph it ----------
    case PN_ALERT: {
      PnAlert a;
      if (!PorpoiseNet.asAlert(msg, a)) return;
      Serial.print(F("# ALERT from node ")); Serial.print(msg.origin);
      Serial.print(F(", zone "));            Serial.print(a.zone);
      Serial.print(F(", severity "));        Serial.println(a.severity);
      captureAndReport(msg.origin, msg.seq, "a perimeter alert");
      break;
    }

    // ---------- a rover found the game target: photograph that too ----------
    // Nothing here is security-specific. The camera does not know or
    // care which project it is part of today.
    case PN_TARGET: {
      Serial.print(F("# vehicle ")); Serial.print(msg.origin);
      Serial.println(F(" found the target"));
      captureAndReport(msg.origin, msg.seq, "a rover's target report");
      break;
    }

    case PN_COMMAND: {
      PnCommand c;
      if (!PorpoiseNet.asCommand(msg, c)) return;
      switch (c.command) {
        case PN_CMD_SNAPSHOT:
          Serial.println(F("# COMMAND: SNAPSHOT"));
          lastCaptureMs = 0;                       // an explicit order skips the cooldown
          captureAndReport(msg.origin, msg.seq, "an operator request");
          break;
        case PN_CMD_ARM:
          armed = true;
          Serial.println(F("# COMMAND: ARM - reacting to alerts again"));
          break;
        case PN_CMD_DISARM:
          armed = false;
          Serial.println(F("# COMMAND: DISARM - alerts will be ignored"));
          break;
        case PN_CMD_PING:
          PorpoiseNet.say(armed ? "camera armed" : "camera disarmed", msg.origin);
          break;
        case PN_CMD_IDENTIFY:
#if USE_FLASH_LED
          for (int i = 0; i < 6; i++) {
            digitalWrite(FLASH_LED_PIN, HIGH); delay(120);
            digitalWrite(FLASH_LED_PIN, LOW);  delay(120);
          }
#endif
          Serial.println(F("# COMMAND: IDENTIFY - this is the camera"));
          break;
        default:
          break;
      }
      break;
    }

    case PN_TEXT:
      Serial.print(F("# node ")); Serial.print(msg.origin);
      Serial.print(F(" says: ")); Serial.println((const char *)msg.payload);
      break;

    default:
      break;
  }
}

// ============================== Setup =================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("# ====================================="));
  Serial.print  (F("#  PorpoiseNet_CameraNode - node "));
  Serial.println(MY_NODE_ID);
  Serial.println(F("# ====================================="));

#if USE_FLASH_LED
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
#endif
#if USE_LOCAL_PIR
  pinMode(PIR_PIN, INPUT);
#endif

  if (!startCamera()) {
    // Carry on anyway: a camera node that cannot see is still a useful
    // relay for the mesh, and saying so is better than a dead board.
    Serial.println(F("# continuing without a camera - this node will still"));
    Serial.println(F("# pass messages along for the rest of the network."));
  }
  sdReady = startSdCard();

  uint8_t channel = MY_CHANNEL;

#if USE_WIFI_UPLOAD
  // ORDER MATTERS. Join the router first, then start PorpoiseNet on
  // whatever channel the router put us on. Doing it the other way round
  // silently breaks the mesh - see the channel trap at the top.
  Serial.print(F("# joining WiFi "));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(400);
    Serial.print(F("."));
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("# WiFi connected, IP ")); Serial.println(WiFi.localIP());
    channel = PN_CHANNEL_FOLLOW_WIFI;   // adopt the router's channel
  } else {
    Serial.println(F("# WiFi did NOT connect. Falling back to MY_CHANNEL so"));
    Serial.println(F("# that the mesh still works and images are saved to SD."));
    WiFi.disconnect();
  }
#endif

  if (!PorpoiseNet.begin(MY_NODE_ID, PN_ROLE_CAMERA, channel, MY_NET_ID)) {
    Serial.println(F("# Radio failed to start - halting."));
    while (true) delay(1000);
  }
  PorpoiseNet.onMessage(onNetworkMessage);

  Serial.println(F("# Ready. Waiting for something to photograph."));
}

// =============================== Loop =================================
void loop() {
  PorpoiseNet.loop();

#if USE_LOCAL_PIR
  // A PIR wired straight to this board. It also broadcasts an alert, so
  // the base station sees the trip and not only the resulting image.
  static bool pirWasHigh = false;
  bool pirHigh = (digitalRead(PIR_PIN) == HIGH);
  if (pirHigh && !pirWasHigh && armed) {
    PnAlert a = {};
    a.sensor   = PN_SENSOR_MOTION;
    a.severity = 2;
    a.zone     = MY_NODE_ID;      // a NUMBER. Never a room name.
    a.count    = captureCount + 1;
    pnCopyStr(a.note, sizeof(a.note), "camera-local PIR");
    PorpoiseNet.raiseAlert(a, true);
    captureAndReport(MY_NODE_ID, 0, "this camera's own PIR");
  }
  pirWasHigh = pirHigh;
#endif

  if (millis() - lastRoster >= ROSTER_EVERY_MS) {
    lastRoster = millis();
    PorpoiseNet.printRoster();
    Serial.print(F("# captures "));  Serial.print(captureCount);
    Serial.print(F("  failed "));    Serial.print(failCount);
    Serial.print(F("  state "));     Serial.println(armed ? F("ARMED") : F("DISARMED"));
  }
}
