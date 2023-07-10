// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#ifndef _ADAFRUIT_TOUCHSCREEN_H_
#define _ADAFRUIT_TOUCHSCREEN_H_
#include <stdint.h>
#include "per/gpio.h"
#include "daisy.h"
#include "daisy_seed.h"

typedef volatile uint32_t RwReg;
const int num_adc_channels = 4;
using namespace daisy;
using namespace seed;
extern daisy::DaisySeed hw;

/** Object that encapsulates the X,Y, and Z/pressure measurements for a touch
 * event. */
class TSPoint {
public:
  TSPoint(void);
  TSPoint(int16_t x, int16_t y, int16_t z);

  bool operator==(TSPoint);
  bool operator!=(TSPoint);

  int16_t x, ///< state variable for the x value
      y,     ///< state variable for the y value
      z;     ///< state variable for the z value
};
/** Object that controls and keeps state for a touch screen. */

class TouchScreen {
public:
AdcChannelConfig touch_config[num_adc_channels];

enum AdcChannel {
   _ypa = 0,
   _xpa,
   _yma,
   _xma,
   NUM_ADC_CHANNELS
};

// Initialize the ADC peripheral with that configuration

void TsInit(){
  hw.adc.Init(touch_config, num_adc_channels);
}

  /**
   * @brief Construct a new Touch Screen object
   *
   * @param xp X+ pin. Must be an analog pin
   * @param yp Y+ pin. Must be an analog pin
   * @param xm X- pin. Can be a digital pin
   * @param ym Y- pin. Can be a digital pin
   * @param rx The resistance in ohms between X+ and X- to calibrate pressure
   * sensing
   */
  TouchScreen(uint16_t rx);

  void SetRx(uint16_t set){
    _rxplate = set;
    pressureThreshhold = 10;
  }
  /**
   * @brief **NOT IMPLEMENTED** Test if the screen has been touched
   *
   * @return true : touch detected false: no touch detected
   */
  bool isTouching(void);
  uint16_t pressure(void);
  int readTouchY();
  int readTouchX();
  TSPoint getPoint();
  int16_t pressureThreshhold; ///< Pressure threshold for `isTouching`

private:
  //uint8_t _yp, _ym, _xm, _xp;
  uint16_t _rxplate;

  };


#endif
