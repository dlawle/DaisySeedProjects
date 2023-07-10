#include "daisy_seed.h"
#include "dsp/part.h"
#include "dsp/strummer.h"
#include "dsp/string_synth_part.h"
#include "cv_scaler.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;
using namespace torus;

DaisySeed	hw; 
Metro		tick;

uint16_t reverb_buffer[32768];
const int32_t kMaxPolyphony = 4;

CvScaler        cv_scaler;
Part            part;
StringSynthPart string_synth;
Strummer        strummer;

const char*           polyListValues[] = {"One", "Two", "Four"};
MappedStringListValue polyListValue(polyListValues, 3, 0);
const char*           modelListValues[] = {"Modal",
                                 "Symp Str",
                                 "Inhrm Str",
                                 "Fm Voice",
                                 "Westn Chrd",
                                 "Str & Verb"};
MappedStringListValue modelListValue(modelListValues, 6, 0);
const char*           eggListValues[]
    = {"Formant", "Chorus", "Reverb", "Formant2", "Ensemble", "Reverb2"};
MappedStringListValue eggListValue(eggListValues, 6, 0);

const char* controlListValues[]
    = {"Frequency", "Structure", "Brightness", "Damping", "Position"};
MappedStringListValue controlListValueOne(controlListValues, 5, 0);
MappedStringListValue controlListValueTwo(controlListValues, 5, 1);
MappedStringListValue controlListValueThree(controlListValues, 5, 2);
MappedStringListValue controlListValueFour(controlListValues, 5, 3);
MappedStringListValue controlListValueFive(controlListValues, 5, 4);

int old_poly = 0;

//easter egg toggle
bool easterEggOn;

int oldModel = 0;
bool trig;

// norm edit menu items
bool exciterIn;
bool strumIn;
bool noteIn;

void ProcessControls(Patch* patch, PerformanceState* state)
{
    // control settings
    cv_scaler.channel_map_[0] = controlListValueOne.GetIndex();
    cv_scaler.channel_map_[1] = controlListValueTwo.GetIndex();
    cv_scaler.channel_map_[2] = controlListValueThree.GetIndex();
    cv_scaler.channel_map_[3] = controlListValueFour.GetIndex();
	cv_scaler.channel_map_[4] = controlListValueFive.GetIndex();

    // normalization settings
    state->internal_note    = !noteIn;
    state->internal_exciter = !exciterIn;
    state->internal_strum   = !strumIn;

    //strum
    state->strum = trig;
}

float input[kMaxBlockSize];
float output[kMaxBlockSize];
float aux[kMaxBlockSize];

const float kNoiseGateThreshold = 0.00003f;
float       in_level            = 0.0f;

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

    PerformanceState performance_state;
    Patch            patch;

    cv_scaler.Read(&patch, &performance_state);

    if(easterEggOn)
    {
        for(size_t i = 0; i < size; ++i)
        {
            input[i] = in[0][i];
        }
        strummer.Process(NULL, size, &performance_state);
        string_synth.Process(
            performance_state, patch, input, output, aux, size);
    }
    else
    {
        // Apply noise gate.
        for(size_t i = 0; i < size; i++)
        {
            float in_sample = in[0][i];
            float error, gain;
            error = in_sample * in_sample - in_level;
            in_level += error * (error > 0.0f ? 0.1f : 0.0001f);
            gain = in_level <= kNoiseGateThreshold
                       ? (1.0f / kNoiseGateThreshold) * in_level
                       : 1.0f;
            input[i] = gain * in_sample;
        }

        strummer.Process(input, size, &performance_state);
        part.Process(performance_state, patch, input, output, aux, size);
    }

    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = output[i];
        out[1][i] = aux[i];
    }
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
   	float samplerate = hw.AudioSampleRate();
    float blocksize  = hw.AudioBlockSize();

	InitResources();

	AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
	adc_config[pot1].InitSingle(A0);
	adc_config[pot2].InitSingle(A1);
	adc_config[pot3].InitSingle(A4);
	adc_config[pot4].InitSingle(A5);
	adc_config[pot5].InitSingle(A9);
	hw.adc.Init(adc_config,NUM_ADC_CHANNELS);
	hw.adc.Start();

	// this needs replaced with a gate...
    tick.Init(5.f, samplerate);

    //polyphony setting
	part.set_polyphony(4);
	string_synth.set_polyphony(4);
 

    //model settings
    part.set_model(torus::ResonatorModel::RESONATOR_MODEL_STRING_AND_REVERB);
    string_synth.set_fx(torus::FxType::FX_REVERB_2);
	easterEggOn = false; 

	strummer.Init(0.01f, samplerate / blocksize);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    cv_scaler.Init();
	trig = false;	
	hw.StartAudio(AudioCallback);
	while(1) {
		trig = false;

		if(tick.Process()){
			trig = true;
		}

	}
}
