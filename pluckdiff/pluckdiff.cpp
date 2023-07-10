#include "daisy_seed.h"
#include "daisysp.h"
#include "diffuser.h"
#include "fx_engine.h"

using namespace daisy;
using namespace daisysp;
using namespace seed; 
using namespace impl; 

DaisySeed 						hw;
Pluck							pluck;
Metro							tick; 
Diffuser						diff;
FxEngine<1024,FORMAT_12_BIT> 	fx;

enum AdcChannel {
   pot1 = 0,
   pot2,
   pot3,
   pot4,
   pot5,
   NUM_ADC_CHANNELS
};

// MIDI note numbers for a major triad
const float kArpeggio[3] = {68.0f, 72.0f, 75.0f};
uint8_t     arp_idx;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float trig, sig_out;
	float decay_knob = hw.adc.GetFloat(pot1);
	float decay = fmap(decay_knob, 0.1f, .99f);

	float time_knob = hw.adc.GetFloat(pot2);
	float time = fmap(time_knob, 0.f, 1.f); 

	float amount_knob = hw.adc.GetFloat(pot3);
	float amount = fmap(amount_knob, 0.f,1.f); 

	for (size_t i = 0; i < size; i++)
	{
		trig = 0.0f;
        if(tick.Process())
        {
            float freq = mtof(kArpeggio[arp_idx]); // convert midi nn to frequency.
            arp_idx = (arp_idx + 1)
                      % 3; // advance the kArpeggio, wrapping at the end.
			pluck.SetDecay(decay);
            pluck.SetFreq(freq);
            trig = 1.0f;
        }
        sig_out = pluck.Process(trig);

		diff.SetAmount(amount);
		diff.SetTime(time);

		out[0][i] = diff.Process(sig_out);
		out[1][i] = diff.Process(sig_out);
	}
}

int main(void)
{
	float init_buff[256]; // buffer for Pluck impulse
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

    // Set up Metro to pulse every second
    tick.Init(1.0f, sample_rate);
    // Set up Pluck algo
    pluck.Init(sample_rate, init_buff, 256, PLUCK_MODE_RECURSIVE);
	pluck.SetFreq(440);
    pluck.SetDecay(0.95f);
    pluck.SetDamp(0.9f);
    pluck.SetAmp(0.3f);

	diff.Init();

    arp_idx = 0;

	hw.StartAudio(AudioCallback); 
	while(1) {}
}
