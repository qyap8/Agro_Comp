#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "encoder_filter.h"
#include "time_tools.h"

// ---------------------- Display -----------------------
static const uint8_t TFT_SCLK_PIN = 12;
static const uint8_t TFT_MOSI_PIN = 11;
static const uint8_t TFT_CS_PIN = 10;
static const uint8_t TFT_DC_PIN = 9;
static const uint8_t TFT_RST_PIN = 8;
static const uint8_t TFT_BL_PIN = 7;

static const uint16_t TFT_RAW_WIDTH = 240;
static const uint16_t TFT_RAW_HEIGHT = 320;
static const uint8_t TFT_ROTATION = 2;

// ---------------------- Input -------------------------
static const uint8_t ENC_A_PIN = 5;
static const uint8_t ENC_B_PIN = 6;
static const uint8_t ENC_BTN_PIN = 4;
static const uint8_t KEY0_PIN = 3;

// ---------------------- Outputs -----------------------
static const uint8_t ZONE_COUNT = 12;
static const uint8_t ZONE_PINS[ZONE_COUNT] = {13, 14, 15, 16, 17, 18, 21, 35, 36, 37, 38, 39};
static const uint8_t PUMP_PIN = 40;
static const bool ACTIVE_HIGH = true;

// ---------------------- Network -----------------------
const char* WIFI_SSID = "MIWIFI_PP6H";
const char* WIFI_PASS = "MM3tCFN7";
const char* NTP_SERVER = "pool.ntp.org";
int gmtOffsetMin = 180;
WiFiServer webServer(80);

// ---------------------- Models ------------------------
struct ZoneConfig {
  bool enabled;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t stopHour;
  uint8_t stopMinute;
  uint16_t durationSec;
  uint8_t daysMask; // Mon..Sun bits
};

ZoneConfig zones[ZONE_COUNT] = {
  {true, 6, 0, 6, 10, 600, 0b1111111},
  {true, 6, 10, 6, 20, 600, 0b1111111},
  {true, 6, 20, 6, 30, 600, 0b1111111},
  {true, 6, 30, 6, 40, 600, 0b1111111},
  {true, 6, 40, 6, 50, 600, 0b1111111},
  {true, 6, 50, 7, 0, 600, 0b1111111},
  {false, 7, 0, 7, 10, 600, 0b1111111},
  {false, 7, 10, 7, 20, 600, 0b1111111},
  {false, 7, 20, 7, 30, 600, 0b1111111},
  {false, 7, 30, 7, 40, 600, 0b1111111},
  {false, 7, 40, 7, 50, 600, 0b1111111},
  {false, 7, 50, 8, 0, 600, 0b1111111}
};

int lastZoneTriggerMinute[ZONE_COUNT];

bool autoEnabled = true;
bool cycleRunning = false;
bool cycleAll = false;
int8_t activeZone = -1;
int8_t cycleCursor = -1;
unsigned long zoneStartedAtMs = 0;
uint16_t activeDurationSec = 0;

// ---------------------- UI state ----------------------
Adafruit_ST7789 tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
EncoderFilter encoder;

enum UiScreen : uint8_t { SCREEN_MAIN = 0, SCREEN_ZONE_MENU };
UiScreen screenMode = SCREEN_MAIN;

int selectedZone = 0;
int listTop = 0;

enum MainFocus : uint8_t { FOCUS_ZONES = 0, FOCUS_ACTIONS };
MainFocus mainFocus = FOCUS_ZONES;
int selectedAction = 0; // 0=START,1=STOP,2=AUTO
bool uiForceFull = true;

enum ZoneField : uint8_t {
  FIELD_ENABLED = 0,
  FIELD_START,
  FIELD_STOP,
  FIELD_DURATION,
  FIELD_DAYS,
  FIELD_RUN_NOW,
  FIELD_BACK,
  FIELD_TOTAL
};

ZoneField zoneField = FIELD_ENABLED;
bool fieldEditMode = false;

unsigned long lastUiFrameMs = 0;
unsigned long lastStatusMs = 0;
bool uiDirty = true;

bool btnPrev = HIGH;
unsigned long btnDownMs = 0;
bool btnClick = false;
bool btnLong = false;
bool btnVeryLong = false;
unsigned long lastShortClickMs = 0;

bool keyPrev = HIGH;

String lastHeaderTime = "";
bool lastHeaderPump = false;
bool lastHeaderAuto = false;
int prevListTop = -1;
int prevSelectedZone = -1;
int prevSelectedAction = -1;
MainFocus prevMainFocus = FOCUS_ZONES;
ZoneField prevZoneField = FIELD_ENABLED;
bool prevFieldEditMode = false;
int prevZoneMenuZone = -1;
int prevZoneTopField = -1;

