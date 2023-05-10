// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
          const unsigned n_range,
          const unsigned n_out,
          const unsigned n_pulses,
          dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(2 * n_pulses * (n_range + 2), VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++)
    {
    load_label0:
        for (unsigned j = 0; j < VALUES_PER_WORD; j++)
        {
            _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
        }
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
           /* <<--compute-params-->> */
           const unsigned n_range,
           const unsigned n_out,
           const unsigned n_pulses,
           dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(2 * n_out, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(2 * n_pulses * (n_range + 2), VALUES_PER_WORD) * n_range;
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
        for (unsigned j = 0; j < VALUES_PER_WORD; j++)
        {
            out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
        }
    }
}

void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
             const unsigned n_range,
             const unsigned n_out,
             const unsigned n_pulses,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
    const unsigned length = round_up(2 * n_pulses * (n_range + 2), VALUES_PER_WORD) / 1;

    for (int i = 0; i < length; i++)
        _outbuff[i] = _inbuff[i];
}

void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
         const unsigned conf_info_n_range,
         const unsigned conf_info_n_out,
         const unsigned conf_info_n_pulses,
         dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
    const unsigned n_range = conf_info_n_range;
    const unsigned n_out = conf_info_n_out;
    const unsigned n_pulses = conf_info_n_pulses;

    // Batching
batching:
    for (unsigned b = 0; b < n_range; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
                 n_range,
                 n_out,
                 n_pulses,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
                    n_range,
                    n_out,
                    n_pulses,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
                  n_range,
                  n_out,
                  n_pulses,
                  store_ctrl, c, b);
        }
    }
}
