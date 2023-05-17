// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned n_range_bins,
	 const unsigned out_size,
	 const unsigned n_pulses,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(n_pulses*((n_range_bins*2)+3), VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
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

    const unsigned length = round_up(2*out_size*out_size, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(n_pulses*((n_range_bins*2)+3), VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    store_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
	}
    }
}


void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
	 const unsigned n_range_bins,
	 const unsigned out_size,
	 const unsigned n_pulses,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    int p, ix, iy;
    const double dR_inv = 1.0/dR;

    for (iy = 0; iy < BP_NPIX_Y; ++iy)
    {
        const double py = (-BP_NPIX_Y/2.0 + 0.5 + iy) * dxdy;
        for (ix = 0; ix < BP_NPIX_X; ++ix)
        {
            complex accum;
            const double px = (-BP_NPIX_X/2.0 + 0.5 + ix) * dxdy;
            accum.real_part = accum.imaginary_part = 0.0f;
            for (p = 0; p < N_PULSES; ++p)
            {
                /* calculate the range R from the platform to this pixel */
                const double xdiff = platpos[p].x - px;
                const double ydiff = platpos[p].y - py;
                const double zdiff = platpos[p].z - z0;
                const double R = sqrt(
                    xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
                /* convert to a range bin index */
                const double bin = (R-R0)*dR_inv;
                if (bin >= 0 && bin <= N_RANGE_UPSAMPLED-2)
                {
                    complex sample, matched_filter, prod;
                    /* interpolation range is [bin_floor, bin_floor+1] */
                    const int bin_floor = (int) bin;
                    /* interpolation weight */
                    const float w = (float) (bin - (double) bin_floor);
                    /* linearly interpolate to obtain a sample at bin */
                    sample.real_part = (1.0f-w)*upsampled_data[p][bin_floor].real_part + w*upsampled_data[p][bin_floor+1].real_part;
                    sample.imaginary_part = (1.0f-w)*upsampled_data[p][bin_floor].imaginary_part + w*upsampled_data[p][bin_floor+1].imaginary_part;
                    /* compute the complex exponential for the matched filter */
                    matched_filter.real_part = cos(2.0 * ku * R);
                    matched_filter.imaginary_part = sin(2.0 * ku * R);
                    /* scale the interpolated sample by the matched filter */
                    prod = cmult(sample, matched_filter);
                    /* accumulate this pulse's contribution into the pixel */
                    accum.real_part += prod.real_part;
                    accum.imaginary_part += prod.imaginary_part;
                }
            }
            //image[iy][ix] = accum;
            _outbuff[iy * BP_NPIX_Y + ix] = accum.real_part;
            _outbuff[iy * BP_NPIX_Y + ix + 1] = accum.imaginary_part;
        }   // End for x
    }       // End for y

    // TODO implement compute functionality
    const unsigned length = round_up(n_pulses*((n_range_bins*2)+3), VALUES_PER_WORD) / 1;

    for (int i = 0; i < length; i++)
        _outbuff[i] = _inbuff[i];
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_n_range_bins,
	 const unsigned conf_info_out_size,
	 const unsigned conf_info_n_pulses,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned n_range_bins = conf_info_n_range_bins;
	 const unsigned out_size = conf_info_out_size;
	 const unsigned n_pulses = conf_info_n_pulses;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 n_range_bins,
	 	 out_size,
	 	 n_pulses,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 n_range_bins,
	 	 out_size,
	 	 n_pulses,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 n_range_bins,
	 	 out_size,
	 	 n_pulses,
                  store_ctrl, c, b);
        }
    }
}
