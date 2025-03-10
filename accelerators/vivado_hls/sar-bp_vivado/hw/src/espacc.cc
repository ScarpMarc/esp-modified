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

    const unsigned length = round_up(n_pulses * ((n_range_bins*2)+3), VALUES_PER_WORD) / n_pulses;
    const unsigned index = length * (batch * n_pulses + chunk);

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

    const unsigned length = round_up((out_size * 2) * (out_size * 2), VALUES_PER_WORD) / n_pulses;
    const unsigned store_offset = round_up(n_pulses * ((n_range_bins*2)+3), VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * n_pulses + chunk);

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
    /*
        Compute single range bin and output the corresponding image pixel.

        DATA FORMAT:
        First three bytes are the x, y, z coordinates of the platform; the rest
        are the range bins. Each range bin is represented by two bytes, the
        first of which is the real part and the second of which is the
        imaginary part.
    */
    float platform_x = _inbuff[0];
    float platform_y = _inbuff[1];
    float platform_z = _inbuff[2];

    complex_t in[n_range_bins];
    for(unsigned int i = 0; i < n_range_bins; ++i) {
        in[i].real_part = _inbuff[i+3];
        in[i].imaginary_part = _inbuff[i+4];
    }
   

    // TODO implement compute functionality
    const unsigned length = round_up(n_pulses * ((n_range_bins*2)+3), VALUES_PER_WORD) / n_pulses;

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
        for (int c = 0; c < n_pulses; c++)
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
