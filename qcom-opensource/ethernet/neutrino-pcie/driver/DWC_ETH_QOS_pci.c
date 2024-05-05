/* ============================================================================
* COPYRIGHT © 2015
*
* Toshiba America Electronic Components
*
* PROJECT:   NEUTRINO
*
* Permission is hereby granted,
* free of charge, to any person obtaining a copy of this software annotated
* with this license and the Software, to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so, subject
* to the following conditions:
*
*
* EXAMPLE PROGRAMS ARE PROVIDED AS-IS WITH NO WARRANTY OF ANY KIND,
* EITHER EXPRESS OR IMPLIED.
*
* TOSHIBA ASSUMES NO LIABILITY FOR CUSTOMERS' PRODUCT DESIGN OR APPLICATIONS.
*
* THIS SOFTWARE IS PROVIDED AS-IS AND HAS NOT BEEN FULLY TESTED.  IT IS
* INTENDED FOR REFERENCE USE ONLY.
*
* TOSHIBA DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES AND ALL LIABILITY OR
* ANY DAMAGES ASSOCIATED WITH YOUR USE OF THIS SOFTWARE.
*
* THIS SOFTWARE IS BEING DISTRIBUTED BY TOSHIBA SOLELY ON AN "AS IS" BASIS
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE HEREBY DISCLAIMED. IN NO EVENT SHALL TOSHIBA BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
* ========================================================================= */

/* =========================================================================
* The Synopsys DWC ETHER QOS Software Driver and documentation (hereinafter
* "Software") is an unsupported proprietary work of Synopsys, Inc. unless
* otherwise expressly agreed to in writing between Synopsys and you.
*
* The Software IS NOT an item of Licensed Software or Licensed Product under
* any End User Software License Agreement or Agreement for Licensed Product
* with Synopsys or any supplement thereto.  Permission is hereby granted,
* free of charge, to any person obtaining a copy of this software annotated
* with this license and the Software, to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so, subject
* to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
* ========================================================================= */

/*! History:
 *      2-March-2016 : Initial
 *     21-March-2016 : Modified "pdev->msi_cap + PCI_MSI_MASK_64" register
 *                     read as "pci_write_config_dword"
 *      3-Jun-2016   : Removed Extra TX/RX Queue/DMA Channel initialization.
 */

/*!@file: DWC_ETH_QOS_pci.c
 * @brief: Driver functions.
 */
#include <linux/firmware.h>
#ifdef CONFIG_PCI_MSM
#include <linux/msm_pcie.h>
#endif
#include <linux/msm-bus.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_yregacc.h"
#include "DWC_ETH_QOS_pci.h"
#include "DWC_ETH_QOS_ipa.h"

struct DWC_ETH_QOS_prv_data *gDWC_ETH_QOS_prv_data;

extern struct list_head ntn_plt_data_list;

static UCHAR dev_addr[6] = { 0xE8, 0xE0, 0xB7, 0xB5, 0x7D, 0xF8};
typedef struct
{
	char mdio_key[32];
	unsigned short mdio_key_len;
	char mac_key[32];
	unsigned short mac_key_len;
	unsigned short mac_str_len;
	char mac_str_def[20];
} config_param_list_t;

static const config_param_list_t config_param_list[] = {
{"MDIOBUSID",9,"MAC_ID", 6, 17, "00:00:00:00:00:00"},
};

#define CONFIG_PARAM_NUM (sizeof(config_param_list)/sizeof(config_param_list[0]))
/* Holds virtual address for BAR0 for register access,
   BAR1 for SRAM memory and BAR2 for Flash memory */
#if !defined(NTN_DECLARE_MEM_FOR_DMAAPI) && !defined(DESC_HOSTMEM_BUF_HOSTMEM)
ULONG cur_virt;
#endif

CHARP fw_filename = "";

