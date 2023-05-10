// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _MAC_VIVADO_H_
#define _MAC_VIVADO_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#ifndef __user
#define __user
#endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

struct mac_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned mac_n;
	unsigned mac_len;
	unsigned mac_vec;
	unsigned src_offset;
	unsigned dst_offset;
};

#define MAC_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct mac_vivado_access)

#endif /* _MAC_VIVADO_H_ */
