#pragma once
#ifndef TOUCHEVENT_H
#define TOUCHEVENT_H 

#include "dev/oled_ssd130x.h"
#include "daisy.h"
#include "daisy_seed.h"

extern daisy::DaisySeed hw;

using Mpr121a = daisy::Mpr121<daisy::Mpr121I2CTransport>;
using MyOledDisplay = daisy::OledDisplay<daisy::SSD130x4WireSpi128x64Driver>;

Mpr121a			touch1;
MyOledDisplay	display;

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
char strbuff[128];
char strbuff2[128];
bool gate = false;

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif


void InitHardware(){
    // touch stuff
	Mpr121a::Config	touch_cfg;
	touch1.Init(touch_cfg);

    /** Configure the Display */
    MyOledDisplay::Config disp_cfg;
    disp_cfg.driver_config.transport_config.pin_config.dc    = hw.GetPin(9);
    disp_cfg.driver_config.transport_config.pin_config.reset = hw.GetPin(30);
    /** And Initialize */
    display.Init(disp_cfg);
}

void touchEvent(){
    currtouched = touch1.Touched();
    for (uint8_t i=0; i<12; i++) {
        // it if *is* touched and *wasnt* touched before, alert!
        if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
            hw.SetLed(true);
            sprintf(strbuff,"%u",i);
            gate = true;
        }
        // if it *was* touched and now *isnt*, alert!
        if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
            hw.SetLed(false);
            sprintf(strbuff, " ");
            gate = false;
        }
    }

    display.Fill(true);
    display.SetCursor(0, 0);
    display.WriteString(strbuff, Font_11x18, false);
    display.Update();

    // reset our state
    lasttouched = currtouched;
}

float TouchedNote(){
    return currtouched;
}

bool ReturnGate(){
    return gate;
}

#endif