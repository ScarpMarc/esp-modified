// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "sar-bp-naive_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define N_RANGE_BINS 8192
#define OUT_SIZE 1024
#define N_PULSES 1024

/* <<--params-->> */
const int32_t n_range_bins = N_RANGE_BINS;
const int32_t out_size = OUT_SIZE;
const int32_t n_pulses = N_PULSES;

#define NACC 1

struct sar-bp-naive_vivado_access sar-bp-naive_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.n_range_bins = N_RANGE_BINS,
		.out_size = OUT_SIZE,
		.n_pulses = N_PULSES,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "sar-bp-naive_vivado.0",
		.ioctl_req = SAR-BP-NAIVE_VIVADO_IOC_ACCESS,
		.esp_desc = &(sar-bp-naive_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
