#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed		hw;
Metro			tick;
 		modal;

enum AdcChannel {
   pot1 = 0,
   pot2,
   pot3,
   pot4,
   pot5,
   NUM_ADC_CHANNELS
};

// minor pentatonic scale 
float kArpeggio[5] = {440.f, 523.25f, 587.33f, 659.25f, 783.99f};
uint8_t     arp_idx;
bool sus = false; 

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig_out;

	float structure = fmap(hw.adc.GetFloat(pot1), 0.f, 1.f);

	float speed = fmap(hw.adc.GetFloat(pot4), 1.f, 40.f);
	tick.SetFreq(speed);

	float accent_knob = hw.adc.GetFloat(pot3);
	float accent = fmap(accent_knob, 0.f, 1.f);
	
	for (size_t i = 0; i < size; i++)
	{
		bool t = tick.Process();
		if(t){
			modal.SetSustain(true);
			modal.SetFreq(kArpeggio[rand() % 5]);
		}

		modal.SetStructure(structure);
        modal.SetBrightness(.1f + structure * .1f);
		modal.SetAccent(accent);

		sig_out = modal.Process(t);

		out[0][i] = sig_out;
		out[1][i] = sig_out;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
	adc_config[pot1].InitSingle(A0);
	adc_config[pot2].InitSingle(A1);
	adc_config[pot3].InitSingle(A4);
	adc_config[pot4].InitSingle(A5);
	adc_config[pot5].InitSingle(A9);
	hw.adc.Init(adc_config,NUM_ADC_CHANNELS);
	hw.adc.Start();

	// modal init
	modal.Init(sample_rate);
	modal.SetDamping(.5);

	// metro init 
	tick.Init(5.f, sample_rate);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
