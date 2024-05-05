/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <asm/mach/map.h>
#include <asm/mach/arch.h>
#include <linux/memblock.h>
#include <linux/pstore_ram.h>
#include "board-dt.h"
#include "platsmp.h"

/* See fs/pstore/ram.c module paramaters for more details */
static struct ramoops_platform_data ramoops_data = {
        .mem_size               = 0x100000,   /*The memory size 1MB*/

        .mem_address            = 0x9ff00000, /*Last MB of physical memory*/

        .mem_type               = 1,          /*Try to use unbuffered memory */

        .record_size = 0x100000,              /*The same as mem_size so the log
                                                is not split into multiple
                                                pieces*/

        .dump_oops              = 1,          /*Save opps as well as panics*/
};

static struct platform_device ramoops_dev = {
        .name = "ramoops",
        .dev = {
                .platform_data = &ramoops_data,
        },
};

static const char *msm8909_dt_match[] __initconst = {
	"qcom,msm8909",
	"qcom,apq8009",
	NULL
};

static void __init msm8909_init(void)
{
	board_dt_populate(NULL);
	memblock_reserve(ramoops_data.mem_address, ramoops_data.mem_size);
	platform_device_register(&ramoops_dev);
}

DT_MACHINE_START(MSM8909_DT,
	"Qualcomm Technologies, Inc. MSM 8909 (Flattened Device Tree)")
	.init_machine	= msm8909_init,
	.dt_compat	= msm8909_dt_match,
	.smp	= &msm8909_smp_ops,
MACHINE_END
