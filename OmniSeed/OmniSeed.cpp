#include "daisy_seed.h"
#include "daisysp.h"
#include "keypad.h"
#include "touchEvent.h"
#include "notes.h"
#include "PolyPluck_p.h"
#include "PolyOsc.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

/******************
 * System setup	  *
******************/
DaisySeed 		hw;
KeyboardMatrix 	kb;
Switch 			oct1,oct2,reset;
// testing cpuloads
CpuLoadMeter loadMeter;

enum AdcChannel {
   pot1 = 0,
   pot2,
   pot3,
   pot4,
   pot5,
   pot6,
   rot,
   NUM_ADC_CHANNELS
};


void logInit();
void syslog();
void systemReset();

/******************
 * Audio setup	  *
******************/
int metaKey, metaScale, touchedPad;
const auto NUM_VOICES = 24;
float trig = 0.0f;
bool sus = false;
bool test = false;
int voice = 0;
bool change_wave = false;

PolyPluck_p<NUM_VOICES>			pluck; 
Resonator						res;
PolyOscillator DSY_SDRAM_BSS	posc;
WaveTableLoader					wtl;

float voiceSelect(float rootFreq, float augFreq);
float buttonHandler();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	// testing cpuloads
	loadMeter.OnBlockStart();
	float rootFreq, augFreq, pludec, bright, structr, damp, attack, decay, feedback, cutoff;
	rootFreq = noteArray[metaKey];
	augFreq = chordNote[metaScale][touchedPad];
	oct1.Debounce();
	oct2.Debounce();

	pludec = fmap(hw.adc.GetFloat(pot1), 0.1f, 1.0f); 
	bright = fmap(hw.adc.GetFloat(pot1), 0.0f, 1.0f);
	damp = fmap(hw.adc.GetFloat(pot2), 0.0f, 1.0f);
	structr = fmap(hw.adc.GetFloat(pot3), 0.0f, 1.0f);
	attack = fmap(hw.adc.GetFloat(pot5), .01, 2);
	decay  = fmap(hw.adc.GetFloat(pot6), 0.1f, 1.0f);
	feedback = fmap(hw.adc.GetFloat(pot3), 0.3f, 0.99f);
	cutoff = fmap(hw.adc.GetFloat(pot4), 1000.f, 19000.f, Mapping::LOG);



	pluck.SetDecay(pludec);
	res.SetBrightness(bright);
	res.SetDamping(damp);
	res.SetStructure(structr);
	posc.SetAttack(attack);
	posc.SetDecay(decay);
	posc.SetCuttoff(cutoff);
	posc.SetFeedback(feedback);

	for (size_t i = 0; i < size; i++)
	{
		float output;

		output = voiceSelect(rootFreq, augFreq);
		
		out[0][i] = output;
		out[1][i] = output;
	}
	// testing cpuloads
	loadMeter.OnBlockEnd();
}

int main(void)
{
	hw.Init(true);
	hw.SetAudioBlockSize(32); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
	adc_config[pot1].InitSingle(A0);
	adc_config[pot2].InitSingle(A1);
	adc_config[pot3].InitSingle(A2);
	adc_config[pot4].InitSingle(A3);
	adc_config[pot5].InitSingle(A4);
	adc_config[pot6].InitSingle(A5);
	adc_config[rot].InitSingle(A7);
	hw.adc.Init(adc_config,NUM_ADC_CHANNELS);
	hw.adc.Start();

	// testing cpuloads
	loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	logInit();

	// hw init
	Init121();
	kb.init();
	oct1.Init(seed::D30, sample_rate);
	oct2.Init(seed::D29, sample_rate);
	reset.Init(seed::D27, sample_rate);

	// audio init
	pluck.Init(sample_rate);
	res.Init(.025, 24, sample_rate);
	posc.Init(sample_rate);

	hw.StartAudio(AudioCallback);
	while(1) {		
		systemReset();
		std::pair<int, int> key = kb.iReadKey();
		
		if(key.first != 99){
			metaKey = key.first;
			metaScale = key.second;
			//hw.Print("Key of: %d", metaKey);
			//hw.PrintLine("\t Scale: %d", metaScale);
		}
		
		touchEvent();
		if(TouchedNote() != 99 && !hasTriggered){
			touchedPad = TouchedNote();
			trig = 1.0f;
			hasTriggered = true; // set the trigger flag to prevent repeated triggers
			test = true;
			//hw.PrintLine("Pad: %d", touchedPad);
		} else { test = false;}

	syslog();
	System::Delay(10);
	}
}

void logInit(){
	hw.StartLog();
	hw.PrintLine("Hello World!");
}

void syslog(){
	// testing cpuloads
	// get the current load (smoothed value and peak values)
	const float avgLoad = loadMeter.GetAvgCpuLoad();
	const float maxLoad = loadMeter.GetMaxCpuLoad();
	const float minLoad = loadMeter.GetMinCpuLoad();
	// print it to the serial connection (as percentages)
	hw.PrintLine("Processing Load %:");
	hw.PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
	hw.PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
	hw.PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
}

float voiceSelect(float rootFreq, float augFreq){
	voice = hw.adc.Get(rot);
	float sig;
	float oct = buttonHandler();
	float finalNote = (rootFreq + augFreq + oct);
	float fnFreq = mtof(finalNote);
	float t_sig = touchedPad;
	switch (voice)
	{
	case 0 ... 1000:
		//plucker
		sig = pluck.Process(trig, finalNote);
		break;

	case 14000 ... 25000:
		// resonator 
		res.SetFreq(fnFreq); 
		sig = res.Process(t_sig);
		break;

	case 26000 ... 36000:
		// poly oscillator
		posc.SetFrequency(touchedPad, fnFreq);
		posc.SetTrigger(touchedPad, test);
		sig = posc.Process();
		break;

	case 45000 ... 55000:
		break;

	case 58000 ... 70000:
		break;


	default:
		voice = 0;
		break;
	}
		
	
	return sig;
}

float buttonHandler(){
	float octave;
	if(oct1.Pressed()){
		octave = 12;
	} else if(oct2.Pressed()){
		octave = -12;
	} else { octave = 0; }

	return octave;
}

void systemReset(){
	reset.Debounce();
	if(reset.Pressed()){
		System::ResetToBootloader();
	}
}