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
 *     21-March-2016 : Added declaration for "DWC_ETH_QOS_config_phy_aneg"
 *      3-Jun-2016   : Some Clean up for unused legacy code.
 */

#ifndef __DWC_ETH_QOS_DRV_H__

#define __DWC_ETH_QOS_DRV_H__

#include <linux/netdev_features.h>

static int DWC_ETH_QOS_open(struct net_device *);

static int DWC_ETH_QOS_close(struct net_device *);

static void DWC_ETH_QOS_set_rx_mode(struct net_device *);

static int DWC_ETH_QOS_start_xmit(struct sk_buff *, struct net_device *);

static void DWC_ETH_QOS_tx_interrupt(struct net_device *,
				     struct DWC_ETH_QOS_prv_data *,
				     UINT chInx);

static struct net_device_stats *DWC_ETH_QOS_get_stats(struct net_device *);

#ifdef CONFIG_NET_POLL_CONTROLLER
static void DWC_ETH_QOS_poll_controller(struct net_device *);
#endif				/*end of CONFIG_NET_POLL_CONTROLLER */

static int DWC_ETH_QOS_set_features(struct net_device *dev, netdev_features_t features);
static int DWC_ETH_QOS_change_mtu(struct net_device *dev, int new_mtu);
static netdev_features_t DWC_ETH_QOS_fix_features(struct net_device *dev, netdev_features_t features);

INT DWC_ETH_QOS_configure_remotewakeup(struct net_device *,
				       struct ifr_data_struct *);

static void DWC_ETH_QOS_program_dcb_algorithm(struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req);

static void DWC_ETH_QOS_program_avb_algorithm(struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req);

static void DWC_ETH_QOS_config_tx_pbl(struct DWC_ETH_QOS_prv_data *pdata,
				      UINT tx_pbl, UINT ch_no);
static void DWC_ETH_QOS_config_rx_pbl(struct DWC_ETH_QOS_prv_data *pdata,
				      UINT rx_pbl, UINT ch_no);

static int DWC_ETH_QOS_handle_prv_ioctl(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifr_data_struct *req);
static int DWC_ETH_QOS_handle_prv_ioctl_ipa(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifreq *ifr);

static int DWC_ETH_QOS_ioctl(struct net_device *, struct ifreq *, int);

irqreturn_t DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS(int, void *);

static int DWC_ETH_QOS_clean_rx_irq(struct DWC_ETH_QOS_prv_data *pdata,
				    int quota, UINT chInx);

static void DWC_ETH_QOS_receive_skb(struct DWC_ETH_QOS_prv_data *pdata,
				    struct net_device *dev,
				    struct sk_buff *skb,
				    UINT chInx);

static void DWC_ETH_QOS_configure_rx_fun_ptr(struct DWC_ETH_QOS_prv_data
					     *pdata);

static int DWC_ETH_QOS_alloc_rx_buf(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata,
				    struct DWC_ETH_QOS_rx_buffer *buffer,
				    gfp_t gfp);

static void DWC_ETH_QOS_default_common_confs(struct DWC_ETH_QOS_prv_data
					     *pdata);
static void DWC_ETH_QOS_default_tx_confs(struct DWC_ETH_QOS_prv_data *pdata);
static void DWC_ETH_QOS_default_tx_confs_single_q(struct DWC_ETH_QOS_prv_data
						  *pdata, UINT qInx);
static void DWC_ETH_QOS_default_rx_confs(struct DWC_ETH_QOS_prv_data *pdata);
static void DWC_ETH_QOS_default_rx_confs_single_q(struct DWC_ETH_QOS_prv_data
						  *pdata, UINT qInx);

int DWC_ETH_QOS_poll(struct DWC_ETH_QOS_prv_data *pdata, int budget, int chInx);

static void DWC_ETH_QOS_mmc_setup(struct DWC_ETH_QOS_prv_data *pdata);
inline unsigned int DWC_ETH_QOS_reg_read(volatile ULONG *ptr);

static INT DWC_ETH_QOS_config_phy_aneg (struct DWC_ETH_QOS_prv_data *pdata, unsigned int enb_dis, unsigned int restart);

#ifdef DWC_ETH_QOS_QUEUE_SELECT_ALGO
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0) )
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb);
#elif	( LINUX_VERSION_CODE == KERNEL_VERSION(3,13,0) ) //3.16.0
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb, void *accel_priv);
#else   //3.13.0
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb, void *accel_priv, select_queue_fallback_t fallback);
#endif	//3.13.0
#endif


static
#if defined(HAVE_INT_NDO_VLAN_RX_ADD_VID) || defined(NETIF_F_HW_VLAN_CTAG_RX)
int
#else
void
#endif
DWC_ETH_QOS_vlan_rx_add_vid(struct net_device *dev,
#ifdef NETIF_F_HW_VLAN_CTAG_RX
                            __always_unused __be16 proto,
#endif
                            u16 vid);
static
#if defined(HAVE_INT_NDO_VLAN_RX_ADD_VID) || defined(NETIF_F_HW_VLAN_CTAG_RX)
int
#else
void
#endif
DWC_ETH_QOS_vlan_rx_kill_vid(struct net_device *dev,
#ifdef NETIF_F_HW_VLAN_CTAG_RX
                             __always_unused __be16 proto,
#endif
                             u16 vid);

#endif
