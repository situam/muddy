class Muximeter {
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
    Muximeter(uint8_t pinEnA, uint8_t pinEnB, uint8_t pinS0, uint8_t pinS1, uint8_t pinS2)
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
    void selectChannel(uint8_t val) {
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