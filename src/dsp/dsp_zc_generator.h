// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Zadoff-Chu sync sequence generator.
//
// Note that this block requires a precomputed LUT for the phasor. In a typical
// use case, this LUT has been initiailized by the phasor bank.

#ifndef DSP_DSP_ZC_GENERATOR_H_
#define DSP_DSP_ZC_GENERATOR_H_

#include <stdint.h>

#include "dsp/dsp_phasor_bank.h"
#include "dsp/dsp_types.h"

typedef struct {
    uint32_t length;
    uint32_t root;
    uint32_t shift;
    phase_t phase;
    phase_t phase_increment;
    phase_t n;
    iq_sample_t value;
    iq_sample_t* lut_phasor;
} zc_generator_state_t;

void dsp_zc_generator_init(
    zc_generator_state_t* state,
    iq_sample_t* lut_phasor,
    uint32_t length,
    uint32_t root,
    uint32_t shift,
    uint32_t rate,
    uint32_t sample_rate);

void dsp_zc_generator_reset(zc_generator_state_t* state);

void dsp_zc_generator_process(
    zc_generator_state_t* state, iq_sample_t* out, size_t size);

#endif  // DSP_DSP_ZC_GENERATOR_H_
