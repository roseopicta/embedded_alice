// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// The phasor bank block manages a bank of N=3 phasors with
// controllable frequency and amplitude and various mixing modes.
// Default: use phasor 1 to shift and add phasors 2 & 3.

#include "dsp/dsp_phasor_bank.h"

#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif  // M_PI

void dsp_phasor_bank_init(
        phasor_bank_state_t* state,
        iq_sample_t* lut_phasor,
        phasor_bank_algorithm_t algorithm,
        uint32_t* frequency,
        float* amplitude,
        uint32_t sample_rate) {
    state->algorithm = algorithm;

    for (size_t i = 0; i < NUM_PHASORS; ++i) {
        state->phase_increment[i] = dsp_phase_increment(
            frequency[i], sample_rate);
        state->amplitude[i] = (accumulator_t)(amplitude[i] * SAMPLE_MAX);
    }

    if (algorithm == PHASOR_BANK_ALGORITHM_SHIFT_TWO_PILOTS) {
        state->amplitude[1] *= DAC_OUTPUT_SCALE;
        state->amplitude[2] *= DAC_OUTPUT_SCALE;
    }

    dsp_phasor_bank_fill_lut(lut_phasor);
    dsp_phasor_bank_reset(state);

    state->lut_phasor = lut_phasor;
}

void dsp_phasor_bank_fill_lut(iq_sample_t* lut_phasor) {
    if (lut_phasor[0].i == SAMPLE_MAX && lut_phasor[0].q == 0) {
        return;
    }

    for (size_t i = 0; i < LUT_PHASOR_SIZE; ++i) {
        float angle = 2 * M_PI * (float)(i) / (float)(LUT_PHASOR_SIZE);
        lut_phasor[i] = (iq_sample_t) {
            .i = cosf(angle) * (float)SAMPLE_MAX,
            .q = sinf(angle) * (float)SAMPLE_MAX };
    }
}

void dsp_phasor_bank_reset(phasor_bank_state_t* state) {
    for (size_t i = 0; i < NUM_PHASORS; ++i) {
        state->phase[i] = 0;
    }
}

void dsp_phasor_bank_process(
        phasor_bank_state_t* state,
        iq_sample_t* in_out,
        size_t size) {
    phasor_bank_state_t s = *state;

    while (size--) {
        iq_sample_t phasors[NUM_PHASORS];
        for (size_t i = 0; i < NUM_PHASORS; ++i) {
            accumulator_t amplitude = s.amplitude[i];
            iq_sample_t p = s.lut_phasor[
                s.phase[i] >> LUT_PHASOR_INTEGRAL_PART_SHIFT];
            /*iq_sample_t p;
            p.i = cosf((float)(s.phase[i] >> 12) / (float)(1 << 20) * 2.0f * M_PI);
            p.q = sinf((float)(s.phase[i] >> 12) / (float)(1 << 20) * 2.0f * M_PI);*/
            phasors[i] = (iq_sample_t) {
                .i = p.i * amplitude SCALE,
                .q = p.q * amplitude SCALE
            };
            s.phase[i] += s.phase_increment[i];
        }

        iq_sample_t x = *in_out;
        if (s.algorithm == PHASOR_BANK_ALGORITHM_SHIFT_TWO_PILOTS) {
            // Use first phasor to modulate.
            iq_sample_t y = phasors[0];
            x = (iq_sample_t) {
                .i = (x.i * y.i - x.q * y.q) SCALE,
                .q = (x.q * y.i + x.i * y.q) SCALE
            };
            // Add the two other phasors to the signal.
            x.i += phasors[1].i + phasors[2].i;
            x.q += phasors[1].q + phasors[2].q;
        }
        *in_out++ = x;
    }

    *state = s;
}
