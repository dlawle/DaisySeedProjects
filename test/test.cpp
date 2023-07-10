#include "daisy_seed.h"
 
using namespace daisy;
using namespace seed;
DaisySeed hw;

const char rows = 3; // set display to four rows
const char cols = 9; // set display to three columns

const char keys[rows][cols] = {
    {'1','2','3','4','5','6','7','8','9'},
    {'0','a','b','c','d','e','f','g','h'},
    {'i','j','k','l','m','n','o','p','q'}
               
};

daisy::Pin rowPins[rows] = {D3 , D2, D1};
daisy::Pin colPins[cols] = {D14, D13, D10, D9, D8, D7, D6, D5, D4};

void InitPins(){
    for(int r = 0; r < rows; r++){
        GPIO rows[r];
        rows[r].Init(rowPins[r], GPIO::Mode::INPUT);
        //rows[r].Write(true);
    }
   
   for(int c = 0; c < cols; c++){
        GPIO cols[c];
        cols[c].Init(colPins[c], GPIO::Mode::OUTPUT);
   }
}

char getKey(){
    char k = 0;
    for(int c = 0; c < cols; c++){
        GPIO cols[c];
        cols[c].Write(true);
        for(int r = 0; r < rows; r++){
            GPIO rows[r];
            if(rows[r].Read() == false){
                while(rows[r].Read() == false){
                    k = keys[r][c];
                }
            }
        }
    }
    return k;
}


int main(void) {
 
  // Initialize the Daisy Seed Hardware
  hw.Init();
 
  // Enable Logging, and set up the USB connection.
  hw.StartLog();
 
  // And Print Hello World!
  hw.PrintLine("Hello World!");

  for(int r = 0; r < rows; r++){
      GPIO rows[r];
      rows[r].Init(rowPins[r], GPIO::Mode::INPUT);
  }
  
  for(int c = 0; c < cols; c++){
      GPIO cols[c];
      cols[c].Init(colPins[c], GPIO::Mode::OUTPUT);
  }

while(1) {
  char key = getKey();
  FixedCapStr<16> str("Value: ");
  str.Append(key);
  hw.PrintLine(str);

  System::Delay(1000); // Wait 1 second between printing
}
}