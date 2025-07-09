//
// Created by arden on 7/8/25.
//

#ifndef SCHLAPPI_VCV_RESAMPLERS_H
#define SCHLAPPI_VCV_RESAMPLERS_H

#include "filter-c/filter.h"

template <int OVERSAMPLE, int QUALITY>
struct IIRUpsampler {
    IIRUpsampler(float cutoff /* 0: 0hz, 1: nyquist */) {
        filter = create_bw_low_pass_filter(QUALITY, 2.0f * OVERSAMPLE, cutoff);
    }

    IIRUpsampler() : IIRUpsampler(0.1f) {

    }

    ~IIRUpsampler() {
        free_bw_low_pass(filter);
    }
    void reset() {

    }

    void process(float in, float* out) {
        for (int i = 0; i < OVERSAMPLE; ++i) {
            out[i] = bw_low_pass(filter, in);
            // out[i] = in;
        }
    }
private:
    BWLowPass* filter;
};

template<int OVERSAMPLE, int QUALITY>
struct IIRDecimator {
    IIRDecimator() : IIRDecimator(0.6f) {

    }

    IIRDecimator(float cutoff) {
        filter = create_che_low_pass_filter(QUALITY, 1.f, 2.0f * OVERSAMPLE, cutoff);
        std::cout << 2.0f * OVERSAMPLE << " " << cutoff << "\n";
    }


    ~IIRDecimator() {
        free_che_low_pass(filter);
        std::cout << "destruct\n";
    }

    void reset() {

    }

    float process(float* in) {
        float res = 0;
        for (int i = 0; i < OVERSAMPLE; ++i) {
            res = che_low_pass(filter, in[i]);
        }
        return res;
    }
private:
    CHELowPass* filter;

};

#endif //SCHLAPPI_VCV_RESAMPLERS_H
