// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// The phasor bank block manages a bank of N=3 phasors with
// controllable frequency and amplitude and various mixing modes.
// Default: use phasor 1 to shift and add phasors 2 & 3 as pilots.

#ifndef DSP_DSP_PHASOR_BANK_H_
#define DSP_DSP_PHASOR_BANK_H_

#include <stdint.h>

#include "dsp/dsp_types.h"

#define NUM_PHASORS 3
#define LUT_PHASOR_LOG2_SIZE 15

#define LUT_PHASOR_FRACTIONAL_PART_SHIFT (LUT_PHASOR_LOG2_SIZE)
#define LUT_PHASOR_INTEGRAL_PART_SHIFT (32 - LUT_PHASOR_LOG2_SIZE)
#define LUT_PHASOR_SIZE (1 << LUT_PHASOR_LOG2_SIZE)

typedef enum {
    PHASOR_BANK_ALGORITHM_SHIFT_TWO_PILOTS
} phasor_bank_algorithm_t;

typedef struct {
    phase_t phase[NUM_PHASORS];
    phase_t phase_increment[NUM_PHASORS];
    accumulator_t amplitude[NUM_PHASORS];
    iq_sample_t* lut_phasor;
    phasor_bank_algorithm_t algorithm;
} phasor_bank_state_t;

void dsp_phasor_bank_init(
    phasor_bank_state_t* state,
    iq_sample_t* lut_phasor,
    phasor_bank_algorithm_t algorithm,
    uint32_t* frequency,
    float* amplitude,
    uint32_t sample_rate);

void dsp_phasor_bank_fill_lut(iq_sample_t* lut_phasor);

void dsp_phasor_bank_reset(phasor_bank_state_t* state);

void dsp_phasor_bank_process(
    phasor_bank_state_t* state, iq_sample_t* in_out, size_t size);

#endif  // DSP_DSP_PHASOR_BANK_H_
