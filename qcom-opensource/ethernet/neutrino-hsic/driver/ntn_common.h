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
 

#ifndef	__LINUX_USBNET_NTN_H
#define	__LINUX_USBNET_NTN_H

#ifdef __KERNEL__

#include <linux/version.h>
/*#include <linux/config.h>*/
//#define DEBUG
#ifdef	CONFIG_USB_DEBUG
#define DEBUG
#endif
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/crc32.h>
#include <linux/types.h> 
#include <linux/device.h>
#include <linux/time.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/net_tstamp.h>
#include <linux/clocksource.h>
#include <linux/spinlock_types.h>
#include <linux/semaphore.h>

//#include "ntn_reg_rw.h"
#include "DWC_ETH_QOS_yapphdr.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
#include <linux/usb/usbnet.h>
#else
#include <../drivers/usb/net/usbnet.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
#include <uapi/linux/sockios.h>
#endif
/* Currently VLAN CTAG Tx/ Rx,  Filtering not enabled.*/
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#include <linux/if_vlan.h>
#define DWC_ETH_QOS_ENABLE_VLAN_TAG
#endif

//#define NTN_DRV_TEST_LOOPBACK 1
//#define NTN_OUI_DEBUG 1
#define NTN_VLAN_TAG			(0x8100)
#define ETH_TYPE_LEGACY			(0x9999)
#define NTN_PHY_DETECT_INTERVAL (1000)

//#define DWC_ETH_QOS_SYSCLOCK        62500000 /* System clock is 62.5MHz */
#define DWC_ETH_QOS_SYSCLOCK        125000000 /* System clock is 125 MHz */
#define DWC_ETH_QOS_AVB_ALGORITHM   27
#define	DWC_ETH_QOS_NO_HW_SUPPORT   -1
#define DWC_ETH_QOS_GET_CONNECTED_SPEED		25

#define NTN_TX_DMA_CH_CNT 5
#define NTN_RX_DMA_CH_CNT 6

#define NTN_TX_QUEUE_CNT (NTN_TX_DMA_CH_CNT)
#define NTN_RX_QUEUE_CNT (NTN_RX_DMA_CH_CNT)
#define NTN_QUEUE_CNT min(NTN_TX_QUEUE_CNT, NTN_RX_QUEUE_CNT)

#define NTN_PHY_ID			    0x00    /* ntn_tbd: need ntn phy id*/

#define NTN_ACCESS_MAC			0x01   /* Vendor Specific command type to access mac registers */  
#define NTN_ACCESS_USB			0x02   /* Reserved for future use */ 

#define PHYSICAL_LINK_STATUS		0x02
	#define	NTN_USB_SS		0x04
	#define	NTN_USB_HS		0x02
	#define	NTN_USB_FS		0x01

/*****************************************************************************/
/* GMII register definitions */
#define GMII_PHY_CONTROL			0x00	/* control reg */
	/* Bit definitions: GMII Control */
	#define GMII_CONTROL_RESET		0x8000	/* reset bit in control reg */
	#define GMII_CONTROL_LOOPBACK		0x4000	/* loopback bit in control reg */
	#define GMII_CONTROL_10MB		0x0000	/* 10 Mbit */
	#define GMII_CONTROL_100MB		0x2000	/* 100Mbit */
	#define GMII_CONTROL_1000MB		0x0040	/* 1000Mbit */
	#define GMII_CONTROL_SPEED_BITS		0x2040	/* speed bit mask */
	#define GMII_CONTROL_ENABLE_AUTO	0x1000	/* autonegotiate enable */
	#define GMII_CONTROL_POWER_DOWN		0x0800
	#define GMII_CONTROL_ISOLATE		0x0400	/* islolate bit */
	#define GMII_CONTROL_START_AUTO		0x0200	/* restart autonegotiate */
	#define GMII_CONTROL_FULL_DUPLEX	0x0100

#define GMII_PHY_STATUS				0x01	/* status reg */
	/* Bit definitions: GMII Status */
	#define GMII_STATUS_100MB_MASK		0xE000	/* any of these indicate 100 Mbit */
	#define GMII_STATUS_10MB_MASK		0x1800	/* either of these indicate 10 Mbit */
	#define GMII_STATUS_AUTO_DONE		0x0020	/* auto negotiation complete */
	#define GMII_STATUS_AUTO		0x0008	/* auto negotiation is available */
	#define GMII_STATUS_LINK_UP		0x0004	/* link status bit */
	#define GMII_STATUS_EXTENDED		0x0001	/* extended regs exist */
	#define GMII_STATUS_100T4		0x8000	/* capable of 100BT4 */
	#define GMII_STATUS_100TXFD		0x4000	/* capable of 100BTX full duplex */
	#define GMII_STATUS_100TX		0x2000	/* capable of 100BTX */
	#define GMII_STATUS_10TFD		0x1000	/* capable of 10BT full duplex */
	#define GMII_STATUS_10T			0x0800	/* capable of 10BT */

