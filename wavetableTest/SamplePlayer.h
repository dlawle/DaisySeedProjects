#pragma once
#ifndef SAMPLEPLAYER_H
#define SAMPLEPLAYER_H

const size_t sample_length = 179777;
float sample_float[sample_length];

class SamplePlayer {
public:
    SamplePlayer() {}
    ~SamplePlayer() {}

    void Init(float sample_rate) {
        sample_rate_ = sample_rate;
        phase_ = 0;
        playback_speed_ = 1.0f;
        sample_data_ = nullptr;
        sample_length_ = 0;
    }

    float sampleProcess(const unsigned int sample, int length) {
	// Convert the unsigned integer data to floating-point data
        for (size_t i = 0; i < sample_length; i++) {
            sample_float[i] = static_cast<float>(sample[i]) / 32768.0f;
        }
    }

    void SetSampleData(const float* data, size_t length) {
        sample_data_ = data;
        sample_length_ = length;
    }

    void Start() {
        phase_ = 0;
    }

    float Process() {
        if (!sample_data_) return 0.0f;
        
        float output = sample_data_[static_cast<size_t>(phase_)];
        phase_ += playback_speed_;
        if (phase_ >= sample_length_) {
            phase_ -= sample_length_;
        }

        return output;
    }

    void SetPlaybackSpeed(float speed) {
        playback_speed_ = speed;
    }

private:
    float sample_rate_;
    float phase_;
    float playback_speed_;
    const float* sample_data_;
    size_t sample_length_;
};

#endif //SAMPLEPLAYER_H