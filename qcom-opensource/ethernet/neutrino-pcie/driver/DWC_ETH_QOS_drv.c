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
 *     21-March-2016 : Added "DWC_ETH_QOS_config_phy_aneg" API and it's calls
 *      3-Jun-2016   : Some Clean up for unused legacy code.
 *                   : TX Channel selection is moved to "DWC_ETH_QOS_select_queue" function.
 *                   : New statistics added to Ethtool for EMAC TX/RX DMA Descriptor Status & M3 MSI Generation Statistics (SRAM).
 *                   : Enabled WDT in INTC and added a counter in M3 Firmware(FW_OSLESS) for WDT half expiry interrupts.
 */

/*!@file: DWC_ETH_QOS_drv.c
 * @brief: Driver functions.
 */

#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_yapphdr.h"
#include "DWC_ETH_QOS_drv.h"
#include "DWC_ETH_QOS_ipa.h"
#include "DWC_ETH_QOS_yregacc.h"

#define AVB_QUEUE_CLASS_A 1
#define AVB_QUEUE_CLASS_B 2

static INT DWC_ETH_QOS_GStatus;

#ifdef NTN_POLLING_METHOD
static void* polling_task_data = NULL;
static struct delayed_work task;
#endif //NTN_POLLING_METHOD

/* SA(Source Address) operations on TX */
unsigned char mac_addr0[6] = { 0xE8, 0xE0, 0xB7, 0xB5, 0x7D, 0xF8 };
unsigned char mac_addr1[6] = { 0xE8, 0xE0, 0xB7, 0xB5, 0x7D, 0xF7 };

/* module parameters for configuring the queue modes
 *
 * */
static int q_op_mode[DWC_ETH_QOS_MAX_TX_QUEUE_CNT] = {
	DWC_ETH_QOS_Q_GENERIC,
	DWC_ETH_QOS_Q_AVB,
	DWC_ETH_QOS_Q_AVB,
//	DWC_ETH_QOS_Q_GENERIC,
//	DWC_ETH_QOS_Q_GENERIC,
//	DWC_ETH_QOS_Q_GENERIC,
//	DWC_ETH_QOS_Q_GENERIC,
//	DWC_ETH_QOS_Q_GENERIC
};
module_param_array(q_op_mode, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(q_op_mode,
	"MTL queue operation mode [0-DISABLED, 1-AVB, 2-DCB, 3-GENERIC]");

#ifdef NTN_POLLING_METHOD
static void polling_task(void)
{
	struct DWC_ETH_QOS_prv_data *pdata = (struct DWC_ETH_QOS_prv_data*)polling_task_data;
//	unsigned int chInx;

	DBGPR("-->polling_task");

	if(pdata == NULL)
	{
		NMSGPR_ALERT( "polling_task data pointer is NULL");
		return;
	}

	disable_irq(pdata->irq_number);
    DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS(pdata->irq_number, pdata);
	enable_irq(pdata->irq_number);


	DBGPR("<--polling_task");

	schedule_delayed_work(&task, usecs_to_jiffies(NTN_POLL_DELAY_US));
	return;
}
#endif //NTN_POLLING_METHOD

void DWC_ETH_QOS_stop_all_ch_tx_dma(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_stop_all_ch_tx_dma\n");

	for(chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++){
		if(!pdata->tx_dma_ch_for_host[chInx])
			continue;
		hw_if->stop_dma_tx(chInx, pdata);
	}

	DBGPR("<--DWC_ETH_QOS_stop_all_ch_tx_dma\n");
}

static void DWC_ETH_QOS_stop_all_ch_rx_dma(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_stop_all_ch_rx_dma\n");

	for(chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		hw_if->stop_dma_rx(chInx, pdata);
	}
	DBGPR("<--DWC_ETH_QOS_stop_all_ch_rx_dma\n");
}

static void DWC_ETH_QOS_start_all_ch_tx_dma(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_start_all_ch_tx_dma\n");

	for(chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++){
		if(!pdata->tx_dma_ch_for_host[chInx])
			continue;
		if (pdata->ipa_enabled && chInx == NTN_TX_DMA_CH_2)
			continue;
		hw_if->start_dma_tx(chInx, pdata);
	}
	DBGPR("<--DWC_ETH_QOS_start_all_ch_tx_dma\n");
}

static void DWC_ETH_QOS_start_all_ch_rx_dma(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_start_all_ch_rx_dma\n");

	for(chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0)
			continue;
		hw_if->start_dma_rx(chInx, pdata);
	}
	DBGPR("<--DWC_ETH_QOS_start_all_ch_rx_dma\n");
}

static void DWC_ETH_QOS_napi_enable_mq(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = NULL;
	int chInx;

	DBGPR("-->DWC_ETH_QOS_napi_enable_mq\n");

	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		rx_dma_ch = GET_RX_DMA_CH_PTR(chInx);
		napi_enable(&rx_dma_ch->napi);
	}

	DBGPR("<--DWC_ETH_QOS_napi_enable_mq\n");
}

static void DWC_ETH_QOS_all_ch_napi_disable(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = NULL;
	int chInx;

	DBGPR("-->DWC_ETH_QOS_napi_enable\n");

	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		rx_dma_ch = GET_RX_DMA_CH_PTR(chInx);
		napi_disable(&rx_dma_ch->napi);
	}

	DBGPR("<--DWC_ETH_QOS_napi_enable\n");
}


#if 0
/*!
 * \details This function is invoked to stop device operation
 * Following operations are performed in this function.
 * - Stop the queue.
 * - Stops DMA TX and RX.
 * - Free the TX and RX skb's.
 * - Issues soft reset to device.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_stop_dev(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct desc_if_struct *desc_if = &(pdata->desc_if);

	DBGPR("-->DWC_ETH_QOS_stop_dev\n");

	netif_tx_disable(pdata->dev);

	DWC_ETH_QOS_all_ch_napi_disable(pdata);

	/* stop DMA TX/RX */
	DWC_ETH_QOS_stop_all_ch_tx_dma(pdata);
	DWC_ETH_QOS_stop_all_ch_rx_dma(pdata);

	/* issue software reset to device */
	hw_if->exit(pdata);

	/* free tx skb's */
	desc_if->tx_skb_free_mem(pdata, NTN_TX_DMA_CH_CNT);
	/* free rx skb's */
	desc_if->rx_skb_free_mem(pdata, NTN_RX_DMA_CH_CNT);

	DBGPR("<--DWC_ETH_QOS_stop_dev\n");
}
#endif

static void DWC_ETH_QOS_tx_desc_mang_ds_dump(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data = NULL;
	struct s_TX_NORMAL_DESC *tx_desc = NULL;
	int chInx, i;

#ifndef YDEBUG
	return;
#endif
	NMSGPR_ALERT( "/**** TX DESC MANAGEMENT DATA STRUCTURE DUMP ****/\n");

	NMSGPR_ALERT( "TX_DESC_QUEUE_CNT = %d\n", NTN_TX_DMA_CH_CNT);
	for (chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++) {

		if(!pdata->tx_dma_ch_for_host[chInx])
			continue;

		tx_desc_data = GET_TX_WRAPPER_DESC(chInx);

		NMSGPR_ALERT( "DMA CHANNEL = %d\n", chInx);

		NMSGPR_ALERT( "\tcur_tx           = %d\n",
			tx_desc_data->cur_tx);
		NMSGPR_ALERT( "\tdirty_tx         = %d\n",
			tx_desc_data->dirty_tx);
		NMSGPR_ALERT( "\tfree_desc_cnt    = %d\n",
			tx_desc_data->free_desc_cnt);
		NMSGPR_ALERT( "\ttx_pkt_queued    = %d\n",
			tx_desc_data->tx_pkt_queued);
		NMSGPR_ALERT( "\tqueue_stopped    = %d\n",
			tx_desc_data->queue_stopped);
		NMSGPR_ALERT( "\tpacket_count     = %d\n",
			tx_desc_data->packet_count);
		NMSGPR_ALERT( "\ttx_threshold_val = %d\n",
			tx_desc_data->tx_threshold_val);
		NMSGPR_ALERT( "\ttsf_on           = %d\n",
			tx_desc_data->tsf_on);
		NMSGPR_ALERT( "\tosf_on           = %d\n",
			tx_desc_data->osf_on);
		NMSGPR_ALERT( "\ttx_pbl           = %d\n",
			tx_desc_data->tx_pbl);

		NMSGPR_ALERT( "\t[<desc_add> <index >] = <TDES0> : <TDES1> : <TDES2> : <TDES3>\n");
		for (i = 0; i < pdata->tx_dma_ch[chInx].desc_cnt; i++) {
			tx_desc = GET_TX_DESC_PTR(chInx, i);
			NMSGPR_ALERT( "\t[%4p %03d] = %#x : %#x : %#x : %#x\n",
				tx_desc, i, tx_desc->TDES0, tx_desc->TDES1,
				tx_desc->TDES2, tx_desc->TDES3);
		}
	}

	NMSGPR_ALERT( "/************************************************/\n");
}


static void DWC_ETH_QOS_rx_desc_mang_ds_dump(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data = NULL;
	struct s_RX_NORMAL_DESC *rx_desc = NULL;
	int chInx, i;
	dma_addr_t *base_ptr = GET_RX_BUFF_POOL_BASE_ADRR(NTN_RX_DMA_CH_0);
	struct DWC_ETH_QOS_rx_buffer *rx_buf_ptrs = NULL;

#ifndef YDEBUG
	return;
#endif
	NMSGPR_ALERT( "/**** RX DESC MANAGEMENT DATA STRUCTURE DUMP ****/\n");

	NMSGPR_ALERT( "RX_DESC_QUEUE_CNT = %d\n", NTN_RX_DMA_CH_CNT);
	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {

		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;

		rx_desc_data = GET_RX_WRAPPER_DESC(chInx);

		NMSGPR_ALERT( "DMA CHANNEL = %d\n", chInx);

		NMSGPR_ALERT( "\tcur_rx                = %d\n",
			rx_desc_data->cur_rx);
		NMSGPR_ALERT( "\tdirty_rx              = %d\n",
			rx_desc_data->dirty_rx);
		NMSGPR_ALERT( "\tpkt_received          = %d\n",
			rx_desc_data->pkt_received);
		NMSGPR_ALERT( "\tskb_realloc_idx       = %d\n",
			rx_desc_data->skb_realloc_idx);
		NMSGPR_ALERT( "\tskb_realloc_threshold = %d\n",
			rx_desc_data->skb_realloc_threshold);
		NMSGPR_ALERT( "\tuse_riwt              = %d\n",
			rx_desc_data->use_riwt);
		NMSGPR_ALERT( "\trx_riwt               = %d\n",
			rx_desc_data->rx_riwt);
		NMSGPR_ALERT( "\trx_coal_frames        = %d\n",
			rx_desc_data->rx_coal_frames);
		NMSGPR_ALERT( "\trx_threshold_val      = %d\n",
			rx_desc_data->rx_threshold_val);
		NMSGPR_ALERT( "\trsf_on                = %d\n",
			rx_desc_data->rsf_on);
		NMSGPR_ALERT( "\trx_pbl                = %d\n",
			rx_desc_data->rx_pbl);
		if (pdata->ipa_enabled) {
			NMSGPR_ALERT("\tRX Base Ring     = %p\n",
				     &GET_RX_BUFF_POOL_BASE_ADRR(chInx));
		}

		NMSGPR_ALERT( "\t[<desc_add> <index >] = <RDES0> : <RDES1> : <RDES2> : <RDES3>\n");
		for (i = 0; i < pdata->rx_dma_ch[chInx].desc_cnt; i++) {
			rx_desc = GET_RX_DESC_PTR(chInx, i);
			NMSGPR_ALERT( "\t[%4p %03d] = %#x : %#x : %#x : %#x\n",
				rx_desc, i, rx_desc->RDES0, rx_desc->RDES1,
				rx_desc->RDES2, rx_desc->RDES3);
			if (pdata->ipa_enabled) {
				rx_buf_ptrs = GET_RX_BUF_PTR(chInx, i);
				if ((rx_buf_ptrs != NULL)  &&
				     (GET_RX_BUFF_POOL_BASE_ADRR(chInx) != NULL)) {
					NMSGPR_ALERT("\t skb mempool %p skb rx buf %p ,"
						     "skb len %d skb dma %p base %p\n",
						     (void *)GET_RX_BUFF_DMA_ADDR(chInx, i),
						     rx_buf_ptrs->skb, rx_buf_ptrs->len,
						     (void *)rx_buf_ptrs->dma, (void *)(base_ptr + i));
				}
			}
		}
	}

	NMSGPR_ALERT( "/************************************************/\n");
}


static void DWC_ETH_QOS_restart_phy(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->DWC_ETH_QOS_restart_phy\n");

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return;
        }

	pdata->oldlink = 0;
	pdata->speed = 0;
	pdata->oldduplex = -1;

	if (pdata->phydev)
		genphy_config_aneg(pdata->phydev);

	DBGPR("<--DWC_ETH_QOS_restart_phy\n");
}

#if 0
/*!
 * \details This function is invoked to start the device operation
 * Following operations are performed in this function.
 * - Initialize software states
 * - Initialize the TX and RX descriptors queue.
 * - Initialize the device to know state
 * - Start the queue.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_start_dev(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct desc_if_struct *desc_if = &(pdata->desc_if);

	DBGPR("-->DWC_ETH_QOS_start_dev\n");

	/* reset all variables */
	DWC_ETH_QOS_default_common_confs(pdata);
	DWC_ETH_QOS_default_tx_confs(pdata);
	DWC_ETH_QOS_default_rx_confs(pdata);

	DWC_ETH_QOS_configure_rx_fun_ptr(pdata);

	DWC_ETH_QOS_napi_enable_mq(pdata);

	/* reinit descriptor */
	desc_if->wrapper_tx_desc_init(pdata);
	desc_if->wrapper_rx_desc_init(pdata);

	DWC_ETH_QOS_tx_desc_mang_ds_dump(pdata);
	DWC_ETH_QOS_rx_desc_mang_ds_dump(pdata);

	/* initializes MAC and DMA */
	hw_if->init(pdata);

	if (pdata->vlan_hash_filtering)
		hw_if->update_vlan_hash_table_reg(pdata->vlan_ht_or_id, pdata);
	else
		hw_if->update_vlan_id(pdata->vlan_ht_or_id, pdata);

	DWC_ETH_QOS_restart_phy(pdata);

	pdata->eee_enabled = DWC_ETH_QOS_eee_init(pdata);

	netif_tx_wake_all_queues(pdata->dev);

	DBGPR("<--DWC_ETH_QOS_start_dev\n");
}
#endif

/*!
 * \details This function is invoked by isr handler when device issues an FATAL
 * bus error interrupt.  Following operations are performed in this function.
 * - Stop the device.
 * - Start the device
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] chInx – queue number.
 *
 * \return void
 */

static void DWC_ETH_QOS_restart_dev(struct DWC_ETH_QOS_prv_data *pdata,
					UINT chInx)
{
	struct desc_if_struct *desc_if = &(pdata->desc_if);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = GET_RX_DMA_CH_PTR(chInx);

	DBGPR("-->DWC_ETH_QOS_restart_dev\n");

	netif_stop_subqueue(pdata->dev, chInx);
	napi_disable(&rx_dma_ch->napi);

	/* stop DMA TX/RX */
	hw_if->stop_dma_tx(chInx, pdata);
	hw_if->stop_dma_rx(chInx, pdata);

	/* free tx skb's */
	desc_if->tx_skb_free_mem_single_q(pdata, chInx);
	/* free rx skb's */
	desc_if->rx_skb_free_mem_single_q(pdata, chInx);

	if ((NTN_TX_QUEUE_CNT == 0) &&
		(NTN_RX_QUEUE_CNT == 0)) {
		/* issue software reset to device */
		hw_if->exit(pdata);

		DWC_ETH_QOS_configure_rx_fun_ptr(pdata);
		DWC_ETH_QOS_default_common_confs(pdata);
	}
	/* reset all variables */
	DWC_ETH_QOS_default_tx_confs_single_q(pdata, chInx);
	DWC_ETH_QOS_default_rx_confs_single_q(pdata, chInx);

	/* reinit descriptor */
	desc_if->wrapper_tx_desc_init_single_q(pdata, chInx);
	desc_if->wrapper_rx_desc_init_single_q(pdata, chInx);

	napi_enable(&rx_dma_ch->napi);

	/* initializes MAC and DMA
	 * NOTE : Do we need to init only one channel
	 * which generate FBE*/
	hw_if->init(pdata);

	DWC_ETH_QOS_restart_phy(pdata);
	pdata->phydev->state = PHY_UP;

	netif_wake_subqueue(pdata->dev, chInx);

	DBGPR("<--DWC_ETH_QOS_restart_dev\n");
}

void DWC_ETH_QOS_disable_all_ch_rx_interrpt(
			struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_disable_all_ch_rx_interrpt\n");

	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		hw_if->disable_rx_interrupt(chInx, pdata);
	}

	DBGPR("<--DWC_ETH_QOS_disable_all_ch_rx_interrpt\n");
}

void DWC_ETH_QOS_enable_all_ch_rx_interrpt(
			struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_enable_all_ch_rx_interrpt\n");

	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		hw_if->enable_rx_interrupt(chInx, pdata);
	}

	DBGPR("<--DWC_ETH_QOS_enable_all_ch_rx_interrpt\n");
}

/*!
* \brief Interrupt Service Routine
* \details Interrupt Service Routine
*
* \param[in] irq         - interrupt number for particular device
* \param[in] device_id   - pointer to device structure
* \return returns positive integer
* \retval IRQ_HANDLED
*/

irqreturn_t DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS_DUMMY(int irq, void *device_id)
{
	NDBGPR_L2("Dummy ISR called\n");
	return IRQ_HANDLED;
}

#if 0
static void print_time2()
{
	struct timespec now;
	unsigned long long now_ns;
	int diff;
	static unsigned long long prev_ns = 0xFFFFFFFFFFFFFFFF;

	getnstimeofday(&now);

	now_ns = now.tv_sec;
	now_ns = (now_ns*1000000000)+now.tv_nsec;

	if(prev_ns == 0xFFFFFFFFFFFFFFFF)
		prev_ns = now_ns;

	diff = now_ns - prev_ns;

	if(diff > 1000000) // ms
		NMSGPR_INFO("ISR Entry Gap = %d\n", diff);
	prev_ns = now_ns;
}


static void print_time(int val)
{
        struct timespec now;
        unsigned long long now_ns;
        int diff;
        static unsigned long long prev_ns = 0xFFFFFFFFFFFFFFFF;

        getnstimeofday(&now);

        now_ns = now.tv_sec;
        now_ns = (now_ns*1000000000)+now.tv_nsec;

	if(prev_ns == 0xFFFFFFFFFFFFFFFF)
		prev_ns = now_ns;

	if(val)
	{
		diff = now_ns - prev_ns;

		if(diff > 500000) //
	        	NMSGPR_INFO("ISR Execution time = %d\n", diff);
	}
	else{
		prev_ns = now_ns;
		//NMSGPR_INFO("first time   :  ");
	}
}
#endif

irqreturn_t DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS(int irq, void *device_id)
{
	ULONG varNTN_INTC_INTSTATUS;
	ULONG varNTN_INTC_MACSTATUS;
	ULONG varNTN_INTC_INTMCUMASK1;
        ULONG var_NTN_INTC_TDMSTATUS;
	ULONG var_raw_DMA_RXCHSTS;
	ULONG varDMA_RXCHSTS;
	ULONG var_raw_DMA_TXCHSTS;
	ULONG varDMA_TXCHSTS;
	ULONG varDMA_RXCHINTMASK;
	ULONG varDMA_TXCHINTMASK;
	ULONG varBitField;
	ULONG varMAC_ISR;
	ULONG varMAC_IMR;
	ULONG varMAC_PMTCSR;
	struct DWC_ETH_QOS_prv_data *pdata =
	    (struct DWC_ETH_QOS_prv_data *)device_id;
	struct net_device *dev = pdata->dev;
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT chInx, chSts;
	ULONG temp;
	int napi_sched = 0;
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = NULL;
	ULONG varMAC_ANS = 0;
	ULONG varMAC_PCS = 0;

	DBGPR("-->DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS\n");


        /* TOP level status register: Check if there is any interrupt from EMAC, if not return with IRQ_NONE. */
        NTN_INTC_INTSTATUS_RgRd(varNTN_INTC_INTSTATUS);

	if (GET_VALUE(varNTN_INTC_INTSTATUS, NTN_INTC_INTSTATUS_TXRX_LPOS, NTN_INTC_INTSTATUS_TXRX_HPOS))
	{
		NTN_INTC_MACSTATUS_RgRd(varNTN_INTC_MACSTATUS);

	/* Handle DMA RX interrupts */
	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {

			/* Check if Current DMA channel didn't raise an interrupt, continue to next channel */
			if(!GET_VALUE(varNTN_INTC_MACSTATUS, (chInx+16), (chInx+16) ))
				continue;
        if(!pdata->rx_dma_ch_for_host[chInx])
            continue;

		if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0) {
			pr_debug("ISR is routed to IPA uC for RXCH0. Skip for this channel \n");
			continue;
		}

            	/* DMA RX Channel level status register */
			DMA_RXCHSTS_RgRd(chInx, var_raw_DMA_RXCHSTS);
	    	/* clear interrupts. This will clear INTC interrupt status as well. */
            	DMA_RXCHSTS_RgWr(chInx, var_raw_DMA_RXCHSTS);
   	    	NDBGPR_TS1("Raw DMA_RXCHSTS[%d] = %#lx\n", chInx, var_raw_DMA_RXCHSTS);

            	/* get enabled interrupt mask */
            	DMA_RXCHINTMASK_RgRd(chInx, varDMA_RXCHINTMASK);
            	varDMA_RXCHINTMASK &= DMA_RXCHINTMASK_MASK;

	    	/* handle only those DMA interrupts which are enabled */
	    	varDMA_RXCHSTS = (var_raw_DMA_RXCHSTS & varDMA_RXCHINTMASK);
			NDBGPR_L2( "DMA_RXCHSTS[%d] = %#lx\n", chInx, varDMA_RXCHSTS);

			if(0 == varDMA_RXCHSTS)
				continue;

		if ((GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_RC_LPOS, DMA_RXCHSTS_RC_HPOS) & 1) ||
			(GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_UNF_LPOS, DMA_RXCHSTS_UNF_HPOS) & 1)) {

				if (!napi_sched) {
					rx_dma_ch = GET_RX_DMA_CH_PTR(chInx);
					napi_sched = 1;
					if (likely(napi_schedule_prep(&rx_dma_ch->napi))) {
						DWC_ETH_QOS_disable_all_ch_rx_interrpt(pdata);
							NDBGPR_TS2("Scheduling RX Poll\n");
						__napi_schedule(&rx_dma_ch->napi);
					} else {
						DWC_ETH_QOS_disable_all_ch_rx_interrpt(pdata);
					}

					if ((GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_RC_LPOS, DMA_RXCHSTS_RC_HPOS) & 1))
						pdata->xstats.rx_normal_irq_n[chInx]++;
					else{
						pdata->xstats.rx_buf_unavailable_irq_n[chInx]++;
							NDBGPR_L2( "RX Buffer unavalialbe for Channel = %d\n", chInx);
				}
			}
		}
		if (GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_RS_LPOS, DMA_RXCHSTS_RS_HPOS) & 1) {
			pdata->xstats.rx_process_stopped_irq_n[chInx]++;
				NDBGPR_L2("RX Stopped for Channel = %d\n", chInx);
			DWC_ETH_QOS_GStatus = -E_DMA_RX_TS;
		}
		if (GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_FE_LPOS, DMA_RXCHSTS_FE_HPOS) & 1) {
			pdata->xstats.fatal_bus_error_irq_n++;
			DWC_ETH_QOS_GStatus = -E_DMA_RX_FE;

			DMA_RXCHSTS_FESTS_UdfRd(chInx, varBitField);
			NDBGPR_L2("RX Fatal Bus error Status %#lx\n", varBitField);

			DMA_RXCHSTS_CHSTS_UdfRd(chInx, varBitField);
			NDBGPR_L2("RX Channel Status %#lx\n", varBitField);

			pdata->restart_channel_idx = chInx;
			schedule_work(&pdata->restartdev_work);
		}

			chSts = GET_VALUE(var_raw_DMA_RXCHSTS, DMA_RXCHSTS_CHSTS_LPOS, DMA_RXCHSTS_CHSTS_HPOS);
			switch(chSts)
			{
				case DMA_RXCHSTS_CHSTS_SUSPEND:
					//NMSGPR_ALERT( "RX Channel %d Suspended\n", chInx);
					break;
				case DMA_RXCHSTS_CHSTS_STOP:
					DMA_RXCHCTL_RgRd(chInx, temp);
					NDBGPR_L2("RX Channel %d Stopped : CTL = %lx\n", chInx, temp);
					break;
			}
    	}

	/* Handle DMA TX interrupts */
	for (chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++) {

			/* Check if Current DMA channel didn't raise an interrupt, continue to next channel */
			if(!GET_VALUE(varNTN_INTC_MACSTATUS, (chInx), (chInx) ))
            continue;

			if(!pdata->tx_dma_ch_for_host[chInx])
                        continue;

		if (pdata->ipa_enabled && chInx == NTN_TX_DMA_CH_2) {
			pr_debug("TX interrupts are handled by IPA uc for TXCH2.skip for the host \n");
			continue;
		}

                /* DMA TX Channel level status register */
                DMA_TXCHSTS_RgRd(chInx, var_raw_DMA_TXCHSTS);
                /* clear interrupts. This will clear INTC interrupt status as well. */
                DMA_TXCHSTS_RgWr(chInx, var_raw_DMA_TXCHSTS);
                NDBGPR_TS1("Raw DMA_TXCHSTS[%d] = %#lx\n", chInx, var_raw_DMA_TXCHSTS);

                /* get enabled interrupt mask */
        	DMA_TXCHINTMASK_RgRd(chInx, varDMA_TXCHINTMASK);
        	varDMA_TXCHINTMASK &= DMA_TXCHINTMASK_MASK;

		/* handle only those DMA interrupts which are enabled */
		varDMA_TXCHSTS = (var_raw_DMA_TXCHSTS & varDMA_TXCHINTMASK);
		NDBGPR_L2("DMA_TXCHSTS[%d] = %#lx\n", chInx, varDMA_TXCHSTS);

		if (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_TC_LPOS, DMA_TXCHSTS_TC_HPOS) & 1) {
			pdata->xstats.tx_normal_irq_n[chInx]++;
			DWC_ETH_QOS_tx_interrupt(dev, pdata, chInx);
		}
		if (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_TS_LPOS, DMA_TXCHSTS_TS_HPOS) & 1) {
			pdata->xstats.tx_process_stopped_irq_n[chInx]++;
				NDBGPR_L2("TX Stopped for Channel = %d\n", chInx);
			DWC_ETH_QOS_GStatus = -E_DMA_TX_TS;
		}
		if (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_UNF_LPOS, DMA_TXCHSTS_UNF_HPOS) & 1) {
			pdata->xstats.tx_buf_unavailable_irq_n[chInx]++;
			DWC_ETH_QOS_GStatus = -E_DMA_TX_UNF;
			if( (chInx == NTN_TX_PKT_AVB_CLASS_A) || (chInx == NTN_TX_PKT_AVB_CLASS_B) )
				NDBGPR_L2( "TX Buffer underflow interrupt, It should be avoided for AVB traffic\n");
		}

		if (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_FE_LPOS, DMA_TXCHSTS_FE_HPOS) & 1) {
			pdata->xstats.fatal_bus_error_irq_n++;
			DWC_ETH_QOS_GStatus = -E_DMA_TX_FE;

                        DMA_TXCHSTS_FESTS_UdfRd(chInx, varBitField);
            NDBGPR_L2("TX Fatal Bus error Status %#lx\n", varBitField);

                        DMA_TXCHSTS_CHSTS_UdfRd(chInx, varBitField);
            NDBGPR_L2("TX Channel Status %#lx\n", varBitField);

			pdata->restart_channel_idx = chInx;
			schedule_work(&pdata->restartdev_work);
		}

		if ( (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_ETC_LPOS, DMA_TXCHSTS_ETC_HPOS) & 1) ){
			NDBGPR_L2( "ETC Interrupt is not handled\n");
		}
		if ( (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_CDE_LPOS, DMA_TXCHSTS_CDE_HPOS) & 1) ){
			NDBGPR_L2("CDE Interrupt is not handled\n");
		}

			chSts = GET_VALUE(var_raw_DMA_TXCHSTS, DMA_TXCHSTS_CHSTS_LPOS, DMA_TXCHSTS_CHSTS_HPOS);
			switch(chSts)
			{
				case DMA_TXCHSTS_CHSTS_SUSPEND:
					//NMSGPR_ALERT( "TX Channel %d Suspended\n", chInx);
					break;
				case DMA_TXCHSTS_CHSTS_STOP:
					NDBGPR_L2("TX Channel %d Stopped\n", chInx);
					break;
				default:
				    break;
			}

    }
	}

	/* Handle MAC interrupts */
	if (GET_VALUE(varNTN_INTC_INTSTATUS, NTN_INTC_INTSTATUS_MAC_LPOS, NTN_INTC_INTSTATUS_MAC_HPOS)) {

		MAC_ISR_RgRd(varMAC_ISR);
		NDBGPR_L2("MAC_ISR = %#lx\n", varMAC_ISR);

		/* handle only those MAC interrupts which are enabled */
		MAC_IMR_RgRd(varMAC_IMR);
		varMAC_ISR = (varMAC_ISR & varMAC_IMR);

		/* PMT interrupt */
		if (GET_VALUE(varMAC_ISR, MAC_ISR_PMTIS_LPOS, MAC_ISR_PMTIS_HPOS) & 1) {
			pdata->xstats.pmt_irq_n++;
			DWC_ETH_QOS_GStatus = S_MAC_ISR_PMTIS;
			MAC_PMTCSR_RgRd(varMAC_PMTCSR);
			if (pdata->power_down)
				schedule_work(&pdata->powerup_work);
		}

		/* RGMII/SMII interrupt */
		if (GET_VALUE(varMAC_ISR, MAC_ISR_RGSMIIS_LPOS, MAC_ISR_RGSMIIS_HPOS) & 1) {
			MAC_PCS_RgRd(varMAC_PCS);
			NDBGPR_L2("RGMII/SMII interrupt: MAC_PCS = %#lx\n", varMAC_PCS);
			if ((varMAC_PCS & 0x80000) == 0x80000) {
				pdata->pcs_link = 1;
				netif_carrier_on(dev);
				if ((varMAC_PCS & 0x10000) == 0x10000) {
					pdata->pcs_duplex = 1;
					hw_if->set_full_duplex(pdata);
				} else {
					pdata->pcs_duplex = 0;
					hw_if->set_half_duplex(pdata);
				}

				if ((varMAC_PCS & 0x60000) == 0x0) {
					pdata->pcs_speed = SPEED_10;
					hw_if->set_mii_speed_10(pdata);
				} else if ((varMAC_PCS & 0x60000) == 0x20000) {
					pdata->pcs_speed = SPEED_100;
					hw_if->set_mii_speed_100(pdata);
				} else if ((varMAC_PCS & 0x60000) == 0x40000) {
					pdata->pcs_speed = SPEED_1000;
					hw_if->set_gmii_speed(pdata);
				}
				NMSGPR_ALERT( "Link is UP:%dMbps & %s duplex\n",
					pdata->pcs_speed, pdata->pcs_duplex ? "Full" : "Half");
			} else {
				NMSGPR_ALERT( "Link is Down\n");
				pdata->pcs_link = 0;
				netif_carrier_off(dev);
			}
		}

		/* PCS Link Status interrupt */
		if (GET_VALUE(varMAC_ISR, MAC_ISR_PCSLCHGIS_LPOS, MAC_ISR_PCSLCHGIS_HPOS) & 1) {
			NDBGPR_L2("PCS Link Status interrupt\n");
			MAC_ANS_RgRd(varMAC_ANS);
			if (GET_VALUE(varMAC_ANS, MAC_ANS_LS_LPOS, MAC_ANS_LS_HPOS) & 1) {
				NMSGPR_ALERT( "Link: Up\n");
				netif_carrier_on(dev);
				pdata->pcs_link = 1;
			} else {
				NMSGPR_ALERT( "Link: Down\n");
				netif_carrier_off(dev);
				pdata->pcs_link = 0;
			}
		}

		/* PCS Auto-Negotiation Complete interrupt */
		if (GET_VALUE(varMAC_ISR, MAC_ISR_PCSANCIS_LPOS, MAC_ISR_PCSANCIS_HPOS) & 1) {
			NDBGPR_L2("PCS Auto-Negotiation Complete interrupt\n");
			MAC_ANS_RgRd(varMAC_ANS);
		}

		/* EEE interrupts */
		if (GET_VALUE(varMAC_ISR, MAC_ISR_LPI_LPOS, MAC_ISR_LPI_HPOS) & 1) {
			DWC_ETH_QOS_handle_eee_interrupt(pdata);
		}
	}

	/* Handle MAC LPI interrupts */
	if (GET_VALUE(varNTN_INTC_INTSTATUS, NTN_INTC_INTSTATUS_MAC_LPI_LPOS, NTN_INTC_INTSTATUS_MAC_LPI_HPOS)) {
		NDBGPR_L2("Neutrino INTC MAC LPI status bit is set.\n");
        }

	/* Handle MAC PM interrupts */
	if (GET_VALUE(varNTN_INTC_INTSTATUS, NTN_INTC_INTSTATUS_MAC_PM_LPOS, NTN_INTC_INTSTATUS_MAC_PM_HPOS)) {
		NDBGPR_L2("Neutrino INTC MAC PM status bit is set.\n");
        }

	/* Handle TDM interrupts */
	if (GET_VALUE(varNTN_INTC_INTSTATUS, NTN_INTC_INTSTATUS_TDM_LPOS, NTN_INTC_INTSTATUS_TDM_HPOS)) {
		NTN_INTC_TDMSTATUS_RgRd(var_NTN_INTC_TDMSTATUS);
		NTN_INTC_TDMSTATUS_RgWr(var_NTN_INTC_TDMSTATUS);

		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_IP_0_OVERFLOW){
			NDBGPR_L2("Neutrino TDM Input 0 Overflow detected.\n");
		}
		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_IP_1_OVERFLOW){
			NDBGPR_L2("Neutrino TDM Input 1 Overflow detected.\n");
		}
		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_IP_2_OVERFLOW){
			NDBGPR_L2("Neutrino TDM Input 2 Overflow detected.\n");
		}
		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_IP_3_OVERFLOW){
			NDBGPR_L2("Neutrino TDM Input 3 Overflow detected.\n");
		}
		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_OP_OVERFLOW){
			NDBGPR_L2("Neutrino TDM Output Overflow detected.\n");
		}
		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_OP_UNDERFLOW){
			NDBGPR_L2("Neutrino TDM Output Underflow detected.\n");
		}
		if(var_NTN_INTC_TDMSTATUS & NTN_INTC_TDM_GENERAL_ERROR){
			NDBGPR_L2("Neutrino TDM General error detected.\n");
		}
        }

	/* Read interrupt mask */
	NTN_INTC_INTMCUMASK1_RgRd(varNTN_INTC_INTMCUMASK1);

	if (pdata->ipa_enabled) {
		/* Enable all GMAC interrupts apart from offload channels controlled by IPA*/
		varNTN_INTC_INTMCUMASK1 &= (NTN_INTC_GMAC_INT_MASK |
					(1 << NTN_INTC_GMAC_INT_TXCH2_BIT_POS) |
					(1 << NTN_INTC_GMAC_INT_RXCH0_BIT_POS));
	} else {
		/* Enable interrupts again : Only TX & RX DMA channel interrupts */
		varNTN_INTC_INTMCUMASK1 &= NTN_INTC_GMAC_INT_MASK;
	}

	NTN_INTC_INTMCUMASK1_RgWr(varNTN_INTC_INTMCUMASK1);

	DBGPR("<--DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS\n");

	return IRQ_HANDLED;
}

/*!
* \brief API to get all hw features.
*
* \details This function is used to check what are all the different
* features the device supports.
*
* \param[in] pdata - pointer to driver private structure
*
* \return none
*/

