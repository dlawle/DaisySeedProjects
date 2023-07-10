#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed				hw;
Oscillator				osc_a,osc_b,osc_c; 

enum AdcChannel {
   pot1 = 0,
   pot2,
   pot3,
   pot4,
   pot5,
   NUM_ADC_CHANNELS
};

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float coarse_knob = hw.adc.GetFloat(pot1); 
    float freq = fmap(coarse_knob, 0.f, 1000.f);

	float detune_knob = hw.adc.GetFloat(pot2); 

	osc_a.SetFreq(freq); 
	osc_b.SetFreq(freq + (.05 * freq * detune_knob)); 
	osc_c.SetFreq(freq - (.05 * freq * detune_knob)); 

	for (size_t i = 0; i < size; i++)
	{
		float output = osc_a.Process() + osc_b.Process() + osc_c.Process(); 

		out[0][i] = output;
		out[1][i] = output;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float samplerate = hw.AudioSampleRate();

	AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
	adc_config[pot1].InitSingle(A0);
	adc_config[pot2].InitSingle(A1);
	adc_config[pot3].InitSingle(A4);
	adc_config[pot4].InitSingle(A5);
	adc_config[pot5].InitSingle(A9);
	hw.adc.Init(adc_config,NUM_ADC_CHANNELS);
	hw.adc.Start();

	// set up oscillators 
	osc_a.Init(samplerate);
	osc_a.SetWaveform(osc_a.WAVE_POLYBLEP_SAW); 

	osc_b.Init(samplerate);
	osc_b.SetWaveform(osc_a.WAVE_POLYBLEP_SAW); 
	
	osc_c.Init(samplerate);
	osc_c.SetWaveform(osc_a.WAVE_POLYBLEP_SAW); 

	hw.StartAudio(AudioCallback);
	while(1) {}
}
