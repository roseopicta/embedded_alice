// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Command line tool employing all DSP blocks to generate a QOSST-compatible
// frame.

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dsp/dsp_parameters.h"
#include "dsp/dsp_phasor_bank.h"
#include "dsp/dsp_rng.h"
#include "dsp/dsp_rrc_filter.h"
#include "dsp/dsp_zc_generator.h"
}

#include <fstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/log.h"

ABSL_FLAG(uint64_t, sample_rate, 2000000000, "Sample rate in Hz");
ABSL_FLAG(uint64_t, symbol_rate, 100000000, "Symbol rate in Hz");
ABSL_FLAG(uint64_t, zc_rate, 50000000, "ZC sample rate in Hz");
ABSL_FLAG(uint32_t, zc_length, 3989, "ZC sequence length");
ABSL_FLAG(uint32_t, zc_root, 5, "ZC root");
ABSL_FLAG(uint32_t, zc_shift, 0, "ZC shift");
ABSL_FLAG(uint32_t, num_symbols, 1000000, "Number of symbols");
ABSL_FLAG(uint32_t, num_null_symbols, 10, "Number of null tail symbols");
ABSL_FLAG(uint32_t, symbol_scale, 7500, "Symbol scale");
ABSL_FLAG(uint32_t, symbol_max_value, 0x5fff, "Symbol maximum value");
ABSL_FLAG(bool, symbol_clamp, false, "Clamp symbols to max_value");
ABSL_FLAG(double, rrc_roll_off, 0.3, "RRC roll-off factor");
ABSL_FLAG(uint32_t, shift_frequency, 0, "Frequency shift (Hz)");
ABSL_FLAG(uint32_t, pilot_1_freq, 200e6, "Pilot 1 frequency in Hz");
ABSL_FLAG(double, pilot_1_amplitude, 0.16, "Pilot 1 amplitude");
ABSL_FLAG(uint32_t, pilot_2_freq, 220e6, "Pilot 2 frequency in Hz");
ABSL_FLAG(double, pilot_2_amplitude, 0.16, "Pilot 2 amplitude");

ABSL_FLAG(std::string, output, "out_iq.bin", "Output I/Q samples file name");
ABSL_FLAG(std::string, output_symbols, "out_symbols.tsv",
          "Output symbols file name");

iq_sample_t lut_phasor[LUT_PHASOR_SIZE];
sample_t lut_rrc[LUT_RRC_SIZE];

