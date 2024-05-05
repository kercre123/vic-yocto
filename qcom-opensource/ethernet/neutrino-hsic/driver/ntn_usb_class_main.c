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
 
/*! History:   
 *      18-July-2016 : Initial 
 */
 
#include "ntn_common.h"
#include "ntn_ptp.h"
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/rwlock.h>
#include <linux/usb/hcd.h>
#include <linux/ctype.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include "ntn_usbnet.c"

#define DRV_VERSION	"1.00.00"
#define RX_URB_SIZE (1024 * 4)

int32_t nic_speed;

struct ntn_ptp_data ntn_pdata;

/*Neutrino Voltage regulator*/
#define NTN_VREG_HSIC_NAME 	"vdd-ntn-hsic"
#define NTN_VREG_HSIC_MIN	1800000
#define NTN_VREG_HSIC_MAX	1800000

#define MAC_CONFIG_KEY_LEN	32
#define MAC_CONFIG_STR_LEN	20
#define MAC_CONFIG_ADR_LEN	6
#define MAC_CONFIG_PARAM_NUM (sizeof(mac_param_list)/sizeof(mac_param_list[0]))

static int msg_enable;
module_param(msg_enable, int, 0);
MODULE_PARM_DESC(msg_enable, "usbnet msg_enable");

static int cfg;

module_param(cfg, int, 0);

MODULE_PARM_DESC(cfg, "Configuration selection");


struct api_context{
	struct completion done;
	int	status;
};

struct ntn_hsic_plt_data {
	struct list_head node;
	struct platform_device *pldev;
	struct regulator *ntn_reg_hsic;
	unsigned int ntn_phy_id;
};

typedef struct
{
	char mdio_key[MAC_CONFIG_KEY_LEN];
	unsigned short mdio_key_len;
	char mac_key[MAC_CONFIG_KEY_LEN];
	unsigned short mac_key_len;
	unsigned short mac_str_len;
	char mac_str_def[MAC_CONFIG_STR_LEN];
} mac_config_param_list_t;

struct list_head ntn_hsic_plt_data_list;

/* Cable plug/unplug timer to configure MAC based
 * on PHY LINK Status
 */
#ifndef NTN_DRV_TEST_LOOPBACK
static struct timer_list synopGMAC_cable_unplug_timer;
static struct delayed_work task;
#endif

struct usb_device *device;
static struct usb_class_driver class;
struct ntn_usb_device ntn_dev;
unsigned int phy_loopback_mode;

struct ntn_data *pdata_phc;

