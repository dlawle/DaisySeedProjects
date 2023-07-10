#include "keypad.h"

void KeyboardMatrix::init() {
    for (int i = 0; i < NUM_ROWS; i++) {
        rowPins[i].pin = hw.GetPin(GPIOrowPins[i]);
        rowPins[i].mode = DSY_GPIO_MODE_INPUT;
        rowPins[i].pull = DSY_GPIO_PULLDOWN;
        dsy_gpio_init(&rowPins[i]);
    }
    for (int i = 0; i < NUM_COLS; i++) {
        colPins[i].pin = hw.GetPin(GPIOcolPins[i]);
        colPins[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
        colPins[i].pull = DSY_GPIO_PULLDOWN;
        dsy_gpio_init(&colPins[i]);
    }
}

std::pair<int, int> KeyboardMatrix::iReadKey() {
    int column = 99;
    int row = 99;
    for (int c = 0; c < NUM_COLS; c++) {
        dsy_gpio_write(&colPins[c], 1);
        for (int r = 0; r < NUM_ROWS; r++) {
            if (dsy_gpio_read(&rowPins[r]) == 1) {
                column = c;
                row = r;
            }
        }
        dsy_gpio_write(&colPins[c], 0);
    }
    return std::make_pair(column, row);
}