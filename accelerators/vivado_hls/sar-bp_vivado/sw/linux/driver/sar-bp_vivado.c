// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "sar-bp_vivado.h"

#define DRV_NAME	"sar-bp_vivado"

/* <<--regs-->> */
#define SAR-BP_N_RANGE_REG 0x48
#define SAR-BP_N_OUT_REG 0x44
#define SAR-BP_N_PULSES_REG 0x40

struct sar-bp_vivado_device {
	struct esp_device esp;
};

static struct esp_driver sar-bp_driver;

static struct of_device_id sar-bp_device_ids[] = {
	{
		.name = "SLD_SAR-BP_VIVADO",
	},
	{
		.name = "eb_04a",
	},
	{
		.compatible = "sld,sar-bp_vivado",
	},
	{ },
};

static int sar-bp_devs;

static inline struct sar-bp_vivado_device *to_sar-bp(struct esp_device *esp)
{
	return container_of(esp, struct sar-bp_vivado_device, esp);
}

static void sar-bp_prep_xfer(struct esp_device *esp, void *arg)
{
	struct sar-bp_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->n_range, esp->iomem + SAR-BP_N_RANGE_REG);
	iowrite32be(a->n_out, esp->iomem + SAR-BP_N_OUT_REG);
	iowrite32be(a->n_pulses, esp->iomem + SAR-BP_N_PULSES_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool sar-bp_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct sar-bp_vivado_device *sar-bp = to_sar-bp(esp); */
	/* struct sar-bp_vivado_access *a = arg; */

	return true;
}

static int sar-bp_probe(struct platform_device *pdev)
{
	struct sar-bp_vivado_device *sar-bp;
	struct esp_device *esp;
	int rc;

	sar-bp = kzalloc(sizeof(*sar-bp), GFP_KERNEL);
	if (sar-bp == NULL)
		return -ENOMEM;
	esp = &sar-bp->esp;
	esp->module = THIS_MODULE;
	esp->number = sar-bp_devs;
	esp->driver = &sar-bp_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	sar-bp_devs++;
	return 0;
 err:
	kfree(sar-bp);
	return rc;
}

static int __exit sar-bp_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct sar-bp_vivado_device *sar-bp = to_sar-bp(esp);

	esp_device_unregister(esp);
	kfree(sar-bp);
	return 0;
}

static struct esp_driver sar-bp_driver = {
	.plat = {
		.probe		= sar-bp_probe,
		.remove		= sar-bp_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = sar-bp_device_ids,
		},
	},
	.xfer_input_ok	= sar-bp_xfer_input_ok,
	.prep_xfer	= sar-bp_prep_xfer,
	.ioctl_cm	= SAR-BP_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct sar-bp_vivado_access),
};

static int __init sar-bp_init(void)
{
	return esp_driver_register(&sar-bp_driver);
}

static void __exit sar-bp_exit(void)
{
	esp_driver_unregister(&sar-bp_driver);
}

module_init(sar-bp_init)
module_exit(sar-bp_exit)

MODULE_DEVICE_TABLE(of, sar-bp_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sar-bp_vivado driver");
