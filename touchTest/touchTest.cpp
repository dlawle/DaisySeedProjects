#include "daisy_seed.h"
#include "daisysp.h"
#include "dev/oled_ssd130x.h"
#include "touchEvent.h"

using namespace daisy;
using namespace daisysp;
using namespace seed; 

DaisySeed 		hw;
Oscillator 		osc;

int Freq[12] = {440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831};

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	int note = TouchedNote();
	osc.SetFreq(Freq[note]);
	for (size_t i = 0; i < size; i++)
	{
		float sig = osc.Process();

		out[0][i] = sig;
		out[1][i] = sig;
	} 
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	// init hardware 
	InitHardware();

	// init osc
	osc.Init(sample_rate);
	osc.SetFreq(440); 
	osc.SetAmp(1.f);
	osc.SetWaveform(osc.WAVE_SAW);

	hw.StartAudio(AudioCallback);
	while(1) {
		touchEvent();
		System::Delay(100);
	}
}
