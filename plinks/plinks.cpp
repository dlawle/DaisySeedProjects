#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed		hw;
PolyPluck<32>	pp;
Metro			tick;
ReverbSc		rv;
MoogLadder		ladder;

enum AdcChannel {
   pot1 = 0,
   pot2,
   pot3,
   pot4,
   pot5,
   NUM_ADC_CHANNELS
};

// MIDI note numbers for a major triad
const float kArpeggio[3] = {48.0f, 52.0f, 55.0f};
uint8_t     arp_idx;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float trig, freq, sig_out;
	trig = 0.f; 

	float speed = fmap(hw.adc.GetFloat(pot4), 1.f, 40.f);
	tick.SetFreq(speed);

	float decay = fmap(hw.adc.GetFloat(pot5), 0.f, 1.f);
	pp.SetDecay(decay);
	
	float feedback = fmap(hw.adc.GetFloat(pot1), 0.f, 1.f);
	rv.SetFeedback(feedback);

	float lpf = fmap(hw.adc.GetFloat(pot2), 1000.f, 32000.f, Mapping::LOG);
	rv.SetLpFreq(lpf);

	float mlpf = fmap(hw.adc.GetFloat(pot3), 1000.f, 32000.f, Mapping::LOG);
	ladder.SetFreq(mlpf);

	for (size_t i = 0; i < size; i++)
	{
		if(tick.Process()){
            arp_idx = (arp_idx + 1) % 3; // advance the kArpeggio, wrapping at the end.
			trig = 1.f;
		}

		freq    =  kArpeggio[arp_idx]; // convert midi nn to frequency.

		sig_out = pp.Process(trig, freq);
		sig_out = ladder.Process(sig_out);

		rv.Process(sig_out, sig_out, &OUT_L[i], &OUT_R[i]);

		out[0][i] = OUT_L[i] + (sig_out * .5f);
		out[1][i] = OUT_R[i];
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

	// pluck init 
	pp.Init(sample_rate);

	// reverb init
	rv.Init(sample_rate);

	// metro init 
	tick.Init(5.f, sample_rate);

	// filter 
    ladder.Init(sample_rate);
    ladder.SetRes(0.4);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