#define GMII_PHY_OUI				0x02	/* most of the OUI bits */
#define GMII_PHY_MODEL				0x03	/* model/rev bits, and rest of OUI */
#define GMII_PHY_ANAR				0x04	/* AN advertisement reg */
	/* Bit definitions: Auto-Negotiation Advertisement */
	#define GMII_ANAR_ASYM_PAUSE		0x0800	/* support asymetric pause */
	#define GMII_ANAR_PAUSE			0x0400	/* support pause packets */
	#define GMII_ANAR_100T4			0x0200	/* support 100BT4 */
	#define GMII_ANAR_100TXFD		0x0100	/* support 100BTX full duplex */
	#define GMII_ANAR_100TX			0x0080	/* support 100BTX half duplex */
	#define GMII_ANAR_10TFD			0x0040	/* support 10BT full duplex */
	#define GMII_ANAR_10T			0x0020	/* support 10BT half duplex */
	#define GMII_SELECTOR_FIELD		0x001F	/* selector field. */

#define GMII_PHY_ANLPAR				0x05	/* AN Link Partner */
	/* Bit definitions: Auto-Negotiation Link Partner Ability */
	#define GMII_ANLPAR_100T4		0x0200	/* support 100BT4 */
	#define GMII_ANLPAR_100TXFD		0x0100	/* support 100BTX full duplex */
	#define GMII_ANLPAR_100TX		0x0080	/* support 100BTX half duplex */
	#define GMII_ANLPAR_10TFD		0x0040	/* support 10BT full duplex */
	#define GMII_ANLPAR_10T			0x0020	/* support 10BT half duplex */
	#define GMII_ANLPAR_PAUSE		0x0400	/* support pause packets */
	#define GMII_ANLPAR_ASYM_PAUSE		0x0800	/* support asymetric pause */
	#define GMII_ANLPAR_ACK			0x4000	/* means LCB was successfully rx'd */
	#define GMII_SELECTOR_8023		0x0001;

#define GMII_PHY_ANER				0x06	/* AN expansion reg */
#define GMII_PHY_1000BT_CONTROL			0x09	/* control reg for 1000BT */
#define GMII_PHY_1000BT_STATUS			0x0A	/* status reg for 1000BT */

#define GMII_PHY_PHYSR				0x11	/* PHY specific status register */
	#define GMII_PHY_PHYSR_SMASK		0xc000
	#define GMII_PHY_PHYSR_GIGA		0x8000
	#define GMII_PHY_PHYSR_100		0x4000
	#define GMII_PHY_PHYSR_FULL		0x2000
	#define GMII_PHY_PHYSR_LINK		0x400

/* Bit definitions: 1000BaseT AUX Control */
#define GMII_1000_AUX_CTRL_MASTER_SLAVE		0x1000
#define GMII_1000_AUX_CTRL_FD_CAPABLE		0x0200	/* full duplex capable */
#define GMII_1000_AUX_CTRL_HD_CAPABLE		0x0100	/* half duplex capable */

/* Bit definitions: 1000BaseT AUX Status */
#define GMII_1000_AUX_STATUS_FD_CAPABLE		0x0800	/* full duplex capable */
#define GMII_1000_AUX_STATUS_HD_CAPABLE		0x0400	/* half duplex capable */

/*Cicada MII Registers */
#define GMII_AUX_CTRL_STATUS			0x1C
#define GMII_AUX_ANEG_CPLT			0x8000
#define GMII_AUX_FDX				0x0020
#define GMII_AUX_SPEED_1000			0x0010
#define GMII_AUX_SPEED_100			0x0008

#define GMII_LED_ACTIVE				0x1a
	#define GMII_LED_ACTIVE_MASK		0xff8f
	#define GMII_LED0_ACTIVE		(1 << 4)
	#define GMII_LED1_ACTIVE		(1 << 5)
	#define GMII_LED2_ACTIVE		(1 << 6)

