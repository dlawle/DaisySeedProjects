#pragma once
#include <stdint.h>

int noteArray[9] = {
51, //Eb
58, //Bb
53, //F
48, //C
55, //G
50, //D
57, //A
52, //E 
59, //B
};

char scale[3] = {'M','m','7'};

int chordNote[3][12] = {
//major = +4, +3, +5
{ 0, 4, 7, 12, 16, 19, 24, 28, 31, 36, 40, 43 },  // maj
//minor = +3 +4 +5
{ 0, 3, 7, 12, 15, 19, 24, 27, 31, 36, 39, 43}, // min
//7th = +4, +3, +3, +2
{ 0, 4, 7, 10, 12, 16, 19, 22, 24, 28, 31, 34 }  // 7th
};

