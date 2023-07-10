// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License
#include "TouchScreen.h"
#include "per/gpio.h"
#include "daisy.h"
#include "daisy_seed.h"

// increase or decrease the touchscreen oversampling. This is a little different
// than you make think: 1 is no oversampling, whatever data we get is
// immediately returned 2 is double-sampling and we only return valid data if
// both points are the same 3+ uses insert sort to get the median value. We
// found 2 is precise yet not too slow so we suggest sticking with it!

#define NUMSAMPLES 2

using namespace daisy;
using namespace seed;
extern daisy::DaisySeed hw;

GPIO YP; // must be analog
GPIO XP; // must be analog
GPIO YM;
GPIO XM;
//const int num_adc_channels = 4;
//AdcChannelConfig touch_config[num_adc_channels];
//
//enum AdcChannel {
//   _ypa = 0,
//   _xpa,
//   _yma,
//   _xma,
//   NUM_ADC_CHANNELS
//};
//
//// Initialize the ADC peripheral with that configuration
//
//void TouchScreenInitTouchAdc(){
//  hw.adc.Init(touch_config, num_adc_channels);
//}

// weird formatting issue with old and new pins 
dsy_gpio_pin oldYP_PIN = seed::A2;
Pin _yp = Pin(static_cast<GPIOPort>(oldYP_PIN.port), oldYP_PIN.pin);
dsy_gpio_pin oldXP_PIN = seed::A3;
Pin _xp = Pin(static_cast<GPIOPort>(oldXP_PIN.port), oldXP_PIN.pin);
dsy_gpio_pin oldYM_PIN = seed::A4;
Pin _ym = Pin(static_cast<GPIOPort>(oldYM_PIN.port), oldYM_PIN.pin);
dsy_gpio_pin oldXM_PIN = seed::A5;
Pin _xm = Pin(static_cast<GPIOPort>(oldXM_PIN.port), oldXM_PIN.pin);

TSPoint::TSPoint(void) { x = y = z = 0; }
/**
 * @brief Construct a new TSPoint::TSPoint object
 *
 * @param x0 The point's X value
 * @param y0 The point's Y value
 * @param z0 The point's Z value
 */
TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}
/**
 * @brief Check if the current point is equivalent to another point
 *
 * @param p1 The other point being checked for equivalence
 * @return `true` : the two points are equivalent
 * `false`: the two points are **not** equivalent
 */
bool TSPoint::operator==(TSPoint p1) {
  return ((p1.x == x) && (p1.y == y) && (p1.z == z));
}
/**
 * @brief Check if the current point is **not** equivalent to another point
 *
 * @param p1 The other point being checked for equivalence

 * @return `true` :the two points are **not** equivalent
 * `false`: the two points are equivalent
 */