void DWC_ETH_QOS_get_all_hw_features(struct DWC_ETH_QOS_prv_data *pdata)
{
	unsigned int varMAC_HFR0;
	unsigned int varMAC_HFR1;
	unsigned int varMAC_HFR2;

	DBGPR("-->DWC_ETH_QOS_get_all_hw_features\n");

	MAC_HFR0_RgRd(varMAC_HFR0);
	MAC_HFR1_RgRd(varMAC_HFR1);
	MAC_HFR2_RgRd(varMAC_HFR2);

	memset(&pdata->hw_feat, 0, sizeof(pdata->hw_feat));
	pdata->hw_feat.mii_sel = ((varMAC_HFR0 >> 0) & MAC_HFR0_MIISEL_Mask);
	pdata->hw_feat.gmii_sel = ((varMAC_HFR0 >> 1) & MAC_HFR0_GMIISEL_Mask);
	pdata->hw_feat.hd_sel = ((varMAC_HFR0 >> 2) & MAC_HFR0_HDSEL_Mask);
	pdata->hw_feat.pcs_sel = ((varMAC_HFR0 >> 3) & MAC_HFR0_PCSSEL_Mask);
	pdata->hw_feat.vlan_hash_en =
	    ((varMAC_HFR0 >> 4) & MAC_HFR0_VLANHASEL_Mask);
	pdata->hw_feat.sma_sel = ((varMAC_HFR0 >> 5) & MAC_HFR0_SMASEL_Mask);
	pdata->hw_feat.rwk_sel = ((varMAC_HFR0 >> 6) & MAC_HFR0_RWKSEL_Mask);
	pdata->hw_feat.mgk_sel = ((varMAC_HFR0 >> 7) & MAC_HFR0_MGKSEL_Mask);
	pdata->hw_feat.mmc_sel = ((varMAC_HFR0 >> 8) & MAC_HFR0_MMCSEL_Mask);
	pdata->hw_feat.arp_offld_en =
	    ((varMAC_HFR0 >> 9) & MAC_HFR0_ARPOFFLDEN_Mask);
	pdata->hw_feat.ts_sel =
	    ((varMAC_HFR0 >> 12) & MAC_HFR0_TSSSEL_Mask);
	pdata->hw_feat.eee_sel = ((varMAC_HFR0 >> 13) & MAC_HFR0_EEESEL_Mask);
	pdata->hw_feat.tx_coe_sel =
	    ((varMAC_HFR0 >> 14) & MAC_HFR0_TXCOESEL_Mask);
	pdata->hw_feat.rx_coe_sel =
	    ((varMAC_HFR0 >> 16) & MAC_HFR0_RXCOE_Mask);
	pdata->hw_feat.mac_addr16_sel =
	    ((varMAC_HFR0 >> 18) & MAC_HFR0_ADDMACADRSEL_Mask);
	pdata->hw_feat.mac_addr32_sel =
	    ((varMAC_HFR0 >> 23) & MAC_HFR0_MACADR32SEL_Mask);
	pdata->hw_feat.mac_addr64_sel =
	    ((varMAC_HFR0 >> 24) & MAC_HFR0_MACADR64SEL_Mask);
	pdata->hw_feat.tsstssel =
	    ((varMAC_HFR0 >> 25) & MAC_HFR0_TSINTSEL_Mask);
	pdata->hw_feat.sa_vlan_ins =
	    ((varMAC_HFR0 >> 27) & MAC_HFR0_SAVLANINS_Mask);
	pdata->hw_feat.act_phy_sel =
	    ((varMAC_HFR0 >> 28) & MAC_HFR0_ACTPHYSEL_Mask);

	pdata->hw_feat.rx_fifo_size =
	    ((varMAC_HFR1 >> 0) & MAC_HFR1_RXFIFOSIZE_Mask);
	    //8;
	pdata->hw_feat.tx_fifo_size =
	    ((varMAC_HFR1 >> 6) & MAC_HFR1_TXFIFOSIZE_Mask);
	    //8;
	pdata->hw_feat.adv_ts_hword =
	    ((varMAC_HFR1 >> 13) & MAC_HFR1_ADVTHWORD_Mask);
	pdata->hw_feat.dcb_en = ((varMAC_HFR1 >> 16) & MAC_HFR1_DCBEN_Mask);
	pdata->hw_feat.sph_en = ((varMAC_HFR1 >> 17) & MAC_HFR1_SPHEN_Mask);
	pdata->hw_feat.tso_en = ((varMAC_HFR1 >> 18) & MAC_HFR1_TSOEN_Mask);
	pdata->hw_feat.dma_debug_gen =
	    ((varMAC_HFR1 >> 19) & MAC_HFR1_DMADEBUGEN_Mask);
	pdata->hw_feat.av_sel = ((varMAC_HFR1 >> 20) & MAC_HFR1_AVSEL_Mask);
	pdata->hw_feat.lp_mode_en =
	    ((varMAC_HFR1 >> 23) & MAC_HFR1_LPMODEEN_Mask);
	pdata->hw_feat.hash_tbl_sz =
	    ((varMAC_HFR1 >> 24) & MAC_HFR1_HASHTBLSZ_Mask);
	pdata->hw_feat.l3l4_filter_num =
	    ((varMAC_HFR1 >> 27) & MAC_HFR1_L3L4FILTERNUM_Mask);

	pdata->hw_feat.rx_q_cnt = ((varMAC_HFR2 >> 0) & MAC_HFR2_RXQCNT_Mask);
	pdata->hw_feat.tx_q_cnt = ((varMAC_HFR2 >> 6) & MAC_HFR2_TXQCNT_Mask);
	pdata->hw_feat.rx_ch_cnt =
	    ((varMAC_HFR2 >> 12) & MAC_HFR2_RXCHCNT_Mask);
	pdata->hw_feat.tx_ch_cnt =
	    ((varMAC_HFR2 >> 18) & MAC_HFR2_TXCHCNT_Mask);
	pdata->hw_feat.pps_out_num =
	    ((varMAC_HFR2 >> 24) & MAC_HFR2_PPSOUTNUM_Mask);
	pdata->hw_feat.aux_snap_num =
	    ((varMAC_HFR2 >> 28) & MAC_HFR2_AUXSNAPNUM_Mask);

	DBGPR("<--DWC_ETH_QOS_get_all_hw_features\n");
}


/*!
* \brief API to print all hw features.
*
* \details This function is used to print all the device feature.
*
* \param[in] pdata - pointer to driver private structure
*
* \return none
*/

void DWC_ETH_QOS_print_all_hw_features(struct DWC_ETH_QOS_prv_data *pdata)
{
	char *str = NULL;
	int phy_mode;

	DBGPR("-->DWC_ETH_QOS_print_all_hw_features\n");

	DBGPR( "\n");
	DBGPR( "=====================================================/\n");
	DBGPR( "\n");
	DBGPR( "10/100 Mbps Support                         : %s\n",
		pdata->hw_feat.mii_sel ? "YES" : "NO");
	DBGPR( "1000 Mbps Support                           : %s\n",
		pdata->hw_feat.gmii_sel ? "YES" : "NO");
	DBGPR( "Half-duplex Support                         : %s\n",
		pdata->hw_feat.hd_sel ? "YES" : "NO");
	DBGPR( "PCS Registers(TBI/SGMII/RTBI PHY interface) : %s\n",
		pdata->hw_feat.pcs_sel ? "YES" : "NO");
	DBGPR( "VLAN Hash Filter Selected                   : %s\n",
		pdata->hw_feat.vlan_hash_en ? "YES" : "NO");
	pdata->vlan_hash_filtering = pdata->hw_feat.vlan_hash_en;
	DBGPR( "SMA (MDIO) Interface                        : %s\n",
		pdata->hw_feat.sma_sel ? "YES" : "NO");
	DBGPR( "PMT Remote Wake-up Packet Enable            : %s\n",
		pdata->hw_feat.rwk_sel ? "YES" : "NO");
	DBGPR( "PMT Magic Packet Enable                     : %s\n",
		pdata->hw_feat.mgk_sel ? "YES" : "NO");
	DBGPR( "RMON/MMC Module Enable                      : %s\n",
		pdata->hw_feat.mmc_sel ? "YES" : "NO");
	DBGPR( "ARP Offload Enabled                         : %s\n",
		pdata->hw_feat.arp_offld_en ? "YES" : "NO");
	DBGPR( "IEEE 1588-2008 Timestamp Enabled            : %s\n",
		pdata->hw_feat.ts_sel ? "YES" : "NO");
	DBGPR( "Energy Efficient Ethernet Enabled           : %s\n",
		pdata->hw_feat.eee_sel ? "YES" : "NO");
	DBGPR( "Transmit Checksum Offload Enabled           : %s\n",
		pdata->hw_feat.tx_coe_sel ? "YES" : "NO");
	DBGPR( "Receive Checksum Offload Enabled            : %s\n",
		pdata->hw_feat.rx_coe_sel ? "YES" : "NO");
	DBGPR( "MAC Addresses 16–31 Selected                : %s\n",
		pdata->hw_feat.mac_addr16_sel ? "YES" : "NO");
	DBGPR( "MAC Addresses 32–63 Selected                : %s\n",
		pdata->hw_feat.mac_addr32_sel ? "YES" : "NO");
	DBGPR( "MAC Addresses 64–127 Selected               : %s\n",
		pdata->hw_feat.mac_addr64_sel ? "YES" : "NO");
	DBGPR( "IPA Feature Enabled                          : %s\n",
		pdata->ipa_enabled ? "YES" : "NO");
	if (pdata->hw_feat.mac_addr64_sel)
		pdata->max_addr_reg_cnt = 128;
	else if (pdata->hw_feat.mac_addr32_sel)
		pdata->max_addr_reg_cnt = 64;
	else if (pdata->hw_feat.mac_addr16_sel)
		pdata->max_addr_reg_cnt = 32;
	else
		pdata->max_addr_reg_cnt = 1;

	switch(pdata->hw_feat.tsstssel) {
	case 0:
		str = "RESERVED";
		break;
	case 1:
		str = "INTERNAL";
		break;
	case 2:
		str = "EXTERNAL";
		break;
	case 3:
		str = "BOTH";
		break;
	}
	DBGPR( "Timestamp System Time Source                : %s\n",
		str);
	DBGPR( "Source Address or VLAN Insertion Enable     : %s\n",
		pdata->hw_feat.sa_vlan_ins ? "YES" : "NO");

	if (pdata->rmii_mode) {
		phy_mode = DWC_ETH_QOS_RMII;
	} else {
		phy_mode = pdata->hw_feat.act_phy_sel;
	}

	switch (phy_mode) {
	case 0:
		str = "GMII/MII";
		break;
	case 1:
		str = "RGMII";
		break;
	case 2:
		str = "SGMII";
		break;
	case 3:
		str = "TBI";
		break;
	case 4:
		str = "RMII";
		break;
	case 5:
		str = "RTBI";
		break;
	case 6:
		str = "SMII";
		break;
	case 7:
		str = "RevMII";
		break;
	default:
		str = "RESERVED";
	}
	DBGPR( "Active PHY Selected                         : %s\n",
		str);

	switch(pdata->hw_feat.rx_fifo_size) {
	case 0:
		str = "128 bytes";
		break;
	case 1:
		str = "256 bytes";
		break;
	case 2:
		str = "512 bytes";
		break;
	case 3:
		str = "1 KBytes";
		break;
	case 4:
		str = "2 KBytes";
		break;
	case 5:
		str = "4 KBytes";
		break;
	case 6:
		str = "8 KBytes";
		break;
	case 7:
		str = "16 KBytes";
		break;
	case 8:
		str = "32 kBytes";
		break;
	case 9:
		str = "64 KBytes";
		break;
	case 10:
		str = "128 KBytes";
		break;
	case 11:
		str = "256 KBytes";
		break;
	default:
		str = "RESERVED";
	}
	DBGPR( "MTL Receive FIFO Size                       : %s\n",
		str);

	switch(pdata->hw_feat.tx_fifo_size) {
	case 0:
		str = "128 bytes";
		break;
	case 1:
		str = "256 bytes";
		break;
	case 2:
		str = "512 bytes";
		break;
	case 3:
		str = "1 KBytes";
		break;
	case 4:
		str = "2 KBytes";
		break;
	case 5:
		str = "4 KBytes";
		break;
	case 6:
		str = "8 KBytes";
		break;
	case 7:
		str = "16 KBytes";
		break;
	case 8:
		str = "32 kBytes";
		break;
	case 9:
		str = "64 KBytes";
		break;
	case 10:
		str = "128 KBytes";
		break;
	case 11:
		str = "256 KBytes";
		break;
	default:
		str = "RESERVED";
	}
	DBGPR( "MTL Transmit FIFO Size                       : %s\n",
		str);
	DBGPR( "IEEE 1588 High Word Register Enable          : %s\n",
		pdata->hw_feat.adv_ts_hword ? "YES" : "NO");
	DBGPR( "DCB Feature Enable                           : %s\n",
		pdata->hw_feat.dcb_en ? "YES" : "NO");
	DBGPR( "Split Header Feature Enable                  : %s\n",
		pdata->hw_feat.sph_en ? "YES" : "NO");
	DBGPR( "TCP Segmentation Offload Enable              : %s\n",
		pdata->hw_feat.tso_en ? "YES" : "NO");
	DBGPR( "DMA Debug Registers Enabled                  : %s\n",
		pdata->hw_feat.dma_debug_gen ? "YES" : "NO");
	DBGPR( "AV Feature Enabled                           : %s\n",
		pdata->hw_feat.av_sel ? "YES" : "NO");
	DBGPR( "Low Power Mode Enabled                       : %s\n",
		pdata->hw_feat.lp_mode_en ? "YES" : "NO");

	switch(pdata->hw_feat.hash_tbl_sz) {
	case 1:
		str = "64";
		pdata->max_hash_table_size = 64;
		break;
	default:
		str = "No hash table selected";
		pdata->max_hash_table_size = 0;
		break;
	}
	DBGPR( "Hash Table Size                              : %s\n",
		str);
	DBGPR( "Total number of L3 or L4 Filters             : %d L3/L4 Filter\n",
		pdata->hw_feat.l3l4_filter_num);
	DBGPR( "Number of MTL Receive Queues                 : %d\n",
		(pdata->hw_feat.rx_q_cnt + 1));
	DBGPR( "Number of MTL Transmit Queues                : %d\n",
		(pdata->hw_feat.tx_q_cnt + 1));
	DBGPR( "Number of DMA Receive Channels               : %d\n",
		(pdata->hw_feat.rx_ch_cnt + 1));
	DBGPR( "Number of DMA Transmit Channels              : %d\n",
		(pdata->hw_feat.tx_ch_cnt + 1));

	switch(pdata->hw_feat.pps_out_num) {
	case 0:
		str = "No PPS output";
		break;
	case 1:
		str = "1 PPS output";
		break;
	case 2:
		str = "2 PPS output";
		break;
	case 3:
		str = "3 PPS output";
		break;
	case 4:
		str = "4 PPS output";
		break;
	default:
		str = "RESERVED";
	}
	DBGPR( "Number of PPS Outputs                        : %s\n",
		str);

	switch(pdata->hw_feat.aux_snap_num) {
	case 0:
		str = "No auxillary input";
		break;
	case 1:
		str = "1 auxillary input";
		break;
	case 2:
		str = "2 auxillary input";
		break;
	case 3:
		str = "3 auxillary input";
		break;
	case 4:
		str = "4 auxillary input";
		break;
	default:
		str = "RESERVED";
	}
	DBGPR( "Number of Auxiliary Snapshot Inputs          : %s",
		str);

	DBGPR( "\n");
	DBGPR( "=====================================================/\n");

	DBGPR("<--DWC_ETH_QOS_print_all_hw_features\n");
}


static const struct net_device_ops DWC_ETH_QOS_netdev_ops = {
	.ndo_open = DWC_ETH_QOS_open,
	.ndo_stop = DWC_ETH_QOS_close,
	.ndo_start_xmit = DWC_ETH_QOS_start_xmit,
	.ndo_get_stats = DWC_ETH_QOS_get_stats,
	.ndo_set_rx_mode = DWC_ETH_QOS_set_rx_mode,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = DWC_ETH_QOS_poll_controller,
#endif				/*end of CONFIG_NET_POLL_CONTROLLER */
	.ndo_set_features = DWC_ETH_QOS_set_features,
	.ndo_fix_features = DWC_ETH_QOS_fix_features,
	.ndo_do_ioctl = DWC_ETH_QOS_ioctl,
#ifdef DWC_ETH_QOS_QUEUE_SELECT_ALGO
	.ndo_select_queue = DWC_ETH_QOS_select_queue,
#endif
	.ndo_vlan_rx_add_vid = DWC_ETH_QOS_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid = DWC_ETH_QOS_vlan_rx_kill_vid,
	.ndo_change_mtu         = DWC_ETH_QOS_change_mtu,
};

struct net_device_ops *DWC_ETH_QOS_get_netdev_ops(void)
{
	return (struct net_device_ops *)&DWC_ETH_QOS_netdev_ops;
}


/*!
 * \brief allcation of Rx skb's for default rx mode.
 *
 * \details This function is invoked by other api's for
 * allocating the Rx skb's with default Rx mode ie non-jumbo
 * and non-split header mode.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] buffer – pointer to wrapper receive buffer data structure.
 * \param[in] gfp – the type of memory allocation.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */

static int DWC_ETH_QOS_alloc_rx_buf(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata,
				    struct DWC_ETH_QOS_rx_buffer *buffer,
				    gfp_t gfp)
{
	struct sk_buff *skb = buffer->skb;
	DBGPR("-->DWC_ETH_QOS_alloc_rx_buf\n");

	if (skb) {
		skb_trim(skb, 0);
		goto map_skb;
	}

	if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0)
		skb = __netdev_alloc_skb_ip_align(pdata->dev, DWC_ETH_QOS_ETH_FRAME_LEN_IPA, gfp);
	else
		skb = __netdev_alloc_skb_ip_align(pdata->dev, pdata->rx_buffer_len, gfp);

	if (skb == NULL) {
		NMSGPR_ALERT( "Failed to allocate skb\n");
		return -ENOMEM;
	}
	buffer->skb = skb;
	if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0)
		buffer->len = DWC_ETH_QOS_ETH_FRAME_LEN_IPA;
	else
		buffer->len = pdata->rx_buffer_len;

 map_skb:

#ifdef NTN_RX_DATA_BUF_IN_SRAM
    //buffer->skb_temp_data_ptr: holds the address given by "__netdev_alloc_skb_ip_align" for data
    //buffer->dma : Holds device address
    //skb->data : holds the virtual address, which is mapped to device address (buffer->dma)

    buffer->skb_temp_data_ptr = skb->data;  //Store skb data pointer to free at the end.

    skb->data = dma_pool_alloc(pdata->rx_mem_pool, GFP_ATOMIC, &buffer->dma);

    if (skb->data == NULL) {
		NMSGPR_ALERT( "Failed to allocate dma coherent memory\n");
            skb->data = buffer->skb_temp_data_ptr;
		return -ENOMEM;
    }

	buffer->dma_iommu = dma_map_single(&pdata->pdev->dev, skb->data,
				     pdata->rx_buffer_len, DMA_FROM_DEVICE);
	if (dma_mapping_error(&pdata->pdev->dev, buffer->dma_iommu))
		NMSGPR_ALERT( "failed to do the RX dma map\n");
#else
	if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0)
		buffer->dma = dma_map_single(&pdata->pdev->dev, skb->data,
			DWC_ETH_QOS_ETH_FRAME_LEN_IPA, DMA_FROM_DEVICE);
	else
		buffer->dma = dma_map_single(&pdata->pdev->dev, skb->data,
				pdata->rx_buffer_len, DMA_FROM_DEVICE);

	if (dma_mapping_error(&pdata->pdev->dev, buffer->dma))
		NMSGPR_ALERT( "failed to do the RX dma map\n");

#endif

    NDBGPR_L2("RX skb->data = %#lx : buffer->dma = %#lx\n", (long unsigned int)skb->data, (long unsigned int)buffer->dma);

	buffer->mapped_as_page = Y_FALSE;

	DBGPR("<--DWC_ETH_QOS_alloc_rx_buf\n");

	return 0;
}


/*!
 * \brief api to configure Rx function pointer after reset.
 *
 * \details This function will initialize the receive function pointers
 * which are used for allocating skb's and receiving the packets based
 * Rx mode - default/jumbo/split header.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_configure_rx_fun_ptr(struct DWC_ETH_QOS_prv_data *pdata)
{
	int max_frame = 0;
	DBGPR("-->DWC_ETH_QOS_configure_rx_fun_ptr\n");
	max_frame = pdata->dev->mtu + ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN;;
	pdata->rx_buffer_len =  max_frame > DWC_ETH_QOS_ETH_FRAME_LEN ?
                                   ALIGN(max_frame, 8) : DWC_ETH_QOS_ETH_FRAME_LEN;

	pdata->clean_rx = DWC_ETH_QOS_clean_rx_irq;
	pdata->alloc_rx_buf = DWC_ETH_QOS_alloc_rx_buf;

	DBGPR("<--DWC_ETH_QOS_configure_rx_fun_ptr\n");
}


/*!
 * \brief api to initialize default values.
 *
 * \details This function is used to initialize differnet parameters to
 * default values which are common parameters between Tx and Rx path.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_default_common_confs(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->DWC_ETH_QOS_default_common_confs\n");

	pdata->drop_tx_pktburstcnt = 1;
	pdata->mac_enable_count = 0;
	pdata->incr_incrx = DWC_ETH_QOS_INCR_ENABLE;
	pdata->flow_ctrl = DWC_ETH_QOS_FLOW_CTRL_TX_RX;
	pdata->oldflow_ctrl = DWC_ETH_QOS_FLOW_CTRL_TX_RX;
	pdata->power_down = 0;
	pdata->tx_sa_ctrl_via_desc = DWC_ETH_QOS_SA0_NONE;
	pdata->tx_sa_ctrl_via_reg = DWC_ETH_QOS_SA0_NONE;
	pdata->hwts_tx_en = 0;
	pdata->hwts_rx_en = 0;
	pdata->l3_l4_filter = 0;
	//pdata->l2_filtering_mode = !!pdata->hw_feat.hash_tbl_sz;
	pdata->l2_filtering_mode = 0;
	pdata->tx_path_in_lpi_mode = 0;
	pdata->use_lpi_tx_automate = true;
	pdata->eee_active = 0;
	pdata->one_nsec_accuracy = 1;

	DBGPR("<--DWC_ETH_QOS_default_common_confs\n");
}


/*!
 * \brief api to initialize Tx parameters.
 *
 * \details This function is used to initialize all Tx
 * parameters to default values on reset.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] qInx – queue number to be initialized.
 *
 * \return void
 */

static void DWC_ETH_QOS_default_tx_confs_single_q(
		struct DWC_ETH_QOS_prv_data *pdata,
		UINT chInx)
{
	struct DWC_ETH_QOS_tx_dma_ch *queue_data;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data =
		GET_TX_WRAPPER_DESC(chInx);
	unsigned int qInx;

	DBGPR("-->DWC_ETH_QOS_default_tx_confs_single_q\n");

	qInx = NTN_GET_QINX_FROM_CHINX(chInx);
	queue_data = GET_TX_DMA_CH_PTR(qInx);
	queue_data->q_op_mode = q_op_mode[qInx];

	desc_data->tx_threshold_val = DWC_ETH_QOS_TX_THRESHOLD_32;
	desc_data->tsf_on = DWC_ETH_QOS_TSF_ENABLE;
	desc_data->osf_on = DWC_ETH_QOS_OSF_ENABLE;
	desc_data->tx_pbl = DWC_ETH_QOS_PBL_32;
	desc_data->tx_vlan_tag_via_reg = Y_FALSE;
	desc_data->tx_vlan_tag_ctrl = DWC_ETH_QOS_TX_VLAN_TAG_INSERT;
	desc_data->vlan_tag_present = 0;
	desc_data->context_setup = 0;
	desc_data->default_mss = 0;

	NDBGPR_L1("OSF on/off : %d for channel Idx %d\n", desc_data->osf_on, chInx);
	DBGPR("<--DWC_ETH_QOS_default_tx_confs_single_q\n");
}


/*!
 * \brief api to initialize Rx parameters.
 *
 * \details This function is used to initialize all Rx
 * parameters to default values on reset.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] chInx – channel number to be initialized.
 *
 * \return void
 */

static void DWC_ETH_QOS_default_rx_confs_single_q(
		struct DWC_ETH_QOS_prv_data *pdata,
		UINT chInx)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data =
		GET_RX_WRAPPER_DESC(chInx);

	DBGPR("-->DWC_ETH_QOS_default_rx_confs_single_q\n");

	desc_data->rx_threshold_val = DWC_ETH_QOS_RX_THRESHOLD_64;
	desc_data->rsf_on = DWC_ETH_QOS_RSF_DISABLE;
	desc_data->rx_pbl = DWC_ETH_QOS_PBL_16;
	desc_data->rx_outer_vlan_strip = DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS;
	desc_data->rx_inner_vlan_strip = DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS;

	DBGPR("<--DWC_ETH_QOS_default_rx_confs_single_q\n");
}

static void DWC_ETH_QOS_default_tx_confs(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_default_tx_confs\n");

	for (chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++) {
		if(!pdata->tx_dma_ch_for_host[chInx])
			continue;
		DWC_ETH_QOS_default_tx_confs_single_q(pdata, chInx);
	}

	DBGPR("<--DWC_ETH_QOS_default_tx_confs\n");
}

static void DWC_ETH_QOS_default_rx_confs(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_default_rx_confs\n");

	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		DWC_ETH_QOS_default_rx_confs_single_q(pdata, chInx);
	}

	DBGPR("<--DWC_ETH_QOS_default_rx_confs\n");
}

/*!
 *  \brief Disable Standard PHY ANEG (BMCR:)
 *  \details
 *
 *  \param[in] enb_dis
 *  \return Success or Failure
 *  \retval  0 Success
 *  \retval -1 Failure
 */

static INT DWC_ETH_QOS_config_phy_aneg (struct DWC_ETH_QOS_prv_data *pdata, unsigned int enb_dis, unsigned int restart) {
	int regval;

	enb_dis = !!enb_dis;
	restart = !!restart;
	DBGPR("-->DWC_ETH_QOS_config_phy_aneg: %s, %s\n",(enb_dis ? "Enable" : "Disable"),(restart ? "Restart" : "No Restart"));

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_BMCR, &regval);
	regval &= ~(BMCR_ANENABLE | BMCR_ANRESTART);
	if(enb_dis)
		regval |= BMCR_ANENABLE;
	if(restart)
		regval |= BMCR_ANRESTART;
	DWC_ETH_QOS_mdio_write_direct(pdata, pdata->phyaddr, MII_BMCR, regval);

	return Y_SUCCESS;
}

/*!
* \brief Common init function used on interface up and also resume
*
* \details Initializes common structures and queues during network
* initialization
*
* \param[in] dev - pointer to net_device structure
*
* \return void
*/
static void DWC_ETH_QOS_init_common(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &pdata->hw_if;
	struct desc_if_struct *desc_if = &(pdata->desc_if);

	DBGPR("-->DWC_ETH_QOS_init_common\n");

	/* default configuration */
	DWC_ETH_QOS_default_common_confs(pdata);
	DWC_ETH_QOS_default_tx_confs(pdata);
	DWC_ETH_QOS_default_rx_confs(pdata);
	DWC_ETH_QOS_configure_rx_fun_ptr(pdata);

	DWC_ETH_QOS_set_rx_mode(dev);

#ifndef DWC_ETH_QOS_BUILTIN
	DWC_ETH_QOS_config_phy_aneg(pdata, 0, 0);
#endif

	desc_if->wrapper_tx_desc_init(pdata);
	desc_if->wrapper_rx_desc_init(pdata);

	DWC_ETH_QOS_tx_desc_mang_ds_dump(pdata);
	DWC_ETH_QOS_rx_desc_mang_ds_dump(pdata);

	DWC_ETH_QOS_mmc_setup(pdata);

	/* initializes MAC and DMA */
	hw_if->init(pdata);

#ifndef DWC_ETH_QOS_BUILTIN
	DWC_ETH_QOS_config_phy_aneg(pdata, 1, 0);
#endif

	pdata->eee_enabled = DWC_ETH_QOS_eee_init(pdata);

	DBGPR("<--DWC_ETH_QOS_init_common\n");
}

static int DWC_ETH_QOS_request_irq(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	int ret = Y_SUCCESS;

	pdata->irq_number = dev->irq;

	enable_irq_wake(dev->irq);

#ifdef NTN_POLLING_METHOD
	ret = request_irq(pdata->irq_number, DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS_DUMMY,
					  IRQF_SHARED, DEV_NAME, pdata);
#else
	ret = request_irq(pdata->irq_number, DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS,
					  IRQF_SHARED, DEV_NAME, pdata);
	NDBGPR_L1("DWC_ETH_QOS IRQ %d requested\n", pdata->irq_number);
#endif

	if (ret != 0) {
		NMSGPR_ALERT("DWC_ETH_QOS request_irq failed ret %d\n", ret);
		ret = -EBUSY;
	}

	return ret;
}

static void DWC_ETH_QOS_free_irq(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	if (pdata->irq_number != 0) {
		disable_irq_wake(dev->irq);
		NDBGPR_L1("DWC_ETH_QOS IRQ %d freed\n", pdata->irq_number);
		free_irq(pdata->irq_number, pdata);
		pdata->irq_number = 0;
	}
}

/*!
* \brief API to open a deivce for data transmission & reception.
*
* \details Opens the interface. The interface is opned whenever
* ifconfig activates it. The open method should register any
* system resource it needs like I/O ports, IRQ, DMA, etc,
* turn on the hardware, and perform any other setup your device requires.
*
* \param[in] dev - pointer to net_device structure
*
* \return integer
*
* \retval 0 on success & negative number on failure.
*/

static int DWC_ETH_QOS_open(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	int ret = Y_SUCCESS;
	struct desc_if_struct *desc_if = &pdata->desc_if;
	struct hw_if_struct *hw_if = &pdata->hw_if;

	DBGPR("-->DWC_ETH_QOS_open\n");
	if (DWC_ETH_QOS_request_irq(dev) != 0) {
		NMSGPR_ALERT("Unable to register IRQ %d\n", pdata->irq_number);
		ret = -EBUSY;
		goto err_irq_0;
	}

	ret = desc_if->alloc_buff_and_desc(pdata);
	if (ret < 0) {
		NMSGPR_ALERT(
		       "failed to allocate buffer/descriptor memory\n");
		ret = -ENOMEM;
		goto err_out_desc_buf_alloc_failed;
	}

	DWC_ETH_QOS_init_common(dev);

	DWC_ETH_QOS_napi_enable_mq(pdata);

	netif_tx_start_all_queues(dev);

	if (pdata->ipa_enabled) {
		/* Configure IPA Related Stuff */
		ret = DWC_ETH_QOS_ipa_ready(pdata);
		if (pdata->prv_ipa.ipa_ready) {
			NDBGPR_L1("%s:%d ipa ready\n", __func__, __LINE__);
			ret = DWC_ETH_QOS_ipa_offload_init(pdata);
			if (!ret) {
				NDBGPR_L1("IPA Offload Initialized Successfully \n");
				pdata->prv_ipa.ipa_offload_init = true;
			}
		}
		else {
			NMSGPR_INFO("%s:%d ipa not ready\n", __func__, __LINE__);
		}

		/* Configure IPA Related Stuff */
		if (pdata->prv_ipa.ipa_uc_ready) {
			NMSGPR_INFO("%s:%d ipa uC ready\n", __func__, __LINE__);
			ret = DWC_ETH_QOS_enable_ipa_offload(pdata);
			if (ret) {
				NMSGPR_ERR("%s:%d unable to enable ipa offload\n",
					   __func__, __LINE__);
				goto err_out_desc_buf_alloc_failed;
			}
		}
		else
			NMSGPR_INFO("%s:%d ipa uC not ready\n", __func__, __LINE__);
	}

#ifdef NTN_POLLING_METHOD
	polling_task_data = (void*)pdata;
	INIT_DELAYED_WORK(&task, (void *)polling_task);
	NMSGPR_INFO("Jiffies : %ld\n", usecs_to_jiffies(NTN_POLL_DELAY_US));
	schedule_delayed_work(&task, usecs_to_jiffies(NTN_POLL_DELAY_US));
#endif //NTN_POLLING_METHOD

	hw_if->ntn_wrap_ts_valid_window_config(pdata->ntn_timestamp_valid_window, pdata);

	/* Disable early transmit complete and underflow interrupt enable bits
	 * for performance */
	DMA_TXCHINTMASK_ETCEN_UdfWr(0, 0);
	DMA_TXCHINTMASK_UNFEN_UdfWr(0, 0);
	netif_carrier_on(pdata->dev);

	if (pdata->enable_phy && pdata->phydev) {
		pdata->phydev->attached_dev = dev;
		pdata->phydev->state = PHY_UP;
		genphy_read_status(pdata->phydev);

		if (pdata->phydev->link) {
			if (pdata->phydev->duplex)
				hw_if->set_full_duplex(pdata);
			else
				hw_if->set_half_duplex(pdata);

			switch (pdata->phydev->speed) {
			case SPEED_1000:
				hw_if->set_gmii_speed(pdata);
				hw_if->ntn_set_tx_clk_125MHz(pdata);
				break;
			case SPEED_100:
				hw_if->set_mii_speed_100(pdata);
				hw_if->ntn_set_tx_clk_25MHz(pdata);
				break;
			case SPEED_10:
				hw_if->set_mii_speed_10(pdata);
				hw_if->ntn_set_tx_clk_2_5MHz(pdata);
				break;
			}
		} else {
			NMSGPR_INFO("PHY link still is not up, resume it!!!!\r\n");
			genphy_resume(pdata->phydev);
		}
		schedule_delayed_work(&pdata->phy_dwork, usecs_to_jiffies(1000000));
	}

	return ret;

 err_out_desc_buf_alloc_failed:
	DWC_ETH_QOS_free_irq(dev);

 err_irq_0:
	DBGPR("<--DWC_ETH_QOS_open\n");
	return ret;
}

/*!
* \brief API to close a device.
*
* \details Stops the interface. The interface is stopped when it is brought
* down. This function should reverse operations performed at open time.
*
* \param[in] dev - pointer to net_device structure
*
* \return integer
*
* \retval 0 on success & negative number on failure.
*/

