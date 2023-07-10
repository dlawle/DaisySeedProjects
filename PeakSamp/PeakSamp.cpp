#include "daisy_seed.h"
#include "daisysp.h"
#include "peak.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

#define kBuffSize 48000 * 60 // 60 seconds at 48kHz

DaisySeed 	hw;
Peak		peak;
Looper		loop;
float DSY_SDRAM_BSS loop_buffer[kBuffSize];

enum AdcChannel {
   pot1 = 0,
   pot2,
   pot3,
   pot4,
   pot5,
   NUM_ADC_CHANNELS
};

void RecHandler(){
	bool t = peak.PeakTrig();
	float length = fmap(hw.adc.GetFloat(pot1), 100.f,1000.f);
	float rec_start;

	if(t){
		loop.TrigRecord();
		rec_start = System::GetNow();
	}

	float currLen = System::GetNow() - rec_start;
	if(currLen >= length){
		loop.TrigRecord();
	}
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	RecHandler();

	for (size_t i = 0; i < size; i++)
	{
		float ins = in[0][i] + in[1][i];
		float samps = loop.Process(ins);
		out[0][i] = samps;
		out[1][i] = in[0][i] + in[1][i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
	adc_config[pot1].InitSingle(A0);
	adc_config[pot2].InitSingle(A1);
	adc_config[pot3].InitSingle(A4);
	adc_config[pot4].InitSingle(A5);
	adc_config[pot5].InitSingle(A9);
	hw.adc.Init(adc_config,NUM_ADC_CHANNELS);
	hw.adc.Start();

	// init loop
	loop.Init(loop_buffer, kBuffSize);
	loop.SetMode(Looper::Mode::FRIPPERTRONICS);

	// init peak
	peak.Init();

	hw.StartAudio(AudioCallback);
	while(1) {
	}
}
