// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef INC_ESPACC_H
#define INC_ESPACC_H

#include "../inc/espacc_config.h"
#include <cstdio>

#include <ap_fixed.h>
#include <ap_int.h>

#define __round_mask(x, y) ((y)-1)
#define round_up(x, y) ((((x)-1) | __round_mask(x, y)) + 1)

// Data types and constants
#define VALUES_PER_WORD (DMA_SIZE / DATA_BITWIDTH)
#if ((SIZE_IN_CHUNK_DATA % VALUES_PER_WORD) == 0)
#define SIZE_IN_CHUNK (SIZE_IN_CHUNK_DATA / VALUES_PER_WORD)
#else
#define SIZE_IN_CHUNK (SIZE_IN_CHUNK_DATA / VALUES_PER_WORD + 1)
#endif
#if ((SIZE_OUT_CHUNK_DATA % VALUES_PER_WORD) == 0)
#define SIZE_OUT_CHUNK (SIZE_OUT_CHUNK_DATA / VALUES_PER_WORD)
#else
#define SIZE_OUT_CHUNK (SIZE_OUT_CHUNK_DATA / VALUES_PER_WORD + 1)
#endif

// data word
#if (IS_TYPE_FIXED_POINT == 1)
typedef ap_fixed<DATA_BITWIDTH, DATA_BITWIDTH - FRAC_BITS> word_t;
typedef ap_fixed<DATA_BITWIDTH / 2, (DATA_BITWIDTH - FRAC_BITS) / 2> half_word_t;
#elif (IS_TYPE_UINT == 1)
typedef ap_uint<DATA_BITWIDTH> word_t;
#elif (IS_TYPE_FLOAT == 1)
#if (DATA_BITWIDTH == 32)
typedef float word_t;
#else
#error "Floating point word bitwidth not supported. Only 32 is supported."
#endif
#else // (IS_TYPE_INT == 1)
typedef ap_int<DATA_BITWIDTH> word_t;
#endif

typedef struct dma_word
{
	word_t word[VALUES_PER_WORD];
} dma_word_t;

typedef word_t in_data_word;
typedef word_t out_data_word;

// Ctrl
typedef struct dma_info
{
	ap_uint<32> index;
	ap_uint<32> length;
	ap_uint<32> size;
} dma_info_t;

typedef struct position_d
{
	double x, y, z;
} position_d_t;

typedef struct complex
{
	word_t real_part;
	word_t imaginary_part;
} complex_t;

typedef struct position
{
	word_t x, y, z;
} position_t;


// The 'size' variable of 'dma_info' indicates the bit-width of the words
// processed by the accelerator. Here are the encodings:
#define SIZE_BYTE 0
#define SIZE_HWORD 1
#define SIZE_WORD 2
#define SIZE_DWORD 3

//*#if (DATA_BITWIDTH == 8)
//*#define SIZE_WORD_T SIZE_BYTE
//*#elif (DATA_BITWIDTH == 16)
//*#define SIZE_WORD_T SIZE_HWORD
//*#elif (DATA_BITWIDTH == 32)
//*#define SIZE_WORD_T SIZE_WORD
//*#else // if (DATA_BITWIDTH == 64)
#define SIZE_WORD_T SIZE_DWORD
//#endif

void top(dma_word_t *out, dma_word_t *in1,
		 /* <<--params-->> */
		 const unsigned conf_info_n_range_bins,
		 const unsigned conf_info_out_size,
		 const unsigned conf_info_n_pulses,
		 dma_info_t &load_ctrl, dma_info_t &store_ctrl);

void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
			 word_t _outbuff[SIZE_OUT_CHUNK_DATA]);

////////////////////////////////////////
//	PHYSICAL CONSTANTS
//
#define SPEED_OF_LIGHT (3.0e8)

////////////////////////////////////////
//	IMPLEMENTATION CONSTANTS
//
const word_t z0 = 0.0f;
// Minimum signal-to-noise ratio w.r.t. the golden output to consider the test passed.
const double min_valid_signal_to_noise_ratio = 100.0;
// "Large" signal-to-noise ratio returned when the outputs are "equal".
const double large_signal_to_noise_ratio = 140.0;

////////////////////////////////////////
//	OTHER CONSTANTS
//
#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *output_filename = "small_kernel3_output.bin";
    static const char *golden_output_filename = "small_golden_kernel3_output.bin";
    static const char *input_filename = "small_kernel3_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *output_filename = "medium_kernel3_output.bin";
    static const char *golden_output_filename = "medium_golden_kernel3_output.bin";
    static const char *input_filename = "medium_kernel3_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *output_filename = "large_kernel3_output.bin";
    static const char *golden_output_filename = "large_golden_kernel3_output.bin";
    static const char *input_filename = "large_kernel3_input.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif
// From kernel
#define MAX_DIR_AND_FILENAME_LEN (1024) 

#define N_RANGE_BINS N_RANGE

// These are for properly indexing the input array.
// Data file format:
//		fc, R0, dR, platpos, upsampled_data

// Start of the global elements
// After fc, R0, dR
#define BUFFER_PLATFORM_POS_STARTING_IDX (unsigned int)3
// After N_PULSES platform positions, each with 3 elements
#define BUFFER_RANGE_BIN_STARTING_IDX (unsigned int)(BUFFER_PLATFORM_POS_STARTING_IDX + (unsigned int)N_PULSES * SINGLE_PLATFORM_POS_DATA_SIZE)

// Sizes
//(N_RANGE_BINS * 2) // Complex
#define SINGLE_PULSE_DATA_SIZE 2 * N_RANGE_UPSAMPLED
// x, y, z
#define SINGLE_PLATFORM_POS_DATA_SIZE 3
// Amount of words in data struct (this only exists to avoid having magic numbers arounds)
#define COMPLEX_DATA_SIZE 2
#define COMPLEX_REAL_OFFSET 0
#define COMPLEX_IMAGINARY_OFFSET 1

// Start of the idx-th element
#define PARAM_FC_IDX 0
#define PARAM_R0_IDX 1
#define PARAM_dR_IDX 2
#define PLATFORM_POS_STARTING_IDX(idx) BUFFER_PLATFORM_POS_STARTING_IDX + SINGLE_PLATFORM_POS_DATA_SIZE * idx
#define RANGE_BIN_STARTING_IDX(idx) BUFFER_RANGE_BIN_STARTING_IDX + SINGLE_PULSE_DATA_SIZE * idx


#ifndef NDEBUG
    extern unsigned int _buffer_d_size;
    extern unsigned int _buffer_size;
	extern unsigned int _size_in_chunk_data;
	extern unsigned int _size_out_chunk_data;
	extern unsigned int _dma_size;
	extern unsigned int _dma_in_size;
#endif

#endif