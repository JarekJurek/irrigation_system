
/*  Grzegorz Pawlak
    grzes.pawlak@gmail.com
    Irrigation System
    based on ESP32 and Blynk 2.0 (lib 1.2.0) platform
    Firmware version 1.1.0
*/

// 0.1.1 - full WIFI functionality, with general OFF button
// 0.1.2 - added rain detector and implemented into general OFF button 
// 0.1.3 - rebulding system  
// 0.1.4 - updated time and run times
// 0.1.5 - updated time and run times
// 1.0.0 - runtime adjustable
// 1.1.0 - time sync from NTP, safety checks for time validity

#define BLYNK_TEMPLATE_ID "TMPLcq7dN-Vf"
#define BLYNK_DEVICE_NAME "Irrigation"

#define BLYNK_FIRMWARE_VERSION        "1.1.0"

#define BLYNK_PRINT Serial

#define APP_DEBUG

#define USE_WROVER_BOARD

#include "BlynkEdgent.h"
#include <WiFi.h>
#include <time.h>

namespace {

constexpr const char* kTzWarsaw = "CET-1CEST,M3.5.0/2,M10.5.0/3";
constexpr time_t kDisableSeconds = 24 * 60 * 60;
constexpr time_t kTimeValidAfterEpoch = 1700000000;  // ~2023-11-14
constexpr uint32_t kMillisPerMinute = 60UL * 1000UL;

constexpr uint8_t kRainSensorPin = 23;
constexpr uint8_t kVirtualPinHour = V8;
constexpr uint8_t kVirtualPinMinute = V9;
constexpr uint8_t kVirtualPinOff = V0;

struct ValveState {
  uint8_t gpio_pin;
  uint8_t blynk_pin;
  int start_hour;
  int start_minute;
  bool is_open;
  uint32_t start_ms;
  int last_start_token;
};

ValveState g_valves[] = {
    {27, V1, 0, 0, false, 0, -1},
    {14, V2, 1, 0, false, 0, -1},
    {32, V3, 2, 0, false, 0, -1},
    {33, V4, 3, 0, false, 0, -1},
    {25, V5, 4, 0, false, 0, -1},
    {26, V6, 5, 0, false, 0, -1},
};

constexpr size_t kValveCount = sizeof(g_valves) / sizeof(g_valves[0]);
static_assert(kValveCount == 6, "Expected exactly 6 valves in g_valves");

// NTP/time config is performed once after Wi-Fi is connected.
bool g_ntp_configured = false;
bool g_run_disabled = false;
// Epoch seconds when the 24h disable should end. 0 means inactive/unknown.
time_t g_run_disabled_until = 0;
int g_run_time_minutes = 20;      // Modifiable by slider.

BlynkTimer g_timer;

bool IsTimeValid(time_t now) {
  // Protects against “time not yet synced” (often defaults to 1970).
  // Threshold is intentionally recent; adjust if you need older dates.
  return now > kTimeValidAfterEpoch;
}

bool GetLocalTimeNonBlocking(struct tm* out) {
  if (out == nullptr) {
    return false;
  }
  // 0ms timeout => return immediately (no blocking DNS/NTP waits).
  return getLocalTime(out, 0);
}

int MakeMinuteToken(const struct tm& ti) {
  // Uniquely identifies a minute within a year; prevents re-triggering within the same minute.
  return (ti.tm_yday * 1440) + (ti.tm_hour * 60) + ti.tm_min;
}

void EnsureNtpConfigured() {
  if (g_ntp_configured) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  // Sets timezone (with DST) and configures NTP servers.
  configTzTime(kTzWarsaw, "time.cloudflare.com", "pool.ntp.org", "time.google.com");
  g_ntp_configured = true;
}

void UpdateBlynkTime() {
  EnsureNtpConfigured();
  struct tm ti;
  if (GetLocalTimeNonBlocking(&ti) && IsTimeValid(time(nullptr))) {
    Blynk.virtualWrite(kVirtualPinHour, ti.tm_hour);
    Blynk.virtualWrite(kVirtualPinMinute, ti.tm_min);
  }
}

void DisableRuns(time_t now) {
  g_run_disabled = true;
  if (IsTimeValid(now)) {
    g_run_disabled_until = now + kDisableSeconds;
  } else {
    g_run_disabled_until = 0;
  }
}

void SetValveOpen(size_t index, bool open) {
  if (index >= kValveCount) {
    return;
  }
  ValveState& valve = g_valves[index];
  digitalWrite(valve.gpio_pin, !open);           // Active-low: LOW=open
  Blynk.virtualWrite(valve.blynk_pin, open);     // Keep app button in sync
  valve.is_open = open;
}

void SetValveManual(size_t index, bool open) {
  if (index >= kValveCount) {
    return;
  }
  ValveState& valve = g_valves[index];
  digitalWrite(valve.gpio_pin, !open);
  valve.is_open = open;
  if (open) {
    valve.start_ms = millis();
  }
}

void CheckStartScheduledRun(size_t index, const struct tm& ti, bool time_ok, int minute_token) {
  if (!time_ok || g_run_disabled) {
    return;
  }
  if (index >= kValveCount) {
    return;
  }

  ValveState& valve = g_valves[index];
  if (valve.is_open) {
    return;
  }

  if (valve.last_start_token == minute_token) {
    return;
  }

  if (valve.start_hour == ti.tm_hour && valve.start_minute == ti.tm_min) {
    valve.last_start_token = minute_token;
    valve.start_ms = millis();
    SetValveOpen(index, true);
  }
}

void CheckStopRun(size_t index) {
  if (index >= kValveCount) {
    return;
  }
  ValveState& valve = g_valves[index];
  if (!valve.is_open) {
    return;
  }

  if (g_run_disabled) {
    SetValveOpen(index, false);
    return;
  }

  const uint32_t elapsed_ms = millis() - valve.start_ms;
  const uint32_t run_ms = (uint32_t)g_run_time_minutes * kMillisPerMinute;
  if (elapsed_ms >= run_ms) {
    SetValveOpen(index, false);
  }
}

}  // namespace

