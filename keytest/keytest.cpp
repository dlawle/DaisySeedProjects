#include "daisy_seed.h"
#include "daisysp.h"
#include "Keypad.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed hw;

const unsigned char numRows = 3;
const unsigned char numCols = 9;
const char* rowPins[] = {"D3" , "D2", "D1"};  // GPIO Input - Pull_Up
const char* colPins[] = {"D14", "D13", "D10", "D9", "D8", "D7", "D6", "D5", "D4"};  // GPIO Output

char keypadArray[numRows][numCols] =
{
    {'1','2','3','4','5','6','7','8','9'},
    {'0','a','b','c','d','e','f','g','h'},
    {'i','j','k','l','m','n','o','p','q'}
};

int main(void)
{
	hw.Init();
	hw.StartLog();
	hw.PrintLine("Starting Up....");

  Keypad keypad((char*)keypadArray, rowPins, colPins, numRows, numCols);

  while (1)
  {
  	if(keypad.getKey() == '1')
  	{
		// You may need to change GPIO_PIN_SET to
		//  GPIO_PIN_RESET depending on your settings
		hw.PrintLine("Key Pressed!");
  	}
	  if(keypad.getKey() == '0')
  	{
		hw.PrintLine("No Key Pressed!");
  	}
  }
}