#define GMII_LED_LINK				0x1c
	#define GMII_LED_LINK_MASK		0xf888
	#define GMII_LED0_LINK_10		(1 << 0)
	#define GMII_LED0_LINK_100		(1 << 1)
	#define GMII_LED0_LINK_1000		(1 << 2)
	#define GMII_LED1_LINK_10		(1 << 4)
	#define GMII_LED1_LINK_100		(1 << 5)
	#define GMII_LED1_LINK_1000		(1 << 6)
	#define GMII_LED2_LINK_10		(1 << 8)
	#define GMII_LED2_LINK_100		(1 << 9)
	#define GMII_LED2_LINK_1000		(1 << 10)

	#define	LED_VALID	(1 << 15) /* UA2 LED Setting */

	#define	LED0_ACTIVE	(1 << 0)
	#define	LED0_LINK_10	(1 << 1)
	#define	LED0_LINK_100	(1 << 2)
	#define	LED0_LINK_1000	(1 << 3)
	#define	LED0_FD		(1 << 4)
	#define LED0_USB3_MASK	0x001f

	#define	LED1_ACTIVE	(1 << 5)
	#define	LED1_LINK_10	(1 << 6)
	#define	LED1_LINK_100	(1 << 7)
	#define	LED1_LINK_1000	(1 << 8)
	#define	LED1_FD		(1 << 9)
	#define LED1_USB3_MASK	0x03e0

	#define	LED2_ACTIVE	(1 << 10)
	#define	LED2_LINK_1000	(1 << 13)
	#define	LED2_LINK_100	(1 << 12)
	#define	LED2_LINK_10	(1 << 11)
	#define	LED2_FD		(1 << 14)
	#define LED2_USB3_MASK	0x7c00

#define GMII_PHYPAGE				0x1e

#define GMII_PHY_PAGE_SELECT			0x1f
	#define GMII_PHY_PAGE_SELECT_EXT	0x0007
	#define GMII_PHY_PAGE_SELECT_PAGE0	0X0000
	#define GMII_PHY_PAGE_SELECT_PAGE1	0X0001
	#define GMII_PHY_PAGE_SELECT_PAGE2	0X0002
	#define GMII_PHY_PAGE_SELECT_PAGE3	0X0003
	#define GMII_PHY_PAGE_SELECT_PAGE4	0X0004
	#define GMII_PHY_PAGE_SELECT_PAGE5	0X0005
	#define GMII_PHY_PAGE_SELECT_PAGE6	0X0006
	

#define NTN_MAX_HASH_TABLE_SIZE 64
#define NTN_MAX_ADDR_REG_CNT 32
#define DWC_ETH_QOS_HTR_CNT (NTN_MAX_HASH_TABLE_SIZE/32)

#define LEGACY_PIPE_IDX   0
#define AVB_PIPE_IDX      1
#define AVBCTL_PIPE_IDX   2
#define GPTP_PIPE_IDX     3

/******************************************************************************/
//ntn private data structure  
struct ntn_data {
    struct net_device *dev;
    struct usbnet *udev;
    struct ptp_clock *ptp_clock;
    struct ptp_clock_info ptp_clock_ops;
    struct delayed_work ptp_overflow_work;
    struct work_struct ptp_tx_work;
    struct sk_buff *ptp_tx_skb;
    u8 phy_id;
    /* The status of the link */
    u32 LinkState;         /* Link status as reported by the Ti Phy */
    u32 DuplexMode;        /* Duplex mode of the Phy */
    u32 Speed;             /* Speed of the Phy */
    u32 LoopBackMode;
    u8  cfg;               /* USB configuration number */
    unsigned legacy_in;
    unsigned legacy_out;
    unsigned avb_in;
    unsigned avb_a_out;
    unsigned avb_b_out;
    unsigned gptp_in;
    unsigned gptp_out;
    unsigned avb_control;
    u32      kevent_pipe_flags;
    u32 flags;
    spinlock_t reg_lock;
    struct semaphore ptp_mutex;
    struct timecounter tc;
    unsigned int default_addend;
    bool one_nsec_accuracy; /* set to 1 if one nano second accuracy*/
//    void         (*ntn_ptp)(struct usbnet *dev, struct sk_buff *skb);
	unsigned int l2_filtering_mode; /* 0 - if perfect and 1 - if hash filtering */
	unsigned int MAC_Packet_Filter;
} __attribute__ ((packed));

// AVB Ether Type 
#define ETH_TYPE_AVB_PTP    0x88F7
#define ETH_TYPE_AVB        0x22F0 
#define ETH_TYPE_AVB_A      0x6002
#define ETH_TYPE_AVB_B	    0x4002

// AVB Class Mask
#define AVB_CLASS_MASK      0xE000

// AVB Class Value   
#define AVB_CLASS_A        0x6000 
#define AVB_CLASS_B        0x4000

// Vendor Specific Ether Type 
#define ETH_TYPE_VEND       0x88B7
// Vendor Specific Sub Type 
#define ETH_VEND_SUB_TYPE_gPTP_TX_TS   0x01
#define ETH_VEND_SUB_TYPE_CLASS_A_B_DATA_TS   0x02
#define ETH_VEND_SUB_TYPE_SPI_TRANSPORT_STREAM   0x03
#define ETH_VEND_SUB_TYPE_IQ0_AUDIO   0x04
#define ETH_VEND_SUB_TYPE_IQ1_AUDIO   0x05
#define ETH_VEND_SUB_TYPE_gPTP_RX_TS   0x00