static const mac_config_param_list_t mac_param_list[] = {
{"MDIOBUSID",9,"MAC_ID", 6, 17, "00:00:00:00:00:00"},
};
static u8 mac_addr[MAC_CONFIG_ADR_LEN]  = DEFAULT_MAC_ADDRESS;

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
		mac_addr[j++] = mac_id;
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
static bool lookfor_macid(char *file_buf, u8 bus_id)
{
	char *string = NULL, *token_n = NULL, *token_s = NULL, *token_m = NULL;
	bool status = false;
	u8 ntn_device_no= 0;

	string = file_buf;
	/* Parse Line-0 */
	token_n = strsep(&string, "\n");
	while (token_n != NULL) {

		/* Check if line is enabled */
		if (token_n[0] != '#') {
			/* Extract the token based space character */
			token_s = strsep(&token_n, " ");
			if (token_s != NULL) {
			if (strncmp (token_s, mac_param_list[0].mdio_key, 9) == 0 ) {
					token_s = strsep(&token_n, " ");
					token_m = strsep(&token_s, ":");
					sscanf(token_m, "%d", &ntn_device_no);
					if (ntn_device_no != bus_id){
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
				if (strncmp (token_s, mac_param_list[0].mac_key, 6) == 0 ) {

					printk("MAC_ID Key is found\n");
					/* Read next word */
					token_s = strsep(&token_n, " \n");
					if (token_s != NULL) {

						/* Check if MAC ID length  and MAC ID is valid */
						if ((isMAC(token_s) == true) && (strlen(token_s) ==  mac_param_list[0].mac_str_len)) {
							/* If user configured MAC ID is valid,  assign default MAC ID */
							extract_macid(token_s);
							status = true;
						} else {
						    printk( "Mac: isMAC(token_s) = %d\n",isMAC(token_s));
							printk( "Mac: strlen(token_s) = %d\n",(int)strlen(token_s));
							printk( "Mac: mac_param_list[0].mac_str_len = %d\n",mac_param_list[0].mac_str_len);
							printk( "Valid Mac ID not found\n");
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
static void parse_config_file(u8 bus_id)
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
		printk( "Mac configuration file not found\n");
		printk( "Using Default MAC Address\n");
		return;
	}
	else  {
		/* Parse the file */
		ret = file_read(filep, 0, data, 512);
		for (i = 0; i < MAC_CONFIG_PARAM_NUM; i++) {
			if (strstr ((const char *)data, mac_param_list[i].mdio_key)) {
				printk("Pattern Match\n");
				if (strncmp(mac_param_list[i].mdio_key, "MDIOBUSID", 9) == 0) {
					/* MAC ID Configuration */
					printk("MAC_ID Configuration\n");
					if (lookfor_macid(data, bus_id) == false) {
						printk( "Using Default MAC Address\n");
					}
		}
			}
		}
	}

	kfree(data);
	filp_close(filep, NULL);

	return;
}

static int setup_regulator_common(struct device *dev, const char *name_supply,
				  const char *name, struct regulator **vreg,
				  int min_uV, int max_uV)
{
	int ret = 0;

	if (of_get_property(dev->of_node, name_supply, NULL)) {
		*vreg = regulator_get(dev, name);
		if (IS_ERR(*vreg)) {
			ret = PTR_ERR(*vreg);
			if (ret == -EPROBE_DEFER) {
				printk("%s: %s probe deferred!\n", __func__,
					 name);
				return -EPROBE_DEFER;
			}else{
				printk("%s: Get %s failed!\n", __func__,
				        name);
			return -1;
			}
		} else {
			ret = regulator_set_voltage(*vreg, min_uV, max_uV);
				if (ret) {
					printk("%s: Set %s failed!\n", 
							__func__, name);
					return -1;
				}
				else {
					ret = regulator_enable(*vreg);
					if (ret) {
						printk("%s: Enable %s failed!\n",
								 __func__, name);
						return -1;
				}
			}
		}
	} else {
		printk("%s: power supply %s not found!\n", __func__, name);
	}

	return 0;
}

static void neu_hsic_free_regs(struct ntn_hsic_plt_data *plt_data)
{
	if (plt_data->ntn_reg_hsic)
		regulator_put(plt_data->ntn_reg_hsic);
}

/*!
* \brief Initialize PHY ID
*
* \details Parse the device tree for PHY ID node
* and assign value in HSIC platform data structure
*
* \param[in] dev - struct device from platform_device
* \param[in] plt_data - struct ntn_hsic_plt_data from HSIC device
* \return 0 on success. Negative error code on failure
*/
static neutrino_init_phy(struct device *dev, struct ntn_hsic_plt_data *plt_data)
{
	int ret = 0;

	if(!dev)
		return -EINVAL;

	ret = of_property_read_u32(dev->of_node, "ntn-phy-id", &plt_data->ntn_phy_id);
	if(ret){
		printk("%s: Failed to find ntn-phy-id value", __func__);
	}

	return ret;
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
static int neutrino_init_regulators(struct device *dev, struct ntn_hsic_plt_data *plt_data)
{
	int ret = 0;

	if (!dev)
		return -EINVAL;

	/* Configure power rails Neutrino*/
	ret = setup_regulator_common(dev, NTN_VREG_HSIC_NAME"-supply",
					NTN_VREG_HSIC_NAME,&plt_data->ntn_reg_hsic,
					NTN_VREG_HSIC_MIN, NTN_VREG_HSIC_MAX);
	return ret;
}

/*!
 * \brief This sequence is used to select Tx Scheduling Algorithm for AVB feature for Queue[1 - 7]
 * \param[in] avb_algo
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int set_avb_algorithm(u32 chInx, u8 avb_algo)
{

  MTL_QECR_AVALG_UdfWr(chInx, avb_algo);

  return Y_SUCCESS;
}


/*!
 * \brief This sequence is used to configure credit-control for Queue[1 - 7]
 * \param[in] chInx
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int config_credit_control(u32 chInx, u32 cc)
{

  MTL_QECR_CC_UdfWr(chInx, cc);

  return Y_SUCCESS;
}


/*!
 * \brief This sequence is used to set tx queue operating mode for Queue[0 - 7]
 * \param[in] chInx
 * \param[in] q_mode
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int set_tx_queue_operating_mode(u32 chInx,
                                       u32 q_mode)
{

  MTL_QTOMR_TXQEN_UdfWr(chInx, q_mode);

  return Y_SUCCESS;
}


/*!
 * \brief This sequence is used to configure send slope credit value
 * required for the credit-based shaper alogorithm for Queue[1 - 7]
 * \param[in] chInx
 * \param[in] sendSlope
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int config_send_slope(u32 chInx,
                          u32 sendSlope)
{
  DBGPR("send slop  %08x\n",sendSlope);
  MTL_QSSCR_SSC_UdfWr(chInx, sendSlope);

  return Y_SUCCESS;
}

/*!
 * \brief This sequence is used to configure idle slope credit value
 * required for the credit-based shaper alogorithm for Queue[1 - 7]
 * \param[in] chInx
 * \param[in] idleSlope
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int config_idle_slope(u32 chInx,
                          u32 idleSlope)
{
  DBGPR("Idle slop  %08x\n",idleSlope);
  MTL_QW_ISCQW_UdfWr(chInx, idleSlope);

  return Y_SUCCESS;
}



/*!
 * \brief This sequence is used to configure low credit value
 * required for the credit-based shaper alogorithm for Queue[1 - 7]
 * \param[in] chInx
 * \param[in] lowCredit
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int config_low_credit(u32 chInx,
                        u32 lowCredit)
{
        int lowCredit_neg = lowCredit;
        DBGPR("lowCreidt = %08x lowCredit_neg:%08x\n",
                        lowCredit, lowCredit_neg);
        MTL_QLCR_LC_UdfWr(chInx, lowCredit_neg);

  MTL_QLCR_LC_UdfWr(chInx, lowCredit);

  return Y_SUCCESS;
}




/*!
 * \brief This sequence is used to configure high credit value required
 * for the credit-based shaper alogorithm for Queue[1 - 7]
 * \param[in] chInx
 * \param[in] hiCredit
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int config_high_credit(u32 chInx,
                           u32 hiCredit)
{
  DBGPR("hiCreidt = %08x\n", hiCredit);

  MTL_QHCR_HC_UdfWr(chInx, hiCredit);

  return Y_SUCCESS;
}

/*!
* \brief This sequence is used to select perfect/inverse matching for L2 DA
* \param[in] perfect_inverse_match
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int config_l2_da_perfect_inverse_match(struct ntn_data *priv, int perfect_inverse_match)
{
	priv->MAC_Packet_Filter = (priv->MAC_Packet_Filter & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);
	priv->MAC_Packet_Filter = (priv->MAC_Packet_Filter & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);
	priv->MAC_Packet_Filter = (priv->MAC_Packet_Filter & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);
	priv->MAC_Packet_Filter = ((priv->MAC_Packet_Filter & MAC_MPFR_DAIF_Wr_Mask) | ((perfect_inverse_match & MAC_MPFR_DAIF_Mask)<<3));
	MAC_MPFR_RgWr_async(priv->MAC_Packet_Filter);

  return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure hash table register for
* hash address filtering
* \param[in] idx
* \param[in] data
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int update_hash_table_reg(int idx, unsigned int data)
{

  MAC_HTR_RgWr_async(idx, data);

  return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure MAC in differnet pkt processing
* modes like promiscuous, multicast, unicast, hash unicast/multicast.
* \param[in] pr_mode
* \param[in] huc_mode
* \param[in] hmc_mode
* \param[in] pm_mode
* \param[in] hpf_mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int config_mac_pkt_filter_reg(struct ntn_data *priv, unsigned char pr_mode, unsigned char huc_mode, unsigned char hmc_mode, unsigned char pm_mode, unsigned char hpf_mode)
{
  /* configure device in differnet modes */
  /* promiscuous, hash unicast, hash multicast, */
  /* all multicast and perfect/hash filtering mode. */
  
  priv->MAC_Packet_Filter &= (unsigned int)(0x003003e8);
  priv->MAC_Packet_Filter |= ((pr_mode) << 31) | ((huc_mode) << 1) | ((hmc_mode) << 2) | ((pm_mode) << 4) | ((hpf_mode) << 10);
  MAC_MPFR_RgWr_async(priv->MAC_Packet_Filter);
  
  DBGPR("PKT Filter Value : %x\n", priv->MAC_Packet_Filter);
  return Y_SUCCESS;
}

/*!
* \brief This sequence is used to update the MAC address in last 96 MAC
* address Low and High register(32-127) for L2 layer filtering
* \param[in] idx
* \param[in] addr
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int update_mac_addr32_127_low_high_reg(int idx, unsigned char addr[])
{
	unsigned int HR = 0x0000FFFF & ((0x1<<17)|(addr[4] | (addr[5] << 8)));
	MAC_MA32_127LR_RgWr_async(idx, (addr[0] | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24)));
	MAC_MA32_127HR_RgWr_async(idx, HR);
	MAC_MA32_127HR_RgWr_async(idx, HR | (0x1<<31));

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to update the MAC address in 3 to 31 MAC
* address Low and High register(3-31) for L2 layer filtering.
*
* MAC Address Registers [0] [1] [2] should not be used for Perfect filtering.
* OS may override valid MAC Addresses (when multiple MACs are enabled). 
* 
* \param[in] idx
* \param[in] addr
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int update_mac_addr3_31_low_high_reg(int idx, unsigned char addr[])
{
	unsigned int HR = 0x0000FFFF & ((0x1<<17)|(addr[4] | (addr[5] << 8)));
	if(idx < 3)
		return Y_FAILURE;

	MAC_MA1_31LR_RgWr_async(idx+3, (addr[0] | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24)));
	MAC_MA1_31HR_RgWr_async(idx+3, HR);
	MAC_MA1_31HR_RgWr_async(idx+3, HR | (0x1<<31));

	return Y_SUCCESS;
}

/*!
 *  * \details This function is invoked by ioctl function when the user issues an
 *  ioctl command to select the AVB algorithm. This function also configures other
 *  parameters like send and idle slope, high and low credit.
 *
 *  \param[in] pdata – pointer to private data structure.
 *  \param[in] req – pointer to ioctl data structure.
 *
 *  \return void
 *
 *  \retval none
 */
static int DWC_ETH_QOS_program_avb_algorithm(struct ifr_data_struct *req)
{
        struct DWC_ETH_QOS_avb_algorithm l_avb_struct, *u_avb_struct =
                (struct DWC_ETH_QOS_avb_algorithm *)req->ptr;

	DBGPR("-->DWC_ETH_QOS_program_avb_algorithm\n");

        if(copy_from_user(&l_avb_struct, u_avb_struct,
                                sizeof(struct DWC_ETH_QOS_avb_algorithm)))
                printk(KERN_ALERT "Failed to fetch AVB Struct info from user\n");

        set_tx_queue_operating_mode(l_avb_struct.chInx,
                (u32)l_avb_struct.op_mode);
        set_avb_algorithm(l_avb_struct.chInx, l_avb_struct.algorithm);
        config_credit_control(l_avb_struct.chInx, l_avb_struct.cc);

	if (1000 == nic_speed) {
		config_send_slope(l_avb_struct.chInx, l_avb_struct.speed1000params.send_slope);
		config_idle_slope(l_avb_struct.chInx, l_avb_struct.speed1000params.idle_slope);
		config_high_credit(l_avb_struct.chInx, l_avb_struct.speed1000params.hi_credit);
		config_low_credit(l_avb_struct.chInx, l_avb_struct.speed1000params.low_credit);
	} else {
		config_send_slope(l_avb_struct.chInx, l_avb_struct.speed100params.send_slope);
		config_idle_slope(l_avb_struct.chInx, l_avb_struct.speed100params.idle_slope);
		config_high_credit(l_avb_struct.chInx, l_avb_struct.speed100params.hi_credit);
		config_low_credit(l_avb_struct.chInx, l_avb_struct.speed100params.low_credit);
	}
	DBGPR("Setting CBS for speed %d, class %s\n",
		nic_speed, l_avb_struct.chInx == 1 ? "A" : "B");
        return Y_SUCCESS;
}

/*!
 * \brief This sequence is used to write into phy registers
 * \param[in] phy_id
 * \param[in] phy_reg
 * \param[in] phy_reg_data
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int write_phy_regs(int phy_id, int phy_reg, int phy_reg_data)
{

        unsigned long retryCount = 1000;
        unsigned long vy_count;
        volatile unsigned long varMAC_GMIIAR;


        /* wait for any previous MII read/write operation to complete */

        /*Poll Until Poll Condition */
        vy_count = 0;
        while (1) {
                if (vy_count > retryCount) {
                        return -Y_FAILURE;
                } else {
                        vy_count++;
                        mdelay(1);
                }
                MAC_GMIIAR_RgRd(varMAC_GMIIAR);
                if (GET_VALUE(varMAC_GMIIAR, MAC_GMIIAR_GB_LPOS, MAC_GMIIAR_GB_HPOS) == 0) {
                        break;
                }
        }
        /* write the data */
        MAC_GMIIDR_GD_UdfWr(phy_reg_data);
        /* initiate the MII write operation by updating desired */
        /* phy address/id (0 - 31) */
        /* phy register offset */
        /* CSR Clock Range (20 - 35MHz) */
        /* Select write operation */
        /* set busy bit */
        MAC_GMIIAR_RgRd(varMAC_GMIIAR);
        varMAC_GMIIAR = varMAC_GMIIAR & (unsigned long) (0x12);
        varMAC_GMIIAR =
            varMAC_GMIIAR | ((phy_id) << 21) | ((phy_reg) << 16) | ((0x2) << 8)
            | ((0x1) << 2) | ((0x1) << 0);
        MAC_GMIIAR_RgWr(varMAC_GMIIAR);

        /*DELAY IMPLEMENTATION USING udelay() */
        udelay(10);
        /* wait for MII write operation to complete */

        /*Poll Until Poll Condition */
        vy_count = 0;
        while (1) {
                if (vy_count > retryCount) {
                        return -Y_FAILURE;
                } else {
                        vy_count++;
                        mdelay(1);
                }
                MAC_GMIIAR_RgRd(varMAC_GMIIAR);
                if (GET_VALUE(varMAC_GMIIAR, MAC_GMIIAR_GB_LPOS, MAC_GMIIAR_GB_HPOS) == 0) {
                        break;
                }
        }

        return Y_SUCCESS;
}



/*!
 * \brief This sequence is used to read the phy registers
 * \param[in] phy_id
 * \param[in] phy_reg
 * \param[out] phy_reg_data
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int read_phy_regs(int phy_id, int phy_reg, int *phy_reg_data)
{
        unsigned long retryCount = 1000;
        unsigned long vy_count;
        volatile unsigned long varMAC_GMIIAR;
        unsigned long varMAC_GMIIDR;

        /* wait for any previous MII read/write operation to complete */

        /*Poll Until Poll Condition */
        vy_count = 0;
        while (1) {
                if (vy_count > retryCount) {
                        return -Y_FAILURE;
                } else {
                        vy_count++;
                        mdelay(1);
                }
                MAC_GMIIAR_RgRd(varMAC_GMIIAR);
                if (GET_VALUE(varMAC_GMIIAR, MAC_GMIIAR_GB_LPOS, MAC_GMIIAR_GB_HPOS) == 0) {
                        break;
                }
        }
        /* initiate the MII read operation by updating desired */
        /* phy address/id (0 - 31) */
        /* phy register offset */
        /* CSR Clock Range (20 - 35MHz) */
        /* Select read operation */
        /* set busy bit */
        MAC_GMIIAR_RgRd(varMAC_GMIIAR);
        varMAC_GMIIAR = varMAC_GMIIAR & (unsigned long) (0x12);
        varMAC_GMIIAR =
            varMAC_GMIIAR | ((phy_id) << 21) | ((phy_reg) << 16) | ((0x2) << 8)
            | ((0x3) << 2) | ((0x1) << 0);
        MAC_GMIIAR_RgWr(varMAC_GMIIAR);

        /*DELAY IMPLEMENTATION USING udelay() */
        udelay(10);
        /* wait for MII write operation to complete */

        /*Poll Until Poll Condition */
        vy_count = 0;
        while (1) {
                if (vy_count > retryCount) {
                        return -Y_FAILURE;
                } else {
                        vy_count++;
                        mdelay(1);
                }
                MAC_GMIIAR_RgRd(varMAC_GMIIAR);
                if (GET_VALUE(varMAC_GMIIAR, MAC_GMIIAR_GB_LPOS, MAC_GMIIAR_GB_HPOS) == 0) {
                        break;
                }
        }
        /* read the data */
        MAC_GMIIDR_RgRd(varMAC_GMIIDR);
        *phy_reg_data =
            GET_VALUE(varMAC_GMIIDR, MAC_GMIIDR_GD_LPOS, MAC_GMIIDR_GD_HPOS);

        return Y_SUCCESS;
}


/*!
 * \brief write MII PHY register, function called by the driver alone
 * 
 * \details Writes MII registers through the API write_phy_reg where the
 * related MAC registers can be configured.
 * 
 * \param[in] pdata - pointer to driver private data structure.
 * \param[in] phyaddr - the phy address to write
 * \param[in] phyreg - the phy regiester id
 * 
 * to write
 * \param[out] phydata - actual data to be written into the phy registers
 * 
 * \return void
 * 
 * \retval  0 - successfully read data from register
 * \retval -1 - error occurred
 * \retval  1 - if the feature is not defined.
 */
int DWC_ETH_QOS_mdio_write_direct(int phyaddr, int phyreg, int phydata)
{
        int phy_reg_write_status;

        phy_reg_write_status = write_phy_regs(phyaddr, phyreg, phydata);
        phy_reg_write_status = 1;
        
        return phy_reg_write_status;
}


/*!
 * \brief read MII PHY register, function called by the driver alone
 * 
 * \details Read MII registers through the API read_phy_reg where the
 * related MAC registers can be configured.
 * 
 * \param[in] phyaddr - the phy address to read
 * \param[in] phyreg - the phy regiester id to read
 * \param[out] phydata - pointer to the value that is read from the phy registers
 * 
 * \return int
 * 
 * \retval  0 - successfully read data from register
 * \retval -1 - error occurred
 * \retval  1 - if the feature is not defined.
 */
int DWC_ETH_QOS_mdio_read_direct(int phyaddr, int phyreg, int *phydata)
{
        int phy_reg_read_status;

        TR("--> DWC_ETH_QOS_mdio_read_direct\n");

	phy_reg_read_status = read_phy_regs(phyaddr, phyreg, phydata);
	phy_reg_read_status = 1;

	TR("<-- DWC_ETH_QOS_mdio_read_direct\n");

        return phy_reg_read_status;
}



/*!
 *  \details This function is invoked by ioctl function when user issues
 *  an ioctl command to enable/disable phy loopback mode.
 *    
 *  \param[in] dev pointer to net device structure.
 *  \param[in] flags flag to indicate whether mac loopback mode to be
 *  enabled/disabled.
 *  
 *  \return integer
 *  
 *  \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_phy_loopback_mode(unsigned int flags)
{
        int ret = 0, regval;
	int phyaddr = 0x01;

        TR("-->DWC_ETH_QOS_config_phy_loopback_mode\n");
	TR("value of flag %d\n",flags);

        if (flags && phy_loopback_mode) {
                TR(KERN_ALERT
                        "PHY loopback mode is already enabled\n");
                return -EINVAL;
        }
        if (!flags && !phy_loopback_mode) {
                TR(KERN_ALERT
                        "PHY loopback mode is already disabled\n");
                return -EINVAL;
        }
        phy_loopback_mode = !!flags;

        DWC_ETH_QOS_mdio_read_direct(phyaddr, MII_BMCR, &regval);
        regval = (regval & (~(1<<14))) | (flags<<14);
        DWC_ETH_QOS_mdio_write_direct(phyaddr, MII_BMCR, regval);

        TR(KERN_ALERT "Succesfully %s PHY loopback mode\n",
                (flags ? "enabled" : "disabled"));

        TR("<--DWC_ETH_QOS_config_phy_loopback_mode\n");

        return ret;
}

/*!
 * \details This function is invoked by ioctl function when user issues an
 * ioctl command to configure L2 destination addressing filtering mode. This
 * function dose following,
 * - selects perfect/hash filtering.
 * - selects perfect/inverse matching.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to IOCTL specific structure.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_confing_l2_da_filter(struct net_device *net,	struct ifr_data_struct *req)
{
	struct usbnet *dev = netdev_priv(net);
	struct ntn_data *priv = (struct ntn_data *)dev->data[0]; 
	
	struct DWC_ETH_QOS_l2_da_filter *u_l2_da_filter =
	  (struct DWC_ETH_QOS_l2_da_filter *)req->ptr;
	struct DWC_ETH_QOS_l2_da_filter l_l2_da_filter;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_confing_l2_da_filter\n");

	if (copy_from_user(&l_l2_da_filter, u_l2_da_filter,
	      sizeof(struct DWC_ETH_QOS_l2_da_filter)))
		return - EFAULT;

	if (l_l2_da_filter.perfect_hash) {
			priv->l2_filtering_mode = 1;
	} else {
			priv->l2_filtering_mode = 0;
	}

	/* configure L2 DA perfect/inverse_matching */
	config_l2_da_perfect_inverse_match(priv,l_l2_da_filter.perfect_inverse_match);

	DBGPR("Successfully selected L2 %s filtering and %s DA matching\n",
		(l_l2_da_filter.perfect_hash ? "HASH" : "PERFECT"),
		(l_l2_da_filter.perfect_inverse_match ? "INVERSE" : "PERFECT"));

	DBGPR("<--DWC_ETH_QOS_confing_l2_da_filter\n");

	return ret;
}

/*!
 * \brief This function is used to enable or disable the Neutrino Wrapper Timestamp Packet Blocking feature
 * \param[in] ena_dis, 1=enable, 0=disable
 * \return Success or Failure
 * \retval  0 Success
 * \retval -1 Failure
 */
static int ntn_wrap_ts_ignore_config(int ena_dis)
{
    int rd_val;

    ETH_AVB_WRAPPER_TS_CTRL_UdfWr(ena_dis);
    ETH_AVB_WRAPPER_TS_CTRL_UdfRd(rd_val);

    if(rd_val != ena_dis)
    {
        printk(KERN_ALERT "ERROR: NTN Wrapper Timestamp Enable Feature wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return -Y_SUCCESS;
    }else{
        DBGPR("NTN Wrapper Timestamp Enable Feature : wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return Y_SUCCESS;
    }
}


/*!
 *  \brief Register read/write function called by driver IOCTL routine
 *   	 This function doesn't do address validation, it is application's resposibility 
 *       to make sure a valid address is passed in.
 *    
 *  \param[in] pdata : pointer to private data structure.
 *  \param[in] req : pointer to ioctl structure.
 *       req->flags = 0: Neutrino Register Read
 *  	 req->flags = 1: Neutrino Register Write
 *       req->flags = 2: PCIe config Register Read
 *       req->flags = 3: PCIe config Register Write
 *       req->adrs : register address
 *       req->ptr : pointer to data
 *  \retval 0: Success, -1 : Failure 
 */
static int DWC_ETH_QOS_RD_WR_REG(struct ifr_data_struct *req)
{
        unsigned int ret = -1;

        switch(req->flags)
        {
                case NTN_REG_RD: /* Neutrino Register Read */
                        ntn_read(req->adrs,req->ptr);
                        ret = 0;
                        break;
                case NTN_REG_WR: /* Neutrino Register Write */
                        ntn_write(req->adrs, *(unsigned int*)req->ptr);
                        ret = 0;
                        break;
                default:
                        ret = -1;
                        break;
        }
        return ret;
}


static void config_rx_outer_vlan_stripping(u32 cmd)
{
	MAC_VLANTR_EVLS_UdfWr(cmd);
}
/*!
 * \brief This function confiures the TDM path for TDM-Ethernet data transfer. 
 * \param[in] pdata : pointer to private data structure.
 * \param[in] req : pointer to ioctl structure.
 *
 * \retval 0: Success, -1 : Failure
 **/
static int NTN_TDM_Config(struct ifr_data_struct *req)
{
    struct NTN_TDM_Config *ntn_tdm_cfg = (struct NTN_TDM_Config *)req->ptr;
	u32 reg_val;
	unsigned char* ptr;
	unsigned int int_mask_val;
	int i;

	//Assert TDM reset	
	ntn_read(0x1008, &reg_val);
	reg_val |= 0x1<<6;
	ntn_write(0x1008, reg_val);

	/* Stop TDM */
	if(ntn_tdm_cfg->tdm_start == 0) {
     		
		DBGPR("Disabled TDM path\n"); 

		//Disable TDM clock
		ntn_read(0x1004, &reg_val);
		reg_val &= ~(0x1<<6);
		ntn_write(0x1004, reg_val);
		
		/* Disable TDM interrupt in INTC Module */
#ifdef NTN_INT_INTX
		NTN_INTC_INTINTXMASK1_RgRd(int_mask_val);
		int_mask_val |= (0x7F<<22);	//Disable all TDM interrupts 
		NTN_INTC_INTINTXMASK1_RgWr(int_mask_val);
#else
		NTN_INTC_INTMCUMASK1_RgRd(int_mask_val);
		int_mask_val |= (0x7F<<22);	//Disable all TDM interrupts 
		NTN_INTC_INTMCUMASK1_RgWr(int_mask_val);
#endif
		return 0;
	}

   	/* Enable TDM interrupt in INTC Module */
#ifdef NTN_INT_INTX
	NTN_INTC_INTINTXMASK1_RgRd(int_mask_val);
	int_mask_val &= ~(0x7F<<22); //Enable all interrupts
	NTN_INTC_INTINTXMASK1_RgWr(int_mask_val);
#else
	NTN_INTC_INTMCUMASK1_RgRd(int_mask_val);
	int_mask_val &= ~(0x7F<<22); //Enable all interrupts
	NTN_INTC_INTMCUMASK1_RgWr(int_mask_val);
#endif

	//Enable TDM clock
	ntn_read(0x1004, &reg_val);
	reg_val |= 0x1<<6;
	ntn_write(0x1004, reg_val);
	
	//Deassert TDM reset	
	ntn_read(0x1008, &reg_val);
	reg_val &= ~(0x1<<6);
	ntn_write(0x1008, reg_val);

	/* Start TDM */
	DBGPR("TDM Path Configuration\n");
	DBGPR("    Sample rate = %d\n", ntn_tdm_cfg->sample_rate);
	DBGPR("    No of channels = %d\n", ntn_tdm_cfg->channels);
	DBGPR("    Mode selected  = %d\n", ntn_tdm_cfg->mode_sel);
	DBGPR("    Protocol selected = %d\n", ntn_tdm_cfg->protocol);
	DBGPR("    Class priority  = %d\n", ntn_tdm_cfg->a_priority);
	DBGPR("    Direction selected = %d\n", ntn_tdm_cfg->direction);
	DBGPR("    Class vid = %d\n", ntn_tdm_cfg->a_vid);

	switch(ntn_tdm_cfg->sample_rate){
		case 48000:
			ntn_write(0x449c, 0x02dc6c08);//TDM DDA Control (M value)  
			ntn_write(0x44a0, 0x00000C00);//T0EVB_DDANRatio (N value)
			ntn_write(0x44e0, 0x0000760C);//T0EVB_DDACtrl2  (PLL output divider)
			ntn_write(0x44e4, 0x01fff700);//DDA_PLL_Ctrl1  
			ntn_write(0x44e8, 0x0000f300);//DDA_PLL_Ctrl2  (BCLK and MCLK divider)
			ntn_write(0x44ec, 0x00000000);//DDA_PLL_UPDT
			ntn_tdm_cfg->fdf = 2;
       			break;
		default:
			break;	
	}

	/*Clock & Reset*/
	ntn_write(0x0010, 0x3030008);//Neutrino eMAC DIV

	/*EMAC RX*/
	/* TDM Source MAC ID */
	ptr = ntn_tdm_cfg->TDM_SRC_ID;	
	DBGPR("SRC: \n");
	for(i=0;i<sizeof(ntn_tdm_cfg->TDM_SRC_ID); i++)
		DBGPR("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_SRC_ID[i]);

	reg_val = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
	ntn_write(0x3010, reg_val);//TDM Header Ethernet MAC Source Low Address Register 
	reg_val = (ptr[5] << 8) | (ptr[4] << 0);
	ntn_write(0x3014, reg_val);//TDM Header Ethernet MAC Source High Address Register 

	ntn_write(0xab98, 0x00003D08);//MAC_PPS0_Width : 125 us
	ntn_write(0xab70, 0x00000212);//MAC_PPS_Control : Set for only PPS0 output
	
	 
	/*TDM*/
	if(ntn_tdm_cfg->direction == NTN_TDM_IN)
	{
		ntn_write(0x4414, 0x08100058);//TDM conf0 

		ntn_read(0x3000, &reg_val);
		reg_val |= (ntn_tdm_cfg->a_priority&0x7)<<2;
		reg_val |= (ntn_tdm_cfg->fdf&0xFF)<<8;
		ntn_write(0x3000, reg_val);//TDM Control Register

   		/* config stream id */
        	ptr = ntn_tdm_cfg->TDM_STREAM_ID;
		DBGPR("Stream: \n");
		for(i=0;i<sizeof(ntn_tdm_cfg->TDM_STREAM_ID); i++)
			DBGPR("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_STREAM_ID[i]);
	
        	reg_val = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
		ntn_write(0x4418, reg_val);//TDM Stream ID Low 
        	reg_val = (ptr[7] << 24) | (ptr[6] << 16) | (ptr[5] << 8) | ptr[4];
		ntn_write(0x441c, reg_val);//TDM Stream ID Hi 

		/* TDM Destination MAC ID */
		ptr = ntn_tdm_cfg->TDM_DST_ID;	
		DBGPR("DST: \n");
		for(i=0;i<sizeof(ntn_tdm_cfg->TDM_DST_ID); i++)
			DBGPR("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_DST_ID[i]);
	
		reg_val = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
		ntn_write(0x4420, reg_val);//TDM Stream ID Low 
		reg_val = (ptr[5] << 8) | (ptr[4] << 0);
		ntn_write(0x4424, reg_val);//TDM Stream ID Low 
	
		reg_val = 0x40005800;
		reg_val |= (ntn_tdm_cfg->channels & 0xF)<<16;
		reg_val |= (ntn_tdm_cfg->protocol & 0x1)<<15;
		ntn_write(0x4410, reg_val);//TDM Control Register

		reg_val = 0x2003072C;
		reg_val |= (ntn_tdm_cfg->mode_sel & 0x1)<<31;
		ntn_write(0x4400, reg_val);//TDM Control Register
	
		ntn_write(0x100C, 0x00002600);//Pin Mux control 
		DBGPR("TDM Out configured\n");
	
#ifndef NTN_DRV_TEST_LOOPBACK
	} else {
#endif
		/* Disable VLAN striping */
        config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP);

		ntn_write(0x100C, 0x00002600);//Pin Mux control 
		ntn_write(0x4490, 0x01F00213);//Pin Mux control 
		ntn_write(0x44b8, 0x00000000);//Pin Mux control 

		/* config stream id */
		ptr = ntn_tdm_cfg->TDM_STREAM_ID;
		DBGPR("Stream: \n");
		for(i=0;i<sizeof(ntn_tdm_cfg->TDM_STREAM_ID); i++)
			DBGPR("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_STREAM_ID[i]);

		reg_val = (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
		ntn_write(0x3084, reg_val);//TDM Stream ID Hi
		reg_val = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
		ntn_write(0x3088, reg_val);//TDM Stream ID Low

		ntn_write(0x44CC, 0x00CC0133);//TDM buffer near underflow threshold register 
		ntn_write(0x44D0, 0x02CC0333);//TDM buffer near overflow threshold register 

		reg_val = 0x6003072C;
		reg_val |= (ntn_tdm_cfg->mode_sel & 0x1)<<31;
		ntn_write(0x4400, reg_val);//TDM Control Register
		DBGPR("TDM In configured\n");
	}

	return 0;
}

/*!
 *  \brief Driver IOCTL routine
 *  \details This function is invoked by main ioctl function when
 *  users request to configure various device features like,
 *  PMT module, TX and RX PBL, TX and RX FIFO threshold level,
 *  TX and RX OSF mode, SA insert/replacement, L2/L3/L4 and
 *  VLAN filtering, AVB/DCB algorithm etc.
 *  \param[in] pdata – pointer to private data structure.
 *  \param[in] req – pointer to ioctl structure.
 *  \return int
 *  \retval 0 - success
 *  \retval negative - failure
 **/
static int DWC_ETH_QOS_handle_prv_ioctl(struct net_device *net, struct ifr_data_struct *req)
{
		struct usbnet *dev = netdev_priv(net);
		struct ntn_data *priv = (struct ntn_data *)dev->data[0]; 
        unsigned int chInx = req->chInx;
        int ret = 0;

        if (chInx > NTN_QUEUE_CNT) {
                printk(KERN_ALERT "Queue number %d is invalid\n" \
                                "Hardware has only %d Tx/Rx Queues\n",
                                chInx, NTN_QUEUE_CNT);
                ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		req->connected_speed = nic_speed;
		DBGPR("%s nic speed %x\n", __func__, req->connected_speed);
                return ret;
        }

	DBGPR("%s cmd %x\n", __func__, req->cmd);

        switch (req->cmd) {
        case DWC_ETH_QOS_AVB_ALGORITHM:
		ret = DWC_ETH_QOS_program_avb_algorithm(req);

                break;
	
	case DWC_ETH_QOS_GET_CONNECTED_SPEED:
		/*ret = ntn_read(MAC_OFFSET, &mac_config);
		if((mac_config & 0xC000) == 0)
			req->connected_speed = SPEED_1000;
		else if ((mac_config & 0xC000) == 0xC000)
			req->connected_speed = SPEED_100;
		else if ((mac_config & 0xC000) == 0x8000)
			req->connected_speed = SPEED_10;*/
		req->connected_speed = priv->Speed;
		break;

	case DWC_ETH_QOS_GET_RX_QCNT:
		req->chInx = NTN_RX_QUEUE_CNT;
		break;

	case DWC_ETH_QOS_GET_TX_QCNT:
		req->chInx = NTN_TX_QUEUE_CNT;
		break;
	
	case DWC_ETH_QOS_L2_DA_FILTERING_CMD:
		ret = DWC_ETH_QOS_confing_l2_da_filter(net, req);
		break;

	case DWC_ETH_QOS_PHY_LOOPBACK_MODE_CMD:
               ret = DWC_ETH_QOS_config_phy_loopback_mode(req->flags);
                if (ret == 0)
                        ret = DWC_ETH_QOS_CONFIG_SUCCESS;
                else
                        ret = DWC_ETH_QOS_CONFIG_FAIL;
                break;

	case DWC_WRAP_TS_FEATURE:
		ntn_wrap_ts_ignore_config(req->flags);
       	 	printk(KERN_ALERT "Neutrino Wrapper TS Feature Value: %d\n", req->flags);
        	break;

        case NTN_DWC_REG_RD_WR_CMD:
                ret = DWC_ETH_QOS_RD_WR_REG((void*)req);
                break;

	case NTN_DWC_TDM_CONFIG_CMD:
		ret = NTN_TDM_Config((void*)req);
		break;

	}
	DBGPR("%s ret %x\n", __func__, ret);

	return ret;
}

/*!
 * \details This function gets the PHC index
 * \param[in] dev <96> pointer to net device structure.
 * \param[in] ethtool_ts_info <96> pointer to ts info structure.
 *
 * \return int
 *
 * \retval +ve(>0) on success, 0 if that string is not
 * \defined and -ve on failure.
 */
static int DWC_ETH_QOS_get_ts_info(struct net_device *dev,
                           struct ethtool_ts_info *info)
{
    DBGPR("-->DWC_ETH_QOS_get_ts_info\n");
    info->phc_index = ntn_phc_index(pdata_phc);
    DBGPR("PHC index = %d\n", info->phc_index);
    DBGPR("<--DWC_ETH_QOS_get_ts_info\n");
    return 0;
}


#ifndef NTN_DRV_TEST_LOOPBACK
/**
 * Function used to detect the cable plugging and unplugging.
 * This function gets scheduled once in every second and polls
 * the PHY register for network cable plug/unplug. Once the 
 * connection is back the GMAC device is configured as per
 * new Duplex mode and Speed of the connection.
 * @param[in] u32 type but is not used currently. 
 * \return returns void.
 * \note This function is tightly coupled with Linux 2.6.xx.
 * \callgraph
 */
static void synopGMAC_linux_cable_unplug_function(void)
{
	s32 status;
	u16 data;
	u32 new_speed, new_duplex, new_linkstate;
	struct usbnet *dev = (struct usbnet *)synopGMAC_cable_unplug_timer.data;
	struct ntn_data *priv = (struct ntn_data *)dev->data[0]; 
	int new_state = 0;
	u32 phy_id = priv->phy_id;

	if(0 != synopGMAC_read_phy_reg(dev, phy_id, PHY_CTRL_STATUS_REG, &data)){
		printk("%s: Error while reading Phy control status register \n",__func__);
	}

	/* Get New Phy duplex mode */
	new_duplex = (data & Mii_ctrl_phy_status_duplex) ? FULLDUPLEX:HALFDUPLEX;

	/* Get New Phy speed */
	if(data & Mii_ctrl_phy_status_speed_1000)
		new_speed = SPEED_1000;
	else if(data & Mii_ctrl_phy_status_speed_100)
		new_speed = SPEED_100;
	else
		new_speed = SPEED_10;
	

	if(0 != synopGMAC_read_phy_reg(dev, phy_id, PHY_STATUS_REG, &data)){
		printk("%s: Error while reading Phy status register \n",__func__);
	}
	/* Get new link status */
	new_linkstate = (data & Mii_phy_status_link_up) ? LINKUP : LINKDOWN;

	if (new_linkstate == LINKUP) {
		/* Link is active */
		/* Check if duplex mode is changed */
		if (new_duplex != priv->DuplexMode) {
			new_state = 1;
			if (new_duplex)
				synopGMAC_set_full_duplex(dev);	
			else
				synopGMAC_set_half_duplex(dev);	
			priv->DuplexMode = new_duplex;
		}

		/* Check if speed is changed */
		if (new_speed != priv->Speed) {
			new_state = 1;
			priv->Speed = new_speed;
			synopGMAC_set_speed(dev, priv->Speed);
		}

		/* Restart if previous link was down. */
		if(new_linkstate != priv->LinkState) {
			new_state = 1;
			priv->LinkState = new_linkstate;
			netif_carrier_on(dev->net);
			usbnet_defer_kevent(dev,EVENT_LINK_RESET);
		}

	} else {
		if(new_linkstate != priv->LinkState) {
			/* Link is down */
			new_state = 1;
			priv->DuplexMode = -1;
		    priv->Speed = 0;
		    priv->LoopBackMode = 0; 
			priv->LinkState = new_linkstate;
		    netif_carrier_off(dev->net);
	    }
    }

	if (new_state){
		if (priv->LinkState == LINKUP) {
			printk("NTN HSIC Link is Up - %s/%s\n",
					((priv->Speed == SPEED_10) ? "10Mbps": ((priv->Speed == SPEED_100) ? "100Mbps": "1Gbps")),
					(priv->DuplexMode == FULLDUPLEX) ? "Full Duplex": "Half Duplex");
		} else {
			printk("NTN HSIC Link is Down\n");
	    }
    }

	schedule_delayed_work(&task, msecs_to_jiffies(NTN_PHY_DETECT_INTERVAL));
}
#endif

static void neutrino_status(struct usbnet *dev, struct urb *urb)
{
	struct ntn_int_data *event;
	int link;

	if (urb->actual_length < 8)
		return;

	event = urb->transfer_buffer;
	link = event->link & NTN_INT_PPLS_LINK;

	if (netif_carrier_ok(dev->net) != link) {
		if (link)
			usbnet_defer_kevent(dev, EVENT_LINK_RESET);
		else
			netif_carrier_off(dev->net);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
		netdev_info(dev->net, "NTN - Link status is: %d\n",
				link);
#else
		devinfo(dev, "NTN - Link status is: %d\n", link);
#endif
	}
}

static int neutrino_mdio_read(struct net_device *netdev, int phy_id, int loc)
{
	struct usbnet *dev = netdev_priv(netdev);
	u32 ret;
	u16 reg_val;

	ret = synopGMAC_read_phy_reg(dev, phy_id, loc, &reg_val);
	CHECK(ret, "synopGMAC_read_phy_reg");
	DBGPR ("%s phy 0x%x loc 0x%x regval 0x%x result 0x%x\n", __func__,
			 phy_id, loc, reg_val, ret);

	return reg_val;
}

static void neutrino_mdio_write(struct net_device *netdev, int phy_id, int loc,
		int val)
{
#if 0
	struct usbnet *dev = netdev_priv(netdev);

	DBGPR ("__func__ = %s\n", __func__);
	synopGMAC_write_phy_reg(dev, phy_id, loc, (u16)val);
	DBGPR ("%s phy 0x%x loc 0x%x regval 0x%x\n", __func__,
		phy_id, loc, val);
#endif
}

static int neutrino_suspend(struct usb_interface *intf,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 10)
		pm_message_t message)
#else
u32 message)
#endif
{
	usbnet_suspend(intf, message);
	return 0;
}

static int neutrino_resume(struct usb_interface *intf)
{
	struct usbnet *dev = usb_get_intfdata(intf);
	u32 ret; 


	/* Power up ethernet PHY */
	ret = synopGMAC_check_phy_init(dev, 0);   
	if (ret < 0)
	{
		return ret;   
	}         

	return usbnet_resume(intf);
}


static void neutrino_get_drvinfo(struct net_device *net,
		struct ethtool_drvinfo *info)
{
	/* Inherit standard device info */
	DBGPR("-->neutrino_get_drvinfo\n");
	usbnet_get_drvinfo(net, info);
	info->eedump_len = 0x3e;
}

static int neutrino_get_settings(struct net_device *net, struct ethtool_cmd *cmd)
{
	struct usbnet *dev = netdev_priv(net);
	DBGPR("-->neutrino_get_settings\n");
	return mii_ethtool_gset(&dev->mii, cmd);
}

static int neutrino_set_settings(struct net_device *net, struct ethtool_cmd *cmd)
{
	struct usbnet *dev = netdev_priv(net);
	DBGPR("-->neutrino_set_settings\n");
	return mii_ethtool_sset(&dev->mii, cmd);
}

/**********************************************************/


static int neutrino_ioctl(struct net_device *net, struct ifreq *ifr, int cmd)
{
	struct usbnet *dev = netdev_priv(net);
	struct ntn_data *priv = NULL;
	s32 retval = 0, phc_index = 0;

	struct ifr_data_struct *req , reg_rw, req_pvt;


	if(ifr == NULL)
	{
		return -EINVAL;
	}

	req = (struct ifr_data_struct *)ifr->ifr_data;

	priv = (struct ntn_data *)dev->data[0];
	

	DBGPR("%s cmd::: %x\n", __func__, cmd);
	switch(cmd)
	{
		case DWC_ETH_QOS_PRV_IOCTL:
			if (copy_from_user(&req_pvt, req, sizeof(struct ifr_data_struct))) {
				printk(KERN_ERR "copy_from_user error: ifr structure\n");
				return -EINVAL;
			}
			DWC_ETH_QOS_handle_prv_ioctl(net, &req_pvt);
			req_pvt.command_error = retval;
			break;
		/* IOCTL to read GMAC registers */
		case IOCTL_READ_REGISTER:
			retval = ntn_reg_read(dev, reg_rw.addr, &reg_rw.data);

			if(copy_to_user(ifr->ifr_data, &reg_rw.data, sizeof(reg_rw.data)))
				printk("ERROR : retval = %d\n",retval);

			CHECK(retval, "IOCTL_READ_REGISTER: ntn_reg_read failed");
			break;

		/* IOCTL to write GMAC registers */
		case IOCTL_WRITE_REGISTER:
			retval = copy_from_user(&reg_rw, req, sizeof(struct ifr_data_struct));

			if(retval)
			{
				printk("copy_from_user error: ifr structure\n");
				retval = -EINVAL;
			}
			retval = ntn_reg_write(dev,reg_rw.addr,reg_rw.data);

			CHECK(retval, "IOCTL_WRITE_REGISTER: ntn_reg_write failed");
			break;

		case SIOCSHWTSTAMP:
			retval = ntn_ptp_hwtstamp_ioctl(net, ifr,  cmd);
			CHECK(retval, "SIOCSHWTSTAMP: ntn_ptp_hwtstamp_ioctl failed");
			break;

		case SIOCETHTOOL:
			if(copy_to_user(ifr->ifr_data, &phc_index, sizeof(phc_index)))
				printk("IOCTL(SIOCETHTOOLNTN) copy_to_user failed\n");
			break;


		default:
			return  generic_mii_ioctl(&dev->mii, if_mii(ifr), cmd, NULL);
	}
	DBGPR("%s ret %x\n", __func__, retval);
	return retval;

}

/*!
* \brief API to configure the multicast address in device.
*
* \details This function collects all the multicast addresse
* and updates the device.
*
* \param[in] dev - pointer to net_device structure.
*
* \retval 0 if perfect filtering is seleted & 1 if hash
* filtering is seleted.
*/
static int DWC_ETH_QOS_prepare_mc_list(struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);
	struct ntn_data *priv = (struct ntn_data *)dev->data[0]; 
	u32 mc_filter[DWC_ETH_QOS_HTR_CNT];
	struct netdev_hw_addr *ha = NULL;
	int crc32_val = 0;
	int ret = 0, i = 1;

	DBGPR("-->DWC_ETH_QOS_prepare_mc_list\n");

	if (priv->l2_filtering_mode) {
		DBGPR("select HASH FILTERING for mc addresses: mc_count = %d\n",
				netdev_mc_count(net));
		ret = 1;
		memset(mc_filter, 0, sizeof(mc_filter));

		netdev_for_each_mc_addr(ha, net) {
			DBGPR("mc addr[%d] = %x:%x:%x:%x:%x:%x\n",i++,
					ha->addr[0], ha->addr[1], ha->addr[2],
					ha->addr[3], ha->addr[4], ha->addr[5]);
			/* The upper 6 bits of the calculated CRC are used to
			 * index the content of the Hash Table Reg 0 and 1.
			 * */
			crc32_val =
				(bitrev32(~crc32_le(~0, ha->addr, 6)) >> 26);
			/* The most significant bit determines the register
			 * to use (Hash Table Reg X, X = 0 and 1) while the
			 * other 5(0x1F) bits determines the bit within the
			 * selected register
			 * */
			mc_filter[crc32_val >> 5] |= (1 << (crc32_val & 0x1F));
		}
		for (i = 0; i < DWC_ETH_QOS_HTR_CNT; i++)
			update_hash_table_reg(i, mc_filter[i]);

	} else {
		DBGPR("select PERFECT FILTERING for mc addresses, mc_count = %d, max_addr_reg_cnt = %d\n",
				netdev_mc_count(net), NTN_MAX_ADDR_REG_CNT);

		netdev_for_each_mc_addr(ha, net) {
			DBGPR("mc addr[%d] = %x:%x:%x:%x:%x:%x\n", i,
					ha->addr[0], ha->addr[1], ha->addr[2],
					ha->addr[3], ha->addr[4], ha->addr[5]);
			if (i < 32)
				update_mac_addr3_31_low_high_reg(i, ha->addr);
			else
				update_mac_addr32_127_low_high_reg(i, ha->addr);
			i++;
		}
	}

	DBGPR("<--DWC_ETH_QOS_prepare_mc_list\n");

	return ret;
}

/*
* \brief API to configure the unicast address in device.
*
* \details This function collects all the unicast addresses
* and updates the device.
*
* \param[in] net - pointer to net_device structure.
*
* \retval 0 if perfect filtering is seleted  & 1 if hash
* filtering is seleted.
*/
static int DWC_ETH_QOS_prepare_uc_list(struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);
	struct ntn_data *priv = (struct ntn_data *)dev->data[0]; 
	u32 uc_filter[DWC_ETH_QOS_HTR_CNT];
	struct netdev_hw_addr *ha = NULL;
	int crc32_val = 0;
	int ret = 0, i = 1;

	DBGPR("-->DWC_ETH_QOS_prepare_uc_list\n");

	if (priv->l2_filtering_mode) {
		DBGPR("select HASH FILTERING for uc addresses: uc_count = %d\n",
				netdev_uc_count(net));
		ret = 1;
		memset(uc_filter, 0, sizeof(uc_filter));

		netdev_for_each_uc_addr(ha, net) {
			DBGPR("uc addr[%d] = %#x:%#x:%#x:%#x:%#x:%#x\n",i++,
					ha->addr[0], ha->addr[1], ha->addr[2],
					ha->addr[3], ha->addr[4], ha->addr[5]);
			crc32_val =
				(bitrev32(~crc32_le(~0, ha->addr, 6)) >> 26);
			uc_filter[crc32_val >> 5] |= (1 << (crc32_val & 0x1F));
		}
		

		// configure hash value of real/default interface also 
		DBGPR("real/default dev_addr = %x:%x:%x:%x:%x:%x\n",
				net->dev_addr[0], net->dev_addr[1], net->dev_addr[2],
				net->dev_addr[3], net->dev_addr[4], net->dev_addr[5]);
				
			crc32_val =
				(bitrev32(~crc32_le(~0, net->dev_addr, 6)) >> 26);
			uc_filter[crc32_val >> 5] |= (1 << (crc32_val & 0x1F));

		for (i = 0; i < DWC_ETH_QOS_HTR_CNT; i++)
			update_hash_table_reg(i, uc_filter[i]);

	} else {
		DBGPR("select PERFECT FILTERING for uc addresses: uc_count = %d\n",
				netdev_uc_count(net));

		netdev_for_each_uc_addr(ha, net) {
			DBGPR("uc addr[%d] = %x:%x:%x:%x:%x:%x\n", i,
					ha->addr[0], ha->addr[1], ha->addr[2],
					ha->addr[3], ha->addr[4], ha->addr[5]);
			if (i < 32)
				update_mac_addr3_31_low_high_reg(i, ha->addr);
			else
				update_mac_addr32_127_low_high_reg(i, ha->addr);
			i++;
		}
	}

	DBGPR("<--DWC_ETH_QOS_prepare_uc_list\n");

	return ret;
}


static struct ethtool_ops neutrino_ethtool_ops = {
	.get_drvinfo		= neutrino_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_msglevel		= usbnet_get_msglevel,
	.set_msglevel		= usbnet_set_msglevel,
	.get_settings		= neutrino_get_settings,
	.set_settings		= neutrino_set_settings,
	.get_ts_info 		= DWC_ETH_QOS_get_ts_info,
};

/*!
* \brief API to set the device receive mode
*
* \details The set_multicast_list function is called when the multicast list
* for the device changes and when the flags change.
*
* \param[in] net - pointer to net_device structure.
*
* \return void
*/

static void neutrino_set_rx_mode(struct net_device *net)
{
	unsigned char pr_mode = 0;
	unsigned char huc_mode = 0;
	unsigned char hmc_mode = 0;
	unsigned char pm_mode = 0;
	unsigned char hpf_mode = 0;
	struct usbnet *dev = netdev_priv(net);
	struct ntn_data *priv = (struct ntn_data *)dev->data[0];
	int mode, i;

	DBGPR("-->DWC_ETH_QOS_set_rx_mode \n");

	if (net->flags & IFF_PROMISC) {
		DBGPR("PROMISCUOUS MODE (Accept all packets irrespective of DA)\n");
		pr_mode = 1;
	} 
	else if ((net->flags & IFF_ALLMULTI) ||
			(netdev_mc_count(net) > NTN_MAX_HASH_TABLE_SIZE)) {
		DBGPR("pass all multicast pkt\n");
		pm_mode = 1;
		for (i = 0; i < DWC_ETH_QOS_HTR_CNT; i++)
			update_hash_table_reg(i, 0xffffffff);
	} 
	else if (!netdev_mc_empty(net)) {
		DBGPR("pass list of multicast pkt\n");
		if ((netdev_mc_count(net) > (NTN_MAX_ADDR_REG_CNT - 1)) &&
			(!NTN_MAX_HASH_TABLE_SIZE)) {
		DBGPR("PROMISCUOUS MODE (multicast)\n");
			// switch to PROMISCUOUS mode 
			pr_mode = 1;
		} else {
			mode = DWC_ETH_QOS_prepare_mc_list(net);
			if (mode) {
		DBGPR("Hash filtering for multicast\n");
				// Hash filtering for multicast 
				hmc_mode = 1;
			} else {
		DBGPR("Perfect filtering for multicast\n");
				// Perfect filtering for multicast 
				hmc_mode = 0;
				hpf_mode = 1;
			}
		}
	}
	// Handle multiple unicast addresses 
	if ((netdev_uc_count(net) > (NTN_MAX_ADDR_REG_CNT - 1)) &&
			(!NTN_MAX_HASH_TABLE_SIZE)) {
		DBGPR("PROMISCUOUS MODE (unicast)\n");
		// switch to PROMISCUOUS mode 
		pr_mode = 1;
	} 
	else if (!netdev_uc_empty(net)) {
		DBGPR("pass all unicast pkt\n");
		mode = DWC_ETH_QOS_prepare_uc_list(net);
		if (mode) {
		DBGPR("Hash filtering for unicast\n");
			// Hash filtering for unicast 
			huc_mode = 1;
		} else {
		DBGPR("Perfect filtering for unicast\n");
			// Perfect filtering for unicast 
			huc_mode = 0;
			hpf_mode = 1;
		}
	}
	config_mac_pkt_filter_reg(priv, pr_mode, huc_mode, hmc_mode, pm_mode, hpf_mode);
	DBGPR("<--DWC_ETH_QOS_set_rx_mode\n");
}

static int neutrino_change_mtu(struct net_device *net, int new_mtu)
{
	struct usbnet *dev = netdev_priv(net);

	if (new_mtu <= 0 || new_mtu > 2000)
		return -EINVAL;

	net->mtu = new_mtu;
	dev->hard_mtu = net->mtu + net->hard_header_len;

	return 0;
}

static int neutrino_set_mac_addr(struct net_device *net, void *p)
{
	struct usbnet *dev = netdev_priv(net);
	struct sockaddr *addr = p;

	if (netif_running(net))
		return -EBUSY;
	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(net->dev_addr, addr->sa_data, ETH_ALEN);

	/* Set the MAC address */
	synopGMAC_set_mac_addr(dev, MAC_MA2HR_RgOffAddr, MAC_MA2LR_RgOffAddr, addr->sa_data); 

	TR("%s called \n",__FUNCTION__);
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 29)
static const struct net_device_ops neutrino_netdev_ops = {
	.ndo_open		= usbnet_open,
	.ndo_stop		= usbnet_stop,
	.ndo_start_xmit		= usbnet_start_xmit,
	.ndo_set_rx_mode 	= neutrino_set_rx_mode,
	.ndo_tx_timeout		= usbnet_tx_timeout,
	.ndo_change_mtu		= neutrino_change_mtu,
	.ndo_do_ioctl		= neutrino_ioctl,
	.ndo_set_mac_address	= neutrino_set_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};
#endif


static int neutrino_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int ret;
	u64 ptp_temp = 0;


#ifdef TEST_MAC_ADD
	int count; 
#endif

	struct ntn_data *priv = NULL;
	struct timespec now;	
	u64 temp1 = 0,temp2 = 0,diff = 0;
	struct ntn_hsic_plt_data *plt_data = NULL;
	u8 bus_id = 1;

	ntn_pdata.dev = dev;
	dev->data[0] = (unsigned long)kzalloc(sizeof(struct ntn_data), GFP_KERNEL);

	DBGPR("ntn_dbg: In %s\n", __func__);
	priv = (struct ntn_data *)dev->data[0];
	

	if(!priv)
	{
		printk("Unable to allocate struct ntn_data\n");        
		return -ENOMEM;
	}

	memset(priv, 0, sizeof(*priv));
	priv->udev = dev;
	priv->udev->intf = intf;
	/* Assignment of All Out Endpoints */
	priv->legacy_out =  usb_sndbulkpipe (dev->udev, 0x01);
	priv->avb_a_out = usb_sndbulkpipe (dev->udev, 0x02);
	priv->avb_b_out = usb_sndbulkpipe (dev->udev, 0x03);
	priv->gptp_out = usb_sndbulkpipe (dev->udev, 0x04);

	DBGPR("Neutrino OUT: Legacy = %x AVBA = %x AVBB = %x GPTP = %x\n",
		priv->legacy_out, priv->avb_a_out, priv->avb_b_out,
		priv->gptp_out);

	/* Assignment of All In Endpoints */
	priv->legacy_in =  usb_rcvbulkpipe (dev->udev, 0x05);
	priv->avb_in = usb_rcvbulkpipe (dev->udev, 0x06);
	priv->avb_control = usb_rcvbulkpipe(dev->udev, 0x07);
	priv->gptp_in = usb_rcvbulkpipe(dev->udev, 0x08);
	priv->l2_filtering_mode = 0;

	DBGPR("Neutrino IN: Legacy = %x AVBA = %x AVB control = %x GPTP = %x\n",
		priv->legacy_in, priv->avb_in, priv->avb_control,
		priv->gptp_in);

	MAC_MPFR_RgRd(priv->MAC_Packet_Filter);

	dev->in = priv->legacy_in;
	dev->out= priv->legacy_out;

	spin_lock_init(&priv->reg_lock);
	if (msg_enable != 0)
		dev->msg_enable = msg_enable;

	/*Lets read the version of ip in to device structure*/
	synopGMAC_read_version(dev);
	printk("Driver Version is %s\n", DRV_VERSION);

	/*Parse config.ini file for MAC address*/
	parse_config_file(bus_id);
	synopGMAC_set_mac_addr(dev, MAC_MA2HR_RgOffAddr, MAC_MA2LR_RgOffAddr, mac_addr);
	synopGMAC_get_mac_addr(dev, MAC_MA2HR_RgOffAddr, MAC_MA2LR_RgOffAddr, dev->net->dev_addr);
	ntn_reg_write(dev, 0x3004, 0x02FF0000);

	printk("Neutrino mac addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
			dev->net->dev_addr[0],
			dev->net->dev_addr[1],
			dev->net->dev_addr[2],
			dev->net->dev_addr[3],
			dev->net->dev_addr[4],
			dev->net->dev_addr[5]);

	memcpy(dev->net->perm_addr, dev->net->dev_addr, ETH_ALEN);

	/*Check for Phy initialization*/
	synopGMAC_set_mdc_clk_div(dev, GmiiCsrClk1);

	list_for_each_entry(plt_data, &ntn_hsic_plt_data_list, node){
		priv->phy_id = plt_data->ntn_phy_id;
	}
	/* Initialize MII structure */
	dev->mii.dev = dev->net;
	dev->mii.mdio_read = neutrino_mdio_read;
	dev->mii.mdio_write = neutrino_mdio_write;
	dev->mii.phy_id_mask = 0xff;                    
	dev->mii.reg_num_mask = 0xff;
	dev->mii.phy_id = priv->phy_id;
	dev->mii.supports_gmii = 1;

	ret = synopGMAC_check_phy_init(dev,0);

	nic_speed = 0;
	if(priv->Speed == 1)
		nic_speed = 10;
	if(priv->Speed == 2)
		nic_speed = 100;
	if(priv->Speed == 3)
		nic_speed = 1000;


#ifndef NTN_DRV_TEST_LOOPBACK
	ntn_reg_write(dev, MAC_MCR_RgOffAddr, 0x00332003);
#endif
	//synopGMAC_promisc_enable(dev);
	synopGMAC_set_speed(dev, priv->Speed); 

#ifdef IPC_OFFLOAD
	/* IPC Checksum offloading is enabled for this driver. Should only be used if Full Ip checksumm offload engine is configured in the hardware */
	synopGMAC_enable_rx_chksum_offload(dev);    /* Enable the offload engine in the receive path */

#endif


	/* The FEF bit in DMA control register is configured to 0 indicating DMA to drop the errored frames. */
	/* Inform the Linux Networking stack about the hardware capability of checksum offloading */
	dev->net->features = NETIF_F_HW_CSUM;

	TR("Setting up the cable unplug timer\n");
#ifndef NTN_DRV_TEST_LOOPBACK
		synopGMAC_cable_unplug_timer.data = (unsigned long)dev;
		INIT_DELAYED_WORK(&task, (void *)synopGMAC_linux_cable_unplug_function);
		schedule_delayed_work(&task, msecs_to_jiffies(NTN_PHY_DETECT_INTERVAL));
#endif
	dev->rx_urb_size = RX_URB_SIZE;
	dev->net->flags |= IFF_ALLMULTI;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
	dev->net->do_ioctl = neutrino_ioctl;
	dev->net->set_mac_address = neutrino_set_mac_addr;
	dev->net->change_mtu = neutrino_change_mtu;
#else
	dev->net->netdev_ops = &neutrino_netdev_ops;
#endif

	dev->net->ethtool_ops = &neutrino_ethtool_ops;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
	dev->net->needed_headroom = 8;
#endif

	dev->net->features |= NETIF_F_IP_CSUM;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22)
	dev->net->features |= NETIF_F_IPV6_CSUM;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
	dev->net->hw_features = NETIF_F_IP_CSUM;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22)
	dev->net->hw_features |= NETIF_F_IPV6_CSUM;
#endif
#endif

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	dev->net->vlan_features = dev->net->hw_features;
	DBGPR("vlan_features 0x%16x\n", dev->net->vlan_features);
#endif

	pdata_phc = priv;
	device = interface_to_usbdev(intf);
	getnstimeofday(&now);
        temp1 = (u64)(now.tv_sec * 1000000000);
        temp1 = (u64)(temp1 +  now.tv_nsec);
	
#ifdef NTN_DRV_TEST_LOOPBACK
	ntn_reg_write(dev, EVBTsCtl, 0x00800025); 
#else
	ntn_reg_write(dev, EVBTsCtl, 0x07800725); 
#endif

        getnstimeofday(&now);
        temp2 = (u64)(now.tv_sec * 1000000000);
        temp2 = (u64)(temp2 +  now.tv_nsec);
	diff = temp2 - temp1;
	DBGPR("Register Write Time %lld\n",diff);

	/* To enable ping commented below lins*/
	ntn_reg_write(dev, VendSpecfPktHdrH, 0);
	ntn_reg_write(dev, VendSpecfPktHdrL, 0);

	ntn_reg_write(dev, VendSpecfSrcAddrH, (dev->net->dev_addr[0] << 8) | dev->net->dev_addr[1]);
	ntn_reg_write(dev, VendSpecfSrcAddrL, (dev->net->dev_addr[2] << 24) | (dev->net->dev_addr[3] << 16) |(dev->net->dev_addr[4] << 8) | dev->net->dev_addr[5]);

	ntn_reg_write(dev, VendSpecfDestAddrH, (dev->net->dev_addr[0] << 8) | dev->net->dev_addr[1]);
	ntn_reg_write(dev, VendSpecfDestAddrL, (dev->net->dev_addr[2] << 24) | (dev->net->dev_addr[3] << 16) |(dev->net->dev_addr[4] << 8) | dev->net->dev_addr[5]);

	ntn_reg_write(dev, EVBTimeOff8, 0x1e8480);   
	ntn_reg_write(dev, EVBTimeOff0, 0x1e8480);   
	priv->one_nsec_accuracy = 1;	

	ntn_ptp_init(priv); 

	ntn_reg_write(dev,0xAB00,0x10057E27);

	/* program Sub Second Increment Reg */
	config_sub_second_increment(DWC_ETH_QOS_SYSCLOCK); 

	ptp_temp = (u64)(50000000ULL << 32);
	priv->default_addend = div_u64(ptp_temp, DWC_ETH_QOS_SYSCLOCK);
	config_addend(priv->default_addend);

	getnstimeofday(&now);
	init_systime(now.tv_sec, now.tv_nsec);

	priv->cfg = NTN_USB_CONFIG_1;

	return 0;
}


static void neutrino_unbind(struct usbnet *dev, struct usb_interface *intf)
{
        struct ntn_data *priv = NULL;

	DBGPR("ntn_dbg: In %s\n", __func__);
	synopGMAC_rx_disable(dev);
	synopGMAC_tx_disable(dev);


	priv = (struct ntn_data *)dev->data[0];
	if(!priv)
	{
		printk(KERN_ALERT "Unable to remove ptp_clock\n");
		return;
	}

	ntn_ptp_remove(priv);
#ifndef NTN_DRV_TEST_LOOPBACK
	cancel_delayed_work_sync(&task);
#endif
	usb_deregister_dev(intf, &class);
}



static int neutrino_rx_fixup(struct usbnet *dev, struct sk_buff *skb)
{
#ifndef NTN_DRV_TEST_LOOPBACK
	int ret = 1;
#endif
	u16 eth_type;
	u8 sub_type = 0;
	u32 *data_ptr = NULL;

        data_ptr = (u32 *)skb->data;
        eth_type = *((u16 *)data_ptr + 6);
        eth_type = cpu_to_be16(eth_type);

#ifndef NTN_OUI_DEBUG
        if (eth_type == ETH_TYPE_VEND)
        {
                sub_type = *((u8 *)data_ptr + 19);
        }

        if((eth_type == ETH_TYPE_AVB_PTP) || ((eth_type == ETH_TYPE_VEND)
                                && ((sub_type == ETH_VEND_SUB_TYPE_gPTP_TX_TS) || (sub_type == ETH_VEND_SUB_TYPE_gPTP_RX_TS))))
        {
#ifndef NTN_DRV_TEST_LOOPBACK
                ret = ntn_ptp_rx_hwtstamp(dev,skb);
#endif
        }

#ifdef NTN_DRV_TEST_LOOPBACK
	DBGPR("RX Channel, skb->len = %d , protocol = %#x \n", skb->len, eth_type);
	if(eth_type == NTN_VLAN_TAG) {
		//4Bytes VID. Data starts from [19]. Modify for Loopback
		skb->data[19] = 0xFF;
	} else {	
		/* It's protcol (ether type) field */
		if (eth_type == ETH_TYPE_AVB_PTP) {
			//No VID. GPTP Data starts from [16]. Modify for Loopback
			skb->data[16] = 0xFF;
		} else if (eth_type == ETH_TYPE_AVB) {
			//No VID. AVB Control. Data starts from [15]. Modify for Loopback
			skb->data[15] = 0xFF;
		} else if (eth_type == ETH_TYPE_LEGACY) {
			//No VID. Data starts from [14]. Modify for Loopback
			skb->data[14] = 0xFF;
		}
	}
#endif
        return 1;

#else
	static struct sk_buff *dbg_skb = NULL;
	static struct sk_buff *dbg_data_ptr = NULL;
	/* skb verification */
	if(skb == NULL)
	{
		printk("NULL skb in neutrino_rx_fixup\n");
		return 1;
	}

	if(dbg_skb == skb){
		printk("received same skb\n");
		return 1;
	}
	dbg_skb = skb;

	/* skb data buffer verification */
	data_ptr = (u32 *)skb->data;
	if(data_ptr == NULL)
	{
		printk("NULL data buffer in neutrino_rx_fixup\n");
		return 1;
	}
	
	if(dbg_data_ptr == (struct sk_buff *)data_ptr){
		printk("received same skb data buffer\n");
		return 1;
	}
	dbg_data_ptr = (struct sk_buff *)data_ptr;

	/* skb length verification */
#if 0
	if(skb->len == 0){
		printk("Received zero length skb\n");
		return 1;
	}
	if(skb->len > 512){
		printk("Received oversize skb = %d\n", skb->len);
		return 1;
	}	
	if(skb->len < 36){
		printk("Received undersided packet = %d\n", skb->len);
		return 1;
	}
#endif
	eth_type = *((u16 *)data_ptr + 6);
	eth_type = cpu_to_be16(eth_type);

	if (eth_type == ETH_TYPE_VEND) 
	{
		sub_type = *((u8 *)data_ptr + 19);
		if( (sub_type != ETH_VEND_SUB_TYPE_gPTP_TX_TS) && (sub_type != ETH_VEND_SUB_TYPE_gPTP_RX_TS) ){
			printk("Unexpected vender specific sub type = %d\n", sub_type);
			return 1;
		}
	}

	if((eth_type == ETH_TYPE_AVB_PTP) || ((eth_type == ETH_TYPE_VEND)))
	{	
#ifndef NTN_DRV_TEST_LOOPBACK
		ret = ntn_ptp_rx_hwtstamp(dev,skb);
#endif
	}
	if(eth_type == ETH_TYPE_AVB)
		DBGPR("AVB data Received\n");

	return 1;
#endif
}


static struct sk_buff *
neutrino_tx_fixup(struct usbnet *dev, struct sk_buff *skb, gfp_t flags)
{
	struct ntn_data *priv = NULL;
	u16 *skb_data_ptr = NULL;
	u16 eth_avb_ptp,eth_avb;
	u16  eth_avb_class;
	priv = (struct ntn_data *)dev->data[0];

	skb_data_ptr = (u16 *)skb->data;
	eth_avb_ptp = *(skb_data_ptr + 6);
	eth_avb_ptp = cpu_to_be16(eth_avb_ptp);

	eth_avb = *(skb_data_ptr + 8);
	eth_avb = cpu_to_be16(eth_avb);
	eth_avb_class = *(skb_data_ptr + 7);
	eth_avb_class = cpu_to_be16(eth_avb_class);


	if(eth_avb_ptp == ETH_TYPE_AVB_PTP)
	{

		if(!skb->sk)
			printk("skb->sk is null\n");
		else
		{               
#ifndef NTN_DRV_TEST_LOOPBACK
			int ret;
			ret = ntn_ptp_tx_hwtstamp(dev,skb);
#endif
			dev->out = priv->gptp_out; 
		}            
	}
	else if (eth_avb == ETH_TYPE_AVB)
	{

		if(eth_avb_class == ETH_TYPE_AVB_A)
		{
			dev->out = priv->avb_a_out;   
		}
		else if(eth_avb_class == ETH_TYPE_AVB_B)
		{
			dev->out = priv->avb_b_out; 
		}
		else
		{
			dev->out = priv->avb_a_out;
		}
	}        
	else
	{
		dev->out = priv->legacy_out;     
	}     

	return skb;
}

static int neutrino_link_reset(struct usbnet *dev)
{
	DBGPR("ntn_dbg: In %s\n", __func__);
	return 0;
}

static int neutrino_reset(struct usbnet *dev)
{
	u32 ret;
	struct ntn_data *priv = (struct ntn_data *)dev->data[0];

	DBGPR("ntn_dbg: In %s\n", __func__);
	/* Power up ethernet PHY */
	ret = synopGMAC_check_phy_init(dev,0);   
	if (ret < 0)
	{
		printk("ethernet phy init failed\n");
		return ret;   
	}          


	/*Initialize the mac interface*/
	//synopGMAC_promisc_enable(dev);
	synopGMAC_set_speed(dev, priv->Speed); 

	/* The FEF bit in DMA control register is configured to 0 indicating DMA to drop the errored frames */
	/* Inform the Linux Networking stack about the hardware capability of checksum offloading */
	dev->net->features = NETIF_F_HW_CSUM;

	dev->rx_urb_size = RX_URB_SIZE;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
static int neutrino_stop(struct usbnet *dev)
{
	DBGPR("ntn_dbg: In %s\n", __func__);
	return 0;
}
#endif


static const struct driver_info neutrino_info = {
	.description = "Neutrino USB HSIC to Ethernet AVB & legacy Ethernet Bridge Chip",
	.bind = neutrino_bind,
	.unbind = neutrino_unbind,
	.status = neutrino_status,
	.link_reset = neutrino_link_reset,
	.reset = neutrino_reset,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	.stop = neutrino_stop,
#endif
	.flags = FLAG_ETHER | FLAG_FRAMING_AX,
	.rx_fixup = neutrino_rx_fixup,
	.tx_fixup = neutrino_tx_fixup,
};

static const struct usb_device_id	products[] = {
	{
		/* ntn_tbd: neutrino Vendor ID, Product ID */
		USB_DEVICE_INTERFACE_NUMBER(0x04C6, 0xb0ab, 0),
		.driver_info = (unsigned long) &neutrino_info,
	}, 
	{ },
};
MODULE_DEVICE_TABLE(usb, products);

static struct usb_driver neutrino_driver = {
	.name =		"neutrino",
	.id_table =	products,
	.probe =	ntn_usbnet_probe,
	.suspend =	neutrino_suspend,
	.resume =	neutrino_resume,
	.disconnect =	ntn_usbnet_disconnect,
};

static int neu_hsic_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct ntn_hsic_plt_data *plt_data;

	DBGPR("ntn_dbg: In %s\n", __func__);
	plt_data = devm_kzalloc(&pdev->dev, sizeof(*plt_data), GFP_KERNEL);
	if (!plt_data){
		printk("%s: Failed devm_kzalloc\n", __func__);
		return -ENOMEM;
	}

	plt_data->pldev = pdev;

	/* Neutrino board init */
	ret = neutrino_init_regulators(dev, plt_data);
	if (ret) {
		printk("%s: Failed to init regulators\n", __func__);
		goto neu_err_hsic_probe;
	}

	/*Neutrino PHY init*/
	ret = neutrino_init_phy(dev, plt_data);
	if(ret) {
		printk("%s: Failed to init phy\n", __func__);
		goto neu_err_hsic_probe;
	}

	if (ret == 0) {
		ret = usb_register(&neutrino_driver);
		if (ret < 0) {
			printk(KERN_ERR "%s: Failed usb_register\n", __func__);
			goto neu_err_hsic_probe;
		}
		/* register Ethernet modules */
		list_add_tail(&plt_data->node, &ntn_hsic_plt_data_list);
	}
	return ret;

neu_err_hsic_probe:
	neu_hsic_free_regs(plt_data);
	devm_kfree(&pdev->dev, plt_data);
	dev_info(dev, "probe failed");
	return ret;
}


static int neu_hsic_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct ntn_hsic_plt_data *plt_data;

	DBGPR("ntn_dbg: In %s\n", __func__);
	list_for_each_entry(plt_data, &ntn_hsic_plt_data_list, node) {
		if (plt_data->pldev == pdev){
			neu_hsic_free_regs(plt_data);
			devm_kfree(&pdev->dev, plt_data);
		}
	}
	usb_deregister(&neutrino_driver);
	return ret;
}

static struct of_device_id neu_hsic_match[] = {
	{	.compatible = "qcom,ntn-hsic",
	},
	{}
};

static struct platform_driver neutrino_hsic_driver = {
	.probe	= neu_hsic_probe,
	.remove	= neu_hsic_remove,
	.driver	= {
		.name		= "ntn-hsic",
		.owner		= THIS_MODULE,
		.of_match_table	= neu_hsic_match,
	},
};



static int __init neutrino_init(void)
{
	int ret = 0;
	DBGPR("ntn_dbg: In %s\n", __func__);
	INIT_LIST_HEAD(&ntn_hsic_plt_data_list);
	ret = platform_driver_register(&neutrino_hsic_driver);
	return ret;
}
module_init(neutrino_init);

static void __exit neutrino_exit(void)
{
	DBGPR("ntn_dbg: In %s\n", __func__);
	platform_driver_unregister(&neutrino_hsic_driver);
}
module_exit(neutrino_exit);

MODULE_AUTHOR("TAEC");
MODULE_DESCRIPTION("Neutrino HSIC to Ethernet AVB & legacy Ethernet Bridge Chip");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