static int DWC_ETH_QOS_close(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &pdata->hw_if;
	struct desc_if_struct *desc_if = &pdata->desc_if;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_close\n");

	if (pdata->eee_enabled)
		del_timer_sync(&pdata->eee_ctrl_timer);

	if (pdata->enable_phy && pdata->phydev)
		genphy_suspend(pdata->phydev);

	netif_tx_disable(dev);
	DWC_ETH_QOS_all_ch_napi_disable(pdata);

	if (pdata->ipa_enabled) {
		ipa_uc_offload_dereg_rdyCB(IPA_UC_NTN);
		ret = DWC_ETH_QOS_disable_ipa_offload(pdata);
		if (ret)
			return ret;
	}

	/* issue software reset to device */
	hw_if->exit(pdata);
	desc_if->tx_free_mem(pdata);
	desc_if->rx_free_mem(pdata);
	DWC_ETH_QOS_free_irq(dev);

#ifdef NTN_POLLING_METHOD
	cancel_delayed_work_sync(&task);
#endif //NTN_POLLING_METHOD

	if (pdata->enable_phy && pdata->phydev)
		pdata->phydev->state = PHY_DOWN;

	DBGPR("<--DWC_ETH_QOS_close\n");

	return Y_SUCCESS;
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
static int DWC_ETH_QOS_prepare_mc_list(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	u32 mc_filter[DWC_ETH_QOS_HTR_CNT];
	struct netdev_hw_addr *ha = NULL;
	int crc32_val = 0;
	int ret = 0, i = 1;

	DBGPR_FILTER("-->DWC_ETH_QOS_prepare_mc_list\n");

	if (pdata->l2_filtering_mode) {
		DBGPR_FILTER("select HASH FILTERING for mc addresses: mc_count = %d\n",
				netdev_mc_count(dev));
		ret = 1;
		memset(mc_filter, 0, sizeof(mc_filter));

			netdev_for_each_mc_addr(ha, dev) {
				DBGPR_FILTER("mc addr[%d] = %#x:%#x:%#x:%#x:%#x:%#x\n",i++,
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
			hw_if->update_hash_table_reg(i, mc_filter[i], pdata);

	} else {
		DBGPR_FILTER("select PERFECT FILTERING for mc addresses, mc_count = %d, max_addr_reg_cnt = %d\n",
				netdev_mc_count(dev), pdata->max_addr_reg_cnt);

		netdev_for_each_mc_addr(ha, dev) {
			DBGPR_FILTER("mc addr[%d] = %#x:%#x:%#x:%#x:%#x:%#x\n", i,
					ha->addr[0], ha->addr[1], ha->addr[2],
					ha->addr[3], ha->addr[4], ha->addr[5]);
				hw_if->update_mac_addr3_31_low_high_reg(i, ha->addr, pdata);
			i++;
		}
	}

	DBGPR_FILTER("<--DWC_ETH_QOS_prepare_mc_list\n");

	return ret;
}

/*!
* \brief API to configure the unicast address in device.
*
* \details This function collects all the unicast addresses
* and updates the device.
*
* \param[in] dev - pointer to net_device structure.
*
* \retval 0 if perfect filtering is seleted  & 1 if hash
* filtering is seleted.
*/
static int DWC_ETH_QOS_prepare_uc_list(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	u32 uc_filter[DWC_ETH_QOS_HTR_CNT];
	struct netdev_hw_addr *ha = NULL;
	int crc32_val = 0;
	int ret = 0, i = 1;

	DBGPR_FILTER("-->DWC_ETH_QOS_prepare_uc_list\n");

	if (pdata->l2_filtering_mode) {
		DBGPR_FILTER("select HASH FILTERING for uc addresses: uc_count = %d\n",
				netdev_uc_count(dev));
		ret = 1;
		memset(uc_filter, 0, sizeof(uc_filter));

			netdev_for_each_uc_addr(ha, dev) {
				DBGPR_FILTER("uc addr[%d] = %#x:%#x:%#x:%#x:%#x:%#x\n",i++,
						ha->addr[0], ha->addr[1], ha->addr[2],
						ha->addr[3], ha->addr[4], ha->addr[5]);
				crc32_val =
					(bitrev32(~crc32_le(~0, ha->addr, 6)) >> 26);
				uc_filter[crc32_val >> 5] |= (1 << (crc32_val & 0x1F));
			}

		/* configure hash value of real/default interface also */
		DBGPR_FILTER("real/default dev_addr = %#x:%#x:%#x:%#x:%#x:%#x\n",
				dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
				dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

			crc32_val =
				(bitrev32(~crc32_le(~0, dev->dev_addr, 6)) >> 26);
			uc_filter[crc32_val >> 5] |= (1 << (crc32_val & 0x1F));

		for (i = 0; i < DWC_ETH_QOS_HTR_CNT; i++)
			hw_if->update_hash_table_reg(i, uc_filter[i], pdata);

	} else {
		DBGPR_FILTER("select PERFECT FILTERING for uc addresses: uc_count = %d\n",
				netdev_uc_count(dev));

		netdev_for_each_uc_addr(ha, dev) {
			DBGPR_FILTER("uc addr[%d] = %#x:%#x:%#x:%#x:%#x:%#x\n", i,
					ha->addr[0], ha->addr[1], ha->addr[2],
					ha->addr[3], ha->addr[4], ha->addr[5]);
				hw_if->update_mac_addr3_31_low_high_reg(i, ha->addr, pdata);
			i++;
		}
	}

	DBGPR_FILTER("<--DWC_ETH_QOS_prepare_uc_list\n");

	return ret;
}

/*!
* \brief API to set the device receive mode
*
* \details The set_multicast_list function is called when the multicast list
* for the device changes and when the flags change.
*
* \param[in] dev - pointer to net_device structure.
*
* \return void
*/
static void DWC_ETH_QOS_set_rx_mode(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned long flags;
	unsigned char pr_mode = 0;
	unsigned char huc_mode = 0;
	unsigned char hmc_mode = 0;
	unsigned char pm_mode = 0;
	unsigned char hpf_mode = 0;
	int mode, i;

	DBGPR_FILTER("-->DWC_ETH_QOS_set_rx_mode\n");

	spin_lock_irqsave(&pdata->lock, flags);

	if (dev->flags & IFF_PROMISC) {
		DBGPR_FILTER("PROMISCUOUS MODE (Accept all packets irrespective of DA)\n");
		pr_mode = 1;
	} else if ((dev->flags & IFF_ALLMULTI) ||
			(netdev_mc_count(dev) > (pdata->max_hash_table_size))) {
		DBGPR_FILTER("pass all multicast pkt\n");
		pm_mode = 1;
		if (pdata->max_hash_table_size) {
			for (i = 0; i < DWC_ETH_QOS_HTR_CNT; i++)
				hw_if->update_hash_table_reg(i, 0xffffffff, pdata);
		}
	} else if (!netdev_mc_empty(dev)) {
		DBGPR_FILTER("pass list of multicast pkt\n");
		if ((netdev_mc_count(dev) > (pdata->max_addr_reg_cnt - 1)) &&
			(!pdata->max_hash_table_size)) {
			/* switch to PROMISCUOUS mode */
			pr_mode = 1;
		} else {
			mode = DWC_ETH_QOS_prepare_mc_list(dev);
			if (mode) {
				/* Hash filtering for multicast */
				hmc_mode = 1;
			} else {
				/* Perfect filtering for multicast */
				hmc_mode = 0;
				hpf_mode = 1;
			}
		}
	}

	/* Handle multiple unicast addresses */
	if ((netdev_uc_count(dev) > (pdata->max_addr_reg_cnt - 1)) &&
			(!pdata->max_hash_table_size)) {
		/* switch to PROMISCUOUS mode */
		printk("switch to promiscuous for unicast\n");
		pr_mode = 1;
	} else if (!netdev_uc_empty(dev)) {
		mode = DWC_ETH_QOS_prepare_uc_list(dev);
		if (mode) {
			/* Hash filtering for unicast */
			huc_mode = 1;
		} else {
			/* Perfect filtering for unicast */
			huc_mode = 0;
			hpf_mode = 1;
		}
	}

	hw_if->config_mac_pkt_filter_reg(pr_mode, huc_mode,
		hmc_mode, pm_mode, hpf_mode, pdata);

	spin_unlock_irqrestore(&pdata->lock, flags);

	DBGPR("<--DWC_ETH_QOS_set_rx_mode\n");
}

/*!
* \brief API to calculate number of descriptor.
*
* \details This function is invoked by start_xmit function. This function
* calculates number of transmit descriptor required for a given transfer.
*
* \param[in] pdata - pointer to private data structure
* \param[in] skb - pointer to sk_buff structure
* \param[in] chInx - Queue number.
*
* \return integer
*
* \retval number of descriptor required.
*/

UINT DWC_ETH_QOS_get_total_desc_cnt(struct DWC_ETH_QOS_prv_data *pdata,
		struct sk_buff *skb, UINT chInx)
{
	UINT count = 0, size = 0;
	INT length = 0;
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	struct hw_if_struct *hw_if = &pdata->hw_if;
	struct s_tx_pkt_features *tx_pkt_features = GET_TX_PKT_FEATURES_PTR;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data =
	    GET_TX_WRAPPER_DESC(chInx);
#endif

	/* SG fragment count */
	count += skb_shinfo(skb)->nr_frags;

	/* descriptors required based on data limit per descriptor */
	length = (skb->len - skb->data_len);
	while (length) {
		size = min(length, DWC_ETH_QOS_MAX_DATA_PER_TXD);
		count++;
		length = length - size;
	}

	/* we need one context descriptor to carry tso details */
	if (skb_shinfo(skb)->gso_size != 0)
		count++;

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	desc_data->vlan_tag_present = 0;
	if (vlan_tx_tag_present(skb)) {
		USHORT vlan_tag = vlan_tx_tag_get(skb);
		desc_data->vlan_tag_present = 1;
		DBGPR_VLAN("%s:VLAN: Tag seen on TX Packet\n",__func__);
		if (vlan_tag != desc_data->vlan_tag_id ||
				desc_data->context_setup == 1) {
			desc_data->vlan_tag_id = vlan_tag;
			if (Y_TRUE == desc_data->tx_vlan_tag_via_reg) {
				DBGPR_VLAN("%s:VLAN: enable_vlan_reg_control\n",__func__);
				hw_if->enable_vlan_reg_control(desc_data, pdata);
			} else {
				DBGPR_VLAN("%s:VLAN: enable_vlan_desc_control\n",__func__);
				hw_if->enable_vlan_desc_control(pdata);
				TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Wr
				    (tx_pkt_features->pkt_attributes, 1);
				TX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Wr
					(tx_pkt_features->vlan_tag, vlan_tag);
				/* we need one context descriptor to carry vlan tag info */
				count++;
			}
		}
		pdata->xstats.tx_vlan_pkt_n++;
	}
#endif
#ifdef DWC_ETH_QOS_ENABLE_DVLAN
	if (pdata->via_reg_or_desc == DWC_ETH_QOS_VIA_DESC) {
		/* we need one context descriptor to carry vlan tag info */
		count++;
	}
#endif /* End of DWC_ETH_QOS_ENABLE_DVLAN */

	return count;
}


/*!
* \brief API to transmit the packets
*
* \details The start_xmit function initiates the transmission of a packet.
* The full packet (protocol headers and all) is contained in a socket buffer
* (sk_buff) structure.
*
* \param[in] skb - pointer to sk_buff structure
* \param[in] dev - pointer to net_device structure
*
* \return integer
*
* \retval 0
*/

static int DWC_ETH_QOS_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	UINT chInx = skb_get_queue_mapping(skb);
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data;
	struct s_tx_pkt_features *tx_pkt_features = GET_TX_PKT_FEATURES_PTR;
	unsigned long flags;
	unsigned int desc_count = 0;
	unsigned int count = 0;
	struct hw_if_struct *hw_if = &pdata->hw_if;
	struct desc_if_struct *desc_if = &pdata->desc_if;
	INT retval = NETDEV_TX_OK;
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	UINT varvlan_pkt;
#endif
	int tso;

	if (ip_hdr(skb)->protocol == IPPROTO_ICMP)
		skb_orphan(skb);
	spin_lock_irqsave(&pdata->tx_lock, flags);
	/* Fetch TX Channel no */
	desc_data = GET_TX_WRAPPER_DESC(chInx);

	DBGPR("-->DWC_ETH_QOS_start_xmit: skb->len = %d, chInx = %u\n",
		skb->len, chInx);

	if (skb->len <= 0)
	{
		dev_kfree_skb_any(skb);
		NMSGPR_ERR( "%s : Empty skb received from stack\n",
			dev->name);
		goto tx_netdev_return;
	}

#ifdef NTN_DRV_TEST_LOOPBACK
	NMSGPR_INFO("TX Channel = %d, skb->len = %d , protocol = %#x \n", chInx, skb->len, eth_type);
#endif
	if ((skb_shinfo(skb)->gso_size == 0) &&
		(skb->len > DWC_ETH_QOS_MAX_SUPPORTED_MTU)) {
		NMSGPR_ERR( "%s : big packet = %d\n", dev->name,
			(u16)skb->len);
		dev_kfree_skb_any(skb);
		dev->stats.tx_dropped++;
		goto tx_netdev_return;
	}

	if ((pdata->eee_enabled) && (pdata->tx_path_in_lpi_mode) &&
		(!pdata->use_lpi_tx_automate)){
		DBGPR( "DWC_ETH_QOS_disable_eee_mode for chInx = %d\n", chInx);
		DWC_ETH_QOS_disable_eee_mode(pdata);
	}

	memset(&pdata->tx_pkt_features, 0, sizeof(pdata->tx_pkt_features));

	/* check total number of desc required for current xfer */
	desc_count = DWC_ETH_QOS_get_total_desc_cnt(pdata, skb, chInx);
	if (desc_data->free_desc_cnt < desc_count) {

		if( (chInx != NTN_TX_PKT_AVB_CLASS_A) && (chInx != NTN_TX_PKT_AVB_CLASS_B) ){
		desc_data->queue_stopped = 1;
			netif_stop_subqueue(dev, chInx);
		}
		DBGPR("stopped TX queue(%d) since there are no sufficient "
			"descriptor available for the current transfer\n",
			chInx);
		retval = NETDEV_TX_BUSY;
		goto tx_netdev_return;
	}
#if 0
if( (chInx == NTN_TX_PKT_AVB_CLASS_A) || (chInx == NTN_TX_PKT_AVB_CLASS_B) )
{
	struct timespec now;
	unsigned long long now_ns;
	getnstimeofday(&now);
	now_ns = now.tv_sec;
	now_ns = (now_ns*1000000000)+now.tv_nsec;
	NMSGPR_INFO("Sub_first = %llu\n", now_ns);
}
#endif

	/* check for hw tstamping */
	if (pdata->hw_feat.tsstssel && pdata->hwts_tx_en) {
		if(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) {
			/* declare that device is doing timestamping */
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
			TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_Mlf_Wr(tx_pkt_features->pkt_attributes, 1);
			DBGPR_PTP("Got PTP pkt to transmit [chInx = %d, cur_tx = %d]\n",
				chInx, desc_data->cur_tx);
		}
	}

	tso = desc_if->handle_tso(dev, skb);
	if (tso < 0) {
		NMSGPR_ALERT( "Unable to handle TSO\n");
		dev_kfree_skb_any(skb);
		retval = NETDEV_TX_OK;
		goto tx_netdev_return;
	}
	if (tso) {
		NMSGPR_INFO("TSO = %d, ChInx = %d\n", tso, chInx);
		pdata->xstats.tx_tso_pkt_n++;
		TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_Mlf_Wr(tx_pkt_features->pkt_attributes, 1);
	} else if (skb->ip_summed == CHECKSUM_PARTIAL) {
		TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_Mlf_Wr(tx_pkt_features->pkt_attributes, 1);
	}

	count = desc_if->map_tx_skb(dev, skb);
	if (count == 0) {
		dev_kfree_skb_any(skb);
		retval = NETDEV_TX_OK;
		goto tx_netdev_return;
	}

	desc_data->packet_count = count;

	if (tso && (desc_data->default_mss != tx_pkt_features->mss))
		count++;

	dev->trans_start = jiffies;

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Rd
		(tx_pkt_features->pkt_attributes, varvlan_pkt);
	if (varvlan_pkt == 0x1) {
		count++;
		DBGPR_VLAN("%s:VLAN: pkt_attributes.vlan\n",__func__);
	}
#endif
#ifdef DWC_ETH_QOS_ENABLE_DVLAN
	NMSGPR_ALERT( "Defined DWC_ETH_QOS_ENABLE_DVLAN\n");
	if (pdata->via_reg_or_desc == DWC_ETH_QOS_VIA_DESC) {
		count++;
	}
#endif /* End of DWC_ETH_QOS_ENABLE_DVLAN */

	desc_data->free_desc_cnt -= count;
	desc_data->tx_pkt_queued += count;

#ifdef DWC_ETH_QOS_ENABLE_TX_PKT_DUMP
	print_pkt(skb, skb->len, 1, (desc_data->cur_tx - 1));
#endif

	/* fallback to software time stamping if core doesn't
	 * support hardware time stamping */
	if ((pdata->hw_feat.tsstssel == 0) || (pdata->hwts_tx_en == 0))
		skb_tx_timestamp(skb);

	/* Extract the launch time from the packet if it is AVB packet */
	if( (chInx == NTN_TX_PKT_AVB_CLASS_A) || (chInx == NTN_TX_PKT_AVB_CLASS_B) )
	{
		tx_pkt_features->launch_time = skb->data[30];
		tx_pkt_features->launch_time = (tx_pkt_features->launch_time<<8) | skb->data[31];
		tx_pkt_features->launch_time = (tx_pkt_features->launch_time<<8) | skb->data[32];
		tx_pkt_features->launch_time = (tx_pkt_features->launch_time<<8) | skb->data[33];
	}

	/* configure required descriptor fields for transmission */
	hw_if->pre_xmit(pdata, chInx, ETHERTYPE_FROM_PACKET(skb->data));

tx_netdev_return:
	spin_unlock_irqrestore(&pdata->tx_lock, flags);

	DBGPR("<--DWC_ETH_QOS_start_xmit\n");

	return retval;
}

static void DWC_ETH_QOS_print_rx_tstamp_info(struct s_RX_NORMAL_DESC *rxdesc,
	unsigned int chInx)
{
	u32 ptp_status = 0;
	u32 pkt_type = 0;
	char *tstamp_dropped = NULL;
	char *tstamp_available = NULL;
	char *ptp_version = NULL;
	char *ptp_pkt_type = NULL;
	char *ptp_msg_type = NULL;

	DBGPR_PTP("-->DWC_ETH_QOS_print_rx_tstamp_info\n");

	/* status in RDES1 is not valid */
	if (!(rxdesc->RDES3 & DWC_ETH_QOS_RDESC3_RS1V))
		return;

	ptp_status = rxdesc->RDES1;
	tstamp_dropped = ((ptp_status & 0x8000) ? "YES" : "NO");
	tstamp_available = ((ptp_status & 0x4000) ? "YES" : "NO");
	ptp_version = ((ptp_status & 0x2000) ? "v2 (1588-2008)" : "v1 (1588-2002)");
	ptp_pkt_type = ((ptp_status & 0x1000) ? "ptp over Eth" : "ptp over IPv4/6");

	pkt_type = ((ptp_status & 0xF00) > 8);
	switch (pkt_type) {
	case 0:
		ptp_msg_type = "NO PTP msg received";
		break;
	case 1:
		ptp_msg_type = "SYNC";
		break;
	case 2:
		ptp_msg_type = "Follow_Up";
		break;
	case 3:
		ptp_msg_type = "Delay_Req";
		break;
	case 4:
		ptp_msg_type = "Delay_Resp";
		break;
	case 5:
		ptp_msg_type = "Pdelay_Req";
		break;
	case 6:
		ptp_msg_type = "Pdelay_Resp";
		break;
	case 7:
		ptp_msg_type = "Pdelay_Resp_Follow_up";
		break;
	case 8:
		ptp_msg_type = "Announce";
		break;
	case 9:
		ptp_msg_type = "Management";
		break;
	case 10:
		ptp_msg_type = "Signaling";
		break;
	case 11:
	case 12:
	case 13:
	case 14:
		ptp_msg_type = "Reserved";
		break;
	case 15:
		ptp_msg_type = "PTP pkr with Reserved Msg Type";
		break;
	}

	DBGPR_PTP("Rx timestamp detail for queue %d\n"
			"tstamp dropped    = %s\n"
			"tstamp available  = %s\n"
			"PTP version       = %s\n"
			"PTP Pkt Type      = %s\n"
			"PTP Msg Type      = %s\n",
			chInx, tstamp_dropped, tstamp_available,
			ptp_version, ptp_pkt_type, ptp_msg_type);

	DBGPR_PTP("<--DWC_ETH_QOS_print_rx_tstamp_info\n");
}


/*!
* \brief API to get rx time stamp value.
*
* \details This function will read received packet's timestamp from
* the descriptor and pass it to stack and also perform some sanity checks.
*
* \param[in] pdata - pointer to private data structure.
* \param[in] skb - pointer to sk_buff structure.
* \param[in] desc_data - pointer to wrapper receive descriptor structure.
* \param[in] chInx - Queue/Channel number.
*
* \return integer
*
* \retval 0 if no context descriptor
* \retval 1 if timestamp is valid
* \retval 2 if time stamp is corrupted
*/

static unsigned char DWC_ETH_QOS_get_rx_hwtstamp(
	struct DWC_ETH_QOS_prv_data *pdata,
	struct sk_buff *skb,
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data,
	unsigned int chInx)
{
	struct s_RX_NORMAL_DESC *rx_normal_desc =
		GET_RX_DESC_PTR(chInx, desc_data->cur_rx);
	struct s_RX_CONTEXT_DESC *rx_context_desc = NULL;
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct skb_shared_hwtstamps *shhwtstamp = NULL;
	u64 ns;
	int retry, ret;
#ifdef NTN_RX_DATA_BUF_IN_SRAM
	struct DWC_ETH_QOS_rx_buffer *buffer;
#endif


	DBGPR_PTP("-->DWC_ETH_QOS_get_rx_hwtstamp\n");

	DWC_ETH_QOS_print_rx_tstamp_info(rx_normal_desc, chInx);

	desc_data->dirty_rx++;
	INCR_RX_DESC_INDEX(desc_data->cur_rx, 1, pdata->rx_dma_ch[chInx].desc_cnt);
	rx_context_desc = (void *)GET_RX_DESC_PTR(chInx, desc_data->cur_rx);

	DBGPR_PTP("\nRX_CONTEX_DESC[%d %4p %d RECEIVED FROM DEVICE]"\
			" = %#x:%#x:%#x:%#x",
			chInx, rx_context_desc, desc_data->cur_rx, rx_context_desc->RDES0,
			rx_context_desc->RDES1,
			rx_context_desc->RDES2, rx_context_desc->RDES3);

	/* check rx tsatmp */
	for (retry = 0; retry < 10; retry++) {
		ret = hw_if->get_rx_tstamp_status(rx_context_desc);
		if (ret == 1) {
			/* time stamp is valid */
			break;
		} else if (ret == 0) {
			NMSGPR_ALERT( "Device has not yet updated the context "
				"desc to hold Rx time stamp(retry = %d)\n", retry);
		} else {
			NMSGPR_ALERT( "Error: Rx time stamp is corrupted(retry = %d)\n", retry);
			return 2;
		}
	}

	if (retry == 10) {
			NMSGPR_ALERT( "Device has not yet updated the context "
				"desc to hold Rx time stamp(retry = %d)\n", retry);
			desc_data->dirty_rx--;
			DECR_RX_DESC_INDEX(desc_data->cur_rx, pdata->rx_dma_ch[chInx].desc_cnt);
			return 0;
	}

	/* Free associated SRAM DMA buffer, as context descriptor is marked as dirty */
#ifdef NTN_RX_DATA_BUF_IN_SRAM
	buffer = GET_RX_BUF_PTR(chInx, desc_data->cur_rx);

	dma_unmap_single(&pdata->pdev->dev, buffer->dma_iommu, pdata->rx_buffer_len, DMA_FROM_DEVICE);
	dma_pool_free(pdata->rx_mem_pool, buffer->skb->data, buffer->dma);
	buffer->dma_iommu = 0;
    buffer->skb->data = buffer->skb_temp_data_ptr;
	buffer->dma = 0;

#endif

	pdata->xstats.rx_timestamp_captured_n++;
	/* get valid tstamp */
	ns = hw_if->get_rx_tstamp(rx_context_desc);

	shhwtstamp = skb_hwtstamps(skb);
	memset(shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp->hwtstamp = ns_to_ktime(ns);

	DBGPR_PTP("<--DWC_ETH_QOS_get_rx_hwtstamp\n");

	return 1;
}


/*!
* \brief API to get tx time stamp value.
*
* \details This function will read timestamp from the descriptor
* and pass it to stack and also perform some sanity checks.
*
* \param[in] pdata - pointer to private data structure.
* \param[in] txdesc - pointer to transmit descriptor structure.
* \param[in] skb - pointer to sk_buff structure.
*
* \return integer
*
* \retval 1 if time stamp is taken
* \retval 0 if time stamp in not taken/valid
*/

static unsigned int DWC_ETH_QOS_get_tx_hwtstamp(
	struct DWC_ETH_QOS_prv_data *pdata,
	struct s_TX_NORMAL_DESC *txdesc,
	struct sk_buff *skb)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct skb_shared_hwtstamps shhwtstamp;
	u64 ns;

	DBGPR_PTP("-->DWC_ETH_QOS_get_tx_hwtstamp\n");

	if (hw_if->drop_tx_status_enabled(pdata) == 0) {
		/* check tx tstamp status */
		if (!hw_if->get_tx_tstamp_status(txdesc)) {
			NMSGPR_ALERT( "tx timestamp is not captured for this packet\n");
			return 0;
		}

		/* get the valid tstamp */
		ns = hw_if->get_tx_tstamp(txdesc);
	} else {
		/* drop tx status mode is enabled, hence read time
		 * stamp from register instead of descriptor */

		/* check tx tstamp status */
		if (!hw_if->get_tx_tstamp_status_via_reg(pdata)) {
			NMSGPR_ALERT( "tx timestamp is not captured for this packet\n");
			return 0;
		}

		/* get the valid tstamp */
		ns = hw_if->get_tx_tstamp_via_reg(pdata);
	}

	pdata->xstats.tx_timestamp_captured_n++;
	memset(&shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp.hwtstamp = ns_to_ktime(ns);
	/* pass tstamp to stack */
	skb_tstamp_tx(skb, &shhwtstamp);

	DBGPR_PTP("<--DWC_ETH_QOS_get_tx_hwtstamp\n");

	return 1;
}


/*!
* \brief API to update the tx status.
*
* \details This function is called in isr handler once after getting
* transmit complete interrupt to update the transmited packet status
* and it does some house keeping work like updating the
* private data structure variables.
*
* \param[in] dev - pointer to net_device structure
* \param[in] pdata - pointer to private data structure.
*
* \return void
*/

static void DWC_ETH_QOS_tx_interrupt(struct net_device *dev,
				     struct DWC_ETH_QOS_prv_data *pdata,
				     UINT chInx)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data =
	    GET_TX_WRAPPER_DESC(chInx);
	struct s_TX_NORMAL_DESC *txptr = NULL;
	struct DWC_ETH_QOS_tx_buffer *buffer = NULL;
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct desc_if_struct *desc_if = &(pdata->desc_if);
#ifndef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
#ifdef DWC_ETH_QOS_ENABLE_ERROR_COUNTERS
	int err_incremented;
#endif
#endif
	unsigned int tstamp_taken = 0;
	unsigned long flags;

	DBGPR("-->DWC_ETH_QOS_tx_interrupt: desc_data->tx_pkt_queued = %d"
		" dirty_tx = %d, chInx = %u\n",
		desc_data->tx_pkt_queued, desc_data->dirty_tx, chInx);

	if (pdata->ipa_enabled && chInx == NTN_TX_DMA_CH_2) {
		NMSGPR_INFO("TX status interrupts are handled by IPA uc for TXCH2 \
					skip for the host \n");
		return;
	}

	spin_lock_irqsave(&pdata->tx_lock, flags);

	pdata->xstats.tx_clean_n[chInx]++;
	while (desc_data->tx_pkt_queued > 0) {
		txptr = GET_TX_DESC_PTR(chInx, desc_data->dirty_tx);
		buffer = GET_TX_BUF_PTR(chInx, desc_data->dirty_tx);
		tstamp_taken = 0;

		if (!hw_if->tx_complete(txptr))
			break;

#ifdef DWC_ETH_QOS_ENABLE_TX_DESC_DUMP
		dump_tx_desc(pdata, desc_data->dirty_tx, desc_data->dirty_tx,
			     0, chInx);
#endif
#ifdef NTN_AVB_LAUNCHTIME_TS_CAPTURE_DUMP
        /* Follwoing code caluclates and converts captured timestamp into 32 bit resolution for easy comparision. It is only for debugging. */
		if( (chInx == NTN_TX_PKT_AVB_CLASS_A) || (chInx == NTN_TX_PKT_AVB_CLASS_B) )
        {
		if (hw_if->get_tx_tstamp_status(txptr)) {
            u64 data = 0;
			int ipg;
#ifdef SYSTEM_IS_64
			static u64 pre_ts = 0;
			data = hw_if->get_tx_tstamp(txptr);
			ipg = ((data - pre_ts) / 1000);
			if( (ipg < 121) || (ipg > 128))
				NDBGPR_L1(" 1722 Pkt sent TS : %llu,    IPG = %d \n", data, ipg);
			pre_ts = data;
#else
			static int pre_ts = 0;
			data = hw_if->get_tx_tstamp(txptr);
			ipg = (((u32)data - pre_ts) / 1000);
			if( (ipg < 121) || (ipg > 128))
				NDBGPR_L1(" 1722 Pkt sent TS : %llu,    IPG = %d \n", data, ipg);
			pre_ts = (u32)data;
#endif
			}
        }
#endif


#ifndef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
		/* update the tx error if any by looking at last segment
		 * for NORMAL descriptors
		 * */
	NDBGPR_L2("DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT\n");
		if ((hw_if->get_tx_desc_ls(txptr)) && !(hw_if->get_tx_desc_ctxt(txptr))) {
			/* check whether skb support hw tstamp */
			if ((pdata->hw_feat.tsstssel) &&
				(skb_shinfo(buffer->skb)->tx_flags & SKBTX_IN_PROGRESS)) {
				tstamp_taken = DWC_ETH_QOS_get_tx_hwtstamp(pdata,
					txptr, buffer->skb);
				if (tstamp_taken) {
					//dump_tx_desc(pdata, desc_data->dirty_tx, desc_data->dirty_tx,
					//		0, chInx);
					DBGPR_PTP("passed tx timestamp to stack[chInx = %d, dirty_tx = %d]\n",
						chInx, desc_data->dirty_tx);
				}
			}
#ifdef DWC_ETH_QOS_ENABLE_ERROR_COUNTERS
			err_incremented = 0;
			if (hw_if->tx_window_error) {
				if (hw_if->tx_window_error(txptr)) {
					err_incremented = 1;
					dev->stats.tx_window_errors++;
				}
			}
			if (hw_if->tx_aborted_error) {
				if (hw_if->tx_aborted_error(txptr)) {
					err_incremented = 1;
					dev->stats.tx_aborted_errors++;
					if (hw_if->tx_handle_aborted_error)
						hw_if->tx_handle_aborted_error(txptr);
				}
			}
			if (hw_if->tx_carrier_lost_error) {
				if (hw_if->tx_carrier_lost_error(txptr)) {
					err_incremented = 1;
					dev->stats.tx_carrier_errors++;
				}
			}
			if (hw_if->tx_fifo_underrun) {
				if (hw_if->tx_fifo_underrun(txptr)) {
					err_incremented = 1;
					dev->stats.tx_fifo_errors++;
					if (hw_if->tx_update_fifo_threshold)
						hw_if->tx_update_fifo_threshold(txptr);
				}
			}
			if (hw_if->tx_get_collision_count)
				dev->stats.collisions +=
				    hw_if->tx_get_collision_count(txptr);

			if (err_incremented == 1)
				dev->stats.tx_errors++;
#endif

			pdata->xstats.q_tx_pkt_n[chInx]++;
			pdata->xstats.tx_pkt_n++;
			dev->stats.tx_packets++;
		}
#else
	NDBGPR_L2("DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT is not selected\n");
		if ((hw_if->get_tx_desc_ls(txptr)) && !(hw_if->get_tx_desc_ctxt(txptr))) {
			/* check whether skb support hw tstamp */
			if ((pdata->hw_feat.tsstssel) &&
				(skb_shinfo(buffer->skb)->tx_flags & SKBTX_IN_PROGRESS)) {
				tstamp_taken = DWC_ETH_QOS_get_tx_hwtstamp(pdata,
					txptr, buffer->skb);
				if (tstamp_taken) {
					dump_tx_desc(pdata, desc_data->dirty_tx, desc_data->dirty_tx,
							0, chInx);
					DBGPR_PTP("passed tx timestamp to stack[chInx = %d, dirty_tx = %d]\n",
						chInx, desc_data->dirty_tx);
				}
			}
		}
#endif

		if (hw_if->get_tx_desc_ctxt(txptr)) {
			NMSGPR_ERR("TX Context descriptor is not handled, Check if TX path has any issue\n");
		}

		dev->stats.tx_bytes += buffer->len;
		dev->stats.tx_bytes += buffer->len2;
		desc_if->unmap_tx_skb(pdata, buffer);

		/* reset the descriptor so that driver/host can reuse it */
		hw_if->tx_desc_reset(desc_data->dirty_tx, pdata, chInx);

		INCR_TX_DESC_INDEX(desc_data->dirty_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		desc_data->free_desc_cnt++;
		desc_data->tx_pkt_queued--;
	}

	if ((desc_data->queue_stopped == 1) && (desc_data->free_desc_cnt > 0)) {
		desc_data->queue_stopped = 0;

		if( (chInx != NTN_TX_PKT_AVB_CLASS_A) && (chInx != NTN_TX_PKT_AVB_CLASS_B) )
		netif_wake_subqueue(dev, chInx);
	}
#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
	/* DMA has finished Transmitting data to MAC Tx-Fifo */
	MAC_MCR_TE_UdfWr(1);
#endif

	if ((pdata->eee_enabled) && (!pdata->tx_path_in_lpi_mode) &&
		(!pdata->use_lpi_tx_automate)) {
		DWC_ETH_QOS_enable_eee_mode(pdata);
		mod_timer(&pdata->eee_ctrl_timer,
			DWC_ETH_QOS_LPI_TIMER(DWC_ETH_QOS_DEFAULT_LPI_TIMER));
	}

	spin_unlock_irqrestore(&pdata->tx_lock, flags);

	DBGPR("<--DWC_ETH_QOS_tx_interrupt: desc_data->tx_pkt_queued = %d\n",
	      desc_data->tx_pkt_queued);
}

#ifdef YDEBUG_FILTER
static void DWC_ETH_QOS_check_rx_filter_status(struct s_RX_NORMAL_DESC *RX_NORMAL_DESC)
{
	u32 rdes2 = RX_NORMAL_DESC->RDES2;
	u32 rdes3 = RX_NORMAL_DESC->RDES3;

	/* Receive Status RDES2 Valid ? */
	if ((rdes3 & 0x8000000) == 0x8000000) {
		if ((rdes2 & 0x400) == 0x400)
			NMSGPR_ALERT( "ARP pkt received\n");
		if ((rdes2 & 0x800) == 0x800)
			NMSGPR_ALERT( "ARP reply not generated\n");
		if ((rdes2 & 0x8000) == 0x8000)
			NMSGPR_ALERT( "VLAN pkt passed VLAN filter\n");
		if ((rdes2 & 0x10000) == 0x10000)
			NMSGPR_ALERT( "SA Address filter fail\n");
		if ((rdes2 & 0x20000) == 0x20000)
			NMSGPR_ALERT( "DA Addess filter fail\n");
		if ((rdes2 & 0x40000) == 0x40000)
			NMSGPR_ALERT( "pkt passed the HASH filter in MAC and HASH value = %#x\n",
					(rdes2 >> 19) & 0xff);
		if ((rdes2 & 0x8000000) == 0x8000000)
			NMSGPR_ALERT( "L3 filter(%d) Match\n", ((rdes2 >> 29) & 0x7));
		if ((rdes2 & 0x10000000) == 0x10000000)
			NMSGPR_ALERT( "L4 filter(%d) Match\n", ((rdes2 >> 29) & 0x7));
	}
}
#endif /* YDEBUG_FILTER */


/* pass skb to upper layer */
static void DWC_ETH_QOS_receive_skb(struct DWC_ETH_QOS_prv_data *pdata,
				    struct net_device *dev, struct sk_buff *skb,
				    UINT chInx)
{
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch = GET_RX_DMA_CH_PTR(chInx);

	skb_record_rx_queue(skb, chInx);
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	if (dev->features & NETIF_F_GRO) {
		napi_gro_receive(&rx_dma_ch->napi, skb);
	} else if ((dev->features & NETIF_F_LRO) &&
		(skb->ip_summed == CHECKSUM_UNNECESSARY)) {
		NMSGPR_ALERT( "INET_LRO not enabled in kernel : %s\n", __func__);
		/*lro_receive_skb(&rx_dma_ch->lro_mgr, skb, (void *)pdata);*/
		rx_dma_ch->lro_flush_needed = 1;
	} else {
		netif_receive_skb(skb);
	}
}

/* Receive Checksum Offload configuration */
static inline void DWC_ETH_QOS_config_rx_csum(struct DWC_ETH_QOS_prv_data *pdata,
		struct sk_buff *skb,
		struct s_RX_NORMAL_DESC *rx_normal_desc)
{
	UINT varRDES1;

	skb->ip_summed = CHECKSUM_NONE;
	if ((pdata->dev_state & NETIF_F_RXCSUM) == NETIF_F_RXCSUM) {
		/* Receive Status RDES1 Valid ? */
		if ((rx_normal_desc->RDES3 & DWC_ETH_QOS_RDESC3_RS1V)) {
			/* check(RDES1.IPCE bit) whether device has done csum correctly or not */
			RX_NORMAL_DESC_RDES1_Ml_Rd(rx_normal_desc->RDES1, varRDES1);
			if ((varRDES1 & 0xC8) == 0x0)
				skb->ip_summed = CHECKSUM_UNNECESSARY;	/* csum done by device */
		}
	}
}

static inline void DWC_ETH_QOS_get_rx_vlan(struct DWC_ETH_QOS_prv_data *pdata,
			struct sk_buff *skb,
			struct s_RX_NORMAL_DESC *rx_normal_desc)
{
	USHORT vlan_tag = 0;
	ULONG vlan_tag_strip = 0;

	if ((pdata->dev_state & NETIF_F_HW_VLAN_CTAG_RX) == NETIF_F_HW_VLAN_CTAG_RX) {
		/* Receive Status RDES0 Valid ? */
		if ((rx_normal_desc->RDES3 & DWC_ETH_QOS_RDESC3_RS0V)) {
			/* device received frame with VLAN Tag or
			 * double VLAN Tag ? */
			DBGPR_VLAN("%s:VLAN: RDES3.DWC_ETH_QOS_RDESC3_RS0V\n",__func__);
			if (((rx_normal_desc->RDES3 & DWC_ETH_QOS_RDESC3_LT) == 0x40000)
				|| ((rx_normal_desc->RDES3 & DWC_ETH_QOS_RDESC3_LT) == 0x50000)) {
                               /* read the VLAN tag stripping register */
                                MAC_VLANTR_EVLS_UdfRd(vlan_tag_strip);
                                if ( vlan_tag_strip )
                                {
				    vlan_tag = rx_normal_desc->RDES0 & 0xffff;
				    /* insert VLAN tag into skb */
				    __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vlan_tag);
                                }
				pdata->xstats.rx_vlan_pkt_n++;
				DBGPR_VLAN("%s:VLAN: RDES3.DWC_ETH_QOS_RDESC3_LT, TAG=%x\n",__func__,vlan_tag);
			}
		}
	}
}

/* This api check for payload type and returns
 * 1 if payload load is TCP else returns 0;
 * */
static int DWC_ETH_QOS_check_for_tcp_payload(struct s_RX_NORMAL_DESC *rxdesc)
{
		u32 pt_type = 0;
		int ret = 0;

		if (rxdesc->RDES3 & DWC_ETH_QOS_RDESC3_RS1V) {
				pt_type = rxdesc->RDES1 & DWC_ETH_QOS_RDESC1_PT;
				if (pt_type == DWC_ETH_QOS_RDESC1_PT_TCP)
						ret = 1;
		}

		return ret;
}

/*!
* \brief API to pass the Rx packets to stack if default mode
* is enabled.
*
* \details This function is invoked by main NAPI function in default
* Rx mode(non jumbo and non split header). This function checks the
* device descriptor for the packets and passes it to stack if any packtes
* are received by device.
*
* \param[in] pdata - pointer to private data structure.
* \param[in] quota - maximum no. of packets that we are allowed to pass
* to into the kernel.
* \param[in] chInx - DMA channel/queue no. to be checked for packet.
*
* \return integer
*
* \retval number of packets received.
*/

static int DWC_ETH_QOS_clean_rx_irq(struct DWC_ETH_QOS_prv_data *pdata,
				    int quota,
				    UINT chInx)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data =
	    GET_RX_WRAPPER_DESC(chInx);
	struct net_device *dev = pdata->dev;
	struct desc_if_struct *desc_if = &pdata->desc_if;
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct sk_buff *skb = NULL;
	int received = 0;
	struct DWC_ETH_QOS_rx_buffer *buffer = NULL;
	struct s_RX_NORMAL_DESC *RX_NORMAL_DESC = NULL;
	UINT pkt_len;
	int ret;

	DBGPR("-->DWC_ETH_QOS_clean_rx_irq: chInx = %u, quota = %d\n",
		chInx, quota);

	while (received < quota) {
		buffer = GET_RX_BUF_PTR(chInx, desc_data->cur_rx);
		RX_NORMAL_DESC = GET_RX_DESC_PTR(chInx, desc_data->cur_rx);

		/* check for data availability */
		if (!(RX_NORMAL_DESC->RDES3 & DWC_ETH_QOS_RDESC3_OWN)) {

#ifdef DWC_ETH_QOS_ENABLE_RX_DESC_DUMP
			dump_rx_desc(chInx, RX_NORMAL_DESC, desc_data->cur_rx);
#endif
			/* assign it to new skb */
			skb = buffer->skb;
			buffer->skb = NULL;

			pkt_len =
			    (RX_NORMAL_DESC->RDES3 & DWC_ETH_QOS_RDESC3_PL);

#ifdef NTN_RX_DATA_BUF_IN_SRAM
            		//NDBGPR_L2("unmap rx dma coherent memory\n");
			dma_unmap_single(&pdata->pdev->dev, buffer->dma_iommu, pdata->rx_buffer_len, DMA_FROM_DEVICE);

		        memcpy(buffer->skb_temp_data_ptr, skb->data, pkt_len);

    	                //NDBGPR_L2("virt_adrs = 0x%lx : buffer->dma = 0x%x\n", (long unsigned int)skb->data, (unsigned int)buffer->dma);
			dma_pool_free(pdata->rx_mem_pool, skb->data, buffer->dma);
            		//NDBGPR_L2("Free rx dma coherent memory done\n");

            		buffer->dma_iommu = 0;
            		skb->data = buffer->skb_temp_data_ptr;
#else
			dma_unmap_single(&pdata->pdev->dev, buffer->dma, pdata->rx_buffer_len, DMA_FROM_DEVICE);
#endif

			buffer->dma = 0;

			/* get the packet length */
			//pkt_len =
			//    (RX_NORMAL_DESC->RDES3 & DWC_ETH_QOS_RDESC3_PL);

#ifdef NTN_DRV_TEST_LOOPBACK
	NMSGPR_INFO("RX Channel = %d\n", chInx);
	update_pkt_lp(skb);
#endif

#ifdef DWC_ETH_QOS_ENABLE_RX_PKT_DUMP
			print_pkt(skb, pkt_len, 0, (desc_data->cur_rx));
#endif
			/* check for bad/oversized packet,
			 * error is valid only for last descriptor (OWN + LD bit set).
			 * */
			//NDBGPR_L2("RDES0: 0x%08x\n", RX_NORMAL_DESC->RDES0);
			//NDBGPR_L2("RDES1: 0x%08x\n", RX_NORMAL_DESC->RDES1);
			//NDBGPR_L2("RDES2: 0x%08x\n", RX_NORMAL_DESC->RDES2);
			//NDBGPR_L2("RDES3: 0x%08x\n", RX_NORMAL_DESC->RDES3);
			if (!(RX_NORMAL_DESC->RDES3 & DWC_ETH_QOS_RDESC3_ES) &&
			    (RX_NORMAL_DESC->RDES3 & DWC_ETH_QOS_RDESC3_LD)) {
				/* pkt_len = pkt_len - 4; */ /* CRC stripping */

				/* code added for copybreak, this should improve
				 * performance for small pkts with large amount
				 * of reassembly being done in the stack
				 * */
				if (pkt_len < DWC_ETH_QOS_COPYBREAK_DEFAULT) {
					struct sk_buff *new_skb =
					    netdev_alloc_skb_ip_align(dev,
								      pkt_len);
					if (new_skb) {
						skb_copy_to_linear_data_offset(new_skb,
							-NET_IP_ALIGN,
							(skb->data - NET_IP_ALIGN),
							(pkt_len + NET_IP_ALIGN));
						/* recycle actual desc skb */
						buffer->skb = skb;
						skb = new_skb;
					} else {
						/* just continue with the old skb */
					}
				}
				skb_put(skb, pkt_len);

				DWC_ETH_QOS_config_rx_csum(pdata, skb,
							RX_NORMAL_DESC);

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
				DWC_ETH_QOS_get_rx_vlan(pdata, skb, RX_NORMAL_DESC);
#endif

#ifdef YDEBUG_FILTER
				DWC_ETH_QOS_check_rx_filter_status(RX_NORMAL_DESC);
#endif

				if ((pdata->hw_feat.tsstssel) && (pdata->hwts_rx_en)) {
					/* get rx tstamp if available */
					if (hw_if->rx_tstamp_available(RX_NORMAL_DESC)) {
						ret = DWC_ETH_QOS_get_rx_hwtstamp(pdata,
								skb, desc_data, chInx);
						if (ret == 0) {
							/* device has not yet updated the CONTEXT desc to hold the
							 * time stamp, hence delay the packet reception
							 * */
							buffer->skb = skb;
                            NMSGPR_ERR("dma_map_single: %s: %s: %d\n", __FILE__, __FUNCTION__, __LINE__);
							buffer->dma = dma_map_single(&pdata->pdev->dev, skb->data,
									pdata->rx_buffer_len, DMA_FROM_DEVICE);
							if (dma_mapping_error(&pdata->pdev->dev, buffer->dma))
								NMSGPR_ALERT( "failed to do the RX dma map\n");

							goto rx_tstmp_failed;
						}
					}
				}


				if (!(dev->features & NETIF_F_GRO) &&
						(dev->features & NETIF_F_LRO)) {
						pdata->tcp_pkt =
								DWC_ETH_QOS_check_for_tcp_payload(RX_NORMAL_DESC);
				}

				dev->last_rx = jiffies;
				/* update the statistics */
				dev->stats.rx_packets++;
				dev->stats.rx_bytes += skb->len;
				DWC_ETH_QOS_receive_skb(pdata, dev, skb, chInx);
#ifdef NTN_DRV_TEST_LOOPBACK
				NMSGPR_INFO("RX Channel = %d, skb->len = %d \n", chInx, skb->len);
#endif
				received++;
			} else {
				NMSGPR_ALERT( "DWC_ETH_QOS clean IRQ failure!! \n");
#ifdef DWC_ETH_QOS_ENABLE_RX_DESC_DUMP
				dump_rx_desc(chInx, RX_NORMAL_DESC, desc_data->cur_rx);
#endif
				if (!(RX_NORMAL_DESC->RDES3 & DWC_ETH_QOS_RDESC3_LD))
					DBGPR("Received oversized pkt, spanned across multiple desc\n");

				/* recycle skb */
				buffer->skb = skb;
				dev->stats.rx_errors++;
				DWC_ETH_QOS_update_rx_errors(dev,
					RX_NORMAL_DESC->RDES3);
			}

			desc_data->dirty_rx++;
			if (desc_data->dirty_rx >= desc_data->skb_realloc_threshold)
				desc_if->realloc_skb(pdata, chInx);

			INCR_RX_DESC_INDEX(desc_data->cur_rx, 1, pdata->rx_dma_ch[chInx].desc_cnt);
		} else {
			/* no more data to read */
			break;
		}
	}

rx_tstmp_failed:

	if (desc_data->dirty_rx)
		desc_if->realloc_skb(pdata, chInx);

	DBGPR("<--DWC_ETH_QOS_clean_rx_irq: received = %d\n", received);

	return received;
}

/*!
* \brief API to update the rx status.
*
* \details This function is called in poll function to update the
* status of received packets.
*
* \param[in] dev - pointer to net_device structure.
* \param[in] rx_status - value of received packet status.
*
* \return void.
*/

void DWC_ETH_QOS_update_rx_errors(struct net_device *dev,
				 unsigned int rx_status)
{
	DBGPR("-->DWC_ETH_QOS_update_rx_errors\n");

	/* received pkt with crc error */
	if ((rx_status & 0x1000000))
		dev->stats.rx_crc_errors++;

	/* received frame alignment */
	if ((rx_status & 0x100000))
		dev->stats.rx_frame_errors++;

	/* receiver fifo overrun */
	if ((rx_status & 0x200000))
		dev->stats.rx_fifo_errors++;

	DBGPR("<--DWC_ETH_QOS_update_rx_errors\n");
}

/*!
* \brief API to pass the received packets to stack
*
* \details This function is provided by NAPI-compliant drivers to operate
* the interface in a polled mode, with interrupts disabled.
*
* \param[in] napi - pointer to napi_stuct structure.
* \param[in] budget - maximum no. of packets that we are allowed to pass
* to into the kernel.
*
* \return integer
*
* \retval number of packets received.
*/

int DWC_ETH_QOS_poll_mq(struct napi_struct *napi, int budget)
{
	struct DWC_ETH_QOS_rx_dma_ch *rx_dma_ch =
		container_of(napi, struct DWC_ETH_QOS_rx_dma_ch, napi);
	struct DWC_ETH_QOS_prv_data *pdata = rx_dma_ch->pdata;
	/* divide the budget evenly among all the queues */
	int per_q_budget = budget / NTN_RX_DMA_CH_CNT;
	int chInx = 0;
	int received = 0, per_q_received = 0;

	DBGPR("-->DWC_ETH_QOS_poll_mq: budget = %d\n", budget);

	pdata->xstats.napi_poll_n++;
	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {

#ifdef DWC_ETH_QOS_TXPOLLING_MODE_ENABLE
		/* check for tx descriptor status */
		if( (chInx < NTN_TX_DMA_CH_CNT) && (pdata->tx_dma_ch_for_host[chInx]) )
			DWC_ETH_QOS_tx_interrupt(pdata->dev, pdata, chInx);
#endif
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;

		if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0) {
			pr_debug("RX pkts are recieved by IPA uC..so skip for the host \n");
			continue;
		}

		rx_dma_ch = GET_RX_DMA_CH_PTR(chInx);

		rx_dma_ch->lro_flush_needed = 0;

#ifdef RX_OLD_CODE
		per_q_received = DWC_ETH_QOS_poll(pdata, per_q_budget, chInx);
#else
		per_q_received = pdata->clean_rx(pdata, per_q_budget, chInx);
#endif
		received += per_q_received;
		pdata->xstats.rx_pkt_n += per_q_received;
		pdata->xstats.q_rx_pkt_n[chInx] += per_q_received;

		if (rx_dma_ch->lro_flush_needed) {
			NMSGPR_ALERT( "INET_LRO not enabled in kernel : %s\n", __func__);
			/*lro_flush_all(&rx_dma_ch->lro_mgr); */
			}
	}

	/* If we processed all pkts, we are done;
	 * tell the kernel & re-enable interrupt */
	if (received < budget) {
		if (pdata->dev->features & NETIF_F_GRO) {
			/* to turn off polling */
			napi_complete(napi);
			/* Enable all ch RX interrupt */
			DWC_ETH_QOS_enable_all_ch_rx_interrpt(pdata);
		} else {
			unsigned long flags;

			spin_lock_irqsave(&pdata->lock, flags);
			__napi_complete(napi);
			/* Enable all ch RX interrupt */
			DWC_ETH_QOS_enable_all_ch_rx_interrpt(pdata);
			spin_unlock_irqrestore(&pdata->lock, flags);
		}
	}

	DBGPR("<--DWC_ETH_QOS_poll_mq\n");

	return received;
}

/*!
* \brief API to return the device/interface status.
*
* \details The get_stats function is called whenever an application needs to
* get statistics for the interface. For example, this happend when ifconfig
* or netstat -i is run.
*
* \param[in] dev - pointer to net_device structure.
*
* \return net_device_stats structure
*
* \retval net_device_stats - returns pointer to net_device_stats structure.
*/

static struct net_device_stats *DWC_ETH_QOS_get_stats(struct net_device *dev)
{

	return &dev->stats;
}

#ifdef CONFIG_NET_POLL_CONTROLLER

/*!
* \brief API to receive packets in polling mode.
*
* \details This is polling receive function used by netconsole and other
* diagnostic tool to allow network i/o with interrupts disabled.
*
* \param[in] dev - pointer to net_device structure
*
* \return void
*/

static void DWC_ETH_QOS_poll_controller(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	DBGPR("-->DWC_ETH_QOS_poll_controller\n");

	disable_irq(pdata->irq_number);
	DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS(pdata->irq_number, pdata);
	enable_irq(pdata->irq_number);

	DBGPR("<--DWC_ETH_QOS_poll_controller\n");
}

#endif	/*end of CONFIG_NET_POLL_CONTROLLER */

/*!
 * \brief User defined parameter setting API
 *
 * \details This function is invoked by kernel to update the device
 * configuration to new features. This function supports enabling and
 * disabling of TX and RX csum features.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] features – device feature to be enabled/disabled.
 *
 * \return int
 *
 * \retval 0
 */

static int DWC_ETH_QOS_set_features(struct net_device *dev, netdev_features_t features)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT dev_rxcsum_enable;
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	UINT dev_rxvlan_enable, dev_txvlan_enable;
#endif

	if (pdata->hw_feat.rx_coe_sel) {
		dev_rxcsum_enable = !!(pdata->dev_state & NETIF_F_RXCSUM);

		if (((features & NETIF_F_RXCSUM) == NETIF_F_RXCSUM)
		    && !dev_rxcsum_enable) {
			hw_if->enable_rx_csum(pdata);
			pdata->dev_state |= NETIF_F_RXCSUM;
			NMSGPR_ALERT( "State change - rxcsum enable\n");
		} else if (((features & NETIF_F_RXCSUM) == 0)
			   && dev_rxcsum_enable) {
			hw_if->disable_rx_csum(pdata);
			pdata->dev_state &= ~NETIF_F_RXCSUM;
			NMSGPR_ALERT( "State change - rxcsum disable\n");
		}
	}
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	dev_rxvlan_enable = !!(pdata->dev_state & NETIF_F_HW_VLAN_CTAG_RX);
	if (((features & NETIF_F_HW_VLAN_CTAG_RX) == NETIF_F_HW_VLAN_CTAG_RX)
	    && !dev_rxvlan_enable) {
		pdata->dev_state |= NETIF_F_HW_VLAN_CTAG_RX;
		hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS, pdata);
		DBGPR_VLAN("%s:VLAN: State change - rxvlan enable\n",__func__);
	} else if (((features & NETIF_F_HW_VLAN_CTAG_RX) == 0) &&
			dev_rxvlan_enable) {
		pdata->dev_state &= ~NETIF_F_HW_VLAN_CTAG_RX;
		hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP, pdata);
		DBGPR_VLAN("%s:VLAN: State change - rxvlan disable\n",__func__);
	}

	dev_txvlan_enable = !!(pdata->dev_state & NETIF_F_HW_VLAN_CTAG_TX);
	if (((features & NETIF_F_HW_VLAN_CTAG_TX) == NETIF_F_HW_VLAN_CTAG_TX)
	    && !dev_txvlan_enable) {
		pdata->dev_state |= NETIF_F_HW_VLAN_CTAG_TX;
		DBGPR_VLAN("%s:VLAN: State change - txvlan enable\n",__func__);
	} else if (((features & NETIF_F_HW_VLAN_CTAG_TX) == 0) &&
			dev_txvlan_enable) {
		pdata->dev_state &= ~NETIF_F_HW_VLAN_CTAG_TX;
		DBGPR_VLAN("%s:VLAN: State change - txvlan disable\n",__func__);
	}