module_param(fw_filename, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(fw_filename, "Firmware filename");

static int rmii_mode = 0;
module_param(rmii_mode, int, 0644);
MODULE_PARM_DESC(rmii_mode, "Force RMII mode");

int DWC_ETH_QOS_request_bus_bandwidth(struct DWC_ETH_QOS_prv_data *pdata, int bandwidth)
{
	int ret = 0;

	if (!pdata)
		return -ENODEV;

	if (!pdata->bus_client)
		return -ENOSYS;

	switch (bandwidth) {
	case NTN_BUS_WIDTH_NONE:
	case NTN_BUS_WIDTH_HIGH:
		ret = msm_bus_scale_client_update_request(
				pdata->bus_client, bandwidth);
		if (!ret) {
			pdata->current_bandwidth_vote = bandwidth;
		} else {
			pr_err("%s: could not set bus bandwidth %d, ret = %d\n",
			       __func__, bandwidth, ret);
		}
		break;

	default:
		pr_err("%s: Invalid request %d", __func__, bandwidth);
		ret = -EINVAL;
	}
	return ret;
}

void DWC_ETH_QOS_init_all_fptrs(struct DWC_ETH_QOS_prv_data *pdata)
{
	DWC_ETH_QOS_init_function_ptrs_dev(&pdata->hw_if);
	DWC_ETH_QOS_init_function_ptrs_desc(&pdata->desc_if);
}

/*!
 * \brief API to kernel read from file
 *
 * \param[in] file   - pointer to file descriptor
 * \param[in] offset - Offset of file to start read
 * \param[in] size   - Size of the buffer to be read
 *
 * \param[out] data   - File data buffer
 *
 *
 * \return integer
 *
 * \retval 0 on success & -ve number on failure.
 */
static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

/*!
 * \brief API to validate MAC ID
 *
 * \param[in] pdev - pointer to MAC ID string
 *
 * \return boolean
 *
 * \retval true on success and false on failure.
 */
bool isMAC(char *s) {
    int i=0;
    if (s == NULL)
	return false;

    for(i = 0; i < 17; i++) {
        if(i % 3 != 2 && !isxdigit(s[i]))
            return false;
        if(i % 3 == 2 && s[i] != ':')
            return false;
    }
    return true;
}

/*!
 * \brief API to extract MAC ID from given string
 *
 * \param[in] pdev - pointer to MAC ID string
 *
 * \return None
 */
void extract_macid(char *string)
{
	char *token_m = NULL;
	int j = 0;
        int mac_id = 0;

	/* Extract MAC ID byte by byte */
	token_m = strsep(&string, ":");
	while(token_m != NULL) {
		sscanf(token_m, "%x", &mac_id);
		dev_addr[j++] = mac_id;
		token_m = strsep(&string, ":");
	}
}

/*!
 * \brief API to parse and extract the user configured MAC ID
 *
 * \param[in] file_buf - Pointer to file data buffer
 *
 * \return boolean
 *
 * \return - True on Success and False in failure
 */
static bool lookfor_macid(char *file_buf, USHORT mdio_bus_id)
{
	char *string = NULL, *token_n = NULL, *token_s = NULL, *token_m = NULL;
	bool status = false;
	int ntn_device_no= 0;

	string = file_buf;
	/* Parse Line-0 */
	token_n = strsep(&string, "\n");
	while (token_n != NULL) {

		/* Check if line is enabled */
		if (token_n[0] != '#') {
			/* Extract the token based space character */
			token_s = strsep(&token_n, " ");
			if (token_s != NULL) {
			if (strncmp (token_s, config_param_list[0].mdio_key, 9) == 0 ) {
					token_s = strsep(&token_n, " ");
					token_m = strsep(&token_s, ":");
					sscanf(token_m, "%d", &ntn_device_no);
					if (ntn_device_no != mdio_bus_id){
						if ((token_n = strsep(&string, "\n")) == NULL)
							break;
						continue;
					}
				}
			}

			/* Extract the token based space character */
			token_s = strsep(&token_n, " ");
			if (token_s != NULL) {
				/* Compare if parsed string matches with key listed in configuration table */
				if (strncmp (token_s, config_param_list[0].mac_key, 6) == 0 ) {

					NDBGPR_L1("MAC_ID Key is found\n");
					/* Read next word */
					token_s = strsep(&token_n, " \n");
					if (token_s != NULL) {

						/* Check if MAC ID length  and MAC ID is valid */
						if ((isMAC(token_s) == true) && (strlen(token_s) ==  config_param_list[0].mac_str_len)) {
							/* If user configured MAC ID is valid,  assign default MAC ID */
							extract_macid(token_s);
							status = true;
						} else {
						    NMSGPR_ALERT( "Mac: isMAC(token_s) = %d\n",isMAC(token_s));
							NMSGPR_ALERT( "Mac: strlen(token_s) = %d\n",(int)strlen(token_s));
							NMSGPR_ALERT( "Mac: config_param_list[0].mac_str_len = %d\n",config_param_list[0].mac_str_len);
							NMSGPR_ALERT( "Valid Mac ID not found\n");
						}
					}
				}
			}
		}
		/* Read next lile */
                if ((token_n = strsep(&string, "\n")) == NULL)
			break;

	}
	return status;
}

/*!
 * \brief Parse the user configuration file for various config
 *
 * \param[in] None
 *
 * \return None
 *
 */
static void parse_config_file(USHORT mdio_bus_id)
{
	struct file *filep = NULL;
	char *data = kmalloc(512, GFP_KERNEL);
	mm_segment_t oldfs;
	int ret, flags = O_RDONLY, i = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filep = filp_open("/etc/ntn_config.ini", flags, 0600);
	/* Try in /data */
	if (IS_ERR(filep))
		filep = filp_open("/data/ntn_config.ini", flags, 0600);
	set_fs(oldfs);
	if(IS_ERR(filep)) {
		NMSGPR_ALERT( "Mac configuration file not found\n");
		NMSGPR_ALERT( "Using Default MAC Address\n");
		return;
	}
	else  {
		/* Parse the file */
		ret = file_read(filep, 0, data, 512);
		for (i = 0; i < CONFIG_PARAM_NUM; i++) {
			if (strstr ((const char *)data, config_param_list[i].mdio_key)) {
				NDBGPR_L1("Pattern Match\n");
				if (strncmp(config_param_list[i].mdio_key, "MDIOBUSID", 9) == 0) {
					/* MAC ID Configuration */
					NDBGPR_L1("MAC_ID Configuration\n");
					if (lookfor_macid(data, mdio_bus_id) == false) {
						//extract_macid ((char *)config_param_list[i].str_def);
					}
		}
			}
		}
	}

	kfree(data);
	filp_close(filep, NULL);

	return;
}

/*!
* \brief Load the firmware to the SRAM for the Neutrino.
*
* \details This function reads the firmware from a binary file and writes
* the contents of the file to the SRAM for the Neutrino. It then resets the
* Neutrino.
*
* \param[in] pdev - pointer to pci_dev structure.
* \param[in] dwc_eth_ntn_reg_pci_base_addr - PCI register base address
* \param[in] dwc_eth_ntn_SRAM_pci_base_addr_virt - Neutrino SRAM base address
*
* \return integer
*
* \retval 0 on success & -ve number on failure.
*/
static int DWC_ETH_QOS_load_fw(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	const struct firmware *fw_entry = NULL;
	unsigned int reg_val;

        if (fw_filename[0] == '\0') {
                NMSGPR_ALERT("Firmware filename not given.\n");
                return -EINVAL;
        }

	if (request_firmware(&fw_entry, fw_filename, &pdev->dev) != 0 ||
	    fw_entry == NULL)
	{
		NMSGPR_ALERT("Failed to get %s.\n",
			fw_filename);
		return -EINVAL;
	}

	/* Set the reset bit */
	reg_val = *(unsigned int*)(pdata->dev->base_addr + 0x1008);
	*(unsigned int*)(pdata->dev->base_addr + 0x1008) =
		(reg_val | 1);
	msleep(1);
	/* Write the FW to SRAM */
	memcpy((char*)pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt, fw_entry->data,
	       fw_entry->size);
	msleep(1);
	/* Clear the reset bit to restart the Neutrino */
	*(unsigned int*)(pdata->dev->base_addr + 0x1008) =
		(reg_val & ~0x3);
	reg_val = *(unsigned int*)(pdata->dev->base_addr + 0x1008);

	release_firmware(fw_entry);

	/* Wait a moment for the Neutrino to boot */
	msleep(pdata->fw_load_delay);

	NMSGPR_INFO("Firmware loaded\n");

	return 0;
}

static int DWC_ETH_QOS_panic_notifier(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	if (gDWC_ETH_QOS_prv_data) {
		DBGPR("DWC_ETH_QOS: 0x%pK\n", gDWC_ETH_QOS_prv_data);
		DWC_ETH_QOS_ipa_stats_read(gDWC_ETH_QOS_prv_data);
	}
	return NOTIFY_DONE;
}

static struct notifier_block DWC_ETH_QOS_panic_blk = {
	.notifier_call  = DWC_ETH_QOS_panic_notifier,
};

#ifdef CONFIG_PCI_MSM

/*!
* \brief Handle the PCI link down event
*
* \details Worker function that stops the phy and brings down the
* ethernet interface to avoid system. Dynamic recovery of the PCI link
* is not supported yet. Call using schedule_work() API.
*
* \param[in] recovert - work_struct. Must be a child of DWC_ETH_QOS_prv_data
*
* \return None
*/
static void DWC_ETH_QOS_pci_recovery_work_handler(struct work_struct *recovery)
{
	struct DWC_ETH_QOS_prv_data *pdata =
		container_of(recovery, struct DWC_ETH_QOS_prv_data, pci_recovery_work);
	if (!pdata) {
		NMSGPR_ALERT("%s: pdata is NULL. Failed to recover PCI link\n", __func__);
		return;
	}

	/* Stop phy interrutps and bring the ethernet interface down.
	   This avoids PCI writes which timeout since the link is down
	   and cause the system to hang */
	NMSGPR_ALERT("%s: Stopping phy and detaching net device\n", __func__);
	pdata->phydev->state = PHY_DOWN;
	netif_device_detach(pdata->dev);
}

/*!
* \brief Callback function for PCI events.
*
* \details This callback function receives PCI link status changes registered
* from msm_pcie_register_event(). During a PCI link down event this fuction
* stores the error state in the pci_dev which persists in the kernel after a
* driver unload. The driver will attemtp to re-enumerate the device on the next
* module load.
*
* \param[in] notify - pointer to msm_pcie_notify containing PCI status
*
* \return None
*/
static void DWC_ETH_QOS_pci_event(struct msm_pcie_notify *notify)
{
	struct DWC_ETH_QOS_prv_data *pdata;
	struct pci_dev *pdev;

	if (!notify || !notify->data) {
		NMSGPR_ALERT("%s: notify is NULL. Failed to handle PCI event\n",
					 __func__);
		return;
	}
	pdata = notify->data;
	pdev = pdata->pdev;

	switch (notify->event) {
		case MSM_PCIE_EVENT_LINKDOWN:
			NMSGPR_ALERT("%s: PCI link down\n", __func__);
			schedule_work(&pdata->pci_recovery_work);
			break;
	default:
		NMSGPR_ALERT("cnss: invalid event from PCIe callback %d\n",
					 notify->event);
	}
}

/*!
* \brief Resume PCI link on module init
*
* \details Recover the PCI link when the module is initialized. PCI link could
* be in suspend state if module was removed while system was running.
*
* \param[in] pdev - the PCIe device
*
* \return integer
*
* \retval 0 on success & -ve number on failure.
*/
static int DWC_ETH_QOS_pci_resume(struct pci_dev *pdev)
{
	int ret = 0;
	int pm_opts = MSM_PCIE_CONFIG_NO_CFG_RESTORE;

	/* Resume PCI device to allow device */
	ret = msm_pcie_pm_control(
			MSM_PCIE_RESUME,
			pdev->bus->number,
			pdev, NULL, pm_opts);

	if (ret) {
		NMSGPR_ALERT("%s: Failed to resume PCIe link\n", __func__);
		goto exit;
	}

	ret = msm_pcie_recover_config(pdev);
	if (ret) {
		NMSGPR_ALERT("%s: Failed to restore PCI config\n", __func__);
		goto exit;
	}


exit:
	return ret;
}

/*!
* \brief Register for PCI link status change events
*
* \details The MSM PCI driver sends link status change events. Register for
* these events so that the link can be recovered in case it fails or when it
* has to be resumed.
*
* \param[in] pdev - the PCIe device
*
* \return None
*/
static void DWC_ETH_QOS_pci_event_register(struct pci_dev *pdev)
{
	int ret;
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	/* Setup PCI link recovery handler */
	INIT_WORK(&pdata->pci_recovery_work, DWC_ETH_QOS_pci_recovery_work_handler);
	pdata->msm_pci_event.events = MSM_PCIE_EVENT_LINKDOWN;

	/* Register PCI status changes.
	'user' must be of type struct pci_dev */
	pdata->msm_pci_event.user = pdev;
	pdata->msm_pci_event.notify.data = pdata;
	pdata->msm_pci_event.mode = MSM_PCIE_TRIGGER_CALLBACK;
	pdata->msm_pci_event.callback = DWC_ETH_QOS_pci_event;
	pdata->msm_pci_event.options = MSM_PCIE_CONFIG_NO_RECOVERY;
	ret = msm_pcie_register_event(&pdata->msm_pci_event);
	if (ret) {
		NMSGPR_ALERT("%s: PCIe event register failed! %d\n", __func__, ret);
	}
}

/*!
* \brief Deregister for PCI link status change events
*
* \param[in] pdev - the PCIe device
*
* \return None
*/
static void DWC_ETH_QOS_pci_event_deregister(struct pci_dev *pdev)
{
	int ret;
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	ret = msm_pcie_deregister_event(&pdata->msm_pci_event);
	if (ret) {
		NMSGPR_ALERT("%s: PCIe event deregister failed! %d\n", __func__, ret);
	}
}

/*!
* \brief Save the PCI link status and suspend it on module exit
*
* \details Save the state of the PCI link so that it can be recovered when the
* module is loaded again and suspend the bus power.
*
* \param[in] pdev - the PCIe device
*
* \return None
*/
static void DWC_ETH_QOS_pci_suspend(struct pci_dev *pdev)
{
	int pm_opts = (MSM_PCIE_CONFIG_NO_CFG_RESTORE | MSM_PCIE_CONFIG_LINKDOWN);

	/* Suspend PCI device to allow device power off */
	if (msm_pcie_pm_control(MSM_PCIE_SUSPEND, pdev->bus->number,
				pdev, NULL, pm_opts)) {
		NMSGPR_ALERT("%s: Failed to shutdown PCIe link\n", __func__);
	}
}

#else /* !CONFIG_PCI_MSM */

static int DWC_ETH_QOS_pci_resume(struct pci_dev *pdev)
{
	return 0;
}

static void DWC_ETH_QOS_pci_event_register(struct pci_dev *pdev)
{
	return;
}

static void DWC_ETH_QOS_pci_event_deregister(struct pci_dev *pdev)
{
	return 0;
}

static void DWC_ETH_QOS_pci_suspend(struct pci_dev *pdev)
{
	return;
}

#endif /* CONFIG_PCI_MSM */

#ifdef NTN_ENABLE_PCIE_MEM_ACCESS
static void DWC_ETH_QOS_config_tamap(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	ULONG_LONG adrs_to_be_replaced, adrs_for_replacement;
	UINT tmap_no, no_of_bits;

	/* Configure TMAP 0 to access full range of host memory */
	tmap_no = 0;
	adrs_to_be_replaced = ( (unsigned long long)0x00000010 << 32);
	adrs_for_replacement = ( (unsigned long long)0x00000000 << 32);
	no_of_bits = 28;
	hw_if->ntn_config_tamap(tmap_no, adrs_to_be_replaced, adrs_for_replacement, no_of_bits, pdata);
}

static void DWC_ETH_QOS_config_ipareg_map(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	ULONG_LONG adrs_to_be_replaced, adrs_for_replacement;
	UINT tmap_no, no_of_bits;

	/* Configure TMAP 1 to access IPA uC Register in host memory */
	tmap_no = 1;
	adrs_to_be_replaced = NTN_PCIE_REGION_MEM_MAP_BASE;
	if(pdata->prv_ipa.ipa_ver >= IPA_HW_v3_0)
		adrs_for_replacement = IPA3_UC_REG_BASE;
	else
		adrs_for_replacement = IPA2_UC_REG_BASE;
	NTN_GET_TAMAP_MASK_BITS(NTN_HOST_TAMAP_MEM_LENGTH, no_of_bits);
	NDBGPR_L1("TAMAP %d, adrs_to_be_replaced %llx adrs_for_replacement "
		"%llx no_of_bits %d \n",tmap_no, adrs_to_be_replaced,
		adrs_for_replacement, no_of_bits);
	hw_if->ntn_config_tamap(tmap_no, adrs_to_be_replaced,
				adrs_for_replacement, no_of_bits, pdata);
}
#endif

/*!
* \brief API to initialize the device.
*
* \details This probing function gets called (during execution of
* pci_register_driver() for already existing devices or later if a
* new device gets inserted) for all PCI devices which match the ID table
* and are not "owned" by the other drivers yet. This function gets passed
* a "struct pci_dev *" for each device whose entry in the ID table matches
* the device. The probe function returns zero when the driver chooses to take
* "ownership" of the device or an error code (negative number) otherwise.
* The probe function always gets called from process context, so it can sleep.
*
* \param[in] pdev - pointer to pci_dev structure.
* \param[in] id   - pointer to table of device ID/ID's the driver is inerested.
*
* \return integer
*
* \retval 0 on success & -ve number on failure.
*/

static int DWC_ETH_QOS_probe(struct pci_dev *pdev,
				const struct pci_device_id *id)
{

	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct net_device *dev = NULL;
	struct platform_device *pldev = NULL;
	struct DWC_ETH_QOS_plt_data *plt_data = NULL;
	int i, ret = 0;
	struct hw_if_struct *hw_if = NULL;
	struct desc_if_struct *desc_if = NULL;
	UCHAR tx_q_count = 0, rx_q_count = 0;
	unsigned int reg_val;
	int domain;
	ULONG dwc_eth_ntn_reg_pci_base_addr_phy;
	ULONG dwc_eth_ntn_reg_pci_base_addr;
	ULONG dwc_eth_ntn_FLASH_pci_base_addr;
	ULONG dwc_eth_ntn_SRAM_pci_base_addr_virt;
	ULONG dwc_eth_ntn_SRAM_pci_base_addr_phy;
	ULONG dwc_eth_ntn_reg_len;
	ULONG dwc_eth_ntn_SRAM_len;
	ULONG dwc_eth_ntn_FLASH_len;
	static USHORT mdio_bus_id = 0;
	struct msm_bus_scale_pdata *ntn_bus_pdata = NULL;
#ifdef NTN_DECLARE_MEM_FOR_DMAAPI
	ULONG phy_mem_adrs;
#endif

	NDBGPR_L1("Debug version\n");

	DBGPR("--> DWC_ETH_QOS_probe\n");


	/* Resume PCI link and restore the state */
	ret = DWC_ETH_QOS_pci_resume(pdev);
	if (ret) {
		goto err_pci_restore_failed;
	}

	ret = pci_enable_device(pdev);
	if (ret) {
		NMSGPR_ALERT( "%s:Unable to enable device\n", DEV_NAME);
		goto err_out_enb_failed;
	}

	/* Query and set the appropriate masks for DMA operations. */
	if ( (ret = pci_set_dma_mask (pdev, DMA_BIT_MASK(64))) &&
		(ret = pci_set_consistent_dma_mask (pdev, DMA_BIT_MASK(64)))) {
		NMSGPR_ALERT(
			"%s: 64 bits DMA Configuration not supported, aborting\n",
			pci_name(pdev));
		goto err_out_dma_mask_failed;
	}

	if (pci_request_regions(pdev, DEV_NAME)) {
		NMSGPR_ALERT( "%s:Failed to get PCI regions\n", DEV_NAME);
		ret = -ENODEV;
		goto err_out_req_reg_failed;
	}
	pci_set_master(pdev);

	/* Read BAR0 and map the Neutrino register base address
	Read BAR1 and map the Neutrino SRAM memory address
	Read BAR2 and map the Neutrino Flash memory address */
	dwc_eth_ntn_reg_len = pci_resource_len(pdev, 0);
	dwc_eth_ntn_SRAM_len = pci_resource_len(pdev, 2);
	dwc_eth_ntn_FLASH_len = pci_resource_len(pdev, 4);

	NDBGPR_L1( "BAR0 length = %ld kb\n", dwc_eth_ntn_reg_len);
	NDBGPR_L1( "BAR2 length = %ld kb\n", dwc_eth_ntn_SRAM_len);
	NDBGPR_L1( "BAR4 length = %ld kb\n", dwc_eth_ntn_FLASH_len);

	dwc_eth_ntn_reg_pci_base_addr_phy = pci_resource_start(pdev, 0);
	dwc_eth_ntn_SRAM_pci_base_addr_phy = pci_resource_start(pdev, 2);
	dwc_eth_ntn_FLASH_pci_base_addr = pci_resource_start(pdev, 4);

	NDBGPR_L1( "BAR0 iommu address = 0x%lx\n", dwc_eth_ntn_reg_pci_base_addr_phy);
	NDBGPR_L1( "BAR2 iommu address = 0x%lx\n", dwc_eth_ntn_SRAM_pci_base_addr_phy);
	NDBGPR_L1( "BAR4 iommu address = 0x%lx\n", dwc_eth_ntn_FLASH_pci_base_addr);

	dwc_eth_ntn_reg_pci_base_addr = (ULONG)ioremap(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));
	dwc_eth_ntn_SRAM_pci_base_addr_virt = (ULONG)ioremap(pci_resource_start(pdev, 2), pci_resource_len(pdev, 2));
#if !defined(NTN_DECLARE_MEM_FOR_DMAAPI) && !defined(DESC_HOSTMEM_BUF_HOSTMEM)
	cur_virt = dwc_eth_ntn_SRAM_pci_base_addr_virt + NTN_DMAAPI_MEM_OFFSET;
#endif
	dwc_eth_ntn_FLASH_pci_base_addr = (ULONG)ioremap(pci_resource_start(pdev, 4), pci_resource_len(pdev, 4));

	NDBGPR_L1( "BAR0 virtual address = 0x%lx\n", dwc_eth_ntn_reg_pci_base_addr);
	NDBGPR_L1( "BAR2 virtual address = 0x%lx\n", dwc_eth_ntn_SRAM_pci_base_addr_virt);
	NDBGPR_L1( "BAR4 virtual address = 0x%lx\n", dwc_eth_ntn_FLASH_pci_base_addr);

	if (((void __iomem *)dwc_eth_ntn_reg_pci_base_addr == NULL)  ||
		((void __iomem *)dwc_eth_ntn_SRAM_pci_base_addr_virt == NULL) ||
		((void __iomem *)dwc_eth_ntn_FLASH_pci_base_addr == NULL) ) {
		NMSGPR_ALERT("%s: cannot map Neutrino BARs, aborting",
						pci_name(pdev));
		ret = -EIO;
		goto err_out_map_failed;
	}
	/*
	{
	unsigned int rd_val;

	rd_val = *(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0x1004);
	*(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0x1004) = (rd_val | 0x80); //Enable MAC Clock

	rd_val = *(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0x1008);
	*(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0x1008) = (rd_val & ~0x80); //Deassert MAC Reset

	NDBGPR_L1( "HFR0 Val = 0x%08x \n", *(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0xA11C));
	NDBGPR_L1( "HFR1 Val = 0x%08x \n", *(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0xA120));
	NDBGPR_L1( "HFR2 Val = 0x%08x \n", *(unsigned int*)(dwc_eth_ntn_reg_pci_base_addr + 0xA124));
	}
	*/
	/* queue count */
	tx_q_count = get_tx_queue_count(dwc_eth_ntn_reg_pci_base_addr);
	rx_q_count = get_rx_queue_count(dwc_eth_ntn_reg_pci_base_addr);

	NDBGPR_L1("No of TX Queue = %d\n", tx_q_count);
	NDBGPR_L1("No of RX Queue = %d\n", rx_q_count);

	/* Need to add 2 to number of queue as this api corresponds to number of DMA channels */
	dev = alloc_etherdev_mqs(sizeof(struct DWC_ETH_QOS_prv_data),
				(tx_q_count+2), (rx_q_count+2));
	if (dev == NULL) {
		NMSGPR_ALERT( "%s:Unable to alloc new net device\n",
		    DEV_NAME);
		ret = -ENOMEM;
		goto err_out_dev_failed;
	}
	++mdio_bus_id;

#ifndef DWC_ETH_QOS_BUILTIN
	/* Read mac address from mac.ini file */
	parse_config_file(mdio_bus_id);
#endif
	if(!is_valid_ether_addr(dev_addr)) {
		NMSGPR_ALERT( "Not found valid mac address\n");
		NMSGPR_ALERT( "Using Default MAC Address\n");
	}
	NMSGPR_INFO( "MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		dev_addr[0], dev_addr[1], dev_addr[2], dev_addr[3], dev_addr[4], dev_addr[5]);
	dev->dev_addr[0] = dev_addr[0];
	dev->dev_addr[1] = dev_addr[1];
	dev->dev_addr[2] = dev_addr[2];
	dev->dev_addr[3] = dev_addr[3];
	dev->dev_addr[4] = dev_addr[4];
	dev->dev_addr[5] = dev_addr[5];

	dev->base_addr = dwc_eth_ntn_reg_pci_base_addr;
	SET_NETDEV_DEV(dev, &pdev->dev);
	pdata = netdev_priv(dev);
	gDWC_ETH_QOS_prv_data = pdata;
	NDBGPR_L1("gDWC_ETH_QOS_prv_data 0x%pK \n", gDWC_ETH_QOS_prv_data);

	DWC_ETH_QOS_init_all_fptrs(pdata);
	hw_if = &(pdata->hw_if);
	desc_if = &(pdata->desc_if);

	pci_set_drvdata(pdev, dev);
	pdata->pdev = pdev;
	pdata->mdio_bus_id = mdio_bus_id;
	pdata->dev = dev;
	pdata->tx_dma_ch_cnt = tx_q_count + 2;
	pdata->rx_dma_ch_cnt = rx_q_count + 2;
	pdata->dwc_eth_ntn_reg_pci_base_addr_phy = dwc_eth_ntn_reg_pci_base_addr_phy;
	pdata->dwc_eth_ntn_FLASH_pci_base_addr = dwc_eth_ntn_FLASH_pci_base_addr;
	pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt = dwc_eth_ntn_SRAM_pci_base_addr_virt;

	/* 	Host: TX DMA CH : 0,2,3,4,
		Host: RX DMA CH : 0,2,3,4,5 */
	for(i=0; i<pdata->rx_dma_ch_cnt && i<NTN_DMA_RX_CH_CNT; i++)
		pdata->rx_dma_ch_for_host[i] = 1;
	for(i=0; i<pdata->tx_dma_ch_cnt && i<NTN_DMA_TX_CH_CNT; i++)
		pdata->tx_dma_ch_for_host[i] = 1;

	/* 	Host: TX Q : 0,1,2
		Host: RX Q : 0,1,2,3 */
	for(i=0; i<rx_q_count && i<NTN_RX_Q_CNT; i++)
		pdata->rx_q_for_host[i] = 1;
	for(i=0; i<tx_q_count && i<NTN_TX_Q_CNT; i++)
		pdata->tx_q_for_host[i] = 1;

	/* 	M3: TX DMA CH : 1
		M3: RX DMA CH : 1 */
	pdata->tx_dma_ch_for_host[1] = 0;
	pdata->rx_dma_ch_for_host[1] = 0;

#ifdef NTN_DECLARE_MEM_FOR_DMAAPI
	phy_mem_adrs = dwc_eth_ntn_SRAM_pci_base_addr_phy;
	ret = dma_declare_coherent_memory(&pdata->pdev->dev, phy_mem_adrs + NTN_DMAAPI_MEM_OFFSET,
              NTN_DMAAPI_MEM_BASE + NTN_DMAAPI_MEM_OFFSET,
              NTN_DMAAPI_MEM_LENGTH,
              DMA_MEMORY_MAP | DMA_MEMORY_EXCLUSIVE);
        if(ret == 0){
                NMSGPR_ALERT( "Coherent memory declaration error!!\n");
                ret = -ENXIO;
                goto err_coherent_mem_declaration;
	}
#endif

#ifdef NTN_TX_DATA_BUF_IN_SRAM
	pdata->tx_mem_pool = dma_pool_create("TX_MEMORY_POOL", &pdata->pdev->dev,
			NTN_TX_MEM_POOL_SIZE, NTN_TX_MEM_POOL_ALIGN, NTN_TX_MEM_POOL_ALLOC);
	if(pdata->tx_mem_pool == NULL)
	{
                NMSGPR_ALERT( "Cann't create TX Memory Pool\n");
                ret = -ENOMEM;
		goto err_tx_mem_pool_creation;
	}
#endif
#ifdef NTN_RX_DATA_BUF_IN_SRAM
	pdata->rx_mem_pool = dma_pool_create("RX_MEMORY_POOL", &pdata->pdev->dev,
			NTN_RX_MEM_POOL_SIZE, NTN_RX_MEM_POOL_ALIGN, NTN_RX_MEM_POOL_ALLOC);
	if(pdata->rx_mem_pool == NULL)
	{
                NMSGPR_ALERT( "Cann't create RX Memory Pool\n");
                ret = -ENOMEM;
		goto err_rx_mem_pool_creation;
	}
#endif
	domain = pci_domain_nr(pdev->bus);
	NDBGPR_L1("%s: RC number for device %d, pci bus number =%d\n", __func__, domain, pdev->bus->number);
	list_for_each_entry(plt_data, &ntn_plt_data_list, node) {
		if (plt_data->rc_num == domain) {
			if (plt_data->bus_num == pdev->bus->number) {
				plt_data->pcidev = pdev;
				pldev = plt_data->pldev;
				break;
			} else {
				if (plt_data->bus_num == -1) {
					/* No device tree entry for bus number*/
					plt_data->pcidev = pdev;
					pldev = plt_data->pldev;
					break;
				}
			}
		}
	}

	if (pldev) {

		ret = of_property_read_u32(pldev->dev.of_node, "qcom,ntn-fw-load-delay-msec", &pdata->fw_load_delay);
		if (ret) {
			NDBGPR_L1("%s: Failed to find fw_load_delay value, hardcoding to 200 msec\n", __func__);
			pdata->fw_load_delay = 200;
		} else {
			NDBGPR_L1("%s: fw_load_delay = %d msec\n", __func__, pdata->fw_load_delay);
		}

		pdata->pcierst_resx = of_property_read_bool(pldev->dev.of_node, "qcom,ntn-pcierst-resx");
		NDBGPR_L1("%s: pcierst_resx = %d\n", __func__, pdata->pcierst_resx);

		pdata->enable_phy = !of_property_read_bool(pldev->dev.of_node, "qcom,ntn-disable-phy");
		NDBGPR_L1("%s: enable_phy = %d\n", __func__, pdata->enable_phy);

		ret = of_property_read_u32(pldev->dev.of_node, "qcom,ntn-rmii-mode", &pdata->rmii_mode);
		if (ret) {
			NDBGPR_L1("%s: Failed to find rmii_mode in the device"
				  " tree, using module parameter\n",__func__);
			pdata->rmii_mode = rmii_mode;
		}
		NDBGPR_L1("%s: rmii_mode = %d\n", __func__, pdata->rmii_mode);

		/*
		* Set the EAVB timestamp window according to the
		* ntn-timestamp-valid-window setting in the device tree.
		* If the timestamp in the AVTP packet is between the
		* current time and the current time plus the timestamp window,
		* Neutrino will hold the packet until the timestamp elapses.
		* Otherwise, the packet is sent immediately.
		* Each unit is 2^24 ns, or approx 16.8 ms.
		* If the setting is not in the device tree, set the value to
		* 4 (67 ms). Note: if this register is not set, Neutrino will default to
		* 0x80 (2.1 sec). 67 ms is chosen as a reasonable default because
		* trying to queue up more than 67 ms worth of packets would cause the
		* TX queue to overflow, and according to the AVB spec, Class B packets
		* should have no more than 50 ms of latency, and Class A packets should
		* have even less latency than that. Thus an application would not try to
		* send AVTP packets with timestamps more than 50 ms in the future.
		*/
		ret = of_property_read_u32(pldev->dev.of_node,
							   "qcom,ntn-timestamp-valid-window",
							   &pdata->ntn_timestamp_valid_window);
		if (ret) {
			NDBGPR_L1("%s: Failed to read ntn-timestamp-valid-window, hardcoding to 4 (67 msec)\n",
					 __func__);
			pdata->ntn_timestamp_valid_window = 4;
			ret = 0; /* This is not a fatal error. */
		} else {
			NDBGPR_L1("%s: ntn-timestamp-valid-window = %d, (%u nsec)\n",
				  __func__,
				  pdata->ntn_timestamp_valid_window,
				  pdata->ntn_timestamp_valid_window << 24);
		}

		ret = of_property_read_u32(pldev->dev.of_node, "qcom,ntn-mdio-bus-id", (u32*)&pdata->mdio_bus_id);

		if (ret) {
			NDBGPR_L1("%s: Failed to find ntn-mdio-bus-id value, hardcoding to 1\n", __func__);
			pdata->mdio_bus_id = mdio_bus_id;
		} else {
			NDBGPR_L1("%s: ntn-mdio-bus-id = %d msec\n", __func__, pdata->mdio_bus_id);
		}

		ret = of_property_read_u32(pldev->dev.of_node, "qcom,ntn-phy-addr", &pdata->phyaddr);

		if (ret) {
			NDBGPR_L1("%s: Failed to find ntn-phy-addr value\n", __func__);
			pdata->phyaddr = NTN_INVALID_PHY_ADDR;
		} else {
			NDBGPR_L1("%s: ntn-phy-addr = %d \n", __func__, pdata->phyaddr);
		}
	} else {
		NMSGPR_ALERT( "%s: Failed to get platform data for the device\n",__func__);
	}

#ifdef NTN_ENABLE_PCIE_MEM_ACCESS
	DWC_ETH_QOS_config_tamap(pdev);
#endif

	/* PHY Mode (MII/RMII or default RGMII)
	 * should be configured prior to GMAC clock enable/reset. */
	/* issue clock enable to GMAC device */
	hw_if->ntn_mac_clock_config(0x1, pdata);
	/* issue software reset to GMAC device */
	hw_if->exit(pdata);

	//Assert TDM reset, in case if it is on.
	reg_val = hw_if->ntn_reg_rd(0x1008, 0, pdata);
	reg_val |= (0x1<<6);
	hw_if->ntn_reg_wr(0x1008, reg_val, 0, pdata);

	/* Enable MSI support: following commneted code just enables the one MSI interrupt. This is only kept for debugging purpose.*/
#if 1
	ret = pci_enable_msi(pdev);
	if(ret) {
		NMSGPR_ALERT( "%s:Enable MSI error\n", DEV_NAME);
		goto err_out_msi_failed;
	}
#else
	pdata->max_irq = NTN_MAX_MSI;
	ret = pci_enable_msi_range(pdev, pdata->max_irq, pdata->max_irq);
	if(ret < 0) //Failed to allocate
	{
		NMSGPR_ALERT( "%s:Enable MSI error\n", DEV_NAME);
		goto err_out_msi_failed;
	} else if (ret > 0) {   //This is the possible number that device can allocate
		pdata->max_irq = ret;
		NMSGPR_ALERT( "Changing max no of MSI to = %d\n", pdata->max_irq);
	}
	NMSGPR_ALERT( "Allocated MSI = %d\n", pdata->max_irq);
#endif

	pci_write_config_dword(pdev, pdev->msi_cap + PCI_MSI_MASK_64, 0);

	//  Only one MSI supported for now.
	dev->irq = pdev->irq;
	NDBGPR_L1( "Allocated IRQ Number = %d\n", dev->irq);

	if (hw_if->ntn_boot_host_initiated(pdata)) {
		ret = DWC_ETH_QOS_load_fw(pdev);
		if (ret) {
			NMSGPR_ALERT("ERROR: Unable to load firmware\n");
			goto err_load_fw_failed;
		}
	} else if (!hw_if->ntn_boot_from_flash_done(pdata)) {
		NMSGPR_ALERT("ERROR: Boot from flash failed\n");
		/* eth interface need to comeup even if firmware is not available
		 * in HW, hence do not return here. Please refer CR 1069885.
		 */
	} else {
		NMSGPR_ALERT("Boot from flash complete\n");
	}

	pdata->fw_ver_cap = hw_if->read_fw_ver_features(pdata);
	NMSGPR_ALERT("DBG Resvd area: 0x%x\n", pdata->fw_ver_cap);
	NMSGPR_ALERT("NTN FW Version: %d.%d\n", (pdata->fw_ver_cap >> 12),
				((pdata->fw_ver_cap >> 8) & 0xF));

	/* Verify if IPA is supported in firmware */
	pdata->ipa_enabled =
		hw_if->ntn_fw_ipa_supported(pdata) && NTN_HOST_IPA_CAPABLE;
	NMSGPR_INFO("NTN IPA enabled: %d\n", pdata->ipa_enabled);
	if (pdata->ipa_enabled) {
		pdata->prv_ipa.ipa_ver = ipa_get_hw_type();
		DWC_ETH_QOS_config_ipareg_map(pdev);
		wakeup_source_init(&pdata->prv_ipa.wlock, "NTN_IPA_WS");
		hw_if->enable_offload(pdata);
	} else {
		hw_if->disable_offload(pdata);
	}

	DWC_ETH_QOS_get_all_hw_features(pdata);
	DWC_ETH_QOS_print_all_hw_features(pdata);

	ret = desc_if->alloc_queue_struct(pdata);
	if (ret < 0) {
		NMSGPR_ALERT( "ERROR: Unable to alloc Tx/Rx queue\n");
		goto err_out_q_alloc_failed;
	}

	dev->netdev_ops = DWC_ETH_QOS_get_netdev_ops();

	pdata->interface = DWC_ETH_QOS_get_phy_interface(pdata);
	/* Bypass PHYLIB for TBI, RTBI and SGMII interface */
	if (pdata->enable_phy && 1 == pdata->hw_feat.sma_sel) {
		ret = DWC_ETH_QOS_mdio_register(dev);
		if (ret < 0) {
			NMSGPR_ALERT( "MDIO bus (id %d) registration failed\n",
			       pdata->bus_id);
			goto err_out_mdio_reg;
		}
	} else {
		NMSGPR_ALERT( "%s: MDIO is not present\n\n", DEV_NAME);
	}

	/* enabling and registration of irq with magic wakeup */
	if (1 == pdata->hw_feat.mgk_sel) {
		device_set_wakeup_capable(&pdev->dev, 1);
		device_set_wakeup_enable(&pdata->pdev->dev, 1);
		pdata->wolopts = WAKE_MAGIC;
	}

	for (i = 0; i < NTN_RX_DMA_CH_CNT; i++) {
		struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = GET_RX_DMA_CH_PTR(i);
		if(!pdata->rx_dma_ch_for_host[i])
			continue;

		netif_napi_add(dev, &rx_dma_ch->napi, DWC_ETH_QOS_poll_mq,
				(64 * NTN_RX_DMA_CH_CNT));
	}

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0) )
	SET_ETHTOOL_OPS(dev, DWC_ETH_QOS_get_ethtool_ops());
