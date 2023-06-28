// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>
#include "../inc/utils.h"
#include <iostream>

#ifndef NDEBUG
unsigned int _buffer_d_size = (SINGLE_PLATFORM_POS_DATA_SIZE * N_PULSES);
unsigned int _buffer_size;
unsigned int _size_in_chunk_data = SIZE_IN_CHUNK_DATA;
unsigned int _size_out_chunk_data = SIZE_OUT_CHUNK_DATA;
unsigned int _dma_size;
unsigned int _dma_in_size;
#endif

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
          const unsigned n_range_bins,
          const unsigned out_size,
          const unsigned n_pulses,
          dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(BUFFER_PLATFORM_POS_STARTING_IDX + n_pulses * ((n_range_bins * 2) + 3), VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++)
    {
    load_label0:
        dma_word_t in1_temp = in1[dma_index + i];
        for (unsigned j = 0; j < VALUES_PER_WORD; j++)
        {
            _inbuff[i * VALUES_PER_WORD + j] = in1_temp.word[j];

            assert(i * VALUES_PER_WORD + j < _size_in_chunk_data);
            assert(dma_index * VALUES_PER_WORD + i < _dma_in_size);
        }
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
           /* <<--compute-params-->> */
           const unsigned n_range_bins,
           const unsigned out_size,
           const unsigned n_pulses,
           dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(2 * out_size * out_size, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(BUFFER_PLATFORM_POS_STARTING_IDX + n_pulses * ((n_range_bins * 2) + 3), VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++)
    {
    store_label1:
        dma_word_t out_temp;
        for (unsigned j = 0; j < VALUES_PER_WORD; j++)
        {
            out_temp.word[j] = _outbuff[i * VALUES_PER_WORD + j];
        }
        out[dma_index + i] = out_temp;
    }
}

/*
    Here the input buffer is the concatenation of the inputs the kernel would usually expect:
    - fc
    - R0
    - dR
    - platpos
    - upsampled_data
*/
void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
             const unsigned n_range_bins,
             const unsigned out_size,
             const unsigned n_pulses,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    //printf("Now computing\n");
    //fflush(stdout);

    double fc = _inbuff[PARAM_FC_IDX];
    double R0 = _inbuff[PARAM_R0_IDX];
    double dR = _inbuff[PARAM_dR_IDX];
    double dxdy; // = dR;
    double ku;   // = 2.0 * M_PI * fc / SPEED_OF_LIGHT;
    dxdy = dR;
    dR /= RANGE_UPSAMPLE_FACTOR;
    ku = 2.0 * M_PI * (double)fc / SPEED_OF_LIGHT;

    int p, ix, iy;
    const double dR_inv = 1.0 / dR;

    for (iy = 0; iy < BP_NPIX_Y; ++iy)
    {
        const double py = (-(double)BP_NPIX_Y / 2.0 + 0.5 + iy) * dxdy;
        for (ix = 0; ix < BP_NPIX_X; ++ix)
        {
            complex accum;
            const double px = (-(double)BP_NPIX_X / 2.0 + 0.5 + ix) * dxdy;
            accum.real_part = accum.imaginary_part = 0.0f;
            for (p = 0; p < N_PULSES; ++p)
            {
                /* calculate the range R from the platform to this pixel */
                // Accessing platform position data
                const double xdiff = _inbuff[PLATFORM_POS_STARTING_IDX(p) + 0] - px;
                const double ydiff = _inbuff[PLATFORM_POS_STARTING_IDX(p) + 1] - py;
                const double zdiff = _inbuff[PLATFORM_POS_STARTING_IDX(p) + 2] - z0;
                assert(PLATFORM_POS_STARTING_IDX(p) + 0 < _size_in_chunk_data);
                assert(PLATFORM_POS_STARTING_IDX(p) + 1 < _size_in_chunk_data);
                assert(PLATFORM_POS_STARTING_IDX(p) + 2 < _size_in_chunk_data);
                const double xsqr = xdiff * xdiff;
                const double ysqr = ydiff * ydiff;
                const double zsqr = zdiff * zdiff;
                /*const double R = sqrt(
                    (double)xdiff * (double)xdiff + (double)ydiff * (double)ydiff + (double)zdiff * (double)zdiff);*/
                const double Rsqr = (double)xsqr + (double)ysqr + (double)zsqr;
                const double R = sqrt(Rsqr);
                /*const float R = hls::sqrt((
                    xdiff + ydiff + zdiff)*(
                    xdiff + ydiff + zdiff) - 2 * xdiff * ydiff - 2 * ydiff * zdiff - 2 * xdiff * zdiff);
                *//* convert to a range bin index */
                const double bin = (double)R  * (double)dR_inv - (double)R0 * (double)dR_inv;
                if (bin >= 0 && bin <= N_RANGE_UPSAMPLED - 2)
                {
                    complex_d sample, matched_filter, prod;
                    /* interpolation range is [bin_floor, bin_floor+1] */
                    const int bin_floor = (int)bin;
                    /* interpolation weight */
                    const double w = (double)(bin - (double)bin_floor);
                    /* linearly interpolate to obtain a sample at bin */
                    // sample.real_part = (1.0f-w)*upsampled_data[p][bin_floor].real_part + w*upsampled_data[p][bin_floor+1].real_part;

                    // This looks really convoluted but it's just summing two consecutive elements, which have size COMPLEX_DATA_SIZE
                    sample.real_part = (1.0 - w) * _inbuff[RANGE_BIN_STARTING_IDX(p) + bin_floor * COMPLEX_DATA_SIZE + COMPLEX_REAL_OFFSET] + w * _inbuff[RANGE_BIN_STARTING_IDX(p) + (bin_floor + 1) * COMPLEX_DATA_SIZE + COMPLEX_REAL_OFFSET]; // +0: real part
                    assert(RANGE_BIN_STARTING_IDX(p) + bin_floor + 1 * COMPLEX_DATA_SIZE + COMPLEX_REAL_OFFSET < _size_in_chunk_data);

                    // sample.imaginary_part = (1.0f-w)*upsampled_data[p][bin_floor].imaginary_part + w*upsampled_data[p][bin_floor+1].imaginary_part;
                    sample.imaginary_part = (1.0 - w) * _inbuff[RANGE_BIN_STARTING_IDX(p) + bin_floor * COMPLEX_DATA_SIZE + COMPLEX_IMAGINARY_OFFSET] + w * _inbuff[RANGE_BIN_STARTING_IDX(p) + (bin_floor + 1) * COMPLEX_DATA_SIZE + COMPLEX_IMAGINARY_OFFSET]; // +1: imaginary part
                    assert(RANGE_BIN_STARTING_IDX(p) + bin_floor + 1 < _size_in_chunk_data);
                    /* compute the complex exponential for the matched filter */
                    //matched_filter.real_part = hls::cos((2.0 * ku) * R - hls::floor(ku * R / M_PI) * M_PI * 2.0);
                    matched_filter.real_part = hls::cos((2.0 * ku) * R);
                    matched_filter.imaginary_part = hls::sin((2.0 * ku) * R);
                    //float temp = (2.0 * ku) * R - hls::floor(ku * R / M_PI) * M_PI * 2.0;
                    //matched_filter.imaginary_part = hls::sin((2.0 * ku) * R - hls::floor(ku * R / M_PI) * M_PI * 2.0);
                    /* scale the interpolated sample by the matched filter */
                    prod = cmult(sample, matched_filter);
                    /* accumulate this pulse's contribution into the pixel */
                    accum.real_part += prod.real_part;
                    accum.imaginary_part += prod.imaginary_part;
                }
            }
            // image[iy][ix] = accum;
            _outbuff[iy * BP_NPIX_X * COMPLEX_DATA_SIZE + ix * COMPLEX_DATA_SIZE] = (float)accum.real_part;
            _outbuff[iy * BP_NPIX_X * COMPLEX_DATA_SIZE + ix * COMPLEX_DATA_SIZE + 1] = (float)accum.imaginary_part;
            assert(iy * BP_NPIX_X * COMPLEX_DATA_SIZE + ix * COMPLEX_DATA_SIZE + 1 < _size_out_chunk_data);
        } // End for x
        std::cout << "Finished iteration \t" << iy + 1 << " /\t" << BP_NPIX_Y << std::endl;
    } // End for y

    // const unsigned length = round_up(n_pulses*((n_range_bins*2)+3), VALUES_PER_WORD) / 1;
}

