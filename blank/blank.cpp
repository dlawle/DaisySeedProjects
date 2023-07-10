#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed hw;

void logInit();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	logInit();
	// Create an ADC Channel Config object
	AdcChannelConfig adc_config;
	
	// Set up the ADC config with a connection to pin A0
	adc_config.InitSingle(A7);
	
	// Initialize the ADC peripheral with that configuration
	hw.adc.Init(&adc_config, 1);
	
	// Start the ADC
	hw.adc.Start();
	hw.StartAudio(AudioCallback);
	while(1) {
		// Read the first ADC that's configured. In our case, this is the only input.
		int value = hw.adc.Get(0);
	
		// In order to know that everything's working let's print that to a serial console:
		hw.PrintLine("ADC Value: %d", value);
	
		// Wait half a second (500 milliseconds)
		System::Delay(500);
	}
}

void logInit(){
	hw.StartLog();
	hw.PrintLine("Hello World!");
}