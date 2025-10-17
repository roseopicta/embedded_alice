// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Definition of the data type used to represent a sample.

#ifndef DSP_DSP_TYPES_H_
#define DSP_DSP_TYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define FIXED_POINT

#ifdef FIXED_POINT

    typedef int16_t sample_t;
    typedef int32_t accumulator_t;

    #define SAMPLE_MAX 32767
    #define SCALE >>15
    #define DAC_OUTPUT_SCALE 1
    #define FMT_STRING "%d"

#else
    typedef float sample_t;
    typedef float accumulator_t;

    #define SAMPLE_MAX 1.0
    #define SCALE
    #define DAC_OUTPUT_SCALE 32767
    #define FMT_STRING "%f"

#endif

typedef struct {
    sample_t i;
    sample_t q;
} iq_sample_t;

typedef uint32_t phase_t;

// Computes frequency / sample_rate * (1 << 32) to get a phase increment
// for a 32-bit phase counter.

inline phase_t dsp_phase_increment(phase_t frequency, phase_t sample_rate) {
    uint64_t increment = (uint64_t)frequency;
    increment <<= 32;
    increment /= sample_rate;
    return (phase_t)increment;
}

#endif  // DSP_DSP_TYPES_H_
