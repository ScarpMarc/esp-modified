// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "sar-bp-naive_vivado.h"

#define DRV_NAME	"sar-bp-naive_vivado"

/* <<--regs-->> */
#define SAR-BP-NAIVE_N_RANGE_BINS_REG 0x48
#define SAR-BP-NAIVE_OUT_SIZE_REG 0x44
#define SAR-BP-NAIVE_N_PULSES_REG 0x40

struct sar-bp-naive_vivado_device {
	struct esp_device esp;
};

static struct esp_driver sar-bp-naive_driver;

static struct of_device_id sar-bp-naive_device_ids[] = {
	{
		.name = "SLD_SAR-BP-NAIVE_VIVADO",
	},
	{
		.name = "eb_77c",
	},
	{
		.compatible = "sld,sar-bp-naive_vivado",
	},
	{ },
};

static int sar-bp-naive_devs;

static inline struct sar-bp-naive_vivado_device *to_sar-bp-naive(struct esp_device *esp)
{
	return container_of(esp, struct sar-bp-naive_vivado_device, esp);
}

static void sar-bp-naive_prep_xfer(struct esp_device *esp, void *arg)
{
	struct sar-bp-naive_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->n_range_bins, esp->iomem + SAR-BP-NAIVE_N_RANGE_BINS_REG);
	iowrite32be(a->out_size, esp->iomem + SAR-BP-NAIVE_OUT_SIZE_REG);
	iowrite32be(a->n_pulses, esp->iomem + SAR-BP-NAIVE_N_PULSES_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool sar-bp-naive_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct sar-bp-naive_vivado_device *sar-bp-naive = to_sar-bp-naive(esp); */
	/* struct sar-bp-naive_vivado_access *a = arg; */

	return true;
}

static int sar-bp-naive_probe(struct platform_device *pdev)
{
	struct sar-bp-naive_vivado_device *sar-bp-naive;
	struct esp_device *esp;
	int rc;

	sar-bp-naive = kzalloc(sizeof(*sar-bp-naive), GFP_KERNEL);
	if (sar-bp-naive == NULL)
		return -ENOMEM;
	esp = &sar-bp-naive->esp;
	esp->module = THIS_MODULE;
	esp->number = sar-bp-naive_devs;
	esp->driver = &sar-bp-naive_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	sar-bp-naive_devs++;
	return 0;
 err:
	kfree(sar-bp-naive);
	return rc;
}

static int __exit sar-bp-naive_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct sar-bp-naive_vivado_device *sar-bp-naive = to_sar-bp-naive(esp);

	esp_device_unregister(esp);
	kfree(sar-bp-naive);
	return 0;
}

static struct esp_driver sar-bp-naive_driver = {
	.plat = {
		.probe		= sar-bp-naive_probe,
		.remove		= sar-bp-naive_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = sar-bp-naive_device_ids,
		},
	},
	.xfer_input_ok	= sar-bp-naive_xfer_input_ok,
	.prep_xfer	= sar-bp-naive_prep_xfer,
	.ioctl_cm	= SAR-BP-NAIVE_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct sar-bp-naive_vivado_access),
};

static int __init sar-bp-naive_init(void)
{
	return esp_driver_register(&sar-bp-naive_driver);
}

static void __exit sar-bp-naive_exit(void)
{
	esp_driver_unregister(&sar-bp-naive_driver);
}

module_init(sar-bp-naive_init)
module_exit(sar-bp-naive_exit)

MODULE_DEVICE_TABLE(of, sar-bp-naive_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sar-bp-naive_vivado driver");
