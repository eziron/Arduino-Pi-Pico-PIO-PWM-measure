#include "PwmIn.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "PIOProgram.h"
#include "hardware/clocks.h"
#include <math.h>

#include "PwmIn.pio.h"

// class that sets up and reads PWM pulses: PwmIn. It has three functions:
// read_period (in seconds)
// read_pulsewidth (in seconds)
// read_dutycycle (between 0 and 1)
// read pulsewidth, period, and calculate the dutycycle

void PwmIn::attach(uint pin) {
  _pin = pin;
  gpio_init(_pin);
  gpio_set_dir(_pin, GPIO_IN);

  _conv_cto = 1.0 / (float)(clock_get_hz(clk_sys) / 1000000);

  PIOProgram pgm(&PwmIn_program);
  pgm.prepare(&_pio, &_sm, &_offset);

  // configure the used pins
  pio_gpio_init(_pio, _pin);
  // make a sm config
  pio_sm_config c = PwmIn_program_get_default_config(_offset);
  // set the 'jmp' pin
  sm_config_set_jmp_pin(&c, _pin);
  // set the 'wait' pin (uses 'in' pins)
  sm_config_set_in_pins(&c, _pin);
  // set shift direction
  sm_config_set_in_shift(&c, false, false, 0);
  // init the pio sm with the config
  pio_sm_init(_pio, _sm, _offset, &c);
  // enable the sm
  pio_sm_set_enabled(_pio, _sm, true);
}

bool PwmIn::available(void) {
  uint fifo_count = pio_sm_get_rx_fifo_level(_pio, _sm);
  return fifo_count > 0 && !(fifo_count % 2);
}

bool PwmIn::update(void) {
  uint fifo_count = pio_sm_get_rx_fifo_level(_pio, _sm);

  if (fifo_count == 0 || fifo_count % 2) {
    pio_sm_clear_fifos(_pio, _sm);
    return false;
  }

  if (fifo_count > 2)
    while (pio_sm_get_rx_fifo_level(_pio, _sm) > 2) {
      uint32_t C = pio_sm_get(_pio, _sm);
    }

  if (pio_sm_get_rx_fifo_level(_pio, _sm) == 2) {
    // read pulse width from the FIFO
    _pulsewidth = pio_sm_get(_pio, _sm);
    // read period from the FIFO
    _period = pio_sm_get(_pio, _sm) + _pulsewidth;
    // the measurements are taken with 2 clock cycles per timer tick
    _pulsewidth = 2 * _pulsewidth;
    // calculate the period in clock cycles:
    _period = 2 * _period;
  } else {
    return false;
  }

  return true;
}

// read_period (in us)
float PwmIn::get_period(void) {
  return (_period * _conv_cto);
}

// read_pulsewidth (in us)
float PwmIn::get_pulsewidth(void) {
  return (_pulsewidth * _conv_cto);
}

// read_dutycycle (between 0 and 1)
float PwmIn::get_dutycycle(void) {
  return ((float)_pulsewidth / (float)_period);
}