bool tailscaleEnabled = false;
String tailscaleHost = "";

// ---------------------- helpers -----------------------
void setOut(uint8_t pin, bool on) {
  digitalWrite(pin, (on == ACTIVE_HIGH) ? HIGH : LOW);
}

void zonesOff() {
  for (uint8_t i = 0; i < ZONE_COUNT; i++) setOut(ZONE_PINS[i], false);
  activeZone = -1;
}

bool pumpIsOn() {
  int v = digitalRead(PUMP_PIN);
  return ACTIVE_HIGH ? (v == HIGH) : (v == LOW);
}

void pumpOn() { setOut(PUMP_PIN, true); }
void pumpOff() { setOut(PUMP_PIN, false); }

String dayMaskText(uint8_t mask) {
  if (mask == 0b1111111) return "Everyday";
  if (mask == 0b0010101) return "MWF";
  if (mask == 0b1100000) return "Weekend";
  char buf[12];
  snprintf(buf, sizeof(buf), "0x%02X", mask);
  return String(buf);
}

void stopCycle() {
  cycleRunning = false;
  cycleAll = false;
  activeDurationSec = 0;
  zoneStartedAtMs = 0;
  zonesOff();
  pumpOff();
  uiDirty = true;
}

void startZone(uint8_t z, uint16_t sec) {
  zonesOff();
  if (z >= ZONE_COUNT || sec == 0) return;
  setOut(ZONE_PINS[z], true);
  pumpOn();
  cycleRunning = true;
  activeZone = z;
  activeDurationSec = sec;
  zoneStartedAtMs = millis();
  uiDirty = true;
}

void startAllCycle() {
  cycleAll = true;
  cycleRunning = true;
  cycleCursor = -1;
  for (uint8_t i = 0; i < ZONE_COUNT; i++) {
    if (zones[i].enabled && zones[i].durationSec > 0) {
      cycleCursor = i;
      startZone(i, zones[i].durationSec);
      return;
    }
  }
  stopCycle();
}

void advanceAllCycle() {
  if (!cycleAll) { stopCycle(); return; }
  int next = cycleCursor + 1;
  while (next < ZONE_COUNT && (!zones[next].enabled || zones[next].durationSec == 0)) next++;
  if (next >= ZONE_COUNT) {
    stopCycle();
    return;
  }
  cycleCursor = next;
  startZone((uint8_t)next, zones[next].durationSec);
}

