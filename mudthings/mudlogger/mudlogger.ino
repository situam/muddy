#include <Wire.h>
#include <Adafruit_INA219.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include "Muximeter.h"

// Defines SSID, PWD, LOG_URL
#include "secrets.h"

#define SDA_PIN 8
#define SCL_PIN 9 

#define PIN_MUX_EN 6
#define PIN_MUX_S1 13 
#define PIN_MUX_S2 12 
#define PIN_MUX_S3 11 

#define K_SAMPLE_COUNT 32
#define K_DELAY_SETTLE_MS 10

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

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);

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

  String line = "";
  for (int8_t r=8; r>=0; r--) {
    if (r==8) {
      mux::disable();
    } else {
      mux::enable();
      mux::selectChannel(r);
    }
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

        line += "M"; line += m;
        line += ":C"; line += c;
        line += ":R"; line += r;
        line += ":";
        line += String(busvoltage, 3);
        line += "\t";
      }

      muximeters[m].disable();
      delay(K_DELAY_SETTLE_MS);
    }
  }
  mux::disable();

  Serial.println(line);

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

  esp_sleep_enable_timer_wakeup(5000000);
  esp_deep_sleep_start();
}

void loop() { }