#else	//3.16.0
	netdev_set_default_ethtool_ops(dev, DWC_ETH_QOS_get_ethtool_ops());
#endif	//3.16.0

	DWC_ETH_QOS_reset_ethtool_stats(pdata);

	if (pdata->hw_feat.tso_en) {
		dev->hw_features = NETIF_F_TSO;
		dev->hw_features |= NETIF_F_SG;
		dev->hw_features |= NETIF_F_IP_CSUM;
		dev->hw_features |= NETIF_F_IPV6_CSUM;
		NDBGPR_L2( "Supports TSO, SG and TX COE\n");
	}
	else if (pdata->hw_feat.tx_coe_sel) {
		dev->hw_features = NETIF_F_IP_CSUM ;
		dev->hw_features |= NETIF_F_IPV6_CSUM;
		NDBGPR_L2( "Supports TX COE\n");
	}

	if (pdata->hw_feat.rx_coe_sel) {
		dev->hw_features |= NETIF_F_RXCSUM;
		dev->hw_features |= NETIF_F_LRO;
		NDBGPR_L2( "Supports RX COE and LRO\n");
	}
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	dev->vlan_features |= dev->hw_features;
	dev->hw_features |= NETIF_F_HW_VLAN_CTAG_RX;
	if (pdata->hw_feat.sa_vlan_ins) {
		dev->hw_features |= NETIF_F_HW_VLAN_CTAG_TX;
		NDBGPR_L2( "VLAN Feature enabled\n");
	}
	if (pdata->hw_feat.vlan_hash_en) {
		dev->hw_features |= NETIF_F_HW_VLAN_CTAG_FILTER;
		NDBGPR_L2( "VLAN HASH Filtering enabled\n");
	}