bool TSPoint::operator!=(TSPoint p1) {
  return ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

#if (NUMSAMPLES > 2)
static void insert_sort(int array[], uint8_t size) {
  uint8_t j;
  int save;

  for (int i = 1; i < size; i++) {
    save = array[i];
    for (j = i; j >= 1 && save < array[j - 1]; j--)
      array[j] = array[j - 1];
    array[j] = save;
  }
}
#endif
/**
 * @brief Measure the X, Y, and pressure and return a TSPoint with the
 * measurements
 *
 * @return TSPoint The measured X, Y, and Z/pressure values
 */
TSPoint TouchScreen::getPoint(void) {
  int x, y, z;
  int samples[NUMSAMPLES];
  uint8_t i, valid;

  valid = 1;

  // Moving Pin init to acceptable mode
  touch_config[_ypa].InitSingle(_yp);
  touch_config[_yma].InitSingle(_ym);
  XP.Init(_xp, GPIO::Mode::INPUT);
  XM.Init(_xm, GPIO::Mode::INPUT);
  //pinMode(_yp, INPUT);
  //pinMode(_ym, INPUT);
  //pinMode(_xp, OUTPUT);
  //pinMode(_xm, OUTPUT);

  //digitalWrite(_xp, HIGH);
  //digitalWrite(_xm, LOW);
  XP.Write(true);
  XM.Write(false);



  for (i = 0; i < NUMSAMPLES; i++) {
    samples[i] = hw.adc.GetFloat(_ypa);
  }

#if NUMSAMPLES > 2
  insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
  // Allow small amount of measurement noise, because capacitive
  // coupling to a TFT display's signals can induce some noise.
  if (samples[0] - samples[1] < -4 || samples[0] - samples[1] > 4) {
    valid = 0;
  } else {
    samples[1] = (samples[0] + samples[1]) >> 1; // average 2 samples
  }
#endif

  x = (1023 - samples[NUMSAMPLES / 2]);

  //pinMode(_xp, INPUT);
  //pinMode(_xm, INPUT);
  //pinMode(_yp, OUTPUT);
  //pinMode(_ym, OUTPUT);
  touch_config[_ypa].InitSingle(_yp);
  touch_config[_yma].InitSingle(_ym);
  XP.Init(_xp, GPIO::Mode::INPUT);
  XM.Init(_xm, GPIO::Mode::INPUT);

  YM.Write(false);
  YP.Write(true);

  for (i = 0; i < NUMSAMPLES; i++) {
    samples[i] = hw.adc.Get(_yma);
  }

#if NUMSAMPLES > 2
  insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
  // Allow small amount of measurement noise, because capacitive
  // coupling to a TFT display's signals can induce some noise.
  if (samples[0] - samples[1] < -4 || samples[0] - samples[1] > 4) {
    valid = 0;
  } else {
    samples[1] = (samples[0] + samples[1]) >> 1; // average 2 samples
  }
#endif

  y = (1023 - samples[NUMSAMPLES / 2]);

  // Set X+ to ground
  // Set Y- to VCC
  // Hi-Z X- and Y+
  XP.Init(_xp, GPIO::Mode::OUTPUT);
  YP.Init(_yp, GPIO::Mode::INPUT);
  //pinMode(_xp, OUTPUT);
  //pinMode(_yp, INPUT);

  XP.Write(false);
  YM.Write(true);

  int z1 = hw.adc.Get(_xma);
  int z2 = hw.adc.Get(_ypa);

  if (_rxplate != 0) {
    // now read the x
    float rtouch;
    rtouch = z2;
    rtouch /= z1;
    rtouch -= 1;
    rtouch *= x;
    rtouch *= _rxplate;
    rtouch /= 1024;

    z = rtouch;
  } else {
    z = (1023 - (z2 - z1));
  }

  if (!valid) {
    z = 0;
  }

  return TSPoint(x, y, z);
}

TouchScreen::TouchScreen(uint16_t rxplate = 0) {
  _rxplate = rxplate;
  pressureThreshhold = 10;
}
/**
 * @brief Read the touch event's X value
 *
 * @return int the X measurement
 */
int TouchScreen::readTouchX(void) {
  YP.Init(_yp, GPIO::Mode::INPUT);
  YM.Init(_ym, GPIO::Mode::INPUT);
  YP.Write(false);
  YM.Write(false);
  //pinMode(_yp, INPUT);
  //pinMode(_ym, INPUT);
  //digitalWrite(_yp, LOW);
  //digitalWrite(_ym, LOW);

  XP.Init(_xp, GPIO::Mode::OUTPUT);
  XP.Write(true);
  XM.Init(_xm, GPIO::Mode::OUTPUT);
  XM.Write(true);
  //pinMode(_xp, OUTPUT);
  //digitalWrite(_xp, HIGH);
  //pinMode(_xm, OUTPUT);
  //digitalWrite(_xm, LOW);

  return (1023 - hw.adc.Get(_ypa));
}
/**
 * @brief Read the touch event's Y value
 *
 * @return int the Y measurement
 */
int TouchScreen::readTouchY(void) {
  XP.Init(_xp, GPIO::Mode::INPUT);
  XM.Init(_xm, GPIO::Mode::INPUT);
  XP.Write(false);
  XM.Write(false);

  //pinMode(_xp, INPUT);
  //pinMode(_xm, INPUT);
  //digitalWrite(_xp, LOW);
  //digitalWrite(_xm, LOW);

  YP.Init(_yp, GPIO::Mode::OUTPUT);
  YP.Write(true);
  YM.Init(_ym, GPIO::Mode::OUTPUT);
  YM.Write(false);

  //pinMode(_yp, OUTPUT);
  //digitalWrite(_yp, HIGH);
  //pinMode(_ym, OUTPUT);
  //digitalWrite(_ym, LOW);

  return (1023 - hw.adc.Get(_xma));
}
/**
 * @brief Read the touch event's Z/pressure value
 *
 * @return int the Z measurement
 */
uint16_t TouchScreen::pressure(void) {
  // Set X+ to ground
  XP.Init(_xp, GPIO::Mode::OUTPUT);
  XP.Write(false);
  //pinMode(_xp, OUTPUT);
  //digitalWrite(_xp, LOW);

  // Set Y- to VCC
  YM.Init(_ym, GPIO::Mode::OUTPUT);
  YM.Write(true);
  //pinMode(_ym, OUTPUT);
  //digitalWrite(_ym, HIGH);

  // Hi-Z X- and Y+
  XM.Write(false);
  XM.Init(_xm, GPIO::Mode::INPUT);
  YP.Write(false);
  YP.Init(_yp, GPIO::Mode::INPUT);
  //digitalWrite(_xm, LOW);
  //pinMode(_xm, INPUT);
  //digitalWrite(_yp, LOW);
  //pinMode(_yp, INPUT);

  int z1 = hw.adc.Get(_xma);
  int z2 = hw.adc.Get(_ypa);

  if (_rxplate != 0) {
    // now read the x
    float rtouch;
    rtouch = z2;
    rtouch /= z1;
    rtouch -= 1;
    rtouch *= readTouchX();
    rtouch *= _rxplate;
    rtouch /= 1024;

    return rtouch;
  } else {
    return (1023 - (z2 - z1));
  }
}