/* Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*/

/*!@file: DWC_ETH_QOS_plt_init.c
 * @brief: Driver functions.
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/pm_wakeup.h>
#include <linux/sched.h>
#include <linux/pm_qos.h>
#include <linux/pm_runtime.h>
#include <linux/esoc_client.h>
#include <linux/pinctrl/consumer.h>
#include "DWC_ETH_QOS_yheader.h"
#include <soc/qcom/boot_stats.h>

#ifdef CONFIG_PCI_MSM
#include <linux/msm_pcie.h>
#else
#include <mach/msm_pcie.h>
#endif

#define NTN_VREG_CORE_NAME 	"vdd-ntn-core"
#define NTN_VREG_PHY_NAME 	"vdd-ntn-phy"
#define NTN_VREG_HSIC_NAME 	"vdd-ntn-hsic"
#define NTN_VREG_PCI_NAME 	"vdd-ntn-pci"
#define NTN_VREG_IO_NAME 	"vdd-ntn-io"
#define NTN_VREG_CORE_MIN	1100000
#define NTN_VREG_CORE_MAX	1100000
#define NTN_VREG_PHY_MIN	2500000
#define NTN_VREG_PHY_MAX	2500000
#define NTN_VREG_HSIC_MIN	1200000
#define NTN_VREG_HSIC_MAX	1200000
#define NTN_VREG_PCI_MIN	1800000
#define NTN_VREG_PCI_MAX	1800000
#define NTN_VREG_IO_MIN		1800000
#define NTN_VREG_IO_MAX		1800000
#define NTN_RESET_GPIO_NAME	"ntn-rst-gpio"
#define NTN_PWR_GPIO_NAME	"ntn-supply-enable-gpio"

extern int DWC_ETH_QOS_init_module(void);
extern void DWC_ETH_QOS_exit_module(void);

struct list_head ntn_plt_data_list;

static int setup_regulator_common(struct device *dev, const char *name_supply,
				  const char *name, struct regulator **vreg,
				  int min_uV, int max_uV)
{
	int ret = 0;

	if (of_get_property(dev->of_node, name_supply, NULL)) {
		*vreg = regulator_get(dev, name);
		if (IS_ERR(*vreg)) {
			ret = PTR_ERR(*vreg);

			if (ret == -EPROBE_DEFER)
				NMSGPR_ALERT("%s: %s probe deferred!\n", __func__, name);
			else
				NMSGPR_ALERT("%s: Get %s failed!\n", __func__, name);
			return -1;
		} else {
			ret = regulator_set_voltage(*vreg, min_uV, max_uV);
			if (ret) {
				NMSGPR_ALERT("%s: Set %s failed!\n", __func__, name);
				return -1;
			} else {
				ret = regulator_enable(*vreg);
				if (ret) {
					NMSGPR_ALERT("%s: Enable %s failed!\n", __func__, name);
					return -1;
				}
			}
		}
	} else {
		NMSGPR_ALERT("%s: power supply %s not found!\n", __func__, name);
	}

	return 0;
}

static int setup_gpio_common(struct device *dev, const char *name, int *gpio,
			     int direction, int value)
{
	int ret = 0;

	if (of_find_property(dev->of_node, name, NULL)) {
		*gpio = ret = of_get_named_gpio(dev->of_node, name, 0);
		if (ret >= 0) {
			ret = gpio_request(*gpio, name);
			if (ret) {
				NMSGPR_ALERT("%s: Can't get GPIO %s, ret = %d\n", __func__, name, *gpio);
				*gpio = -1;
				return -1;
			}

			ret = gpio_direction_output(*gpio, direction);
			if (ret) {
				NMSGPR_ALERT("%s: Can't set GPIO %s direction, ret = %d\n", __func__, name, ret);
				return -1;
			}

			gpio_set_value(*gpio, value);
		} else {
			if (ret == -EPROBE_DEFER)
				NMSGPR_ALERT("get NTN_PWR_GPIO_NAME probe defer\n");
			else
				NMSGPR_ALERT("can't get gpio %s ret %d", name, ret);
			return -1;
		}
	} else {
		NMSGPR_ALERT("can't find gpio %s", name);
	}

	return 0;
}

/*!
* \brief Initialize and turn on regulators
*
* \details Parse the device tree for regulator nodes and
* turn them on. Missing nodes are non fatal.
*
* \param[in] dev - struct device from PCI device
*
* \return 0 on success. Negative error code on failure
*/
static int DWC_ETH_QOS_init_regulators(struct device *dev, struct DWC_ETH_QOS_plt_data *plt_data)
{
	int ret = 0;

	if (!dev)
		return -EINVAL;

	/* Configure power rails Neutrino*/
	ret = setup_regulator_common(dev, NTN_VREG_HSIC_NAME"-supply", NTN_VREG_HSIC_NAME,
				     &plt_data->ntn_reg_hsic, NTN_VREG_HSIC_MIN, NTN_VREG_HSIC_MAX);
	if (ret)
		goto exit_error;

	ret = setup_regulator_common(dev, NTN_VREG_PCI_NAME"-supply", NTN_VREG_PCI_NAME,
				     &plt_data->ntn_reg_pci, NTN_VREG_PCI_MIN, NTN_VREG_PCI_MAX);
	if (ret)
		goto exit_error;

	ret = setup_regulator_common(dev, NTN_VREG_IO_NAME"-supply", NTN_VREG_IO_NAME,
				     &plt_data->ntn_reg_io, NTN_VREG_IO_MIN, NTN_VREG_IO_MAX);
	if (ret)
		goto exit_error;

	ret = setup_regulator_common(dev, NTN_VREG_CORE_NAME"-supply", NTN_VREG_CORE_NAME,
				     &plt_data->ntn_reg_core, NTN_VREG_CORE_MIN, NTN_VREG_CORE_MAX);
	if (ret)
		goto exit_error;

	ret = setup_regulator_common(dev, NTN_VREG_PHY_NAME"-supply", NTN_VREG_PHY_NAME,
				     &plt_data->ntn_reg_phy, NTN_VREG_PHY_MIN, NTN_VREG_PHY_MAX);
	if (ret)
		goto exit_error;

	return ret;

exit_error:
	if (plt_data->ntn_reg_phy)
		regulator_put(plt_data->ntn_reg_phy);
	if (plt_data->ntn_reg_core)
		regulator_put(plt_data->ntn_reg_core);
	if (plt_data->ntn_reg_io)
		regulator_put(plt_data->ntn_reg_io);
	if (plt_data->ntn_reg_pci)
		regulator_put(plt_data->ntn_reg_pci);
	if (plt_data->ntn_reg_hsic)
		regulator_put(plt_data->ntn_reg_hsic);
	return ret;
}

