#pragma once
#ifndef POLYOSC_H_
#define POLYOSC_H_

#include "Synthesis/Oscillator.h"
#include "daisy_seed.h"

using namespace daisy;

const int NUM_OSC = 12;

namespace daisysp {
class PolyOscillator
{
public:
    void Init(float sample_rate)
    {
        for(int i = 0; i < NUM_OSC; i++)
        {
            sample_rate_ = sample_rate;
            osc_[i].Init(sample_rate_);
            osc_[i].SetWaveform(Oscillator::WAVE_SIN);
            env_[i].Init(sample_rate_);
            env_[i].SetTime(ADSR_SEG_ATTACK, 0.01f);
            env_[i].SetTime(ADSR_SEG_DECAY, 0.1f);
            rev_.Init(sample_rate);
            rev_.SetFeedback(0.9f);
            rev_.SetLpFreq(18000.0f);
            trig_[i] = false; // set trigger to false initially
        }
    }

    void SetFrequency(int voice, float freq)
    {
        //for(int i = 0; i < NUM_OSC; i++)
        //{
        //    osc_[i].SetFreq(freq);
        //}
        osc_[voice].SetFreq(freq);
    }

    void SetTrigger(int voice, bool trig)
    {
        if(voice >= 0 && voice < NUM_OSC)
        {
            trig_[voice] = trig;
            if(trig)
            {
                env_[voice].Trigger(); // trigger envelope if trigger is on
            }
        }
    }

    void SetAttack(float attack)
    {
        for(int i = 0; i < NUM_OSC; i++)
        {
            env_[i].SetTime(ADENV_SEG_ATTACK, attack);
        }
    }

    void SetDecay(float decay)
    {
        for(int i = 0; i < NUM_OSC; i++)
        {
            env_[i].SetTime(ADENV_SEG_DECAY, decay);
        }
    }

    void SetCuttoff(float cutoff)
    {
        rev_.SetLpFreq(cutoff);
    }

    void SetFeedback(float feedback)
    {
        rev_.SetFeedback(feedback);
    }

    float Process()
    {
        float pout = 0;
        float out;
        for(int i = 0; i < NUM_OSC; i++)
        {
            float env_val = env_[i].Process();
            pout += osc_[i].Process() * env_val;
        }
        rev_.Process(pout, pout, &out, &out);
        return out * 0.25f; // normalize by the number of voices
    }

    void ChangeWave() 
    {
        waveform_ = 0;
        final_wave_ = Oscillator::WAVE_POLYBLEP_TRI;
        waveform_ = (waveform_ % final_wave_ + final_wave_) % final_wave_;

        for(int i = 0; i < NUM_OSC; i++)
        {
            osc_[i].SetWaveform((uint8_t)waveform_);
        }
    }

private:
    Oscillator osc_[NUM_OSC];
    AdEnv env_[NUM_OSC];
    ReverbSc  rev_;
    bool trig_[NUM_OSC];
    float sample_rate_;
    float e_attack_;
    float e_decay_;
    int waveform_;
    int final_wave_;
};
}


#endif //POLYOSC_H_