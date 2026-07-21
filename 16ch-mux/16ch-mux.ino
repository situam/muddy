#define PIN_MUX_S0 D0 
#define PIN_MUX_S1 D1 
#define PIN_MUX_S2 D2 
#define PIN_MUX_SIG A3

class Mux16 {
  private: 
    uint8_t _pinEnA;
    uint8_t _pinEnB;
    uint8_t _pinS0;
    uint8_t _pinS1;
    uint8_t _pinS2;

    void _selectMuxA() {
      digitalWrite(_pinEnA, 0);
      digitalWrite(_pinEnB, 1);
    }
    void _selectMuxB() {
      digitalWrite(_pinEnA, 1);
      digitalWrite(_pinEnB, 0);
    }

  public: 
    Mux16(uint8_t pinEnA, uint8_t pinEnB, uint8_t pinS0, uint8_t pinS1, uint8_t pinS2)
      : _pinEnA(pinEnA), _pinEnB(pinEnB), _pinS0(pinS0), _pinS1(pinS1), _pinS2(pinS2)
      {}

    void init() {
      pinMode(_pinEnA, OUTPUT);
      pinMode(_pinEnB, OUTPUT);
      pinMode(_pinS0, OUTPUT);
      pinMode(_pinS1, OUTPUT);
      pinMode(_pinS2, OUTPUT);

      disable();
    }
    
    void disable() {
      digitalWrite(_pinEnA, 1);
      digitalWrite(_pinEnB, 1);
    }

    // val expects range 0..15
    void selectInput(uint8_t val) {
      if (val < 8) {
        _selectMuxA();
      } else {
        _selectMuxB();
        val -= 8;
      }
      digitalWrite(_pinS0, val & 0b001);
      digitalWrite(_pinS1, val & 0b010);
      digitalWrite(_pinS2, val & 0b100);
    }
};

Mux16 muxes[] = {
  Mux16(D4, D5, PIN_MUX_S0, PIN_MUX_S1, PIN_MUX_S2),
  Mux16(D7, D8, PIN_MUX_S0, PIN_MUX_S1, PIN_MUX_S2)
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
      mux.selectInput(i);
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