#define NTN_USB_CONFIG_1    1
#define NTN_USB_CONFIG_2    2
#define NTN_DEBUG	    1
#define NTN_FLAG_PTP    (1 << 9)

#define Y_SUCCESS 1

struct ntn_int_data {
	__le16 res1;
#define NTN_INT_PPLS_LINK	(1 << 0)
#define NTN_INT_SPLS_LINK	(1 << 1)
#define NTN_INT_CABOFF_UNPLUG	(1 << 7)
	u8 link;
	__le16 res2;
	u8 status;
	__le16 res3;
} __attribute__ ((packed));

struct ntn_usb_device
{
    unsigned in;
    unsigned char *bulk_in_buffer;      /* the buffer to receive data */
    size_t  bulk_in_size;               /* the size of the receive buffer */
};
/*
#ifdef NTN_DEBUG
#define DBGPR(x...) printk(KERN_DEBUG x)
#endif*/

#if 0
static int ntn_reset(struct usbnet *dev);
static int ntn_link_reset(struct usbnet *dev);
static int ntn_AutoDetach(struct usbnet *dev, int in_pm);
#endif

#define TR0(fmt, args...) printk(KERN_CRIT "SynopGMAC: " fmt, ##args)				

#ifdef DEBUG
#undef TR
#  define TR(fmt, args...) printk(KERN_CRIT "SynopGMAC: " fmt, ##args)
	#define DBGPR(x...) printk(KERN_DEBUG x)
#else
# define TR(fmt, args...) /* not debugging: nothing */
	#define DBGPR(x...) 
#endif


#define DEFAULT_DELAY_VARIABLE  10
#define DEFAULT_LOOP_VARIABLE   1000

/* There are platform related endian conversions
 *
 */

#define LE32_TO_CPU	__le32_to_cpu
#define BE32_TO_CPU	__be32_to_cpu
#define CPU_TO_LE32	__cpu_to_le32

/* Error Codes */
#define ESYNOPGMACNOERR   0
#define ESYNOPGMACNOMEM   1
#define ESYNOPGMACPHYERR  2
#define ESYNOPGMACBUSY    3

struct Network_interface_data
{
	u32 unit;
	u32 addr;
	u32 data;
};

#if 0
typedef int bool;
enum synopGMAC_boolean
 { 
    false = 0,
    true = 1 
 };
#endif

#define CHECK(ret, func)    \
    if(0 != ret) {        \
        TR("%s: Failed at %s\n", __func__, func); \
        return ret; \
    }

#define CHECK_TIME (HZ)

#endif /*__KERNEL__ */

/* Refer cfgpath[] in driver for sequence */
typedef enum{
	SPI12USB,
	IQ02USB,
	IQ12USB,
	EVB2TDM0,
	HDMI2USB,

} TEST_CFG_INDEX;

/*
struct ifr_data_struct_usb
{
        uint32_t unit;
        uint32_t addr;
        uint32_t data;
};
*/

struct ifr_data_struct_1722
{
        uint32_t txred_len;
        uint32_t pkt_len;
        uint8_t  data[200];
};


struct AVB_BW_parameters {
        unsigned int idle_slope;
        unsigned int send_slope;
        unsigned int hi_credit;
        unsigned int low_credit;
};



#define IOCTL_READ_REGISTER  SIOCDEVPRIVATE+1
#define IOCTL_WRITE_REGISTER SIOCDEVPRIVATE+7
#define IOCTL_READ_IPSTRUCT  SIOCDEVPRIVATE+3
#define IOCTL_READ_RXDESC    SIOCDEVPRIVATE+4
#define IOCTL_READ_TXDESC    SIOCDEVPRIVATE+5
#define IOCTL_POWER_DOWN     SIOCDEVPRIVATE+6
#define IOCTL_NTN_TIME       SIOCDEVPRIVATE+12
#define IOCTL_EP1_TX         SIOCDEVPRIVATE+9
#define IOCTL_HDMI           SIOCDEVPRIVATE+2
#define IOCTL_PHY_LOOPBACK   SIOCDEVPRIVATE+10
#define IOCTL_FREQ_FACTOR    SIOCDEVPRIVATE+8
#define IOCTL_ADDEND_WRITE   SIOCDEVPRIVATE+11
#define IOCTL_CONFIG_PATH    SIOCDEVPRIVATE+13
#define IOCTL_DECONFIG_PATH  SIOCDEVPRIVATE+14



#endif /* __LINUX_USBNET_NTN_H */