/*!
* \brief Initialize and configure GPIOs
*
* \details Parse the device tree for GPIO nodes controlling
* power supplies and Neutrino reset. Missing nodes are non fatal
* because some platforms like IVI don't use GPIOs for power control.
*
* \param[in] dev - struct device from PCI device
*
* \return 0 on success. Negative error code on failure
*/
static int DWC_ETH_QOS_init_gpios(struct device *dev, struct DWC_ETH_QOS_plt_data *plt_data)
{
	int ret = 0;

	if (!dev)
		return -EINVAL;

	plt_data->supply_gpio_num = -1;
	plt_data->resx_gpio_num = -1;

	/* Configure GPIOs required for GPIO regulator control */
	ret = setup_gpio_common(dev, NTN_PWR_GPIO_NAME, &plt_data->supply_gpio_num, 0x1, 0x1);
	if (ret)
		goto exit_error;

	/* Configure GPIOs required for the Neutrino reset */
	ret = setup_gpio_common(dev, NTN_RESET_GPIO_NAME, &plt_data->resx_gpio_num, 0x1, 0x0);
	if (ret) {
		NMSGPR_ALERT("%s: Failed to toggle OFF %s\n", __func__,NTN_RESET_GPIO_NAME);
		goto exit_error;
	}
	if (gpio_is_valid(plt_data->resx_gpio_num)) {
		msleep(10);
		gpio_set_value(plt_data->resx_gpio_num, 0x1);
	}
	return ret;

exit_error:
	if (gpio_is_valid(plt_data->resx_gpio_num))
		gpio_free(plt_data->resx_gpio_num);
	plt_data->resx_gpio_num = -1;
	if (gpio_is_valid(plt_data->supply_gpio_num))
		gpio_free(plt_data->supply_gpio_num);
	plt_data->supply_gpio_num = -1;
	return ret;
}

static void DWC_ETH_QOS_free_regs_and_gpios(struct DWC_ETH_QOS_plt_data *plt_data)
{
	if (gpio_is_valid(plt_data->resx_gpio_num)) {
		/* Keep NTN in reset while the driver is not loaded */
		gpio_set_value(plt_data->resx_gpio_num, 0x0);
		gpio_free(plt_data->resx_gpio_num);
	}

	if (gpio_is_valid(plt_data->supply_gpio_num)) {
		/* Power off Neutrino when it is not in use */
		gpio_set_value(plt_data->supply_gpio_num, 0x0);
		gpio_free(plt_data->supply_gpio_num);
	}

	if (plt_data->ntn_reg_phy)
		regulator_put(plt_data->ntn_reg_phy);

	if (plt_data->ntn_reg_core)
		regulator_put(plt_data->ntn_reg_core);

	if (plt_data->ntn_reg_io)
		regulator_put(plt_data->ntn_reg_io);

	if (plt_data->ntn_reg_pci)
		regulator_put(plt_data->ntn_reg_pci);

	if (plt_data->ntn_reg_hsic)
		regulator_put(plt_data->ntn_reg_hsic);
}

static int DWC_ETH_QOS_pcie_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct DWC_ETH_QOS_plt_data *plt_data;

	DBGPR("Entry: %s\n", __func__);

	plt_data = devm_kzalloc(&pdev->dev, sizeof(*plt_data), GFP_KERNEL);
	if (!plt_data)
		return -ENOMEM;

	plt_data->pldev = pdev;

