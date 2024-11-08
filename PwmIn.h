#include "pico/stdlib.h"
#include "hardware/pio.h"

// class that sets up and reads PWM pulses: PwmIn. It has three functions:
// read_period (in seconds)
// read_pulsewidth (in seconds)
// read_dutycycle (between 0 and 1)
// read pulsewidth, period, and calculate the dutycycle

class PwmIn {
public:

  bool attach(uint pin);
  bool available(void);
  bool update(void);

  // read_period (in us)
  float get_period(void);

  // read_pulsewidth (in us)
  float get_pulsewidth(void);

  // read_dutycycle (between 0 and 1)
  float get_dutycycle(void);



private:
  PIO _pio;
  int _sm = -1;
  int _offset = -1;
  uint _pin;

  float _conv_cto = 0.0075188;
  // data about the PWM input measured in pio clock cycles
  uint32_t _pulsewidth;
  uint32_t _period;
};