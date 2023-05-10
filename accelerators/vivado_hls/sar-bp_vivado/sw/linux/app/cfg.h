// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "sar-bp_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define N_RANGE 1024
#define N_OUT 1024
#define N_PULSES 1024

/* <<--params-->> */
const int32_t n_range = N_RANGE;
const int32_t n_out = N_OUT;
const int32_t n_pulses = N_PULSES;

#define NACC 1

struct sar-bp_vivado_access sar-bp_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.n_range = N_RANGE,
		.n_out = N_OUT,
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
		.devname = "sar-bp_vivado.0",
		.ioctl_req = SAR-BP_VIVADO_IOC_ACCESS,
		.esp_desc = &(sar-bp_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