BLYNK_WRITE(V0) {                 // Main turn-off button (24h disable)
  const bool is_on = (param.asInt() != 0);
  if (is_on) {                    // If the button is ON, disable scheduling for 24h.
    DisableRuns(time(nullptr));
  } else {
    g_run_disabled = false;
    g_run_disabled_until = 0;
  }
}

BLYNK_WRITE(V1) {
  SetValveManual(0, param.asInt() != 0);
}

BLYNK_WRITE(V2) {
  SetValveManual(1, param.asInt() != 0);
}

BLYNK_WRITE(V3) {
  SetValveManual(2, param.asInt() != 0);
}

BLYNK_WRITE(V4) {
  SetValveManual(3, param.asInt() != 0);
}

BLYNK_WRITE(V5) {
  SetValveManual(4, param.asInt() != 0);
}

BLYNK_WRITE(V6) {
  SetValveManual(5, param.asInt() != 0);
}

BLYNK_WRITE(V7) {
  g_run_time_minutes = param.asInt();
  if (g_run_time_minutes > 59 || g_run_time_minutes < 1) {
    g_run_time_minutes = 20;
  }
}

void setup() {
  for (size_t i = 0; i < kValveCount; ++i) {
    pinMode(g_valves[i].gpio_pin, OUTPUT);
    digitalWrite(g_valves[i].gpio_pin, 1);
  }

  pinMode(kRainSensorPin, INPUT);

  Serial.begin(9600);

  g_timer.setInterval(59000L, UpdateBlynkTime);  // publish time every 59 seconds
  BlynkEdgent.begin();
}

void loop() {
  BlynkEdgent.run();
  g_timer.run();

  EnsureNtpConfigured();

  struct tm ti;
  bool time_ok = GetLocalTimeNonBlocking(&ti);
  const time_t now = time(nullptr);
  if (!(time_ok && IsTimeValid(now))) {
    time_ok = false;
  }

  const bool rain_detected = !digitalRead(kRainSensorPin);  // sensor's 1 - no rain, 0 - rain

  if (rain_detected && !g_run_disabled) {
    Blynk.virtualWrite(kVirtualPinOff, 1);
    DisableRuns(time_ok ? now : 0);
  }

  // If disable was triggered before time was valid, start the 24h timer once we have a real clock.
  if (g_run_disabled && time_ok && g_run_disabled_until == 0) {
    g_run_disabled_until = now + kDisableSeconds;
  }

  if (g_run_disabled && time_ok && g_run_disabled_until != 0 && now >= g_run_disabled_until && !rain_detected) {
    Blynk.virtualWrite(kVirtualPinOff, 0);
    g_run_disabled = false;
    g_run_disabled_until = 0;
  }

  // Safety behavior: if time is not valid yet (e.g., no NTP sync), skip
  // automatic schedule starts. Manual control via Blynk still works.
  const int minute_token = time_ok ? MakeMinuteToken(ti) : -1;

  for (size_t i = 0; i < kValveCount; ++i) {
    CheckStartScheduledRun(i, ti, time_ok, minute_token);
    CheckStopRun(i);
  }
}
