#include "daisy_seed.h"
#include "daisysp.h"
#include "TouchScreen.h"
#include "per/gpio.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed hw;
TouchScreen ts = TouchScreen(300);
Oscillator osc;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	TSPoint p = ts.getPoint();
	float freq = p.x;
	osc.SetFreq(freq);
	for (size_t i = 0; i < size; i++)
	{
		float sig;
		sig = osc.Process();
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

	ts.SetRx(300);

	osc.Init(sample_rate);
	osc.SetAmp(0.5);
	osc.SetWaveform(Oscillator::WAVE_SAW);
	osc.SetFreq(440);
	hw.StartAudio(AudioCallback);
	while(1) {
	}
}
