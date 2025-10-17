// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Structure holding DSP parameters.

#ifndef DSP_DSP_PARAMETERS_H_
#define DSP_DSP_PARAMETERS_H_

#include "dsp/dsp_types.h"

#define NUM_PILOTS 2

typedef struct {
    uint32_t sample_rate;
    uint32_t symbol_rate;
    uint32_t zc_rate;
    uint32_t num_symbols;
    uint32_t num_null_symbols;

    uint32_t zc_length;
    uint32_t zc_root;
    uint32_t zc_shift;

    uint32_t shift_frequency;

    uint32_t symbol_scale;
    uint32_t symbol_max_value;
    bool symbol_clamp;

    uint32_t pilot_frequency[NUM_PILOTS];
    float pilot_amplitude[NUM_PILOTS];

    float rrc_roll_off;
} dsp_parameter_t;

#endif  // DSP_DSP_PARAMETERS_H_