void ensureTimeSync() {
  configTime(gmtOffsetMin * 60, 0, NTP_SERVER);
  struct tm t;
  int retries = 0;
  while (!getLocalTime(&t) && retries < 20) {
    delay(200);
    retries++;
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(250);
  if (WiFi.status() == WL_CONNECTED) {
    webServer.begin();
    ensureTimeSync();
  }
}

int getQueryInt(const String& req, const String& key, int defVal) {
  int keyPos = req.indexOf(key + "=");
  if (keyPos < 0) return defVal;
  int valStart = keyPos + key.length() + 1;
  int valEnd = req.indexOf('&', valStart);
  if (valEnd < 0) valEnd = req.indexOf(' ', valStart);
  if (valEnd < 0) valEnd = req.length();
  String v = req.substring(valStart, valEnd);
  v.trim();
  return v.length() ? v.toInt() : defVal;
}

String zoneEditPage(int zi) {
  if (zi < 0 || zi >= ZONE_COUNT) zi = 0;
  ZoneConfig& z = zones[zi];

  String h;
  h += "<!doctype html><html><head><meta charset='utf-8'>";
  h += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  h += "<style>body{font-family:Arial;background:#111;color:#eef;padding:12px}.card{background:#1e2530;padding:12px;border-radius:10px;margin:8px 0}.btn{display:inline-block;padding:8px 12px;margin:4px;border-radius:8px;color:#fff;text-decoration:none}.b1{background:#0a84ff}.b2{background:#2ecc71}.b3{background:#e67e22}.b4{background:#e74c3c}.sm{padding:6px 10px}.row{margin:8px 0}</style></head><body>";
  h += "<h2>Zone " + String(zi + 1) + " editor</h2>";
  h += "<div class='card'>";
  h += "<div>Status: " + String(z.enabled ? "ON" : "OFF") + " <a class='btn b1 sm' href='/setzone?z=" + String(zi + 1) + "&en=" + String(z.enabled ? 0 : 1) + "'>Toggle</a></div>";

  auto hhmmLinks = [&](const String& label, int hour, int minute, const String& hKey, const String& mKey) {
    String r;
    r += "<div class='row'>" + label + ": <b>" + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + "</b>";
    r += " <a class='btn b3 sm' href='/setzone?z=" + String(zi + 1) + "&" + hKey + "=" + String((hour + 23) % 24) + "'>H-</a>";
    r += " <a class='btn b2 sm' href='/setzone?z=" + String(zi + 1) + "&" + hKey + "=" + String((hour + 1) % 24) + "'>H+</a>";
    r += " <a class='btn b3 sm' href='/setzone?z=" + String(zi + 1) + "&" + mKey + "=" + String((minute + 55) % 60) + "'>M-</a>";
    r += " <a class='btn b2 sm' href='/setzone?z=" + String(zi + 1) + "&" + mKey + "=" + String((minute + 5) % 60) + "'>M+</a>";
    r += "</div>";
    return r;
  };

  h += hhmmLinks("Start", z.startHour, z.startMinute, "sh", "sm");
  h += hhmmLinks("Stop", z.stopHour, z.stopMinute, "eh", "em");

  h += "<div class='row'>Duration: <b>" + String(z.durationSec) + "s</b>";
  int dMinus = (int)z.durationSec - 30; if (dMinus < 30) dMinus = 30;
  int dPlus = (int)z.durationSec + 30; if (dPlus > 7200) dPlus = 7200;
  h += " <a class='btn b3 sm' href='/setzone?z=" + String(zi + 1) + "&dur=" + String(dMinus) + "'>-30s</a>";
  h += " <a class='btn b2 sm' href='/setzone?z=" + String(zi + 1) + "&dur=" + String(dPlus) + "'>+30s</a></div>";

  h += "<div class='row'>Days: <b>" + dayMaskText(z.daysMask) + "</b>";
  h += " <a class='btn b1 sm' href='/setzone?z=" + String(zi + 1) + "&days=127'>Everyday</a>";
  h += " <a class='btn b1 sm' href='/setzone?z=" + String(zi + 1) + "&days=21'>MWF</a>";
  h += " <a class='btn b1 sm' href='/setzone?z=" + String(zi + 1) + "&days=96'>Weekend</a></div>";

  h += "<div class='row'><a class='btn b2' href='/run?z=" + String(zi + 1) + "'>Run now</a>";
  h += " <a class='btn b4' href='/'>Back</a></div>";
  h += "</div></body></html>";
  return h;
}

String webPage() {
  String h;
  h += "<!doctype html><html><head><meta charset='utf-8'>";
  h += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  h += "<style>body{font-family:Arial;background:#111;color:#eef;padding:12px}.card{background:#1e2530;padding:12px;border-radius:10px;margin:8px 0}.btn{display:inline-block;padding:8px 12px;margin:4px;border-radius:8px;color:#fff;text-decoration:none}.b1{background:#0a84ff}.b2{background:#2ecc71}.b3{background:#e67e22}.b4{background:#e74c3c}table{width:100%;font-size:13px}td{padding:3px}</style></head><body>";
  h += "<h2>Irrigation Controller</h2>";
  h += "<div class='card'>Time: " + localTimeString();
  h += "<br>Auto: " + String(autoEnabled ? "ON" : "OFF");
  h += "<br>Pump: " + String(pumpIsOn() ? "ON" : "OFF");
  h += "</div>";

  h += "<div class='card'>";
  h += "<a class='btn b1' href='/startall'>Start all</a>";
  h += "<a class='btn b4' href='/stop'>Stop</a>";
  h += "<a class='btn b2' href='/auto/on'>Auto ON</a>";
  h += "<a class='btn b3' href='/auto/off'>Auto OFF</a>";
  h += "</div>";

  h += "<div class='card'><h3>Zones</h3><table>";
  for (int i = 0; i < ZONE_COUNT; i++) {
    h += "<tr><td><a class='btn b1' href='/zone?z=" + String(i + 1) + "'>Z" + String(i + 1) + "</a></td>";
    h += "<td>" + String(zones[i].enabled ? "ON" : "OFF") + "</td>";
    h += "<td>" + String(zones[i].startHour) + ":" + (zones[i].startMinute < 10 ? "0" : "") + String(zones[i].startMinute);
    h += "-" + String(zones[i].stopHour) + ":" + (zones[i].stopMinute < 10 ? "0" : "") + String(zones[i].stopMinute) + "</td>";
    h += "<td>" + String(zones[i].durationSec) + "s</td>";
    h += "<td><a class='btn b1' href='/run?z=" + String(i + 1) + "'>Run</a></td></tr>";
  }
  h += "</table></div>";

  h += "<div class='card'><b>Tailscale remote access:</b><br>";
  h += "State: " + String(tailscaleEnabled ? "ENABLED" : "DISABLED") + "<br>";
  h += "Tailnet Host: " + (tailscaleHost.length() ? tailscaleHost : String("(not set)")) + "<br>";
  h += "Use subnet-router or device with Tailscale in your LAN.<br>";
  h += "<a class='btn b2' href='/tailscale/on'>Enable</a>";
  h += "<a class='btn b4' href='/tailscale/off'>Disable</a><br>";
  h += "Set host: /tailscale/set?host=irrigation.tailnet-name.ts.net";
  if (tailscaleEnabled && tailscaleHost.length()) {
    h += "<br>Remote URL: http://" + tailscaleHost;
  }
  h += "</div>";

  h += "<div class='card'><b>Set time:</b> /settime?y=2026&mo=2&d=18&h=14&mi=30&s=00<br>";
  h += "<b>Set tz:</b> /tz?min=180</div>";

  h += "</body></html>";
  return h;
}

void handleWeb() {
  WiFiClient client = webServer.available();
  if (!client) return;

  client.setTimeout(300);
  String req = client.readStringUntil('
');
  req.trim();
  while (client.connected()) {
    String line = client.readStringUntil('
');
    if (line == "\r" || line.length() <= 1) break;
  }

  String body;

  if (req.indexOf("GET /startall") >= 0) startAllCycle();
  else if (req.indexOf("GET /stop") >= 0) stopCycle();
  else if (req.indexOf("GET /auto/on") >= 0) autoEnabled = true;
  else if (req.indexOf("GET /auto/off") >= 0) autoEnabled = false;
  else if (req.indexOf("GET /tailscale/on") >= 0) tailscaleEnabled = true;
  else if (req.indexOf("GET /tailscale/off") >= 0) tailscaleEnabled = false;
  else if (req.indexOf("GET /tailscale/set") >= 0) {
    int hp = req.indexOf("host=");
    if (hp >= 0) {
      int st = hp + 5;
      int en = req.indexOf(' ', st);
      if (en < 0) en = req.length();
      tailscaleHost = req.substring(st, en);
      tailscaleHost.replace("%2E", ".");
      tailscaleHost.replace("%3A", ":");
      tailscaleHost.replace("%2F", "/");
      tailscaleHost.replace("%20", "");
    }
  } else if (req.indexOf("GET /run") >= 0) {
    int z = getQueryInt(req, "z", 1);
    if (z >= 1 && z <= ZONE_COUNT) startZone((uint8_t)(z - 1), zones[z - 1].durationSec);
  } else if (req.indexOf("GET /setzone") >= 0) {
    int z = getQueryInt(req, "z", 1) - 1;
    if (z >= 0 && z < ZONE_COUNT) {
      zones[z].enabled = getQueryInt(req, "en", zones[z].enabled ? 1 : 0) > 0;
      zones[z].startHour = constrain(getQueryInt(req, "sh", zones[z].startHour), 0, 23);
      zones[z].startMinute = constrain(getQueryInt(req, "sm", zones[z].startMinute), 0, 59);
      zones[z].stopHour = constrain(getQueryInt(req, "eh", zones[z].stopHour), 0, 23);
      zones[z].stopMinute = constrain(getQueryInt(req, "em", zones[z].stopMinute), 0, 59);
      zones[z].durationSec = constrain(getQueryInt(req, "dur", zones[z].durationSec), 30, 7200);
      zones[z].daysMask = constrain(getQueryInt(req, "days", zones[z].daysMask), 0, 127);
      uiDirty = true;
    }
  } else if (req.indexOf("GET /settime") >= 0) {
    int y = getQueryInt(req, "y", 2026);
    int mo = getQueryInt(req, "mo", 1);
    int d = getQueryInt(req, "d", 1);
    int h = getQueryInt(req, "h", 0);
    int mi = getQueryInt(req, "mi", 0);
    int s = getQueryInt(req, "s", 0);
    setManualDateTime(y, mo, d, h, mi, s);
  } else if (req.indexOf("GET /tz") >= 0) {
    gmtOffsetMin = constrain(getQueryInt(req, "min", gmtOffsetMin), -720, 840);
    applyTimezoneOffsetMinutes(gmtOffsetMin);
  }

  if (req.indexOf("GET /zone") >= 0) {
    int zi = getQueryInt(req, "z", 1) - 1;
    body = zoneEditPage(zi);
  } else {
    body = webPage();
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println("Connection: close");
  client.println();
  client.print(body);
  client.stop();
}

// ---------------------- UI render ---------------------
void drawHeaderDynamic(bool force = false) {
  String tm = localTimeString();
  bool pump = pumpIsOn();

  if (force || tm != lastHeaderTime) {
    tft.fillRect(148, 4, 88, 10, ST77XX_BLUE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
    tft.setCursor(150, 6);
    tft.print(tm);
    lastHeaderTime = tm;
  }

  if (force || pump != lastHeaderPump || autoEnabled != lastHeaderAuto) {
    tft.fillRect(148, 16, 88, 14, ST77XX_BLUE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
    tft.setCursor(150, 18);
    tft.print(String("P:") + (pump ? "ON" : "OFF") + " A:" + (autoEnabled ? "ON" : "OFF"));
    lastHeaderPump = pump;
    lastHeaderAuto = autoEnabled;
  }
}

void drawHeader() {
  tft.fillRect(0, 0, tft.width(), 34, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setCursor(6, 8);
  tft.print("IRR CTRL");
  drawHeaderDynamic(true);
}

void executeMainAction() {
  if (selectedAction == 0) startAllCycle();
  else if (selectedAction == 1) stopCycle();
  else if (selectedAction == 2) autoEnabled = !autoEnabled;
  uiDirty = true;
}

void drawMainButtons(bool force = false) {
  int y = tft.height() - 44;
  int bw = (tft.width() - 8) / 3;

  if (!force && prevSelectedAction == selectedAction && prevMainFocus == mainFocus) return;

  tft.fillRect(0, y, tft.width(), 44, ST77XX_BLACK);

  for (int i = 0; i < 3; i++) {
    int x = 2 + i * bw;
    bool sel = (mainFocus == FOCUS_ACTIONS && selectedAction == i);

    uint16_t base = (i == 0) ? ST77XX_GREEN : ((i == 1) ? ST77XX_RED : ST77XX_BLUE);
    uint16_t bg = sel ? ST77XX_YELLOW : base;
    uint16_t fg = sel ? ST77XX_BLACK : ST77XX_WHITE;

    tft.fillRoundRect(x, y + 4, bw - 2, 34, 4, bg);
    tft.drawRoundRect(x, y + 4, bw - 2, 34, 4, ST77XX_WHITE);

    tft.setTextSize(1);
    tft.setTextColor(fg, bg);
    tft.setCursor(x + 8, y + 16);
    if (i == 0) tft.print("START");
    else if (i == 1) tft.print("STOP");
    else tft.print(autoEnabled ? "AUTO ON" : "AUTO OFF");
  }

  prevSelectedAction = selectedAction;
  prevMainFocus = mainFocus;
}

void drawZoneRow(int z, int y, bool selected) {
  uint16_t bg = selected ? ST77XX_MAGENTA : ST77XX_BLACK;
  uint16_t fg = selected ? ST77XX_WHITE : ST77XX_CYAN;
  tft.fillRoundRect(4, y, tft.width() - 8, 28, 4, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(2);
  tft.setCursor(10, y + 7);
  int minDur = zones[z].durationSec / 60;
  tft.printf("Z%02d  %2dm  %s", z + 1, minDur, zones[z].enabled ? "ON" : "OFF");
}

void drawMainList(bool force = false) {
  if (selectedZone < 0) selectedZone = 0;
  if (selectedZone >= ZONE_COUNT) selectedZone = ZONE_COUNT - 1;

  const int rowH = 32;
  const int yStart = 42;
  const int listBottom = tft.height() - 48;
  int visibleRows = (listBottom - yStart) / rowH;
  if (visibleRows < 3) visibleRows = 3;

  if (selectedZone < listTop) listTop = selectedZone;
  if (selectedZone >= listTop + visibleRows) listTop = selectedZone - visibleRows + 1;

  if (force) {
    tft.fillScreen(ST77XX_BLACK);
    drawHeader();
    tft.drawFastHLine(0, 36, tft.width(), ST77XX_WHITE);
    tft.fillRect(0, 38, tft.width(), listBottom - 38, ST77XX_BLACK);

    for (int r = 0; r < visibleRows; r++) {
      int z = listTop + r;
      if (z >= ZONE_COUNT) break;
      int y = yStart + r * rowH;
      drawZoneRow(z, y, z == selectedZone && mainFocus == FOCUS_ZONES);
    }

    drawMainButtons(true);
    prevListTop = listTop;
    prevSelectedZone = selectedZone;
    return;
  }

  if (listTop != prevListTop) {
    tft.fillRect(0, 38, tft.width(), listBottom - 38, ST77XX_BLACK);
    for (int r = 0; r < visibleRows; r++) {
      int z = listTop + r;
      if (z >= ZONE_COUNT) break;
      int y = yStart + r * rowH;
      drawZoneRow(z, y, z == selectedZone && mainFocus == FOCUS_ZONES);
    }
    prevListTop = listTop;
    prevSelectedZone = selectedZone;
    return;
  }

  if (selectedZone != prevSelectedZone || mainFocus != prevMainFocus) {
    if (prevSelectedZone >= listTop && prevSelectedZone < listTop + visibleRows) {
      int yOld = yStart + (prevSelectedZone - listTop) * rowH;
      drawZoneRow(prevSelectedZone, yOld, false);
    }
    if (selectedZone >= listTop && selectedZone < listTop + visibleRows) {
      int yNew = yStart + (selectedZone - listTop) * rowH;
      drawZoneRow(selectedZone, yNew, mainFocus == FOCUS_ZONES);
    }
    prevSelectedZone = selectedZone;
  }

  drawMainButtons(false);
}

void drawZoneMenuRow(int idx, int y, ZoneConfig& z) {
  const char* labels[FIELD_TOTAL] = {
    "Enabled", "Start", "Stop", "Duration", "Days", "Run now", "Back"
  };

  bool sel = (zoneField == idx);
  uint16_t bg = sel ? ST77XX_CYAN : ST77XX_BLACK;
  uint16_t fg = sel ? ST77XX_BLACK : ST77XX_WHITE;

  tft.fillRoundRect(6, y, tft.width() - 12, 44, 5, bg);
  tft.drawRoundRect(6, y, tft.width() - 12, 44, 5, sel ? ST77XX_YELLOW : ST77XX_BLUE);

  tft.setTextSize(2);
  tft.setTextColor(fg, bg);
  tft.setCursor(12, y + 13);
  tft.print(labels[idx]);

  String val = "";
  if (idx == FIELD_ENABLED) val = z.enabled ? "ON" : "OFF";
  else if (idx == FIELD_START) {
    char b[8]; snprintf(b, sizeof(b), "%02d:%02d", z.startHour, z.startMinute); val = b;
  } else if (idx == FIELD_STOP) {
    char b[8]; snprintf(b, sizeof(b), "%02d:%02d", z.stopHour, z.stopMinute); val = b;
  } else if (idx == FIELD_DURATION) val = String(z.durationSec) + "s";
  else if (idx == FIELD_DAYS) val = dayMaskText(z.daysMask);
  else if (idx == FIELD_RUN_NOW) val = "RUN";
  else if (idx == FIELD_BACK) val = "EXIT";

  tft.setCursor(132, y + 13);
  tft.print(val);

  if (sel && fieldEditMode && idx <= FIELD_DAYS) {
    tft.setCursor(220, y + 13);
    tft.print("*");
  }
}

void drawZoneMenu(bool force = false) {
  ZoneConfig& z = zones[selectedZone];

  const int yStart = 74;
  const int rowH = 46;
  const int visibleRows = 4;
  int topField = (int)zoneField - visibleRows / 2;
  if (topField < 0) topField = 0;
  if (topField > FIELD_TOTAL - visibleRows) topField = FIELD_TOTAL - visibleRows;
  if (topField < 0) topField = 0;

  if (force || prevZoneMenuZone != selectedZone || topField != prevZoneTopField) {
    tft.fillScreen(ST77XX_BLACK);
    drawHeader();
    tft.drawFastHLine(0, 36, tft.width(), ST77XX_WHITE);
    tft.fillRect(0, 38, tft.width(), tft.height() - 38, ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.setCursor(8, 42);
    tft.printf("ZONE %02d", selectedZone + 1);

    for (int r = 0; r < visibleRows; r++) {
      int idx = topField + r;
      if (idx >= FIELD_TOTAL) break;
      drawZoneMenuRow(idx, yStart + r * rowH, z);
    }

    prevZoneField = zoneField;
    prevFieldEditMode = fieldEditMode;
    prevZoneMenuZone = selectedZone;
    prevZoneTopField = topField;
    return;
  }

  if (zoneField != prevZoneField || fieldEditMode != prevFieldEditMode) {
    if (prevZoneField >= topField && prevZoneField < topField + visibleRows) {
      drawZoneMenuRow(prevZoneField, yStart + (prevZoneField - topField) * rowH, z);
    }
    if (zoneField >= topField && zoneField < topField + visibleRows) {
      drawZoneMenuRow(zoneField, yStart + (zoneField - topField) * rowH, z);
    }
    prevZoneField = zoneField;
    prevFieldEditMode = fieldEditMode;
  } else {
    drawZoneMenuRow(zoneField, yStart + (zoneField - topField) * rowH, z);
  }
}

void renderUi(bool force = false, bool statusOnly = false) {
  if (screenMode == SCREEN_MAIN) {
    if (force) drawMainList(true);
    else if (!statusOnly) drawMainList(false);
    drawHeaderDynamic(force || statusOnly);
  } else {
    if (force || !statusOnly) drawZoneMenu(force);
    drawHeaderDynamic(force || statusOnly);
  }
}

// ---------------------- Input -------------------------
void pollButtons() {
  bool b = digitalRead(ENC_BTN_PIN);
  unsigned long now = millis();

  if (b != btnPrev) {
    btnPrev = b;
    if (b == LOW) {
      btnDownMs = now;
      btnLong = false;
    } else {
      unsigned long held = now - btnDownMs;
      if (held >= 1200) btnVeryLong = true;
      else if (held >= 600) btnLong = true;
      else {
        if (now - lastShortClickMs < 300) btnVeryLong = true;
        else btnClick = true;
        lastShortClickMs = now;
      }
    }
  }

  bool k = digitalRead(KEY0_PIN);
  if (k == HIGH && keyPrev == LOW) {
    stopCycle();
  }
  keyPrev = k;
}

void rotateDaysMask(ZoneConfig& z, int8_t step) {
  static const uint8_t presets[4] = {0b1111111, 0b0010101, 0b1100000, 0b0111110};
  int idx = 0;
  for (int i = 0; i < 4; i++) if (presets[i] == z.daysMask) idx = i;
  idx += step;
  if (idx < 0) idx = 3;
  if (idx > 3) idx = 0;
  z.daysMask = presets[idx];
}

void handleInput() {
  int8_t step = encoder.readStep();
  pollButtons();

  if (screenMode == SCREEN_MAIN) {
    if (step != 0) {
      if (mainFocus == FOCUS_ZONES) {
        selectedZone += step;
        if (selectedZone < 0) selectedZone = 0;
        if (selectedZone >= ZONE_COUNT) selectedZone = ZONE_COUNT - 1;
      } else {
        selectedAction += step;
        if (selectedAction < 0) selectedAction = 2;
        if (selectedAction > 2) selectedAction = 0;
      }
      uiDirty = true;
    }

    if (btnClick) {
      btnClick = false;
      if (mainFocus == FOCUS_ZONES) {
        screenMode = SCREEN_ZONE_MENU;
        uiForceFull = true;
        zoneField = FIELD_ENABLED;
        fieldEditMode = false;
      } else {
        executeMainAction();
      }
      uiDirty = true;
    }

    if (btnLong) {
      btnLong = false;
      mainFocus = (mainFocus == FOCUS_ZONES) ? FOCUS_ACTIONS : FOCUS_ZONES;
      uiDirty = true;
    }

    if (btnVeryLong) {
      btnVeryLong = false;
      stopCycle();
      uiDirty = true;
    }

    return;
  }

  ZoneConfig& z = zones[selectedZone];

  if (step != 0) {
    if (!fieldEditMode) {
      int f = (int)zoneField + step;
      if (f < 0) f = FIELD_TOTAL - 1;
      if (f >= FIELD_TOTAL) f = 0;
      zoneField = (ZoneField)f;
    } else {
      if (zoneField == FIELD_ENABLED) z.enabled = step > 0;
      else if (zoneField == FIELD_START) {
        int m = z.startHour * 60 + z.startMinute + step * 5;
        if (m < 0) m += 1440;
        if (m >= 1440) m -= 1440;
        z.startHour = m / 60;
        z.startMinute = m % 60;
      } else if (zoneField == FIELD_STOP) {
        int m = z.stopHour * 60 + z.stopMinute + step * 5;
        if (m < 0) m += 1440;
        if (m >= 1440) m -= 1440;
        z.stopHour = m / 60;
        z.stopMinute = m % 60;
      } else if (zoneField == FIELD_DURATION) {
        int d = (int)z.durationSec + step * 30;
        if (d < 30) d = 30;
        if (d > 7200) d = 7200;
        z.durationSec = (uint16_t)d;
      } else if (zoneField == FIELD_DAYS) {
        rotateDaysMask(z, step);
      }
    }
    uiDirty = true;
  }

  if (btnClick) {
    btnClick = false;

    if (zoneField == FIELD_RUN_NOW) {
      startZone((uint8_t)selectedZone, z.durationSec);
      uiDirty = true;
    } else if (zoneField == FIELD_BACK) {
      fieldEditMode = false;
      screenMode = SCREEN_MAIN;
      mainFocus = FOCUS_ZONES;
      uiForceFull = true;
      uiDirty = true;
    } else {
      fieldEditMode = !fieldEditMode;
      uiDirty = true;
    }
  }

  if (btnLong) {
    btnLong = false;
    if (fieldEditMode) {
      fieldEditMode = false;
      zoneField = (ZoneField)(((int)zoneField + 1) % FIELD_TOTAL);
      uiDirty = true;
    } else {
      screenMode = SCREEN_MAIN;
      mainFocus = FOCUS_ZONES;
      uiForceFull = true;
      uiDirty = true;
    }
  }

  if (btnVeryLong) {
    btnVeryLong = false;
    screenMode = SCREEN_MAIN;
    mainFocus = FOCUS_ZONES;
    uiForceFull = true;
    fieldEditMode = false;
    uiDirty = true;
  }
}

// ---------------------- runtime -----------------------
void updateSchedule() {
  if (!autoEnabled || cycleRunning) return;

  struct tm t;
  if (!getLocalTime(&t)) return;

  int minuteStamp = t.tm_yday * 1440 + t.tm_hour * 60 + t.tm_min;

  for (uint8_t i = 0; i < ZONE_COUNT; i++) {
    ZoneConfig& z = zones[i];
    if (!z.enabled) continue;
    if (!dayAllowed(z.daysMask, t.tm_wday)) continue;

    if (z.startHour == t.tm_hour && z.startMinute == t.tm_min && lastZoneTriggerMinute[i] != minuteStamp) {
      lastZoneTriggerMinute[i] = minuteStamp;
      startZone(i, z.durationSec);
      return;
    }
  }
}

void updateCycle() {
  if (!cycleRunning || activeZone < 0) return;
  unsigned long elapsed = (millis() - zoneStartedAtMs) / 1000;
  if (elapsed >= activeDurationSec) {
    if (cycleAll) advanceAllCycle();
    else stopCycle();
  }
}

void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.equalsIgnoreCase("status")) {
    Serial.printf("[STATUS] time=%s activeZone=%d pump=%d auto=%d\n", localTimeString().c_str(), activeZone + 1, pumpIsOn(), autoEnabled);
    return;
  }

  if (cmd.equalsIgnoreCase("start")) { startAllCycle(); return; }
  if (cmd.equalsIgnoreCase("stop")) { stopCycle(); return; }
  if (cmd.equalsIgnoreCase("auto on")) { autoEnabled = true; uiDirty = true; return; }
  if (cmd.equalsIgnoreCase("auto off")) { autoEnabled = false; uiDirty = true; return; }

  if (cmd.startsWith("zone ")) {
    int z = 1, sec = 300;
    if (sscanf(cmd.c_str(), "zone %d %d", &z, &sec) == 2 && z >= 1 && z <= ZONE_COUNT && sec > 0) {
      startZone((uint8_t)(z - 1), (uint16_t)sec);
      return;
    }
  }

  if (cmd.startsWith("tz ")) {
    int m = 180;
    if (sscanf(cmd.c_str(), "tz %d", &m) == 1) {
      gmtOffsetMin = m;
      applyTimezoneOffsetMinutes(gmtOffsetMin);
      return;
    }
  }

  if (cmd.startsWith("time set ")) {
    int y, mo, d, h, mi, s;
    if (sscanf(cmd.c_str(), "time set %d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s) == 6) {
      setManualDateTime(y, mo, d, h, mi, s);
      uiDirty = true;
      return;
    }
  }

  Serial.println("[SER] status | start | stop | auto on/off | zone <n> <sec> | tz <minutes> | time set YYYY-MM-DD HH:MM:SS");
}

void setup() {
  Serial.begin(115200);
  applyTimezoneOffsetMinutes(gmtOffsetMin);

  for (uint8_t i = 0; i < ZONE_COUNT; i++) {
    pinMode(ZONE_PINS[i], OUTPUT);
    setOut(ZONE_PINS[i], false);
    lastZoneTriggerMinute[i] = -1;
  }

  pinMode(PUMP_PIN, OUTPUT);
  pumpOff();

  pinMode(ENC_BTN_PIN, INPUT_PULLUP);
  pinMode(KEY0_PIN, INPUT_PULLUP);
  encoder.begin(ENC_A_PIN, ENC_B_PIN);
  encoder.setDetentThreshold(4);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);
  SPI.begin(TFT_SCLK_PIN, -1, TFT_MOSI_PIN, TFT_CS_PIN);
  tft.init(TFT_RAW_WIDTH, TFT_RAW_HEIGHT);
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(ST77XX_BLACK);

  connectWiFi();
  renderUi(true);
  uiForceFull = false;
}

void loop() {
  handleSerial();
  handleInput();
  handleWeb();
  updateSchedule();
  updateCycle();

  bool dueStatus = (millis() - lastStatusMs) >= 500;

  if (uiDirty) {
    renderUi(uiForceFull, false);
    uiForceFull = false;
    uiDirty = false;
    lastUiFrameMs = millis();
  } else if (dueStatus) {
    renderUi(false, true);
    lastStatusMs = millis();
  }

  delay(6);
}
