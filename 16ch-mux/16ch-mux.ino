#include "Muximeter.h"

#define PIN_MUX_S0 D0 
#define PIN_MUX_S1 D1 
#define PIN_MUX_S2 D2 
#define PIN_MUX_SIG A3

Muximeter muxes[] = {
  Muximeter(D4, D5, PIN_MUX_S0, PIN_MUX_S1, PIN_MUX_S2),
  Muximeter(D7, D8, PIN_MUX_S0, PIN_MUX_S1, PIN_MUX_S2)
};

void setup() {
  Serial.begin(115200);

  pinMode(PIN_MUX_SIG, INPUT);

  for (auto &mux : muxes) {
    mux.init();
  }
}

void loop() {
  for (auto &mux : muxes) {
    for (uint8_t i=0; i<16; i++) {
      mux.selectChannel(i);
      delay(2); // let settle

      int val = analogRead(PIN_MUX_SIG);
      Serial.print(val);
      Serial.print(",");
    }
    
    mux.disable(); // break before make
    delay(2);
  }
  Serial.println("");
}