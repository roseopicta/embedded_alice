// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// RRC filter.

#include "dsp/dsp_rrc_filter.h"

#include <math.h>
#include <stdio.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif  // M_PI

void dsp_rrc_filter_init(
        rrc_filter_state_t* state,
        sample_t* lut_rrc,
        float roll_off,
        uint32_t symbol_rate,
        uint32_t sample_rate) {
    state->phase_increment = dsp_phase_increment(symbol_rate, sample_rate);

    // Compute impulse response
    float scale = SAMPLE_MAX / (1.0f + roll_off * (4.0f / M_PI - 1.0f));
    int32_t mid_point = LUT_RRC_SIZE / 2;

    state->lut_rrc = lut_rrc;
    for (int32_t i = 0; i < LUT_RRC_SIZE; i++) {
        size_t symbol = i / LUT_RRC_POINTS_PER_SYMBOL;
        size_t phase = i % LUT_RRC_POINTS_PER_SYMBOL;

        size_t target_index = symbol * LUT_RRC_SYMBOL_FACTOR + phase * LUT_RRC_PHASE_FACTOR;
        float t = (i - mid_point) / (float)(LUT_RRC_POINTS_PER_SYMBOL);
        float denum_scale = 4.0f * roll_off * t;

        if (i == mid_point) {
            lut_rrc[target_index] = SAMPLE_MAX;
        } else if (fabs(denum_scale) == 1.0f) {
            lut_rrc[target_index] = scale * (roll_off / M_SQRT2) *
                ((1.0f + 2.0f / M_PI) * sinf(M_PI / (4.0f * roll_off)) +
                 (1.0f - 2.0f / M_PI) * cosf(M_PI / (4.0f * roll_off)));
        } else {
            lut_rrc[target_index] = (sinf(M_PI * t * (1.0f - roll_off)) +
                4.0f * roll_off * t * cosf(M_PI * t * (1.0f + roll_off))) /
                (M_PI * t * (1.0f - denum_scale * denum_scale)) * scale;
        }
    }

    dsp_rrc_filter_reset(state);
}

void dsp_rrc_filter_reset(rrc_filter_state_t* state) {
    state->phase = 0;
    for (size_t i = 0; i < LUT_RRC_NUM_SYMBOLS; ++i) {
        state->past_symbols[i] = (iq_sample_t) { .i = 0, .q = 0 };
    }
}

size_t dsp_rrc_filter_process(
        rrc_filter_state_t* state,
        iq_sample_t* in,
        iq_sample_t* out,
        size_t size) {
    rrc_filter_state_t s = *state;

    // Keep track of how many samples are consumed
    size_t consumed = 0;

    while (size--) {
        //sample_t* coeff = s.lut_rrc + (s.phase >> 24) * LUT_RRC_NUM_SYMBOLS;
        sample_t* coeff = s.lut_rrc + (s.phase >> 24) * LUT_RRC_PHASE_FACTOR;

        accumulator_t acc_i = 0;
        accumulator_t acc_q = 0;
        for (size_t k = 0; k < LUT_RRC_NUM_SYMBOLS; ++k) {
            iq_sample_t symbol = s.past_symbols[k];
            acc_i += ((accumulator_t)*coeff * (accumulator_t)symbol.i);
            acc_q += ((accumulator_t)*coeff * (accumulator_t)symbol.q);
            coeff += LUT_RRC_SYMBOL_FACTOR;
        }
        *out++ = (iq_sample_t) { .i = acc_i SCALE, .q = acc_q SCALE };

        // Advance the symbol clock.
        phase_t previous_phase = s.phase;
        s.phase += s.phase_increment;

        // Phase wrap: it's time to push a new symbol.
        if (s.phase < previous_phase) {
            for (size_t k = LUT_RRC_NUM_SYMBOLS - 1; k >= 1; --k) {
                s.past_symbols[k] = s.past_symbols[k - 1];
            }
            s.past_symbols[0] = *in++;
            consumed++;
        }
    }

    *state = s;

    return consumed;
}