#endif /* end of DWC_ETH_QOS_ENABLE_VLAN_TAG */
	dev->features |= dev->hw_features;
	pdata->dev_state |= dev->features;

	DWC_ETH_QOS_init_rx_coalesce(pdata);

#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_init(pdata);
#endif	/* end of DWC_ETH_QOS_CONFIG_PTP */

	spin_lock_init(&pdata->lock);
	mutex_init(&pdata->mlock);
	spin_lock_init(&pdata->tx_lock);
	mutex_init(&pdata->pmt_lock);
	spin_lock_init(&pdata->fqtss_lock);

	INIT_WORK(&pdata->powerup_work, DWC_ETH_QOS_powerup_handler);
	INIT_WORK(&pdata->restartdev_work, DWC_ETH_QOS_restart_dev_handler);
	INIT_DELAYED_WORK(&pdata->phy_dwork, (void *)DWC_ETH_QOS_adjust_link);

	ret = register_netdev(dev);
	if (ret) {
		NMSGPR_ALERT( "%s: Net device registration failed\n",
		    DEV_NAME);
		goto err_out_netdev_failed;
	}

	/* Initialize PCI link status and register for status changes */
	DWC_ETH_QOS_pci_event_register(pdev);

	DBGPR("<-- DWC_ETH_QOS_probe\n");

	netif_carrier_off(dev);
	NDBGPR_L1("%s: Carrier off until the LINK is up\n", DEV_NAME);

	/* Register bus client */
	ntn_bus_pdata = msm_bus_cl_get_pdata(pldev);
	if( ntn_bus_pdata != NULL ){
		pdata->bus_client = msm_bus_scale_register_client(ntn_bus_pdata);
		if (!pdata->bus_client)
			pr_err("Unable to register bus client\n");
	} else
		pr_err("msm_bus_cl_get_pdata() failed\n");
	return 0;

 err_out_netdev_failed:
#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_remove(pdata);
#endif	/* end of DWC_ETH_QOS_CONFIG_PTP */

	if (1 == pdata->hw_feat.sma_sel)
		DWC_ETH_QOS_mdio_unregister(dev);

 err_out_mdio_reg:
	desc_if->free_queue_struct(pdata);

 err_load_fw_failed:
 err_out_q_alloc_failed:
    pci_disable_msi(pdev);
 err_out_msi_failed:

#ifdef NTN_RX_DATA_BUF_IN_SRAM
    dma_pool_destroy(pdata->rx_mem_pool);
 err_rx_mem_pool_creation:
#endif
#ifdef NTN_TX_DATA_BUF_IN_SRAM
    dma_pool_destroy(pdata->tx_mem_pool);
 err_tx_mem_pool_creation:
#endif
#ifdef NTN_DECLARE_MEM_FOR_DMAAPI
    dma_release_declared_memory(&pdata->pdev->dev);

 err_coherent_mem_declaration:
#endif
	atomic_notifier_chain_unregister(&panic_notifier_list,
					 &DWC_ETH_QOS_panic_blk);
    /* issue clock disable to GMAC device */
    hw_if->ntn_mac_clock_config(0x0, pdata);

	free_netdev(dev);
	pci_set_drvdata(pdev, NULL);

 err_out_dev_failed:
    if((void __iomem*)dwc_eth_ntn_reg_pci_base_addr != NULL)
	    pci_iounmap(pdev, (void __iomem *)dwc_eth_ntn_reg_pci_base_addr);

 err_out_map_failed:
	pci_release_regions(pdev);

 err_out_dma_mask_failed:
 err_out_req_reg_failed:
	pci_disable_device(pdev);

 err_out_enb_failed:
 err_pci_restore_failed:

	return ret;
}