void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
         const unsigned conf_info_n_range_bins,
         const unsigned conf_info_out_size,
         const unsigned conf_info_n_pulses,
         dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{
    std::cout << "Initialising" << std::endl;
    // fflush(stdout);
    /* <<--local-params-->> */
    // const unsigned n_range_bins = conf_info_n_range_bins;
    // const unsigned out_size = conf_info_out_size;
    // const unsigned n_pulses = conf_info_n_pulses;
    std::cout << "Range bins: " << conf_info_n_range_bins << std::endl
              << "Pulse amount: " << conf_info_n_pulses << std::endl
              << "Out size (bytes): " << conf_info_out_size << std::endl;
    // printf("Range bins: %u\nPulse amount: %u\nOut size (bytes): %u\n", n_range_bins, n_pulses, out_size);
    // fflush(stdout);

    std::cout << "Input buffer size (bytes): " << SIZE_IN_CHUNK_DATA << std::endl
              << "Output buffer size (bytes): " << SIZE_OUT_CHUNK_DATA << std::endl;

    // printf("Input buffer size (bytes): %u\n", SIZE_IN_CHUNK_DATA);
    // printf("Output buffer size (bytes): %u\n", SIZE_OUT_CHUNK_DATA);
    // fflush(stdout);

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            //word_t _inbuff[SIZE_IN_CHUNK_DATA];
            //word_t _outbuff[SIZE_OUT_CHUNK_DATA];
            word_t *_inbuff = (word_t *)malloc(SIZE_IN_CHUNK_DATA * sizeof(word_t));
            word_t *_outbuff = (word_t *)malloc(SIZE_OUT_CHUNK_DATA * sizeof(word_t));

            load(_inbuff, in1,
                 /* <<--args-->> */
                 conf_info_n_range_bins,
                 conf_info_out_size,
                 conf_info_n_pulses,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
                    conf_info_n_range_bins,
                    conf_info_out_size,
                    conf_info_n_pulses,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
                  conf_info_n_range_bins,
                  conf_info_out_size,
                  conf_info_n_pulses,
                  store_ctrl, c, b);

            free(_inbuff);
            free(_outbuff);
        }
    }
}
