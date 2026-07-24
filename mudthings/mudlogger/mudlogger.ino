#include <Wire.h>
#include <Adafruit_INA219.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include <FS.h>
#include <LittleFS.h>

#include <sys/time.h>

#include "Muximeter.h"

// Defines SSID, PWD, LOG_URL
#include "secrets.h"

using std::array;

#define SDA_PIN 8
#define SCL_PIN 9 

#define PIN_MUX_EN 6
#define PIN_MUX_S1 13 
#define PIN_MUX_S2 12 
#define PIN_MUX_S3 11 

#define K_SAMPLE_COUNT 32
#define K_DELAY_SETTLE_MS 10

#define N_READINGS 32 * 8

#define SLEEP_INTERVAL_SECONDS 120
// #define DEBUG_NOSLEEP

// If defined, print local logs to serial
// #define PRINT_LOGS

Adafruit_INA219 ina219;
Muximeter muximeters[] = {
  Muximeter(1, 2, 3, 4, 5),
  Muximeter(7, 10, 3, 4, 5),
};
constexpr uint8_t nMuxis = sizeof(muximeters)/sizeof(muximeters[0]);

namespace mux {
  void enable() {
    digitalWrite(PIN_MUX_EN, 0);
  }
  void disable() {
    digitalWrite(PIN_MUX_EN, 1);
  }
  void init() {
    pinMode(PIN_MUX_S1, OUTPUT);
    pinMode(PIN_MUX_S2, OUTPUT);
    pinMode(PIN_MUX_S3, OUTPUT);
    pinMode(PIN_MUX_EN, OUTPUT);
    disable();
  }
  void selectChannel(uint8_t val) {
    digitalWrite(PIN_MUX_S1, val & 0b001);
    digitalWrite(PIN_MUX_S2, val & 0b010);
    digitalWrite(PIN_MUX_S3, val & 0b100);
  }
}

struct Log {
  uint32_t time;
  float readings[N_READINGS];
};

struct LogData {
  uint32_t time;
  uint8_t readings[N_READINGS];
};

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(100); }
  delay(100);

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Woke up by timer.");
  } else {
    Serial.println("Reset.");
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;
    settimeofday(&time, NULL);
  }

  Wire.begin(SDA_PIN, SCL_PIN);

  mux::init();
  for (auto &muxi : muximeters) {
    muxi.init();
  }

  // initialize ina219 with default measurement range of 32V, 2A
  if (!ina219.begin()) {
    Serial.println("INA219 not found");
    while (1);
  }
  // ina219.setCalibration_32V_2A();    // set measurement range to 32V, 2A  (do not exceed 26V!)
  // ina219.setCalibration_32V_1A();    // set measurement range to 32V, 1A  (do not exceed 26V!)
  ina219.setCalibration_16V_400mA(); // set measurement range to 16V, 400mA
 
  #ifdef PRINT_LOGS
  Serial.println("Printing local logs:");
  print_local_logs();
  #else
  run();
  #endif
}

void loop() { }

Log read_sensors() {
  Log log;
  log.time = time(NULL);
  for (int8_t r=7; r>=0; r--) {
    mux::enable();
    mux::selectChannel(r);
    delay(K_DELAY_SETTLE_MS); // let settle
    for (uint8_t m=0; m<nMuxis; m++) {
      for (uint8_t c=0; c<16; c++) {
        muximeters[m].selectChannel(c);
        delay(K_DELAY_SETTLE_MS);

        // read data from ina219
        float busvoltage = 0;
        for (uint8_t i=0;i<K_SAMPLE_COUNT;i++) {
          busvoltage += ina219.getBusVoltage_V();
        }
        if (busvoltage!=0) busvoltage/=K_SAMPLE_COUNT;

        uint8_t i = (r << 5) + (m << 4) + c;
        log.readings[i] = busvoltage;
      }

      muximeters[m].disable();
      delay(K_DELAY_SETTLE_MS);
    }
  }
  return log;
}

void format_log(String* line, Log* log) {
  *line += "T:"; *line += (uint32_t) log->time; *line += "\t";
  for (int i = 0; i < N_READINGS; i++) {
    int r = i >> 5;
    int m = (i >> 4) & 1;
    int c = i & 31;
    *line += "M"; *line += m;
    *line += ":C"; *line += c;
    *line += ":R"; *line += r;
    *line += ":";
    *line += String(log->readings[i], 3);
    *line += "\t";
  }
}

void log_local(Log* log) {
  LogData data;
  data.time = log->time;
  for (int i = 0; i < N_READINGS; i++) {
    data.readings[i] = (uint8_t) min(max(log->readings[i] * 256.0, 0.0), 255.9);
  }

  if (!LittleFS.begin(true)) {
    Serial.println("FS init failed :(");
    return;
  }
  File file = LittleFS.open("/log", "a");
  if (!file) {
    Serial.println("FS open file failed :(");
    return;
  }
  file.write((const uint8_t*) &data, sizeof(data));
  file.close();
  Serial.println("Wrote local log.");
}

void print_local_logs() {
  Serial.println("Send me something over serial to trigger printing");
  while(!Serial.available()) {
    delay(100);
  }

  if (!LittleFS.begin(true)) {
    Serial.println("FS init failed :(");
    return;
  }
  File file = LittleFS.open("/log", "r");
  if (!file) {
    Serial.println("FS open file failed :(");
    return;
  }
  int size = file.available();
  int n_logs = size / sizeof(LogData);
  LogData data;
  while (file.available() >= sizeof(LogData)) {
    if (file.read((uint8_t*) &data, sizeof(LogData)) != sizeof(LogData)) {
      Serial.println("FS file read failed :(");
      return;
    }
    Serial.write((const char*) &data, sizeof(data));
    // Log log;
    // log.time = data.time;
    // for (int i = 0; i < N_READINGS; i++) {
    //   log.readings[i] = (float) data.readings[i] / 256.0;
    // }
    // String line = "";
    // format_log(&line, &log);
    // Serial.println(line);
  }

  Serial.println("Done.");
}

void log_wifi(Log* log) {
  String line = "";
  format_log(&line, log);

  WiFi.begin(SSID, PWD);
  for (uint8_t i=0; i<20;i++) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      continue;
    }
    Serial.print("connected!");

    HTTPClient http;
    String url = LOG_URL;
    http.begin(url.c_str());
    http.POST(line);
    http.end();
    break;
  }

  Serial.println("Posted log:");
  Serial.println(line);
}

void run() {
  auto log = read_sensors();

  log_local(&log);
  log_wifi(&log);

  #ifndef DEBUG_NOSLEEP
  esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL_SECONDS * 1000);
  esp_deep_sleep_start();
  #else
  delay(1000);
  run();
  #endif
}

