// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Zadoff-Chu sync sequence generator.

#include "dsp/dsp_zc_generator.h"

#include <math.h>

void dsp_zc_generator_init(
        zc_generator_state_t* state,
        iq_sample_t* lut_phasor,
        uint32_t length,
        uint32_t root,
        uint32_t shift,
        uint32_t rate,
        uint32_t sample_rate) {
    state->lut_phasor = lut_phasor;
    state->length = length;
    state->root = root;
    state->shift = shift;
    state->phase_increment = dsp_phase_increment(rate, sample_rate);

    dsp_phasor_bank_fill_lut(lut_phasor);
    dsp_zc_generator_reset(state);
}

void dsp_zc_generator_reset(zc_generator_state_t* state) {
    state->phase = -1;
    state->n = -1;
    state->value = (iq_sample_t) { .i = 0, .q = 0 };
}

void dsp_zc_generator_process(
        zc_generator_state_t* state,
        iq_sample_t* out,
        size_t size) {
    zc_generator_state_t s = *state;
    while (size--) {
        phase_t previous_phase = s.phase;
        s.phase += s.phase_increment;
        if (s.phase < previous_phase) {
            uint32_t l = s.length;
            s.n = (s.n + 1) % l;
            uint32_t n = s.n;
            uint32_t u = s.root;
            uint32_t i = u * n * (n + (l % 2) + 2 * s.shift);
            i *= (1 << 31) / l;
            iq_sample_t v = s.lut_phasor[-i >> LUT_PHASOR_INTEGRAL_PART_SHIFT];
            s.value = (iq_sample_t) {
                .i = (accumulator_t)(v.i) * DAC_OUTPUT_SCALE,
                .q = (accumulator_t)(v.q) * DAC_OUTPUT_SCALE };
            /*float t = -M_PI * (float)i / (float)l;
            s.value = (iq_sample_t) {
                .i = cosf(t) * SAMPLE_MAX,
                .q = sinf(t) * SAMPLE_MAX };*/
        }
        *out++ = s.value;
    }
    *state = s;
}