#ifndef DWC_ETH_QOS_BUILTIN
	/* Neutrino board init */
	ret = DWC_ETH_QOS_init_regulators(dev, plt_data);
	if (ret) {
		NMSGPR_ALERT("%s: Failed to init regulators\n", __func__);
		goto DWC_ETH_QOS_err_pcie_probe;
	}

	ret = DWC_ETH_QOS_init_gpios(dev, plt_data);
	if (ret) {
		NMSGPR_ALERT("%s: Failed to init GPIOs\n", __func__);
		goto DWC_ETH_QOS_err_pcie_probe;
	}

	/* add delay for neutrino reset propagation*/
	ret = of_property_read_u32(dev->of_node, "qcom,ntn-rst-delay-msec", &plt_data->ntn_rst_delay);
	if (ret) {
		NMSGPR_ALERT("%s: Failed to find ntn_rst_delay value, hardcoding to 100 msec\n", __func__);
		plt_data->ntn_rst_delay = 500;
	} else {
		NDBGPR_L1("%s: ntn_rst_delay = %d msec\n", __func__, plt_data->ntn_rst_delay);
	}
	msleep(plt_data->ntn_rst_delay);
#endif

	/* read bus number */
	ret = of_property_read_u32(dev->of_node, "qcom,ntn-bus-num", &plt_data->bus_num);
	if (ret) {
		plt_data->bus_num = -1;
		NMSGPR_ALERT( "%s: Failed to find PCIe bus number from device tree!\n", __func__);
	} else {
		NDBGPR_L1( "%s: Found PCIe bus number! %d\n", __func__, plt_data->bus_num);
	}

	/* issue PCIe enumeration */
	ret = of_property_read_u32(dev->of_node, "qcom,ntn-rc-num", &plt_data->rc_num);
	if (ret) {
		NMSGPR_ALERT( "%s: Failed to find PCIe RC number!, RC=%d\n", __func__, plt_data->rc_num);
		goto DWC_ETH_QOS_err_pcie_probe;
	} else {
		NDBGPR_L1( "%s: Found PCIe RC number! %d\n", __func__, plt_data->rc_num);
		ret = msm_pcie_enumerate(plt_data->rc_num);
		if (ret) {
			NMSGPR_ALERT( "%s: Failed to enable PCIe RC%x!\n", __func__, plt_data->rc_num);
			goto DWC_ETH_QOS_err_pcie_probe;
		} else {
			NDBGPR_L1( "%s: PCIe enumeration for RC number %d successful!\n", __func__, plt_data->rc_num);
		}
	}

	list_add_tail(&plt_data->node, &ntn_plt_data_list);

	dev_info(dev, "probe success");
	return ret;

DWC_ETH_QOS_err_pcie_probe:
#ifndef DWC_ETH_QOS_BUILTIN
	DWC_ETH_QOS_free_regs_and_gpios(plt_data);
#endif
	dev_info(dev, "probe failed");
	return ret;
}

static int DWC_ETH_QOS_pcie_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct DWC_ETH_QOS_plt_data *plt_data;
	DBGPR( "Entry:%s.\n", __func__);

#ifndef DWC_ETH_QOS_BUILTIN
	list_for_each_entry(plt_data, &ntn_plt_data_list, node) {
		if (plt_data->pldev == pdev) {
			DWC_ETH_QOS_free_regs_and_gpios(plt_data);
			break;
		}
	}
#endif

	DBGPR( "Exit:%s.\n", __func__);
	return ret;
}

static struct of_device_id DWC_ETH_QOS_pcie_match[] = {
	{	.compatible = "qcom,ntn_avb",
	},
	{}
};

static struct platform_driver DWC_ETH_QOS_pcie_driver = {
	.probe	= DWC_ETH_QOS_pcie_probe,
	.remove	= DWC_ETH_QOS_pcie_remove,
	.driver	= {
		.name		= "ntn_avb",
		.owner		= THIS_MODULE,
		.of_match_table	= DWC_ETH_QOS_pcie_match,
	},
};

static int __init DWC_ETH_QOS_pcie_init(void)
{
	int ret = 0;

	INIT_LIST_HEAD(&ntn_plt_data_list);
	DBGPR( "Entry:%s.\n", __func__);
	place_marker("M - DRIVER Ethernet Init");
	ret = platform_driver_register(&DWC_ETH_QOS_pcie_driver);

	if (ret == 0) {
		/* register Ethernet modules */
		ret = DWC_ETH_QOS_init_module();
	}
	place_marker("M - DRIVER Ethernet Ready");
	DBGPR( "Exit:%s.\n", __func__);
	return ret;
}

static void __exit DWC_ETH_QOS_pcie_exit(void)
{
	DBGPR( "Entry:%s.\n", __func__);
	platform_driver_unregister(&DWC_ETH_QOS_pcie_driver);
	DWC_ETH_QOS_exit_module();
	DBGPR( "Exit:%s.\n", __func__);
}

fs_initcall(DWC_ETH_QOS_pcie_init);
module_exit(DWC_ETH_QOS_pcie_exit);