#endif	/* DWC_ETH_QOS_ENABLE_VLAN_TAG */
	DBGPR("<--DWC_ETH_QOS_set_features\n");

	return 0;
}


/*!
 * \brief User defined parameter setting API
 *
 * \details This function is invoked by kernel to adjusts the requested
 * feature flags according to device-specific constraints, and returns the
 * resulting flags. This API must not modify the device state.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] features – device supported features.
 *
 * \return u32
 *
 * \retval modified flag
 */

static netdev_features_t DWC_ETH_QOS_fix_features(struct net_device *dev, netdev_features_t features)
{

	DBGPR("-->DWC_ETH_QOS_fix_features: %#llx\n", features);

	DBGPR("<--DWC_ETH_QOS_fix_features: %#llx\n", features);

	return features;
}

static int DWC_ETH_QOS_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(netdev);
	int old_mtu   = netdev->mtu;
	int max_frame = new_mtu + ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN;

	/*for MIN MTU, it may be possible that HW can add padded bytes to
	  make it 60B. However, currently MIN MTU is not being supporting
	  via this command
        */
	if ((max_frame < DWC_ETH_QOS_MIN_SUPPORTED_MTU) ||
	    (max_frame > DWC_ETH_QOS_MAX_SUPPORTED_MTU)) {
		NMSGPR_ERR("invalid MTU setting %s: %s: %d: %d\n",__FILE__,__FUNCTION__,__LINE__,max_frame);
		return -EINVAL;
	}
	/*set MTU */
	if (old_mtu != new_mtu && netif_running(netdev)) {
		NDBGPR_L2("changing MTU from %d to %d\n", netdev->mtu, new_mtu);
		netdev->mtu = new_mtu;
		pdata->dev->mtu = new_mtu;
		pdata->rx_buffer_len = max_frame > DWC_ETH_QOS_ETH_FRAME_LEN ?
				   ALIGN(max_frame, 8) : DWC_ETH_QOS_ETH_FRAME_LEN;
		netdev_update_features(netdev);
		/*reconfigure or restart the mac */
		DWC_ETH_QOS_close(netdev);
		DWC_ETH_QOS_open(netdev);
		/*program the MAC HW for giant pkt if max_frame> 1522 B*/
		if ( max_frame > DWC_ETH_QOS_ETH_FRAME_LEN )
		{
		    MAC_MCR_GPSLCE_UdfWr((ULONG)0x1);
		    MAC_MECR_GPSL_UdfWr((ULONG)0x1<<11);
		}
	}

	return 0;
}


/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to enable/disable L3/L4 filtering.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] flags – flag to indicate whether L3/L4 filtering to be
 *                  enabled/disabled.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_l3_l4_filtering(struct net_device *dev,
		unsigned int flags)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int ret = 0;

	DBGPR_FILTER("-->DWC_ETH_QOS_config_l3_l4_filtering\n");

	if (flags && pdata->l3_l4_filter) {
		NMSGPR_ALERT(
			"L3/L4 filtering is already enabled\n");
		return -EINVAL;
	}

	if (!flags && !pdata->l3_l4_filter) {
		NMSGPR_ALERT(
			"L3/L4 filtering is already disabled\n");
		return -EINVAL;
	}

	pdata->l3_l4_filter = !!flags;
	hw_if->config_l3_l4_filter_enable(pdata->l3_l4_filter, pdata);

	DBGPR_FILTER("Succesfully %s L3/L4 filtering\n",
		(flags ? "ENABLED" : "DISABLED"));

	DBGPR_FILTER("<--DWC_ETH_QOS_config_l3_l4_filtering\n");

	return ret;
}


/*!
 * \details This function is invoked by ioctl function when user issues an
 * ioctl command to configure L3(IPv4) filtering. This function does following,
 * - enable/disable IPv4 filtering.
 * - select source/destination address matching.
 * - select perfect/inverse matching.
 * - Update the IPv4 address into MAC register.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to IOCTL specific structure.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_ip4_filters(struct net_device *dev,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_l3_l4_filter *u_l3_filter =
		(struct DWC_ETH_QOS_l3_l4_filter *)req->ptr;
	struct DWC_ETH_QOS_l3_l4_filter l_l3_filter;
	int ret = 0;

	DBGPR_FILTER("-->DWC_ETH_QOS_config_ip4_filters\n");

	if (pdata->hw_feat.l3l4_filter_num == 0)
		return DWC_ETH_QOS_NO_HW_SUPPORT;

	if (copy_from_user(&l_l3_filter, u_l3_filter,
		sizeof(struct DWC_ETH_QOS_l3_l4_filter)))
		return -EFAULT;

	if ((l_l3_filter.filter_no + 1) > pdata->hw_feat.l3l4_filter_num) {
		NMSGPR_ALERT( "%d filter is not supported in the HW\n",
			l_l3_filter.filter_no);
		return DWC_ETH_QOS_NO_HW_SUPPORT;
	}

	if (!pdata->l3_l4_filter) {
		hw_if->config_l3_l4_filter_enable(1, pdata);
		pdata->l3_l4_filter = 1;
	}

	/* configure the L3 filters */
	hw_if->config_l3_filters(l_l3_filter.filter_no,
			l_l3_filter.filter_enb_dis, 0,
			l_l3_filter.src_dst_addr_match,
			l_l3_filter.perfect_inverse_match, pdata);

	if (!l_l3_filter.src_dst_addr_match)
		hw_if->update_ip4_addr0(l_l3_filter.filter_no,
				l_l3_filter.ip4_addr, pdata);
	else
		hw_if->update_ip4_addr1(l_l3_filter.filter_no,
				l_l3_filter.ip4_addr, pdata);

	DBGPR_FILTER("Successfully %s IPv4 %s %s addressing filtering on %d filter\n",
		(l_l3_filter.filter_enb_dis ? "ENABLED" : "DISABLED"),
		(l_l3_filter.perfect_inverse_match ? "INVERSE" : "PERFECT"),
		(l_l3_filter.src_dst_addr_match ? "DESTINATION" : "SOURCE"),
		l_l3_filter.filter_no);

	DBGPR_FILTER("<--DWC_ETH_QOS_config_ip4_filters\n");

	return ret;
}


/*!
 * \details This function is invoked by ioctl function when user issues an
 * ioctl command to configure L3(IPv6) filtering. This function does following,
 * - enable/disable IPv6 filtering.
 * - select source/destination address matching.
 * - select perfect/inverse matching.
 * - Update the IPv6 address into MAC register.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to IOCTL specific structure.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_ip6_filters(struct net_device *dev,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_l3_l4_filter *u_l3_filter =
		(struct DWC_ETH_QOS_l3_l4_filter *)req->ptr;
	struct DWC_ETH_QOS_l3_l4_filter l_l3_filter;
	int ret = 0;

	DBGPR_FILTER("-->DWC_ETH_QOS_config_ip6_filters\n");

	if (pdata->hw_feat.l3l4_filter_num == 0)
		return DWC_ETH_QOS_NO_HW_SUPPORT;

	if (copy_from_user(&l_l3_filter, u_l3_filter,
		sizeof(struct DWC_ETH_QOS_l3_l4_filter)))
		return -EFAULT;

	if ((l_l3_filter.filter_no + 1) > pdata->hw_feat.l3l4_filter_num) {
		NMSGPR_ALERT( "%d filter is not supported in the HW\n",
			l_l3_filter.filter_no);
		return DWC_ETH_QOS_NO_HW_SUPPORT;
	}

	if (!pdata->l3_l4_filter) {
		hw_if->config_l3_l4_filter_enable(1, pdata);
		pdata->l3_l4_filter = 1;
	}

	/* configure the L3 filters */
	hw_if->config_l3_filters(l_l3_filter.filter_no,
			l_l3_filter.filter_enb_dis, 1,
			l_l3_filter.src_dst_addr_match,
			l_l3_filter.perfect_inverse_match, pdata);

	hw_if->update_ip6_addr(l_l3_filter.filter_no,
			l_l3_filter.ip6_addr, pdata);

	DBGPR_FILTER("Successfully %s IPv6 %s %s addressing filtering on %d filter\n",
		(l_l3_filter.filter_enb_dis ? "ENABLED" : "DISABLED"),
		(l_l3_filter.perfect_inverse_match ? "INVERSE" : "PERFECT"),
		(l_l3_filter.src_dst_addr_match ? "DESTINATION" : "SOURCE"),
		l_l3_filter.filter_no);

	DBGPR_FILTER("<--DWC_ETH_QOS_config_ip6_filters\n");

	return ret;
}

/*!
 * \details This function is invoked by ioctl function when user issues an
 * ioctl command to configure L4(TCP/UDP) filtering. This function does following,
 * - enable/disable L4 filtering.
 * - select TCP/UDP filtering.
 * - select source/destination port matching.
 * - select perfect/inverse matching.
 * - Update the port number into MAC register.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to IOCTL specific structure.
 * \param[in] tcp_udp – flag to indicate TCP/UDP filtering.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_tcp_udp_filters(struct net_device *dev,
		struct ifr_data_struct *req,
		int tcp_udp)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_l3_l4_filter *u_l4_filter =
		(struct DWC_ETH_QOS_l3_l4_filter *)req->ptr;
	struct DWC_ETH_QOS_l3_l4_filter l_l4_filter;
	int ret = 0;

	DBGPR_FILTER("-->DWC_ETH_QOS_config_tcp_udp_filters\n");

	if (pdata->hw_feat.l3l4_filter_num == 0)
		return DWC_ETH_QOS_NO_HW_SUPPORT;

	if (copy_from_user(&l_l4_filter, u_l4_filter,
		sizeof(struct DWC_ETH_QOS_l3_l4_filter)))
		return -EFAULT;

	if ((l_l4_filter.filter_no + 1) > pdata->hw_feat.l3l4_filter_num) {
		NMSGPR_ALERT( "%d filter is not supported in the HW\n",
			l_l4_filter.filter_no);
		return DWC_ETH_QOS_NO_HW_SUPPORT;
	}

	if (!pdata->l3_l4_filter) {
		hw_if->config_l3_l4_filter_enable(1, pdata);
		pdata->l3_l4_filter = 1;
	}

	/* configure the L4 filters */
	hw_if->config_l4_filters(l_l4_filter.filter_no,
			l_l4_filter.filter_enb_dis,
			tcp_udp,
			l_l4_filter.src_dst_addr_match,
			l_l4_filter.perfect_inverse_match, pdata);

	if (l_l4_filter.src_dst_addr_match)
		hw_if->update_l4_da_port_no(l_l4_filter.filter_no,
				l_l4_filter.port_no, pdata);
	else
		hw_if->update_l4_sa_port_no(l_l4_filter.filter_no,
				l_l4_filter.port_no, pdata);

	DBGPR_FILTER("Successfully %s %s %s %s Port number filtering on %d filter\n",
		(l_l4_filter.filter_enb_dis ? "ENABLED" : "DISABLED"),
		(tcp_udp ? "UDP" : "TCP"),
		(l_l4_filter.perfect_inverse_match ? "INVERSE" : "PERFECT"),
		(l_l4_filter.src_dst_addr_match ? "DESTINATION" : "SOURCE"),
		l_l4_filter.filter_no);

	DBGPR_FILTER("<--DWC_ETH_QOS_config_tcp_udp_filters\n");

	return ret;
}


/*!
 * \details This function is invoked by ioctl function when user issues an
 * ioctl command to configure VALN filtering. This function does following,
 * - enable/disable VLAN filtering.
 * - select perfect/hash filtering.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to IOCTL specific structure.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_vlan_filter(struct net_device *dev,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_vlan_filter *u_vlan_filter =
		(struct DWC_ETH_QOS_vlan_filter *)req->ptr;
	struct DWC_ETH_QOS_vlan_filter l_vlan_filter;
	int ret = 0;

	DBGPR_FILTER("-->DWC_ETH_QOS_config_vlan_filter\n");

	if (copy_from_user(&l_vlan_filter, u_vlan_filter,
		sizeof(struct DWC_ETH_QOS_vlan_filter)))
		return -EFAULT;

	if ((l_vlan_filter.perfect_hash) &&
		(pdata->hw_feat.vlan_hash_en == 0)) {
		NMSGPR_ALERT( "VLAN HASH filtering is not supported\n");
		return DWC_ETH_QOS_NO_HW_SUPPORT;
	}

	/* configure the vlan filter */
	hw_if->config_vlan_filtering(l_vlan_filter.filter_enb_dis,
					l_vlan_filter.perfect_hash,
					l_vlan_filter.perfect_inverse_match, pdata);
	pdata->vlan_hash_filtering = l_vlan_filter.perfect_hash;

	DBGPR_FILTER("Successfully %s VLAN %s filtering and %s matching\n",
		(l_vlan_filter.filter_enb_dis ? "ENABLED" : "DISABLED"),
		(l_vlan_filter.perfect_hash ? "HASH" : "PERFECT"),
		(l_vlan_filter.perfect_inverse_match ? "INVERSE" : "PERFECT"));

	DBGPR_FILTER("<--DWC_ETH_QOS_config_vlan_filter\n");

	return ret;
}


/*!
 * \details This function is invoked by ioctl function when user issues an
 * ioctl command to enable/disable ARP offloading feature.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to IOCTL specific structure.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_arp_offload(struct net_device *dev,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_arp_offload *u_arp_offload =
		(struct DWC_ETH_QOS_arp_offload *)req->ptr;
	struct DWC_ETH_QOS_arp_offload l_arp_offload;
	int ret = 0;

	NMSGPR_ALERT( "-->DWC_ETH_QOS_config_arp_offload\n");

	if (pdata->hw_feat.arp_offld_en == 0)
		return DWC_ETH_QOS_NO_HW_SUPPORT;

	if (copy_from_user(&l_arp_offload, u_arp_offload,
		sizeof(struct DWC_ETH_QOS_arp_offload)))
		return -EFAULT;

	/* configure the L3 filters */
	hw_if->config_arp_offload(req->flags, pdata);
	hw_if->update_arp_offload_ip_addr(l_arp_offload.ip_addr, pdata);
	pdata->arp_offload = req->flags;

	NMSGPR_ALERT( "Successfully %s arp Offload\n",
		(req->flags ? "ENABLED" : "DISABLED"));

	NMSGPR_ALERT( "<--DWC_ETH_QOS_config_arp_offload\n");

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
static int DWC_ETH_QOS_confing_l2_da_filter(struct net_device *dev,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct DWC_ETH_QOS_l2_da_filter *u_l2_da_filter =
	  (struct DWC_ETH_QOS_l2_da_filter *)req->ptr;
	struct DWC_ETH_QOS_l2_da_filter l_l2_da_filter;
	int ret = 0;

	DBGPR_FILTER("-->DWC_ETH_QOS_confing_l2_da_filter\n");

	if (copy_from_user(&l_l2_da_filter, u_l2_da_filter,
	      sizeof(struct DWC_ETH_QOS_l2_da_filter)))
		return - EFAULT;

	if (l_l2_da_filter.perfect_hash) {
		if (pdata->hw_feat.hash_tbl_sz > 0)
			pdata->l2_filtering_mode = 1;
		else
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
	} else {
		if (pdata->max_addr_reg_cnt > 1)
			pdata->l2_filtering_mode = 0;
		else
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
	}

	/* configure L2 DA perfect/inverse_matching */
	hw_if->config_l2_da_perfect_inverse_match(l_l2_da_filter.perfect_inverse_match, pdata);

	DBGPR_FILTER("Successfully selected L2 %s filtering and %s DA matching\n",
		(l_l2_da_filter.perfect_hash ? "HASH" : "PERFECT"),
		(l_l2_da_filter.perfect_inverse_match ? "INVERSE" : "PERFECT"));

	DBGPR_FILTER("<--DWC_ETH_QOS_confing_l2_da_filter\n");

	return ret;
}

/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to enable/disable mac loopback mode.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] flags – flag to indicate whether mac loopback mode to be
 *                  enabled/disabled.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_mac_loopback_mode(struct net_device *dev,
		unsigned int flags)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_config_mac_loopback_mode\n");

	if (flags && pdata->mac_loopback_mode) {
		NMSGPR_ALERT(
			"MAC loopback mode is already enabled\n");
		return -EINVAL;
	}
	if (!flags && !pdata->mac_loopback_mode) {
		NMSGPR_ALERT(
			"MAC loopback mode is already disabled\n");
		return -EINVAL;
	}
	pdata->mac_loopback_mode = !!flags;
	hw_if->config_mac_loopback_mode(flags, pdata);

	NMSGPR_ALERT( "Succesfully %s MAC loopback mode\n",
		(flags ? "enabled" : "disabled"));

	DBGPR("<--DWC_ETH_QOS_config_mac_loopback_mode\n");

	return ret;
}

/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to enable/disable phy loopback mode.
 *
 * \param[in] dev pointer to net device structure.
 * \param[in] flags flag to indicate whether mac loopback mode to be
 *            enabled/disabled.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 **/
static int DWC_ETH_QOS_config_phy_loopback_mode(struct net_device *dev,
                unsigned int flags)
{
        struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
        int ret = 0, regval;

        DBGPR("-->DWC_ETH_QOS_config_phy_loopback_mode\n");

        if (flags && pdata->phy_loopback_mode) {
                NMSGPR_ALERT(
                        "PHY loopback mode is already enabled\n");
                return -EINVAL;
        }
        if (!flags && !pdata->phy_loopback_mode) {
                NMSGPR_ALERT(
                        "PHY loopback mode is already disabled\n");
                return -EINVAL;
        }
        pdata->phy_loopback_mode = !!flags;

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_BMCR, &regval);
	regval = (regval & (~(1<<14))) | (flags<<14);
	DWC_ETH_QOS_mdio_write_direct(pdata, pdata->phyaddr, MII_BMCR, regval);

	/* Shallow Loopback */
	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_AUX_CTRL, &regval);
	regval = (regval & (~(1<<5))) | (flags<<5);
	DWC_ETH_QOS_mdio_write_direct(pdata, pdata->phyaddr, MII_AUX_CTRL, regval);
	NMSGPR_ALERT( "Succesfully %s PHY loopback (shallow) mode\n",
                (flags ? "enabled" : "disabled"));

        DBGPR("<--DWC_ETH_QOS_config_phy_loopback_mode\n");

        return ret;
}

#ifdef DWC_ETH_QOS_ENABLE_DVLAN
static INT config_tx_dvlan_processing_via_reg(struct DWC_ETH_QOS_prv_data *pdata,
						UINT flags)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	NMSGPR_ALERT( "--> config_tx_dvlan_processing_via_reg()\n");

	if (pdata->in_out & DWC_ETH_QOS_DVLAN_OUTER)
		hw_if->config_tx_outer_vlan(pdata->op_type,
					pdata->outer_vlan_tag, pdata);

	if (pdata->in_out & DWC_ETH_QOS_DVLAN_INNER)
		hw_if->config_tx_inner_vlan(pdata->op_type,
					pdata->inner_vlan_tag, pdata);

	if (flags == DWC_ETH_QOS_DVLAN_DISABLE)
		hw_if->config_mac_for_vlan_pkt(); /* restore default configurations */
	else
		hw_if->config_dvlan(1, pdata);

	NMSGPR_ALERT( "<-- config_tx_dvlan_processing_via_reg()\n");

	return Y_SUCCESS;
}