using namespace std;

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);

    dsp_parameter_t dsp_parameters;

    dsp_parameters.sample_rate =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_sample_rate));
    dsp_parameters.symbol_rate =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_symbol_rate));
    dsp_parameters.zc_rate =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_zc_rate));
    dsp_parameters.num_symbols =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_num_symbols));
    dsp_parameters.num_null_symbols =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_num_null_symbols));

    dsp_parameters.zc_length =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_zc_length));
    dsp_parameters.zc_root =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_zc_root));
    dsp_parameters.zc_shift =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_zc_shift));

    dsp_parameters.shift_frequency =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_shift_frequency));
    dsp_parameters.pilot_frequency[0] =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_pilot_1_freq));
    dsp_parameters.pilot_frequency[1] =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_pilot_2_freq));
    dsp_parameters.pilot_amplitude[0] =
        static_cast<float>(absl::GetFlag(FLAGS_pilot_1_amplitude));
    dsp_parameters.pilot_amplitude[1] =
        static_cast<float>(absl::GetFlag(FLAGS_pilot_2_amplitude));

    dsp_parameters.symbol_scale =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_symbol_scale));
    dsp_parameters.symbol_max_value =
        static_cast<uint32_t>(absl::GetFlag(FLAGS_symbol_max_value));
    dsp_parameters.symbol_clamp = absl::GetFlag(FLAGS_symbol_clamp);

    dsp_parameters.rrc_roll_off =
        static_cast<float>(absl::GetFlag(FLAGS_rrc_roll_off));

    // Allocate symbols buffer (and RRC tail)
    const size_t LUT_RRC_NUM_SYMBOLS_LOCAL = LUT_RRC_NUM_SYMBOLS;
    vector<iq_sample_t> symbols(
        (dsp_parameters.num_symbols + LUT_RRC_NUM_SYMBOLS_LOCAL));

    rng_state_t rng_state;
    LOG(INFO) << "Generating random symbols...";
    dsp_rng_init(&rng_state, dsp_parameters.symbol_scale,
                 dsp_parameters.symbol_max_value, dsp_parameters.symbol_clamp,
                 1, 0);
    dsp_rng_generate_icdf(&rng_state, symbols.data(),
                          dsp_parameters.num_symbols);

    for (size_t i = 0; i < LUT_RRC_NUM_SYMBOLS_LOCAL; ++i) {
        symbols[i + dsp_parameters.num_symbols] = iq_sample_t{0, 0};
    }

    const uint32_t sr = dsp_parameters.sample_rate;
    const uint32_t symbol_rate = dsp_parameters.symbol_rate;
    const uint32_t zc_rate = dsp_parameters.zc_rate;
    size_t num_samples_zc = dsp_parameters.zc_length * (sr / zc_rate);
    size_t num_samples_qd = dsp_parameters.num_symbols * (sr / symbol_rate);
    size_t num_samples_tail =
        dsp_parameters.num_null_symbols * (sr / symbol_rate);
    size_t num_samples = num_samples_zc + num_samples_qd + num_samples_tail;

    LOG(INFO) << "Generating IQ samples...";
    vector<iq_sample_t> samples(num_samples);

    rrc_filter_state_t rrc_state;
    dsp_rrc_filter_init(&rrc_state, &lut_rrc[0], dsp_parameters.rrc_roll_off,
                        symbol_rate, sr);

    size_t num_first_samples_truncated = sr / symbol_rate * 25 / 4;
    size_t consumed = dsp_rrc_filter_process(&rrc_state, &symbols[0],
                                             &samples[num_samples_zc],
                                             num_first_samples_truncated);
    consumed +=
        dsp_rrc_filter_process(&rrc_state, &symbols[consumed],
                               &samples[num_samples_zc], num_samples_qd);

    phasor_bank_state_t phasor_state;
    uint32_t frequencies[3] = {dsp_parameters.shift_frequency,
                               dsp_parameters.pilot_frequency[0],
                               dsp_parameters.pilot_frequency[1]};
    float amplitudes[3] = {0.70710678118f, dsp_parameters.pilot_amplitude[0],
                           dsp_parameters.pilot_amplitude[1]};
    dsp_phasor_bank_init(&phasor_state, &lut_phasor[0],
                         PHASOR_BANK_ALGORITHM_SHIFT_TWO_PILOTS, frequencies,
                         amplitudes, sr);
    dsp_phasor_bank_process(&phasor_state, &samples[num_samples_zc],
                            num_samples_qd + num_samples_tail);

    LOG(INFO) << "Generating sync sequence...";
    zc_generator_state_t zc_state;
    dsp_zc_generator_init(&zc_state, phasor_state.lut_phasor,
                          dsp_parameters.zc_length, dsp_parameters.zc_root,
                          dsp_parameters.zc_shift, zc_rate, sr);
    dsp_zc_generator_process(&zc_state, &samples[0], num_samples_zc);

    LOG(INFO) << "Done...";

    vector<short> dac_samples(2 * num_samples);
    for (size_t i = 0; i < num_samples; ++i) {
        dac_samples[i * 2] = samples[i].i;
        dac_samples[i * 2 + 1] = samples[i].q;
    }

    // Write TSV
    std::ofstream symbols_file(absl::GetFlag(FLAGS_output_symbols));
    if (!symbols_file) {
        LOG(ERROR) << "Failed to open " << absl::GetFlag(FLAGS_output_symbols);
    } else {
        for (const auto& s : symbols) {
            symbols_file << s.i << '\t' << s.q << '\n';
        }
    }

    // Write binary samples
    std::ofstream bin_file(absl::GetFlag(FLAGS_output), std::ios::binary);
    if (!bin_file) {
        LOG(ERROR) << "Failed to open " << absl::GetFlag(FLAGS_output);
    } else {
        bin_file.write(reinterpret_cast<const char*>(dac_samples.data()),
                       dac_samples.size() * sizeof(int16_t));
    }

    return 0;
}
