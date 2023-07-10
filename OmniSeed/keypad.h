#pragma once

#include "daisy_seed.h"
#include <utility>

using namespace daisy;
using namespace seed;
extern daisy::DaisySeed hw;

class KeyboardMatrix {
public:
    KeyboardMatrix(){};
    void init();

    std::pair<int, int> iReadKey();
private:
    const unsigned char NUM_ROWS = 3;
    const unsigned char NUM_COLS = 9;
    uint8_t GPIOcolPins[9] = { 4, 5, 6, 7, 8, 9, 10, 13, 14 };
    uint8_t GPIOrowPins[3] = { 1, 2, 3 };
    char values[3][9] = {
        { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i' },
        { 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r' },
        { 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1' }
    };
    int notes[3][9] = {
        {  1, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 10,11,12,13,14,15,16,17,18 },
        { 19,20,21,22,23,24,25,26,27 }     
    };
    dsy_gpio colPins[9];
    dsy_gpio rowPins[3];
};

