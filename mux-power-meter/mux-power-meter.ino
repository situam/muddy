#include <Wire.h>
#include <Adafruit_INA219.h>

#define SDA_PIN 8
#define SCL_PIN 9 

#define PIN_MUX_S1 13 
#define PIN_MUX_S2 12 
#define PIN_MUX_S3 11 

#define K_SAMPLE_COUNT 10

Adafruit_INA219 ina219;

float busvoltage = 0;
float current_mA = 0;
float power_mW = 0;

namespace mux {
  void setup() {
    pinMode(PIN_MUX_S1, OUTPUT);
    pinMode(PIN_MUX_S2, OUTPUT);
    pinMode(PIN_MUX_S3, OUTPUT);
  }
  void selectInput(uint8_t val) {
    digitalWrite(PIN_MUX_S1, val & 0b001);
    digitalWrite(PIN_MUX_S2, val & 0b010);
    digitalWrite(PIN_MUX_S3, val & 0b100);
  }
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);

  mux::setup();

  // initialize ina219 with default measurement range of 32V, 2A
  if (!ina219.begin()) {
    Serial.println("INA219 not found");
    while (1);
  }

  // ina219.setCalibration_32V_2A();    // set measurement range to 32V, 2A  (do not exceed 26V!)
  // ina219.setCalibration_32V_1A();    // set measurement range to 32V, 1A  (do not exceed 26V!)
  // ina219.setCalibration_16V_400mA(); // set measurement range to 16V, 400mA
}

void loop() {
  for (uint8_t input=0; input<8; input++) {
    mux::selectInput(input);
    delay(2); // let settle

    // read data from ina219
    busvoltage = 0;
    current_mA = 0;
    power_mW = 0;
    for (uint8_t i=0; i < K_SAMPLE_COUNT; i++) {
      busvoltage += ina219.getBusVoltage_V();
      current_mA += ina219.getCurrent_mA();
      power_mW += ina219.getPower_mW();
      delay(2);
    }
    if (busvoltage!=0) busvoltage /= K_SAMPLE_COUNT;
    if (current_mA!=0) current_mA /= K_SAMPLE_COUNT;
    if (power_mW!=0) power_mW /= K_SAMPLE_COUNT;

    Serial.print(busvoltage);
    Serial.print(",");
  }
  Serial.println("");
}