/*!
* \brief API to release all the resources from the driver.
*
* \details The remove function gets called whenever a device being handled
* by this driver is removed (either during deregistration of the driver or
* when it is manually pulled out of a hot-pluggable slot). This function
* should reverse operations performed at probe time. The remove function
* always gets called from process context, so it can sleep.
*
* \param[in] pdev - pointer to pci_dev structure.
*
* \return void
*/

static void DWC_ETH_QOS_remove(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct desc_if_struct *desc_if = &(pdata->desc_if);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = NULL;
	int i;
	DBGPR("--> DWC_ETH_QOS_remove\n");

	cancel_delayed_work_sync(&pdata->phy_dwork);
	flush_delayed_work(&pdata->phy_dwork);
	if (pdata->irq_number != 0) {
		disable_irq_wake(dev->irq);
		free_irq(pdata->irq_number, pdata);
		pdata->irq_number = 0;
	}

	if (1 == pdata->hw_feat.sma_sel)
		DWC_ETH_QOS_mdio_unregister(dev);

#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_remove(pdata);
#endif /* end of DWC_ETH_QOS_CONFIG_PTP */

	unregister_netdev(dev);

#ifdef NTN_RX_DATA_BUF_IN_SRAM
    dma_pool_destroy(pdata->rx_mem_pool);
#endif
#ifdef NTN_TX_DATA_BUF_IN_SRAM
    dma_pool_destroy(pdata->tx_mem_pool);
#endif
#ifdef NTN_DECLARE_MEM_FOR_DMAAPI
    dma_release_declared_memory(&pdata->pdev->dev);
#endif

	/* issue clock disable to GMAC device */
    hw_if->ntn_mac_clock_config(0x0, pdata);

	/* If NAPI is enabled, delete any references to the NAPI struct. */
	for (i = 0; i < NTN_RX_DMA_CH_CNT; i++) {
		rx_dma_ch = GET_RX_DMA_CH_PTR(i);
		if (!pdata->rx_dma_ch_for_host[i])
			continue;
		netif_napi_del(&rx_dma_ch->napi);
	}

	desc_if->free_queue_struct(pdata);

	/* Deregister from PCI link events */
	DWC_ETH_QOS_pci_event_deregister(pdev);

	/* Save the PCI link status and suspend it */
	DWC_ETH_QOS_pci_suspend(pdev);


	pci_disable_msi(pdev);
	pci_set_drvdata(pdev, NULL);

	if((void __iomem*)pdata->dev->base_addr != NULL)
        pci_iounmap(pdev, (void __iomem *)pdata->dev->base_addr);

	if ((void __iomem*)pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt != NULL)
		pci_iounmap(pdev, (void __iomem *)pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt);
	if ((void __iomem*)pdata->dwc_eth_ntn_FLASH_pci_base_addr != NULL)
		pci_iounmap(pdev, (void __iomem *)pdata->dwc_eth_ntn_FLASH_pci_base_addr);

	free_netdev(dev);
	pci_release_regions(pdev);
	pci_disable_device(pdev);

	DBGPR("<-- DWC_ETH_QOS_remove\n");

	return;
}

