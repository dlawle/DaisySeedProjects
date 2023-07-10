#pragma once
#ifndef TOUCHEVENT_H
#define TOUCHEVENT_H 

#include "daisy.h"
#include "daisy_seed.h"

extern daisy::DaisySeed hw;

using Mpr121a = daisy::Mpr121<daisy::Mpr121I2CTransport>;
Mpr121a			touch1;

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int pad;

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

void Init121(){
    // touch stuff
	Mpr121a::Config	touch_cfg;
	touch1.Init(touch_cfg);
}

bool hasTriggered = false;
void touchEvent(){
    currtouched = touch1.Touched();
    for (uint8_t i=0; i<12; i++) {
        // it if *is* touched and *wasnt* touched before, alert!
        if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
            hw.SetLed(true);
            //hw.PrintLine("touched: %d", i);
            pad = i;
            hasTriggered = false; // reset the trigger flag when a new touch is detected
        }
        // if it *was* touched and now *isnt*, alert!
        if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
            hw.SetLed(false);
            pad = 99;
            hasTriggered = false; // reset the trigger flag when the touch is released
        }
    }
    // reset our state
    lasttouched = currtouched;
}

int TouchedNote(){
    return pad;
}

#endif