static int config_tx_dvlan_processing_via_desc(struct DWC_ETH_QOS_prv_data *pdata,
						UINT flags)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	NMSGPR_ALERT( "-->config_tx_dvlan_processing_via_desc\n");

	if (flags == DWC_ETH_QOS_DVLAN_DISABLE) {
		hw_if->config_mac_for_vlan_pkt(); /* restore default configurations */
		pdata->via_reg_or_desc = 0;
	} else {
		hw_if->config_dvlan(1, pdata);
	}

	if (pdata->in_out & DWC_ETH_QOS_DVLAN_INNER)
			MAC_IVLANTIRR_VLTI_UdfWr(1);

	if (pdata->in_out & DWC_ETH_QOS_DVLAN_OUTER)
			MAC_VLANTIRR_VLTI_UdfWr(1);

	NMSGPR_ALERT( "<--config_tx_dvlan_processing_via_desc\n");

	return Y_SUCCESS;
}

/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to configure mac double vlan processing feature.
 *
 * \param[in] pdata - pointer to private data structure.
 * \param[in] flags – Each bit in this variable carry some information related
 *		      double vlan processing.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_tx_dvlan_processing(
		struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_config_dvlan l_config_doubule_vlan,
					  *u_config_doubule_vlan = req->ptr;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_config_tx_dvlan_processing\n");

	if(copy_from_user(&l_config_doubule_vlan, u_config_doubule_vlan,
				sizeof(struct DWC_ETH_QOS_config_dvlan))) {
		NMSGPR_ALERT( "Failed to fetch Double vlan Struct info from user\n");
		return DWC_ETH_QOS_CONFIG_FAIL;
	}

	pdata->inner_vlan_tag = l_config_doubule_vlan.inner_vlan_tag;
	pdata->outer_vlan_tag = l_config_doubule_vlan.outer_vlan_tag;
	pdata->op_type = l_config_doubule_vlan.op_type;
	pdata->in_out = l_config_doubule_vlan.in_out;
	pdata->via_reg_or_desc = l_config_doubule_vlan.via_reg_or_desc;

	if (pdata->via_reg_or_desc == DWC_ETH_QOS_VIA_REG)
		ret = config_tx_dvlan_processing_via_reg(pdata, req->flags);
	else
		ret = config_tx_dvlan_processing_via_desc(pdata, req->flags);

	DBGPR("<--DWC_ETH_QOS_config_tx_dvlan_processing\n");

	return ret;
}

/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to configure mac double vlan processing feature.
 *
 * \param[in] pdata - pointer to private data structure.
 * \param[in] flags – Each bit in this variable carry some information related
 *		      double vlan processing.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_rx_dvlan_processing(
		struct DWC_ETH_QOS_prv_data *pdata, unsigned int flags)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_config_rx_dvlan_processing\n");

	hw_if->config_dvlan(1, pdata);
	if (flags == DWC_ETH_QOS_DVLAN_NONE) {
		hw_if->config_dvlan(0, pdata);
		hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP, pdata);
		hw_if->config_rx_inner_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP, pdata);
	} else if (flags == DWC_ETH_QOS_DVLAN_INNER) {
		hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP, pdata);
		hw_if->config_rx_inner_vlan_stripping(DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS, pdata);
	} else if (flags == DWC_ETH_QOS_DVLAN_OUTER) {
		hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS, pdata);
		hw_if->config_rx_inner_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP, pdata);
	} else if (flags == DWC_ETH_QOS_DVLAN_BOTH) {
		hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS, pdata);
		hw_if->config_rx_inner_vlan_stripping(DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS, pdata);
	} else {
		NMSGPR_ALERT( "ERROR : double VLAN Rx configuration - Invalid argument");
		ret = DWC_ETH_QOS_CONFIG_FAIL;
	}

	DBGPR("<--DWC_ETH_QOS_config_rx_dvlan_processing\n");

	return ret;
}

/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to configure mac double vlan (svlan) processing feature.
 *
 * \param[in] pdata - pointer to private data structure.
 * \param[in] flags – Each bit in this variable carry some information related
 *		      double vlan processing.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_svlan(struct DWC_ETH_QOS_prv_data *pdata,
					unsigned int flags)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_config_svlan\n");

	ret = hw_if->config_svlan(flags, pdata);
	if (ret == Y_FAILURE)
		ret = DWC_ETH_QOS_CONFIG_FAIL;

	DBGPR("<--DWC_ETH_QOS_config_svlan\n");

	return ret;
}
#endif /* end of DWC_ETH_QOS_ENABLE_DVLAN */

static VOID DWC_ETH_QOS_config_timer_registers(
				struct DWC_ETH_QOS_prv_data *pdata)
{
		struct timespec now;
		struct hw_if_struct *hw_if = &(pdata->hw_if);
		u64 temp;

		DBGPR("-->DWC_ETH_QOS_config_timer_registers\n");

		/* program Sub Second Increment Reg */
		hw_if->config_sub_second_increment(DWC_ETH_QOS_SYSCLOCK, pdata);

		/* formula is :
		 * addend = 2^32/freq_div_ratio;
		 *
		 * where, freq_div_ratio = DWC_ETH_QOS_SYSCLOCK/50MHz
		 *
		 * hence, addend = ((2^32) * 50MHz)/DWC_ETH_QOS_SYSCLOCK;
		 *
		 * NOTE: DWC_ETH_QOS_SYSCLOCK should be >= 50MHz to
		 *       achive 20ns accuracy.
		 *
		 * 2^x * y == (y << x), hence
		 * 2^32 * 50000000 ==> (50000000 << 32)
		 * */
		temp = (u64)(50000000ULL << 32);
		pdata->default_addend = div_u64(temp, DWC_ETH_QOS_SYSCLOCK);

		hw_if->config_addend(pdata->default_addend, pdata);

		/* initialize system time */
		getnstimeofday(&now);
		hw_if->init_systime(now.tv_sec, now.tv_nsec, pdata);

		DBGPR("-->DWC_ETH_QOS_config_timer_registers\n");
}

/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to configure PTP offloading feature.
 *
 * \param[in] pdata - pointer to private data structure.
 * \param[in] flags – Each bit in this variable carry some information related
 *		      double vlan processing.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_ptpoffload(
		struct DWC_ETH_QOS_prv_data *pdata,
		struct DWC_ETH_QOS_config_ptpoffloading *u_conf_ptp)
{
	UINT pto_cntrl;
	UINT varMAC_TCR;
	struct DWC_ETH_QOS_config_ptpoffloading l_conf_ptp;
	struct hw_if_struct *hw_if = &(pdata->hw_if);


	if(copy_from_user(&l_conf_ptp, u_conf_ptp,
				sizeof(struct DWC_ETH_QOS_config_ptpoffloading))) {
		NMSGPR_ALERT( "Failed to fetch Double vlan Struct info from user\n");
		return DWC_ETH_QOS_CONFIG_FAIL;
	}

	NMSGPR_ALERT("-->DWC_ETH_QOS_config_ptpoffload - %d\n",l_conf_ptp.mode);

	pto_cntrl = MAC_PTOCR_PTOEN; /* enable ptp offloading */
	varMAC_TCR = MAC_TCR_TSENA | MAC_TCR_TSIPENA | MAC_TCR_TSVER2ENA
			| MAC_TCR_TSCFUPDT | MAC_TCR_TSCTRLSSR;
	if (l_conf_ptp.mode == DWC_ETH_QOS_PTP_ORDINARY_SLAVE) {

		varMAC_TCR |= MAC_TCR_TSEVENTENA;
		pdata->ptp_offloading_mode = DWC_ETH_QOS_PTP_ORDINARY_SLAVE;

	} else if (l_conf_ptp.mode == DWC_ETH_QOS_PTP_TRASPARENT_SLAVE) {

		pto_cntrl |= MAC_PTOCR_APDREQEN;
		varMAC_TCR |= MAC_TCR_TSEVENTENA;
		varMAC_TCR |= MAC_TCR_SNAPTYPSEL_1;
		pdata->ptp_offloading_mode =
			DWC_ETH_QOS_PTP_TRASPARENT_SLAVE;

	} else if (l_conf_ptp.mode == DWC_ETH_QOS_PTP_ORDINARY_MASTER) {

		pto_cntrl |= MAC_PTOCR_ASYNCEN;
		varMAC_TCR |= MAC_TCR_TSEVENTENA;
		varMAC_TCR |= MAC_TCR_TSMASTERENA;
		pdata->ptp_offloading_mode = DWC_ETH_QOS_PTP_ORDINARY_MASTER;

	} else if(l_conf_ptp.mode == DWC_ETH_QOS_PTP_TRASPARENT_MASTER) {

		pto_cntrl |= MAC_PTOCR_ASYNCEN | MAC_PTOCR_APDREQEN;
		varMAC_TCR |= MAC_TCR_SNAPTYPSEL_1;
		varMAC_TCR |= MAC_TCR_TSEVENTENA;
		varMAC_TCR |= MAC_TCR_TSMASTERENA;
		pdata->ptp_offloading_mode =
			DWC_ETH_QOS_PTP_TRASPARENT_MASTER;

	} else if (l_conf_ptp.mode == DWC_ETH_QOS_PTP_PEER_TO_PEER_TRANSPARENT) {

		pto_cntrl |= MAC_PTOCR_APDREQEN;
		varMAC_TCR |= MAC_TCR_SNAPTYPSEL_3;
		pdata->ptp_offloading_mode =
			DWC_ETH_QOS_PTP_PEER_TO_PEER_TRANSPARENT;
	}

	pdata->ptp_offload = 1;
	if (l_conf_ptp.en_dis == DWC_ETH_QOS_PTP_OFFLOADING_DISABLE) {
		pto_cntrl = 0;
		varMAC_TCR = 0;
		pdata->ptp_offload = 0;
	}

	pto_cntrl |= (l_conf_ptp.domain_num << 8);
	hw_if->config_hw_time_stamping(varMAC_TCR, pdata);
	DWC_ETH_QOS_config_timer_registers(pdata);
	hw_if->config_ptpoffload_engine(pto_cntrl, l_conf_ptp.mc_uc, pdata);

	NMSGPR_ALERT("<--DWC_ETH_QOS_config_ptpoffload\n");

	return Y_SUCCESS;
}









/*!
 * \details This function is invoked by ioctl function when user issues
 * an ioctl command to enable/disable pfc.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] flags – flag to indicate whether pfc to be enabled/disabled.
 *
 * \return integer
 *
 * \retval zero on success and -ve number on failure.
 */
static int DWC_ETH_QOS_config_pfc(struct net_device *dev,
		unsigned int flags)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_config_pfc\n");

	if (!pdata->hw_feat.dcb_en) {
		NMSGPR_ALERT( "PFC is not supported\n");
		return DWC_ETH_QOS_NO_HW_SUPPORT;
	}

	hw_if->config_pfc(flags, pdata);

	NMSGPR_ALERT( "Succesfully %s PFC(Priority Based Flow Control)\n",
		(flags ? "enabled" : "disabled"));

	DBGPR("<--DWC_ETH_QOS_config_pfc\n");

	return ret;
}

/*!
 * \brief Register read/write function called by driver IOCTL routine
 *     This function doesn't do address validation, it is application's resposibility
 *     to make sure a valid address is passed in.
 *
 * \param[in] pdata : pointer to private data structure.
 * \param[in] req : pointer to ioctl structure.
 *     	req->flags = 0: Neutrino Register Read
 *     	req->flags = 1: Neutrino Register Write
 *     	req->flags = 2: PCIe config Register Read
 *     	req->flags = 3: PCIe config Register Write
 *
 *		req->bar_num = 0: Use BAR0 for Register access
 *		req->bar_num = 2: Use BAR2 for SRAM access
 *		req->bar_num = 4: Use BAR4 for Flash access
 *
 *	req->adrs : register address
 *	req->ptr : pointer to data
 *
 * \retval 0: Success, -1 : Failure
 */
static int DWC_ETH_QOS_RD_WR_REG(struct DWC_ETH_QOS_prv_data *pdata, struct ifr_data_struct *req)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned int ret = -1;
	unsigned int data = 0;

	do
	{
		if((req->flags == NTN_REG_WR)||(req->flags == NTN_PCIE_CONFIG_REG_WR))
		{
			if(copy_from_user(&data, (unsigned int*)req->ptr, sizeof(unsigned int)))
			{
				NMSGPR_ALERT( "Failed to copy write data value\n");
				ret = -EFAULT;
				break;
			}
		}

		switch(req->flags)
		{
			case NTN_REG_RD: /* Neutrino Register Read */
				data = hw_if->ntn_reg_rd(req->adrs, req->bar_num, pdata);
				ret = 0;
				break;
			case NTN_REG_WR: /* Neutrino Register Write */
				hw_if->ntn_reg_wr(req->adrs, data, req->bar_num, pdata);
				ret = 0;
				break;
			case NTN_PCIE_CONFIG_REG_RD: /* PCIe config Register Read */
				pci_read_config_dword(pdata->pdev, req->adrs, &data);
				ret = 0;
				break;
			case NTN_PCIE_CONFIG_REG_WR: /* PCIe config Register Write */
				pci_write_config_dword(pdata->pdev, req->adrs, data);
				ret = 0;
				break;
			default:
				ret = -1;
				break;
		}

		if((ret == 0) && ((req->flags == NTN_REG_RD)||(req->flags == NTN_PCIE_CONFIG_REG_RD)))
		{
			if(copy_to_user((unsigned int*)req->ptr, &data, sizeof(unsigned int)))
			{
				NMSGPR_ALERT( "Failed to copy read data value\n");
				ret = -EFAULT;
			}
		}
	}while(0);

	return ret;
}


/*!
 * \brief This function confiures the TDM path for TDM-Ethernet data transfer.
 * \param[in] pdata : pointer to private data structure.
 * \param[in] req : pointer to ioctl structure.
 *
 * \retval 0: Success, -1 : Failure
 **/