static struct pci_device_id DWC_ETH_QOS_id = {
	PCI_DEVICE(VENDOR_ID, DEVICE_ID)
};

struct pci_dev *DWC_ETH_QOS_pcidev;

static struct pci_driver DWC_ETH_QOS_pci_driver = {
	.name = "DWC_ETH_QOS",
	.id_table = &DWC_ETH_QOS_id,
	.probe = DWC_ETH_QOS_probe,
	.remove = DWC_ETH_QOS_remove,
	.shutdown = DWC_ETH_QOS_shutdown,
	.suspend_late = DWC_ETH_QOS_suspend_late,
	.resume_early = DWC_ETH_QOS_resume_early,
#ifdef CONFIG_PM
	.suspend = DWC_ETH_QOS_suspend,
	.resume = DWC_ETH_QOS_resume,
#endif
	.driver = {
		   .name = DEV_NAME,
		   .owner = THIS_MODULE,
	},
};

static void DWC_ETH_QOS_shutdown(struct pci_dev *pdev)
{
	DBGPR( "-->DWC_ETH_QOS_shutdown\n");
	DBGPR( "Handle the shutdown\n");
	DBGPR( ">--DWC_ETH_QOS_shutdown\n");

	return;
}

static INT DWC_ETH_QOS_suspend_late(struct pci_dev *pdev, pm_message_t state)
{
	DBGPR( "-->DWC_ETH_QOS_suspend_late\n");
	DBGPR( "Handle the suspend_late\n");
	DBGPR( "<--DWC_ETH_QOS_suspend_late\n");

	return 0;
}

