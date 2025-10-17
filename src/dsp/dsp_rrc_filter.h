// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// RRC filter.

#ifndef DSP_DSP_RRC_FILTER_H_
#define DSP_DSP_RRC_FILTER_H_

#include "dsp/dsp_types.h"

#define LUT_RRC_NUM_SYMBOLS 11
#define LUT_RRC_POINTS_PER_SYMBOL 256
#define LUT_RRC_SIZE LUT_RRC_NUM_SYMBOLS * LUT_RRC_POINTS_PER_SYMBOL

// "Natural" ordering of the impulse response
//#define LUT_RRC_PHASE_FACTOR 1
//#define LUT_RRC_SYMBOL_FACTOR LUT_RRC_POINTS_PER_SYMBOL

// Reordering of the impulse response for SIMD optimizations
#define LUT_RRC_PHASE_FACTOR LUT_RRC_NUM_SYMBOLS
#define LUT_RRC_SYMBOL_FACTOR 1

typedef struct {
    phase_t phase;
    phase_t phase_increment;

    iq_sample_t past_symbols[LUT_RRC_NUM_SYMBOLS];

    sample_t* lut_rrc;
}  rrc_filter_state_t;

void dsp_rrc_filter_init(
    rrc_filter_state_t* state,
    sample_t* lut_rrc,
    float roll_off,
    uint32_t symbol_rate,
    uint32_t sample_rate);

void dsp_rrc_filter_reset(rrc_filter_state_t* state);

size_t dsp_rrc_filter_process(
    rrc_filter_state_t* state,
    iq_sample_t* in,
    iq_sample_t* out,
    size_t size);

#endif  // DSP_DSP_RRC_FILTER_H_