static int NTN_TDM_Config(struct DWC_ETH_QOS_prv_data *pdata, struct ifr_data_struct *req)
{
        struct NTN_TDM_Config *ntn_tdm_cfg = (struct NTN_TDM_Config *)req->ptr;
        struct hw_if_struct *hw_if = &pdata->hw_if;
        unsigned int ret = -1;
	long unsigned int reg_val;
	unsigned char* ptr;
	unsigned int int_mask_val;
	int i;

		//Assert TDM reset
		reg_val = hw_if->ntn_reg_rd(0x1008, 0, pdata);
		reg_val |= 0x1<<6;
		hw_if->ntn_reg_wr(0x1008, reg_val, 0, pdata);

	/* Stop TDM */
	if(ntn_tdm_cfg->tdm_start == 0){
     	NMSGPR_INFO("Disabled TDM path\n");

		//Disable TDM clock
		reg_val = hw_if->ntn_reg_rd(0x1004, 0, pdata);
		reg_val &= ~(0x1<<6);
		hw_if->ntn_reg_wr(0x1004, reg_val, 0, pdata);

		/* Disable TDM interrupt in INTC Module */
		NTN_INTC_INTMCUMASK1_RgRd(int_mask_val);
		int_mask_val |= (0x7F<<22);	//Disable all TDM interrupts
		NTN_INTC_INTMCUMASK1_RgWr(int_mask_val);
		return 0;
	}

	/* Enable TDM interrupt in INTC Module */
	NTN_INTC_INTMCUMASK1_RgRd(int_mask_val);
	int_mask_val &= ~(0x7F<<22); //Enable all interrupts
	NTN_INTC_INTMCUMASK1_RgWr(int_mask_val);

	//Enable TDM clock
	reg_val = hw_if->ntn_reg_rd(0x1004, 0, pdata);
	reg_val |= 0x1<<6;
	hw_if->ntn_reg_wr(0x1004, reg_val, 0, pdata);

	//Deassert TDM reset
	reg_val = hw_if->ntn_reg_rd(0x1008, 0, pdata);
	reg_val &= ~(0x1<<6);
	hw_if->ntn_reg_wr(0x1008, reg_val, 0, pdata);

	/* Start TDM */
	NMSGPR_INFO("TDM Path Configuration\n");
	NMSGPR_INFO("    Sample rate = %d\n", ntn_tdm_cfg->sample_rate);
	NMSGPR_INFO("    No of channels = %d\n", ntn_tdm_cfg->channels);
	NMSGPR_INFO("    Mode selected  = %d\n", ntn_tdm_cfg->mode_sel);
	NMSGPR_INFO("    Protocol selected = %d\n", ntn_tdm_cfg->protocol);
	NMSGPR_INFO("    Class priority  = %d\n", ntn_tdm_cfg->a_priority);
	NMSGPR_INFO("    Direction selected = %d\n", ntn_tdm_cfg->direction);
	NMSGPR_INFO("    Class vid = %d\n", ntn_tdm_cfg->a_vid);

	switch(ntn_tdm_cfg->sample_rate){
	case 48000:
		hw_if->ntn_reg_wr(0x449c, 0x02dc6c08, 0, pdata);	//TDM DDA Control (M value)
		hw_if->ntn_reg_wr(0x44a0, 0x00000C00, 0, pdata);   	//T0EVB_DDANRatio (N value)
#ifdef NTN_DRV_TEST_LOOPBACK
		hw_if->ntn_reg_wr(0x44e0, 0x0000760C, 0, pdata);	//T0EVB_DDACtrl2  (PLL output divider)
#else
		hw_if->ntn_reg_wr(0x44e0, 0x0000060C, 0, pdata);	//T0EVB_DDACtrl2  (PLL output divider)
#endif
		hw_if->ntn_reg_wr(0x44e4, 0x01fff700, 0, pdata);	//DDA_PLL_Ctrl1
		hw_if->ntn_reg_wr(0x44e8, 0x0000f300, 0, pdata);	//DDA_PLL_Ctrl2  (BCLK and MCLK divider)
		hw_if->ntn_reg_wr(0x44ec, 0x00000000, 0, pdata);	//DDA_PLL_UPDT
		ntn_tdm_cfg->fdf = 2;
       	break;
	case 44100:
		hw_if->ntn_reg_wr(0x449c, 0x02dc6c08, 0, pdata);	//TDM DDA Control (M value)
		hw_if->ntn_reg_wr(0x44a0, 0x00006E40, 0, pdata);   	//T0EVB_DDANRatio (N value)
		hw_if->ntn_reg_wr(0x44e0, 0x0001060C, 0, pdata);	//T0EVB_DDACtrl2  (PLL output divider)
		hw_if->ntn_reg_wr(0x44e4, 0x01fff700, 0, pdata);	//DDA_PLL_Ctrl1
		hw_if->ntn_reg_wr(0x44e8, 0x0000f300, 0, pdata);	//DDA_PLL_Ctrl2  (BCLK and MCLK divider)
		hw_if->ntn_reg_wr(0x44ec, 0x00000000, 0, pdata);	//DDA_PLL_UPDT
		ntn_tdm_cfg->fdf = 1;
	break;
//	case 96000:
//		hw_if->ntn_reg_wr(0x449c, 0x02dc6c08, 0, pdata);	//TDM DDA Control (M value)
//		hw_if->ntn_reg_wr(0x44a0, 0x00000C00, 0, pdata);   	//T0EVB_DDANRatio (N value)
//		hw_if->ntn_reg_wr(0x44e0, 0x0000060C, 0, pdata);	//T0EVB_DDACtrl2  (PLL output divider)
//		hw_if->ntn_reg_wr(0x44e4, 0x00FFB700, 0, pdata);	//DDA_PLL_Ctrl1
//		hw_if->ntn_reg_wr(0x44e8, 0x0000f300, 0, pdata);	//DDA_PLL_Ctrl2  ( BCLK and MCLK divider )
//	break;
	default: break;
	}

	/*Clock & Reset*/
	hw_if->ntn_reg_wr(0x0010, 0x3030008, 0, pdata);		//Neutrino eMAC DIV

	/*EMAC RX*/
	/* TDM Source MAC ID */
        ptr = ntn_tdm_cfg->TDM_SRC_ID;
//        NMSGPR_INFO("SRC: \n");
//        for(i=0;i<sizeof(ntn_tdm_cfg->TDM_SRC_ID); i++)
//                NMSGPR_INFO("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_SRC_ID[i]);

        reg_val = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
	hw_if->ntn_reg_wr(0x3010, reg_val, 0, pdata);		//TDM Header Ethernet MAC Source Low Address Register
        reg_val = (ptr[5] << 8) | (ptr[4] << 0);
	hw_if->ntn_reg_wr(0x3014, reg_val, 0, pdata);		//TDM Header Ethernet MAC Source High Address Register

#ifdef NTN_DRV_TEST_LOOPBACK
	hw_if->ntn_reg_wr(0xab98, 0x00003D08, 0, pdata);	//MAC_PPS0_Width : 125 us
#else
	hw_if->ntn_reg_wr(0xab98, 0x0000186a, 0, pdata);	//MAC_PPS0_Width : 125 us
#endif
	hw_if->ntn_reg_wr(0xab70, 0x00000212, 0, pdata);       //MAC_PPS_Control : Set for only PPS0 output


	/*TDM*/
	if(ntn_tdm_cfg->direction == NTN_TDM_IN)
	{
	hw_if->ntn_reg_wr(0x4414, 0x08100058, 0, pdata);		//TDM conf0

	reg_val = hw_if->ntn_reg_rd(0x3000, 0, pdata);
	reg_val |= (ntn_tdm_cfg->a_priority&0x7)<<2;
	reg_val |= (ntn_tdm_cfg->fdf&0xFF)<<8;
	hw_if->ntn_reg_wr(0x3000, reg_val, 0, pdata);			//TDM Control Register

   	/* config stream id */
        ptr = ntn_tdm_cfg->TDM_STREAM_ID;
		        NMSGPR_INFO("Stream: \n");
		        for(i=0;i<sizeof(ntn_tdm_cfg->TDM_STREAM_ID); i++)
		                NMSGPR_INFO("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_STREAM_ID[i]);

        reg_val = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
	hw_if->ntn_reg_wr(0x4418, reg_val, 0, pdata);		//TDM Stream ID low
        reg_val = (ptr[7] << 24) | (ptr[6] << 16) | (ptr[5] << 8) | ptr[4];
	hw_if->ntn_reg_wr(0x441c, reg_val, 0, pdata);		//TDM Stream ID hi

	/* TDM Destination MAC ID */
        ptr = ntn_tdm_cfg->TDM_DST_ID;
//        NMSGPR_INFO("DST: \n");
//        for(i=0;i<sizeof(ntn_tdm_cfg->TDM_DST_ID); i++)
//                NMSGPR_INFO("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_DST_ID[i]);

        reg_val = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
	hw_if->ntn_reg_wr(0x4420, reg_val, 0, pdata);		//TDM MAC low
        reg_val = (ptr[5] << 8) | (ptr[4] << 0);
	hw_if->ntn_reg_wr(0x4424, reg_val, 0, pdata);		//TDM MAC hi

	reg_val = 0x40005800;
	reg_val |= (ntn_tdm_cfg->channels & 0xF)<<16;
	reg_val |= (ntn_tdm_cfg->protocol & 0x1)<<15;
	hw_if->ntn_reg_wr(0x4410, reg_val, 0, pdata);			//TDM Control Register

	//reg_val = hw_if->ntn_reg_rd(0x4400, 0, pdata);			//TDM Stream 0 Control register
	reg_val = 0x2003072C;
	reg_val |= (ntn_tdm_cfg->mode_sel & 0x1)<<31;
	hw_if->ntn_reg_wr(0x4400, reg_val, 0, pdata);			//TDM Control Register

#ifdef NTN_DRV_TEST_LOOPBACK
	hw_if->ntn_reg_wr(0x100C, 0x00002600, 0, pdata);	//Pin Mux control
#else
	hw_if->ntn_reg_wr(0x100C, 0x00002A00, 0, pdata);	//Pin Mux control
#endif
		NMSGPR_INFO("TDM Out configured\n");

#ifndef NTN_DRV_TEST_LOOPBACK
	}
	else
	{
#endif

		/* Disable VLAN striping */
                pdata->dev_state &= ~NETIF_F_HW_VLAN_CTAG_RX;
                hw_if->config_rx_outer_vlan_stripping(DWC_ETH_QOS_RX_NO_VLAN_STRIP, pdata);
#ifdef NTN_DRV_TEST_LOOPBACK
		hw_if->ntn_reg_wr(0x100C, 0x00002600, 0, pdata);	//Pin Mux control
		hw_if->ntn_reg_wr(0x4490, 0x01F00213, 0, pdata);	//Pin Mux control
		hw_if->ntn_reg_wr(0x44b8, 0x00000000, 0, pdata);	//Pin Mux control
#else
		hw_if->ntn_reg_wr(0x100C, 0x00002A00, 0, pdata);	//Pin Mux control
		hw_if->ntn_reg_wr(0x4490, 0x01F00213, 0, pdata);	//Pin Mux control
		hw_if->ntn_reg_wr(0x44b8, 0x0006e61e, 0, pdata);	//Pin Mux control
		reg_val = 0x0003072C;
		reg_val |= (ntn_tdm_cfg->mode_sel & 0x1)<<31;
		hw_if->ntn_reg_wr(0x4400, reg_val, 0, pdata);			//TDM Control Register
#endif
		/* config stream id */
		ptr = ntn_tdm_cfg->TDM_STREAM_ID;
		NMSGPR_INFO("Stream: \n");
		for(i=0;i<sizeof(ntn_tdm_cfg->TDM_STREAM_ID); i++)
			NMSGPR_INFO("%x : %x \n",ptr[i], ntn_tdm_cfg->TDM_STREAM_ID[i]);

		reg_val = (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
		hw_if->ntn_reg_wr(0x3084, reg_val, 0, pdata);		//TDM Stream ID hi
		reg_val = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
		hw_if->ntn_reg_wr(0x3088, reg_val, 0, pdata);		//TDM Stream ID low

		hw_if->ntn_reg_wr(0x44CC, 0x00CC0133, 0, pdata);	//TDM buffer near underflow threshold register
		hw_if->ntn_reg_wr(0x44D0, 0x02CC0333, 0, pdata);	//TDM buffer near overflow threshold register

#ifdef NTN_DRV_TEST_LOOPBACK
		reg_val = 0x6003072C;
		reg_val |= (ntn_tdm_cfg->mode_sel & 0x1)<<31;
		hw_if->ntn_reg_wr(0x4400, reg_val, 0, pdata);		//TDM Control Register
#endif
		NMSGPR_INFO("TDM In configured\n");
	}

	ret = 0;
        return ret;
}

static int DWC_ETH_QOS_handle_prv_ioctl_ipa(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifreq *ifr)
{
		struct ifr_data_struct_ipa *ipa_ioctl_data;
		int ret = 0;
		int chInx_tx_ipa, chInx_rx_ipa;
		unsigned long missing;

		DBGPR("-->DWC_ETH_QOS_handle_prv_ioctl_ipa\n");

		if ( !ifr || !ifr->ifr_ifru.ifru_data  )
			   return -EINVAL;

		ipa_ioctl_data = kzalloc(sizeof(struct ifr_data_struct_ipa), GFP_KERNEL);
		if (!ipa_ioctl_data)
			   return -ENOMEM;

		missing = copy_from_user(ipa_ioctl_data, ifr->ifr_ifru.ifru_data, sizeof(struct ifr_data_struct_ipa));
		if (missing)
		   return -EFAULT;

		chInx_tx_ipa = ipa_ioctl_data->chInx_tx_ipa;
		chInx_rx_ipa = ipa_ioctl_data->chInx_rx_ipa;

		if ( (chInx_tx_ipa != NTN_TX_DMA_CH_2) ||
			(chInx_rx_ipa != NTN_RX_DMA_CH_0) )
		{
			NMSGPR_ERR("the RX/TX channels passed are not owned by IPA,correct channels to \
				pass TX: %d RX: %d \n",NTN_TX_DMA_CH_2,NTN_RX_DMA_CH_0);
			return DWC_ETH_QOS_CONFIG_FAIL;
		}

		switch ( ipa_ioctl_data->cmd ){

		case DWC_ETH_QOS_IPA_VLAN_ENABLE_CMD:
		   if (!pdata->prv_ipa.vlan_enable) {
			   if (ipa_ioctl_data->vlan_id > MIN_VLAN_ID && ipa_ioctl_data->vlan_id <= MAX_VLAN_ID){
				   pdata->prv_ipa.vlan_id = ipa_ioctl_data->vlan_id;
				   ret = DWC_ETH_QOS_disable_enable_ipa_offload(pdata,chInx_tx_ipa,chInx_rx_ipa);
				   if (!ret)
					pdata->prv_ipa.vlan_enable = true;
		   }
		   else
			NMSGPR_ERR("INVALID VLAN-ID: %d passed in the IOCTL cmd \n",ipa_ioctl_data->vlan_id);
		   }
		   break;
		case DWC_ETH_QOS_IPA_VLAN_DISABLE_CMD:
			if (pdata->prv_ipa.vlan_enable) {
				pdata->prv_ipa.vlan_id = 0;
				ret = DWC_ETH_QOS_disable_enable_ipa_offload(pdata,chInx_tx_ipa,chInx_rx_ipa);
				if (!ret)
					pdata->prv_ipa.vlan_enable = false;
			}
		   break;

		default:
		   ret = -EOPNOTSUPP;
		   NMSGPR_ERR( "Unsupported IPA IOCTL call\n");
		}
		return ret;
}
/*!
 * \brief Driver IOCTL routine
 *
 * \details This function is invoked by main ioctl function when
 * users request to configure various device features like,
 * PMT module, TX and RX PBL, TX and RX FIFO threshold level,
 * TX and RX OSF mode, SA insert/replacement, L2/L3/L4 and
 * VLAN filtering, AVB/DCB algorithm etc.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] req – pointer to ioctl structure.
 *
 * \return int
 *
 * \retval 0 - success
 * \retval negative - failure
 */

static int DWC_ETH_QOS_handle_prv_ioctl(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifr_data_struct *req)
{
	unsigned int chInx = req->chInx;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data =
	    GET_TX_WRAPPER_DESC(chInx);
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data =
	    GET_RX_WRAPPER_DESC(chInx);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct net_device *dev = pdata->dev;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_handle_prv_ioctl\n");

	if (chInx > NTN_QUEUE_CNT) {
		NMSGPR_ALERT( "Queue number %d is invalid\n" \
				"Hardware has only %d Tx/Rx Queues\n",
				chInx, NTN_QUEUE_CNT);
		ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		return ret;
	}

	switch (req->cmd) {
	case DWC_ETH_QOS_POWERUP_MAGIC_CMD:
		if (pdata->hw_feat.mgk_sel) {
			ret = DWC_ETH_QOS_powerup(dev, DWC_ETH_QOS_IOCTL_CONTEXT);
			if (ret == 0)
				ret = DWC_ETH_QOS_CONFIG_SUCCESS;
			else
				ret = DWC_ETH_QOS_CONFIG_FAIL;
		} else {
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_POWERDOWN_MAGIC_CMD:
		if (pdata->hw_feat.mgk_sel) {
			ret =
			  DWC_ETH_QOS_powerdown(dev,
			    DWC_ETH_QOS_MAGIC_WAKEUP, DWC_ETH_QOS_IOCTL_CONTEXT);
			if (ret == 0)
				ret = DWC_ETH_QOS_CONFIG_SUCCESS;
			else
				ret = DWC_ETH_QOS_CONFIG_FAIL;
		} else {
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_POWERUP_REMOTE_WAKEUP_CMD:
		if (pdata->hw_feat.rwk_sel) {
			ret = DWC_ETH_QOS_powerup(dev, DWC_ETH_QOS_IOCTL_CONTEXT);
			if (ret == 0)
				ret = DWC_ETH_QOS_CONFIG_SUCCESS;
			else
				ret = DWC_ETH_QOS_CONFIG_FAIL;
		} else {
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_POWERDOWN_REMOTE_WAKEUP_CMD:
		if (pdata->hw_feat.rwk_sel) {
			ret = DWC_ETH_QOS_configure_remotewakeup(dev, req);
			if (ret == 0)
				ret = DWC_ETH_QOS_CONFIG_SUCCESS;
			else
				ret = DWC_ETH_QOS_CONFIG_FAIL;
		} else {
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_RX_THRESHOLD_CMD:
		rx_desc_data->rx_threshold_val = req->flags;
		hw_if->config_rx_threshold(chInx,
					rx_desc_data->rx_threshold_val, pdata);
		NMSGPR_ALERT( "Configured Rx threshold with %d\n",
		       rx_desc_data->rx_threshold_val);
		break;

	case DWC_ETH_QOS_TX_THRESHOLD_CMD:
		tx_desc_data->tx_threshold_val = req->flags;
		hw_if->config_tx_threshold(chInx,
					tx_desc_data->tx_threshold_val, pdata);
		NMSGPR_ALERT( "Configured Tx threshold with %d\n",
		       tx_desc_data->tx_threshold_val);
		break;

	case DWC_ETH_QOS_RSF_CMD:
		rx_desc_data->rsf_on = req->flags;
		hw_if->config_rsf_mode(chInx, rx_desc_data->rsf_on, pdata);
		NMSGPR_ALERT( "Receive store and forward mode %s\n",
		       (rx_desc_data->rsf_on) ? "enabled" : "disabled");
		break;

	case DWC_ETH_QOS_TSF_CMD:
		tx_desc_data->tsf_on = req->flags;
		hw_if->config_tsf_mode(chInx, tx_desc_data->tsf_on, pdata);
		NMSGPR_ALERT( "Transmit store and forward mode %s\n",
		       (tx_desc_data->tsf_on) ? "enabled" : "disabled");
		break;

	case DWC_ETH_QOS_OSF_CMD:
		tx_desc_data->osf_on = req->flags;
		hw_if->config_osf_mode(chInx, tx_desc_data->osf_on, pdata);
		NMSGPR_ALERT( "Transmit DMA OSF mode is %s\n",
		       (tx_desc_data->osf_on) ? "enabled" : "disabled");
		break;

	case DWC_ETH_QOS_INCR_INCRX_CMD:
		pdata->incr_incrx = req->flags;
		hw_if->config_incr_incrx_mode(pdata->incr_incrx, pdata);
		NMSGPR_ALERT( "%s mode is enabled\n",
		       (pdata->incr_incrx) ? "INCRX" : "INCR");
		break;

	case DWC_ETH_QOS_RX_PBL_CMD:
		rx_desc_data->rx_pbl = req->flags;
		DWC_ETH_QOS_config_rx_pbl(pdata, rx_desc_data->rx_pbl, chInx);
		break;

	case DWC_ETH_QOS_TX_PBL_CMD:
		tx_desc_data->tx_pbl = req->flags;
		DWC_ETH_QOS_config_tx_pbl(pdata, tx_desc_data->tx_pbl, chInx);
		break;

#ifdef DWC_ETH_QOS_ENABLE_DVLAN
	case DWC_ETH_QOS_DVLAN_TX_PROCESSING_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			ret = DWC_ETH_QOS_config_tx_dvlan_processing(pdata, req);
		} else {
			NMSGPR_ALERT( "No HW support for Single/Double VLAN\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;
	case DWC_ETH_QOS_DVLAN_RX_PROCESSING_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			ret = DWC_ETH_QOS_config_rx_dvlan_processing(pdata, req->flags);
		} else {
			NMSGPR_ALERT( "No HW support for Single/Double VLAN\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;
	case DWC_ETH_QOS_SVLAN_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			ret = DWC_ETH_QOS_config_svlan(pdata, req->flags);
		} else {
			NMSGPR_ALERT( "No HW support for Single/Double VLAN\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;
#endif /* end of DWC_ETH_QOS_ENABLE_DVLAN */
	case DWC_ETH_QOS_PTPOFFLOADING_CMD:
		if (pdata->hw_feat.tsstssel) {
			ret = DWC_ETH_QOS_config_ptpoffload(pdata,
					req->ptr);
		} else {
			NMSGPR_ALERT( "No HW support for PTP\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_SA0_DESC_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			pdata->tx_sa_ctrl_via_desc = req->flags;
			pdata->tx_sa_ctrl_via_reg = DWC_ETH_QOS_SA0_NONE;
			if (req->flags == DWC_ETH_QOS_SA0_NONE) {
				memcpy(pdata->mac_addr, pdata->dev->dev_addr,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			} else {
				memcpy(pdata->mac_addr, mac_addr0,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			}
			hw_if->configure_mac_addr0_reg(pdata->mac_addr, pdata);
			hw_if->configure_sa_via_reg(pdata->tx_sa_ctrl_via_reg, pdata);
			NMSGPR_ALERT(
			       "SA will use MAC0 with descriptor for configuration %d\n",
			       pdata->tx_sa_ctrl_via_desc);
		} else {
			NMSGPR_ALERT(
			       "Device doesn't supports SA Insertion/Replacement\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_SA1_DESC_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			pdata->tx_sa_ctrl_via_desc = req->flags;
			pdata->tx_sa_ctrl_via_reg = DWC_ETH_QOS_SA1_NONE;
			if (req->flags == DWC_ETH_QOS_SA1_NONE) {
				memcpy(pdata->mac_addr, pdata->dev->dev_addr,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			} else {
				memcpy(pdata->mac_addr, mac_addr1,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			}
			hw_if->configure_mac_addr1_reg(pdata->mac_addr, pdata);
			hw_if->configure_sa_via_reg(pdata->tx_sa_ctrl_via_reg, pdata);
			NMSGPR_ALERT(
			       "SA will use MAC1 with descriptor for configuration %d\n",
			       pdata->tx_sa_ctrl_via_desc);
		} else {
			NMSGPR_ALERT(
			       "Device doesn't supports SA Insertion/Replacement\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_SA0_REG_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			pdata->tx_sa_ctrl_via_reg = req->flags;
			pdata->tx_sa_ctrl_via_desc = DWC_ETH_QOS_SA0_NONE;
			if (req->flags == DWC_ETH_QOS_SA0_NONE) {
				memcpy(pdata->mac_addr, pdata->dev->dev_addr,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			} else {
				memcpy(pdata->mac_addr, mac_addr0,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			}
			hw_if->configure_mac_addr0_reg(pdata->mac_addr, pdata);
			hw_if->configure_sa_via_reg(pdata->tx_sa_ctrl_via_reg, pdata);
			NMSGPR_ALERT(
			       "SA will use MAC0 with register for configuration %d\n",
			       pdata->tx_sa_ctrl_via_desc);
		} else {
			NMSGPR_ALERT(
			       "Device doesn't supports SA Insertion/Replacement\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_SA1_REG_CMD:
		if (pdata->hw_feat.sa_vlan_ins) {
			pdata->tx_sa_ctrl_via_reg = req->flags;
			pdata->tx_sa_ctrl_via_desc = DWC_ETH_QOS_SA1_NONE;
			if (req->flags == DWC_ETH_QOS_SA1_NONE) {
				memcpy(pdata->mac_addr, pdata->dev->dev_addr,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			} else {
				memcpy(pdata->mac_addr, mac_addr1,
				       DWC_ETH_QOS_MAC_ADDR_LEN);
			}
			hw_if->configure_mac_addr1_reg(pdata->mac_addr, pdata);
			hw_if->configure_sa_via_reg(pdata->tx_sa_ctrl_via_reg, pdata);
			NMSGPR_ALERT(
			       "SA will use MAC1 with register for configuration %d\n",
			       pdata->tx_sa_ctrl_via_desc);
		} else {
			NMSGPR_ALERT(
			       "Device doesn't supports SA Insertion/Replacement\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_SETUP_CONTEXT_DESCRIPTOR:
		if (pdata->hw_feat.sa_vlan_ins) {
			tx_desc_data->context_setup = req->context_setup;
			if (tx_desc_data->context_setup == 1) {
				NMSGPR_ALERT( "Context descriptor will be transmitted"\
						" with every normal descriptor on %d DMA Channel\n",
						chInx);
			}
			else {
				NMSGPR_ALERT( "Context descriptor will be setup"\
						" only if VLAN id changes %d\n", chInx);
			}
		}
		else {
			NMSGPR_ALERT(
			       "Device doesn't support VLAN operations\n");
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;

	case DWC_ETH_QOS_GET_RX_QCNT:
		req->chInx = NTN_RX_QUEUE_CNT;
		break;

	case DWC_ETH_QOS_GET_TX_QCNT:
		req->chInx = NTN_TX_QUEUE_CNT;
		break;

	case DWC_ETH_QOS_GET_CONNECTED_SPEED:
		req->connected_speed = pdata->speed;
		break;

	case DWC_ETH_QOS_DCB_ALGORITHM:
		DWC_ETH_QOS_program_dcb_algorithm(pdata, req);
		break;

	case DWC_ETH_QOS_AVB_ALGORITHM:
		DWC_ETH_QOS_program_avb_algorithm(pdata, req);
		break;

	case DWC_ETH_QOS_RX_SPLIT_HDR_CMD:
		ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		break;
	case DWC_ETH_QOS_L3_L4_FILTER_CMD:
		if (pdata->hw_feat.l3l4_filter_num > 0) {
			ret = DWC_ETH_QOS_config_l3_l4_filtering(dev, req->flags);
			if (ret == 0)
				ret = DWC_ETH_QOS_CONFIG_SUCCESS;
			else
				ret = DWC_ETH_QOS_CONFIG_FAIL;
		} else {
			ret = DWC_ETH_QOS_NO_HW_SUPPORT;
		}
		break;
	case DWC_ETH_QOS_IPV4_FILTERING_CMD:
		ret = DWC_ETH_QOS_config_ip4_filters(dev, req);
		break;
	case DWC_ETH_QOS_IPV6_FILTERING_CMD:
		ret = DWC_ETH_QOS_config_ip6_filters(dev, req);
		break;
	case DWC_ETH_QOS_UDP_FILTERING_CMD:
		ret = DWC_ETH_QOS_config_tcp_udp_filters(dev, req, 1);
		break;
	case DWC_ETH_QOS_TCP_FILTERING_CMD:
		ret = DWC_ETH_QOS_config_tcp_udp_filters(dev, req, 0);
		break;
	case DWC_ETH_QOS_VLAN_FILTERING_CMD:
		ret = DWC_ETH_QOS_config_vlan_filter(dev, req);
		break;
	case DWC_ETH_QOS_L2_DA_FILTERING_CMD:
		ret = DWC_ETH_QOS_confing_l2_da_filter(dev, req);
		break;
	case DWC_ETH_QOS_ARP_OFFLOAD_CMD:
		ret = DWC_ETH_QOS_config_arp_offload(dev, req);
		break;
	case DWC_ETH_QOS_AXI_PBL_CMD:
		pdata->axi_pbl = req->flags;
		hw_if->config_axi_pbl_val(pdata->axi_pbl, pdata);
		NMSGPR_ALERT( "AXI PBL value: %d\n", pdata->axi_pbl);
		break;
	case DWC_ETH_QOS_AXI_WORL_CMD:
		pdata->axi_worl = req->flags;
		hw_if->config_axi_worl_val(pdata->axi_worl, pdata);
		NMSGPR_ALERT( "AXI WORL value: %d\n", pdata->axi_worl);
		break;
	case DWC_ETH_QOS_AXI_RORL_CMD:
		pdata->axi_rorl = req->flags;
		hw_if->config_axi_rorl_val(pdata->axi_rorl, pdata);
		NMSGPR_ALERT( "AXI RORL value: %d\n", pdata->axi_rorl);
		break;
	case DWC_ETH_QOS_MAC_LOOPBACK_MODE_CMD:
		ret = DWC_ETH_QOS_config_mac_loopback_mode(dev, req->flags);
		if (ret == 0)
			ret = DWC_ETH_QOS_CONFIG_SUCCESS;
		else
			ret = DWC_ETH_QOS_CONFIG_FAIL;
		break;
	case DWC_ETH_QOS_PHY_LOOPBACK_MODE_CMD:
		ret = DWC_ETH_QOS_config_phy_loopback_mode(dev, req->flags);
		if (ret == 0)
			ret = DWC_ETH_QOS_CONFIG_SUCCESS;
		else
			ret = DWC_ETH_QOS_CONFIG_FAIL;
		break;
	case DWC_ETH_QOS_PFC_CMD:
		ret = DWC_ETH_QOS_config_pfc(dev, req->flags);
		break;
	case NTN_DWC_REG_RD_WR_CMD:
		ret = DWC_ETH_QOS_RD_WR_REG(pdata, (void*)req);
		break;
    case DWC_WRAP_TS_FEATURE:
        hw_if->ntn_wrap_ts_ignore_config(req->flags, pdata);
        NMSGPR_ALERT( "Neutrino Wrapper TS Feature Value: %d\n", req->flags);
        break;
	case NTN_DWC_GET_FREE_TX_DESC_CNT_CMD:
                *(unsigned int*)(req->ptr) = tx_desc_data->free_desc_cnt;
		ret = DWC_ETH_QOS_CONFIG_SUCCESS;
		break;
	case NTN_DWC_TDM_CONFIG_CMD:
		ret = NTN_TDM_Config(pdata, (void*)req);
		break;

	default:
		ret = -EOPNOTSUPP;
		NMSGPR_ALERT( "Unsupported command call\n");
	}

	DBGPR("<--DWC_ETH_QOS_handle_prv_ioctl\n");

	return ret;
}


/*!
 * \brief control hw timestamping.
 *
 * \details This function is used to configure the MAC to enable/disable both
 * outgoing(Tx) and incoming(Rx) packets time stamping based on user input.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] ifr – pointer to IOCTL specific structure.
 *
 * \return int
 *
 * \retval 0 - success
 * \retval negative - failure
 */

static int DWC_ETH_QOS_handle_hwtstamp_ioctl(struct DWC_ETH_QOS_prv_data *pdata,
	struct ifreq *ifr)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct hwtstamp_config config;
	u32 ptp_v2 = 0;
	u32 tstamp_all = 0;
	u32 ptp_over_ipv4_udp = 0;
	u32 ptp_over_ipv6_udp = 0;
	u32 ptp_over_ethernet = 0;
	u32 snap_type_sel = 0;
	u32 ts_master_en = 0;
	u32 ts_event_en = 0;
	u32 av_8021asm_en = 0;
	u32 varMAC_TCR = 0;
	u64 temp = 0;
	struct timespec now;

	DBGPR_PTP("-->DWC_ETH_QOS_handle_hwtstamp_ioctl\n");

	if (!pdata->hw_feat.tsstssel) {
		NMSGPR_ALERT( "No hw timestamping is available in this core\n");
		return -EOPNOTSUPP;
	}

	if (copy_from_user(&config, ifr->ifr_data,
		sizeof(struct hwtstamp_config)))
		return -EFAULT;

	DBGPR_PTP("config.flags = %#x, tx_type = %#x, rx_filter = %#x\n",
		config.flags, config.tx_type, config.rx_filter);

	/* reserved for future extensions */
	if (config.flags)
		return -EINVAL;

	switch (config.tx_type) {
	case HWTSTAMP_TX_OFF:
		pdata->hwts_tx_en = 0;
		break;
	case HWTSTAMP_TX_ON:
		pdata->hwts_tx_en = 1;
		break;
	default:
		return -ERANGE;
	}

	switch (config.rx_filter) {
	/* time stamp no incoming packet at all */
	case HWTSTAMP_FILTER_NONE:
		config.rx_filter = HWTSTAMP_FILTER_NONE;
		break;

	/* PTP v1, UDP, any kind of event packet */
	case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_EVENT;
		/* take time stamp for all event messages */
		snap_type_sel = MAC_TCR_SNAPTYPSEL_1;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		break;

	/* PTP v1, UDP, Sync packet */
	case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_SYNC;
		/* take time stamp for SYNC messages only */
		ts_event_en = MAC_TCR_TSEVENTENA;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		break;

	/* PTP v1, UDP, Delay_req packet */
	case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ;
		/* take time stamp for Delay_Req messages only */
		ts_master_en = MAC_TCR_TSMASTERENA;
		ts_event_en = MAC_TCR_TSEVENTENA;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		break;

	/* PTP v2, UDP, any kind of event packet */
	case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_EVENT;
		ptp_v2 = MAC_TCR_TSVER2ENA;
		/* take time stamp for all event messages */
		snap_type_sel = MAC_TCR_SNAPTYPSEL_1;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		break;

	/* PTP v2, UDP, Sync packet */
	case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_SYNC;
		ptp_v2 = MAC_TCR_TSVER2ENA;
		/* take time stamp for SYNC messages only */
		ts_event_en = MAC_TCR_TSEVENTENA;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		break;

	/* PTP v2, UDP, Delay_req packet */
	case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ;
		ptp_v2 = MAC_TCR_TSVER2ENA;
		/* take time stamp for Delay_Req messages only */
		ts_master_en = MAC_TCR_TSMASTERENA;
		ts_event_en = MAC_TCR_TSEVENTENA;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		break;

	/* PTP v2/802.AS1, any layer, any kind of event packet */
	case HWTSTAMP_FILTER_PTP_V2_EVENT:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
		ptp_v2 = MAC_TCR_TSVER2ENA;
		/* take time stamp for all event messages */
		snap_type_sel = MAC_TCR_SNAPTYPSEL_1;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		ptp_over_ethernet = MAC_TCR_TSIPENA;
		av_8021asm_en = MAC_TCR_AV8021ASMEN;
		break;

	/* PTP v2/802.AS1, any layer, Sync packet */
	case HWTSTAMP_FILTER_PTP_V2_SYNC:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_SYNC;
		ptp_v2 = MAC_TCR_TSVER2ENA;
		/* take time stamp for SYNC messages only */
		ts_event_en = MAC_TCR_TSEVENTENA;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		ptp_over_ethernet = MAC_TCR_TSIPENA;
		av_8021asm_en = MAC_TCR_AV8021ASMEN;
		break;

	/* PTP v2/802.AS1, any layer, Delay_req packet */
	case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_DELAY_REQ;
		ptp_v2 = MAC_TCR_TSVER2ENA;
		/* take time stamp for Delay_Req messages only */
		ts_master_en = MAC_TCR_TSMASTERENA;
		ts_event_en = MAC_TCR_TSEVENTENA;

		ptp_over_ipv4_udp = MAC_TCR_TSIPV4ENA;
		ptp_over_ipv6_udp = MAC_TCR_TSIPV6ENA;
		ptp_over_ethernet = MAC_TCR_TSIPENA;
		av_8021asm_en = MAC_TCR_AV8021ASMEN;
		break;

	/* time stamp any incoming packet */
	case HWTSTAMP_FILTER_ALL:
		config.rx_filter = HWTSTAMP_FILTER_ALL;
		tstamp_all = MAC_TCR_TSENALL;
		break;

	default:
		return -ERANGE;
	}
	pdata->hwts_rx_en = ((config.rx_filter == HWTSTAMP_FILTER_NONE) ? 0 : 1);

	if (!pdata->hwts_tx_en && !pdata->hwts_rx_en) {
		/* disable hw time stamping */
		hw_if->config_hw_time_stamping(varMAC_TCR, pdata);
	} else {
		varMAC_TCR = (MAC_TCR_TSENA | MAC_TCR_TSCFUPDT | MAC_TCR_TSCTRLSSR |
				tstamp_all | ptp_v2 | ptp_over_ethernet | ptp_over_ipv6_udp |
				ptp_over_ipv4_udp | ts_event_en | ts_master_en |
				snap_type_sel | av_8021asm_en);

		if (!pdata->one_nsec_accuracy)
			varMAC_TCR &= ~MAC_TCR_TSCTRLSSR;

		hw_if->config_hw_time_stamping(varMAC_TCR, pdata);

		/* program Sub Second Increment Reg */
		hw_if->config_sub_second_increment(DWC_ETH_QOS_SYSCLOCK, pdata);

		/* formula is :
		 * addend = 2^32/freq_div_ratio;
		 *
		 * where, freq_div_ratio = DWC_ETH_QOS_SYSCLOCK/50MHz
		 *
		 * hence, addend = ((2^32) * 50MHz)/DWC_ETH_QOS_SYSCLOCK;
		 *
		 * NOTE: DWC_ETH_QOS_SYSCLOCK should be >= 50MHz to
		 *       achive 20ns accuracy.
		 *
		 * 2^x * y == (y << x), hence
		 * 2^32 * 50000000 ==> (50000000 << 32)
		 * */
		temp = (u64)(50000000ULL << 32);
		pdata->default_addend = div_u64(temp, DWC_ETH_QOS_SYSCLOCK);

		hw_if->config_addend(pdata->default_addend, pdata);

		/* initialize system time */
		getnstimeofday(&now);
		hw_if->init_systime(now.tv_sec, now.tv_nsec, pdata);
	}

	DBGPR_PTP("config.flags = %#x, tx_type = %#x, rx_filter = %#x\n",
		config.flags, config.tx_type, config.rx_filter);

	DBGPR_PTP("<--DWC_ETH_QOS_handle_hwtstamp_ioctl\n");

	return (copy_to_user(ifr->ifr_data, &config,
		sizeof(struct hwtstamp_config))) ? -EFAULT : 0;
}


/*!
 * \brief Driver IOCTL routine
 *
 * \details This function is invoked by kernel when a user request an ioctl
 * which can't be handled by the generic interface code. Following operations
 * are performed in this functions.
 * - Configuring the PMT module.
 * - Configuring TX and RX PBL.
 * - Configuring the TX and RX FIFO threshold level.
 * - Configuring the TX and RX OSF mode.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] ifr – pointer to IOCTL specific structure.
 * \param[in] cmd – IOCTL command.
 *
 * \return int
 *
 * \retval 0 - success
 * \retval negative - failure
 */

static int DWC_ETH_QOS_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct ifr_data_struct *req = ifr->ifr_ifru.ifru_data;
	struct ifr_data_struct req_pvt;
	struct mii_ioctl_data *data = if_mii(ifr);
	unsigned int reg_val = 0;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_ioctl\n");

	if (!netif_running(dev) || (pdata->enable_phy && !pdata->phydev)) {
		DBGPR("<--DWC_ETH_QOS_ioctl - error\n");
		return -EINVAL;
	}

	mutex_lock(&pdata->mlock);
	switch (cmd) {
	case SIOCGMIIPHY:
		data->phy_id = pdata->phyaddr;
		NMSGPR_ALERT( "PHY ID: SIOCGMIIPHY\n");
		break;

	case SIOCGMIIREG:
		ret =
		    DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr,
				(data->reg_num & 0x1F), &reg_val);
		if (ret)
			ret = -EIO;

		data->val_out = reg_val;
		NMSGPR_ALERT( "PHY ID: SIOCGMIIREG reg:%#x reg_val:%#x\n",
		       (data->reg_num & 0x1F), reg_val);
		break;

	case SIOCSMIIREG:
		NMSGPR_ALERT( "PHY ID: SIOCSMIIPHY\n");
		break;

	case DWC_ETH_QOS_PRV_IOCTL:
		if (copy_from_user(&req_pvt, req,
				sizeof(struct ifr_data_struct))) {
			NMSGPR_ERR("DWC_ETH_QOS_ioctl copy from user error\n");
			return -EFAULT;
		}
		ret = DWC_ETH_QOS_handle_prv_ioctl(pdata, &req_pvt);
		if (copy_to_user(&req->command_error, &ret,
				sizeof(req->command_error))) {
			NMSGPR_ERR("DWC_ETH_QOS_ioctl copy to user error\n");
			return -EFAULT;
		}
		break;

	case DWC_ETH_QOS_PRV_IOCTL_IPA:
		if ( !pdata->prv_ipa.ipa_ready || !pdata->prv_ipa.ipa_uc_ready ) {
			ret = -EAGAIN;
			NMSGPR_INFO("IPA or IPA uc is not ready \n");
			break;
		}
		ret = DWC_ETH_QOS_handle_prv_ioctl_ipa(pdata, ifr);
        break;

	case SIOCSHWTSTAMP:
		ret = DWC_ETH_QOS_handle_hwtstamp_ioctl(pdata, ifr);
		break;

	default:
		ret = -EOPNOTSUPP;
		NDBGPR_L1( "Unsupported IOCTL call\n");
	}
	mutex_unlock(&pdata->mlock);

	DBGPR("<--DWC_ETH_QOS_ioctl\n");

	return ret;
}

#ifdef DWC_ETH_QOS_QUEUE_SELECT_ALGO
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0) )
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb)
#elif	( LINUX_VERSION_CODE == KERNEL_VERSION(3,13,0) )
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb, void *accel_priv)
#else   //3.13.0
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb, void *accel_priv, select_queue_fallback_t fallback)
#endif	//3.13.0
{
	static u16 txqueue_select = 0;
	UINT eth_or_vlan_tag;
	UINT avb_priority;
	UINT eth_type;
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	UINT tx_ch_host = pdata->ipa_enabled ? NTN_TX_PKT_HOST_IPA : NTN_TX_PKT_HOST;

	DBGPR("-->DWC_ETH_QOS_select_queue\n");

	/* Linearize the skb in case it is in fragments */
	if (skb_linearize(skb) != 0) {

		return tx_ch_host;
	}

	/* TX Channel assignment based on Vlan tag and protocol type */
	eth_or_vlan_tag = htons(((skb->data[13]<<8) | skb->data[12]));
	//NMSGPR_ALERT( "ETH TYPE or VLAN TAG : %#x\n", eth_or_vlan_tag);
	if(eth_or_vlan_tag == NTN_VLAN_TAG)
	{
		eth_type = htons(((skb->data[17]<<8) | skb->data[16]));
		//NMSGPR_INFO("VLAN ETH TYPE : %#x\n", eth_type);
		if(eth_type == NTN_ETH_TYPE_AVB)
		{
			/* Extract VLAN priority field from the tx data */
			avb_priority = htons((skb->data[15]<<8) | skb->data[14]);
			//NMSGPR_INFO("VID : %#x\n", avb_priority);
			avb_priority >>= 13;
			//NMSGPR_INFO("AVB Priority : %#x\n", avb_priority);
			if(avb_priority == NTN_AVB_PRIORITY_CLASS_A){
				txqueue_select = NTN_TX_PKT_AVB_CLASS_A;
			}else if(avb_priority == NTN_AVB_PRIORITY_CLASS_B){
				txqueue_select = NTN_TX_PKT_AVB_CLASS_B;
			}else{
				txqueue_select = tx_ch_host;
			}
		}else{
			/* VLAN but not AVB send it to Q0 */
			txqueue_select = NTN_TX_PKT_VLAN_NOT_AVB;
		}
	}
	else
	{
		/* It's protcol (ether type) field */
		eth_type = eth_or_vlan_tag;
		switch(eth_type){
			case NTN_GPTP_ETH_TYPE:
				txqueue_select = NTN_TX_PKT_GPTP;
				break;
			default:
				txqueue_select = tx_ch_host;
				break;
		}
	}

	DBGPR("<--DWC_ETH_QOS_select_queue txqueue-select:%d\n",
		txqueue_select);

	return txqueue_select;
}
#endif

unsigned int crc32_snps_le(unsigned int initval, unsigned char *data, unsigned int size)
{
	unsigned int crc = initval;
	unsigned int poly = 0x04c11db7;
	unsigned int temp = 0;
	unsigned char my_data = 0;
	int bit_count;
	for(bit_count = 0; bit_count < size; bit_count++) {
		if((bit_count % 8) == 0) my_data = data[bit_count/8];
		DBGPR_FILTER("%s my_data = %x crc=%x\n", __func__, my_data,crc);
		temp = ((crc >> 31) ^  my_data) &  0x1;
		crc <<= 1;
		if(temp != 0) crc ^= poly;
		my_data >>=1;
	}
		DBGPR_FILTER("%s my_data = %x crc=%x\n", __func__, my_data,crc);
	return ~crc;
}



/*!
* \brief API to delete vid to HW filter.
*
* \details This function is invoked by upper layer when a VLAN id is removed.
* This function deletes the VLAN id from the HW filter.
* vlan id can be removed with vconfig -
* vconfig rem <interface_name > <vlan_id>
*
* \param[in] dev - pointer to net_device structure
* \param[in] vid - vlan id to be removed.
*
* \return void
*/
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
                             u16 vid)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned short new_index, old_index;
	int crc32_val = 0;
	unsigned int enb_12bit_vhash;

	DBGPR( "-->DWC_ETH_QOS_vlan_rx_kill_vid: vid = %d\n",
		vid);

	if (pdata->vlan_hash_filtering) {
		crc32_val = (bitrev32(~crc32_le(~0, (unsigned char *)&vid, 2)) >> 28);

		enb_12bit_vhash = hw_if->get_vlan_tag_comparison(pdata);
		if (enb_12bit_vhash) {
			/* neget 4-bit crc value for 12-bit VLAN hash comparison */
			new_index = (1 << (~crc32_val & 0xF));
		} else {
			new_index = (1 << (crc32_val & 0xF));
		}

		old_index = hw_if->get_vlan_hash_table_reg(pdata);
		old_index &= ~new_index;
		hw_if->update_vlan_hash_table_reg(old_index, pdata);
		pdata->vlan_ht_or_id = old_index;
	} else {
		/* By default, receive only VLAN pkt with VID = 1
		 * becasue writting 0 will pass all VLAN pkt */
		hw_if->update_vlan_id(1, pdata);
		pdata->vlan_ht_or_id = 1;
	}

	DBGPR( "<--DWC_ETH_QOS_vlan_rx_kill_vid\n");

#if defined(HAVE_INT_NDO_VLAN_RX_ADD_VID) || defined(NETIF_F_HW_VLAN_CTAG_RX)
	return 0;
#else
	return;
#endif
}


/*!
* \brief API to add vid to HW filter.
*
* \details This function is invoked by upper layer when a new VALN id is
* registered. This function updates the HW filter with new VLAN id.
* New vlan id can be added with vconfig -
* vconfig add <interface_name > <vlan_id>
*
* \param[in] dev - pointer to net_device structure
* \param[in] vid - new vlan id.
*
* \return void
*/
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
                            u16 vid)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned short new_index, old_index;
	int crc32_val = 0;
	unsigned int enb_12bit_vhash;

	DBGPR( "-->DWC_ETH_QOS_vlan_rx_add_vid: vid = %d\n",
		vid);

	if (pdata->vlan_hash_filtering) {
		/* The upper 4 bits of the calculated CRC are used to
		 * index the content of the VLAN Hash Table Reg.
		 * */
		crc32_val = (bitrev32(~crc32_le(~0, (unsigned char *)&vid, 2)) >> 28);

		/* These 4(0xF) bits determines the bit within the
		 * VLAN Hash Table Reg 0
		 * */
		enb_12bit_vhash = hw_if->get_vlan_tag_comparison(pdata);
		if (enb_12bit_vhash) {
			/* neget 4-bit crc value for 12-bit VLAN hash comparison */
			new_index = (1 << (~crc32_val & 0xF));
		} else {
			new_index = (1 << (crc32_val & 0xF));
		}

		old_index = hw_if->get_vlan_hash_table_reg(pdata);
		old_index |= new_index;
		hw_if->update_vlan_hash_table_reg(old_index,pdata);
		pdata->vlan_ht_or_id = old_index;
	} else {
		hw_if->update_vlan_id(vid, pdata);
		pdata->vlan_ht_or_id = vid;
	}

	DBGPR( "<--DWC_ETH_QOS_vlan_rx_add_vid\n");

#if defined(HAVE_INT_NDO_VLAN_RX_ADD_VID) || defined(NETIF_F_HW_VLAN_CTAG_RX)
	return 0;
#else
	return;
#endif
}

/*!
 * \brief API called to put device in powerdown mode
 *
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to move the device to power down state. Following operations
 * are performed in this function.
 * - stop the phy.
 * - stop the queue.
 * - Disable napi.
 * - Stop DMA TX and RX process.
 * - Enable power down mode using PMT module.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] wakeup_type – remote wake-on-lan or magic packet.
 * \param[in] caller – netif_detach gets called conditionally based
 *                     on caller, IOCTL or DRIVER-suspend
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

INT DWC_ETH_QOS_powerdown(struct net_device *dev, UINT wakeup_type,
		UINT caller)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	DBGPR(KERN_ALERT "-->DWC_ETH_QOS_powerdown\n");

	if (!dev || !netif_running(dev) ||
	    (caller == DWC_ETH_QOS_IOCTL_CONTEXT && pdata->power_down)) {
		NMSGPR_ALERT(
		       "Device is already powered down and will powerup for %s\n",
		       DWC_ETH_QOS_POWER_DOWN_TYPE(pdata));
		DBGPR("<--DWC_ETH_QOS_powerdown\n");
		return -EINVAL;
	}

	mutex_lock(&pdata->pmt_lock);

	if (caller == DWC_ETH_QOS_DRIVER_CONTEXT)
		netif_device_detach(dev);

	netif_tx_disable(dev);
	DWC_ETH_QOS_all_ch_napi_disable(pdata);

	/* stop DMA TX/RX */
	DWC_ETH_QOS_stop_all_ch_tx_dma(pdata);
	DWC_ETH_QOS_stop_all_ch_rx_dma(pdata);

	DWC_ETH_QOS_free_irq(dev);

	/* enable power down mode by programming the PMT regs */
	if (wakeup_type & DWC_ETH_QOS_REMOTE_WAKEUP)
		hw_if->enable_remote_pmt(pdata);
	if (wakeup_type & DWC_ETH_QOS_MAGIC_WAKEUP)
		hw_if->enable_magic_pmt(pdata);
	pdata->power_down_type = wakeup_type;

	if (pdata->power_down_type) {
		/* Setup the PHY to 10M for power saving */
		DWC_ETH_QOS_enable_phy_wol(pdata);
		/* For WOL functionality, magic packets are only detected by
		 * the MAC0 filter */
		hw_if->configure_mac_addr0_reg(pdata->dev->dev_addr, pdata);
		/* Configure the PME interrupt pin
		   pme_mac_msk (BIT4)=0; unmask mac_pmt_intr_o
		   pme_o_pls (BIT16)=0; level signal
		   pme_o_lvl (BIT17)=0; active low level signal
		*/
		hw_if->ntn_config_pme(0x0F00000F, pdata);
	}

	if (caller == DWC_ETH_QOS_IOCTL_CONTEXT)
		pdata->power_down = 1;

	if (pdata->enable_phy && pdata->phydev)
		pdata->phydev->state = PHY_DOWN;

	mutex_unlock(&pdata->pmt_lock);

	DBGPR("<--DWC_ETH_QOS_powerdown\n");

	return 0;
}

/* Work function to call powerup as interruptible task */
void DWC_ETH_QOS_powerup_handler(struct work_struct *work)
{
	struct DWC_ETH_QOS_prv_data *pdata = container_of(work,
		struct DWC_ETH_QOS_prv_data, powerup_work);
	DBGPR("<--DWC_ETH_QOS_powerup_handler\n");
	DWC_ETH_QOS_powerup(pdata->dev, DWC_ETH_QOS_IOCTL_CONTEXT);
	DBGPR("-->DWC_ETH_QOS_powerup_handler\n");
}

/* Work function to call restart device */
void DWC_ETH_QOS_restart_dev_handler(struct work_struct *work)
{
	struct DWC_ETH_QOS_prv_data *pdata = container_of(work,
		struct DWC_ETH_QOS_prv_data, restartdev_work);
	DBGPR("-->DWC_ETH_QOS_restart_dev_handler\n");
	DWC_ETH_QOS_restart_dev(pdata, pdata->restart_channel_idx);
	DBGPR("<--DWC_ETH_QOS_restart_dev_handler\n");
}

/*!
 * \brief API to powerup the device
 *
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to move the device to out of power down state. Following
 * operations are performed in this function.
 * - Wakeup the device using PMT module if supported.
 * - Starts the phy.
 * - Enable MAC and DMA TX and RX process.
 * - Enable napi.
 * - Starts the queue.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] caller – netif_attach gets called conditionally based
 *                     on caller, IOCTL or DRIVER-suspend
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

INT DWC_ETH_QOS_powerup(struct net_device *dev, UINT caller)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct desc_if_struct *desc_if = &(pdata->desc_if);

	DBGPR("-->DWC_ETH_QOS_powerup\n");

	if (!dev || !netif_running(dev) ||
	    (caller == DWC_ETH_QOS_IOCTL_CONTEXT && !pdata->power_down)) {
		NMSGPR_ALERT( "Device is already powered up\n");
		DBGPR(KERN_ALERT "<--DWC_ETH_QOS_powerup\n");
		return -EINVAL;
	}

	mutex_lock(&pdata->pmt_lock);

	/* Return PHY to original state before suspend */
	if (pdata->power_down_type)
		DWC_ETH_QOS_disable_phy_wol(pdata);

	if (pdata->power_down_type & DWC_ETH_QOS_MAGIC_WAKEUP) {
		hw_if->disable_magic_pmt(pdata);
		pdata->power_down_type &= ~DWC_ETH_QOS_MAGIC_WAKEUP;
	}

	if (pdata->power_down_type & DWC_ETH_QOS_REMOTE_WAKEUP) {
		hw_if->disable_remote_pmt(pdata);
		pdata->power_down_type &= ~DWC_ETH_QOS_REMOTE_WAKEUP;
	}

	pdata->power_down = 0;

	if (DWC_ETH_QOS_request_irq(dev) != 0) {
		NMSGPR_ALERT("Unable to register IRQ %d\n", pdata->irq_number);
		return -EBUSY;
	}

	if (pdata->pcierst_resx) {
		/* Clean old rx skbs to avoid leak since they are allocated
		 * again when initializing rx descriptors */
		desc_if->rx_skb_free_mem(pdata, NTN_RX_DMA_CH_CNT);
		/* Avoid leak of eee timer */
		if (pdata->eee_enabled)
			del_timer_sync(&pdata->eee_ctrl_timer);
		DWC_ETH_QOS_init_common(dev);
	} else {
		/* Re-initialize the MAC since it can get reset during WOL */
		hw_if->init(pdata);
		if (pdata->enable_phy && pdata->phydev)
			genphy_restart_aneg(pdata->phydev);
	}

	/* enable MAC TX/RX */
	hw_if->start_mac_tx_rx(pdata);

	/* enable DMA TX/RX */
	DWC_ETH_QOS_start_all_ch_tx_dma(pdata);
	DWC_ETH_QOS_start_all_ch_rx_dma(pdata);

	if (caller == DWC_ETH_QOS_DRIVER_CONTEXT)
		netif_device_attach(dev);

	DWC_ETH_QOS_napi_enable_mq(pdata);

	netif_tx_start_all_queues(dev);

	if (pdata->enable_phy && pdata->phydev)
		pdata->phydev->state = PHY_UP;

	mutex_unlock(&pdata->pmt_lock);

	DBGPR("<--DWC_ETH_QOS_powerup\n");

	return 0;
}

/*!
 * \brief API to configure remote wakeup
 *
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to move the device to power down state using remote wakeup.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] req – pointer to ioctl data structure.
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

INT DWC_ETH_QOS_configure_remotewakeup(struct net_device *dev,
				       struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	if (!dev || !netif_running(dev) || !pdata->hw_feat.rwk_sel
	    || pdata->power_down) {
		NMSGPR_ALERT(
		       "Device is already powered down and will powerup for %s\n",
		       DWC_ETH_QOS_POWER_DOWN_TYPE(pdata));
		return -EINVAL;
	}

	hw_if->configure_rwk_filter(req->rwk_filter_values,
				    req->rwk_filter_length, pdata);

	DWC_ETH_QOS_powerdown(dev, DWC_ETH_QOS_REMOTE_WAKEUP,
			DWC_ETH_QOS_IOCTL_CONTEXT);

	return 0;
}

/*!
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to change the RX DMA PBL value. This function will program
 * the device to configure the user specified RX PBL value.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] rx_pbl – RX DMA pbl value to be programmed.
 *
 * \return void
 *
 * \retval none
 */

static void DWC_ETH_QOS_config_rx_pbl(struct DWC_ETH_QOS_prv_data *pdata,
				      UINT rx_pbl,
				      UINT chInx)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT pblx8_val = 0;

	DBGPR("-->DWC_ETH_QOS_config_rx_pbl: %d\n", rx_pbl);

	switch (rx_pbl) {
	case DWC_ETH_QOS_PBL_1:
	case DWC_ETH_QOS_PBL_2:
	case DWC_ETH_QOS_PBL_4:
	case DWC_ETH_QOS_PBL_8:
	case DWC_ETH_QOS_PBL_16:
	case DWC_ETH_QOS_PBL_32:
		hw_if->config_rx_pbl_val(chInx, rx_pbl, pdata);
		hw_if->config_pblx8(chInx, 0);
		break;
	case DWC_ETH_QOS_PBL_64:
	case DWC_ETH_QOS_PBL_128:
	case DWC_ETH_QOS_PBL_256:
		hw_if->config_rx_pbl_val(chInx, rx_pbl / 8, pdata);
		hw_if->config_pblx8(chInx, 1);
		pblx8_val = 1;
		break;
	}

	switch (pblx8_val) {
		case 0:
			NMSGPR_ALERT( "Tx PBL[%d] value: %d\n",
					chInx, hw_if->get_tx_pbl_val(chInx, pdata));
			NMSGPR_ALERT( "Rx PBL[%d] value: %d\n",
					chInx, hw_if->get_rx_pbl_val(chInx, pdata));
			break;
		case 1:
			NMSGPR_ALERT( "Tx PBL[%d] value: %d\n",
					chInx, (hw_if->get_tx_pbl_val(chInx, pdata) * 8));
			NMSGPR_ALERT( "Rx PBL[%d] value: %d\n",
					chInx, (hw_if->get_rx_pbl_val(chInx, pdata) * 8));
			break;
	}

	DBGPR("<--DWC_ETH_QOS_config_rx_pbl\n");
}

/*!
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to change the TX DMA PBL value. This function will program
 * the device to configure the user specified TX PBL value.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] tx_pbl – TX DMA pbl value to be programmed.
 *
 * \return void
 *
 * \retval none
 */

static void DWC_ETH_QOS_config_tx_pbl(struct DWC_ETH_QOS_prv_data *pdata,
				      UINT tx_pbl,
				      UINT chInx)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT pblx8_val = 0;

	DBGPR("-->DWC_ETH_QOS_config_tx_pbl: %d\n", tx_pbl);

	switch (tx_pbl) {
	case DWC_ETH_QOS_PBL_1:
	case DWC_ETH_QOS_PBL_2:
	case DWC_ETH_QOS_PBL_4:
	case DWC_ETH_QOS_PBL_8:
	case DWC_ETH_QOS_PBL_16:
	case DWC_ETH_QOS_PBL_32:
		hw_if->config_tx_pbl_val(chInx, tx_pbl, pdata);
		hw_if->config_pblx8(chInx, 0);
		break;
	case DWC_ETH_QOS_PBL_64:
	case DWC_ETH_QOS_PBL_128:
	case DWC_ETH_QOS_PBL_256:
		hw_if->config_tx_pbl_val(chInx, tx_pbl / 8, pdata);
		hw_if->config_pblx8(chInx, 1);
		pblx8_val = 1;
		break;
	}

	switch (pblx8_val) {
		case 0:
			NMSGPR_ALERT( "Tx PBL[%d] value: %d\n",
					chInx, hw_if->get_tx_pbl_val(chInx, pdata));
			NMSGPR_ALERT( "Rx PBL[%d] value: %d\n",
					chInx, hw_if->get_rx_pbl_val(chInx, pdata));
			break;
		case 1:
			NMSGPR_ALERT( "Tx PBL[%d] value: %d\n",
					chInx, (hw_if->get_tx_pbl_val(chInx, pdata) * 8));
			NMSGPR_ALERT( "Rx PBL[%d] value: %d\n",
					chInx, (hw_if->get_rx_pbl_val(chInx, pdata) * 8));
			break;
	}

	DBGPR("<--DWC_ETH_QOS_config_tx_pbl\n");
}


/*!
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to select the DCB algorithm.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] req – pointer to ioctl data structure.
 *
 * \return void
 *
 * \retval none
 */

static void DWC_ETH_QOS_program_dcb_algorithm(struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_dcb_algorithm l_dcb_struct, *u_dcb_struct =
		(struct DWC_ETH_QOS_dcb_algorithm *)req->ptr;
	struct hw_if_struct *hw_if = &pdata->hw_if;

	DBGPR("-->DWC_ETH_QOS_program_dcb_algorithm\n");

	if(copy_from_user(&l_dcb_struct, u_dcb_struct,
				sizeof(struct DWC_ETH_QOS_dcb_algorithm)))
		NMSGPR_ALERT( "Failed to fetch DCB Struct info from user\n");

	hw_if->set_tx_queue_operating_mode(l_dcb_struct.chInx,
		(UINT)l_dcb_struct.op_mode, pdata);
	hw_if->set_dcb_algorithm(l_dcb_struct.algorithm, pdata);
	hw_if->set_dcb_queue_weight(l_dcb_struct.chInx, l_dcb_struct.weight, pdata);

	DBGPR("<--DWC_ETH_QOS_program_dcb_algorithm\n");

	return;
}

/*!
 * \details This function is invoked by mdio::DWC_ETH_QOS_adjust_link function when
 * the link speed changes.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 *
 * \retval none
 */

void DWC_ETH_QOS_reload_fqtss_cfg(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &pdata->hw_if;
	struct DWC_ETH_QOS_avb_algorithm_params *avb_params;
	unsigned long flags;

	spin_lock_irqsave(&pdata->fqtss_lock, flags);

	NDBGPR_L1("---------------------------------------------\n");
	NDBGPR_L1("Reloading CBS for speed %d\n", pdata->speed);
	NDBGPR_L1("--- Class A ---\n");

	if (pdata->speed == SPEED_1000)
		avb_params = &(pdata->cbsSpeed1000Cfg[0]);
	else
		avb_params = &(pdata->cbsSpeed100Cfg[0]);

	hw_if->config_send_slope(AVB_QUEUE_CLASS_A, avb_params->send_slope, pdata);
	hw_if->config_idle_slope(AVB_QUEUE_CLASS_A, avb_params->idle_slope, pdata);
	hw_if->config_high_credit(AVB_QUEUE_CLASS_A, avb_params->hi_credit, pdata);
	hw_if->config_low_credit(AVB_QUEUE_CLASS_A, avb_params->low_credit, pdata);

	NDBGPR_L1("--- Class B ---\n");

	if (pdata->speed == SPEED_1000)
		avb_params = &(pdata->cbsSpeed1000Cfg[1]);
	else
		avb_params = &(pdata->cbsSpeed100Cfg[1]);

	hw_if->config_send_slope(AVB_QUEUE_CLASS_B, avb_params->send_slope, pdata);
	hw_if->config_idle_slope(AVB_QUEUE_CLASS_B, avb_params->idle_slope, pdata);
	hw_if->config_high_credit(AVB_QUEUE_CLASS_B, avb_params->hi_credit, pdata);
	hw_if->config_low_credit(AVB_QUEUE_CLASS_B, avb_params->low_credit, pdata);

	NDBGPR_L1("---------------------------------------------\n");

	spin_unlock_irqrestore(&pdata->fqtss_lock, flags);
}

/*!
 * \details This function is invoked by ioctl function when the user issues an
 * ioctl command to select the AVB algorithm. This function also configures other
 * parameters like send and idle slope, high and low credit.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] req – pointer to ioctl data structure.
 *
 * \return void
 *
 * \retval none
 */

static void DWC_ETH_QOS_program_avb_algorithm(struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req)
{
	struct DWC_ETH_QOS_avb_algorithm l_avb_struct, *u_avb_struct =
		(struct DWC_ETH_QOS_avb_algorithm *)req->ptr;
	struct hw_if_struct *hw_if = &pdata->hw_if;
	struct DWC_ETH_QOS_avb_algorithm_params *avb_params;
	unsigned long flags;

	spin_lock_irqsave(&pdata->fqtss_lock, flags);

	DBGPR("-->DWC_ETH_QOS_program_avb_algorithm\n");

	if(copy_from_user(&l_avb_struct, u_avb_struct,
				sizeof(struct DWC_ETH_QOS_avb_algorithm)))
		NMSGPR_ALERT( "Failed to fetch AVB Struct info from user\n");

	pdata->cbsSpeed100Cfg[l_avb_struct.chInx - 1] = l_avb_struct.speed100params;
	pdata->cbsSpeed1000Cfg[l_avb_struct.chInx - 1] = l_avb_struct.speed1000params;

	if (pdata->speed == SPEED_1000)
		avb_params = &(pdata->cbsSpeed1000Cfg[l_avb_struct.chInx - 1]);
	else
		avb_params = &(pdata->cbsSpeed100Cfg[l_avb_struct.chInx - 1]);

	NDBGPR_L1("---------------------------------------------\n");
	NDBGPR_L1("Setting CBS for speed %d, class %s\n", pdata->speed, l_avb_struct.chInx == 1 ? "A" : "B");

	hw_if->set_tx_queue_operating_mode(l_avb_struct.chInx,
		(UINT)l_avb_struct.op_mode, pdata);
	hw_if->set_avb_algorithm(l_avb_struct.chInx, l_avb_struct.algorithm, pdata);
	hw_if->config_credit_control(l_avb_struct.chInx, l_avb_struct.cc, pdata);
	hw_if->config_send_slope(l_avb_struct.chInx, avb_params->send_slope, pdata);
	hw_if->config_idle_slope(l_avb_struct.chInx, avb_params->idle_slope, pdata);
	hw_if->config_high_credit(l_avb_struct.chInx, avb_params->hi_credit, pdata);
	hw_if->config_low_credit(l_avb_struct.chInx, avb_params->low_credit, pdata);

	NDBGPR_L1("---------------------------------------------\n");

	DBGPR("<--DWC_ETH_QOS_program_avb_algorithm\n");

	spin_unlock_irqrestore(&pdata->fqtss_lock, flags);

	return;
}

/*!
* \brief API to read the registers & prints the value.
* \details This function will read all the device register except
* data register & prints the values.
*
* \return none
*/
#if 0
void dbgpr_regs(void)
{
	UINT val0;
	UINT val1;
	UINT val2;
	UINT val3;
	UINT val4;
	UINT val5;

	MAC_PMTCSR_RgRd(val0);
	MMC_RXICMP_ERR_OCTETS_RgRd(val1);
	MMC_RXICMP_GD_OCTETS_RgRd(val2);
	MMC_RXTCP_ERR_OCTETS_RgRd(val3);
	MMC_RXTCP_GD_OCTETS_RgRd(val4);
	MMC_RXUDP_ERR_OCTETS_RgRd(val5);

	DBGPR("dbgpr_regs: MAC_PMTCSR:%#x\n"
	      "dbgpr_regs: MMC_RXICMP_ERR_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXICMP_GD_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXTCP_ERR_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXTCP_GD_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXUDP_ERR_OCTETS:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXUDP_GD_OCTETS_RgRd(val0);
	MMC_RXIPV6_NOPAY_OCTETS_RgRd(val1);
	MMC_RXIPV6_HDRERR_OCTETS_RgRd(val2);
	MMC_RXIPV6_GD_OCTETS_RgRd(val3);
	MMC_RXIPV4_UDSBL_OCTETS_RgRd(val4);
	MMC_RXIPV4_FRAG_OCTETS_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXUDP_GD_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV6_NOPAY_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV6_HDRERR_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV6_GD_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_UDSBL_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_FRAG_OCTETS:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXIPV4_NOPAY_OCTETS_RgRd(val0);
	MMC_RXIPV4_HDRERR_OCTETS_RgRd(val1);
	MMC_RXIPV4_GD_OCTETS_RgRd(val2);
	MMC_RXICMP_ERR_PKTS_RgRd(val3);
	MMC_RXICMP_GD_PKTS_RgRd(val4);
	MMC_RXTCP_ERR_PKTS_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXIPV4_NOPAY_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_HDRERR_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_GD_OCTETS:%#x\n"
	      "dbgpr_regs: MMC_RXICMP_ERR_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXICMP_GD_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXTCP_ERR_PKTS:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXTCP_GD_PKTS_RgRd(val0);
	MMC_RXUDP_ERR_PKTS_RgRd(val1);
	MMC_RXUDP_GD_PKTS_RgRd(val2);
	MMC_RXIPV6_NOPAY_PKTS_RgRd(val3);
	MMC_RXIPV6_HDRERR_PKTS_RgRd(val4);
	MMC_RXIPV6_GD_PKTS_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXTCP_GD_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXUDP_ERR_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXUDP_GD_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV6_NOPAY_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV6_HDRERR_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV6_GD_PKTS:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXIPV4_UBSBL_PKTS_RgRd(val0);
	MMC_RXIPV4_FRAG_PKTS_RgRd(val1);
	MMC_RXIPV4_NOPAY_PKTS_RgRd(val2);
	MMC_RXIPV4_HDRERR_PKTS_RgRd(val3);
	MMC_RXIPV4_GD_PKTS_RgRd(val4);
	MMC_RXCTRLPACKETS_G_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXIPV4_UBSBL_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_FRAG_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_NOPAY_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_HDRERR_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXIPV4_GD_PKTS:%#x\n"
	      "dbgpr_regs: MMC_RXCTRLPACKETS_G:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXRCVERROR_RgRd(val0);
	MMC_RXWATCHDOGERROR_RgRd(val1);
	MMC_RXVLANPACKETS_GB_RgRd(val2);
	MMC_RXFIFOOVERFLOW_RgRd(val3);
	MMC_RXPAUSEPACKETS_RgRd(val4);
	MMC_RXOUTOFRANGETYPE_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXRCVERROR:%#x\n"
	      "dbgpr_regs: MMC_RXWATCHDOGERROR:%#x\n"
	      "dbgpr_regs: MMC_RXVLANPACKETS_GB:%#x\n"
	      "dbgpr_regs: MMC_RXFIFOOVERFLOW:%#x\n"
	      "dbgpr_regs: MMC_RXPAUSEPACKETS:%#x\n"
	      "dbgpr_regs: MMC_RXOUTOFRANGETYPE:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXLENGTHERROR_RgRd(val0);
	MMC_RXUNICASTPACKETS_G_RgRd(val1);
	MMC_RX1024TOMAXOCTETS_GB_RgRd(val2);
	MMC_RX512TO1023OCTETS_GB_RgRd(val3);
	MMC_RX256TO511OCTETS_GB_RgRd(val4);
	MMC_RX128TO255OCTETS_GB_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXLENGTHERROR:%#x\n"
	      "dbgpr_regs: MMC_RXUNICASTPACKETS_G:%#x\n"
	      "dbgpr_regs: MMC_RX1024TOMAXOCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_RX512TO1023OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_RX256TO511OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_RX128TO255OCTETS_GB:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RX65TO127OCTETS_GB_RgRd(val0);
	MMC_RX64OCTETS_GB_RgRd(val1);
	MMC_RXOVERSIZE_G_RgRd(val2);
	MMC_RXUNDERSIZE_G_RgRd(val3);
	MMC_RXJABBERERROR_RgRd(val4);
	MMC_RXRUNTERROR_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RX65TO127OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_RX64OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_RXOVERSIZE_G:%#x\n"
	      "dbgpr_regs: MMC_RXUNDERSIZE_G:%#x\n"
	      "dbgpr_regs: MMC_RXJABBERERROR:%#x\n"
	      "dbgpr_regs: MMC_RXRUNTERROR:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXALIGNMENTERROR_RgRd(val0);
	MMC_RXCRCERROR_RgRd(val1);
	MMC_RXMULTICASTPACKETS_G_RgRd(val2);
	MMC_RXBROADCASTPACKETS_G_RgRd(val3);
	MMC_RXOCTETCOUNT_G_RgRd(val4);
	MMC_RXOCTETCOUNT_GB_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXALIGNMENTERROR:%#x\n"
	      "dbgpr_regs: MMC_RXCRCERROR:%#x\n"
	      "dbgpr_regs: MMC_RXMULTICASTPACKETS_G:%#x\n"
	      "dbgpr_regs: MMC_RXBROADCASTPACKETS_G:%#x\n"
	      "dbgpr_regs: MMC_RXOCTETCOUNT_G:%#x\n"
	      "dbgpr_regs: MMC_RXOCTETCOUNT_GB:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_RXPACKETCOUNT_GB_RgRd(val0);
	MMC_TXOVERSIZE_G_RgRd(val1);
	MMC_TXVLANPACKETS_G_RgRd(val2);
	MMC_TXPAUSEPACKETS_RgRd(val3);
	MMC_TXEXCESSDEF_RgRd(val4);
	MMC_TXPACKETSCOUNT_G_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_RXPACKETCOUNT_GB:%#x\n"
	      "dbgpr_regs: MMC_TXOVERSIZE_G:%#x\n"
	      "dbgpr_regs: MMC_TXVLANPACKETS_G:%#x\n"
	      "dbgpr_regs: MMC_TXPAUSEPACKETS:%#x\n"
	      "dbgpr_regs: MMC_TXEXCESSDEF:%#x\n"
	      "dbgpr_regs: MMC_TXPACKETSCOUNT_G:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_TXOCTETCOUNT_G_RgRd(val0);
	MMC_TXCARRIERERROR_RgRd(val1);
	MMC_TXEXESSCOL_RgRd(val2);
	MMC_TXLATECOL_RgRd(val3);
	MMC_TXDEFERRED_RgRd(val4);
	MMC_TXMULTICOL_G_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_TXOCTETCOUNT_G:%#x\n"
	      "dbgpr_regs: MMC_TXCARRIERERROR:%#x\n"
	      "dbgpr_regs: MMC_TXEXESSCOL:%#x\n"
	      "dbgpr_regs: MMC_TXLATECOL:%#x\n"
	      "dbgpr_regs: MMC_TXDEFERRED:%#x\n"
	      "dbgpr_regs: MMC_TXMULTICOL_G:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_TXSINGLECOL_G_RgRd(val0);
	MMC_TXUNDERFLOWERROR_RgRd(val1);
	MMC_TXBROADCASTPACKETS_GB_RgRd(val2);
	MMC_TXMULTICASTPACKETS_GB_RgRd(val3);
	MMC_TXUNICASTPACKETS_GB_RgRd(val4);
	MMC_TX1024TOMAXOCTETS_GB_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_TXSINGLECOL_G:%#x\n"
	      "dbgpr_regs: MMC_TXUNDERFLOWERROR:%#x\n"
	      "dbgpr_regs: MMC_TXBROADCASTPACKETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TXMULTICASTPACKETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TXUNICASTPACKETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TX1024TOMAXOCTETS_GB:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_TX512TO1023OCTETS_GB_RgRd(val0);
	MMC_TX256TO511OCTETS_GB_RgRd(val1);
	MMC_TX128TO255OCTETS_GB_RgRd(val2);
	MMC_TX65TO127OCTETS_GB_RgRd(val3);
	MMC_TX64OCTETS_GB_RgRd(val4);
	MMC_TXMULTICASTPACKETS_G_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_TX512TO1023OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TX256TO511OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TX128TO255OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TX65TO127OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TX64OCTETS_GB:%#x\n"
	      "dbgpr_regs: MMC_TXMULTICASTPACKETS_G:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_TXBROADCASTPACKETS_G_RgRd(val0);
	MMC_TXPACKETCOUNT_GB_RgRd(val1);
	MMC_TXOCTETCOUNT_GB_RgRd(val2);
	MMC_IPC_INTR_RX_RgRd(val3);
	MMC_IPC_INTR_MASK_RX_RgRd(val4);
	MMC_INTR_MASK_TX_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_TXBROADCASTPACKETS_G:%#x\n"
	      "dbgpr_regs: MMC_TXPACKETCOUNT_GB:%#x\n"
	      "dbgpr_regs: MMC_TXOCTETCOUNT_GB:%#x\n"
	      "dbgpr_regs: MMC_IPC_INTR_RX:%#x\n"
	      "dbgpr_regs: MMC_IPC_INTR_MASK_RX:%#x\n"
	      "dbgpr_regs: MMC_INTR_MASK_TX:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MMC_INTR_MASK_RX_RgRd(val0);
	MMC_INTR_TX_RgRd(val1);
	MMC_INTR_RX_RgRd(val2);
	MMC_CNTRL_RgRd(val3);
	MAC_MA1LR_RgRd(val4);
	MAC_MA1HR_RgRd(val5);

	DBGPR("dbgpr_regs: MMC_INTR_MASK_RX:%#x\n"
	      "dbgpr_regs: MMC_INTR_TX:%#x\n"
	      "dbgpr_regs: MMC_INTR_RX:%#x\n"
	      "dbgpr_regs: MMC_CNTRL:%#x\n"
	      "dbgpr_regs: MAC_MA1LR:%#x\n"
	      "dbgpr_regs: MAC_MA1HR:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MAC_MA0LR_RgRd(val0);
	MAC_MA0HR_RgRd(val1);
	MAC_GPIOR_RgRd(val2);
	MAC_GMIIDR_RgRd(val3);
	MAC_GMIIAR_RgRd(val4);
	MAC_HFR2_RgRd(val5);

	DBGPR("dbgpr_regs: MAC_MA0LR:%#x\n"
	      "dbgpr_regs: MAC_MA0HR:%#x\n"
	      "dbgpr_regs: MAC_GPIOR:%#x\n"
	      "dbgpr_regs: MAC_GMIIDR:%#x\n"
	      "dbgpr_regs: MAC_GMIIAR:%#x\n"
	      "dbgpr_regs: MAC_HFR2:%#x\n", val0, val1, val2, val3, val4, val5);

	MAC_HFR1_RgRd(val0);
	MAC_HFR0_RgRd(val1);
	MAC_MDR_RgRd(val2);
	MAC_VR_RgRd(val3);
	MAC_HTR7_RgRd(val4);
	MAC_HTR6_RgRd(val5);

	DBGPR("dbgpr_regs: MAC_HFR1:%#x\n"
	      "dbgpr_regs: MAC_HFR0:%#x\n"
	      "dbgpr_regs: MAC_MDR:%#x\n"
	      "dbgpr_regs: MAC_VR:%#x\n"
	      "dbgpr_regs: MAC_HTR7:%#x\n"
	      "dbgpr_regs: MAC_HTR6:%#x\n", val0, val1, val2, val3, val4, val5);

	MAC_HTR5_RgRd(val0);
	MAC_HTR4_RgRd(val1);
	MAC_HTR3_RgRd(val2);
	MAC_HTR2_RgRd(val3);
	MAC_HTR1_RgRd(val4);
	MAC_HTR0_RgRd(val5);

	DBGPR("dbgpr_regs: MAC_HTR5:%#x\n"
	      "dbgpr_regs: MAC_HTR4:%#x\n"
	      "dbgpr_regs: MAC_HTR3:%#x\n"
	      "dbgpr_regs: MAC_HTR2:%#x\n"
	      "dbgpr_regs: MAC_HTR1:%#x\n"
	      "dbgpr_regs: MAC_HTR0:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_RIWTR7_RgRd(val0);
	DMA_RIWTR6_RgRd(val1);
	DMA_RIWTR5_RgRd(val2);
	DMA_RIWTR4_RgRd(val3);
	DMA_RIWTR3_RgRd(val4);
	DMA_RIWTR2_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RIWTR7:%#x\n"
	      "dbgpr_regs: DMA_RIWTR6:%#x\n"
	      "dbgpr_regs: DMA_RIWTR5:%#x\n"
	      "dbgpr_regs: DMA_RIWTR4:%#x\n"
	      "dbgpr_regs: DMA_RIWTR3:%#x\n"
	      "dbgpr_regs: DMA_RIWTR2:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_RIWTR1_RgRd(val0);
	DMA_RIWTR0_RgRd(val1);
	DMA_RDRLR7_RgRd(val2);
	DMA_RDRLR6_RgRd(val3);
	DMA_RDRLR5_RgRd(val4);
	DMA_RDRLR4_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RIWTR1:%#x\n"
	      "dbgpr_regs: DMA_RIWTR0:%#x\n"
	      "dbgpr_regs: DMA_RDRLR7:%#x\n"
	      "dbgpr_regs: DMA_RDRLR6:%#x\n"
	      "dbgpr_regs: DMA_RDRLR5:%#x\n"
	      "dbgpr_regs: DMA_RDRLR4:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_RDRLR3_RgRd(val0);
	DMA_RDRLR2_RgRd(val1);
	DMA_RDRLR1_RgRd(val2);
	DMA_RDRLR0_RgRd(val3);
	DMA_TDRLR7_RgRd(val4);
	DMA_TDRLR6_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RDRLR3:%#x\n"
	      "dbgpr_regs: DMA_RDRLR2:%#x\n"
	      "dbgpr_regs: DMA_RDRLR1:%#x\n"
	      "dbgpr_regs: DMA_RDRLR0:%#x\n"
	      "dbgpr_regs: DMA_TDRLR7:%#x\n"
	      "dbgpr_regs: DMA_TDRLR6:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_TDRLR5_RgRd(val0);
	DMA_TDRLR4_RgRd(val1);
	DMA_TDRLR3_RgRd(val2);
	DMA_TDRLR2_RgRd(val3);
	DMA_TDRLR1_RgRd(val4);
	DMA_TDRLR0_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_TDRLR5:%#x\n"
	      "dbgpr_regs: DMA_TDRLR4:%#x\n"
	      "dbgpr_regs: DMA_TDRLR3:%#x\n"
	      "dbgpr_regs: DMA_TDRLR2:%#x\n"
	      "dbgpr_regs: DMA_TDRLR1:%#x\n"
	      "dbgpr_regs: DMA_TDRLR0:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_RDTP_RPDR7_RgRd(val0);
	DMA_RDTP_RPDR6_RgRd(val1);
	DMA_RDTP_RPDR5_RgRd(val2);
	DMA_RDTP_RPDR4_RgRd(val3);
	DMA_RDTP_RPDR3_RgRd(val4);
	DMA_RDTP_RPDR2_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RDTP_RPDR7:%#x\n"
	      "dbgpr_regs: DMA_RDTP_RPDR6:%#x\n"
	      "dbgpr_regs: DMA_RDTP_RPDR5:%#x\n"
	      "dbgpr_regs: DMA_RDTP_RPDR4:%#x\n"
	      "dbgpr_regs: DMA_RDTP_RPDR3:%#x\n"
	      "dbgpr_regs: DMA_RDTP_RPDR2:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_RDTP_RPDR1_RgRd(val0);
	DMA_RDTP_RPDR0_RgRd(val1);
	DMA_TDTP_TPDR7_RgRd(val2);
	DMA_TDTP_TPDR6_RgRd(val3);
	DMA_TDTP_TPDR5_RgRd(val4);
	DMA_TDTP_TPDR4_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RDTP_RPDR1:%#x\n"
	      "dbgpr_regs: DMA_RDTP_RPDR0:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR7:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR6:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR5:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR4:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_TDTP_TPDR3_RgRd(val0);
	DMA_TDTP_TPDR2_RgRd(val1);
	DMA_TDTP_TPDR1_RgRd(val2);
	DMA_TDTP_TPDR0_RgRd(val3);
	DMA_RDLAR7_RgRd(val4);
	DMA_RDLAR6_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_TDTP_TPDR3:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR2:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR1:%#x\n"
	      "dbgpr_regs: DMA_TDTP_TPDR0:%#x\n"
	      "dbgpr_regs: DMA_RDLAR7:%#x\n"
	      "dbgpr_regs: DMA_RDLAR6:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_RDLAR5_RgRd(val0);
	DMA_RDLAR4_RgRd(val1);
	DMA_RDLAR3_RgRd(val2);
	DMA_RDLAR2_RgRd(val3);
	DMA_RDLAR1_RgRd(val4);
	DMA_RDLAR0_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RDLAR5:%#x\n"
	      "dbgpr_regs: DMA_RDLAR4:%#x\n"
	      "dbgpr_regs: DMA_RDLAR3:%#x\n"
	      "dbgpr_regs: DMA_RDLAR2:%#x\n"
	      "dbgpr_regs: DMA_RDLAR1:%#x\n"
	      "dbgpr_regs: DMA_RDLAR0:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_TDLAR7_RgRd(val0);
	DMA_TDLAR6_RgRd(val1);
	DMA_TDLAR5_RgRd(val2);
	DMA_TDLAR4_RgRd(val3);
	DMA_TDLAR3_RgRd(val4);
	DMA_TDLAR2_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_TDLAR7:%#x\n"
	      "dbgpr_regs: DMA_TDLAR6:%#x\n"
	      "dbgpr_regs: DMA_TDLAR5:%#x\n"
	      "dbgpr_regs: DMA_TDLAR4:%#x\n"
	      "dbgpr_regs: DMA_TDLAR3:%#x\n"
	      "dbgpr_regs: DMA_TDLAR2:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_TDLAR1_RgRd(val0);
	DMA_TDLAR0_RgRd(val1);
	DMA_IER7_RgRd(val2);
	DMA_IER6_RgRd(val3);
	DMA_IER5_RgRd(val4);
	DMA_IER4_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_TDLAR1:%#x\n"
	      "dbgpr_regs: DMA_TDLAR0:%#x\n"
	      "dbgpr_regs: DMA_IER7:%#x\n"
	      "dbgpr_regs: DMA_IER6:%#x\n"
	      "dbgpr_regs: DMA_IER5:%#x\n"
	      "dbgpr_regs: DMA_IER4:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_IER3_RgRd(val0);
	DMA_IER2_RgRd(val1);
	DMA_IER1_RgRd(val2);
	DMA_IER0_RgRd(val3);
	MAC_IMR_RgRd(val4);
	MAC_ISR_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_IER3:%#x\n"
	      "dbgpr_regs: DMA_IER2:%#x\n"
	      "dbgpr_regs: DMA_IER1:%#x\n"
	      "dbgpr_regs: DMA_IER0:%#x\n"
	      "dbgpr_regs: MAC_IMR:%#x\n"
	      "dbgpr_regs: MAC_ISR:%#x\n", val0, val1, val2, val3, val4, val5);

	MTL_ISR_RgRd(val0);
	DMA_SR7_RgRd(val1);
	DMA_SR6_RgRd(val2);
	DMA_SR5_RgRd(val3);
	DMA_SR4_RgRd(val4);
	DMA_SR3_RgRd(val5);

	DBGPR("dbgpr_regs: MTL_ISR:%#x\n"
	      "dbgpr_regs: DMA_SR7:%#x\n"
	      "dbgpr_regs: DMA_SR6:%#x\n"
	      "dbgpr_regs: DMA_SR5:%#x\n"
	      "dbgpr_regs: DMA_SR4:%#x\n"
	      "dbgpr_regs: DMA_SR3:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_SR2_RgRd(val0);
	DMA_SR1_RgRd(val1);
	DMA_SR0_RgRd(val2);
	DMA_ISR_RgRd(val3);
	DMA_DSR2_RgRd(val4);
	DMA_DSR1_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_SR2:%#x\n"
	      "dbgpr_regs: DMA_SR1:%#x\n"
	      "dbgpr_regs: DMA_SR0:%#x\n"
	      "dbgpr_regs: DMA_ISR:%#x\n"
	      "dbgpr_regs: DMA_DSR2:%#x\n"
	      "dbgpr_regs: DMA_DSR1:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_DSR0_RgRd(val0);
	MTL_Q0RDR_RgRd(val1);
	MTL_Q0ESR_RgRd(val2);
	MTL_Q0TDR_RgRd(val3);
	DMA_CHRBAR7_RgRd(val4);
	DMA_CHRBAR6_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_DSR0:%#x\n"
	      "dbgpr_regs: MTL_Q0RDR:%#x\n"
	      "dbgpr_regs: MTL_Q0ESR:%#x\n"
	      "dbgpr_regs: MTL_Q0TDR:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR7:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR6:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_CHRBAR5_RgRd(val0);
	DMA_CHRBAR4_RgRd(val1);
	DMA_CHRBAR3_RgRd(val2);
	DMA_CHRBAR2_RgRd(val3);
	DMA_CHRBAR1_RgRd(val4);
	DMA_CHRBAR0_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_CHRBAR5:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR4:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR3:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR2:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR1:%#x\n"
	      "dbgpr_regs: DMA_CHRBAR0:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_CHTBAR7_RgRd(val0);
	DMA_CHTBAR6_RgRd(val1);
	DMA_CHTBAR5_RgRd(val2);
	DMA_CHTBAR4_RgRd(val3);
	DMA_CHTBAR3_RgRd(val4);
	DMA_CHTBAR2_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_CHTBAR7:%#x\n"
	      "dbgpr_regs: DMA_CHTBAR6:%#x\n"
	      "dbgpr_regs: DMA_CHTBAR5:%#x\n"
	      "dbgpr_regs: DMA_CHTBAR4:%#x\n"
	      "dbgpr_regs: DMA_CHTBAR3:%#x\n"
	      "dbgpr_regs: DMA_CHTBAR2:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_CHTBAR1_RgRd(val0);
	DMA_CHTBAR0_RgRd(val1);
	DMA_CHRDR7_RgRd(val2);
	DMA_CHRDR6_RgRd(val3);
	DMA_CHRDR5_RgRd(val4);
	DMA_CHRDR4_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_CHTBAR1:%#x\n"
	      "dbgpr_regs: DMA_CHTBAR0:%#x\n"
	      "dbgpr_regs: DMA_CHRDR7:%#x\n"
	      "dbgpr_regs: DMA_CHRDR6:%#x\n"
	      "dbgpr_regs: DMA_CHRDR5:%#x\n"
	      "dbgpr_regs: DMA_CHRDR4:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_CHRDR3_RgRd(val0);
	DMA_CHRDR2_RgRd(val1);
	DMA_CHRDR1_RgRd(val2);
	DMA_CHRDR0_RgRd(val3);
	DMA_CHTDR7_RgRd(val4);
	DMA_CHTDR6_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_CHRDR3:%#x\n"
	      "dbgpr_regs: DMA_CHRDR2:%#x\n"
	      "dbgpr_regs: DMA_CHRDR1:%#x\n"
	      "dbgpr_regs: DMA_CHRDR0:%#x\n"
	      "dbgpr_regs: DMA_CHTDR7:%#x\n"
	      "dbgpr_regs: DMA_CHTDR6:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_CHTDR5_RgRd(val0);
	DMA_CHTDR4_RgRd(val1);
	DMA_CHTDR3_RgRd(val2);
	DMA_CHTDR2_RgRd(val3);
	DMA_CHTDR1_RgRd(val4);
	DMA_CHTDR0_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_CHTDR5:%#x\n"
	      "dbgpr_regs: DMA_CHTDR4:%#x\n"
	      "dbgpr_regs: DMA_CHTDR3:%#x\n"
	      "dbgpr_regs: DMA_CHTDR2:%#x\n"
	      "dbgpr_regs: DMA_CHTDR1:%#x\n"
	      "dbgpr_regs: DMA_CHTDR0:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_SFCSR7_RgRd(val0);
	DMA_SFCSR6_RgRd(val1);
	DMA_SFCSR5_RgRd(val2);
	DMA_SFCSR4_RgRd(val3);
	DMA_SFCSR3_RgRd(val4);
	DMA_SFCSR2_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_SFCSR7:%#x\n"
	      "dbgpr_regs: DMA_SFCSR6:%#x\n"
	      "dbgpr_regs: DMA_SFCSR5:%#x\n"
	      "dbgpr_regs: DMA_SFCSR4:%#x\n"
	      "dbgpr_regs: DMA_SFCSR3:%#x\n"
	      "dbgpr_regs: DMA_SFCSR2:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_SFCSR1_RgRd(val0);
	DMA_SFCSR0_RgRd(val1);
	MAC_IVLANTIRR_RgRd(val2);
	MAC_VLANTIRR_RgRd(val3);
	MAC_VLANHTR_RgRd(val4);
	MAC_VLANTR_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_SFCSR1:%#x\n"
	      "dbgpr_regs: DMA_SFCSR0:%#x\n"
	      "dbgpr_regs: MAC_IVLANTIRR:%#x\n"
	      "dbgpr_regs: MAC_VLANTIRR:%#x\n"
	      "dbgpr_regs: MAC_VLANHTR:%#x\n"
	      "dbgpr_regs: MAC_VLANTR:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_SBUS_RgRd(val0);
	DMA_BMR_RgRd(val1);
	MTL_Q0RCR_RgRd(val2);
	MTL_Q0OCR_RgRd(val3);
	MTL_Q0ROMR_RgRd(val4);
	MTL_Q0QR_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_SBUS:%#x\n"
	      "dbgpr_regs: DMA_BMR:%#x\n"
	      "dbgpr_regs: MTL_Q0RCR:%#x\n"
	      "dbgpr_regs: MTL_Q0OCR:%#x\n"
	      "dbgpr_regs: MTL_Q0ROMR:%#x\n"
	      "dbgpr_regs: MTL_Q0QR:%#x\n", val0, val1, val2, val3, val4, val5);

	MTL_Q0ECR_RgRd(val0);
	MTL_Q0UCR_RgRd(val1);
	MTL_Q0TOMR_RgRd(val2);
	MTL_RQDCM1R_RgRd(val3);
	MTL_RQDCM0R_RgRd(val4);
	MTL_FDDR_RgRd(val5);

	DBGPR("dbgpr_regs: MTL_Q0ECR:%#x\n"
	      "dbgpr_regs: MTL_Q0UCR:%#x\n"
	      "dbgpr_regs: MTL_Q0TOMR:%#x\n"
	      "dbgpr_regs: MTL_RQDCM1R:%#x\n"
	      "dbgpr_regs: MTL_RQDCM0R:%#x\n"
	      "dbgpr_regs: MTL_FDDR:%#x\n", val0, val1, val2, val3, val4, val5);

	MTL_FDACS_RgRd(val0);
	MTL_OMR_RgRd(val1);
	MAC_RQC1R_RgRd(val2);
	MAC_RQC0R_RgRd(val3);
	MAC_TQPM1R_RgRd(val4);
	MAC_TQPM0R_RgRd(val5);

	DBGPR("dbgpr_regs: MTL_FDACS:%#x\n"
	      "dbgpr_regs: MTL_OMR:%#x\n"
	      "dbgpr_regs: MAC_RQC1R:%#x\n"
	      "dbgpr_regs: MAC_RQC0R:%#x\n"
	      "dbgpr_regs: MAC_TQPM1R:%#x\n"
	      "dbgpr_regs: MAC_TQPM0R:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MAC_RFCR_RgRd(val0);
	MAC_QTFCR7_RgRd(val1);
	MAC_QTFCR6_RgRd(val2);
	MAC_QTFCR5_RgRd(val3);
	MAC_QTFCR4_RgRd(val4);
	MAC_QTFCR3_RgRd(val5);

	DBGPR("dbgpr_regs: MAC_RFCR:%#x\n"
	      "dbgpr_regs: MAC_QTFCR7:%#x\n"
	      "dbgpr_regs: MAC_QTFCR6:%#x\n"
	      "dbgpr_regs: MAC_QTFCR5:%#x\n"
	      "dbgpr_regs: MAC_QTFCR4:%#x\n"
	      "dbgpr_regs: MAC_QTFCR3:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	MAC_QTFCR2_RgRd(val0);
	MAC_QTFCR1_RgRd(val1);
	MAC_Q0TFCR_RgRd(val2);
	DMA_AXI4CR7_RgRd(val3);
	DMA_AXI4CR6_RgRd(val4);
	DMA_AXI4CR5_RgRd(val5);

	DBGPR("dbgpr_regs: MAC_QTFCR2:%#x\n"
	      "dbgpr_regs: MAC_QTFCR1:%#x\n"
	      "dbgpr_regs: MAC_Q0TFCR:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR7:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR6:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR5:%#x\n",
	      val0, val1, val2, val3, val4, val5);

	DMA_AXI4CR4_RgRd(val0);
	DMA_AXI4CR3_RgRd(val1);
	DMA_AXI4CR2_RgRd(val2);
	DMA_AXI4CR1_RgRd(val3);
	DMA_AXI4CR0_RgRd(val4);
	DMA_RCR7_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_AXI4CR4:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR3:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR2:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR1:%#x\n"
	      "dbgpr_regs: DMA_AXI4CR0:%#x\n"
	      "dbgpr_regs: DMA_RCR7:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_RCR6_RgRd(val0);
	DMA_RCR5_RgRd(val1);
	DMA_RCR4_RgRd(val2);
	DMA_RCR3_RgRd(val3);
	DMA_RCR2_RgRd(val4);
	DMA_RCR1_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RCR6:%#x\n"
	      "dbgpr_regs: DMA_RCR5:%#x\n"
	      "dbgpr_regs: DMA_RCR4:%#x\n"
	      "dbgpr_regs: DMA_RCR3:%#x\n"
	      "dbgpr_regs: DMA_RCR2:%#x\n"
	      "dbgpr_regs: DMA_RCR1:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_RCR0_RgRd(val0);
	DMA_TCR7_RgRd(val1);
	DMA_TCR6_RgRd(val2);
	DMA_TCR5_RgRd(val3);
	DMA_TCR4_RgRd(val4);
	DMA_TCR3_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_RCR0:%#x\n"
	      "dbgpr_regs: DMA_TCR7:%#x\n"
	      "dbgpr_regs: DMA_TCR6:%#x\n"
	      "dbgpr_regs: DMA_TCR5:%#x\n"
	      "dbgpr_regs: DMA_TCR4:%#x\n"
	      "dbgpr_regs: DMA_TCR3:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_TCR2_RgRd(val0);
	DMA_TCR1_RgRd(val1);
	DMA_TCR0_RgRd(val2);
	DMA_CR7_RgRd(val3);
	DMA_CR6_RgRd(val4);
	DMA_CR5_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_TCR2:%#x\n"
	      "dbgpr_regs: DMA_TCR1:%#x\n"
	      "dbgpr_regs: DMA_TCR0:%#x\n"
	      "dbgpr_regs: DMA_CR7:%#x\n"
	      "dbgpr_regs: DMA_CR6:%#x\n"
	      "dbgpr_regs: DMA_CR5:%#x\n", val0, val1, val2, val3, val4, val5);

	DMA_CR4_RgRd(val0);
	DMA_CR3_RgRd(val1);
	DMA_CR2_RgRd(val2);
	DMA_CR1_RgRd(val3);
	DMA_CR0_RgRd(val4);
	MAC_WTR_RgRd(val5);

	DBGPR("dbgpr_regs: DMA_CR4:%#x\n"
	      "dbgpr_regs: DMA_CR3:%#x\n"
	      "dbgpr_regs: DMA_CR2:%#x\n"
	      "dbgpr_regs: DMA_CR1:%#x\n"
	      "dbgpr_regs: DMA_CR0:%#x\n"
	      "dbgpr_regs: MAC_WTR:%#x\n", val0, val1, val2, val3, val4, val5);

	MAC_MPFR_RgRd(val0);
	MAC_MECR_RgRd(val1);
	MAC_MCR_RgRd(val2);

	DBGPR("dbgpr_regs: MAC_MPFR:%#x\n"
	      "dbgpr_regs: MAC_MECR:%#x\n"
	      "dbgpr_regs: MAC_MCR:%#x\n", val0, val1, val2);

	return;
}
#endif

/*!
 * \details This function is invoked by DWC_ETH_QOS_start_xmit and
 * DWC_ETH_QOS_tx_interrupt function for dumping the TX descriptor contents
 * which are prepared for packet transmission and which are transmitted by
 * device. It is mainly used during development phase for debug purpose. Use
 * of these function may affect the performance during normal operation.
 *
 * \param[in] pdata – pointer to private data structure.
 * \param[in] first_desc_idx – first descriptor index for the current
 *		transfer.
 * \param[in] last_desc_idx – last descriptor index for the current transfer.
 * \param[in] flag – to indicate from which function it is called.
 *
 * \return void
 */

void dump_tx_desc(struct DWC_ETH_QOS_prv_data *pdata, int first_desc_idx,
		  int last_desc_idx, int flag, UINT chInx)
{
	int i;
	struct s_TX_NORMAL_DESC *desc = NULL;
	UINT varCTXT;

	NTN_PRINT_TS;
	if (first_desc_idx == last_desc_idx) {
		desc = GET_TX_DESC_PTR(chInx, first_desc_idx);

		TX_NORMAL_DESC_TDES3_CTXT_Mlf_Rd(desc->TDES3, varCTXT);

		NMSGPR_ALERT( "\n%s[%02d %4p %03d %s] = %#x:%#x:%#x:%#x\n",
		       (varCTXT == 1) ? "TX_CONTXT_DESC" : "TX_NORMAL_DESC",
		       chInx, desc, first_desc_idx,
		       ((flag == 1) ? "QUEUED FOR TRANSMISSION" :
			((flag == 0) ? "FREED/FETCHED BY DEVICE" : "DEBUG DESC DUMP")),
			desc->TDES0, desc->TDES1,
			desc->TDES2, desc->TDES3);
	} else {
		int lp_cnt;
		if (first_desc_idx > last_desc_idx)
			lp_cnt = last_desc_idx + pdata->tx_dma_ch[chInx].desc_cnt - first_desc_idx;
		else
			lp_cnt = last_desc_idx - first_desc_idx;

		for (i = first_desc_idx; lp_cnt >= 0; lp_cnt--) {
			desc = GET_TX_DESC_PTR(chInx, i);

			TX_NORMAL_DESC_TDES3_CTXT_Mlf_Rd(desc->TDES3, varCTXT);

			NMSGPR_ALERT( "\n%s[%02d %4p %03d %s] = %#x:%#x:%#x:%#x\n",
			       (varCTXT == 1) ? "TX_CONTXT_DESC" : "TX_NORMAL_DESC",
			       chInx, desc, i,
			       ((flag == 1) ? "QUEUED FOR TRANSMISSION" :
				"FREED/FETCHED BY DEVICE"), desc->TDES0,
			       desc->TDES1, desc->TDES2, desc->TDES3);
			INCR_TX_DESC_INDEX(i, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		}
	}
}

/*!
 * \details This function is invoked by poll function for dumping the
 * RX descriptor contents. It is mainly used during development phase for
 * debug purpose. Use of these function may affect the performance during
 * normal operation
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

void dump_rx_desc(UINT chInx, struct s_RX_NORMAL_DESC *desc, int desc_idx)
{
	NTN_PRINT_TS;
	NMSGPR_ALERT( "\nRX_NORMAL_DESC[%02d %4p %03d RECEIVED FROM DEVICE]"\
		" = %#x:%#x:%#x:%#x\n",
		chInx, desc, desc_idx, desc->RDES0, desc->RDES1,
		desc->RDES2, desc->RDES3);
}

/*!
 * \details This function is invoked by start_xmit and poll function for
 * dumping the content of packet to be transmitted by device or received
 * from device. It is mainly used during development phase for debug purpose.
 * Use of these functions may affect the performance during normal operation.
 *
 * \param[in] skb – pointer to socket buffer structure.
 * \param[in] len – length of packet to be transmitted/received.
 * \param[in] tx_rx – packet to be transmitted or received.
 * \param[in] desc_idx – descriptor index to be used for transmission or
 *			reception of packet.
 *
 * \return void
 */

void print_pkt(struct sk_buff *skb, int len, bool tx_rx, int desc_idx)
{
	int i, j = 0;
	unsigned char *buf = skb->data;

	NMSGPR_ALERT(
	       "\n\n/***********************************************************/\n");

	NMSGPR_ALERT( "%s pkt of %d Bytes [DESC index = %d]\n\n",
	       (tx_rx ? "TX" : "RX"), len, desc_idx);
	NMSGPR_ALERT( "Dst MAC addr(6 bytes)\n");
	for (i = 0; i < 6; i++)
		printk("%#.2x%s", buf[i], (((i == 5) ? "" : ":")));
	NMSGPR_ALERT( "\nSrc MAC addr(6 bytes)\n");
	for (i = 6; i <= 11; i++)
		printk("%#.2x%s", buf[i], (((i == 11) ? "" : ":")));
	i = (buf[12] << 8 | buf[13]);
	NMSGPR_ALERT( "\nType/Length(2 bytes)\n%#x", i);

	NMSGPR_ALERT( "\nPay Load : %d bytes\n", (len - 14));
	for (i = 14, j = 1; i < len; i++, j++) {
		printk("%#.2x%s", buf[i], (((i == (len - 1)) ? "" : ":")));
		if ((j % 16) == 0)
			NMSGPR_ALERT( "");
	}

	NMSGPR_ALERT(
	       "/*************************************************************/\n\n");
}

#ifdef NTN_DRV_TEST_LOOPBACK
void update_pkt_lp(struct sk_buff *skb)
{
	UINT eth_or_vlan_tag;
	UINT eth_type;

	eth_or_vlan_tag = htons(((skb->data[13]<<8) | skb->data[12]));
	if(eth_or_vlan_tag == NTN_VLAN_TAG)
	{
		eth_type = htons(((skb->data[17]<<8) | skb->data[16]));
		//4Bytes VID. Data starts from [19]. Modify for Loopback
		//skb->data[18] = skb->data[19] = skb->data[20] = skb->data[21] = 0xFF;
		skb->data[19] = 0xFF;
	}
	else
	{
		eth_type = eth_or_vlan_tag;
		/* It's protcol (ether type) field */
		if (eth_type == NTN_GPTP_ETH_TYPE){
			//No VID. GPTP Data starts from [16]. Modify for Loopback
			skb->data[16] = 0xFF;
		} else if (eth_type == NTN_ETH_TYPE_AVB) {
			//No VID. AVB Control. Data starts from [15]. Modify for Loopback
			skb->data[15] = 0xFF;
		} else {
			//No VID. Data starts from [14]. Modify for Loopback
			skb->data[14] = 0xFF;
		}
	}
	NMSGPR_ALERT("%s:RX Channel skb->len = %d , protocol = %#x \n", __func__, skb->len, eth_type);
}
#endif


/*!
 * \details This function is invoked by probe function. This function will
 * initialize default receive coalesce parameters and sw timer value and store
 * it in respective receive data structure.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

void DWC_ETH_QOS_init_rx_coalesce(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data = NULL;
	UINT chInx;

	DBGPR("-->DWC_ETH_QOS_init_rx_coalesce\n");

	for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		rx_desc_data = GET_RX_WRAPPER_DESC(chInx);

		rx_desc_data->use_riwt = 1;
		rx_desc_data->rx_coal_frames = DWC_ETH_QOS_RX_MAX_FRAMES;

		if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0)
			rx_desc_data->rx_riwt = DWC_ETH_QOS_MAX_DMA_RIWT;
		else
			rx_desc_data->rx_riwt =
				DWC_ETH_QOS_usec2riwt(DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC, pdata);
	}

	DBGPR("<--DWC_ETH_QOS_init_rx_coalesce\n");
}


/*!
 * \details This function is invoked by open() function. This function will
 * clear MMC structure.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_mmc_setup(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->DWC_ETH_QOS_mmc_setup\n");

	if (pdata->hw_feat.mmc_sel) {
		memset(&pdata->mmc, 0, sizeof(struct DWC_ETH_QOS_mmc_counters));
	} else
		NMSGPR_ALERT( "No MMC/RMON module available in the HW\n");

	DBGPR("<--DWC_ETH_QOS_mmc_setup\n");
}

inline unsigned int DWC_ETH_QOS_reg_read(volatile ULONG *ptr)
{
	return ioread32((void *)ptr);
}

inline unsigned long long DWC_ETH_QOS_reg_read64(volatile ULONG *ptr)
{
	return(le64_to_cpu(cpu_to_le32(ioread32((void *)ptr)) | ((u64)(cpu_to_le32(ioread32((void *)ptr + sizeof(u32)))) << 32)));
}


/*!
 * \details This function is invoked by ethtool function when user wants to
 * read MMC counters. This function will read the MMC if supported by core
 * and store it in DWC_ETH_QOS_mmc_counters structure. By default all the
 * MMC are programmed "read on reset" hence all the fields of the
 * DWC_ETH_QOS_mmc_counters are incremented.
 *
 * open() function. This function will
 * initialize MMC control register ie it disable all MMC interrupt and all
 * MMC register are configured to clear on read.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

void DWC_ETH_QOS_mmc_read(struct DWC_ETH_QOS_mmc_counters *mmc, struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->DWC_ETH_QOS_mmc_read\n");

	/* MMC TX counter registers */
	mmc->mmc_tx_octetcount_gb += DWC_ETH_QOS_reg_read(MMC_TXOCTETCOUNT_GB_RgOffAddr);
	mmc->mmc_tx_framecount_gb += DWC_ETH_QOS_reg_read(MMC_TXPACKETCOUNT_GB_RgOffAddr);
	mmc->mmc_tx_broadcastframe_g += DWC_ETH_QOS_reg_read(MMC_TXBROADCASTPACKETS_G_RgOffAddr);
	mmc->mmc_tx_multicastframe_g += DWC_ETH_QOS_reg_read(MMC_TXMULTICASTPACKETS_G_RgOffAddr);
	mmc->mmc_tx_64_octets_gb += DWC_ETH_QOS_reg_read(MMC_TX64OCTETS_GB_RgOffAddr);
	mmc->mmc_tx_65_to_127_octets_gb += DWC_ETH_QOS_reg_read(MMC_TX65TO127OCTETS_GB_RgOffAddr);
	mmc->mmc_tx_128_to_255_octets_gb += DWC_ETH_QOS_reg_read(MMC_TX128TO255OCTETS_GB_RgOffAddr);
	mmc->mmc_tx_256_to_511_octets_gb += DWC_ETH_QOS_reg_read(MMC_TX256TO511OCTETS_GB_RgOffAddr);
	mmc->mmc_tx_512_to_1023_octets_gb += DWC_ETH_QOS_reg_read(MMC_TX512TO1023OCTETS_GB_RgOffAddr);
	mmc->mmc_tx_1024_to_max_octets_gb += DWC_ETH_QOS_reg_read(MMC_TX1024TOMAXOCTETS_GB_RgOffAddr);
	mmc->mmc_tx_unicast_gb += DWC_ETH_QOS_reg_read(MMC_TXUNICASTPACKETS_GB_RgOffAddr);
	mmc->mmc_tx_multicast_gb += DWC_ETH_QOS_reg_read(MMC_TXMULTICASTPACKETS_GB_RgOffAddr);
	mmc->mmc_tx_broadcast_gb += DWC_ETH_QOS_reg_read(MMC_TXBROADCASTPACKETS_GB_RgOffAddr);
	mmc->mmc_tx_underflow_error += DWC_ETH_QOS_reg_read(MMC_TXUNDERFLOWERROR_RgOffAddr);
	mmc->mmc_tx_singlecol_g += DWC_ETH_QOS_reg_read(MMC_TXSINGLECOL_G_RgOffAddr);
	mmc->mmc_tx_multicol_g += DWC_ETH_QOS_reg_read(MMC_TXMULTICOL_G_RgOffAddr);
	mmc->mmc_tx_deferred += DWC_ETH_QOS_reg_read(MMC_TXDEFERRED_RgOffAddr);
	mmc->mmc_tx_latecol += DWC_ETH_QOS_reg_read(MMC_TXLATECOL_RgOffAddr);
	mmc->mmc_tx_exesscol += DWC_ETH_QOS_reg_read(MMC_TXEXESSCOL_RgOffAddr);
	mmc->mmc_tx_carrier_error += DWC_ETH_QOS_reg_read(MMC_TXCARRIERERROR_RgOffAddr);
	mmc->mmc_tx_octetcount_g += DWC_ETH_QOS_reg_read(MMC_TXOCTETCOUNT_G_RgOffAddr);
	mmc->mmc_tx_framecount_g += DWC_ETH_QOS_reg_read(MMC_TXPACKETSCOUNT_G_RgOffAddr);
	mmc->mmc_tx_excessdef += DWC_ETH_QOS_reg_read(MMC_TXEXCESSDEF_RgOffAddr);
	mmc->mmc_tx_pause_frame += DWC_ETH_QOS_reg_read(MMC_TXPAUSEPACKETS_RgOffAddr);
	mmc->mmc_tx_vlan_frame_g += DWC_ETH_QOS_reg_read(MMC_TXVLANPACKETS_G_RgOffAddr);
	mmc->mmc_tx_osize_frame_g += DWC_ETH_QOS_reg_read(MMC_TXOVERSIZE_G_RgOffAddr);

	/* MMC RX counter registers */
	mmc->mmc_rx_framecount_gb += DWC_ETH_QOS_reg_read(MMC_RXPACKETCOUNT_GB_RgOffAddr);
	mmc->mmc_rx_octetcount_gb += DWC_ETH_QOS_reg_read(MMC_RXOCTETCOUNT_GB_RgOffAddr);
	mmc->mmc_rx_octetcount_g += DWC_ETH_QOS_reg_read(MMC_RXOCTETCOUNT_G_RgOffAddr);
	mmc->mmc_rx_broadcastframe_g += DWC_ETH_QOS_reg_read(MMC_RXBROADCASTPACKETS_G_RgOffAddr);
	mmc->mmc_rx_multicastframe_g += DWC_ETH_QOS_reg_read(MMC_RXMULTICASTPACKETS_G_RgOffAddr);
	mmc->mmc_rx_crc_errror += DWC_ETH_QOS_reg_read(MMC_RXCRCERROR_RgOffAddr);
	mmc->mmc_rx_align_error += DWC_ETH_QOS_reg_read(MMC_RXALIGNMENTERROR_RgOffAddr);
	mmc->mmc_rx_run_error += DWC_ETH_QOS_reg_read(MMC_RXRUNTERROR_RgOffAddr);
	mmc->mmc_rx_jabber_error += DWC_ETH_QOS_reg_read(MMC_RXJABBERERROR_RgOffAddr);
	mmc->mmc_rx_undersize_g += DWC_ETH_QOS_reg_read(MMC_RXUNDERSIZE_G_RgOffAddr);
	mmc->mmc_rx_oversize_g += DWC_ETH_QOS_reg_read(MMC_RXOVERSIZE_G_RgOffAddr);
	mmc->mmc_rx_64_octets_gb += DWC_ETH_QOS_reg_read(MMC_RX64OCTETS_GB_RgOffAddr);
	mmc->mmc_rx_65_to_127_octets_gb += DWC_ETH_QOS_reg_read(MMC_RX65TO127OCTETS_GB_RgOffAddr);
	mmc->mmc_rx_128_to_255_octets_gb += DWC_ETH_QOS_reg_read(MMC_RX128TO255OCTETS_GB_RgOffAddr);
	mmc->mmc_rx_256_to_511_octets_gb += DWC_ETH_QOS_reg_read(MMC_RX256TO511OCTETS_GB_RgOffAddr);
	mmc->mmc_rx_512_to_1023_octets_gb += DWC_ETH_QOS_reg_read(MMC_RX512TO1023OCTETS_GB_RgOffAddr);
	mmc->mmc_rx_1024_to_max_octets_gb += DWC_ETH_QOS_reg_read(MMC_RX1024TOMAXOCTETS_GB_RgOffAddr);
	mmc->mmc_rx_unicast_g += DWC_ETH_QOS_reg_read(MMC_RXUNICASTPACKETS_G_RgOffAddr);
	mmc->mmc_rx_length_error += DWC_ETH_QOS_reg_read(MMC_RXLENGTHERROR_RgOffAddr);
	mmc->mmc_rx_outofrangetype += DWC_ETH_QOS_reg_read(MMC_RXOUTOFRANGETYPE_RgOffAddr);
	mmc->mmc_rx_pause_frames += DWC_ETH_QOS_reg_read(MMC_RXPAUSEPACKETS_RgOffAddr);
	mmc->mmc_rx_fifo_overflow += DWC_ETH_QOS_reg_read(MMC_RXFIFOOVERFLOW_RgOffAddr);
	mmc->mmc_rx_vlan_frames_gb += DWC_ETH_QOS_reg_read(MMC_RXVLANPACKETS_GB_RgOffAddr);
	mmc->mmc_rx_watchdog_error += DWC_ETH_QOS_reg_read(MMC_RXWATCHDOGERROR_RgOffAddr);
	mmc->mmc_rx_receive_error += DWC_ETH_QOS_reg_read(MMC_RXRCVERROR_RgOffAddr);
	mmc->mmc_rx_ctrl_frames_g += DWC_ETH_QOS_reg_read(MMC_RXCTRLPACKETS_G_RgOffAddr);

	/* IPC */
	mmc->mmc_rx_ipc_intr_mask += DWC_ETH_QOS_reg_read(MMC_IPC_INTR_MASK_RX_RgOffAddr);
	mmc->mmc_rx_ipc_intr += DWC_ETH_QOS_reg_read(MMC_IPC_INTR_RX_RgOffAddr);

	/* IPv4 */
	mmc->mmc_rx_ipv4_gd += DWC_ETH_QOS_reg_read(MMC_RXIPV4_GD_PKTS_RgOffAddr);
	mmc->mmc_rx_ipv4_hderr += DWC_ETH_QOS_reg_read(MMC_RXIPV4_HDRERR_PKTS_RgOffAddr);
	mmc->mmc_rx_ipv4_nopay += DWC_ETH_QOS_reg_read(MMC_RXIPV4_NOPAY_PKTS_RgOffAddr);
	mmc->mmc_rx_ipv4_frag += DWC_ETH_QOS_reg_read(MMC_RXIPV4_FRAG_PKTS_RgOffAddr);
	mmc->mmc_rx_ipv4_udsbl += DWC_ETH_QOS_reg_read(MMC_RXIPV4_UBSBL_PKTS_RgOffAddr);

	/* IPV6 */
	mmc->mmc_rx_ipv6_gd += DWC_ETH_QOS_reg_read(MMC_RXIPV6_GD_PKTS_RgOffAddr);
	mmc->mmc_rx_ipv6_hderr += DWC_ETH_QOS_reg_read(MMC_RXIPV6_HDRERR_PKTS_RgOffAddr);
	mmc->mmc_rx_ipv6_nopay += DWC_ETH_QOS_reg_read(MMC_RXIPV6_NOPAY_PKTS_RgOffAddr);

	/* Protocols */
	mmc->mmc_rx_udp_gd += DWC_ETH_QOS_reg_read(MMC_RXUDP_GD_PKTS_RgOffAddr);
	mmc->mmc_rx_udp_err += DWC_ETH_QOS_reg_read(MMC_RXUDP_ERR_PKTS_RgOffAddr);
	mmc->mmc_rx_tcp_gd += DWC_ETH_QOS_reg_read(MMC_RXTCP_GD_PKTS_RgOffAddr);
	mmc->mmc_rx_tcp_err += DWC_ETH_QOS_reg_read(MMC_RXTCP_ERR_PKTS_RgOffAddr);
	mmc->mmc_rx_icmp_gd += DWC_ETH_QOS_reg_read(MMC_RXICMP_GD_PKTS_RgOffAddr);
	mmc->mmc_rx_icmp_err += DWC_ETH_QOS_reg_read(MMC_RXICMP_ERR_PKTS_RgOffAddr);

	/* IPv4 */
	mmc->mmc_rx_ipv4_gd_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV4_GD_OCTETS_RgOffAddr);
	mmc->mmc_rx_ipv4_hderr_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV4_HDRERR_OCTETS_RgOffAddr);
	mmc->mmc_rx_ipv4_nopay_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV4_NOPAY_OCTETS_RgOffAddr);
	mmc->mmc_rx_ipv4_frag_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV4_FRAG_OCTETS_RgOffAddr);
	mmc->mmc_rx_ipv4_udsbl_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV4_UDSBL_OCTETS_RgOffAddr);

	/* IPV6 */
	mmc->mmc_rx_ipv6_gd_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV6_GD_OCTETS_RgOffAddr);
	mmc->mmc_rx_ipv6_hderr_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV6_HDRERR_OCTETS_RgOffAddr);
	mmc->mmc_rx_ipv6_nopay_octets += DWC_ETH_QOS_reg_read(MMC_RXIPV6_NOPAY_OCTETS_RgOffAddr);

	/* Protocols */
	mmc->mmc_rx_udp_gd_octets += DWC_ETH_QOS_reg_read(MMC_RXUDP_GD_OCTETS_RgOffAddr);
	mmc->mmc_rx_udp_err_octets += DWC_ETH_QOS_reg_read(MMC_RXUDP_ERR_OCTETS_RgOffAddr);
	mmc->mmc_rx_tcp_gd_octets += DWC_ETH_QOS_reg_read(MMC_RXTCP_GD_OCTETS_RgOffAddr);
	mmc->mmc_rx_tcp_err_octets += DWC_ETH_QOS_reg_read(MMC_RXTCP_ERR_OCTETS_RgOffAddr);
	mmc->mmc_rx_icmp_gd_octets += DWC_ETH_QOS_reg_read(MMC_RXICMP_GD_OCTETS_RgOffAddr);
	mmc->mmc_rx_icmp_err_octets += DWC_ETH_QOS_reg_read(MMC_RXICMP_ERR_OCTETS_RgOffAddr);

	DBGPR("<--DWC_ETH_QOS_mmc_read\n");
}


/*!
 * \details This function is invoked by ethtool function when user wants to
 * read DMA Descriptor stats.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

void DWC_ETH_QOS_desc_stats_read(struct DWC_ETH_QOS_prv_data *pdata)
{
	int chInx = 0;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data;
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data;
	DBGPR("-->DWC_ETH_QOS_desc_stats_read\n");


	/* TX DMA Descriptors Status for all channels [0-4] */
	for(chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++){
		if(!pdata->tx_dma_ch_for_host[chInx])
			continue;

		tx_desc_data = GET_TX_WRAPPER_DESC(chInx);
		pdata->xstats.txch_status[chInx] =          DWC_ETH_QOS_reg_read(DMA_TXCHSTS_RgOffAddr(chInx));
		pdata->xstats.txch_control[chInx] =         DWC_ETH_QOS_reg_read(DMA_TXCHCTL_RgOffAddr(chInx));
		pdata->xstats.txch_intrmask[chInx] =        DWC_ETH_QOS_reg_read(DMA_TXCHINTMASK_RgOffAddr(chInx));
		pdata->xstats.txch_desc_list_haddr[chInx] = DWC_ETH_QOS_reg_read(DMA_TXCH_DESC_LISTHADDR_RgOffAddr(chInx));
		pdata->xstats.txch_desc_list_laddr[chInx] = DWC_ETH_QOS_reg_read(DMA_TXCH_DESC_LISTLADDR_RgOffAddr(chInx));
		pdata->xstats.txch_desc_ring_len[chInx] =   DWC_ETH_QOS_reg_read(DMA_TXCH_DESC_RING_LENGTH_RgOffAddr(chInx));
		pdata->xstats.txch_desc_curr[chInx] =       DWC_ETH_QOS_reg_read(DMA_TXCH_CUR_DESC_RgOffAddr(chInx));
		pdata->xstats.txch_desc_tail[chInx] =       DWC_ETH_QOS_reg_read(DMA_TXCH_DESC_TAILPTR_RgOffAddr(chInx));
		pdata->xstats.txch_desc_buf_haddr[chInx] =  DWC_ETH_QOS_reg_read(DMA_TXCH_CUR_BUFHA_RgOffAddr(chInx));
		pdata->xstats.txch_desc_buf_laddr[chInx] =  DWC_ETH_QOS_reg_read(DMA_TXCH_CUR_BUFLA_RgOffAddr(chInx));
		pdata->xstats.txch_sw_cur_tx[chInx] =       tx_desc_data->cur_tx;
		pdata->xstats.txch_sw_dirty_tx[chInx] =     tx_desc_data->dirty_tx;
	}

	/* RX DMA Descriptors Status for all channels [0-5] */
	for(chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;

		rx_desc_data = GET_RX_WRAPPER_DESC(chInx);
		pdata->xstats.rxch_status[chInx] =          DWC_ETH_QOS_reg_read(DMA_RXCHSTS_RgOffAddr(chInx));
		pdata->xstats.rxch_control[chInx] =         DWC_ETH_QOS_reg_read(DMA_RXCHCTL_RgOffAddr(chInx));
		pdata->xstats.rxch_intrmask[chInx] =        DWC_ETH_QOS_reg_read(DMA_RXCHINTMASK_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_list_haddr[chInx] = DWC_ETH_QOS_reg_read(DMA_RXCH_DESC_LISTHADDR_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_list_laddr[chInx] = DWC_ETH_QOS_reg_read(DMA_RXCH_DESC_LISTLADDR_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_ring_len[chInx] =   DWC_ETH_QOS_reg_read(DMA_RXCH_DESC_RING_LENGTH_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_curr[chInx] =       DWC_ETH_QOS_reg_read(DMA_RXCH_CUR_DESC_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_tail[chInx] =       DWC_ETH_QOS_reg_read(DMA_RXCH_DESC_TAILPTR_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_buf_haddr[chInx] =  DWC_ETH_QOS_reg_read(DMA_RXCH_CUR_BUFHA_RgOffAddr(chInx));
		pdata->xstats.rxch_desc_buf_laddr[chInx] =  DWC_ETH_QOS_reg_read(DMA_RXCH_CUR_BUFLA_RgOffAddr(chInx));
		pdata->xstats.rxch_sw_cur_rx[chInx] =       rx_desc_data->cur_rx;
		pdata->xstats.rxch_sw_dirty_rx[chInx] =     rx_desc_data->dirty_rx;
	}

	DBGPR("<--DWC_ETH_QOS_desc_stats_read\n");
}

/*!
 * \details This function is invoked by ethtool function when user wants to
 * read M3 Firmware stats (MSI Count, M3 Tick, M3 WDT).
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

void DWC_ETH_QOS_m3fw_stats_read(struct DWC_ETH_QOS_prv_data *pdata)
{
	int chInx = 0;
	DBGPR("-->DWC_ETH_QOS_m3fw_stats_read\n");

	/*M3 Firmware Counters*/
	pdata->xstats.m3_tick_cnt = DWC_ETH_QOS_reg_read64(M3STAT_M3TICK_MemOffAddr);
	pdata->xstats.m3_wdt_cnt =  DWC_ETH_QOS_reg_read(M3STAT_WDT_MemOffAddr);

	/* TX DMA MSI Interrupt for all channels [0-4] */
	for(chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++){
		if(!pdata->tx_dma_ch_for_host[chInx])
			continue;
		pdata->xstats.m3_msi_txch[chInx]= DWC_ETH_QOS_reg_read(M3STAT_MSI_CNT_TXCH_MemOffAddr(chInx));
	}

	/* RX DMA MSI Interrupt for all channels [0-5] */
	for(chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		if(!pdata->rx_dma_ch_for_host[chInx])
			continue;
		pdata->xstats.m3_msi_rxch[chInx]= DWC_ETH_QOS_reg_read(M3STAT_MSI_CNT_RXCH_MemOffAddr(chInx));
	}
	DBGPR("<--DWC_ETH_QOS_m3fw_stats_read\n");
}

/*!
 * \details This function is invoked by the Probe Function before the ethtool registration
 * It resets/initializes the Ethtool stats.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */
void DWC_ETH_QOS_reset_ethtool_stats(struct DWC_ETH_QOS_prv_data *pdata)
{
	int chInx = 0;
	DBGPR("-->DWC_ETH_QOS_reset_ethtool_stats\n");

	/* TX DMA Descriptors Status for all channels [0-4] */
	for(chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++){
		pdata->xstats.txch_status[chInx] = 0;
		pdata->xstats.txch_control[chInx] = 0;
		pdata->xstats.txch_intrmask[chInx] = 0;
		pdata->xstats.txch_desc_list_haddr[chInx] = 0;
		pdata->xstats.txch_desc_list_laddr[chInx] = 0;
		pdata->xstats.txch_desc_ring_len[chInx] = 0;
		pdata->xstats.txch_desc_curr[chInx] = 0;
		pdata->xstats.txch_desc_tail[chInx] = 0;
		pdata->xstats.txch_desc_buf_haddr[chInx] = 0;
		pdata->xstats.txch_desc_buf_laddr[chInx] = 0;
		pdata->xstats.txch_sw_cur_tx[chInx] = 0;
		pdata->xstats.txch_sw_dirty_tx[chInx] = 0;

		pdata->xstats.m3_msi_txch[chInx]= 0;
	}

	/* RX DMA Descriptors Status for all channels [0-5] */
	for(chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++){
		pdata->xstats.rxch_status[chInx] = 0;
		pdata->xstats.rxch_control[chInx] = 0;
		pdata->xstats.rxch_intrmask[chInx] = 0;
		pdata->xstats.rxch_desc_list_haddr[chInx] =  0;
		pdata->xstats.rxch_desc_list_laddr[chInx] =  0;
		pdata->xstats.rxch_desc_ring_len[chInx] =  0;
		pdata->xstats.rxch_desc_curr[chInx] =  0;
		pdata->xstats.rxch_desc_tail[chInx] =  0;
		pdata->xstats.rxch_desc_buf_haddr[chInx] =  0;
		pdata->xstats.rxch_desc_buf_laddr[chInx] =  0;
		pdata->xstats.rxch_sw_cur_rx[chInx] =  0;
		pdata->xstats.rxch_sw_dirty_rx[chInx] =  0;

		pdata->xstats.m3_msi_rxch[chInx]= 0;
	}

	/*M3 Firmware Counters*/
	pdata->xstats.m3_tick_cnt = 0;
	pdata->xstats.m3_wdt_cnt = 0;

	DBGPR("<--DWC_ETH_QOS_reset_ethtool_stats\n");
}

phy_interface_t DWC_ETH_QOS_get_phy_interface(struct DWC_ETH_QOS_prv_data *pdata)
{
	phy_interface_t ret = PHY_INTERFACE_MODE_MII;

	DBGPR("-->DWC_ETH_QOS_get_phy_interface\n");

	if (pdata->rmii_mode || pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_RMII) {
		NDBGPR_L2("Phy is RMII\n");
		ret = PHY_INTERFACE_MODE_RMII;
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_GMII_MII) {
		if (pdata->hw_feat.gmii_sel){
			NDBGPR_L2("Phy is GMII\n");
			ret = PHY_INTERFACE_MODE_GMII;
		}
		else if (pdata->hw_feat.mii_sel){
			NDBGPR_L2("Phy is MII\n");
			ret = PHY_INTERFACE_MODE_MII;
		}
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_RGMII) {
		NDBGPR_L2("Phy is RGMII\n");
		ret = PHY_INTERFACE_MODE_RGMII;
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_SGMII) {
		NDBGPR_L2("Phy is SGMII\n");
		ret = PHY_INTERFACE_MODE_SGMII;
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_TBI) {
		NDBGPR_L2("Phy is TBI\n");
		ret = PHY_INTERFACE_MODE_TBI;
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_RTBI) {
		NDBGPR_L2("Phy is RTBI\n");
		ret = PHY_INTERFACE_MODE_RTBI;
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_SMII) {
		NDBGPR_L2("Phy is SMII\n");
		ret = PHY_INTERFACE_MODE_SMII;
	} else if (pdata->hw_feat.act_phy_sel == DWC_ETH_QOS_RevMII) {
		NMSGPR_ALERT( "Phy is RevMII: Not supported\n");
		//what to return ?
	} else {
		NMSGPR_ALERT( "Missing interface support between"\
		    "PHY and MAC\n\n");
		ret = PHY_INTERFACE_MODE_NA;
	}

	DBGPR("<--DWC_ETH_QOS_get_phy_interface\n");

	return ret;
}