static INT DWC_ETH_QOS_resume_early(struct pci_dev *pdev)
{
	DBGPR( "-->DWC_ETH_QOS_resume_early\n");
	DBGPR( "Handle the resume_early\n");
	DBGPR( "<--DWC_ETH_QOS_resume_early\n");

	return 0;
}

#ifdef CONFIG_PM

/*!
 * \brief Routine to put the device in suspend mode
 *
 * \details This function gets called by PCI core when the device is being
 * suspended. The suspended state is passed as input argument to it.
 * Following operations are performed in this function,
 * - stop the phy.
 * - detach the device from stack.
 * - stop the queue.
 * - Disable napi.
 * - Stop DMA TX and RX process.
 * - Enable power down mode using PMT module or disable MAC TX and RX process.
 * - Save the pci state.
 *
 * \param[in] pdev – pointer to pci device structure.
 * \param[in] state – suspend state of device.
 *
 * \return int
 *
 * \retval 0
 */

static INT DWC_ETH_QOS_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	INT ret, pmt_flags = 0;
	unsigned int rwk_filter_values[] = {
		/* for filter 0 CRC is computed on 0 - 7 bytes from offset */
		0x000000ff,

		/* for filter 1 CRC is computed on 0 - 7 bytes from offset */
		0x000000ff,

		/* for filter 2 CRC is computed on 0 - 7 bytes from offset */
		0x000000ff,

		/* for filter 3 CRC is computed on 0 - 31 bytes from offset */
		0x000000ff,

		/* filter 0, 1 independently enabled and would apply for
		 * unicast packet only filter 3, 2 combined as,
		 * "Filter-3 pattern AND NOT Filter-2 pattern" */
		0x03050101,

		/* filter 3, 2, 1 and 0 offset is 50, 58, 66, 74 bytes
		 * from start */
		0x4a423a32,

		/* pattern for filter 1 and 0, "0x55", "11", repeated 8 times */
		0xe7b77eed,

		/* pattern for filter 3 and 4, "0x44", "33", repeated 8 times */
		0x9b8a5506,
	};

	DBGPR("-->DWC_ETH_QOS_suspend\n");

	if (!dev || !netif_running(dev) || (!pdata->hw_feat.mgk_sel &&
			!pdata->hw_feat.rwk_sel)) {
		DBGPR("<--DWC_ETH_QOS_dev_suspend\n");
		return -EINVAL;
	}

	if (pdata->hw_feat.rwk_sel && (pdata->wolopts & WAKE_UCAST)) {
		pmt_flags |= DWC_ETH_QOS_REMOTE_WAKEUP;
		hw_if->configure_rwk_filter(rwk_filter_values, 8, pdata);
	}

	if (pdata->hw_feat.mgk_sel && (pdata->wolopts & WAKE_MAGIC))
		pmt_flags |= DWC_ETH_QOS_MAGIC_WAKEUP;

	ret = DWC_ETH_QOS_powerdown(dev, pmt_flags, DWC_ETH_QOS_DRIVER_CONTEXT);
	pci_disable_msi(pdev);
#ifdef CONFIG_PCI_MSM
	msm_pcie_shadow_control(pdev, false);
#endif
	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));

	DBGPR("<--DWC_ETH_QOS_suspend\n");

	return ret;
}

/*!
 * \brief Routine to resume device operation
 *
 * \details This function gets called by PCI core when the device is being
 * resumed. It is always called after suspend has been called. These function
 * reverse operations performed at suspend time. Following operations are
 * performed in this function,
 * - restores the saved pci power state.
 * - Wakeup the device using PMT module if supported.
 * - Starts the phy.
 * - Enable MAC and DMA TX and RX process.
 * - Attach the device to stack.
 * - Enable napi.
 * - Starts the queue.
 *
 * \param[in] pdev – pointer to pci device structure.
 *
 * \return int
 *
 * \retval 0
 */

static INT DWC_ETH_QOS_resume(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	INT ret;
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	DBGPR("-->DWC_ETH_QOS_resume\n");

	if (!dev || !netif_running(dev)) {
		DBGPR("<--DWC_ETH_QOS_dev_resume\n");
		return -EINVAL;
	}

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
#ifdef CONFIG_PCI_MSM
	msm_pcie_shadow_control(pdev, true);
#endif

	/* Recover NTN in case whole chip was in reset */
	if (pdata->pcierst_resx) {
		if (hw_if->ntn_boot_host_initiated(pdata)) {
			DWC_ETH_QOS_load_fw(pdev);
			if (pdata->ipa_enabled)
				hw_if->enable_offload(pdata);
		}
	}

#ifdef NTN_ENABLE_PCIE_MEM_ACCESS
	DWC_ETH_QOS_config_tamap(pdev);
#endif

	pci_enable_msi(pdev);
	ret = DWC_ETH_QOS_powerup(dev, DWC_ETH_QOS_DRIVER_CONTEXT);
	if (pdata->prv_ipa.ipa_offload_susp)
		DWC_ETH_QOS_ipa_offload_resume(pdata);

	/* Hold the wake lock for 5sec to ensure any traffic*/
	pm_wakeup_event(&pdev->dev, 5000);

	DBGPR("<--DWC_ETH_QOS_resume\n");

	return ret;
}

#endif	/* CONFIG_PM */

/*!
* \brief API to register the driver.
*
* \details This is the first function called when the driver is loaded.
* It register the driver with PCI sub-system
*
* \return void.
*/
int DWC_ETH_QOS_init_module(void)
{
	INT ret = 0;

	DBGPR( "-->DWC_ETH_QOS_init_module\n");

	ret = pci_register_driver(&DWC_ETH_QOS_pci_driver);
	if (ret < 0) {
		NMSGPR_ALERT( "DWC_ETH_QOS:driver registration failed");
		return ret;
	}

	atomic_notifier_chain_register(&panic_notifier_list,
			&DWC_ETH_QOS_panic_blk);
	DBGPR( "<--DWC_ETH_QOS_init_module\n");

	return ret;
}

/*!
* \brief API to unregister the driver.
*
* \details This is the first function called when the driver is removed.
* It unregister the driver from PCI sub-system
*
* \return void.
*/
void DWC_ETH_QOS_exit_module(void)
{
	DBGPR("-->DWC_ETH_QOS_exit_module\n");

	atomic_notifier_chain_unregister(&panic_notifier_list,
					 &DWC_ETH_QOS_panic_blk);
	pci_unregister_driver(&DWC_ETH_QOS_pci_driver);

	DBGPR( "<--DWC_ETH_QOS_exit_module\n");
}

/*!
* \brief Macro to declare the module author.
*
* \details This macro is used to declare the module's authore.
*/
MODULE_AUTHOR("Toshiba America Electronic Component");

/*!
* \brief Macro to describe what the module does.
*
* \details This macro is used to describe what the module does.
*/
MODULE_DESCRIPTION("Neutrino Driver");

/*!
* \brief Macro to describe the module license.
*
* \details This macro is used to describe the module license.
*/
MODULE_LICENSE("GPL");
