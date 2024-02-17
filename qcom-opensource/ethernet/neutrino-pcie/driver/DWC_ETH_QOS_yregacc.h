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
 *     21-March-2016 : Renamed eMAC TX clock config MACROs,
 *                     Added eMAC RX clock config MACROs
 *      3-Jun-2016   : New statistics added to Ethtool for EMAC TX/RX DMA Descriptor Status & M3 MSI Generation Statistics (SRAM).
 *                   : Enabled WDT in INTC and added a counter in M3 Firmware(FW_OSLESS) for WDT half expiry interrupts.
 */

#ifndef __DWC_ETH_QOS__REGACC__H__

#define __DWC_ETH_QOS__REGACC__H__


#define MAKE_MASK_32(e, s) (((e)-(s))==31?0xffffffffUL:((1UL<<((e)-(s)+1))-1))

#define MAKE_MASK_64(e, s) (((e)-(s))==63?0xffffffffffffffffULL:((1ULL<<((e)-(s)+1))-1))

#define GET_BITS(e, s, reg, data) \
  (data = ((e)-(s) > 31) ?\
    (((reg)>>(s))&MAKE_MASK_64(e,s)) :\
    (((reg)>>(s))&MAKE_MASK_32(e,s)))

#define SET_BITS(e, s, reg, val) do { \
  if((e)-(s) > 31) { \
    reg = ( (((val)<<(s))&(MAKE_MASK_64((e),(s))<<(s))) | ((reg)&(~(MAKE_MASK_64((e),(s))<<(s)))) ); \
  } \
  else { \
    reg = ( (((val)<<(s))&(MAKE_MASK_32((e),(s))<<(s))) | ((reg)&(~(MAKE_MASK_32((e),(s))<<(s)))) ); \
  } \
} while(0)

/******************************************************************************/
/**                          Neutrino Base Address                           **/
/******************************************************************************/
#define  PCIE_OFFSET        (0x20000)
#define  MAC_OFFSET         (0xA000)
#define  INTC_OFFSET        (0x8000)

#define  NTN_REG_BASE_ADRS  (pdata->dev->base_addr)

#define  NTN_REG_PHY_BASE_ADRS  (pdata->dwc_eth_ntn_reg_pci_base_addr_phy)
#define  NTN_SRAM_BASE_ADRS (pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt)

#define  BASE_ADDRESS       (NTN_REG_BASE_ADRS + MAC_OFFSET)
#define  MAC_BASE_ADDRESS   (NTN_REG_BASE_ADRS + MAC_OFFSET)
#define  INTC_BASE_ADDRESS  (NTN_REG_BASE_ADRS + INTC_OFFSET)
#define  PCIE_BASE_ADDRESS  (NTN_REG_BASE_ADRS + PCIE_OFFSET)


#define  WRAPPER_OFFSET             (0x3000)
#define  WRAPPER_DMA_OFFSET         (0x3100)
#define  WRAPPER_DMA_TX_CH_OFFSET   (0x3200)
#define  WRAPPER_DMA_RX_CH_OFFSET   (0x3800)
#define  DMA_INTER_CH_OFFSET        (0x40)

#define  M3_SRAM_DEBUG_OFFSET       (0xC800)  // Debugging count SRAM area start address
#define  M3STAT_MSI_CNT_TXCH_OFFSET (0x00)
#define  M3STAT_MSI_MAC_LPI_OFFSET  (0x14)
#define  M3STAT_MSI_CNT_RXCH_OFFSET (0x18)
#define  M3STAT_MSI_TDM_OFFSET      (0x30)
#define  M3STAT_MSI_CAN_OFFSET      (0x34)
#define  M3STAT_RESV_OFFSET         (0x38)
#define  M3STAT_M3TICK_OFFSET       (0x3C)
#define  M3STAT_WDT_OFFSET          (0x44)

#define  DMA_BASE_ADDRESS           (NTN_REG_BASE_ADRS + WRAPPER_DMA_OFFSET)
#define  DMA_TX_CH_BASE_ADDRESS     (NTN_REG_BASE_ADRS + WRAPPER_DMA_TX_CH_OFFSET)
#define  DMA_RX_CH_BASE_ADDRESS     (NTN_REG_BASE_ADRS + WRAPPER_DMA_RX_CH_OFFSET)
#define  M3STAT_BASE_ADDRESS        (NTN_SRAM_BASE_ADRS + M3_SRAM_DEBUG_OFFSET)
#define  DMA_TX_CH_PHY_BASE_ADDRESS (NTN_REG_PHY_BASE_ADRS + WRAPPER_DMA_TX_CH_OFFSET)
#define  DMA_RX_CH_PHY_BASE_ADDRESS (NTN_REG_PHY_BASE_ADRS + WRAPPER_DMA_RX_CH_OFFSET)

#define NTN_M3_DBG_CNT_START   0x0000C800
#define NTN_M3_DBG_DMA_TXCH2_DB_OFST  (2 * 4)
#define NTN_M3_DBG_DMA_RXCH0_DB_OFST  (6 * 4)
#define NTN_M3_DBG_RSVD_OFST   (4*14)
#define NTN_M3_DBG_IPA_CAPABLE_MASK 0x2
#define PCIE_SRAM_BAR_NUM 2
/******************************************************************************/

/* virtual_registers Low Bit Macro Name's */
#define TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_LBIT_POS  0x4
#define TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_LBIT_POS  0x3
#define TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_LBIT_POS  0x2
#define TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_LBIT_POS  0x1
#define TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_LBIT_POS  0
#define RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_LBIT_POS  0x5
#define RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_LBIT_POS  0x4
#define RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_LBIT_POS  0x3
#define RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_LBIT_POS  0x2
#define RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_LBIT_POS  0x1
#define RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_LBIT_POS  0
#define RX_PKT_FEATURES_VLAN_TAG_SVT_LBIT_POS  0x10
#define RX_PKT_FEATURES_VLAN_TAG_VT_LBIT_POS  0
#define RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_LBIT_POS  0x1
#define RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_LBIT_POS  0
#define TX_PKT_FEATURES_TCP_HDR_LEN_LEN_LBIT_POS  0
#define TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_LBIT_POS  0x1
#define TX_PKT_FEATURES_PKT_TYPE_IP_PKT_LBIT_POS  0
#define TX_PKT_FEATURES_TUCSE_TCPCSE_LBIT_POS  0
#define TX_PKT_FEATURES_TUCSO_TCPCSO_LBIT_POS  0
#define TX_PKT_FEATURES_TUCSS_TCPCSS_LBIT_POS  0
#define TX_PKT_FEATURES_IPCSE_IPCSE_LBIT_POS  0
#define TX_PKT_FEATURES_IPCSO_IPCSO_LBIT_POS  0
#define TX_PKT_FEATURES_IPCSS_IPCSS_LBIT_POS  0
#define TX_PKT_FEATURES_PAY_LEN_PAY_LEN_LBIT_POS  0
#define TX_PKT_FEATURES_HDR_LEN_HDR_LEN_LBIT_POS  0
#define TX_PKT_FEATURES_MSS_MSS_LBIT_POS  0
#define TX_PKT_FEATURES_VLAN_TAG_SVT_LBIT_POS  0x10
#define TX_PKT_FEATURES_VLAN_TAG_VT_LBIT_POS  0
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_LBIT_POS  0x3
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_LBIT_POS  0x2
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_LBIT_POS  0x1
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_LBIT_POS  0

/* virtual_registers High Bit Macro Name's */
#define TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_HBIT_POS  0x4
#define TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_HBIT_POS  0x3
#define TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_HBIT_POS  0x2
#define TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_HBIT_POS  0x1
#define TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_HBIT_POS  0
#define RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_HBIT_POS  0x5
#define RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_HBIT_POS  0x4
#define RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_HBIT_POS  0x3
#define RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_HBIT_POS  0x2
#define RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_HBIT_POS  0x1
#define RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_HBIT_POS  0
#define RX_PKT_FEATURES_VLAN_TAG_SVT_HBIT_POS  0x1f
#define RX_PKT_FEATURES_VLAN_TAG_VT_HBIT_POS  0xf
#define RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_HBIT_POS  0x1
#define RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_HBIT_POS  0
#define TX_PKT_FEATURES_TCP_HDR_LEN_LEN_HBIT_POS  0x1f
#define TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_HBIT_POS  0x1
#define TX_PKT_FEATURES_PKT_TYPE_IP_PKT_HBIT_POS  0
#define TX_PKT_FEATURES_TUCSE_TCPCSE_HBIT_POS  0xf
#define TX_PKT_FEATURES_TUCSO_TCPCSO_HBIT_POS  0x7
#define TX_PKT_FEATURES_TUCSS_TCPCSS_HBIT_POS  0x7
#define TX_PKT_FEATURES_IPCSE_IPCSE_HBIT_POS  0xf
#define TX_PKT_FEATURES_IPCSO_IPCSO_HBIT_POS  0x7
#define TX_PKT_FEATURES_IPCSS_IPCSS_HBIT_POS  0x7
#define TX_PKT_FEATURES_PAY_LEN_PAY_LEN_HBIT_POS  0x3f
#define TX_PKT_FEATURES_HDR_LEN_HDR_LEN_HBIT_POS  0x3f
#define TX_PKT_FEATURES_MSS_MSS_HBIT_POS  0x3f
#define TX_PKT_FEATURES_VLAN_TAG_SVT_HBIT_POS  0x1f
#define TX_PKT_FEATURES_VLAN_TAG_VT_HBIT_POS  0xf
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_HBIT_POS  0x3
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_HBIT_POS  0x2
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_HBIT_POS  0x1
#define TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_HBIT_POS  0

/* virtual_registers Register-Field Read-Write Macros */
#define TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_HEARTBEAT_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_WINDOW_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_FIFO_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_CARRIER_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_HBIT_POS, TX_ERROR_COUNTERS_TX_ERRORS_ABORTED_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_MISSED_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_FIFO_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_LBIT_POS, ptr, data); \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_HBIT_POS, RX_ERROR_COUNTERS_RX_ERRORS_LENGTH_ERROR_LBIT_POS, ptr, data); \
} while(0)


#define RX_PKT_FEATURES_VLAN_TAG_SVT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_PKT_FEATURES_VLAN_TAG_SVT_HBIT_POS, RX_PKT_FEATURES_VLAN_TAG_SVT_LBIT_POS, ptr, data); \
} while(0)

#define RX_PKT_FEATURES_VLAN_TAG_SVT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_PKT_FEATURES_VLAN_TAG_SVT_HBIT_POS, RX_PKT_FEATURES_VLAN_TAG_SVT_LBIT_POS, ptr, data); \
} while(0)


#define RX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_PKT_FEATURES_VLAN_TAG_VT_HBIT_POS, RX_PKT_FEATURES_VLAN_TAG_VT_LBIT_POS, ptr, data); \
} while(0)

#define RX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_PKT_FEATURES_VLAN_TAG_VT_HBIT_POS, RX_PKT_FEATURES_VLAN_TAG_VT_LBIT_POS, ptr, data); \
} while(0)


#define RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_HBIT_POS, RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_LBIT_POS, ptr, data); \
} while(0)

#define RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_HBIT_POS, RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_LBIT_POS, ptr, data); \
} while(0)


#define RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_HBIT_POS, RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_LBIT_POS, ptr, data); \
} while(0)

#define RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_HBIT_POS, RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_TCP_HDR_LEN_LEN_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_TCP_HDR_LEN_LEN_HBIT_POS, TX_PKT_FEATURES_TCP_HDR_LEN_LEN_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_TCP_HDR_LEN_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_TCP_HDR_LEN_LEN_HBIT_POS, TX_PKT_FEATURES_TCP_HDR_LEN_LEN_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_HBIT_POS, TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_HBIT_POS, TX_PKT_FEATURES_PKT_TYPE_TCP_PKT_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PKT_TYPE_IP_PKT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PKT_TYPE_IP_PKT_HBIT_POS, TX_PKT_FEATURES_PKT_TYPE_IP_PKT_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PKT_TYPE_IP_PKT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PKT_TYPE_IP_PKT_HBIT_POS, TX_PKT_FEATURES_PKT_TYPE_IP_PKT_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_TUCSE_TCPCSE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_TUCSE_TCPCSE_HBIT_POS, TX_PKT_FEATURES_TUCSE_TCPCSE_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_TUCSE_TCPCSE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_TUCSE_TCPCSE_HBIT_POS, TX_PKT_FEATURES_TUCSE_TCPCSE_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_TUCSO_TCPCSO_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_TUCSO_TCPCSO_HBIT_POS, TX_PKT_FEATURES_TUCSO_TCPCSO_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_TUCSO_TCPCSO_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_TUCSO_TCPCSO_HBIT_POS, TX_PKT_FEATURES_TUCSO_TCPCSO_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_TUCSS_TCPCSS_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_TUCSS_TCPCSS_HBIT_POS, TX_PKT_FEATURES_TUCSS_TCPCSS_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_TUCSS_TCPCSS_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_TUCSS_TCPCSS_HBIT_POS, TX_PKT_FEATURES_TUCSS_TCPCSS_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_IPCSE_IPCSE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_IPCSE_IPCSE_HBIT_POS, TX_PKT_FEATURES_IPCSE_IPCSE_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_IPCSE_IPCSE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_IPCSE_IPCSE_HBIT_POS, TX_PKT_FEATURES_IPCSE_IPCSE_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_IPCSO_IPCSO_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_IPCSO_IPCSO_HBIT_POS, TX_PKT_FEATURES_IPCSO_IPCSO_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_IPCSO_IPCSO_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_IPCSO_IPCSO_HBIT_POS, TX_PKT_FEATURES_IPCSO_IPCSO_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_IPCSS_IPCSS_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_IPCSS_IPCSS_HBIT_POS, TX_PKT_FEATURES_IPCSS_IPCSS_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_IPCSS_IPCSS_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_IPCSS_IPCSS_HBIT_POS, TX_PKT_FEATURES_IPCSS_IPCSS_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PAY_LEN_PAY_LEN_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PAY_LEN_PAY_LEN_HBIT_POS, TX_PKT_FEATURES_PAY_LEN_PAY_LEN_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PAY_LEN_PAY_LEN_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PAY_LEN_PAY_LEN_HBIT_POS, TX_PKT_FEATURES_PAY_LEN_PAY_LEN_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_HDR_LEN_HDR_LEN_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_HDR_LEN_HDR_LEN_HBIT_POS, TX_PKT_FEATURES_HDR_LEN_HDR_LEN_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_HDR_LEN_HDR_LEN_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_HDR_LEN_HDR_LEN_HBIT_POS, TX_PKT_FEATURES_HDR_LEN_HDR_LEN_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_MSS_MSS_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_MSS_MSS_HBIT_POS, TX_PKT_FEATURES_MSS_MSS_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_MSS_MSS_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_MSS_MSS_HBIT_POS, TX_PKT_FEATURES_MSS_MSS_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_VLAN_TAG_SVT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_VLAN_TAG_SVT_HBIT_POS, TX_PKT_FEATURES_VLAN_TAG_SVT_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_VLAN_TAG_SVT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_VLAN_TAG_SVT_HBIT_POS, TX_PKT_FEATURES_VLAN_TAG_SVT_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_VLAN_TAG_VT_HBIT_POS, TX_PKT_FEATURES_VLAN_TAG_VT_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_VLAN_TAG_VT_HBIT_POS, TX_PKT_FEATURES_VLAN_TAG_VT_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_LBIT_POS, ptr, data); \
} while(0)


#define TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_LBIT_POS, ptr, data); \
} while(0)

#define TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_HBIT_POS, TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_LBIT_POS, ptr, data); \
} while(0)



/* virtual_registers Register Read-Write Macros */
#define TX_ERROR_COUNTERS_TX_ERRORS_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_ERROR_COUNTERS_TX_ERRORS_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_ERROR_COUNTERS_RX_ERRORS_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_ERROR_COUNTERS_RX_ERRORS_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_PKT_FEATURES_VLAN_TAG_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_PKT_FEATURES_VLAN_TAG_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_PKT_FEATURES_PKT_ATTRIBUTES_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_PKT_FEATURES_PKT_ATTRIBUTES_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_TCP_HDR_LEN_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_TCP_HDR_LEN_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_PKT_TYPE_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_PKT_TYPE_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_TUCSE_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_TUCSE_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_TUCSO_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_TUCSO_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_TUCSS_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_TUCSS_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_IPCSE_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_IPCSE_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_IPCSO_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_IPCSO_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_IPCSS_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_IPCSS_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_PAY_LEN_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_PAY_LEN_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_HDR_LEN_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_HDR_LEN_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_MSS_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_MSS_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_VLAN_TAG_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_VLAN_TAG_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_PKT_FEATURES_PKT_ATTRIBUTES_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_PKT_FEATURES_PKT_ATTRIBUTES_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)



#define MAC_ARPA_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x0210))

#define MAC_ARPA_RgWr(data) do {\
		iowrite32(data, (void *)MAC_ARPA_RgOffAddr);\
} while(0)

#define MAC_ARPA_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ARPA_RgOffAddr);\
} while(0)

#define MAC_ARPA_ARPPA_UdfWr(data) do {\
		MAC_ARPA_RgWr(data);\
} while(0)

#define MAC_ARPA_ARPPA_UdfRd(data) do {\
		MAC_ARPA_RgRd(data);\
} while(0)


#define MAC_L3A3R7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa6c))

#define MAC_L3A3R7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R7_RgOffAddr);\
} while(0)

#define MAC_L3A3R7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R7_RgOffAddr);\
} while(0)

#define MAC_L3A3R7_L3A30_UdfWr(data) do {\
		MAC_L3A3R7_RgWr(data);\
} while(0)

#define MAC_L3A3R7_L3A30_UdfRd(data) do {\
		MAC_L3A3R7_RgRd(data);\
} while(0)


#define MAC_L3A3R6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa3c))

#define MAC_L3A3R6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R6_RgOffAddr);\
} while(0)

#define MAC_L3A3R6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R6_RgOffAddr);\
} while(0)

#define MAC_L3A3R6_L3A30_UdfWr(data) do {\
		MAC_L3A3R6_RgWr(data);\
} while(0)

#define MAC_L3A3R6_L3A30_UdfRd(data) do {\
		MAC_L3A3R6_RgRd(data);\
} while(0)


#define MAC_L3A3R5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa0c))

#define MAC_L3A3R5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R5_RgOffAddr);\
} while(0)

#define MAC_L3A3R5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R5_RgOffAddr);\
} while(0)

#define MAC_L3A3R5_L3A30_UdfWr(data) do {\
		MAC_L3A3R5_RgWr(data);\
} while(0)

#define MAC_L3A3R5_L3A30_UdfRd(data) do {\
		MAC_L3A3R5_RgRd(data);\
} while(0)


#define MAC_L3A3R4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9dc))

#define MAC_L3A3R4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R4_RgOffAddr);\
} while(0)

#define MAC_L3A3R4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R4_RgOffAddr);\
} while(0)

#define MAC_L3A3R4_L3A30_UdfWr(data) do {\
		MAC_L3A3R4_RgWr(data);\
} while(0)

#define MAC_L3A3R4_L3A30_UdfRd(data) do {\
		MAC_L3A3R4_RgRd(data);\
} while(0)


#define MAC_L3A3R3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9ac))

#define MAC_L3A3R3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R3_RgOffAddr);\
} while(0)

#define MAC_L3A3R3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R3_RgOffAddr);\
} while(0)

#define MAC_L3A3R3_L3A30_UdfWr(data) do {\
		MAC_L3A3R3_RgWr(data);\
} while(0)

#define MAC_L3A3R3_L3A30_UdfRd(data) do {\
		MAC_L3A3R3_RgRd(data);\
} while(0)


#define MAC_L3A3R2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x97c))

#define MAC_L3A3R2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R2_RgOffAddr);\
} while(0)

#define MAC_L3A3R2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R2_RgOffAddr);\
} while(0)

#define MAC_L3A3R2_L3A30_UdfWr(data) do {\
		MAC_L3A3R2_RgWr(data);\
} while(0)

#define MAC_L3A3R2_L3A30_UdfRd(data) do {\
		MAC_L3A3R2_RgRd(data);\
} while(0)


#define MAC_L3A3R1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x94c))

#define MAC_L3A3R1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R1_RgOffAddr);\
} while(0)

#define MAC_L3A3R1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R1_RgOffAddr);\
} while(0)

#define MAC_L3A3R1_L3A30_UdfWr(data) do {\
		MAC_L3A3R1_RgWr(data);\
} while(0)

#define MAC_L3A3R1_L3A30_UdfRd(data) do {\
		MAC_L3A3R1_RgRd(data);\
} while(0)


#define MAC_L3A3R0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x91c))

#define MAC_L3A3R0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A3R0_RgOffAddr);\
} while(0)

#define MAC_L3A3R0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A3R0_RgOffAddr);\
} while(0)

#define MAC_L3A3R0_L3A30_UdfWr(data) do {\
		MAC_L3A3R0_RgWr(data);\
} while(0)

#define MAC_L3A3R0_L3A30_UdfRd(data) do {\
		MAC_L3A3R0_RgRd(data);\
} while(0)


#define MAC_L3A2R7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa68))

#define MAC_L3A2R7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R7_RgOffAddr);\
} while(0)

#define MAC_L3A2R7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R7_RgOffAddr);\
} while(0)

#define MAC_L3A2R7_L3A20_UdfWr(data) do {\
		MAC_L3A2R7_RgWr(data);\
} while(0)

#define MAC_L3A2R7_L3A20_UdfRd(data) do {\
		MAC_L3A2R7_RgRd(data);\
} while(0)


#define MAC_L3A2R6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa38))

#define MAC_L3A2R6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R6_RgOffAddr);\
} while(0)

#define MAC_L3A2R6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R6_RgOffAddr);\
} while(0)

#define MAC_L3A2R6_L3A20_UdfWr(data) do {\
		MAC_L3A2R6_RgWr(data);\
} while(0)

#define MAC_L3A2R6_L3A20_UdfRd(data) do {\
		MAC_L3A2R6_RgRd(data);\
} while(0)


#define MAC_L3A2R5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa08))

#define MAC_L3A2R5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R5_RgOffAddr);\
} while(0)

#define MAC_L3A2R5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R5_RgOffAddr);\
} while(0)

#define MAC_L3A2R5_L3A20_UdfWr(data) do {\
		MAC_L3A2R5_RgWr(data);\
} while(0)

#define MAC_L3A2R5_L3A20_UdfRd(data) do {\
		MAC_L3A2R5_RgRd(data);\
} while(0)


#define MAC_L3A2R4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9d8))

#define MAC_L3A2R4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R4_RgOffAddr);\
} while(0)

#define MAC_L3A2R4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R4_RgOffAddr);\
} while(0)

#define MAC_L3A2R4_L3A20_UdfWr(data) do {\
		MAC_L3A2R4_RgWr(data);\
} while(0)

#define MAC_L3A2R4_L3A20_UdfRd(data) do {\
		MAC_L3A2R4_RgRd(data);\
} while(0)


#define MAC_L3A2R3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9a8))

#define MAC_L3A2R3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R3_RgOffAddr);\
} while(0)

#define MAC_L3A2R3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R3_RgOffAddr);\
} while(0)

#define MAC_L3A2R3_L3A20_UdfWr(data) do {\
		MAC_L3A2R3_RgWr(data);\
} while(0)

#define MAC_L3A2R3_L3A20_UdfRd(data) do {\
		MAC_L3A2R3_RgRd(data);\
} while(0)


#define MAC_L3A2R2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x978))

#define MAC_L3A2R2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R2_RgOffAddr);\
} while(0)

#define MAC_L3A2R2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R2_RgOffAddr);\
} while(0)

#define MAC_L3A2R2_L3A20_UdfWr(data) do {\
		MAC_L3A2R2_RgWr(data);\
} while(0)

#define MAC_L3A2R2_L3A20_UdfRd(data) do {\
		MAC_L3A2R2_RgRd(data);\
} while(0)


#define MAC_L3A2R1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x948))

#define MAC_L3A2R1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R1_RgOffAddr);\
} while(0)

#define MAC_L3A2R1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R1_RgOffAddr);\
} while(0)

#define MAC_L3A2R1_L3A20_UdfWr(data) do {\
		MAC_L3A2R1_RgWr(data);\
} while(0)

#define MAC_L3A2R1_L3A20_UdfRd(data) do {\
		MAC_L3A2R1_RgRd(data);\
} while(0)


#define MAC_L3A2R0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x918))

#define MAC_L3A2R0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A2R0_RgOffAddr);\
} while(0)

#define MAC_L3A2R0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A2R0_RgOffAddr);\
} while(0)

#define MAC_L3A2R0_L3A20_UdfWr(data) do {\
		MAC_L3A2R0_RgWr(data);\
} while(0)

#define MAC_L3A2R0_L3A20_UdfRd(data) do {\
		MAC_L3A2R0_RgRd(data);\
} while(0)


#define MAC_L3A1R7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa64))

#define MAC_L3A1R7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R7_RgOffAddr);\
} while(0)

#define MAC_L3A1R7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R7_RgOffAddr);\
} while(0)

#define MAC_L3A1R7_L3A10_UdfWr(data) do {\
		MAC_L3A1R7_RgWr(data);\
} while(0)

#define MAC_L3A1R7_L3A10_UdfRd(data) do {\
		MAC_L3A1R7_RgRd(data);\
} while(0)


#define MAC_L3A1R6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa34))

#define MAC_L3A1R6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R6_RgOffAddr);\
} while(0)

#define MAC_L3A1R6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R6_RgOffAddr);\
} while(0)

#define MAC_L3A1R6_L3A10_UdfWr(data) do {\
		MAC_L3A1R6_RgWr(data);\
} while(0)

#define MAC_L3A1R6_L3A10_UdfRd(data) do {\
		MAC_L3A1R6_RgRd(data);\
} while(0)


#define MAC_L3A1R5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa04))

#define MAC_L3A1R5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R5_RgOffAddr);\
} while(0)

#define MAC_L3A1R5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R5_RgOffAddr);\
} while(0)

#define MAC_L3A1R5_L3A10_UdfWr(data) do {\
		MAC_L3A1R5_RgWr(data);\
} while(0)

#define MAC_L3A1R5_L3A10_UdfRd(data) do {\
		MAC_L3A1R5_RgRd(data);\
} while(0)


#define MAC_L3A1R4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9d4))

#define MAC_L3A1R4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R4_RgOffAddr);\
} while(0)

#define MAC_L3A1R4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R4_RgOffAddr);\
} while(0)

#define MAC_L3A1R4_L3A10_UdfWr(data) do {\
		MAC_L3A1R4_RgWr(data);\
} while(0)

#define MAC_L3A1R4_L3A10_UdfRd(data) do {\
		MAC_L3A1R4_RgRd(data);\
} while(0)


#define MAC_L3A1R3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9a4))

#define MAC_L3A1R3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R3_RgOffAddr);\
} while(0)

#define MAC_L3A1R3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R3_RgOffAddr);\
} while(0)

#define MAC_L3A1R3_L3A10_UdfWr(data) do {\
		MAC_L3A1R3_RgWr(data);\
} while(0)

#define MAC_L3A1R3_L3A10_UdfRd(data) do {\
		MAC_L3A1R3_RgRd(data);\
} while(0)


#define MAC_L3A1R2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x974))

#define MAC_L3A1R2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R2_RgOffAddr);\
} while(0)

#define MAC_L3A1R2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R2_RgOffAddr);\
} while(0)

#define MAC_L3A1R2_L3A10_UdfWr(data) do {\
		MAC_L3A1R2_RgWr(data);\
} while(0)

#define MAC_L3A1R2_L3A10_UdfRd(data) do {\
		MAC_L3A1R2_RgRd(data);\
} while(0)


#define MAC_L3A1R1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x944))

#define MAC_L3A1R1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R1_RgOffAddr);\
} while(0)

#define MAC_L3A1R1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R1_RgOffAddr);\
} while(0)

#define MAC_L3A1R1_L3A10_UdfWr(data) do {\
		MAC_L3A1R1_RgWr(data);\
} while(0)

#define MAC_L3A1R1_L3A10_UdfRd(data) do {\
		MAC_L3A1R1_RgRd(data);\
} while(0)


#define MAC_L3A1R0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x914))

#define MAC_L3A1R0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A1R0_RgOffAddr);\
} while(0)

#define MAC_L3A1R0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A1R0_RgOffAddr);\
} while(0)

#define MAC_L3A1R0_L3A10_UdfWr(data) do {\
		MAC_L3A1R0_RgWr(data);\
} while(0)

#define MAC_L3A1R0_L3A10_UdfRd(data) do {\
		MAC_L3A1R0_RgRd(data);\
} while(0)


#define MAC_L3A0R7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa60))

#define MAC_L3A0R7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R7_RgOffAddr);\
} while(0)

#define MAC_L3A0R7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R7_RgOffAddr);\
} while(0)

#define MAC_L3A0R7_L3A00_UdfWr(data) do {\
		MAC_L3A0R7_RgWr(data);\
} while(0)

#define MAC_L3A0R7_L3A00_UdfRd(data) do {\
		MAC_L3A0R7_RgRd(data);\
} while(0)


#define MAC_L3A0R6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa30))

#define MAC_L3A0R6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R6_RgOffAddr);\
} while(0)

#define MAC_L3A0R6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R6_RgOffAddr);\
} while(0)

#define MAC_L3A0R6_L3A00_UdfWr(data) do {\
		MAC_L3A0R6_RgWr(data);\
} while(0)

#define MAC_L3A0R6_L3A00_UdfRd(data) do {\
		MAC_L3A0R6_RgRd(data);\
} while(0)


#define MAC_L3A0R5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa00))

#define MAC_L3A0R5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R5_RgOffAddr);\
} while(0)

#define MAC_L3A0R5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R5_RgOffAddr);\
} while(0)

#define MAC_L3A0R5_L3A00_UdfWr(data) do {\
		MAC_L3A0R5_RgWr(data);\
} while(0)

#define MAC_L3A0R5_L3A00_UdfRd(data) do {\
		MAC_L3A0R5_RgRd(data);\
} while(0)


#define MAC_L3A0R4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9d0))

#define MAC_L3A0R4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R4_RgOffAddr);\
} while(0)

#define MAC_L3A0R4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R4_RgOffAddr);\
} while(0)

#define MAC_L3A0R4_L3A00_UdfWr(data) do {\
		MAC_L3A0R4_RgWr(data);\
} while(0)

#define MAC_L3A0R4_L3A00_UdfRd(data) do {\
		MAC_L3A0R4_RgRd(data);\
} while(0)


#define MAC_L3A0R3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9a0))

#define MAC_L3A0R3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R3_RgOffAddr);\
} while(0)

#define MAC_L3A0R3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R3_RgOffAddr);\
} while(0)

#define MAC_L3A0R3_L3A00_UdfWr(data) do {\
		MAC_L3A0R3_RgWr(data);\
} while(0)

#define MAC_L3A0R3_L3A00_UdfRd(data) do {\
		MAC_L3A0R3_RgRd(data);\
} while(0)


#define MAC_L3A0R2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x970))

#define MAC_L3A0R2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R2_RgOffAddr);\
} while(0)

#define MAC_L3A0R2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R2_RgOffAddr);\
} while(0)

#define MAC_L3A0R2_L3A00_UdfWr(data) do {\
		MAC_L3A0R2_RgWr(data);\
} while(0)

#define MAC_L3A0R2_L3A00_UdfRd(data) do {\
		MAC_L3A0R2_RgRd(data);\
} while(0)


#define MAC_L3A0R1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x940))

#define MAC_L3A0R1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R1_RgOffAddr);\
} while(0)

#define MAC_L3A0R1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R1_RgOffAddr);\
} while(0)

#define MAC_L3A0R1_L3A00_UdfWr(data) do {\
		MAC_L3A0R1_RgWr(data);\
} while(0)

#define MAC_L3A0R1_L3A00_UdfRd(data) do {\
		MAC_L3A0R1_RgRd(data);\
} while(0)


#define MAC_L3A0R0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x910))

#define MAC_L3A0R0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3A0R0_RgOffAddr);\
} while(0)

#define MAC_L3A0R0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3A0R0_RgOffAddr);\
} while(0)

#define MAC_L3A0R0_L3A00_UdfWr(data) do {\
		MAC_L3A0R0_RgWr(data);\
} while(0)

#define MAC_L3A0R0_L3A00_UdfRd(data) do {\
		MAC_L3A0R0_RgRd(data);\
} while(0)


#define MAC_L4AR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa54))

#define MAC_L4AR7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR7_RgOffAddr);\
} while(0)

#define MAC_L4AR7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR7_RgOffAddr);\
} while(0)

/*#define MAC_L4AR7_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR7_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR7_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR7_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR7_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR7_RgRd(v);\
		v = ((v & MAC_L4AR7_L4SP0_Wr_Mask) | ((data & MAC_L4AR7_L4SP0_Mask)<<0));\
		MAC_L4AR7_RgWr(v);\
} while(0)

#define MAC_L4AR7_L4SP0_UdfRd(data) do {\
		MAC_L4AR7_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR7_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR7_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR7_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR7_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR7_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR7_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR7_RgRd(v);\
		v = ((v & MAC_L4AR7_L4DP0_Wr_Mask) | ((data & MAC_L4AR7_L4DP0_Mask)<<16));\
		MAC_L4AR7_RgWr(v);\
} while(0)

#define MAC_L4AR7_L4DP0_UdfRd(data) do {\
		MAC_L4AR7_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR7_L4DP0_Mask);\
} while(0)


#define MAC_L4AR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa24))

#define MAC_L4AR6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR6_RgOffAddr);\
} while(0)

#define MAC_L4AR6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR6_RgOffAddr);\
} while(0)

/*#define MAC_L4AR6_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR6_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR6_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR6_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR6_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR6_RgRd(v);\
		v = ((v & MAC_L4AR6_L4SP0_Wr_Mask) | ((data & MAC_L4AR6_L4SP0_Mask)<<0));\
		MAC_L4AR6_RgWr(v);\
} while(0)

#define MAC_L4AR6_L4SP0_UdfRd(data) do {\
		MAC_L4AR6_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR6_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR6_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR6_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR6_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR6_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR6_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR6_RgRd(v);\
		v = ((v & MAC_L4AR6_L4DP0_Wr_Mask) | ((data & MAC_L4AR6_L4DP0_Mask)<<16));\
		MAC_L4AR6_RgWr(v);\
} while(0)

#define MAC_L4AR6_L4DP0_UdfRd(data) do {\
		MAC_L4AR6_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR6_L4DP0_Mask);\
} while(0)


#define MAC_L4AR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9f4))

#define MAC_L4AR5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR5_RgOffAddr);\
} while(0)

#define MAC_L4AR5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR5_RgOffAddr);\
} while(0)

/*#define MAC_L4AR5_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR5_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR5_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR5_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR5_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR5_RgRd(v);\
		v = ((v & MAC_L4AR5_L4SP0_Wr_Mask) | ((data & MAC_L4AR5_L4SP0_Mask)<<0));\
		MAC_L4AR5_RgWr(v);\
} while(0)

#define MAC_L4AR5_L4SP0_UdfRd(data) do {\
		MAC_L4AR5_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR5_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR5_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR5_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR5_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR5_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR5_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR5_RgRd(v);\
		v = ((v & MAC_L4AR5_L4DP0_Wr_Mask) | ((data & MAC_L4AR5_L4DP0_Mask)<<16));\
		MAC_L4AR5_RgWr(v);\
} while(0)

#define MAC_L4AR5_L4DP0_UdfRd(data) do {\
		MAC_L4AR5_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR5_L4DP0_Mask);\
} while(0)


#define MAC_L4AR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9c4))

#define MAC_L4AR4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR4_RgOffAddr);\
} while(0)

#define MAC_L4AR4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR4_RgOffAddr);\
} while(0)

/*#define MAC_L4AR4_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR4_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR4_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR4_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR4_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR4_RgRd(v);\
		v = ((v & MAC_L4AR4_L4SP0_Wr_Mask) | ((data & MAC_L4AR4_L4SP0_Mask)<<0));\
		MAC_L4AR4_RgWr(v);\
} while(0)

#define MAC_L4AR4_L4SP0_UdfRd(data) do {\
		MAC_L4AR4_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR4_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR4_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR4_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR4_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR4_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR4_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR4_RgRd(v);\
		v = ((v & MAC_L4AR4_L4DP0_Wr_Mask) | ((data & MAC_L4AR4_L4DP0_Mask)<<16));\
		MAC_L4AR4_RgWr(v);\
} while(0)

#define MAC_L4AR4_L4DP0_UdfRd(data) do {\
		MAC_L4AR4_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR4_L4DP0_Mask);\
} while(0)


#define MAC_L4AR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x994))

#define MAC_L4AR3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR3_RgOffAddr);\
} while(0)

#define MAC_L4AR3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR3_RgOffAddr);\
} while(0)

/*#define MAC_L4AR3_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR3_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR3_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR3_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR3_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR3_RgRd(v);\
		v = ((v & MAC_L4AR3_L4SP0_Wr_Mask) | ((data & MAC_L4AR3_L4SP0_Mask)<<0));\
		MAC_L4AR3_RgWr(v);\
} while(0)

#define MAC_L4AR3_L4SP0_UdfRd(data) do {\
		MAC_L4AR3_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR3_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR3_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR3_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR3_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR3_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR3_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR3_RgRd(v);\
		v = ((v & MAC_L4AR3_L4DP0_Wr_Mask) | ((data & MAC_L4AR3_L4DP0_Mask)<<16));\
		MAC_L4AR3_RgWr(v);\
} while(0)

#define MAC_L4AR3_L4DP0_UdfRd(data) do {\
		MAC_L4AR3_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR3_L4DP0_Mask);\
} while(0)


#define MAC_L4AR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x964))

#define MAC_L4AR2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR2_RgOffAddr);\
} while(0)

#define MAC_L4AR2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR2_RgOffAddr);\
} while(0)

/*#define MAC_L4AR2_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR2_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR2_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR2_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR2_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR2_RgRd(v);\
		v = ((v & MAC_L4AR2_L4SP0_Wr_Mask) | ((data & MAC_L4AR2_L4SP0_Mask)<<0));\
		MAC_L4AR2_RgWr(v);\
} while(0)

#define MAC_L4AR2_L4SP0_UdfRd(data) do {\
		MAC_L4AR2_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR2_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR2_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR2_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR2_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR2_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR2_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR2_RgRd(v);\
		v = ((v & MAC_L4AR2_L4DP0_Wr_Mask) | ((data & MAC_L4AR2_L4DP0_Mask)<<16));\
		MAC_L4AR2_RgWr(v);\
} while(0)

#define MAC_L4AR2_L4DP0_UdfRd(data) do {\
		MAC_L4AR2_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR2_L4DP0_Mask);\
} while(0)


#define MAC_L4AR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x934))

#define MAC_L4AR1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR1_RgOffAddr);\
} while(0)

#define MAC_L4AR1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR1_RgOffAddr);\
} while(0)

/*#define MAC_L4AR1_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR1_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR1_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR1_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR1_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR1_RgRd(v);\
		v = ((v & MAC_L4AR1_L4SP0_Wr_Mask) | ((data & MAC_L4AR1_L4SP0_Mask)<<0));\
		MAC_L4AR1_RgWr(v);\
} while(0)

#define MAC_L4AR1_L4SP0_UdfRd(data) do {\
		MAC_L4AR1_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR1_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR1_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR1_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR1_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR1_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR1_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR1_RgRd(v);\
		v = ((v & MAC_L4AR1_L4DP0_Wr_Mask) | ((data & MAC_L4AR1_L4DP0_Mask)<<16));\
		MAC_L4AR1_RgWr(v);\
} while(0)

#define MAC_L4AR1_L4DP0_UdfRd(data) do {\
		MAC_L4AR1_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR1_L4DP0_Mask);\
} while(0)


#define MAC_L4AR0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x904))

#define MAC_L4AR0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L4AR0_RgOffAddr);\
} while(0)

#define MAC_L4AR0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L4AR0_RgOffAddr);\
} while(0)

/*#define MAC_L4AR0_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR0_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR0_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR0_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR0_L4SP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR0_RgRd(v);\
		v = ((v & MAC_L4AR0_L4SP0_Wr_Mask) | ((data & MAC_L4AR0_L4SP0_Mask)<<0));\
		MAC_L4AR0_RgWr(v);\
} while(0)

#define MAC_L4AR0_L4SP0_UdfRd(data) do {\
		MAC_L4AR0_RgRd(data);\
		data = ((data >> 0) & MAC_L4AR0_L4SP0_Mask);\
} while(0)

/*#define MAC_L4AR0_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR0_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR0_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR0_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR0_L4DP0_UdfWr(data) do{\
		ULONG v;\
		MAC_L4AR0_RgRd(v);\
		v = ((v & MAC_L4AR0_L4DP0_Wr_Mask) | ((data & MAC_L4AR0_L4DP0_Mask)<<16));\
		MAC_L4AR0_RgWr(v);\
} while(0)

#define MAC_L4AR0_L4DP0_UdfRd(data) do {\
		MAC_L4AR0_RgRd(data);\
		data = ((data >> 16) & MAC_L4AR0_L4DP0_Mask);\
} while(0)


#define MAC_L3L4CR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa50))

#define MAC_L3L4CR7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR7_RgOffAddr);\
} while(0)

#define MAC_L3L4CR7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR7_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR7_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR7_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR7_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR7_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR7_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR7_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR7_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR7_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR7_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR7_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR7_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR7_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR7_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR7_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR7_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR7_L3PEN0_Mask)<<0));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR7_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR7_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR7_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR7_L3SAM0_Mask)<<2));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR7_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR7_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR7_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR7_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR7_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR7_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR7_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR7_L3DAM0_Mask)<<4));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR7_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR7_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR7_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR7_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR7_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR7_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR7_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR7_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR7_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR7_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR7_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR7_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR7_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR7_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR7_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR7_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR7_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR7_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR7_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR7_L4PEN0_Mask)<<16));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR7_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR7_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR7_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR7_L4SPM0_Mask)<<18));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR7_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR7_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR7_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR7_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR7_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR7_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR7_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR7_L4DPM0_Mask)<<20));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR7_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR7_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR7_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR7_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR7_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR7_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR7_RgRd(v);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR7_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR7_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR7_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR7_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR7_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR7_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR7_RgWr(v);\
} while(0)

#define MAC_L3L4CR7_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR7_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR7_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa20))

#define MAC_L3L4CR6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR6_RgOffAddr);\
} while(0)

#define MAC_L3L4CR6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR6_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR6_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR6_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR6_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR6_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR6_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR6_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR6_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR6_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR6_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR6_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR6_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR6_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR6_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR6_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR6_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR6_L3PEN0_Mask)<<0));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR6_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR6_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR6_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR6_L3SAM0_Mask)<<2));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR6_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR6_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR6_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR6_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR6_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR6_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR6_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR6_L3DAM0_Mask)<<4));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR6_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR6_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR6_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR6_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR6_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR6_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR6_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR6_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR6_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR6_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR6_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR6_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR6_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR6_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR6_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR6_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR6_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR6_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR6_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR6_L4PEN0_Mask)<<16));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR6_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR6_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR6_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR6_L4SPM0_Mask)<<18));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR6_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR6_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR6_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR6_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR6_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR6_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR6_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR6_L4DPM0_Mask)<<20));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR6_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR6_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR6_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR6_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR6_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR6_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR6_RgRd(v);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR6_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR6_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR6_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR6_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR6_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR6_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR6_RgWr(v);\
} while(0)

#define MAC_L3L4CR6_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR6_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR6_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9f0))

#define MAC_L3L4CR5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR5_RgOffAddr);\
} while(0)

#define MAC_L3L4CR5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR5_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR5_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR5_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR5_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR5_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR5_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR5_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR5_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR5_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR5_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR5_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR5_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR5_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR5_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR5_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR5_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR5_L3PEN0_Mask)<<0));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR5_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR5_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR5_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR5_L3SAM0_Mask)<<2));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR5_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR5_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR5_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR5_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR5_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR5_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR5_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR5_L3DAM0_Mask)<<4));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR5_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR5_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR5_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR5_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR5_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR5_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR5_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR5_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR5_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR5_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR5_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR5_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR5_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR5_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR5_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR5_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR5_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR5_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR5_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR5_L4PEN0_Mask)<<16));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR5_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR5_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR5_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR5_L4SPM0_Mask)<<18));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR5_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR5_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR5_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR5_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR5_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR5_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR5_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR5_L4DPM0_Mask)<<20));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR5_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR5_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR5_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR5_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR5_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR5_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR5_RgRd(v);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR5_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR5_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR5_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR5_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR5_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR5_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR5_RgWr(v);\
} while(0)

#define MAC_L3L4CR5_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR5_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR5_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9c0))

#define MAC_L3L4CR4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR4_RgOffAddr);\
} while(0)

#define MAC_L3L4CR4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR4_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR4_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR4_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR4_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR4_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR4_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR4_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR4_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR4_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR4_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR4_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR4_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR4_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR4_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR4_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR4_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR4_L3PEN0_Mask)<<0));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR4_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR4_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR4_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR4_L3SAM0_Mask)<<2));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR4_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR4_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR4_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR4_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR4_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR4_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR4_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR4_L3DAM0_Mask)<<4));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR4_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR4_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR4_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR4_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR4_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR4_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR4_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR4_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR4_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR4_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR4_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR4_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR4_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR4_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR4_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR4_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR4_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR4_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR4_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR4_L4PEN0_Mask)<<16));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR4_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR4_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR4_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR4_L4SPM0_Mask)<<18));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR4_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR4_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR4_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR4_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR4_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR4_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR4_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR4_L4DPM0_Mask)<<20));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR4_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR4_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR4_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR4_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR4_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR4_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR4_RgRd(v);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR4_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR4_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR4_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR4_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR4_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR4_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR4_RgWr(v);\
} while(0)

#define MAC_L3L4CR4_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR4_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR4_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x990))

#define MAC_L3L4CR3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR3_RgOffAddr);\
} while(0)

#define MAC_L3L4CR3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR3_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR3_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR3_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR3_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR3_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR3_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR3_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR3_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR3_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR3_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR3_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR3_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR3_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR3_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR3_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR3_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR3_L3PEN0_Mask)<<0));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR3_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR3_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR3_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR3_L3SAM0_Mask)<<2));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR3_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR3_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR3_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR3_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR3_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR3_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR3_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR3_L3DAM0_Mask)<<4));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR3_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR3_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR3_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR3_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR3_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR3_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR3_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR3_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR3_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR3_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR3_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR3_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR3_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR3_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR3_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR3_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR3_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR3_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR3_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR3_L4PEN0_Mask)<<16));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR3_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR3_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR3_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR3_L4SPM0_Mask)<<18));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR3_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR3_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR3_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR3_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR3_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR3_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR3_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR3_L4DPM0_Mask)<<20));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR3_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR3_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR3_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR3_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR3_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR3_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR3_RgRd(v);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR3_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR3_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR3_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR3_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR3_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR3_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR3_RgWr(v);\
} while(0)

#define MAC_L3L4CR3_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR3_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR3_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x960))

#define MAC_L3L4CR2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR2_RgOffAddr);\
} while(0)

#define MAC_L3L4CR2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR2_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR2_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR2_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR2_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR2_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR2_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR2_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR2_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR2_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR2_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR2_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR2_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR2_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR2_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR2_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR2_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR2_L3PEN0_Mask)<<0));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR2_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR2_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR2_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR2_L3SAM0_Mask)<<2));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR2_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR2_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR2_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR2_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR2_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR2_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR2_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR2_L3DAM0_Mask)<<4));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR2_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR2_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR2_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR2_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR2_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR2_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR2_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR2_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR2_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR2_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR2_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR2_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR2_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR2_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR2_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR2_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR2_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR2_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR2_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR2_L4PEN0_Mask)<<16));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR2_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR2_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR2_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR2_L4SPM0_Mask)<<18));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR2_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR2_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR2_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR2_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR2_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR2_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR2_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR2_L4DPM0_Mask)<<20));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR2_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR2_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR2_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR2_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR2_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR2_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR2_RgRd(v);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR2_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR2_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR2_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR2_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR2_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR2_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR2_RgWr(v);\
} while(0)

#define MAC_L3L4CR2_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR2_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR2_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x930))

#define MAC_L3L4CR1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR1_RgOffAddr);\
} while(0)

#define MAC_L3L4CR1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR1_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR1_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR1_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR1_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR1_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR1_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR1_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR1_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR1_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR1_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR1_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR1_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR1_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR1_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR1_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR1_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR1_L3PEN0_Mask)<<0));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR1_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR1_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR1_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR1_L3SAM0_Mask)<<2));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR1_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR1_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR1_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR1_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR1_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR1_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR1_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR1_L3DAM0_Mask)<<4));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR1_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR1_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR1_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR1_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR1_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR1_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR1_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR1_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR1_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR1_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR1_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR1_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR1_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR1_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR1_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR1_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR1_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR1_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR1_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR1_L4PEN0_Mask)<<16));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR1_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR1_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR1_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR1_L4SPM0_Mask)<<18));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR1_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR1_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR1_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR1_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR1_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR1_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR1_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR1_L4DPM0_Mask)<<20));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR1_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR1_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR1_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR1_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR1_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR1_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR1_RgRd(v);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR1_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR1_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR1_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR1_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR1_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR1_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR1_RgWr(v);\
} while(0)

#define MAC_L3L4CR1_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR1_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR1_L4DPIM0_Mask);\
} while(0)


#define MAC_L3L4CR0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x900))

#define MAC_L3L4CR0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_L3L4CR0_RgOffAddr);\
} while(0)

#define MAC_L3L4CR0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_L3L4CR0_RgOffAddr);\
} while(0)

/*#define  MAC_L3L4CR0_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR0_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR0_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR0_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define  MAC_L3L4CR0_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR0_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR0_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR0_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR0_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR0_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR0_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR0_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define MAC_L3L4CR0_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR0_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR0_L3PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR0_L3PEN0_Mask)<<0));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3PEN0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 0) & MAC_L3L4CR0_L3PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR0_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR0_L3SAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR0_L3SAM0_Mask)<<2));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3SAM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 2) & MAC_L3L4CR0_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR0_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR0_L3SAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR0_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3SAIM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 3) & MAC_L3L4CR0_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR0_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR0_L3DAM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR0_L3DAM0_Mask)<<4));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3DAM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 4) & MAC_L3L4CR0_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR0_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR0_L3DAIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR0_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3DAIM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 5) & MAC_L3L4CR0_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR0_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR0_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR0_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR0_L3HSBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR0_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3HSBM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 6) & MAC_L3L4CR0_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR0_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR0_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR0_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR0_L3HDBM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR0_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L3HDBM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 11) & MAC_L3L4CR0_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR0_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR0_L4PEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR0_L4PEN0_Mask)<<16));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L4PEN0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 16) & MAC_L3L4CR0_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR0_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR0_L4SPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR0_L4SPM0_Mask)<<18));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L4SPM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 18) & MAC_L3L4CR0_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR0_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR0_L4SPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR0_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L4SPIM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 19) & MAC_L3L4CR0_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR0_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR0_L4DPM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR0_L4DPM0_Mask)<<20));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L4DPM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 20) & MAC_L3L4CR0_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR0_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR0_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR0_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR0_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR0_L4DPIM0_UdfWr(data) do {\
		ULONG v;\
		MAC_L3L4CR0_RgRd(v);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR0_Mask_1))<<1);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR0_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR0_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR0_Mask_22))<<22);\
		v = ((v & MAC_L3L4CR0_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR0_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR0_RgWr(v);\
} while(0)

#define MAC_L3L4CR0_L4DPIM0_UdfRd(data) do {\
		MAC_L3L4CR0_RgRd(data);\
		data = ((data >> 21) & MAC_L3L4CR0_L4DPIM0_Mask);\
} while(0)


#define MAC_GPIOS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x20c))

#define MAC_GPIOS_RgWr(data) do {\
		iowrite32(data, (void *)MAC_GPIOS_RgOffAddr);\
} while(0)

#define MAC_GPIOS_RgRd(data) do {\
		(data) = ioread32((void *)MAC_GPIOS_RgOffAddr);\
} while(0)

/*#define MAC_GPIOS_GPO_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_GPIOS_GPO_Mask (ULONG)(0xffff)

/*#define MAC_GPIOS_GPO_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_GPIOS_GPO_Wr_Mask (ULONG)(0xffff)

#define MAC_GPIOS_GPO_UdfWr(data) do{\
		ULONG v;\
		MAC_GPIOS_RgRd(v);\
		v = ((v & MAC_GPIOS_GPO_Wr_Mask) | ((data & MAC_GPIOS_GPO_Mask)<<16));\
		MAC_GPIOS_RgWr(v);\
} while(0)

#define MAC_GPIOS_GPO_UdfRd(data) do {\
		MAC_GPIOS_RgRd(data);\
		data = ((data >> 16) & MAC_GPIOS_GPO_Mask);\
} while(0)

/*#define MAC_GPIOS_GPIS_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_GPIOS_GPIS_Mask (ULONG)(0xffff)

#define MAC_GPIOS_GPIS_UdfRd(data) do {\
		MAC_GPIOS_RgRd(data);\
		data = ((data >> 0) & MAC_GPIOS_GPIS_Mask);\
} while(0)


#define MAC_PCS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xf8))

#define MAC_PCS_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PCS_RgOffAddr);\
} while(0)

#define MAC_PCS_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PCS_RgOffAddr);\
} while(0)

/*#define  MAC_PCS_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_PCS_Mask_22 (ULONG)(0x3ff)

/*#define MAC_PCS_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_PCS_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define  MAC_PCS_Mask_5 (ULONG)(~(~0<<(11)))*/

#define  MAC_PCS_Mask_5 (ULONG)(0x7ff)

/*#define MAC_PCS_RES_Wr_Mask_5 (ULONG)(~((~(~0<<(11)))<<(5)))*/

#define MAC_PCS_RES_Wr_Mask_5 (ULONG)(0xffff001f)

/*#define  MAC_PCS_Mask_3 (ULONG)(~(~0<<(1)))*/

#define  MAC_PCS_Mask_3 (ULONG)(0x1)

/*#define MAC_PCS_RES_Wr_Mask_3 (ULONG)(~((~(~0<<(1)))<<(3)))*/

#define MAC_PCS_RES_Wr_Mask_3 (ULONG)(0xfffffff7)

/*#define MAC_PCS_FALSCARDET_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_FALSCARDET_Mask (ULONG)(0x1)

/*#define MAC_PCS_FALSCARDET_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_PCS_FALSCARDET_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_PCS_FALSCARDET_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_FALSCARDET_Wr_Mask) | ((data & MAC_PCS_FALSCARDET_Mask)<<21));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_FALSCARDET_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 21) & MAC_PCS_FALSCARDET_Mask);\
} while(0)

/*#define MAC_PCS_JABTO_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_JABTO_Mask (ULONG)(0x1)

/*#define MAC_PCS_JABTO_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_PCS_JABTO_Wr_Mask (ULONG)(0xffefffff)

#define MAC_PCS_JABTO_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_JABTO_Wr_Mask) | ((data & MAC_PCS_JABTO_Mask)<<20));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_JABTO_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 20) & MAC_PCS_JABTO_Mask);\
} while(0)

/*#define MAC_PCS_LNKSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_LNKSTS_Mask (ULONG)(0x1)

/*#define MAC_PCS_LNKSTS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_PCS_LNKSTS_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_PCS_LNKSTS_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_LNKSTS_Wr_Mask) | ((data & MAC_PCS_LNKSTS_Mask)<<19));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_LNKSTS_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 19) & MAC_PCS_LNKSTS_Mask);\
} while(0)

/*#define MAC_PCS_LNKSPEED_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_PCS_LNKSPEED_Mask (ULONG)(0x3)

/*#define MAC_PCS_LNKSPEED_Wr_Mask (ULONG)(~((~(~0 << (2))) << (17)))*/

#define MAC_PCS_LNKSPEED_Wr_Mask (ULONG)(0xfff9ffff)

#define MAC_PCS_LNKSPEED_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_LNKSPEED_Wr_Mask) | ((data & MAC_PCS_LNKSPEED_Mask)<<17));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_LNKSPEED_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 17) & MAC_PCS_LNKSPEED_Mask);\
} while(0)

/*#define MAC_PCS_LNKMOD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_LNKMOD_Mask (ULONG)(0x1)

/*#define MAC_PCS_LNKMOD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_PCS_LNKMOD_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_PCS_LNKMOD_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_LNKMOD_Wr_Mask) | ((data & MAC_PCS_LNKMOD_Mask)<<16));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_LNKMOD_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 16) & MAC_PCS_LNKMOD_Mask);\
} while(0)

/*#define MAC_PCS_SMIDRXS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_SMIDRXS_Mask (ULONG)(0x1)

/*#define MAC_PCS_SMIDRXS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_PCS_SMIDRXS_Wr_Mask (ULONG)(0xffffffef)

#define MAC_PCS_SMIDRXS_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_SMIDRXS_Wr_Mask) | ((data & MAC_PCS_SMIDRXS_Mask)<<4));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_SMIDRXS_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 4) & MAC_PCS_SMIDRXS_Mask);\
} while(0)

/*#define MAC_PCS_SFTERR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_SFTERR_Mask (ULONG)(0x1)

/*#define MAC_PCS_SFTERR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_PCS_SFTERR_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_PCS_SFTERR_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_SFTERR_Wr_Mask) | ((data & MAC_PCS_SFTERR_Mask)<<2));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_SFTERR_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 2) & MAC_PCS_SFTERR_Mask);\
} while(0)

/*#define MAC_PCS_LUD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_LUD_Mask (ULONG)(0x1)

/*#define MAC_PCS_LUD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_PCS_LUD_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_PCS_LUD_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_LUD_Wr_Mask) | ((data & MAC_PCS_LUD_Mask)<<1));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_LUD_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 1) & MAC_PCS_LUD_Mask);\
} while(0)

/*#define MAC_PCS_TC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PCS_TC_Mask (ULONG)(0x1)

/*#define MAC_PCS_TC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_PCS_TC_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_PCS_TC_UdfWr(data) do {\
		ULONG v;\
		MAC_PCS_RgRd(v);\
		v = (v & (MAC_PCS_RES_Wr_Mask_22))|((( 0) & (MAC_PCS_Mask_22))<<22);\
		v = (v & (MAC_PCS_RES_Wr_Mask_5))|((( 0) & (MAC_PCS_Mask_5))<<5);\
		v = (v & (MAC_PCS_RES_Wr_Mask_3))|((( 0) & (MAC_PCS_Mask_3))<<3);\
		v = ((v & MAC_PCS_TC_Wr_Mask) | ((data & MAC_PCS_TC_Mask)<<0));\
		MAC_PCS_RgWr(v);\
} while(0)

#define MAC_PCS_TC_UdfRd(data) do {\
		MAC_PCS_RgRd(data);\
		data = ((data >> 0) & MAC_PCS_TC_Mask);\
} while(0)


#define MAC_TES_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xf4))

#define MAC_TES_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TES_RgOffAddr);\
} while(0)

/*#define MAC_TES_GFD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TES_GFD_Mask (ULONG)(0x1)

#define MAC_TES_GFD_UdfRd(data) do {\
		MAC_TES_RgRd(data);\
		data = ((data >> 15) & MAC_TES_GFD_Mask);\
} while(0)

/*#define MAC_TES_GHD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TES_GHD_Mask (ULONG)(0x1)

#define MAC_TES_GHD_UdfRd(data) do {\
		MAC_TES_RgRd(data);\
		data = ((data >> 14) & MAC_TES_GHD_Mask);\
} while(0)


#define MAC_AE_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xf0))

#define MAC_AE_RgRd(data) do {\
		(data) = ioread32((void *)MAC_AE_RgOffAddr);\
} while(0)

/*#define MAC_AE_NPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AE_NPA_Mask (ULONG)(0x1)

#define MAC_AE_NPA_UdfRd(data) do {\
		MAC_AE_RgRd(data);\
		data = ((data >> 2) & MAC_AE_NPA_Mask);\
} while(0)

/*#define MAC_AE_NPR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AE_NPR_Mask (ULONG)(0x1)

#define MAC_AE_NPR_UdfRd(data) do {\
		MAC_AE_RgRd(data);\
		data = ((data >> 1) & MAC_AE_NPR_Mask);\
} while(0)


#define MAC_ALPA_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xec))

#define MAC_ALPA_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ALPA_RgOffAddr);\
} while(0)

/*#define MAC_ALPA_NP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ALPA_NP_Mask (ULONG)(0x1)

#define MAC_ALPA_NP_UdfRd(data) do {\
		MAC_ALPA_RgRd(data);\
		data = ((data >> 15) & MAC_ALPA_NP_Mask);\
} while(0)

/*#define MAC_ALPA_ACK_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ALPA_ACK_Mask (ULONG)(0x1)

#define MAC_ALPA_ACK_UdfRd(data) do {\
		MAC_ALPA_RgRd(data);\
		data = ((data >> 14) & MAC_ALPA_ACK_Mask);\
} while(0)

/*#define MAC_ALPA_RFE_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_ALPA_RFE_Mask (ULONG)(0x3)

#define MAC_ALPA_RFE_UdfRd(data) do {\
		MAC_ALPA_RgRd(data);\
		data = ((data >> 12) & MAC_ALPA_RFE_Mask);\
} while(0)

/*#define MAC_ALPA_PSE_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_ALPA_PSE_Mask (ULONG)(0x3)

#define MAC_ALPA_PSE_UdfRd(data) do {\
		MAC_ALPA_RgRd(data);\
		data = ((data >> 7) & MAC_ALPA_PSE_Mask);\
} while(0)

/*#define MAC_ALPA_HD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ALPA_HD_Mask (ULONG)(0x1)

#define MAC_ALPA_HD_UdfRd(data) do {\
		MAC_ALPA_RgRd(data);\
		data = ((data >> 6) & MAC_ALPA_HD_Mask);\
} while(0)

/*#define MAC_ALPA_FD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ALPA_FD_Mask (ULONG)(0x1)

#define MAC_ALPA_FD_UdfRd(data) do {\
		MAC_ALPA_RgRd(data);\
		data = ((data >> 5) & MAC_ALPA_FD_Mask);\
} while(0)


#define MAC_AAD_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe8))

#define MAC_AAD_RgWr(data) do {\
		iowrite32(data, (void *)MAC_AAD_RgOffAddr);\
} while(0)

#define MAC_AAD_RgRd(data) do {\
		(data) = ioread32((void *)MAC_AAD_RgOffAddr);\
} while(0)

/*#define  MAC_AAD_Mask_16 (ULONG)(~(~0<<(16)))*/

#define  MAC_AAD_Mask_16 (ULONG)(0xffff)

/*#define MAC_AAD_RES_Wr_Mask_16 (ULONG)(~((~(~0<<(16)))<<(16)))*/

#define MAC_AAD_RES_Wr_Mask_16 (ULONG)(0xffff)

/*#define  MAC_AAD_Mask_14 (ULONG)(~(~0<<(1)))*/

#define  MAC_AAD_Mask_14 (ULONG)(0x1)

/*#define MAC_AAD_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(1)))<<(14)))*/

#define MAC_AAD_RES_Wr_Mask_14 (ULONG)(0xffffbfff)

/*#define  MAC_AAD_Mask_9 (ULONG)(~(~0<<(3)))*/

#define  MAC_AAD_Mask_9 (ULONG)(0x7)

/*#define MAC_AAD_RES_Wr_Mask_9 (ULONG)(~((~(~0<<(3)))<<(9)))*/

#define MAC_AAD_RES_Wr_Mask_9 (ULONG)(0xfffff1ff)

/*#define  MAC_AAD_Mask_0 (ULONG)(~(~0<<(5)))*/

#define  MAC_AAD_Mask_0 (ULONG)(0x1f)

/*#define MAC_AAD_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(5)))<<(0)))*/

#define MAC_AAD_RES_Wr_Mask_0 (ULONG)(0xffffffe0)

/*#define MAC_AAD_NPS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AAD_NPS_Mask (ULONG)(0x1)

/*#define MAC_AAD_NPS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MAC_AAD_NPS_Wr_Mask (ULONG)(0xffff7fff)

#define MAC_AAD_NPS_UdfWr(data) do {\
		ULONG v;\
		MAC_AAD_RgRd(v);\
		v = (v & (MAC_AAD_RES_Wr_Mask_16))|((( 0) & (MAC_AAD_Mask_16))<<16);\
		v = (v & (MAC_AAD_RES_Wr_Mask_14))|((( 0) & (MAC_AAD_Mask_14))<<14);\
		v = (v & (MAC_AAD_RES_Wr_Mask_9))|((( 0) & (MAC_AAD_Mask_9))<<9);\
		v = (v & (MAC_AAD_RES_Wr_Mask_0))|((( 0) & (MAC_AAD_Mask_0))<<0);\
		v = ((v & MAC_AAD_NPS_Wr_Mask) | ((data & MAC_AAD_NPS_Mask)<<15));\
		MAC_AAD_RgWr(v);\
} while(0)

#define MAC_AAD_NPS_UdfRd(data) do {\
		MAC_AAD_RgRd(data);\
		data = ((data >> 15) & MAC_AAD_NPS_Mask);\
} while(0)

/*#define MAC_AAD_RFE_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_AAD_RFE_Mask (ULONG)(0x3)

/*#define MAC_AAD_RFE_Wr_Mask (ULONG)(~((~(~0 << (2))) << (12)))*/

#define MAC_AAD_RFE_Wr_Mask (ULONG)(0xffffcfff)

#define MAC_AAD_RFE_UdfWr(data) do {\
		ULONG v;\
		MAC_AAD_RgRd(v);\
		v = (v & (MAC_AAD_RES_Wr_Mask_16))|((( 0) & (MAC_AAD_Mask_16))<<16);\
		v = (v & (MAC_AAD_RES_Wr_Mask_14))|((( 0) & (MAC_AAD_Mask_14))<<14);\
		v = (v & (MAC_AAD_RES_Wr_Mask_9))|((( 0) & (MAC_AAD_Mask_9))<<9);\
		v = (v & (MAC_AAD_RES_Wr_Mask_0))|((( 0) & (MAC_AAD_Mask_0))<<0);\
		v = ((v & MAC_AAD_RFE_Wr_Mask) | ((data & MAC_AAD_RFE_Mask)<<12));\
		MAC_AAD_RgWr(v);\
} while(0)

#define MAC_AAD_RFE_UdfRd(data) do {\
		MAC_AAD_RgRd(data);\
		data = ((data >> 12) & MAC_AAD_RFE_Mask);\
} while(0)

/*#define MAC_AAD_PSE_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_AAD_PSE_Mask (ULONG)(0x3)

/*#define MAC_AAD_PSE_Wr_Mask (ULONG)(~((~(~0 << (2))) << (7)))*/

#define MAC_AAD_PSE_Wr_Mask (ULONG)(0xfffffe7f)

#define MAC_AAD_PSE_UdfWr(data) do {\
		ULONG v;\
		MAC_AAD_RgRd(v);\
		v = (v & (MAC_AAD_RES_Wr_Mask_16))|((( 0) & (MAC_AAD_Mask_16))<<16);\
		v = (v & (MAC_AAD_RES_Wr_Mask_14))|((( 0) & (MAC_AAD_Mask_14))<<14);\
		v = (v & (MAC_AAD_RES_Wr_Mask_9))|((( 0) & (MAC_AAD_Mask_9))<<9);\
		v = (v & (MAC_AAD_RES_Wr_Mask_0))|((( 0) & (MAC_AAD_Mask_0))<<0);\
		v = ((v & MAC_AAD_PSE_Wr_Mask) | ((data & MAC_AAD_PSE_Mask)<<7));\
		MAC_AAD_RgWr(v);\
} while(0)

#define MAC_AAD_PSE_UdfRd(data) do {\
		MAC_AAD_RgRd(data);\
		data = ((data >> 7) & MAC_AAD_PSE_Mask);\
} while(0)

/*#define MAC_AAD_HD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AAD_HD_Mask (ULONG)(0x1)

/*#define MAC_AAD_HD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MAC_AAD_HD_Wr_Mask (ULONG)(0xffffffbf)

#define MAC_AAD_HD_UdfWr(data) do {\
		ULONG v;\
		MAC_AAD_RgRd(v);\
		v = (v & (MAC_AAD_RES_Wr_Mask_16))|((( 0) & (MAC_AAD_Mask_16))<<16);\
		v = (v & (MAC_AAD_RES_Wr_Mask_14))|((( 0) & (MAC_AAD_Mask_14))<<14);\
		v = (v & (MAC_AAD_RES_Wr_Mask_9))|((( 0) & (MAC_AAD_Mask_9))<<9);\
		v = (v & (MAC_AAD_RES_Wr_Mask_0))|((( 0) & (MAC_AAD_Mask_0))<<0);\
		v = ((v & MAC_AAD_HD_Wr_Mask) | ((data & MAC_AAD_HD_Mask)<<6));\
		MAC_AAD_RgWr(v);\
} while(0)

#define MAC_AAD_HD_UdfRd(data) do {\
		MAC_AAD_RgRd(data);\
		data = ((data >> 6) & MAC_AAD_HD_Mask);\
} while(0)

/*#define MAC_AAD_FD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AAD_FD_Mask (ULONG)(0x1)

/*#define MAC_AAD_FD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_AAD_FD_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_AAD_FD_UdfWr(data) do {\
		ULONG v;\
		MAC_AAD_RgRd(v);\
		v = (v & (MAC_AAD_RES_Wr_Mask_16))|((( 0) & (MAC_AAD_Mask_16))<<16);\
		v = (v & (MAC_AAD_RES_Wr_Mask_14))|((( 0) & (MAC_AAD_Mask_14))<<14);\
		v = (v & (MAC_AAD_RES_Wr_Mask_9))|((( 0) & (MAC_AAD_Mask_9))<<9);\
		v = (v & (MAC_AAD_RES_Wr_Mask_0))|((( 0) & (MAC_AAD_Mask_0))<<0);\
		v = ((v & MAC_AAD_FD_Wr_Mask) | ((data & MAC_AAD_FD_Mask)<<5));\
		MAC_AAD_RgWr(v);\
} while(0)

#define MAC_AAD_FD_UdfRd(data) do {\
		MAC_AAD_RgRd(data);\
		data = ((data >> 5) & MAC_AAD_FD_Mask);\
} while(0)


#define MAC_ANS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe4))

#define MAC_ANS_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ANS_RgOffAddr);\
} while(0)

/*#define MAC_ANS_ES_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANS_ES_Mask (ULONG)(0x1)

#define MAC_ANS_ES_UdfRd(data) do {\
		MAC_ANS_RgRd(data);\
		data = ((data >> 8) & MAC_ANS_ES_Mask);\
} while(0)

/*#define MAC_ANS_ANC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANS_ANC_Mask (ULONG)(0x1)

#define MAC_ANS_ANC_UdfRd(data) do {\
		MAC_ANS_RgRd(data);\
		data = ((data >> 5) & MAC_ANS_ANC_Mask);\
} while(0)

/*#define MAC_ANS_ANA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANS_ANA_Mask (ULONG)(0x1)

#define MAC_ANS_ANA_UdfRd(data) do {\
		MAC_ANS_RgRd(data);\
		data = ((data >> 3) & MAC_ANS_ANA_Mask);\
} while(0)

/*#define MAC_ANS_LS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANS_LS_Mask (ULONG)(0x1)

#define MAC_ANS_LS_UdfRd(data) do {\
		MAC_ANS_RgRd(data);\
		data = ((data >> 2) & MAC_ANS_LS_Mask);\
} while(0)


#define MAC_ANC_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe0))

#define MAC_ANC_RgWr(data) do {\
		iowrite32(data, (void *)MAC_ANC_RgOffAddr);\
} while(0)

#define MAC_ANC_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ANC_RgOffAddr);\
} while(0)

/*#define  MAC_ANC_Mask_19 (ULONG)(~(~0<<(13)))*/

#define  MAC_ANC_Mask_19 (ULONG)(0x1fff)

/*#define MAC_ANC_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(13)))<<(19)))*/

#define MAC_ANC_RES_Wr_Mask_19 (ULONG)(0x7ffff)

/*#define  MAC_ANC_Mask_15 (ULONG)(~(~0<<(1)))*/

#define  MAC_ANC_Mask_15 (ULONG)(0x1)

/*#define MAC_ANC_RES_Wr_Mask_15 (ULONG)(~((~(~0<<(1)))<<(15)))*/

#define MAC_ANC_RES_Wr_Mask_15 (ULONG)(0xffff7fff)

/*#define  MAC_ANC_Mask_13 (ULONG)(~(~0<<(1)))*/

#define  MAC_ANC_Mask_13 (ULONG)(0x1)

/*#define MAC_ANC_RES_Wr_Mask_13 (ULONG)(~((~(~0<<(1)))<<(13)))*/

#define MAC_ANC_RES_Wr_Mask_13 (ULONG)(0xffffdfff)

/*#define  MAC_ANC_Mask_10 (ULONG)(~(~0<<(2)))*/

#define  MAC_ANC_Mask_10 (ULONG)(0x3)

/*#define MAC_ANC_RES_Wr_Mask_10 (ULONG)(~((~(~0<<(2)))<<(10)))*/

#define MAC_ANC_RES_Wr_Mask_10 (ULONG)(0xfffff3ff)

/*#define  MAC_ANC_Mask_0 (ULONG)(~(~0<<(9)))*/

#define  MAC_ANC_Mask_0 (ULONG)(0x1ff)

/*#define MAC_ANC_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(9)))<<(0)))*/

#define MAC_ANC_RES_Wr_Mask_0 (ULONG)(0xfffffe00)

/*#define MAC_ANC_SGMRAL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANC_SGMRAL_Mask (ULONG)(0x1)

/*#define MAC_ANC_SGMRAL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_ANC_SGMRAL_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_ANC_SGMRAL_UdfWr(data) do {\
		ULONG v;\
		MAC_ANC_RgRd(v);\
		v = (v & (MAC_ANC_RES_Wr_Mask_19))|((( 0) & (MAC_ANC_Mask_19))<<19);\
		v = (v & (MAC_ANC_RES_Wr_Mask_15))|((( 0) & (MAC_ANC_Mask_15))<<15);\
		v = (v & (MAC_ANC_RES_Wr_Mask_13))|((( 0) & (MAC_ANC_Mask_13))<<13);\
		v = (v & (MAC_ANC_RES_Wr_Mask_10))|((( 0) & (MAC_ANC_Mask_10))<<10);\
		v = (v & (MAC_ANC_RES_Wr_Mask_0))|((( 0) & (MAC_ANC_Mask_0))<<0);\
		v = ((v & MAC_ANC_SGMRAL_Wr_Mask) | ((data & MAC_ANC_SGMRAL_Mask)<<18));\
		MAC_ANC_RgWr(v);\
} while(0)

#define MAC_ANC_SGMRAL_UdfRd(data) do {\
		MAC_ANC_RgRd(data);\
		data = ((data >> 18) & MAC_ANC_SGMRAL_Mask);\
} while(0)

/*#define MAC_ANC_LR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANC_LR_Mask (ULONG)(0x1)

/*#define MAC_ANC_LR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MAC_ANC_LR_Wr_Mask (ULONG)(0xfffdffff)

#define MAC_ANC_LR_UdfWr(data) do {\
		ULONG v;\
		MAC_ANC_RgRd(v);\
		v = (v & (MAC_ANC_RES_Wr_Mask_19))|((( 0) & (MAC_ANC_Mask_19))<<19);\
		v = (v & (MAC_ANC_RES_Wr_Mask_15))|((( 0) & (MAC_ANC_Mask_15))<<15);\
		v = (v & (MAC_ANC_RES_Wr_Mask_13))|((( 0) & (MAC_ANC_Mask_13))<<13);\
		v = (v & (MAC_ANC_RES_Wr_Mask_10))|((( 0) & (MAC_ANC_Mask_10))<<10);\
		v = (v & (MAC_ANC_RES_Wr_Mask_0))|((( 0) & (MAC_ANC_Mask_0))<<0);\
		v = ((v & MAC_ANC_LR_Wr_Mask) | ((data & MAC_ANC_LR_Mask)<<17));\
		MAC_ANC_RgWr(v);\
} while(0)

#define MAC_ANC_LR_UdfRd(data) do {\
		MAC_ANC_RgRd(data);\
		data = ((data >> 17) & MAC_ANC_LR_Mask);\
} while(0)

/*#define MAC_ANC_ECD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANC_ECD_Mask (ULONG)(0x1)

/*#define MAC_ANC_ECD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_ANC_ECD_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_ANC_ECD_UdfWr(data) do {\
		ULONG v;\
		MAC_ANC_RgRd(v);\
		v = (v & (MAC_ANC_RES_Wr_Mask_19))|((( 0) & (MAC_ANC_Mask_19))<<19);\
		v = (v & (MAC_ANC_RES_Wr_Mask_15))|((( 0) & (MAC_ANC_Mask_15))<<15);\
		v = (v & (MAC_ANC_RES_Wr_Mask_13))|((( 0) & (MAC_ANC_Mask_13))<<13);\
		v = (v & (MAC_ANC_RES_Wr_Mask_10))|((( 0) & (MAC_ANC_Mask_10))<<10);\
		v = (v & (MAC_ANC_RES_Wr_Mask_0))|((( 0) & (MAC_ANC_Mask_0))<<0);\
		v = ((v & MAC_ANC_ECD_Wr_Mask) | ((data & MAC_ANC_ECD_Mask)<<16));\
		MAC_ANC_RgWr(v);\
} while(0)

#define MAC_ANC_ECD_UdfRd(data) do {\
		MAC_ANC_RgRd(data);\
		data = ((data >> 16) & MAC_ANC_ECD_Mask);\
} while(0)

/*#define MAC_ANC_ELE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANC_ELE_Mask (ULONG)(0x1)

/*#define MAC_ANC_ELE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MAC_ANC_ELE_Wr_Mask (ULONG)(0xffffbfff)

#define MAC_ANC_ELE_UdfWr(data) do {\
		ULONG v;\
		MAC_ANC_RgRd(v);\
		v = (v & (MAC_ANC_RES_Wr_Mask_19))|((( 0) & (MAC_ANC_Mask_19))<<19);\
		v = (v & (MAC_ANC_RES_Wr_Mask_15))|((( 0) & (MAC_ANC_Mask_15))<<15);\
		v = (v & (MAC_ANC_RES_Wr_Mask_13))|((( 0) & (MAC_ANC_Mask_13))<<13);\
		v = (v & (MAC_ANC_RES_Wr_Mask_10))|((( 0) & (MAC_ANC_Mask_10))<<10);\
		v = (v & (MAC_ANC_RES_Wr_Mask_0))|((( 0) & (MAC_ANC_Mask_0))<<0);\
		v = ((v & MAC_ANC_ELE_Wr_Mask) | ((data & MAC_ANC_ELE_Mask)<<14));\
		MAC_ANC_RgWr(v);\
} while(0)

#define MAC_ANC_ELE_UdfRd(data) do {\
		MAC_ANC_RgRd(data);\
		data = ((data >> 14) & MAC_ANC_ELE_Mask);\
} while(0)

/*#define MAC_ANC_ANE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANC_ANE_Mask (ULONG)(0x1)

/*#define MAC_ANC_ANE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MAC_ANC_ANE_Wr_Mask (ULONG)(0xffffefff)

#define MAC_ANC_ANE_UdfWr(data) do {\
		ULONG v;\
		MAC_ANC_RgRd(v);\
		v = (v & (MAC_ANC_RES_Wr_Mask_19))|((( 0) & (MAC_ANC_Mask_19))<<19);\
		v = (v & (MAC_ANC_RES_Wr_Mask_15))|((( 0) & (MAC_ANC_Mask_15))<<15);\
		v = (v & (MAC_ANC_RES_Wr_Mask_13))|((( 0) & (MAC_ANC_Mask_13))<<13);\
		v = (v & (MAC_ANC_RES_Wr_Mask_10))|((( 0) & (MAC_ANC_Mask_10))<<10);\
		v = (v & (MAC_ANC_RES_Wr_Mask_0))|((( 0) & (MAC_ANC_Mask_0))<<0);\
		v = ((v & MAC_ANC_ANE_Wr_Mask) | ((data & MAC_ANC_ANE_Mask)<<12));\
		MAC_ANC_RgWr(v);\
} while(0)

#define MAC_ANC_ANE_UdfRd(data) do {\
		MAC_ANC_RgRd(data);\
		data = ((data >> 12) & MAC_ANC_ANE_Mask);\
} while(0)

/*#define MAC_ANC_RAN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ANC_RAN_Mask (ULONG)(0x1)

/*#define MAC_ANC_RAN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MAC_ANC_RAN_Wr_Mask (ULONG)(0xfffffdff)

#define MAC_ANC_RAN_UdfWr(data) do {\
		ULONG v;\
		MAC_ANC_RgRd(v);\
		v = (v & (MAC_ANC_RES_Wr_Mask_19))|((( 0) & (MAC_ANC_Mask_19))<<19);\
		v = (v & (MAC_ANC_RES_Wr_Mask_15))|((( 0) & (MAC_ANC_Mask_15))<<15);\
		v = (v & (MAC_ANC_RES_Wr_Mask_13))|((( 0) & (MAC_ANC_Mask_13))<<13);\
		v = (v & (MAC_ANC_RES_Wr_Mask_10))|((( 0) & (MAC_ANC_Mask_10))<<10);\
		v = (v & (MAC_ANC_RES_Wr_Mask_0))|((( 0) & (MAC_ANC_Mask_0))<<0);\
		v = ((v & MAC_ANC_RAN_Wr_Mask) | ((data & MAC_ANC_RAN_Mask)<<9));\
		MAC_ANC_RgWr(v);\
} while(0)

#define MAC_ANC_RAN_UdfRd(data) do {\
		MAC_ANC_RgRd(data);\
		data = ((data >> 9) & MAC_ANC_RAN_Mask);\
} while(0)


#define MAC_LPC_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd4))

#define MAC_LPC_RgWr(data) do {\
		iowrite32(data, (void *)MAC_LPC_RgOffAddr);\
} while(0)

#define MAC_LPC_RgRd(data) do {\
		(data) = ioread32((void *)MAC_LPC_RgOffAddr);\
} while(0)

/*#define  MAC_LPC_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MAC_LPC_Mask_26 (ULONG)(0x3f)

/*#define MAC_LPC_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MAC_LPC_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MAC_LPC_TLPIEX_Mask (ULONG)(~(~0<<(10)))*/

#define MAC_LPC_TLPIEX_Mask (ULONG)(0x3ff)

/*#define MAC_LPC_TLPIEX_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MAC_LPC_TLPIEX_Wr_Mask (ULONG)(0xfc00ffff)

#define MAC_LPC_TLPIEX_UdfWr(data) do {\
		ULONG v;\
		MAC_LPC_RgRd(v);\
		v = (v & (MAC_LPC_RES_Wr_Mask_26))|((( 0) & (MAC_LPC_Mask_26))<<26);\
		v = ((v & MAC_LPC_TLPIEX_Wr_Mask) | ((data & MAC_LPC_TLPIEX_Mask)<<16));\
		MAC_LPC_RgWr(v);\
} while(0)

#define MAC_LPC_TLPIEX_UdfRd(data) do {\
		MAC_LPC_RgRd(data);\
		data = ((data >> 16) & MAC_LPC_TLPIEX_Mask);\
} while(0)

/*#define MAC_LPC_TWT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_LPC_TWT_Mask (ULONG)(0xffff)

/*#define MAC_LPC_TWT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_LPC_TWT_Wr_Mask (ULONG)(0xffff0000)

#define MAC_LPC_TWT_UdfWr(data) do {\
		ULONG v;\
		MAC_LPC_RgRd(v);\
		v = (v & (MAC_LPC_RES_Wr_Mask_26))|((( 0) & (MAC_LPC_Mask_26))<<26);\
		v = ((v & MAC_LPC_TWT_Wr_Mask) | ((data & MAC_LPC_TWT_Mask)<<0));\
		MAC_LPC_RgWr(v);\
} while(0)

#define MAC_LPC_TWT_UdfRd(data) do {\
		MAC_LPC_RgRd(data);\
		data = ((data >> 0) & MAC_LPC_TWT_Mask);\
} while(0)


#define MAC_LPS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd0))

#define MAC_LPS_RgWr(data) do {\
		iowrite32(data, (void *)MAC_LPS_RgOffAddr);\
} while(0)

#define MAC_LPS_RgRd(data) do {\
		(data) = ioread32((void *)MAC_LPS_RgOffAddr);\
} while(0)

/*#define  MAC_LPS_Mask_20 (ULONG)(~(~0<<(12)))*/

#define  MAC_LPS_Mask_20 (ULONG)(0xfff)

/*#define MAC_LPS_RES_Wr_Mask_20 (ULONG)(~((~(~0<<(12)))<<(20)))*/

#define MAC_LPS_RES_Wr_Mask_20 (ULONG)(0xfffff)

/*#define  MAC_LPS_Mask_10 (ULONG)(~(~0<<(6)))*/

#define  MAC_LPS_Mask_10 (ULONG)(0x3f)

/*#define MAC_LPS_RES_Wr_Mask_10 (ULONG)(~((~(~0<<(6)))<<(10)))*/

#define MAC_LPS_RES_Wr_Mask_10 (ULONG)(0xffff03ff)

/*#define  MAC_LPS_Mask_4 (ULONG)(~(~0<<(4)))*/

#define  MAC_LPS_Mask_4 (ULONG)(0xf)

/*#define MAC_LPS_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(4)))<<(4)))*/

#define MAC_LPS_RES_Wr_Mask_4 (ULONG)(0xffffff0f)

/*#define MAC_LPS_LPITXA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_LPITXA_Mask (ULONG)(0x1)

/*#define MAC_LPS_LPITXA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_LPS_LPITXA_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_LPS_LPITXA_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_LPITXA_Wr_Mask) | ((data & MAC_LPS_LPITXA_Mask)<<19));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_LPITXA_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 19) & MAC_LPS_LPITXA_Mask);\
} while(0)

/*#define MAC_LPS_PLSEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_PLSEN_Mask (ULONG)(0x1)

/*#define MAC_LPS_PLSEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_LPS_PLSEN_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_LPS_PLSEN_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_PLSEN_Wr_Mask) | ((data & MAC_LPS_PLSEN_Mask)<<18));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_PLSEN_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 18) & MAC_LPS_PLSEN_Mask);\
} while(0)

/*#define MAC_LPS_PLS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_PLS_Mask (ULONG)(0x1)

/*#define MAC_LPS_PLS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MAC_LPS_PLS_Wr_Mask (ULONG)(0xfffdffff)

#define MAC_LPS_PLS_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_PLS_Wr_Mask) | ((data & MAC_LPS_PLS_Mask)<<17));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_PLS_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 17) & MAC_LPS_PLS_Mask);\
} while(0)

/*#define MAC_LPS_LPIEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_LPIEN_Mask (ULONG)(0x1)

/*#define MAC_LPS_LPIEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_LPS_LPIEN_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_LPS_LPIEN_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_LPIEN_Wr_Mask) | ((data & MAC_LPS_LPIEN_Mask)<<16));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_LPIEN_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 16) & MAC_LPS_LPIEN_Mask);\
} while(0)

/*#define MAC_LPS_RLPIST_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_RLPIST_Mask (ULONG)(0x1)

#define MAC_LPS_RLPIST_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 9) & MAC_LPS_RLPIST_Mask);\
} while(0)

/*#define MAC_LPS_TLPIST_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_TLPIST_Mask (ULONG)(0x1)

#define MAC_LPS_TLPIST_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 8) & MAC_LPS_TLPIST_Mask);\
} while(0)

/*#define MAC_LPS_RLPIEX_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_RLPIEX_Mask (ULONG)(0x1)

/*#define MAC_LPS_RLPIEX_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_LPS_RLPIEX_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_LPS_RLPIEX_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_RLPIEX_Wr_Mask) | ((data & MAC_LPS_RLPIEX_Mask)<<3));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_RLPIEX_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 3) & MAC_LPS_RLPIEX_Mask);\
} while(0)

/*#define MAC_LPS_RLPIEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_RLPIEN_Mask (ULONG)(0x1)

/*#define MAC_LPS_RLPIEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_LPS_RLPIEN_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_LPS_RLPIEN_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_RLPIEN_Wr_Mask) | ((data & MAC_LPS_RLPIEN_Mask)<<2));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_RLPIEN_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 2) & MAC_LPS_RLPIEN_Mask);\
} while(0)

/*#define MAC_LPS_TLPIEX_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_TLPIEX_Mask (ULONG)(0x1)

/*#define MAC_LPS_TLPIEX_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_LPS_TLPIEX_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_LPS_TLPIEX_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_TLPIEX_Wr_Mask) | ((data & MAC_LPS_TLPIEX_Mask)<<1));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_TLPIEX_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 1) & MAC_LPS_TLPIEX_Mask);\
} while(0)

/*#define MAC_LPS_TLPIEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_LPS_TLPIEN_Mask (ULONG)(0x1)

/*#define MAC_LPS_TLPIEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_LPS_TLPIEN_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_LPS_TLPIEN_UdfWr(data) do {\
		ULONG v;\
		MAC_LPS_RgRd(v);\
		v = (v & (MAC_LPS_RES_Wr_Mask_20))|((( 0) & (MAC_LPS_Mask_20))<<20);\
		v = (v & (MAC_LPS_RES_Wr_Mask_10))|((( 0) & (MAC_LPS_Mask_10))<<10);\
		v = (v & (MAC_LPS_RES_Wr_Mask_4))|((( 0) & (MAC_LPS_Mask_4))<<4);\
		v = ((v & MAC_LPS_TLPIEN_Wr_Mask) | ((data & MAC_LPS_TLPIEN_Mask)<<0));\
		MAC_LPS_RgWr(v);\
} while(0)

#define MAC_LPS_TLPIEN_UdfRd(data) do {\
		MAC_LPS_RgRd(data);\
		data = ((data >> 0) & MAC_LPS_TLPIEN_Mask);\
} while(0)


#define MAC_PPS_WIDTH3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xbbc))

#define MAC_PPS_WIDTH3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_WIDTH3_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_WIDTH3_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH3_PPSWIDTH0_UdfWr(data) do {\
		MAC_PPS_WIDTH3_RgWr(data);\
} while(0)

#define MAC_PPS_WIDTH3_PPSWIDTH0_UdfRd(data) do {\
		MAC_PPS_WIDTH3_RgRd(data);\
} while(0)


#define MAC_PPS_WIDTH2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xbac))

#define MAC_PPS_WIDTH2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_WIDTH2_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_WIDTH2_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH2_PPSWIDTH0_UdfWr(data) do {\
		MAC_PPS_WIDTH2_RgWr(data);\
} while(0)

#define MAC_PPS_WIDTH2_PPSWIDTH0_UdfRd(data) do {\
		MAC_PPS_WIDTH2_RgRd(data);\
} while(0)


#define MAC_PPS_WIDTH1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb9c))

#define MAC_PPS_WIDTH1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_WIDTH1_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_WIDTH1_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH1_PPSWIDTH0_UdfWr(data) do {\
		MAC_PPS_WIDTH1_RgWr(data);\
} while(0)

#define MAC_PPS_WIDTH1_PPSWIDTH0_UdfRd(data) do {\
		MAC_PPS_WIDTH1_RgRd(data);\
} while(0)


#define MAC_PPS_WIDTH0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb8c))

#define MAC_PPS_WIDTH0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_WIDTH0_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_WIDTH0_RgOffAddr);\
} while(0)

#define MAC_PPS_WIDTH0_PPSWIDTH0_UdfWr(data) do {\
		MAC_PPS_WIDTH0_RgWr(data);\
} while(0)

#define MAC_PPS_WIDTH0_PPSWIDTH0_UdfRd(data) do {\
		MAC_PPS_WIDTH0_RgRd(data);\
} while(0)


#define MAC_PPS_INTVAL3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xbb8))

#define MAC_PPS_INTVAL3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_INTVAL3_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_INTVAL3_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL3_PPSINT0_UdfWr(data) do {\
		MAC_PPS_INTVAL3_RgWr(data);\
} while(0)

#define MAC_PPS_INTVAL3_PPSINT0_UdfRd(data) do {\
		MAC_PPS_INTVAL3_RgRd(data);\
} while(0)


#define MAC_PPS_INTVAL2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xba8))

#define MAC_PPS_INTVAL2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_INTVAL2_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_INTVAL2_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL2_PPSINT0_UdfWr(data) do {\
		MAC_PPS_INTVAL2_RgWr(data);\
} while(0)

#define MAC_PPS_INTVAL2_PPSINT0_UdfRd(data) do {\
		MAC_PPS_INTVAL2_RgRd(data);\
} while(0)


#define MAC_PPS_INTVAL1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb98))

#define MAC_PPS_INTVAL1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_INTVAL1_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_INTVAL1_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL1_PPSINT0_UdfWr(data) do {\
		MAC_PPS_INTVAL1_RgWr(data);\
} while(0)

#define MAC_PPS_INTVAL1_PPSINT0_UdfRd(data) do {\
		MAC_PPS_INTVAL1_RgRd(data);\
} while(0)


#define MAC_PPS_INTVAL0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb88))

#define MAC_PPS_INTVAL0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_INTVAL0_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_INTVAL0_RgOffAddr);\
} while(0)

#define MAC_PPS_INTVAL0_PPSINT0_UdfWr(data) do {\
		MAC_PPS_INTVAL0_RgWr(data);\
} while(0)

#define MAC_PPS_INTVAL0_PPSINT0_UdfRd(data) do {\
		MAC_PPS_INTVAL0_RgRd(data);\
} while(0)


#define MAC_PPS_TTNS3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xbb4))

#define MAC_PPS_TTNS3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTNS3_RgOffAddr);\
} while(0)

#define MAC_PPS_TTNS3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTNS3_RgOffAddr);\
} while(0)

/*#define MAC_PPS_TTNS3_TTSL0_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_PPS_TTNS3_TTSL0_Mask (ULONG)(0x7fffffff)

/*#define MAC_PPS_TTNS3_TTSL0_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_PPS_TTNS3_TTSL0_Wr_Mask (ULONG)(0x80000000)

#define MAC_PPS_TTNS3_TTSL0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS3_RgRd(v);\
		v = ((v & MAC_PPS_TTNS3_TTSL0_Wr_Mask) | ((data & MAC_PPS_TTNS3_TTSL0_Mask)<<0));\
		MAC_PPS_TTNS3_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS3_TTSL0_UdfRd(data) do {\
		MAC_PPS_TTNS3_RgRd(data);\
		data = ((data >> 0) & MAC_PPS_TTNS3_TTSL0_Mask);\
} while(0)

/*#define MAC_PPS_TTNS3_TRGTBUSY0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PPS_TTNS3_TRGTBUSY0_Mask (ULONG)(0x1)

/*#define MAC_PPS_TTNS3_TRGTBUSY0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_PPS_TTNS3_TRGTBUSY0_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_PPS_TTNS3_TRGTBUSY0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS3_RgRd(v);\
		v = ((v & MAC_PPS_TTNS3_TRGTBUSY0_Wr_Mask) | ((data & MAC_PPS_TTNS3_TRGTBUSY0_Mask)<<31));\
		MAC_PPS_TTNS3_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS3_TRGTBUSY0_UdfRd(data) do {\
		MAC_PPS_TTNS3_RgRd(data);\
		data = ((data >> 31) & MAC_PPS_TTNS3_TRGTBUSY0_Mask);\
} while(0)


#define MAC_PPS_TTNS2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xba4))

#define MAC_PPS_TTNS2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTNS2_RgOffAddr);\
} while(0)

#define MAC_PPS_TTNS2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTNS2_RgOffAddr);\
} while(0)

/*#define MAC_PPS_TTNS2_TTSL0_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_PPS_TTNS2_TTSL0_Mask (ULONG)(0x7fffffff)

/*#define MAC_PPS_TTNS2_TTSL0_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_PPS_TTNS2_TTSL0_Wr_Mask (ULONG)(0x80000000)

#define MAC_PPS_TTNS2_TTSL0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS2_RgRd(v);\
		v = ((v & MAC_PPS_TTNS2_TTSL0_Wr_Mask) | ((data & MAC_PPS_TTNS2_TTSL0_Mask)<<0));\
		MAC_PPS_TTNS2_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS2_TTSL0_UdfRd(data) do {\
		MAC_PPS_TTNS2_RgRd(data);\
		data = ((data >> 0) & MAC_PPS_TTNS2_TTSL0_Mask);\
} while(0)

/*#define MAC_PPS_TTNS2_TRGTBUSY0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PPS_TTNS2_TRGTBUSY0_Mask (ULONG)(0x1)

/*#define MAC_PPS_TTNS2_TRGTBUSY0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_PPS_TTNS2_TRGTBUSY0_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_PPS_TTNS2_TRGTBUSY0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS2_RgRd(v);\
		v = ((v & MAC_PPS_TTNS2_TRGTBUSY0_Wr_Mask) | ((data & MAC_PPS_TTNS2_TRGTBUSY0_Mask)<<31));\
		MAC_PPS_TTNS2_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS2_TRGTBUSY0_UdfRd(data) do {\
		MAC_PPS_TTNS2_RgRd(data);\
		data = ((data >> 31) & MAC_PPS_TTNS2_TRGTBUSY0_Mask);\
} while(0)








#define MAC_LMIR_RgOffAddr ((volatile unsigned int *)(BASE_ADDRESS + 0xbd0))

#define MAC_LMIR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_LMIR_RgOffAddr);\
} while(0)

#define MAC_LMIR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_LMIR_RgOffAddr);\
} while(0)

#define MAC_SPI2R_RgOffAddr ((volatile unsigned int *)(BASE_ADDRESS + 0xbcc))

#define MAC_SPI2R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_SPI2R_RgOffAddr);\
} while(0)

#define MAC_SPI2R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_SPI2R_RgOffAddr);\
} while(0)

/*#define  MAC_SPI2R_Mask_16 (unsigned int)(~(~0<<(16)))*/

#define  MAC_SPI2R_Mask_16 (unsigned int)(0xffff)

/*#define MAC_SPI2R_RES_Wr_Mask_16 (unsigned int)(~((~(~0<<(16)))<<(16)))*/

#define MAC_SPI2R_RES_Wr_Mask_16 (unsigned int)(0xffff)

/*#define MAC_SPI2R_SPIO_Mask (unsigned int)(~(~0<<(16)))*/

#define MAC_SPI2R_SPIO_Mask (unsigned int)(0xffff)

/*#define MAC_SPI2R_SPIO_Wr_Mask (unsigned int)(~((~(~0 << (16))) << (0)))*/

#define MAC_SPI2R_SPIO_Wr_Mask (unsigned int)(0xffff0000)

#define MAC_SPI2R_SPIO_UdfWr(data) do {\
		unsigned int v = 0; \
		v = (v & (MAC_SPI2R_RES_Wr_Mask_16))|((( 0) & (MAC_SPI2R_Mask_16))<<16);\
		(v) = ((v & MAC_SPI2R_SPIO_Wr_Mask) | ((data & MAC_SPI2R_SPIO_Mask)<<0));\
		MAC_SPI2R_RgWr(v);\
} while(0)

#define MAC_SPI2R_SPIO_UdfRd(data) do {\
		MAC_SPI2R_RgRd(data);\
		data = ((data >> 0) & MAC_SPI2R_SPIO_Mask);\
} while(0)


#define MAC_SPI1R_RgOffAddr ((volatile unsigned int *)(BASE_ADDRESS + 0xbc8))

#define MAC_SPI1R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_SPI1R_RgOffAddr);\
} while(0)

#define MAC_SPI1R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_SPI1R_RgOffAddr);\
} while(0)

#define MAC_SPI1R_SPIO_UdfWr(data) do {\
		MAC_SPI1R_RgWr(data);\
} while(0)

#define MAC_SPI1R_SPIO_UdfRd(data) do {\
		MAC_SPI1R_RgRd(data);\
} while(0)


#define MAC_SPI0R_RgOffAddr ((volatile unsigned int *)(BASE_ADDRESS + 0xbc4))

#define MAC_SPI0R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_SPI0R_RgOffAddr);\
} while(0)

#define MAC_SPI0R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_SPI0R_RgOffAddr);\
} while(0)

#define MAC_SPI0R_SPIO_UdfWr(data) do {\
		MAC_SPI0R_RgWr(data);\
} while(0)

#define MAC_SPI0R_SPIO_UdfRd(data) do {\
		MAC_SPI0R_RgRd(data);\
} while(0)


#define MAC_PTO_CR_RgOffAddr ((volatile unsigned int *)(BASE_ADDRESS + 0xbc0))

#define MAC_PTO_CR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PTO_CR_RgOffAddr);\
} while(0)

#define MAC_PTO_CR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PTO_CR_RgOffAddr);\
} while(0)

/*#define  MAC_PTO_CR_Mask_16 (unsigned int)(~(~0<<(16)))*/

#define  MAC_PTO_CR_Mask_16 (unsigned int)(0xffff)

/*#define MAC_PTO_CR_RES_Wr_Mask_16 (unsigned int)(~((~(~0<<(16)))<<(16)))*/

#define MAC_PTO_CR_RES_Wr_Mask_16 (unsigned int)(0xffff)

/*#define  MAC_PTO_CR_Mask_6 (unsigned int)(~(~0<<(2)))*/

#define  MAC_PTO_CR_Mask_6 (unsigned int)(0x3)

/*#define MAC_PTO_CR_RES_Wr_Mask_6 (unsigned int)(~((~(~0<<(2)))<<(6)))*/

#define MAC_PTO_CR_RES_Wr_Mask_6 (unsigned int)(0xffffff3f)

/*#define  MAC_PTO_CR_Mask_3 (unsigned int)(~(~0<<(1)))*/

#define  MAC_PTO_CR_Mask_3 (unsigned int)(0x1)

/*#define MAC_PTO_CR_RES_Wr_Mask_3 (unsigned int)(~((~(~0<<(1)))<<(3)))*/

#define MAC_PTO_CR_RES_Wr_Mask_3 (unsigned int)(0xfffffff7)

/*#define MAC_PTO_CR_DN_Mask (unsigned int)(~(~0<<(8)))*/

#define MAC_PTO_CR_DN_Mask (unsigned int)(0xff)

/*#define MAC_PTO_CR_DN_Wr_Mask (unsigned int)(~((~(~0 << (8))) << (8)))*/

#define MAC_PTO_CR_DN_Wr_Mask (unsigned int)(0xffff00ff)

#define MAC_PTO_CR_DN_UdfWr(data) do {\
		unsigned int v;\
		MAC_PTO_CR_RgRd(v);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_16))|((( 0) & (MAC_PTO_CR_Mask_16))<<16);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_6))|((( 0) & (MAC_PTO_CR_Mask_6))<<6);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_3))|((( 0) & (MAC_PTO_CR_Mask_3))<<3);\
		v = ((v & MAC_PTO_CR_DN_Wr_Mask) | ((data & MAC_PTO_CR_DN_Mask)<<8));\
		MAC_PTO_CR_RgWr(v);\
} while(0)

#define MAC_PTO_CR_DN_UdfRd(data) do {\
		MAC_PTO_CR_RgRd(data);\
		data = ((data >> 8) & MAC_PTO_CR_DN_Mask);\
} while(0)

/*#define MAC_PTO_CR_APDREQTRIG_Mask (unsigned int)(~(~0<<(1)))*/

#define MAC_PTO_CR_APDREQTRIG_Mask (unsigned int)(0x1)

/*#define MAC_PTO_CR_APDREQTRIG_Wr_Mask (unsigned int)(~((~(~0 << (1))) << (5)))*/

#define MAC_PTO_CR_APDREQTRIG_Wr_Mask (unsigned int)(0xffffffdf)

#define MAC_PTO_CR_APDREQTRIG_UdfWr(data) do {\
		unsigned int v;\
		MAC_PTO_CR_RgRd(v);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_16))|((( 0) & (MAC_PTO_CR_Mask_16))<<16);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_6))|((( 0) & (MAC_PTO_CR_Mask_6))<<6);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_3))|((( 0) & (MAC_PTO_CR_Mask_3))<<3);\
		v = ((v & MAC_PTO_CR_APDREQTRIG_Wr_Mask) | ((data & MAC_PTO_CR_APDREQTRIG_Mask)<<5));\
		MAC_PTO_CR_RgWr(v);\
} while(0)

#define MAC_PTO_CR_APDREQTRIG_UdfRd(data) do {\
		MAC_PTO_CR_RgRd(data);\
		data = ((data >> 5) & MAC_PTO_CR_APDREQTRIG_Mask);\
} while(0)

/*#define MAC_PTO_CR_ASYNCTRIG_Mask (unsigned int)(~(~0<<(1)))*/

#define MAC_PTO_CR_ASYNCTRIG_Mask (unsigned int)(0x1)

/*#define MAC_PTO_CR_ASYNCTRIG_Wr_Mask (unsigned int)(~((~(~0 << (1))) << (4)))*/

#define MAC_PTO_CR_ASYNCTRIG_Wr_Mask (unsigned int)(0xffffffef)

#define MAC_PTO_CR_ASYNCTRIG_UdfWr(data) do {\
		unsigned int v;\
		MAC_PTO_CR_RgRd(v);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_16))|((( 0) & (MAC_PTO_CR_Mask_16))<<16);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_6))|((( 0) & (MAC_PTO_CR_Mask_6))<<6);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_3))|((( 0) & (MAC_PTO_CR_Mask_3))<<3);\
		v = ((v & MAC_PTO_CR_ASYNCTRIG_Wr_Mask) | ((data & MAC_PTO_CR_ASYNCTRIG_Mask)<<4));\
		MAC_PTO_CR_RgWr(v);\
} while(0)

#define MAC_PTO_CR_ASYNCTRIG_UdfRd(data) do {\
		MAC_PTO_CR_RgRd(data);\
		data = ((data >> 4) & MAC_PTO_CR_ASYNCTRIG_Mask);\
} while(0)

/*#define MAC_PTO_CR_APDREQEN_Mask (unsigned int)(~(~0<<(1)))*/

#define MAC_PTO_CR_APDREQEN_Mask (unsigned int)(0x1)

/*#define MAC_PTO_CR_APDREQEN_Wr_Mask (unsigned int)(~((~(~0 << (1))) << (2)))*/

#define MAC_PTO_CR_APDREQEN_Wr_Mask (unsigned int)(0xfffffffb)

#define MAC_PTO_CR_APDREQEN_UdfWr(data) do {\
		unsigned int v;\
		MAC_PTO_CR_RgRd(v);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_16))|((( 0) & (MAC_PTO_CR_Mask_16))<<16);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_6))|((( 0) & (MAC_PTO_CR_Mask_6))<<6);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_3))|((( 0) & (MAC_PTO_CR_Mask_3))<<3);\
		v = ((v & MAC_PTO_CR_APDREQEN_Wr_Mask) | ((data & MAC_PTO_CR_APDREQEN_Mask)<<2));\
		MAC_PTO_CR_RgWr(v);\
} while(0)

#define MAC_PTO_CR_APDREQEN_UdfRd(data) do {\
		MAC_PTO_CR_RgRd(data);\
		data = ((data >> 2) & MAC_PTO_CR_APDREQEN_Mask);\
} while(0)

/*#define MAC_PTO_CR_ASYNCEN_Mask (unsigned int)(~(~0<<(1)))*/

#define MAC_PTO_CR_ASYNCEN_Mask (unsigned int)(0x1)

/*#define MAC_PTO_CR_ASYNCEN_Wr_Mask (unsigned int)(~((~(~0 << (1))) << (1)))*/

#define MAC_PTO_CR_ASYNCEN_Wr_Mask (unsigned int)(0xfffffffd)

#define MAC_PTO_CR_ASYNCEN_UdfWr(data) do {\
		unsigned int v;\
		MAC_PTO_CR_RgRd(v);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_16))|((( 0) & (MAC_PTO_CR_Mask_16))<<16);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_6))|((( 0) & (MAC_PTO_CR_Mask_6))<<6);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_3))|((( 0) & (MAC_PTO_CR_Mask_3))<<3);\
		v = ((v & MAC_PTO_CR_ASYNCEN_Wr_Mask) | ((data & MAC_PTO_CR_ASYNCEN_Mask)<<1));\
		MAC_PTO_CR_RgWr(v);\
} while(0)

#define MAC_PTO_CR_ASYNCEN_UdfRd(data) do {\
		MAC_PTO_CR_RgRd(data);\
		data = ((data >> 1) & MAC_PTO_CR_ASYNCEN_Mask);\
} while(0)

/*#define MAC_PTO_CR_PTOEN_Mask (unsigned int)(~(~0<<(1)))*/

#define MAC_PTO_CR_PTOEN_Mask (unsigned int)(0x1)

/*#define MAC_PTO_CR_PTOEN_Wr_Mask (unsigned int)(~((~(~0 << (1))) << (0)))*/

#define MAC_PTO_CR_PTOEN_Wr_Mask (unsigned int)(0xfffffffe)

#define MAC_PTO_CR_PTOEN_UdfWr(data) do {\
		unsigned int v;\
		MAC_PTO_CR_RgRd(v);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_16))|((( 0) & (MAC_PTO_CR_Mask_16))<<16);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_6))|((( 0) & (MAC_PTO_CR_Mask_6))<<6);\
		v = (v & (MAC_PTO_CR_RES_Wr_Mask_3))|((( 0) & (MAC_PTO_CR_Mask_3))<<3);\
		v = ((v & MAC_PTO_CR_PTOEN_Wr_Mask) | ((data & MAC_PTO_CR_PTOEN_Mask)<<0));\
		MAC_PTO_CR_RgWr(v);\
} while(0)

#define MAC_PTO_CR_PTOEN_UdfRd(data) do {\
		MAC_PTO_CR_RgRd(data);\
		data = ((data >> 0) & MAC_PTO_CR_PTOEN_Mask);\
} while(0)























#define MAC_PPS_TTNS1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb94))

#define MAC_PPS_TTNS1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTNS1_RgOffAddr);\
} while(0)

#define MAC_PPS_TTNS1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTNS1_RgOffAddr);\
} while(0)

/*#define MAC_PPS_TTNS1_TTSL0_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_PPS_TTNS1_TTSL0_Mask (ULONG)(0x7fffffff)

/*#define MAC_PPS_TTNS1_TTSL0_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_PPS_TTNS1_TTSL0_Wr_Mask (ULONG)(0x80000000)

#define MAC_PPS_TTNS1_TTSL0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS1_RgRd(v);\
		v = ((v & MAC_PPS_TTNS1_TTSL0_Wr_Mask) | ((data & MAC_PPS_TTNS1_TTSL0_Mask)<<0));\
		MAC_PPS_TTNS1_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS1_TTSL0_UdfRd(data) do {\
		MAC_PPS_TTNS1_RgRd(data);\
		data = ((data >> 0) & MAC_PPS_TTNS1_TTSL0_Mask);\
} while(0)

/*#define MAC_PPS_TTNS1_TRGTBUSY0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PPS_TTNS1_TRGTBUSY0_Mask (ULONG)(0x1)

/*#define MAC_PPS_TTNS1_TRGTBUSY0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_PPS_TTNS1_TRGTBUSY0_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_PPS_TTNS1_TRGTBUSY0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS1_RgRd(v);\
		v = ((v & MAC_PPS_TTNS1_TRGTBUSY0_Wr_Mask) | ((data & MAC_PPS_TTNS1_TRGTBUSY0_Mask)<<31));\
		MAC_PPS_TTNS1_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS1_TRGTBUSY0_UdfRd(data) do {\
		MAC_PPS_TTNS1_RgRd(data);\
		data = ((data >> 31) & MAC_PPS_TTNS1_TRGTBUSY0_Mask);\
} while(0)


#define MAC_PPS_TTNS0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb84))

#define MAC_PPS_TTNS0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTNS0_RgOffAddr);\
} while(0)

#define MAC_PPS_TTNS0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTNS0_RgOffAddr);\
} while(0)

/*#define MAC_PPS_TTNS0_TTSL0_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_PPS_TTNS0_TTSL0_Mask (ULONG)(0x7fffffff)

/*#define MAC_PPS_TTNS0_TTSL0_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_PPS_TTNS0_TTSL0_Wr_Mask (ULONG)(0x80000000)

#define MAC_PPS_TTNS0_TTSL0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS0_RgRd(v);\
		v = ((v & MAC_PPS_TTNS0_TTSL0_Wr_Mask) | ((data & MAC_PPS_TTNS0_TTSL0_Mask)<<0));\
		MAC_PPS_TTNS0_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS0_TTSL0_UdfRd(data) do {\
		MAC_PPS_TTNS0_RgRd(data);\
		data = ((data >> 0) & MAC_PPS_TTNS0_TTSL0_Mask);\
} while(0)

/*#define MAC_PPS_TTNS0_TRGTBUSY0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PPS_TTNS0_TRGTBUSY0_Mask (ULONG)(0x1)

/*#define MAC_PPS_TTNS0_TRGTBUSY0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_PPS_TTNS0_TRGTBUSY0_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_PPS_TTNS0_TRGTBUSY0_UdfWr(data) do{\
		ULONG v;\
		MAC_PPS_TTNS0_RgRd(v);\
		v = ((v & MAC_PPS_TTNS0_TRGTBUSY0_Wr_Mask) | ((data & MAC_PPS_TTNS0_TRGTBUSY0_Mask)<<31));\
		MAC_PPS_TTNS0_RgWr(v);\
} while(0)

#define MAC_PPS_TTNS0_TRGTBUSY0_UdfRd(data) do {\
		MAC_PPS_TTNS0_RgRd(data);\
		data = ((data >> 31) & MAC_PPS_TTNS0_TRGTBUSY0_Mask);\
} while(0)


#define MAC_PPS_TTS3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xbb0))

#define MAC_PPS_TTS3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTS3_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTS3_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS3_TSTRH0_UdfWr(data) do {\
		MAC_PPS_TTS3_RgWr(data);\
} while(0)

#define MAC_PPS_TTS3_TSTRH0_UdfRd(data) do {\
		MAC_PPS_TTS3_RgRd(data);\
} while(0)


#define MAC_PPS_TTS2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xba0))

#define MAC_PPS_TTS2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTS2_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTS2_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS2_TSTRH0_UdfWr(data) do {\
		MAC_PPS_TTS2_RgWr(data);\
} while(0)

#define MAC_PPS_TTS2_TSTRH0_UdfRd(data) do {\
		MAC_PPS_TTS2_RgRd(data);\
} while(0)


#define MAC_PPS_TTS1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb90))

#define MAC_PPS_TTS1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTS1_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTS1_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS1_TSTRH0_UdfWr(data) do {\
		MAC_PPS_TTS1_RgWr(data);\
} while(0)

#define MAC_PPS_TTS1_TSTRH0_UdfRd(data) do {\
		MAC_PPS_TTS1_RgRd(data);\
} while(0)


#define MAC_PPS_TTS0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb80))

#define MAC_PPS_TTS0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPS_TTS0_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPS_TTS0_RgOffAddr);\
} while(0)

#define MAC_PPS_TTS0_TSTRH0_UdfWr(data) do {\
		MAC_PPS_TTS0_RgWr(data);\
} while(0)

#define MAC_PPS_TTS0_TSTRH0_UdfRd(data) do {\
		MAC_PPS_TTS0_RgRd(data);\
} while(0)


#define MAC_PPSC_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb70))

#define MAC_PPSC_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PPSC_RgOffAddr);\
} while(0)

#define MAC_PPSC_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PPSC_RgOffAddr);\
} while(0)

/*#define  MAC_PPSC_Mask_31 (ULONG)(~(~0<<(1)))*/

#define  MAC_PPSC_Mask_31 (ULONG)(0x1)

/*#define MAC_PPSC_RES_Wr_Mask_31 (ULONG)(~((~(~0<<(1)))<<(31)))*/

#define MAC_PPSC_RES_Wr_Mask_31 (ULONG)(0x7fffffff)

/*#define  MAC_PPSC_Mask_27 (ULONG)(~(~0<<(2)))*/

#define  MAC_PPSC_Mask_27 (ULONG)(0x3)

/*#define MAC_PPSC_RES_Wr_Mask_27 (ULONG)(~((~(~0<<(2)))<<(27)))*/

#define MAC_PPSC_RES_Wr_Mask_27 (ULONG)(0xe7ffffff)

/*#define  MAC_PPSC_Mask_23 (ULONG)(~(~0<<(1)))*/

#define  MAC_PPSC_Mask_23 (ULONG)(0x1)

/*#define MAC_PPSC_RES_Wr_Mask_23 (ULONG)(~((~(~0<<(1)))<<(23)))*/

#define MAC_PPSC_RES_Wr_Mask_23 (ULONG)(0xff7fffff)

/*#define  MAC_PPSC_Mask_19 (ULONG)(~(~0<<(2)))*/

#define  MAC_PPSC_Mask_19 (ULONG)(0x3)

/*#define MAC_PPSC_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(2)))<<(19)))*/

#define MAC_PPSC_RES_Wr_Mask_19 (ULONG)(0xffe7ffff)

/*#define  MAC_PPSC_Mask_15 (ULONG)(~(~0<<(1)))*/

#define  MAC_PPSC_Mask_15 (ULONG)(0x1)

/*#define MAC_PPSC_RES_Wr_Mask_15 (ULONG)(~((~(~0<<(1)))<<(15)))*/

#define MAC_PPSC_RES_Wr_Mask_15 (ULONG)(0xffff7fff)

/*#define  MAC_PPSC_Mask_11 (ULONG)(~(~0<<(2)))*/

#define  MAC_PPSC_Mask_11 (ULONG)(0x3)

/*#define MAC_PPSC_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(2)))<<(11)))*/

#define MAC_PPSC_RES_Wr_Mask_11 (ULONG)(0xffffe7ff)

/*#define  MAC_PPSC_Mask_7 (ULONG)(~(~0<<(1)))*/

#define  MAC_PPSC_Mask_7 (ULONG)(0x1)

/*#define MAC_PPSC_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(1)))<<(7)))*/

#define MAC_PPSC_RES_Wr_Mask_7 (ULONG)(0xffffff7f)

/*#define MAC_PPSC_TRGTMODSEL3_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_PPSC_TRGTMODSEL3_Mask (ULONG)(0x3)

/*#define MAC_PPSC_TRGTMODSEL3_Wr_Mask (ULONG)(~((~(~0 << (2))) << (29)))*/

#define MAC_PPSC_TRGTMODSEL3_Wr_Mask (ULONG)(0x9fffffff)

#define MAC_PPSC_TRGTMODSEL3_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_TRGTMODSEL3_Wr_Mask) | ((data & MAC_PPSC_TRGTMODSEL3_Mask)<<29));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_TRGTMODSEL3_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 29) & MAC_PPSC_TRGTMODSEL3_Mask);\
} while(0)

/*#define MAC_PPSC_PPSCMD3_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_PPSC_PPSCMD3_Mask (ULONG)(0x7)

/*#define MAC_PPSC_PPSCMD3_Wr_Mask (ULONG)(~((~(~0 << (3))) << (24)))*/

#define MAC_PPSC_PPSCMD3_Wr_Mask (ULONG)(0xf8ffffff)

#define MAC_PPSC_PPSCMD3_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_PPSCMD3_Wr_Mask) | ((data & MAC_PPSC_PPSCMD3_Mask)<<24));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_PPSCMD3_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 24) & MAC_PPSC_PPSCMD3_Mask);\
} while(0)

/*#define MAC_PPSC_TRGTMODSEL2_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_PPSC_TRGTMODSEL2_Mask (ULONG)(0x3)

/*#define MAC_PPSC_TRGTMODSEL2_Wr_Mask (ULONG)(~((~(~0 << (2))) << (21)))*/

#define MAC_PPSC_TRGTMODSEL2_Wr_Mask (ULONG)(0xff9fffff)

#define MAC_PPSC_TRGTMODSEL2_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_TRGTMODSEL2_Wr_Mask) | ((data & MAC_PPSC_TRGTMODSEL2_Mask)<<21));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_TRGTMODSEL2_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 21) & MAC_PPSC_TRGTMODSEL2_Mask);\
} while(0)

/*#define MAC_PPSC_PPSCMD2_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_PPSC_PPSCMD2_Mask (ULONG)(0x7)

/*#define MAC_PPSC_PPSCMD2_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MAC_PPSC_PPSCMD2_Wr_Mask (ULONG)(0xfff8ffff)

#define MAC_PPSC_PPSCMD2_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_PPSCMD2_Wr_Mask) | ((data & MAC_PPSC_PPSCMD2_Mask)<<16));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_PPSCMD2_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 16) & MAC_PPSC_PPSCMD2_Mask);\
} while(0)

/*#define MAC_PPSC_TRGTMODSEL1_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_PPSC_TRGTMODSEL1_Mask (ULONG)(0x3)

/*#define MAC_PPSC_TRGTMODSEL1_Wr_Mask (ULONG)(~((~(~0 << (2))) << (13)))*/

#define MAC_PPSC_TRGTMODSEL1_Wr_Mask (ULONG)(0xffff9fff)

#define MAC_PPSC_TRGTMODSEL1_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_TRGTMODSEL1_Wr_Mask) | ((data & MAC_PPSC_TRGTMODSEL1_Mask)<<13));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_TRGTMODSEL1_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 13) & MAC_PPSC_TRGTMODSEL1_Mask);\
} while(0)

/*#define MAC_PPSC_PPSCMD1_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_PPSC_PPSCMD1_Mask (ULONG)(0x7)

/*#define MAC_PPSC_PPSCMD1_Wr_Mask (ULONG)(~((~(~0 << (3))) << (8)))*/

#define MAC_PPSC_PPSCMD1_Wr_Mask (ULONG)(0xfffff8ff)

#define MAC_PPSC_PPSCMD1_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_PPSCMD1_Wr_Mask) | ((data & MAC_PPSC_PPSCMD1_Mask)<<8));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_PPSCMD1_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 8) & MAC_PPSC_PPSCMD1_Mask);\
} while(0)

/*#define MAC_PPSC_TRGTMODSEL0_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_PPSC_TRGTMODSEL0_Mask (ULONG)(0x3)

/*#define MAC_PPSC_TRGTMODSEL0_Wr_Mask (ULONG)(~((~(~0 << (2))) << (5)))*/

#define MAC_PPSC_TRGTMODSEL0_Wr_Mask (ULONG)(0xffffff9f)

#define MAC_PPSC_TRGTMODSEL0_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_TRGTMODSEL0_Wr_Mask) | ((data & MAC_PPSC_TRGTMODSEL0_Mask)<<5));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_TRGTMODSEL0_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 5) & MAC_PPSC_TRGTMODSEL0_Mask);\
} while(0)

/*#define MAC_PPSC_PPSEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PPSC_PPSEN0_Mask (ULONG)(0x1)

/*#define MAC_PPSC_PPSEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_PPSC_PPSEN0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_PPSC_PPSEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_PPSEN0_Wr_Mask) | ((data & MAC_PPSC_PPSEN0_Mask)<<4));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_PPSEN0_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 4) & MAC_PPSC_PPSEN0_Mask);\
} while(0)

/*#define MAC_PPSC_PPSCTRL0_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_PPSC_PPSCTRL0_Mask (ULONG)(0xf)

/*#define MAC_PPSC_PPSCTRL0_Wr_Mask (ULONG)(~((~(~0 << (4))) << (0)))*/

#define MAC_PPSC_PPSCTRL0_Wr_Mask (ULONG)(0xfffffff0)

#define MAC_PPSC_PPSCTRL0_UdfWr(data) do {\
		ULONG v;\
		MAC_PPSC_RgRd(v);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_31))|((( 0) & (MAC_PPSC_Mask_31))<<31);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_27))|((( 0) & (MAC_PPSC_Mask_27))<<27);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_23))|((( 0) & (MAC_PPSC_Mask_23))<<23);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_19))|((( 0) & (MAC_PPSC_Mask_19))<<19);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_15))|((( 0) & (MAC_PPSC_Mask_15))<<15);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_11))|((( 0) & (MAC_PPSC_Mask_11))<<11);\
		v = (v & (MAC_PPSC_RES_Wr_Mask_7))|((( 0) & (MAC_PPSC_Mask_7))<<7);\
		v = ((v & MAC_PPSC_PPSCTRL0_Wr_Mask) | ((data & MAC_PPSC_PPSCTRL0_Mask)<<0));\
		MAC_PPSC_RgWr(v);\
} while(0)

#define MAC_PPSC_PPSCTRL0_UdfRd(data) do {\
		MAC_PPSC_RgRd(data);\
		data = ((data >> 0) & MAC_PPSC_PPSCTRL0_Mask);\
} while(0)


#define MAC_TEAC_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb54))

#define MAC_TEAC_RgWr(data) do {\
		iowrite32(data, (void *)MAC_TEAC_RgOffAddr);\
} while(0)

#define MAC_TEAC_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TEAC_RgOffAddr);\
} while(0)

/*#define  MAC_TEAC_Mask_31 (ULONG)(~(~0<<(1)))*/

#define  MAC_TEAC_Mask_31 (ULONG)(0x1)

/*#define MAC_TEAC_RES_Wr_Mask_31 (ULONG)(~((~(~0<<(1)))<<(31)))*/

#define MAC_TEAC_RES_Wr_Mask_31 (ULONG)(0x7fffffff)

/*#define MAC_TEAC_OSTIAC_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_TEAC_OSTIAC_Mask (ULONG)(0x7fffffff)

/*#define MAC_TEAC_OSTIAC_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_TEAC_OSTIAC_Wr_Mask (ULONG)(0x80000000)

#define MAC_TEAC_OSTIAC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MAC_TEAC_RES_Wr_Mask_31))|((( 0) & (MAC_TEAC_Mask_31))<<31);\
		(v) = ((v & MAC_TEAC_OSTIAC_Wr_Mask) | ((data & MAC_TEAC_OSTIAC_Mask)<<0));\
		MAC_TEAC_RgWr(v);\
} while(0)

#define MAC_TEAC_OSTIAC_UdfRd(data) do {\
		MAC_TEAC_RgRd(data);\
		data = ((data >> 0) & MAC_TEAC_OSTIAC_Mask);\
} while(0)


#define MAC_TIAC_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb50))

#define MAC_TIAC_RgWr(data) do {\
		iowrite32(data, (void *)MAC_TIAC_RgOffAddr);\
} while(0)

#define MAC_TIAC_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TIAC_RgOffAddr);\
} while(0)

#define MAC_TIAC_OSTIAC_UdfWr(data) do {\
		MAC_TIAC_RgWr(data);\
} while(0)

#define MAC_TIAC_OSTIAC_UdfRd(data) do {\
		MAC_TIAC_RgRd(data);\
} while(0)


#define MAC_ATS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb4c))

#define MAC_ATS_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ATS_RgOffAddr);\
} while(0)

#define MAC_ATS_AUXTSHI_UdfRd(data) do {\
		MAC_ATS_RgRd(data);\
} while(0)


#define MAC_ATN_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb48))

#define MAC_ATN_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ATN_RgOffAddr);\
} while(0)

/*#define MAC_ATN_AUXTSLO_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_ATN_AUXTSLO_Mask (ULONG)(0x7fffffff)

#define MAC_ATN_AUXTSLO_UdfRd(data) do {\
		MAC_ATN_RgRd(data);\
		data = ((data >> 0) & MAC_ATN_AUXTSLO_Mask);\
} while(0)


#define MAC_AC_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb40))

#define MAC_AC_RgWr(data) do {\
		iowrite32(data, (void *)MAC_AC_RgOffAddr);\
} while(0)

#define MAC_AC_RgRd(data) do {\
		(data) = ioread32((void *)MAC_AC_RgOffAddr);\
} while(0)

/*#define  MAC_AC_Mask_8 (ULONG)(~(~0<<(24)))*/

#define  MAC_AC_Mask_8 (ULONG)(0xffffff)

/*#define MAC_AC_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(24)))<<(8)))*/

#define MAC_AC_RES_Wr_Mask_8 (ULONG)(0xff)

/*#define  MAC_AC_Mask_1 (ULONG)(~(~0<<(3)))*/

#define  MAC_AC_Mask_1 (ULONG)(0x7)

/*#define MAC_AC_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(3)))<<(1)))*/

#define MAC_AC_RES_Wr_Mask_1 (ULONG)(0xfffffff1)

/*#define MAC_AC_ATSEN3_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AC_ATSEN3_Mask (ULONG)(0x1)

/*#define MAC_AC_ATSEN3_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_AC_ATSEN3_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_AC_ATSEN3_UdfWr(data) do {\
		ULONG v;\
		MAC_AC_RgRd(v);\
		v = (v & (MAC_AC_RES_Wr_Mask_8))|((( 0) & (MAC_AC_Mask_8))<<8);\
		v = (v & (MAC_AC_RES_Wr_Mask_1))|((( 0) & (MAC_AC_Mask_1))<<1);\
		v = ((v & MAC_AC_ATSEN3_Wr_Mask) | ((data & MAC_AC_ATSEN3_Mask)<<7));\
		MAC_AC_RgWr(v);\
} while(0)

#define MAC_AC_ATSEN3_UdfRd(data) do {\
		MAC_AC_RgRd(data);\
		data = ((data >> 7) & MAC_AC_ATSEN3_Mask);\
} while(0)

/*#define MAC_AC_ATSEN2_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AC_ATSEN2_Mask (ULONG)(0x1)

/*#define MAC_AC_ATSEN2_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MAC_AC_ATSEN2_Wr_Mask (ULONG)(0xffffffbf)

#define MAC_AC_ATSEN2_UdfWr(data) do {\
		ULONG v;\
		MAC_AC_RgRd(v);\
		v = (v & (MAC_AC_RES_Wr_Mask_8))|((( 0) & (MAC_AC_Mask_8))<<8);\
		v = (v & (MAC_AC_RES_Wr_Mask_1))|((( 0) & (MAC_AC_Mask_1))<<1);\
		v = ((v & MAC_AC_ATSEN2_Wr_Mask) | ((data & MAC_AC_ATSEN2_Mask)<<6));\
		MAC_AC_RgWr(v);\
} while(0)

#define MAC_AC_ATSEN2_UdfRd(data) do {\
		MAC_AC_RgRd(data);\
		data = ((data >> 6) & MAC_AC_ATSEN2_Mask);\
} while(0)

/*#define MAC_AC_ATSEN1_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AC_ATSEN1_Mask (ULONG)(0x1)

/*#define MAC_AC_ATSEN1_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_AC_ATSEN1_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_AC_ATSEN1_UdfWr(data) do {\
		ULONG v;\
		MAC_AC_RgRd(v);\
		v = (v & (MAC_AC_RES_Wr_Mask_8))|((( 0) & (MAC_AC_Mask_8))<<8);\
		v = (v & (MAC_AC_RES_Wr_Mask_1))|((( 0) & (MAC_AC_Mask_1))<<1);\
		v = ((v & MAC_AC_ATSEN1_Wr_Mask) | ((data & MAC_AC_ATSEN1_Mask)<<5));\
		MAC_AC_RgWr(v);\
} while(0)

#define MAC_AC_ATSEN1_UdfRd(data) do {\
		MAC_AC_RgRd(data);\
		data = ((data >> 5) & MAC_AC_ATSEN1_Mask);\
} while(0)

/*#define MAC_AC_ATSEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AC_ATSEN0_Mask (ULONG)(0x1)

/*#define MAC_AC_ATSEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_AC_ATSEN0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_AC_ATSEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_AC_RgRd(v);\
		v = (v & (MAC_AC_RES_Wr_Mask_8))|((( 0) & (MAC_AC_Mask_8))<<8);\
		v = (v & (MAC_AC_RES_Wr_Mask_1))|((( 0) & (MAC_AC_Mask_1))<<1);\
		v = ((v & MAC_AC_ATSEN0_Wr_Mask) | ((data & MAC_AC_ATSEN0_Mask)<<4));\
		MAC_AC_RgWr(v);\
} while(0)

#define MAC_AC_ATSEN0_UdfRd(data) do {\
		MAC_AC_RgRd(data);\
		data = ((data >> 4) & MAC_AC_ATSEN0_Mask);\
} while(0)

/*#define MAC_AC_ATSFC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_AC_ATSFC_Mask (ULONG)(0x1)

/*#define MAC_AC_ATSFC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_AC_ATSFC_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_AC_ATSFC_UdfWr(data) do {\
		ULONG v;\
		MAC_AC_RgRd(v);\
		v = (v & (MAC_AC_RES_Wr_Mask_8))|((( 0) & (MAC_AC_Mask_8))<<8);\
		v = (v & (MAC_AC_RES_Wr_Mask_1))|((( 0) & (MAC_AC_Mask_1))<<1);\
		v = ((v & MAC_AC_ATSFC_Wr_Mask) | ((data & MAC_AC_ATSFC_Mask)<<0));\
		MAC_AC_RgWr(v);\
} while(0)

#define MAC_AC_ATSFC_UdfRd(data) do {\
		MAC_AC_RgRd(data);\
		data = ((data >> 0) & MAC_AC_ATSFC_Mask);\
} while(0)


#define MAC_TTN_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb34))

#define MAC_TTN_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TTN_RgOffAddr);\
} while(0)

#define MAC_TTN_TXTSSTSHI_UdfRd(data) do {\
		MAC_TTN_RgRd(data);\
} while(0)


#define MAC_TTSN_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb30))

#define MAC_TTSN_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TTSN_RgOffAddr);\
} while(0)

/*#define MAC_TTSN_TXTSSTSMIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TTSN_TXTSSTSMIS_Mask (ULONG)(0x1)

#define MAC_TTSN_TXTSSTSMIS_UdfRd(data) do {\
		MAC_TTSN_RgRd(data);\
		data = ((data >> 31) & MAC_TTSN_TXTSSTSMIS_Mask);\
} while(0)

/*#define MAC_TTSN_TXTSSTSLO_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_TTSN_TXTSSTSLO_Mask (ULONG)(0x7fffffff)

#define MAC_TTSN_TXTSSTSLO_UdfRd(data) do {\
		MAC_TTSN_RgRd(data);\
		data = ((data >> 0) & MAC_TTSN_TXTSSTSLO_Mask);\
} while(0)


#define MAC_TSR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb20))

#define MAC_TSR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TSR_RgOffAddr);\
} while(0)

/*#define MAC_TSR_ATSNS_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_TSR_ATSNS_Mask (ULONG)(0x1f)

#define MAC_TSR_ATSNS_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 25) & MAC_TSR_ATSNS_Mask);\
} while(0)

/*#define MAC_TSR_ATSSTM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_ATSSTM_Mask (ULONG)(0x1)

#define MAC_TSR_ATSSTM_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 24) & MAC_TSR_ATSSTM_Mask);\
} while(0)

/*#define MAC_TSR_ATSSTN_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_TSR_ATSSTN_Mask (ULONG)(0xf)

#define MAC_TSR_ATSSTN_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 16) & MAC_TSR_ATSSTN_Mask);\
} while(0)

/*#define MAC_TSR_TSTRGTERR3_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTRGTERR3_Mask (ULONG)(0x1)

#define MAC_TSR_TSTRGTERR3_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 9) & MAC_TSR_TSTRGTERR3_Mask);\
} while(0)

/*#define MAC_TSR_TSTARGT3_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTARGT3_Mask (ULONG)(0x1)

#define MAC_TSR_TSTARGT3_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 8) & MAC_TSR_TSTARGT3_Mask);\
} while(0)

/*#define MAC_TSR_TSTRGTERR2_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTRGTERR2_Mask (ULONG)(0x1)

#define MAC_TSR_TSTRGTERR2_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 7) & MAC_TSR_TSTRGTERR2_Mask);\
} while(0)

/*#define MAC_TSR_TSTARGT2_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTARGT2_Mask (ULONG)(0x1)

#define MAC_TSR_TSTARGT2_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 6) & MAC_TSR_TSTARGT2_Mask);\
} while(0)

/*#define MAC_TSR_TSTRGTERR1_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTRGTERR1_Mask (ULONG)(0x1)

#define MAC_TSR_TSTRGTERR1_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 5) & MAC_TSR_TSTRGTERR1_Mask);\
} while(0)

/*#define MAC_TSR_TSTARGT1_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTARGT1_Mask (ULONG)(0x1)

#define MAC_TSR_TSTARGT1_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 4) & MAC_TSR_TSTARGT1_Mask);\
} while(0)

/*#define MAC_TSR_TSTRGTERR0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTRGTERR0_Mask (ULONG)(0x1)

#define MAC_TSR_TSTRGTERR0_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 3) & MAC_TSR_TSTRGTERR0_Mask);\
} while(0)

/*#define MAC_TSR_AUXTSTRIG_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_AUXTSTRIG_Mask (ULONG)(0x1)

#define MAC_TSR_AUXTSTRIG_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 2) & MAC_TSR_AUXTSTRIG_Mask);\
} while(0)

/*#define MAC_TSR_TSTARGT0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSTARGT0_Mask (ULONG)(0x1)

#define MAC_TSR_TSTARGT0_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 1) & MAC_TSR_TSTARGT0_Mask);\
} while(0)

/*#define MAC_TSR_TSSOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TSR_TSSOVF_Mask (ULONG)(0x1)

#define MAC_TSR_TSSOVF_UdfRd(data) do {\
		MAC_TSR_RgRd(data);\
		data = ((data >> 0) & MAC_TSR_TSSOVF_Mask);\
} while(0)


#define MAC_STHWR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb1c))

#define MAC_STHWR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_STHWR_RgOffAddr);\
} while(0)

#define MAC_STHWR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_STHWR_RgOffAddr);\
} while(0)

/*#define  MAC_STHWR_Mask_16 (ULONG)(~(~0<<(16)))*/

#define  MAC_STHWR_Mask_16 (ULONG)(0xffff)

/*#define MAC_STHWR_RES_Wr_Mask_16 (ULONG)(~((~(~0<<(16)))<<(16)))*/

#define MAC_STHWR_RES_Wr_Mask_16 (ULONG)(0xffff)

/*#define MAC_STHWR_TSHWR_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_STHWR_TSHWR_Mask (ULONG)(0xffff)

/*#define MAC_STHWR_TSHWR_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_STHWR_TSHWR_Wr_Mask (ULONG)(0xffff0000)

#define MAC_STHWR_TSHWR_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MAC_STHWR_RES_Wr_Mask_16))|((( 0) & (MAC_STHWR_Mask_16))<<16);\
		(v) = ((v & MAC_STHWR_TSHWR_Wr_Mask) | ((data & MAC_STHWR_TSHWR_Mask)<<0));\
		MAC_STHWR_RgWr(v);\
} while(0)

#define MAC_STHWR_TSHWR_UdfRd(data) do {\
		MAC_STHWR_RgRd(data);\
		data = ((data >> 0) & MAC_STHWR_TSHWR_Mask);\
} while(0)


#define MAC_TAR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb18))

#define MAC_TAR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_TAR_RgOffAddr);\
} while(0)

#define MAC_TAR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TAR_RgOffAddr);\
} while(0)

#define MAC_TAR_TSAR_UdfWr(data) do {\
		MAC_TAR_RgWr(data);\
} while(0)

#define MAC_TAR_TSAR_UdfRd(data) do {\
		MAC_TAR_RgRd(data);\
} while(0)


#define MAC_STNSUR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb14))

#define MAC_STNSUR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_STNSUR_RgOffAddr);\
} while(0)

#define MAC_STNSUR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_STNSUR_RgOffAddr);\
} while(0)

/*#define MAC_STNSUR_ADDSUB_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_STNSUR_ADDSUB_Mask (ULONG)(0x1)

/*#define MAC_STNSUR_ADDSUB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_STNSUR_ADDSUB_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_STNSUR_ADDSUB_UdfWr(data) do{\
		ULONG v;\
		MAC_STNSUR_RgRd(v);\
		v = ((v & MAC_STNSUR_ADDSUB_Wr_Mask) | ((data & MAC_STNSUR_ADDSUB_Mask)<<31));\
		MAC_STNSUR_RgWr(v);\
} while(0)

#define MAC_STNSUR_ADDSUB_UdfRd(data) do {\
		MAC_STNSUR_RgRd(data);\
		data = ((data >> 31) & MAC_STNSUR_ADDSUB_Mask);\
} while(0)

/*#define MAC_STNSUR_TSSS_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_STNSUR_TSSS_Mask (ULONG)(0x7fffffff)

/*#define MAC_STNSUR_TSSS_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_STNSUR_TSSS_Wr_Mask (ULONG)(0x80000000)

#define MAC_STNSUR_TSSS_UdfWr(data) do{\
		ULONG v;\
		MAC_STNSUR_RgRd(v);\
		v = ((v & MAC_STNSUR_TSSS_Wr_Mask) | ((data & MAC_STNSUR_TSSS_Mask)<<0));\
		MAC_STNSUR_RgWr(v);\
} while(0)

#define MAC_STNSUR_TSSS_UdfRd(data) do {\
		MAC_STNSUR_RgRd(data);\
		data = ((data >> 0) & MAC_STNSUR_TSSS_Mask);\
} while(0)


#define MAC_STSUR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb10))

#define MAC_STSUR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_STSUR_RgOffAddr);\
} while(0)

#define MAC_STSUR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_STSUR_RgOffAddr);\
} while(0)

#define MAC_STSUR_TSS_UdfWr(data) do {\
		MAC_STSUR_RgWr(data);\
} while(0)

#define MAC_STSUR_TSS_UdfRd(data) do {\
		MAC_STSUR_RgRd(data);\
} while(0)


#define MAC_STNSR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb0c))

#define MAC_STNSR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_STNSR_RgOffAddr);\
} while(0)

/*#define MAC_STNSR_TSSS_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_STNSR_TSSS_Mask (ULONG)(0x7fffffff)

#define MAC_STNSR_TSSS_UdfRd(data) do {\
		MAC_STNSR_RgRd(data);\
		data = ((data >> 0) & MAC_STNSR_TSSS_Mask);\
} while(0)


#define MAC_STSR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb08))

#define MAC_STSR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_STSR_RgOffAddr);\
} while(0)

#define MAC_STSR_TSS_UdfRd(data) do {\
		MAC_STSR_RgRd(data);\
} while(0)


#define MAC_SSIR_RgOffAddr ((volatile unsigned int *)(BASE_ADDRESS + 0xb04))

#define MAC_SSIR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_SSIR_RgOffAddr);\
} while(0)

#define MAC_SSIR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_SSIR_RgOffAddr);\
} while(0)

/*#define  MAC_SSIR_Mask_24 (unsigned int)(~(~0<<(8)))*/

#define  MAC_SSIR_Mask_24 (unsigned int)(0xff)

/*#define MAC_SSIR_RES_Wr_Mask_24 (unsigned int)(~((~(~0<<(8)))<<(24)))*/

#define MAC_SSIR_RES_Wr_Mask_24 (unsigned int)(0xffffff)

/*#define  MAC_SSIR_Mask_0 (unsigned int)(~(~0<<(8)))*/

#define  MAC_SSIR_Mask_0 (unsigned int)(0xff)

/*#define MAC_SSIR_RES_Wr_Mask_0 (unsigned int)(~((~(~0<<(8)))<<(0)))*/

#define MAC_SSIR_RES_Wr_Mask_0 (unsigned int)(0xffffff00)

/*#define MAC_SSIR_SSINC_Mask (unsigned int)(~(~0<<(8)))*/

#define MAC_SSIR_SSINC_Mask (unsigned int)(0xff)

/*#define MAC_SSIR_SSINC_Wr_Mask (unsigned int)(~((~(~0 << (8))) << (16)))*/

#define MAC_SSIR_SSINC_Wr_Mask (unsigned int)(0xff00ffff)

#define MAC_SSIR_SSINC_UdfWr(data) do {\
		unsigned int v;\
		MAC_SSIR_RgRd(v);\
		v = (v & (MAC_SSIR_RES_Wr_Mask_24))|((( 0) & (MAC_SSIR_Mask_24))<<24);\
		v = (v & (MAC_SSIR_RES_Wr_Mask_0))|((( 0) & (MAC_SSIR_Mask_0))<<0);\
		v = ((v & MAC_SSIR_SSINC_Wr_Mask) | ((data & MAC_SSIR_SSINC_Mask)<<16));\
		MAC_SSIR_RgWr(v);\
} while(0)

#define MAC_SSIR_SSINC_UdfRd(data) do {\
		MAC_SSIR_RgRd(data);\
		data = ((data >> 16) & MAC_SSIR_SSINC_Mask);\
} while(0)

/*#define MAC_SSIR_SNSINC_Mask (unsigned int)(~(~0<<(8)))*/

#define MAC_SSIR_SNSINC_Mask (unsigned int)(0xff)

/*#define MAC_SSIR_SNSINC_Wr_Mask (unsigned int)(~((~(~0 << (8))) << (8)))*/

#define MAC_SSIR_SNSINC_Wr_Mask (unsigned int)(0xffff00ff)

#define MAC_SSIR_SNSINC_UdfWr(data) do {\
		unsigned int v;\
		MAC_SSIR_RgRd(v);\
		v = (v & (MAC_SSIR_RES_Wr_Mask_24))|((( 0) & (MAC_SSIR_Mask_24))<<24);\
		v = (v & (MAC_SSIR_RES_Wr_Mask_0))|((( 0) & (MAC_SSIR_Mask_0))<<0);\
		v = ((v & MAC_SSIR_SNSINC_Wr_Mask) | ((data & MAC_SSIR_SNSINC_Mask)<<8));\
		MAC_SSIR_RgWr(v);\
} while(0)

#define MAC_SSIR_SNSINC_UdfRd(data) do {\
		MAC_SSIR_RgRd(data);\
		data = ((data >> 8) & MAC_SSIR_SNSINC_Mask);\
} while(0)

#define MAC_TCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb00))

#define MAC_TCR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_TCR_RgOffAddr);\
} while(0)

#define MAC_TCR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TCR_RgOffAddr);\
} while(0)

/*#define  MAC_TCR_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MAC_TCR_Mask_29 (ULONG)(0x7)

/*#define MAC_TCR_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MAC_TCR_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define  MAC_TCR_Mask_25 (ULONG)(~(~0<<(3)))*/

#define  MAC_TCR_Mask_25 (ULONG)(0x7)

/*#define MAC_TCR_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(3)))<<(25)))*/

#define MAC_TCR_RES_Wr_Mask_25 (ULONG)(0xf1ffffff)

/*#define  MAC_TCR_Mask_21 (ULONG)(~(~0<<(3)))*/

#define  MAC_TCR_Mask_21 (ULONG)(0x7)

/*#define MAC_TCR_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(3)))<<(21)))*/

#define MAC_TCR_RES_Wr_Mask_21 (ULONG)(0xff1fffff)

/*#define  MAC_TCR_Mask_19 (ULONG)(~(~0<<(1)))*/

#define  MAC_TCR_Mask_19 (ULONG)(0x1)

/*#define MAC_TCR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(1)))<<(19)))*/

#define MAC_TCR_RES_Wr_Mask_19 (ULONG)(0xfff7ffff)

/*#define  MAC_TCR_Mask_6 (ULONG)(~(~0<<(2)))*/

#define  MAC_TCR_Mask_6 (ULONG)(0x3)

/*#define MAC_TCR_RES_Wr_Mask_6 (ULONG)(~((~(~0<<(2)))<<(6)))*/

#define MAC_TCR_RES_Wr_Mask_6 (ULONG)(0xffffff3f)

/*#define MAC_TCR_AV8021ASMEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_AV8021ASMEN_Mask (ULONG)(0x1)

/*#define MAC_TCR_AV8021ASMEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (28)))*/

#define MAC_TCR_AV8021ASMEN_Wr_Mask (ULONG)(0xefffffff)

#define MAC_TCR_AV8021ASMEN_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_AV8021ASMEN_Wr_Mask) | ((data & MAC_TCR_AV8021ASMEN_Mask)<<28));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_AV8021ASMEN_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 28) & MAC_TCR_AV8021ASMEN_Mask);\
} while(0)

/*#define MAC_TCR_TXTSSTSM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TXTSSTSM_Mask (ULONG)(0x1)

/*#define MAC_TCR_TXTSSTSM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MAC_TCR_TXTSSTSM_Wr_Mask (ULONG)(0xfeffffff)

#define MAC_TCR_TXTSSTSM_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TXTSSTSM_Wr_Mask) | ((data & MAC_TCR_TXTSSTSM_Mask)<<24));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TXTSSTSM_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 24) & MAC_TCR_TXTSSTSM_Mask);\
} while(0)

/*#define MAC_TCR_ESTI_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_ESTI_Mask (ULONG)(0x1)

/*#define MAC_TCR_ESTI_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_TCR_ESTI_Wr_Mask (ULONG)(0xffefffff)

#define MAC_TCR_ESTI_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_ESTI_Wr_Mask) | ((data & MAC_TCR_ESTI_Mask)<<20));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_ESTI_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 20) & MAC_TCR_ESTI_Mask);\
} while(0)

/*#define MAC_TCR_TSENMACADDR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSENMACADDR_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSENMACADDR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_TCR_TSENMACADDR_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_TCR_TSENMACADDR_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSENMACADDR_Wr_Mask) | ((data & MAC_TCR_TSENMACADDR_Mask)<<18));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSENMACADDR_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 18) & MAC_TCR_TSENMACADDR_Mask);\
} while(0)

/*#define MAC_TCR_SNAPTYPSEL_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_TCR_SNAPTYPSEL_Mask (ULONG)(0x3)

/*#define MAC_TCR_SNAPTYPSEL_Wr_Mask (ULONG)(~((~(~0 << (2))) << (16)))*/

#define MAC_TCR_SNAPTYPSEL_Wr_Mask (ULONG)(0xfffcffff)

#define MAC_TCR_SNAPTYPSEL_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_SNAPTYPSEL_Wr_Mask) | ((data & MAC_TCR_SNAPTYPSEL_Mask)<<16));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_SNAPTYPSEL_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 16) & MAC_TCR_SNAPTYPSEL_Mask);\
} while(0)

/*#define MAC_TCR_TSMSTRENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSMSTRENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSMSTRENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MAC_TCR_TSMSTRENA_Wr_Mask (ULONG)(0xffff7fff)

#define MAC_TCR_TSMSTRENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSMSTRENA_Wr_Mask) | ((data & MAC_TCR_TSMSTRENA_Mask)<<15));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSMSTRENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 15) & MAC_TCR_TSMSTRENA_Mask);\
} while(0)

/*#define MAC_TCR_TSEVNTENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSEVNTENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSEVNTENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MAC_TCR_TSEVNTENA_Wr_Mask (ULONG)(0xffffbfff)

#define MAC_TCR_TSEVNTENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSEVNTENA_Wr_Mask) | ((data & MAC_TCR_TSEVNTENA_Mask)<<14));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSEVNTENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 14) & MAC_TCR_TSEVNTENA_Mask);\
} while(0)

/*#define MAC_TCR_TSIPV4ENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSIPV4ENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSIPV4ENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MAC_TCR_TSIPV4ENA_Wr_Mask (ULONG)(0xffffdfff)

#define MAC_TCR_TSIPV4ENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSIPV4ENA_Wr_Mask) | ((data & MAC_TCR_TSIPV4ENA_Mask)<<13));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSIPV4ENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 13) & MAC_TCR_TSIPV4ENA_Mask);\
} while(0)

/*#define MAC_TCR_TSIPV6ENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSIPV6ENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSIPV6ENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MAC_TCR_TSIPV6ENA_Wr_Mask (ULONG)(0xffffefff)

#define MAC_TCR_TSIPV6ENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSIPV6ENA_Wr_Mask) | ((data & MAC_TCR_TSIPV6ENA_Mask)<<12));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSIPV6ENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 12) & MAC_TCR_TSIPV6ENA_Mask);\
} while(0)

/*#define MAC_TCR_TSIPENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSIPENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSIPENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MAC_TCR_TSIPENA_Wr_Mask (ULONG)(0xfffff7ff)

#define MAC_TCR_TSIPENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSIPENA_Wr_Mask) | ((data & MAC_TCR_TSIPENA_Mask)<<11));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSIPENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 11) & MAC_TCR_TSIPENA_Mask);\
} while(0)

/*#define MAC_TCR_TSVER2ENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSVER2ENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSVER2ENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MAC_TCR_TSVER2ENA_Wr_Mask (ULONG)(0xfffffbff)

#define MAC_TCR_TSVER2ENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSVER2ENA_Wr_Mask) | ((data & MAC_TCR_TSVER2ENA_Mask)<<10));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSVER2ENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 10) & MAC_TCR_TSVER2ENA_Mask);\
} while(0)

/*#define MAC_TCR_TSCTRLSSR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSCTRLSSR_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSCTRLSSR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MAC_TCR_TSCTRLSSR_Wr_Mask (ULONG)(0xfffffdff)

#define MAC_TCR_TSCTRLSSR_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSCTRLSSR_Wr_Mask) | ((data & MAC_TCR_TSCTRLSSR_Mask)<<9));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSCTRLSSR_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 9) & MAC_TCR_TSCTRLSSR_Mask);\
} while(0)

/*#define MAC_TCR_TSENALL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSENALL_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSENALL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MAC_TCR_TSENALL_Wr_Mask (ULONG)(0xfffffeff)

#define MAC_TCR_TSENALL_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSENALL_Wr_Mask) | ((data & MAC_TCR_TSENALL_Mask)<<8));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSENALL_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 8) & MAC_TCR_TSENALL_Mask);\
} while(0)

/*#define MAC_TCR_TSADDREG_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSADDREG_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSADDREG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_TCR_TSADDREG_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_TCR_TSADDREG_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSADDREG_Wr_Mask) | ((data & MAC_TCR_TSADDREG_Mask)<<5));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSADDREG_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 5) & MAC_TCR_TSADDREG_Mask);\
} while(0)

/*#define MAC_TCR_TSTRIG_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSTRIG_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSTRIG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_TCR_TSTRIG_Wr_Mask (ULONG)(0xffffffef)

#define MAC_TCR_TSTRIG_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSTRIG_Wr_Mask) | ((data & MAC_TCR_TSTRIG_Mask)<<4));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSTRIG_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 4) & MAC_TCR_TSTRIG_Mask);\
} while(0)

/*#define MAC_TCR_TSUPDT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSUPDT_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSUPDT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_TCR_TSUPDT_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_TCR_TSUPDT_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSUPDT_Wr_Mask) | ((data & MAC_TCR_TSUPDT_Mask)<<3));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSUPDT_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 3) & MAC_TCR_TSUPDT_Mask);\
} while(0)

/*#define MAC_TCR_TSINIT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSINIT_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSINIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_TCR_TSINIT_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_TCR_TSINIT_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSINIT_Wr_Mask) | ((data & MAC_TCR_TSINIT_Mask)<<2));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSINIT_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 2) & MAC_TCR_TSINIT_Mask);\
} while(0)

/*#define MAC_TCR_TSCFUPDT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSCFUPDT_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSCFUPDT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_TCR_TSCFUPDT_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_TCR_TSCFUPDT_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSCFUPDT_Wr_Mask) | ((data & MAC_TCR_TSCFUPDT_Mask)<<1));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSCFUPDT_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 1) & MAC_TCR_TSCFUPDT_Mask);\
} while(0)

/*#define MAC_TCR_TSENA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_TCR_TSENA_Mask (ULONG)(0x1)

/*#define MAC_TCR_TSENA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_TCR_TSENA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_TCR_TSENA_UdfWr(data) do {\
		ULONG v;\
		MAC_TCR_RgRd(v);\
		v = (v & (MAC_TCR_RES_Wr_Mask_29))|((( 0) & (MAC_TCR_Mask_29))<<29);\
		v = (v & (MAC_TCR_RES_Wr_Mask_25))|((( 0) & (MAC_TCR_Mask_25))<<25);\
		v = (v & (MAC_TCR_RES_Wr_Mask_21))|((( 0) & (MAC_TCR_Mask_21))<<21);\
		v = (v & (MAC_TCR_RES_Wr_Mask_19))|((( 0) & (MAC_TCR_Mask_19))<<19);\
		v = (v & (MAC_TCR_RES_Wr_Mask_6))|((( 0) & (MAC_TCR_Mask_6))<<6);\
		v = ((v & MAC_TCR_TSENA_Wr_Mask) | ((data & MAC_TCR_TSENA_Mask)<<0));\
		MAC_TCR_RgWr(v);\
} while(0)

#define MAC_TCR_TSENA_UdfRd(data) do {\
		MAC_TCR_RgRd(data);\
		data = ((data >> 0) & MAC_TCR_TSENA_Mask);\
} while(0)


#define MTL_DSR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc0c))

#define MTL_DSR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_DSR_RgOffAddr);\
} while(0)

#define MTL_DSR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_DSR_RgOffAddr);\
} while(0)

/*#define  MTL_DSR_Mask_10 (ULONG)(~(~0<<(6)))*/

#define  MTL_DSR_Mask_10 (ULONG)(0x3f)

/*#define MTL_DSR_RES_Wr_Mask_10 (ULONG)(~((~(~0<<(6)))<<(10)))*/

#define MTL_DSR_RES_Wr_Mask_10 (ULONG)(0xffff03ff)

/*#define  MTL_DSR_Mask_5 (ULONG)(~(~0<<(3)))*/

#define  MTL_DSR_Mask_5 (ULONG)(0x7)

/*#define MTL_DSR_RES_Wr_Mask_5 (ULONG)(~((~(~0<<(3)))<<(5)))*/

#define MTL_DSR_RES_Wr_Mask_5 (ULONG)(0xffffff1f)

/*#define MTL_DSR_LOCR_Mask (ULONG)(~(~0<<(16)))*/

#define MTL_DSR_LOCR_Mask (ULONG)(0xffff)

#define MTL_DSR_LOCR_UdfRd(data) do {\
		MTL_DSR_RgRd(data);\
		data = ((data >> 16) & MTL_DSR_LOCR_Mask);\
} while(0)

/*#define MTL_DSR_STSI_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_DSR_STSI_Mask (ULONG)(0x1)

#define MTL_DSR_STSI_UdfRd(data) do {\
		MTL_DSR_RgRd(data);\
		data = ((data >> 9) & MTL_DSR_STSI_Mask);\
} while(0)

/*#define MTL_DSR_PKTI_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_DSR_PKTI_Mask (ULONG)(0x1)

#define MTL_DSR_PKTI_UdfRd(data) do {\
		MTL_DSR_RgRd(data);\
		data = ((data >> 8) & MTL_DSR_PKTI_Mask);\
} while(0)

/*#define MTL_DSR_BYTEEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_DSR_BYTEEN_Mask (ULONG)(0x3)

/*#define MTL_DSR_BYTEEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (3)))*/

#define MTL_DSR_BYTEEN_Wr_Mask (ULONG)(0xffffffe7)

#define MTL_DSR_BYTEEN_UdfWr(data) do {\
		ULONG v;\
		MTL_DSR_RgRd(v);\
		v = (v & (MTL_DSR_RES_Wr_Mask_10))|((( 0) & (MTL_DSR_Mask_10))<<10);\
		v = (v & (MTL_DSR_RES_Wr_Mask_5))|((( 0) & (MTL_DSR_Mask_5))<<5);\
		v = ((v & MTL_DSR_BYTEEN_Wr_Mask) | ((data & MTL_DSR_BYTEEN_Mask)<<3));\
		MTL_DSR_RgWr(v);\
} while(0)

#define MTL_DSR_BYTEEN_UdfRd(data) do {\
		MTL_DSR_RgRd(data);\
		data = ((data >> 3) & MTL_DSR_BYTEEN_Mask);\
} while(0)

/*#define MTL_DSR_PKTSTATE_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_DSR_PKTSTATE_Mask (ULONG)(0x3)

#define MTL_DSR_PKTSTATE_UdfRd(data) do {\
		MTL_DSR_RgRd(data);\
		data = ((data >> 1) & MTL_DSR_PKTSTATE_Mask);\
} while(0)

/*#define MTL_DSR_FIFOBUSY_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_DSR_FIFOBUSY_Mask (ULONG)(0x1)

#define MTL_DSR_FIFOBUSY_UdfRd(data) do {\
		MTL_DSR_RgRd(data);\
		data = ((data >> 0) & MTL_DSR_FIFOBUSY_Mask);\
} while(0)


#define MAC_RWPFFR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc4))

#define MAC_RWPFFR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_RWPFFR_RgOffAddr);\
} while(0)

#define MAC_RWPFFR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RWPFFR_RgOffAddr);\
} while(0)

#define MAC_RWPFFR_MAC_RWPFF_UdfWr(data) do {\
		MAC_RWPFFR_RgWr(data);\
} while(0)

#define MAC_RWPFFR_MAC_RWPFF_UdfRd(data) do {\
		MAC_RWPFFR_RgRd(data);\
} while(0)


#define MAC_RTSR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb8))

#define MAC_RTSR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RTSR_RgOffAddr);\
} while(0)

/*#define MAC_RTSR_RWT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_RWT_Mask (ULONG)(0x1)

#define MAC_RTSR_RWT_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 8) & MAC_RTSR_RWT_Mask);\
} while(0)

/*#define MAC_RTSR_EXCOL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_EXCOL_Mask (ULONG)(0x1)

#define MAC_RTSR_EXCOL_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 5) & MAC_RTSR_EXCOL_Mask);\
} while(0)

/*#define MAC_RTSR_LCOL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_LCOL_Mask (ULONG)(0x1)

#define MAC_RTSR_LCOL_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 4) & MAC_RTSR_LCOL_Mask);\
} while(0)

/*#define MAC_RTSR_EXDEF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_EXDEF_Mask (ULONG)(0x1)

#define MAC_RTSR_EXDEF_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 3) & MAC_RTSR_EXDEF_Mask);\
} while(0)

/*#define MAC_RTSR_LCARR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_LCARR_Mask (ULONG)(0x1)

#define MAC_RTSR_LCARR_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 2) & MAC_RTSR_LCARR_Mask);\
} while(0)

/*#define MAC_RTSR_NCARR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_NCARR_Mask (ULONG)(0x1)

#define MAC_RTSR_NCARR_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 1) & MAC_RTSR_NCARR_Mask);\
} while(0)

/*#define MAC_RTSR_TJT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RTSR_TJT_Mask (ULONG)(0x1)

#define MAC_RTSR_TJT_UdfRd(data) do {\
		MAC_RTSR_RgRd(data);\
		data = ((data >> 0) & MAC_RTSR_TJT_Mask);\
} while(0)


#define MTL_IER_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc1c))

#define MTL_IER_RgWr(data) do {\
		iowrite32(data, (void *)MTL_IER_RgOffAddr);\
} while(0)

#define MTL_IER_RgRd(data) do {\
		(data) = ioread32((void *)MTL_IER_RgOffAddr);\
} while(0)

/*#define  MTL_IER_Mask_17 (ULONG)(~(~0<<(15)))*/

#define  MTL_IER_Mask_17 (ULONG)(0x7fff)

/*#define MTL_IER_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(15)))<<(17)))*/

#define MTL_IER_RES_Wr_Mask_17 (ULONG)(0x1ffff)

/*#define MTL_IER_MACIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_MACIE_Mask (ULONG)(0x1)

/*#define MTL_IER_MACIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MTL_IER_MACIE_Wr_Mask (ULONG)(0xfffeffff)

#define MTL_IER_MACIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_MACIE_Wr_Mask) | ((data & MTL_IER_MACIE_Mask)<<16));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_MACIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 16) & MTL_IER_MACIE_Mask);\
} while(0)

/*#define MTL_IER_Q7RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q7RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q7RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MTL_IER_Q7RXOIE_Wr_Mask (ULONG)(0xffff7fff)

#define MTL_IER_Q7RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q7RXOIE_Wr_Mask) | ((data & MTL_IER_Q7RXOIE_Mask)<<15));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q7RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 15) & MTL_IER_Q7RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q7TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q7TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q7TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MTL_IER_Q7TXUIE_Wr_Mask (ULONG)(0xffffbfff)

#define MTL_IER_Q7TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q7TXUIE_Wr_Mask) | ((data & MTL_IER_Q7TXUIE_Mask)<<14));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q7TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 14) & MTL_IER_Q7TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q6RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q6RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q6RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MTL_IER_Q6RXOIE_Wr_Mask (ULONG)(0xffffdfff)

#define MTL_IER_Q6RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q6RXOIE_Wr_Mask) | ((data & MTL_IER_Q6RXOIE_Mask)<<13));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q6RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 13) & MTL_IER_Q6RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q6TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q6TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q6TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MTL_IER_Q6TXUIE_Wr_Mask (ULONG)(0xffffefff)

#define MTL_IER_Q6TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q6TXUIE_Wr_Mask) | ((data & MTL_IER_Q6TXUIE_Mask)<<12));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q6TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 12) & MTL_IER_Q6TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q5RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q5RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q5RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_IER_Q5RXOIE_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_IER_Q5RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q5RXOIE_Wr_Mask) | ((data & MTL_IER_Q5RXOIE_Mask)<<11));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q5RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 11) & MTL_IER_Q5RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q5TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q5TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q5TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MTL_IER_Q5TXUIE_Wr_Mask (ULONG)(0xfffffbff)

#define MTL_IER_Q5TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q5TXUIE_Wr_Mask) | ((data & MTL_IER_Q5TXUIE_Mask)<<10));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q5TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 10) & MTL_IER_Q5TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q4RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q4RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q4RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MTL_IER_Q4RXOIE_Wr_Mask (ULONG)(0xfffffdff)

#define MTL_IER_Q4RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q4RXOIE_Wr_Mask) | ((data & MTL_IER_Q4RXOIE_Mask)<<9));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q4RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 9) & MTL_IER_Q4RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q4TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q4TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q4TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MTL_IER_Q4TXUIE_Wr_Mask (ULONG)(0xfffffeff)

#define MTL_IER_Q4TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q4TXUIE_Wr_Mask) | ((data & MTL_IER_Q4TXUIE_Mask)<<8));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q4TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 8) & MTL_IER_Q4TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q3RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q3RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q3RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MTL_IER_Q3RXOIE_Wr_Mask (ULONG)(0xffffff7f)

#define MTL_IER_Q3RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q3RXOIE_Wr_Mask) | ((data & MTL_IER_Q3RXOIE_Mask)<<7));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q3RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 7) & MTL_IER_Q3RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q3TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q3TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q3TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MTL_IER_Q3TXUIE_Wr_Mask (ULONG)(0xffffffbf)

#define MTL_IER_Q3TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q3TXUIE_Wr_Mask) | ((data & MTL_IER_Q3TXUIE_Mask)<<6));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q3TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 6) & MTL_IER_Q3TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q2RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q2RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q2RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MTL_IER_Q2RXOIE_Wr_Mask (ULONG)(0xffffffdf)

#define MTL_IER_Q2RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q2RXOIE_Wr_Mask) | ((data & MTL_IER_Q2RXOIE_Mask)<<5));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q2RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 5) & MTL_IER_Q2RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q2TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q2TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q2TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MTL_IER_Q2TXUIE_Wr_Mask (ULONG)(0xffffffef)

#define MTL_IER_Q2TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q2TXUIE_Wr_Mask) | ((data & MTL_IER_Q2TXUIE_Mask)<<4));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q2TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 4) & MTL_IER_Q2TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q1RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q1RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q1RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_IER_Q1RXOIE_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_IER_Q1RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q1RXOIE_Wr_Mask) | ((data & MTL_IER_Q1RXOIE_Mask)<<3));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q1RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 3) & MTL_IER_Q1RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q1TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q1TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q1TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_IER_Q1TXUIE_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_IER_Q1TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q1TXUIE_Wr_Mask) | ((data & MTL_IER_Q1TXUIE_Mask)<<2));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q1TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 2) & MTL_IER_Q1TXUIE_Mask);\
} while(0)

/*#define MTL_IER_Q0RXOIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q0RXOIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q0RXOIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_IER_Q0RXOIE_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_IER_Q0RXOIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q0RXOIE_Wr_Mask) | ((data & MTL_IER_Q0RXOIE_Mask)<<1));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q0RXOIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 1) & MTL_IER_Q0RXOIE_Mask);\
} while(0)

/*#define MTL_IER_Q0TXUIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_IER_Q0TXUIE_Mask (ULONG)(0x1)

/*#define MTL_IER_Q0TXUIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_IER_Q0TXUIE_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_IER_Q0TXUIE_UdfWr(data) do {\
		ULONG v;\
		MTL_IER_RgRd(v);\
		v = (v & (MTL_IER_RES_Wr_Mask_17))|((( 0) & (MTL_IER_Mask_17))<<17);\
		v = ((v & MTL_IER_Q0TXUIE_Wr_Mask) | ((data & MTL_IER_Q0TXUIE_Mask)<<0));\
		MTL_IER_RgWr(v);\
} while(0)

#define MTL_IER_Q0TXUIE_UdfRd(data) do {\
		MTL_IER_RgRd(data);\
		data = ((data >> 0) & MTL_IER_Q0TXUIE_Mask);\
} while(0)


#define MTL_QRCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xefc))

#define MTL_QRCR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR7_RgOffAddr);\
} while(0)

#define MTL_QRCR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR7_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR7_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR7_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR7_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR7_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR7_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR7_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR7_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR7_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR7_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR7_RgRd(v);\
		v = (v & (MTL_QRCR7_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR7_Mask_4))<<4);\
		v = ((v & MTL_QRCR7_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR7_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR7_RgWr(v);\
} while(0)

#define MTL_QRCR7_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR7_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR7_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR7_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR7_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR7_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR7_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR7_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR7_RgRd(v);\
		v = (v & (MTL_QRCR7_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR7_Mask_4))<<4);\
		v = ((v & MTL_QRCR7_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR7_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR7_RgWr(v);\
} while(0)

#define MTL_QRCR7_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR7_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR7_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xebc))

#define MTL_QRCR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR6_RgOffAddr);\
} while(0)

#define MTL_QRCR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR6_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR6_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR6_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR6_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR6_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR6_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR6_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR6_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR6_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR6_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR6_RgRd(v);\
		v = (v & (MTL_QRCR6_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR6_Mask_4))<<4);\
		v = ((v & MTL_QRCR6_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR6_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR6_RgWr(v);\
} while(0)

#define MTL_QRCR6_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR6_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR6_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR6_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR6_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR6_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR6_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR6_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR6_RgRd(v);\
		v = (v & (MTL_QRCR6_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR6_Mask_4))<<4);\
		v = ((v & MTL_QRCR6_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR6_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR6_RgWr(v);\
} while(0)

#define MTL_QRCR6_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR6_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR6_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe7c))

#define MTL_QRCR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR5_RgOffAddr);\
} while(0)

#define MTL_QRCR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR5_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR5_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR5_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR5_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR5_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR5_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR5_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR5_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR5_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR5_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR5_RgRd(v);\
		v = (v & (MTL_QRCR5_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR5_Mask_4))<<4);\
		v = ((v & MTL_QRCR5_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR5_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR5_RgWr(v);\
} while(0)

#define MTL_QRCR5_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR5_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR5_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR5_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR5_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR5_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR5_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR5_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR5_RgRd(v);\
		v = (v & (MTL_QRCR5_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR5_Mask_4))<<4);\
		v = ((v & MTL_QRCR5_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR5_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR5_RgWr(v);\
} while(0)

#define MTL_QRCR5_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR5_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR5_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe3c))

#define MTL_QRCR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR4_RgOffAddr);\
} while(0)

#define MTL_QRCR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR4_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR4_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR4_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR4_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR4_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR4_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR4_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR4_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR4_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR4_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR4_RgRd(v);\
		v = (v & (MTL_QRCR4_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR4_Mask_4))<<4);\
		v = ((v & MTL_QRCR4_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR4_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR4_RgWr(v);\
} while(0)

#define MTL_QRCR4_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR4_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR4_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR4_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR4_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR4_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR4_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR4_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR4_RgRd(v);\
		v = (v & (MTL_QRCR4_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR4_Mask_4))<<4);\
		v = ((v & MTL_QRCR4_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR4_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR4_RgWr(v);\
} while(0)

#define MTL_QRCR4_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR4_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR4_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdfc))

#define MTL_QRCR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR3_RgOffAddr);\
} while(0)

#define MTL_QRCR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR3_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR3_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR3_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR3_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR3_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR3_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR3_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR3_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR3_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR3_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR3_RgRd(v);\
		v = (v & (MTL_QRCR3_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR3_Mask_4))<<4);\
		v = ((v & MTL_QRCR3_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR3_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR3_RgWr(v);\
} while(0)

#define MTL_QRCR3_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR3_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR3_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR3_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR3_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR3_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR3_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR3_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR3_RgRd(v);\
		v = (v & (MTL_QRCR3_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR3_Mask_4))<<4);\
		v = ((v & MTL_QRCR3_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR3_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR3_RgWr(v);\
} while(0)

#define MTL_QRCR3_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR3_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR3_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdbc))

#define MTL_QRCR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR2_RgOffAddr);\
} while(0)

#define MTL_QRCR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR2_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR2_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR2_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR2_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR2_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR2_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR2_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR2_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR2_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR2_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR2_RgRd(v);\
		v = (v & (MTL_QRCR2_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR2_Mask_4))<<4);\
		v = ((v & MTL_QRCR2_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR2_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR2_RgWr(v);\
} while(0)

#define MTL_QRCR2_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR2_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR2_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR2_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR2_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR2_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR2_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR2_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR2_RgRd(v);\
		v = (v & (MTL_QRCR2_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR2_Mask_4))<<4);\
		v = ((v & MTL_QRCR2_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR2_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR2_RgWr(v);\
} while(0)

#define MTL_QRCR2_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR2_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR2_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd7c))

#define MTL_QRCR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QRCR1_RgOffAddr);\
} while(0)

#define MTL_QRCR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRCR1_RgOffAddr);\
} while(0)

/*#define  MTL_QRCR1_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR1_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR1_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR1_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR1_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR1_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR1_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR1_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR1_RXQ_WEGT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR1_RgRd(v);\
		v = (v & (MTL_QRCR1_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR1_Mask_4))<<4);\
		v = ((v & MTL_QRCR1_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR1_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR1_RgWr(v);\
} while(0)

#define MTL_QRCR1_RXQ_WEGT_UdfRd(data) do {\
		MTL_QRCR1_RgRd(data);\
		data = ((data >> 0) & MTL_QRCR1_RXQ_WEGT_Mask);\
} while(0)

/*#define MTL_QRCR1_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR1_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR1_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR1_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR1_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_QRCR1_RgRd(v);\
		v = (v & (MTL_QRCR1_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR1_Mask_4))<<4);\
		v = ((v & MTL_QRCR1_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR1_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR1_RgWr(v);\
} while(0)

#define MTL_QRCR1_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_QRCR1_RgRd(data);\
		data = ((data >> 3) & MTL_QRCR1_RXQ_PKT_ARBIT_Mask);\
} while(0)


#define MTL_QRDR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xef8))

#define MTL_QRDR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR7_RgOffAddr);\
} while(0)

/*#define MTL_QRDR7_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR7_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR7_RWCSTS_UdfRd(data) do {\
		MTL_QRDR7_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR7_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR7_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR7_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR7_RRCSTS_UdfRd(data) do {\
		MTL_QRDR7_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR7_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR7_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR7_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR7_RXQSTS_UdfRd(data) do {\
		MTL_QRDR7_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR7_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR7_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR7_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR7_PRXQ_UdfRd(data) do {\
		MTL_QRDR7_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR7_PRXQ_Mask);\
} while(0)


#define MTL_QRDR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xeb8))

#define MTL_QRDR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR6_RgOffAddr);\
} while(0)

/*#define MTL_QRDR6_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR6_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR6_RWCSTS_UdfRd(data) do {\
		MTL_QRDR6_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR6_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR6_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR6_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR6_RRCSTS_UdfRd(data) do {\
		MTL_QRDR6_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR6_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR6_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR6_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR6_RXQSTS_UdfRd(data) do {\
		MTL_QRDR6_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR6_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR6_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR6_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR6_PRXQ_UdfRd(data) do {\
		MTL_QRDR6_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR6_PRXQ_Mask);\
} while(0)


#define MTL_QRDR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe78))

#define MTL_QRDR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR5_RgOffAddr);\
} while(0)

/*#define MTL_QRDR5_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR5_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR5_RWCSTS_UdfRd(data) do {\
		MTL_QRDR5_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR5_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR5_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR5_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR5_RRCSTS_UdfRd(data) do {\
		MTL_QRDR5_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR5_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR5_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR5_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR5_RXQSTS_UdfRd(data) do {\
		MTL_QRDR5_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR5_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR5_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR5_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR5_PRXQ_UdfRd(data) do {\
		MTL_QRDR5_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR5_PRXQ_Mask);\
} while(0)


#define MTL_QRDR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe38))

#define MTL_QRDR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR4_RgOffAddr);\
} while(0)

/*#define MTL_QRDR4_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR4_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR4_RWCSTS_UdfRd(data) do {\
		MTL_QRDR4_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR4_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR4_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR4_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR4_RRCSTS_UdfRd(data) do {\
		MTL_QRDR4_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR4_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR4_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR4_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR4_RXQSTS_UdfRd(data) do {\
		MTL_QRDR4_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR4_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR4_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR4_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR4_PRXQ_UdfRd(data) do {\
		MTL_QRDR4_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR4_PRXQ_Mask);\
} while(0)


#define MTL_QRDR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdf8))

#define MTL_QRDR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR3_RgOffAddr);\
} while(0)

/*#define MTL_QRDR3_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR3_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR3_RWCSTS_UdfRd(data) do {\
		MTL_QRDR3_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR3_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR3_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR3_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR3_RRCSTS_UdfRd(data) do {\
		MTL_QRDR3_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR3_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR3_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR3_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR3_RXQSTS_UdfRd(data) do {\
		MTL_QRDR3_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR3_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR3_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR3_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR3_PRXQ_UdfRd(data) do {\
		MTL_QRDR3_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR3_PRXQ_Mask);\
} while(0)


#define MTL_QRDR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdb8))

#define MTL_QRDR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR2_RgOffAddr);\
} while(0)

/*#define MTL_QRDR2_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR2_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR2_RWCSTS_UdfRd(data) do {\
		MTL_QRDR2_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR2_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR2_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR2_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR2_RRCSTS_UdfRd(data) do {\
		MTL_QRDR2_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR2_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR2_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR2_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR2_RXQSTS_UdfRd(data) do {\
		MTL_QRDR2_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR2_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR2_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR2_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR2_PRXQ_UdfRd(data) do {\
		MTL_QRDR2_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR2_PRXQ_Mask);\
} while(0)


#define MTL_QRDR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd78))

#define MTL_QRDR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QRDR1_RgOffAddr);\
} while(0)

/*#define MTL_QRDR1_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR1_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR1_RWCSTS_UdfRd(data) do {\
		MTL_QRDR1_RgRd(data);\
		data = ((data >> 0) & MTL_QRDR1_RWCSTS_Mask);\
} while(0)

/*#define MTL_QRDR1_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR1_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR1_RRCSTS_UdfRd(data) do {\
		MTL_QRDR1_RgRd(data);\
		data = ((data >> 1) & MTL_QRDR1_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR1_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR1_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR1_RXQSTS_UdfRd(data) do {\
		MTL_QRDR1_RgRd(data);\
		data = ((data >> 4) & MTL_QRDR1_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR1_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR1_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR1_PRXQ_UdfRd(data) do {\
		MTL_QRDR1_RgRd(data);\
		data = ((data >> 16) & MTL_QRDR1_PRXQ_Mask);\
} while(0)


#define MTL_QOCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xef4))

#define MTL_QOCR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR7_RgOffAddr);\
} while(0)

#define MTL_QOCR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR7_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR7_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR7_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR7_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR7_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR7_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR7_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR7_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR7_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR7_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR7_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR7_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR7_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR7_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR7_RgRd(v);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR7_Mask_12))<<12);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR7_Mask_28))<<28);\
		v = ((v & MTL_QOCR7_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR7_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR7_RgWr(v);\
} while(0)

#define MTL_QOCR7_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR7_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR7_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR7_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR7_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR7_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR7_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR7_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR7_RgRd(v);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR7_Mask_12))<<12);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR7_Mask_28))<<28);\
		v = ((v & MTL_QOCR7_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR7_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR7_RgWr(v);\
} while(0)

#define MTL_QOCR7_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR7_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR7_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR7_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR7_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR7_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR7_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR7_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR7_RgRd(v);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR7_Mask_12))<<12);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR7_Mask_28))<<28);\
		v = ((v & MTL_QOCR7_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR7_MISPKTCNT_Mask)<<16));\
		MTL_QOCR7_RgWr(v);\
} while(0)

#define MTL_QOCR7_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR7_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR7_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR7_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR7_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR7_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR7_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR7_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR7_RgRd(v);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR7_Mask_12))<<12);\
		v = (v & (MTL_QOCR7_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR7_Mask_28))<<28);\
		v = ((v & MTL_QOCR7_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR7_MISCNTOVF_Mask)<<27));\
		MTL_QOCR7_RgWr(v);\
} while(0)

#define MTL_QOCR7_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR7_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR7_MISCNTOVF_Mask);\
} while(0)


#define MTL_QOCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xeb4))

#define MTL_QOCR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR6_RgOffAddr);\
} while(0)

#define MTL_QOCR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR6_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR6_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR6_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR6_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR6_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR6_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR6_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR6_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR6_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR6_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR6_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR6_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR6_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR6_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR6_RgRd(v);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR6_Mask_12))<<12);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR6_Mask_28))<<28);\
		v = ((v & MTL_QOCR6_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR6_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR6_RgWr(v);\
} while(0)

#define MTL_QOCR6_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR6_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR6_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR6_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR6_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR6_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR6_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR6_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR6_RgRd(v);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR6_Mask_12))<<12);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR6_Mask_28))<<28);\
		v = ((v & MTL_QOCR6_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR6_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR6_RgWr(v);\
} while(0)

#define MTL_QOCR6_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR6_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR6_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR6_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR6_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR6_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR6_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR6_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR6_RgRd(v);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR6_Mask_12))<<12);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR6_Mask_28))<<28);\
		v = ((v & MTL_QOCR6_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR6_MISPKTCNT_Mask)<<16));\
		MTL_QOCR6_RgWr(v);\
} while(0)

#define MTL_QOCR6_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR6_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR6_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR6_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR6_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR6_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR6_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR6_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR6_RgRd(v);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR6_Mask_12))<<12);\
		v = (v & (MTL_QOCR6_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR6_Mask_28))<<28);\
		v = ((v & MTL_QOCR6_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR6_MISCNTOVF_Mask)<<27));\
		MTL_QOCR6_RgWr(v);\
} while(0)

#define MTL_QOCR6_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR6_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR6_MISCNTOVF_Mask);\
} while(0)


#define MTL_QOCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe74))

#define MTL_QOCR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR5_RgOffAddr);\
} while(0)

#define MTL_QOCR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR5_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR5_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR5_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR5_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR5_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR5_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR5_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR5_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR5_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR5_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR5_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR5_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR5_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR5_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR5_RgRd(v);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR5_Mask_12))<<12);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR5_Mask_28))<<28);\
		v = ((v & MTL_QOCR5_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR5_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR5_RgWr(v);\
} while(0)

#define MTL_QOCR5_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR5_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR5_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR5_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR5_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR5_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR5_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR5_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR5_RgRd(v);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR5_Mask_12))<<12);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR5_Mask_28))<<28);\
		v = ((v & MTL_QOCR5_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR5_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR5_RgWr(v);\
} while(0)

#define MTL_QOCR5_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR5_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR5_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR5_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR5_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR5_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR5_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR5_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR5_RgRd(v);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR5_Mask_12))<<12);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR5_Mask_28))<<28);\
		v = ((v & MTL_QOCR5_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR5_MISPKTCNT_Mask)<<16));\
		MTL_QOCR5_RgWr(v);\
} while(0)

#define MTL_QOCR5_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR5_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR5_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR5_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR5_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR5_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR5_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR5_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR5_RgRd(v);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR5_Mask_12))<<12);\
		v = (v & (MTL_QOCR5_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR5_Mask_28))<<28);\
		v = ((v & MTL_QOCR5_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR5_MISCNTOVF_Mask)<<27));\
		MTL_QOCR5_RgWr(v);\
} while(0)

#define MTL_QOCR5_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR5_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR5_MISCNTOVF_Mask);\
} while(0)


#define MTL_QOCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe34))

#define MTL_QOCR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR4_RgOffAddr);\
} while(0)

#define MTL_QOCR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR4_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR4_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR4_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR4_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR4_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR4_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR4_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR4_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR4_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR4_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR4_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR4_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR4_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR4_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR4_RgRd(v);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR4_Mask_12))<<12);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR4_Mask_28))<<28);\
		v = ((v & MTL_QOCR4_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR4_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR4_RgWr(v);\
} while(0)

#define MTL_QOCR4_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR4_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR4_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR4_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR4_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR4_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR4_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR4_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR4_RgRd(v);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR4_Mask_12))<<12);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR4_Mask_28))<<28);\
		v = ((v & MTL_QOCR4_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR4_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR4_RgWr(v);\
} while(0)

#define MTL_QOCR4_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR4_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR4_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR4_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR4_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR4_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR4_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR4_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR4_RgRd(v);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR4_Mask_12))<<12);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR4_Mask_28))<<28);\
		v = ((v & MTL_QOCR4_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR4_MISPKTCNT_Mask)<<16));\
		MTL_QOCR4_RgWr(v);\
} while(0)

#define MTL_QOCR4_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR4_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR4_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR4_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR4_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR4_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR4_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR4_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR4_RgRd(v);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR4_Mask_12))<<12);\
		v = (v & (MTL_QOCR4_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR4_Mask_28))<<28);\
		v = ((v & MTL_QOCR4_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR4_MISCNTOVF_Mask)<<27));\
		MTL_QOCR4_RgWr(v);\
} while(0)

#define MTL_QOCR4_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR4_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR4_MISCNTOVF_Mask);\
} while(0)


#define MTL_QOCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdf4))

#define MTL_QOCR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR3_RgOffAddr);\
} while(0)

#define MTL_QOCR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR3_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR3_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR3_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR3_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR3_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR3_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR3_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR3_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR3_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR3_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR3_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR3_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR3_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR3_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR3_RgRd(v);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR3_Mask_12))<<12);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR3_Mask_28))<<28);\
		v = ((v & MTL_QOCR3_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR3_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR3_RgWr(v);\
} while(0)

#define MTL_QOCR3_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR3_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR3_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR3_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR3_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR3_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR3_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR3_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR3_RgRd(v);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR3_Mask_12))<<12);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR3_Mask_28))<<28);\
		v = ((v & MTL_QOCR3_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR3_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR3_RgWr(v);\
} while(0)

#define MTL_QOCR3_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR3_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR3_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR3_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR3_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR3_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR3_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR3_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR3_RgRd(v);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR3_Mask_12))<<12);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR3_Mask_28))<<28);\
		v = ((v & MTL_QOCR3_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR3_MISPKTCNT_Mask)<<16));\
		MTL_QOCR3_RgWr(v);\
} while(0)

#define MTL_QOCR3_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR3_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR3_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR3_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR3_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR3_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR3_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR3_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR3_RgRd(v);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR3_Mask_12))<<12);\
		v = (v & (MTL_QOCR3_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR3_Mask_28))<<28);\
		v = ((v & MTL_QOCR3_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR3_MISCNTOVF_Mask)<<27));\
		MTL_QOCR3_RgWr(v);\
} while(0)

#define MTL_QOCR3_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR3_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR3_MISCNTOVF_Mask);\
} while(0)


#define MTL_QOCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdb4))

#define MTL_QOCR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR2_RgOffAddr);\
} while(0)

#define MTL_QOCR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR2_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR2_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR2_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR2_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR2_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR2_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR2_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR2_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR2_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR2_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR2_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR2_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR2_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR2_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR2_RgRd(v);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR2_Mask_12))<<12);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR2_Mask_28))<<28);\
		v = ((v & MTL_QOCR2_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR2_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR2_RgWr(v);\
} while(0)

#define MTL_QOCR2_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR2_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR2_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR2_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR2_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR2_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR2_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR2_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR2_RgRd(v);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR2_Mask_12))<<12);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR2_Mask_28))<<28);\
		v = ((v & MTL_QOCR2_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR2_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR2_RgWr(v);\
} while(0)

#define MTL_QOCR2_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR2_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR2_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR2_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR2_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR2_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR2_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR2_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR2_RgRd(v);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR2_Mask_12))<<12);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR2_Mask_28))<<28);\
		v = ((v & MTL_QOCR2_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR2_MISPKTCNT_Mask)<<16));\
		MTL_QOCR2_RgWr(v);\
} while(0)

#define MTL_QOCR2_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR2_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR2_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR2_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR2_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR2_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR2_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR2_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR2_RgRd(v);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR2_Mask_12))<<12);\
		v = (v & (MTL_QOCR2_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR2_Mask_28))<<28);\
		v = ((v & MTL_QOCR2_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR2_MISCNTOVF_Mask)<<27));\
		MTL_QOCR2_RgWr(v);\
} while(0)

#define MTL_QOCR2_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR2_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR2_MISCNTOVF_Mask);\
} while(0)


#define MTL_QOCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd74))

#define MTL_QOCR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QOCR1_RgOffAddr);\
} while(0)

#define MTL_QOCR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QOCR1_RgOffAddr);\
} while(0)

/*#define  MTL_QOCR1_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR1_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR1_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR1_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MTL_QOCR1_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR1_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR1_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR1_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define MTL_QOCR1_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR1_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR1_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR1_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR1_OVFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR1_RgRd(v);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR1_Mask_12))<<12);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR1_Mask_28))<<28);\
		v = ((v & MTL_QOCR1_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR1_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR1_RgWr(v);\
} while(0)

#define MTL_QOCR1_OVFPKTCNT_UdfRd(data) do {\
		MTL_QOCR1_RgRd(data);\
		data = ((data >> 0) & MTL_QOCR1_OVFPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR1_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR1_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR1_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR1_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR1_OVFCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR1_RgRd(v);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR1_Mask_12))<<12);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR1_Mask_28))<<28);\
		v = ((v & MTL_QOCR1_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR1_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR1_RgWr(v);\
} while(0)

#define MTL_QOCR1_OVFCNTOVF_UdfRd(data) do {\
		MTL_QOCR1_RgRd(data);\
		data = ((data >> 11) & MTL_QOCR1_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR1_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR1_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR1_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR1_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR1_MISPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR1_RgRd(v);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR1_Mask_12))<<12);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR1_Mask_28))<<28);\
		v = ((v & MTL_QOCR1_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR1_MISPKTCNT_Mask)<<16));\
		MTL_QOCR1_RgWr(v);\
} while(0)

#define MTL_QOCR1_MISPKTCNT_UdfRd(data) do {\
		MTL_QOCR1_RgRd(data);\
		data = ((data >> 16) & MTL_QOCR1_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR1_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR1_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR1_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR1_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR1_MISCNTOVF_UdfWr(data) do {\
		ULONG v;\
		MTL_QOCR1_RgRd(v);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR1_Mask_12))<<12);\
		v = (v & (MTL_QOCR1_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR1_Mask_28))<<28);\
		v = ((v & MTL_QOCR1_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR1_MISCNTOVF_Mask)<<27));\
		MTL_QOCR1_RgWr(v);\
} while(0)

#define MTL_QOCR1_MISCNTOVF_UdfRd(data) do {\
		MTL_QOCR1_RgRd(data);\
		data = ((data >> 27) & MTL_QOCR1_MISCNTOVF_Mask);\
} while(0)

#define MTL_QLCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xee4))

#define MTL_QLCR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR7_RgOffAddr);\
} while(0)

#define MTL_QLCR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR7_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR7_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR7_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR7_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR7_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR7_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR7_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR7_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR7_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR7_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR7_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR7_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR7_LC_Wr_Mask) | ((data & MTL_QLCR7_LC_Mask)<<0));\
		MTL_QLCR7_RgWr(v);\
} while(0)

#define MTL_QLCR7_LC_UdfRd(data) do {\
		MTL_QLCR7_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR7_LC_Mask);\
} while(0)


#define MTL_QLCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xea4))

#define MTL_QLCR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR6_RgOffAddr);\
} while(0)

#define MTL_QLCR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR6_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR6_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR6_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR6_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR6_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR6_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR6_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR6_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR6_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR6_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR6_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR6_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR6_LC_Wr_Mask) | ((data & MTL_QLCR6_LC_Mask)<<0));\
		MTL_QLCR6_RgWr(v);\
} while(0)

#define MTL_QLCR6_LC_UdfRd(data) do {\
		MTL_QLCR6_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR6_LC_Mask);\
} while(0)


#define MTL_QLCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe64))

#define MTL_QLCR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR5_RgOffAddr);\
} while(0)

#define MTL_QLCR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR5_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR5_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR5_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR5_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR5_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR5_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR5_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR5_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR5_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR5_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR5_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR5_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR5_LC_Wr_Mask) | ((data & MTL_QLCR5_LC_Mask)<<0));\
		MTL_QLCR5_RgWr(v);\
} while(0)

#define MTL_QLCR5_LC_UdfRd(data) do {\
		MTL_QLCR5_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR5_LC_Mask);\
} while(0)


#define MTL_QLCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe24))

#define MTL_QLCR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR4_RgOffAddr);\
} while(0)

#define MTL_QLCR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR4_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR4_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR4_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR4_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR4_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR4_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR4_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR4_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR4_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR4_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR4_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR4_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR4_LC_Wr_Mask) | ((data & MTL_QLCR4_LC_Mask)<<0));\
		MTL_QLCR4_RgWr(v);\
} while(0)

#define MTL_QLCR4_LC_UdfRd(data) do {\
		MTL_QLCR4_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR4_LC_Mask);\
} while(0)


#define MTL_QLCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xde4))

#define MTL_QLCR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR3_RgOffAddr);\
} while(0)

#define MTL_QLCR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR3_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR3_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR3_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR3_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR3_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR3_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR3_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR3_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR3_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR3_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR3_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR3_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR3_LC_Wr_Mask) | ((data & MTL_QLCR3_LC_Mask)<<0));\
		MTL_QLCR3_RgWr(v);\
} while(0)

#define MTL_QLCR3_LC_UdfRd(data) do {\
		MTL_QLCR3_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR3_LC_Mask);\
} while(0)


#define MTL_QLCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xda4))

#define MTL_QLCR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR2_RgOffAddr);\
} while(0)

#define MTL_QLCR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR2_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR2_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR2_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR2_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR2_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR2_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR2_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR2_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR2_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR2_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR2_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR2_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR2_LC_Wr_Mask) | ((data & MTL_QLCR2_LC_Mask)<<0));\
		MTL_QLCR2_RgWr(v);\
} while(0)

#define MTL_QLCR2_LC_UdfRd(data) do {\
		MTL_QLCR2_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR2_LC_Mask);\
} while(0)


#define MTL_QLCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd64))

#define MTL_QLCR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QLCR1_RgOffAddr);\
} while(0)

#define MTL_QLCR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QLCR1_RgOffAddr);\
} while(0)

/*#define  MTL_QLCR1_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR1_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR1_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR1_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR1_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR1_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR1_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR1_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR1_LC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR1_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR1_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR1_LC_Wr_Mask) | ((data & MTL_QLCR1_LC_Mask)<<0));\
		MTL_QLCR1_RgWr(v);\
} while(0)

#define MTL_QLCR1_LC_UdfRd(data) do {\
		MTL_QLCR1_RgRd(data);\
		data = ((data >> 0) & MTL_QLCR1_LC_Mask);\
} while(0)


#define MTL_QHCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xee0))

#define MTL_QHCR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR7_RgOffAddr);\
} while(0)

#define MTL_QHCR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR7_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR7_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR7_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR7_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR7_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR7_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR7_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR7_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR7_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR7_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR7_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR7_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR7_HC_Wr_Mask) | ((data & MTL_QHCR7_HC_Mask)<<0));\
		MTL_QHCR7_RgWr(v);\
} while(0)

#define MTL_QHCR7_HC_UdfRd(data) do {\
		MTL_QHCR7_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR7_HC_Mask);\
} while(0)


#define MTL_QHCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xea0))

#define MTL_QHCR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR6_RgOffAddr);\
} while(0)

#define MTL_QHCR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR6_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR6_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR6_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR6_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR6_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR6_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR6_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR6_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR6_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR6_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR6_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR6_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR6_HC_Wr_Mask) | ((data & MTL_QHCR6_HC_Mask)<<0));\
		MTL_QHCR6_RgWr(v);\
} while(0)

#define MTL_QHCR6_HC_UdfRd(data) do {\
		MTL_QHCR6_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR6_HC_Mask);\
} while(0)


#define MTL_QHCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe60))

#define MTL_QHCR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR5_RgOffAddr);\
} while(0)

#define MTL_QHCR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR5_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR5_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR5_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR5_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR5_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR5_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR5_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR5_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR5_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR5_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR5_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR5_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR5_HC_Wr_Mask) | ((data & MTL_QHCR5_HC_Mask)<<0));\
		MTL_QHCR5_RgWr(v);\
} while(0)

#define MTL_QHCR5_HC_UdfRd(data) do {\
		MTL_QHCR5_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR5_HC_Mask);\
} while(0)


#define MTL_QHCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe20))

#define MTL_QHCR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR4_RgOffAddr);\
} while(0)

#define MTL_QHCR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR4_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR4_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR4_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR4_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR4_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR4_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR4_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR4_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR4_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR4_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR4_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR4_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR4_HC_Wr_Mask) | ((data & MTL_QHCR4_HC_Mask)<<0));\
		MTL_QHCR4_RgWr(v);\
} while(0)

#define MTL_QHCR4_HC_UdfRd(data) do {\
		MTL_QHCR4_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR4_HC_Mask);\
} while(0)


#define MTL_QHCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xde0))

#define MTL_QHCR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR3_RgOffAddr);\
} while(0)

#define MTL_QHCR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR3_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR3_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR3_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR3_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR3_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR3_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR3_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR3_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR3_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR3_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR3_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR3_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR3_HC_Wr_Mask) | ((data & MTL_QHCR3_HC_Mask)<<0));\
		MTL_QHCR3_RgWr(v);\
} while(0)

#define MTL_QHCR3_HC_UdfRd(data) do {\
		MTL_QHCR3_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR3_HC_Mask);\
} while(0)


#define MTL_QHCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xda0))

#define MTL_QHCR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR2_RgOffAddr);\
} while(0)

#define MTL_QHCR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR2_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR2_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR2_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR2_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR2_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR2_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR2_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR2_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR2_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR2_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR2_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR2_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR2_HC_Wr_Mask) | ((data & MTL_QHCR2_HC_Mask)<<0));\
		MTL_QHCR2_RgWr(v);\
} while(0)

#define MTL_QHCR2_HC_UdfRd(data) do {\
		MTL_QHCR2_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR2_HC_Mask);\
} while(0)


#define MTL_QHCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd60))

#define MTL_QHCR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QHCR1_RgOffAddr);\
} while(0)

#define MTL_QHCR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QHCR1_RgOffAddr);\
} while(0)

/*#define  MTL_QHCR1_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR1_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR1_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR1_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR1_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR1_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR1_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR1_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR1_HC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR1_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR1_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR1_HC_Wr_Mask) | ((data & MTL_QHCR1_HC_Mask)<<0));\
		MTL_QHCR1_RgWr(v);\
} while(0)

#define MTL_QHCR1_HC_UdfRd(data) do {\
		MTL_QHCR1_RgRd(data);\
		data = ((data >> 0) & MTL_QHCR1_HC_Mask);\
} while(0)


#define MTL_QSSCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xedc))

#define MTL_QSSCR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR7_RgOffAddr);\
} while(0)

#define MTL_QSSCR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR7_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR7_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR7_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR7_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR7_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR7_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR7_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR7_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR7_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR7_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR7_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR7_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR7_SSC_Wr_Mask) | ((data & MTL_QSSCR7_SSC_Mask)<<0));\
		MTL_QSSCR7_RgWr(v);\
} while(0)

#define MTL_QSSCR7_SSC_UdfRd(data) do {\
		MTL_QSSCR7_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR7_SSC_Mask);\
} while(0)


#define MTL_QSSCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe9c))

#define MTL_QSSCR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR6_RgOffAddr);\
} while(0)

#define MTL_QSSCR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR6_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR6_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR6_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR6_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR6_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR6_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR6_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR6_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR6_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR6_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR6_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR6_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR6_SSC_Wr_Mask) | ((data & MTL_QSSCR6_SSC_Mask)<<0));\
		MTL_QSSCR6_RgWr(v);\
} while(0)

#define MTL_QSSCR6_SSC_UdfRd(data) do {\
		MTL_QSSCR6_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR6_SSC_Mask);\
} while(0)


#define MTL_QSSCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe5c))

#define MTL_QSSCR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR5_RgOffAddr);\
} while(0)

#define MTL_QSSCR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR5_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR5_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR5_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR5_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR5_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR5_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR5_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR5_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR5_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR5_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR5_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR5_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR5_SSC_Wr_Mask) | ((data & MTL_QSSCR5_SSC_Mask)<<0));\
		MTL_QSSCR5_RgWr(v);\
} while(0)

#define MTL_QSSCR5_SSC_UdfRd(data) do {\
		MTL_QSSCR5_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR5_SSC_Mask);\
} while(0)


#define MTL_QSSCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe1c))

#define MTL_QSSCR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR4_RgOffAddr);\
} while(0)

#define MTL_QSSCR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR4_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR4_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR4_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR4_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR4_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR4_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR4_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR4_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR4_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR4_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR4_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR4_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR4_SSC_Wr_Mask) | ((data & MTL_QSSCR4_SSC_Mask)<<0));\
		MTL_QSSCR4_RgWr(v);\
} while(0)

#define MTL_QSSCR4_SSC_UdfRd(data) do {\
		MTL_QSSCR4_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR4_SSC_Mask);\
} while(0)


#define MTL_QSSCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xddc))

#define MTL_QSSCR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR3_RgOffAddr);\
} while(0)

#define MTL_QSSCR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR3_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR3_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR3_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR3_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR3_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR3_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR3_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR3_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR3_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR3_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR3_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR3_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR3_SSC_Wr_Mask) | ((data & MTL_QSSCR3_SSC_Mask)<<0));\
		MTL_QSSCR3_RgWr(v);\
} while(0)

#define MTL_QSSCR3_SSC_UdfRd(data) do {\
		MTL_QSSCR3_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR3_SSC_Mask);\
} while(0)


#define MTL_QSSCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd9c))

#define MTL_QSSCR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR2_RgOffAddr);\
} while(0)

#define MTL_QSSCR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR2_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR2_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR2_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR2_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR2_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR2_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR2_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR2_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR2_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR2_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR2_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR2_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR2_SSC_Wr_Mask) | ((data & MTL_QSSCR2_SSC_Mask)<<0));\
		MTL_QSSCR2_RgWr(v);\
} while(0)

#define MTL_QSSCR2_SSC_UdfRd(data) do {\
		MTL_QSSCR2_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR2_SSC_Mask);\
} while(0)


#define MTL_QSSCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd5c))

#define MTL_QSSCR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QSSCR1_RgOffAddr);\
} while(0)

#define MTL_QSSCR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QSSCR1_RgOffAddr);\
} while(0)

/*#define  MTL_QSSCR1_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR1_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR1_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR1_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR1_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR1_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR1_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR1_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR1_SSC_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR1_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR1_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR1_SSC_Wr_Mask) | ((data & MTL_QSSCR1_SSC_Mask)<<0));\
		MTL_QSSCR1_RgWr(v);\
} while(0)

#define MTL_QSSCR1_SSC_UdfRd(data) do {\
		MTL_QSSCR1_RgRd(data);\
		data = ((data >> 0) & MTL_QSSCR1_SSC_Mask);\
} while(0)


#define MTL_QW7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xed8))

#define MTL_QW7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW7_RgOffAddr);\
} while(0)

#define MTL_QW7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW7_RgOffAddr);\
} while(0)

/*#define  MTL_QW7_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW7_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW7_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW7_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW7_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW7_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW7_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW7_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW7_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW7_RES_Wr_Mask_21))|((( 0) & (MTL_QW7_Mask_21))<<21);\
		(v) = ((v & MTL_QW7_ISCQW_Wr_Mask) | ((data & MTL_QW7_ISCQW_Mask)<<0));\
		MTL_QW7_RgWr(v);\
} while(0)

#define MTL_QW7_ISCQW_UdfRd(data) do {\
		MTL_QW7_RgRd(data);\
		data = ((data >> 0) & MTL_QW7_ISCQW_Mask);\
} while(0)


#define MTL_QW6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe98))

#define MTL_QW6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW6_RgOffAddr);\
} while(0)

#define MTL_QW6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW6_RgOffAddr);\
} while(0)

/*#define  MTL_QW6_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW6_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW6_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW6_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW6_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW6_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW6_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW6_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW6_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW6_RES_Wr_Mask_21))|((( 0) & (MTL_QW6_Mask_21))<<21);\
		(v) = ((v & MTL_QW6_ISCQW_Wr_Mask) | ((data & MTL_QW6_ISCQW_Mask)<<0));\
		MTL_QW6_RgWr(v);\
} while(0)

#define MTL_QW6_ISCQW_UdfRd(data) do {\
		MTL_QW6_RgRd(data);\
		data = ((data >> 0) & MTL_QW6_ISCQW_Mask);\
} while(0)


#define MTL_QW5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe58))

#define MTL_QW5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW5_RgOffAddr);\
} while(0)

#define MTL_QW5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW5_RgOffAddr);\
} while(0)

/*#define  MTL_QW5_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW5_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW5_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW5_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW5_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW5_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW5_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW5_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW5_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW5_RES_Wr_Mask_21))|((( 0) & (MTL_QW5_Mask_21))<<21);\
		(v) = ((v & MTL_QW5_ISCQW_Wr_Mask) | ((data & MTL_QW5_ISCQW_Mask)<<0));\
		MTL_QW5_RgWr(v);\
} while(0)

#define MTL_QW5_ISCQW_UdfRd(data) do {\
		MTL_QW5_RgRd(data);\
		data = ((data >> 0) & MTL_QW5_ISCQW_Mask);\
} while(0)


#define MTL_QW4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe18))

#define MTL_QW4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW4_RgOffAddr);\
} while(0)

#define MTL_QW4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW4_RgOffAddr);\
} while(0)

/*#define  MTL_QW4_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW4_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW4_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW4_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW4_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW4_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW4_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW4_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW4_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW4_RES_Wr_Mask_21))|((( 0) & (MTL_QW4_Mask_21))<<21);\
		(v) = ((v & MTL_QW4_ISCQW_Wr_Mask) | ((data & MTL_QW4_ISCQW_Mask)<<0));\
		MTL_QW4_RgWr(v);\
} while(0)

#define MTL_QW4_ISCQW_UdfRd(data) do {\
		MTL_QW4_RgRd(data);\
		data = ((data >> 0) & MTL_QW4_ISCQW_Mask);\
} while(0)


#define MTL_QW3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdd8))

#define MTL_QW3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW3_RgOffAddr);\
} while(0)

#define MTL_QW3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW3_RgOffAddr);\
} while(0)

/*#define  MTL_QW3_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW3_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW3_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW3_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW3_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW3_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW3_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW3_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW3_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW3_RES_Wr_Mask_21))|((( 0) & (MTL_QW3_Mask_21))<<21);\
		(v) = ((v & MTL_QW3_ISCQW_Wr_Mask) | ((data & MTL_QW3_ISCQW_Mask)<<0));\
		MTL_QW3_RgWr(v);\
} while(0)

#define MTL_QW3_ISCQW_UdfRd(data) do {\
		MTL_QW3_RgRd(data);\
		data = ((data >> 0) & MTL_QW3_ISCQW_Mask);\
} while(0)


#define MTL_QW2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd98))

#define MTL_QW2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW2_RgOffAddr);\
} while(0)

#define MTL_QW2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW2_RgOffAddr);\
} while(0)

/*#define  MTL_QW2_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW2_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW2_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW2_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW2_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW2_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW2_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW2_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW2_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW2_RES_Wr_Mask_21))|((( 0) & (MTL_QW2_Mask_21))<<21);\
		(v) = ((v & MTL_QW2_ISCQW_Wr_Mask) | ((data & MTL_QW2_ISCQW_Mask)<<0));\
		MTL_QW2_RgWr(v);\
} while(0)

#define MTL_QW2_ISCQW_UdfRd(data) do {\
		MTL_QW2_RgRd(data);\
		data = ((data >> 0) & MTL_QW2_ISCQW_Mask);\
} while(0)


#define MTL_QW1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd58))

#define MTL_QW1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QW1_RgOffAddr);\
} while(0)

#define MTL_QW1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QW1_RgOffAddr);\
} while(0)

/*#define  MTL_QW1_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW1_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW1_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW1_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW1_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW1_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW1_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW1_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW1_ISCQW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW1_RES_Wr_Mask_21))|((( 0) & (MTL_QW1_Mask_21))<<21);\
		(v) = ((v & MTL_QW1_ISCQW_Wr_Mask) | ((data & MTL_QW1_ISCQW_Mask)<<0));\
		MTL_QW1_RgWr(v);\
} while(0)

#define MTL_QW1_ISCQW_UdfRd(data) do {\
		MTL_QW1_RgRd(data);\
		data = ((data >> 0) & MTL_QW1_ISCQW_Mask);\
} while(0)


#define MTL_QESR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xed4))

#define MTL_QESR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR7_RgOffAddr);\
} while(0)

/*#define MTL_QESR7_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR7_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR7_ABS_UdfRd(data) do {\
		MTL_QESR7_RgRd(data);\
		data = ((data >> 0) & MTL_QESR7_ABS_Mask);\
} while(0)

/*#define MTL_QESR7_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR7_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR7_ABSU_UdfRd(data) do {\
		MTL_QESR7_RgRd(data);\
		data = ((data >> 24) & MTL_QESR7_ABSU_Mask);\
} while(0)


#define MTL_QESR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe94))

#define MTL_QESR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR6_RgOffAddr);\
} while(0)

/*#define MTL_QESR6_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR6_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR6_ABS_UdfRd(data) do {\
		MTL_QESR6_RgRd(data);\
		data = ((data >> 0) & MTL_QESR6_ABS_Mask);\
} while(0)

/*#define MTL_QESR6_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR6_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR6_ABSU_UdfRd(data) do {\
		MTL_QESR6_RgRd(data);\
		data = ((data >> 24) & MTL_QESR6_ABSU_Mask);\
} while(0)


#define MTL_QESR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe54))

#define MTL_QESR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR5_RgOffAddr);\
} while(0)

/*#define MTL_QESR5_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR5_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR5_ABS_UdfRd(data) do {\
		MTL_QESR5_RgRd(data);\
		data = ((data >> 0) & MTL_QESR5_ABS_Mask);\
} while(0)

/*#define MTL_QESR5_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR5_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR5_ABSU_UdfRd(data) do {\
		MTL_QESR5_RgRd(data);\
		data = ((data >> 24) & MTL_QESR5_ABSU_Mask);\
} while(0)


#define MTL_QESR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe14))

#define MTL_QESR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR4_RgOffAddr);\
} while(0)

/*#define MTL_QESR4_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR4_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR4_ABS_UdfRd(data) do {\
		MTL_QESR4_RgRd(data);\
		data = ((data >> 0) & MTL_QESR4_ABS_Mask);\
} while(0)

/*#define MTL_QESR4_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR4_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR4_ABSU_UdfRd(data) do {\
		MTL_QESR4_RgRd(data);\
		data = ((data >> 24) & MTL_QESR4_ABSU_Mask);\
} while(0)


#define MTL_QESR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdd4))

#define MTL_QESR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR3_RgOffAddr);\
} while(0)

/*#define MTL_QESR3_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR3_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR3_ABS_UdfRd(data) do {\
		MTL_QESR3_RgRd(data);\
		data = ((data >> 0) & MTL_QESR3_ABS_Mask);\
} while(0)

/*#define MTL_QESR3_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR3_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR3_ABSU_UdfRd(data) do {\
		MTL_QESR3_RgRd(data);\
		data = ((data >> 24) & MTL_QESR3_ABSU_Mask);\
} while(0)


#define MTL_QESR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd94))

#define MTL_QESR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR2_RgOffAddr);\
} while(0)

/*#define MTL_QESR2_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR2_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR2_ABS_UdfRd(data) do {\
		MTL_QESR2_RgRd(data);\
		data = ((data >> 0) & MTL_QESR2_ABS_Mask);\
} while(0)

/*#define MTL_QESR2_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR2_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR2_ABSU_UdfRd(data) do {\
		MTL_QESR2_RgRd(data);\
		data = ((data >> 24) & MTL_QESR2_ABSU_Mask);\
} while(0)


#define MTL_QESR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd54))

#define MTL_QESR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QESR1_RgOffAddr);\
} while(0)

/*#define MTL_QESR1_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR1_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR1_ABS_UdfRd(data) do {\
		MTL_QESR1_RgRd(data);\
		data = ((data >> 0) & MTL_QESR1_ABS_Mask);\
} while(0)

/*#define MTL_QESR1_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR1_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR1_ABSU_UdfRd(data) do {\
		MTL_QESR1_RgRd(data);\
		data = ((data >> 24) & MTL_QESR1_ABSU_Mask);\
} while(0)


#define MTL_QECR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xed0))

#define MTL_QECR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR7_RgOffAddr);\
} while(0)

#define MTL_QECR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR7_RgOffAddr);\
} while(0)

/*#define  MTL_QECR7_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR7_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR7_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR7_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR7_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR7_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR7_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR7_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR7_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR7_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR7_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR7_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR7_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR7_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR7_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR7_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR7_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR7_RgRd(v);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_0))|((( 0) & (MTL_QECR7_Mask_0))<<0);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_7))|((( 0) & (MTL_QECR7_Mask_7))<<7);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_25))|((( 0) & (MTL_QECR7_Mask_25))<<25);\
		v = ((v & MTL_QECR7_AVALG_Wr_Mask) | ((data & MTL_QECR7_AVALG_Mask)<<2));\
		MTL_QECR7_RgWr(v);\
} while(0)

#define MTL_QECR7_AVALG_UdfRd(data) do {\
		MTL_QECR7_RgRd(data);\
		data = ((data >> 2) & MTL_QECR7_AVALG_Mask);\
} while(0)

/*#define MTL_QECR7_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR7_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR7_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR7_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR7_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR7_RgRd(v);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_0))|((( 0) & (MTL_QECR7_Mask_0))<<0);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_7))|((( 0) & (MTL_QECR7_Mask_7))<<7);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_25))|((( 0) & (MTL_QECR7_Mask_25))<<25);\
		v = ((v & MTL_QECR7_CC_Wr_Mask) | ((data & MTL_QECR7_CC_Mask)<<3));\
		MTL_QECR7_RgWr(v);\
} while(0)

#define MTL_QECR7_CC_UdfRd(data) do {\
		MTL_QECR7_RgRd(data);\
		data = ((data >> 3) & MTL_QECR7_CC_Mask);\
} while(0)

/*#define MTL_QECR7_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR7_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR7_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR7_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR7_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR7_RgRd(v);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_0))|((( 0) & (MTL_QECR7_Mask_0))<<0);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_7))|((( 0) & (MTL_QECR7_Mask_7))<<7);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_25))|((( 0) & (MTL_QECR7_Mask_25))<<25);\
		v = ((v & MTL_QECR7_SLC_Wr_Mask) | ((data & MTL_QECR7_SLC_Mask)<<4));\
		MTL_QECR7_RgWr(v);\
} while(0)

#define MTL_QECR7_SLC_UdfRd(data) do {\
		MTL_QECR7_RgRd(data);\
		data = ((data >> 4) & MTL_QECR7_SLC_Mask);\
} while(0)

/*#define MTL_QECR7_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR7_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR7_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR7_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR7_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR7_RgRd(v);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_0))|((( 0) & (MTL_QECR7_Mask_0))<<0);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_7))|((( 0) & (MTL_QECR7_Mask_7))<<7);\
		v = (v & (MTL_QECR7_RES_Wr_Mask_25))|((( 0) & (MTL_QECR7_Mask_25))<<25);\
		v = ((v & MTL_QECR7_ABPSSIE_Wr_Mask) | ((data & MTL_QECR7_ABPSSIE_Mask)<<24));\
		MTL_QECR7_RgWr(v);\
} while(0)

#define MTL_QECR7_ABPSSIE_UdfRd(data) do {\
		MTL_QECR7_RgRd(data);\
		data = ((data >> 24) & MTL_QECR7_ABPSSIE_Mask);\
} while(0)


#define MTL_QECR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe90))

#define MTL_QECR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR6_RgOffAddr);\
} while(0)

#define MTL_QECR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR6_RgOffAddr);\
} while(0)

/*#define  MTL_QECR6_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR6_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR6_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR6_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR6_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR6_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR6_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR6_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR6_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR6_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR6_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR6_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR6_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR6_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR6_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR6_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR6_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR6_RgRd(v);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_0))|((( 0) & (MTL_QECR6_Mask_0))<<0);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_7))|((( 0) & (MTL_QECR6_Mask_7))<<7);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_25))|((( 0) & (MTL_QECR6_Mask_25))<<25);\
		v = ((v & MTL_QECR6_AVALG_Wr_Mask) | ((data & MTL_QECR6_AVALG_Mask)<<2));\
		MTL_QECR6_RgWr(v);\
} while(0)

#define MTL_QECR6_AVALG_UdfRd(data) do {\
		MTL_QECR6_RgRd(data);\
		data = ((data >> 2) & MTL_QECR6_AVALG_Mask);\
} while(0)

/*#define MTL_QECR6_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR6_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR6_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR6_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR6_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR6_RgRd(v);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_0))|((( 0) & (MTL_QECR6_Mask_0))<<0);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_7))|((( 0) & (MTL_QECR6_Mask_7))<<7);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_25))|((( 0) & (MTL_QECR6_Mask_25))<<25);\
		v = ((v & MTL_QECR6_CC_Wr_Mask) | ((data & MTL_QECR6_CC_Mask)<<3));\
		MTL_QECR6_RgWr(v);\
} while(0)

#define MTL_QECR6_CC_UdfRd(data) do {\
		MTL_QECR6_RgRd(data);\
		data = ((data >> 3) & MTL_QECR6_CC_Mask);\
} while(0)

/*#define MTL_QECR6_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR6_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR6_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR6_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR6_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR6_RgRd(v);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_0))|((( 0) & (MTL_QECR6_Mask_0))<<0);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_7))|((( 0) & (MTL_QECR6_Mask_7))<<7);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_25))|((( 0) & (MTL_QECR6_Mask_25))<<25);\
		v = ((v & MTL_QECR6_SLC_Wr_Mask) | ((data & MTL_QECR6_SLC_Mask)<<4));\
		MTL_QECR6_RgWr(v);\
} while(0)

#define MTL_QECR6_SLC_UdfRd(data) do {\
		MTL_QECR6_RgRd(data);\
		data = ((data >> 4) & MTL_QECR6_SLC_Mask);\
} while(0)

/*#define MTL_QECR6_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR6_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR6_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR6_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR6_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR6_RgRd(v);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_0))|((( 0) & (MTL_QECR6_Mask_0))<<0);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_7))|((( 0) & (MTL_QECR6_Mask_7))<<7);\
		v = (v & (MTL_QECR6_RES_Wr_Mask_25))|((( 0) & (MTL_QECR6_Mask_25))<<25);\
		v = ((v & MTL_QECR6_ABPSSIE_Wr_Mask) | ((data & MTL_QECR6_ABPSSIE_Mask)<<24));\
		MTL_QECR6_RgWr(v);\
} while(0)

#define MTL_QECR6_ABPSSIE_UdfRd(data) do {\
		MTL_QECR6_RgRd(data);\
		data = ((data >> 24) & MTL_QECR6_ABPSSIE_Mask);\
} while(0)


#define MTL_QECR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe50))

#define MTL_QECR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR5_RgOffAddr);\
} while(0)

#define MTL_QECR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR5_RgOffAddr);\
} while(0)

/*#define  MTL_QECR5_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR5_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR5_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR5_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR5_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR5_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR5_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR5_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR5_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR5_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR5_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR5_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR5_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR5_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR5_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR5_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR5_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR5_RgRd(v);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_0))|((( 0) & (MTL_QECR5_Mask_0))<<0);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_7))|((( 0) & (MTL_QECR5_Mask_7))<<7);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_25))|((( 0) & (MTL_QECR5_Mask_25))<<25);\
		v = ((v & MTL_QECR5_AVALG_Wr_Mask) | ((data & MTL_QECR5_AVALG_Mask)<<2));\
		MTL_QECR5_RgWr(v);\
} while(0)

#define MTL_QECR5_AVALG_UdfRd(data) do {\
		MTL_QECR5_RgRd(data);\
		data = ((data >> 2) & MTL_QECR5_AVALG_Mask);\
} while(0)

/*#define MTL_QECR5_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR5_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR5_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR5_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR5_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR5_RgRd(v);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_0))|((( 0) & (MTL_QECR5_Mask_0))<<0);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_7))|((( 0) & (MTL_QECR5_Mask_7))<<7);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_25))|((( 0) & (MTL_QECR5_Mask_25))<<25);\
		v = ((v & MTL_QECR5_CC_Wr_Mask) | ((data & MTL_QECR5_CC_Mask)<<3));\
		MTL_QECR5_RgWr(v);\
} while(0)

#define MTL_QECR5_CC_UdfRd(data) do {\
		MTL_QECR5_RgRd(data);\
		data = ((data >> 3) & MTL_QECR5_CC_Mask);\
} while(0)

/*#define MTL_QECR5_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR5_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR5_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR5_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR5_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR5_RgRd(v);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_0))|((( 0) & (MTL_QECR5_Mask_0))<<0);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_7))|((( 0) & (MTL_QECR5_Mask_7))<<7);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_25))|((( 0) & (MTL_QECR5_Mask_25))<<25);\
		v = ((v & MTL_QECR5_SLC_Wr_Mask) | ((data & MTL_QECR5_SLC_Mask)<<4));\
		MTL_QECR5_RgWr(v);\
} while(0)

#define MTL_QECR5_SLC_UdfRd(data) do {\
		MTL_QECR5_RgRd(data);\
		data = ((data >> 4) & MTL_QECR5_SLC_Mask);\
} while(0)

/*#define MTL_QECR5_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR5_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR5_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR5_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR5_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR5_RgRd(v);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_0))|((( 0) & (MTL_QECR5_Mask_0))<<0);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_7))|((( 0) & (MTL_QECR5_Mask_7))<<7);\
		v = (v & (MTL_QECR5_RES_Wr_Mask_25))|((( 0) & (MTL_QECR5_Mask_25))<<25);\
		v = ((v & MTL_QECR5_ABPSSIE_Wr_Mask) | ((data & MTL_QECR5_ABPSSIE_Mask)<<24));\
		MTL_QECR5_RgWr(v);\
} while(0)

#define MTL_QECR5_ABPSSIE_UdfRd(data) do {\
		MTL_QECR5_RgRd(data);\
		data = ((data >> 24) & MTL_QECR5_ABPSSIE_Mask);\
} while(0)


#define MTL_QECR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe10))

#define MTL_QECR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR4_RgOffAddr);\
} while(0)

#define MTL_QECR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR4_RgOffAddr);\
} while(0)

/*#define  MTL_QECR4_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR4_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR4_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR4_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR4_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR4_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR4_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR4_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR4_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR4_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR4_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR4_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR4_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR4_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR4_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR4_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR4_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR4_RgRd(v);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_0))|((( 0) & (MTL_QECR4_Mask_0))<<0);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_7))|((( 0) & (MTL_QECR4_Mask_7))<<7);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_25))|((( 0) & (MTL_QECR4_Mask_25))<<25);\
		v = ((v & MTL_QECR4_AVALG_Wr_Mask) | ((data & MTL_QECR4_AVALG_Mask)<<2));\
		MTL_QECR4_RgWr(v);\
} while(0)

#define MTL_QECR4_AVALG_UdfRd(data) do {\
		MTL_QECR4_RgRd(data);\
		data = ((data >> 2) & MTL_QECR4_AVALG_Mask);\
} while(0)

/*#define MTL_QECR4_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR4_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR4_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR4_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR4_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR4_RgRd(v);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_0))|((( 0) & (MTL_QECR4_Mask_0))<<0);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_7))|((( 0) & (MTL_QECR4_Mask_7))<<7);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_25))|((( 0) & (MTL_QECR4_Mask_25))<<25);\
		v = ((v & MTL_QECR4_CC_Wr_Mask) | ((data & MTL_QECR4_CC_Mask)<<3));\
		MTL_QECR4_RgWr(v);\
} while(0)

#define MTL_QECR4_CC_UdfRd(data) do {\
		MTL_QECR4_RgRd(data);\
		data = ((data >> 3) & MTL_QECR4_CC_Mask);\
} while(0)

/*#define MTL_QECR4_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR4_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR4_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR4_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR4_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR4_RgRd(v);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_0))|((( 0) & (MTL_QECR4_Mask_0))<<0);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_7))|((( 0) & (MTL_QECR4_Mask_7))<<7);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_25))|((( 0) & (MTL_QECR4_Mask_25))<<25);\
		v = ((v & MTL_QECR4_SLC_Wr_Mask) | ((data & MTL_QECR4_SLC_Mask)<<4));\
		MTL_QECR4_RgWr(v);\
} while(0)

#define MTL_QECR4_SLC_UdfRd(data) do {\
		MTL_QECR4_RgRd(data);\
		data = ((data >> 4) & MTL_QECR4_SLC_Mask);\
} while(0)

/*#define MTL_QECR4_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR4_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR4_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR4_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR4_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR4_RgRd(v);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_0))|((( 0) & (MTL_QECR4_Mask_0))<<0);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_7))|((( 0) & (MTL_QECR4_Mask_7))<<7);\
		v = (v & (MTL_QECR4_RES_Wr_Mask_25))|((( 0) & (MTL_QECR4_Mask_25))<<25);\
		v = ((v & MTL_QECR4_ABPSSIE_Wr_Mask) | ((data & MTL_QECR4_ABPSSIE_Mask)<<24));\
		MTL_QECR4_RgWr(v);\
} while(0)

#define MTL_QECR4_ABPSSIE_UdfRd(data) do {\
		MTL_QECR4_RgRd(data);\
		data = ((data >> 24) & MTL_QECR4_ABPSSIE_Mask);\
} while(0)


#define MTL_QECR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdd0))

#define MTL_QECR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR3_RgOffAddr);\
} while(0)

#define MTL_QECR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR3_RgOffAddr);\
} while(0)

/*#define  MTL_QECR3_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR3_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR3_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR3_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR3_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR3_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR3_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR3_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR3_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR3_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR3_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR3_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR3_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR3_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR3_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR3_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR3_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR3_RgRd(v);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_0))|((( 0) & (MTL_QECR3_Mask_0))<<0);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_7))|((( 0) & (MTL_QECR3_Mask_7))<<7);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_25))|((( 0) & (MTL_QECR3_Mask_25))<<25);\
		v = ((v & MTL_QECR3_AVALG_Wr_Mask) | ((data & MTL_QECR3_AVALG_Mask)<<2));\
		MTL_QECR3_RgWr(v);\
} while(0)

#define MTL_QECR3_AVALG_UdfRd(data) do {\
		MTL_QECR3_RgRd(data);\
		data = ((data >> 2) & MTL_QECR3_AVALG_Mask);\
} while(0)

/*#define MTL_QECR3_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR3_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR3_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR3_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR3_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR3_RgRd(v);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_0))|((( 0) & (MTL_QECR3_Mask_0))<<0);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_7))|((( 0) & (MTL_QECR3_Mask_7))<<7);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_25))|((( 0) & (MTL_QECR3_Mask_25))<<25);\
		v = ((v & MTL_QECR3_CC_Wr_Mask) | ((data & MTL_QECR3_CC_Mask)<<3));\
		MTL_QECR3_RgWr(v);\
} while(0)

#define MTL_QECR3_CC_UdfRd(data) do {\
		MTL_QECR3_RgRd(data);\
		data = ((data >> 3) & MTL_QECR3_CC_Mask);\
} while(0)

/*#define MTL_QECR3_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR3_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR3_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR3_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR3_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR3_RgRd(v);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_0))|((( 0) & (MTL_QECR3_Mask_0))<<0);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_7))|((( 0) & (MTL_QECR3_Mask_7))<<7);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_25))|((( 0) & (MTL_QECR3_Mask_25))<<25);\
		v = ((v & MTL_QECR3_SLC_Wr_Mask) | ((data & MTL_QECR3_SLC_Mask)<<4));\
		MTL_QECR3_RgWr(v);\
} while(0)

#define MTL_QECR3_SLC_UdfRd(data) do {\
		MTL_QECR3_RgRd(data);\
		data = ((data >> 4) & MTL_QECR3_SLC_Mask);\
} while(0)

/*#define MTL_QECR3_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR3_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR3_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR3_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR3_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR3_RgRd(v);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_0))|((( 0) & (MTL_QECR3_Mask_0))<<0);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_7))|((( 0) & (MTL_QECR3_Mask_7))<<7);\
		v = (v & (MTL_QECR3_RES_Wr_Mask_25))|((( 0) & (MTL_QECR3_Mask_25))<<25);\
		v = ((v & MTL_QECR3_ABPSSIE_Wr_Mask) | ((data & MTL_QECR3_ABPSSIE_Mask)<<24));\
		MTL_QECR3_RgWr(v);\
} while(0)

#define MTL_QECR3_ABPSSIE_UdfRd(data) do {\
		MTL_QECR3_RgRd(data);\
		data = ((data >> 24) & MTL_QECR3_ABPSSIE_Mask);\
} while(0)


#define MTL_QECR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd90))

#define MTL_QECR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR2_RgOffAddr);\
} while(0)

#define MTL_QECR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR2_RgOffAddr);\
} while(0)

/*#define  MTL_QECR2_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR2_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR2_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR2_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR2_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR2_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR2_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR2_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR2_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR2_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR2_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR2_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR2_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR2_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR2_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR2_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR2_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR2_RgRd(v);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_0))|((( 0) & (MTL_QECR2_Mask_0))<<0);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_7))|((( 0) & (MTL_QECR2_Mask_7))<<7);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_25))|((( 0) & (MTL_QECR2_Mask_25))<<25);\
		v = ((v & MTL_QECR2_AVALG_Wr_Mask) | ((data & MTL_QECR2_AVALG_Mask)<<2));\
		MTL_QECR2_RgWr(v);\
} while(0)

#define MTL_QECR2_AVALG_UdfRd(data) do {\
		MTL_QECR2_RgRd(data);\
		data = ((data >> 2) & MTL_QECR2_AVALG_Mask);\
} while(0)

/*#define MTL_QECR2_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR2_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR2_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR2_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR2_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR2_RgRd(v);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_0))|((( 0) & (MTL_QECR2_Mask_0))<<0);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_7))|((( 0) & (MTL_QECR2_Mask_7))<<7);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_25))|((( 0) & (MTL_QECR2_Mask_25))<<25);\
		v = ((v & MTL_QECR2_CC_Wr_Mask) | ((data & MTL_QECR2_CC_Mask)<<3));\
		MTL_QECR2_RgWr(v);\
} while(0)

#define MTL_QECR2_CC_UdfRd(data) do {\
		MTL_QECR2_RgRd(data);\
		data = ((data >> 3) & MTL_QECR2_CC_Mask);\
} while(0)

/*#define MTL_QECR2_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR2_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR2_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR2_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR2_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR2_RgRd(v);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_0))|((( 0) & (MTL_QECR2_Mask_0))<<0);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_7))|((( 0) & (MTL_QECR2_Mask_7))<<7);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_25))|((( 0) & (MTL_QECR2_Mask_25))<<25);\
		v = ((v & MTL_QECR2_SLC_Wr_Mask) | ((data & MTL_QECR2_SLC_Mask)<<4));\
		MTL_QECR2_RgWr(v);\
} while(0)

#define MTL_QECR2_SLC_UdfRd(data) do {\
		MTL_QECR2_RgRd(data);\
		data = ((data >> 4) & MTL_QECR2_SLC_Mask);\
} while(0)

/*#define MTL_QECR2_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR2_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR2_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR2_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR2_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR2_RgRd(v);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_0))|((( 0) & (MTL_QECR2_Mask_0))<<0);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_7))|((( 0) & (MTL_QECR2_Mask_7))<<7);\
		v = (v & (MTL_QECR2_RES_Wr_Mask_25))|((( 0) & (MTL_QECR2_Mask_25))<<25);\
		v = ((v & MTL_QECR2_ABPSSIE_Wr_Mask) | ((data & MTL_QECR2_ABPSSIE_Mask)<<24));\
		MTL_QECR2_RgWr(v);\
} while(0)

#define MTL_QECR2_ABPSSIE_UdfRd(data) do {\
		MTL_QECR2_RgRd(data);\
		data = ((data >> 24) & MTL_QECR2_ABPSSIE_Mask);\
} while(0)


#define MTL_QECR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd50))

#define MTL_QECR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QECR1_RgOffAddr);\
} while(0)

#define MTL_QECR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QECR1_RgOffAddr);\
} while(0)

/*#define  MTL_QECR1_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR1_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR1_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR1_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define  MTL_QECR1_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR1_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR1_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR1_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR1_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR1_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR1_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR1_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MTL_QECR1_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR1_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR1_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR1_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR1_AVALG_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR1_RgRd(v);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_0))|((( 0) & (MTL_QECR1_Mask_0))<<0);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_7))|((( 0) & (MTL_QECR1_Mask_7))<<7);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_25))|((( 0) & (MTL_QECR1_Mask_25))<<25);\
		v = ((v & MTL_QECR1_AVALG_Wr_Mask) | ((data & MTL_QECR1_AVALG_Mask)<<2));\
		MTL_QECR1_RgWr(v);\
} while(0)

#define MTL_QECR1_AVALG_UdfRd(data) do {\
		MTL_QECR1_RgRd(data);\
		data = ((data >> 2) & MTL_QECR1_AVALG_Mask);\
} while(0)

/*#define MTL_QECR1_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR1_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR1_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR1_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR1_CC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR1_RgRd(v);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_0))|((( 0) & (MTL_QECR1_Mask_0))<<0);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_7))|((( 0) & (MTL_QECR1_Mask_7))<<7);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_25))|((( 0) & (MTL_QECR1_Mask_25))<<25);\
		v = ((v & MTL_QECR1_CC_Wr_Mask) | ((data & MTL_QECR1_CC_Mask)<<3));\
		MTL_QECR1_RgWr(v);\
} while(0)

#define MTL_QECR1_CC_UdfRd(data) do {\
		MTL_QECR1_RgRd(data);\
		data = ((data >> 3) & MTL_QECR1_CC_Mask);\
} while(0)

/*#define MTL_QECR1_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR1_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR1_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR1_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR1_SLC_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR1_RgRd(v);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_0))|((( 0) & (MTL_QECR1_Mask_0))<<0);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_7))|((( 0) & (MTL_QECR1_Mask_7))<<7);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_25))|((( 0) & (MTL_QECR1_Mask_25))<<25);\
		v = ((v & MTL_QECR1_SLC_Wr_Mask) | ((data & MTL_QECR1_SLC_Mask)<<4));\
		MTL_QECR1_RgWr(v);\
} while(0)

#define MTL_QECR1_SLC_UdfRd(data) do {\
		MTL_QECR1_RgRd(data);\
		data = ((data >> 4) & MTL_QECR1_SLC_Mask);\
} while(0)

/*#define MTL_QECR1_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR1_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR1_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR1_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR1_ABPSSIE_UdfWr(data) do {\
		ULONG v;\
		MTL_QECR1_RgRd(v);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_0))|((( 0) & (MTL_QECR1_Mask_0))<<0);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_7))|((( 0) & (MTL_QECR1_Mask_7))<<7);\
		v = (v & (MTL_QECR1_RES_Wr_Mask_25))|((( 0) & (MTL_QECR1_Mask_25))<<25);\
		v = ((v & MTL_QECR1_ABPSSIE_Wr_Mask) | ((data & MTL_QECR1_ABPSSIE_Mask)<<24));\
		MTL_QECR1_RgWr(v);\
} while(0)

#define MTL_QECR1_ABPSSIE_UdfRd(data) do {\
		MTL_QECR1_RgRd(data);\
		data = ((data >> 24) & MTL_QECR1_ABPSSIE_Mask);\
} while(0)


#define MTL_QTDR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xec8))

#define MTL_QTDR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR7_RgOffAddr);\
} while(0)

/*#define MTL_QTDR7_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR7_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR7_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR7_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR7_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR7_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR7_TRCSTS_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR7_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR7_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR7_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR7_TWCSTS_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR7_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR7_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR7_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR7_TXQSTS_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR7_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR7_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR7_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR7_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR7_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR7_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR7_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR7_PTXQ_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR7_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR7_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR7_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR7_STXSTSF_UdfRd(data) do {\
		MTL_QTDR7_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR7_STXSTSF_Mask);\
} while(0)


#define MTL_QTDR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe88))

#define MTL_QTDR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR6_RgOffAddr);\
} while(0)

/*#define MTL_QTDR6_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR6_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR6_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR6_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR6_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR6_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR6_TRCSTS_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR6_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR6_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR6_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR6_TWCSTS_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR6_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR6_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR6_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR6_TXQSTS_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR6_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR6_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR6_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR6_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR6_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR6_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR6_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR6_PTXQ_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR6_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR6_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR6_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR6_STXSTSF_UdfRd(data) do {\
		MTL_QTDR6_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR6_STXSTSF_Mask);\
} while(0)


#define MTL_QTDR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe48))

#define MTL_QTDR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR5_RgOffAddr);\
} while(0)

/*#define MTL_QTDR5_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR5_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR5_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR5_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR5_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR5_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR5_TRCSTS_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR5_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR5_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR5_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR5_TWCSTS_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR5_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR5_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR5_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR5_TXQSTS_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR5_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR5_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR5_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR5_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR5_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR5_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR5_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR5_PTXQ_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR5_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR5_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR5_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR5_STXSTSF_UdfRd(data) do {\
		MTL_QTDR5_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR5_STXSTSF_Mask);\
} while(0)


#define MTL_QTDR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe08))

#define MTL_QTDR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR4_RgOffAddr);\
} while(0)

/*#define MTL_QTDR4_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR4_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR4_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR4_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR4_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR4_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR4_TRCSTS_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR4_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR4_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR4_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR4_TWCSTS_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR4_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR4_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR4_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR4_TXQSTS_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR4_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR4_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR4_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR4_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR4_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR4_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR4_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR4_PTXQ_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR4_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR4_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR4_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR4_STXSTSF_UdfRd(data) do {\
		MTL_QTDR4_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR4_STXSTSF_Mask);\
} while(0)


#define MTL_QTDR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdc8))

#define MTL_QTDR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR3_RgOffAddr);\
} while(0)

/*#define MTL_QTDR3_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR3_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR3_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR3_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR3_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR3_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR3_TRCSTS_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR3_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR3_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR3_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR3_TWCSTS_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR3_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR3_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR3_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR3_TXQSTS_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR3_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR3_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR3_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR3_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR3_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR3_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR3_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR3_PTXQ_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR3_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR3_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR3_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR3_STXSTSF_UdfRd(data) do {\
		MTL_QTDR3_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR3_STXSTSF_Mask);\
} while(0)


#define MTL_QTDR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd88))

#define MTL_QTDR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR2_RgOffAddr);\
} while(0)

/*#define MTL_QTDR2_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR2_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR2_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR2_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR2_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR2_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR2_TRCSTS_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR2_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR2_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR2_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR2_TWCSTS_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR2_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR2_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR2_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR2_TXQSTS_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR2_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR2_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR2_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR2_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR2_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR2_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR2_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR2_PTXQ_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR2_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR2_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR2_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR2_STXSTSF_UdfRd(data) do {\
		MTL_QTDR2_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR2_STXSTSF_Mask);\
} while(0)


#define MTL_QTDR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd48))

#define MTL_QTDR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTDR1_RgOffAddr);\
} while(0)

/*#define MTL_QTDR1_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR1_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR1_TXQPAUSED_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 0) & MTL_QTDR1_TXQPAUSED_Mask);\
} while(0)

/*#define MTL_QTDR1_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR1_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR1_TRCSTS_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 1) & MTL_QTDR1_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR1_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR1_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR1_TWCSTS_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 3) & MTL_QTDR1_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR1_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR1_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR1_TXQSTS_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 4) & MTL_QTDR1_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR1_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR1_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR1_TXSTSFSTS_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 5) & MTL_QTDR1_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR1_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR1_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR1_PTXQ_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 16) & MTL_QTDR1_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR1_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR1_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR1_STXSTSF_UdfRd(data) do {\
		MTL_QTDR1_RgRd(data);\
		data = ((data >> 20) & MTL_QTDR1_STXSTSF_Mask);\
} while(0)


#define MTL_QUCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xec4))

#define MTL_QUCR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR7_RgOffAddr);\
} while(0)

#define MTL_QUCR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR7_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR7_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR7_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR7_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR7_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR7_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR7_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR7_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR7_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR7_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR7_RgRd(v);\
		v = (v & (MTL_QUCR7_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR7_Mask_12))<<12);\
		v = ((v & MTL_QUCR7_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR7_UFPKTCNT_Mask)<<0));\
		MTL_QUCR7_RgWr(v);\
} while(0)

#define MTL_QUCR7_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR7_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR7_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR7_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR7_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR7_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR7_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR7_UFCNTOVF_Mask);\
} while(0)


#define MTL_QUCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe84))

#define MTL_QUCR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR6_RgOffAddr);\
} while(0)

#define MTL_QUCR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR6_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR6_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR6_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR6_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR6_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR6_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR6_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR6_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR6_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR6_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR6_RgRd(v);\
		v = (v & (MTL_QUCR6_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR6_Mask_12))<<12);\
		v = ((v & MTL_QUCR6_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR6_UFPKTCNT_Mask)<<0));\
		MTL_QUCR6_RgWr(v);\
} while(0)

#define MTL_QUCR6_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR6_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR6_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR6_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR6_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR6_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR6_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR6_UFCNTOVF_Mask);\
} while(0)


#define MTL_QUCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe44))

#define MTL_QUCR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR5_RgOffAddr);\
} while(0)

#define MTL_QUCR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR5_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR5_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR5_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR5_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR5_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR5_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR5_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR5_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR5_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR5_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR5_RgRd(v);\
		v = (v & (MTL_QUCR5_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR5_Mask_12))<<12);\
		v = ((v & MTL_QUCR5_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR5_UFPKTCNT_Mask)<<0));\
		MTL_QUCR5_RgWr(v);\
} while(0)

#define MTL_QUCR5_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR5_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR5_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR5_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR5_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR5_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR5_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR5_UFCNTOVF_Mask);\
} while(0)


#define MTL_QUCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe04))

#define MTL_QUCR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR4_RgOffAddr);\
} while(0)

#define MTL_QUCR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR4_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR4_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR4_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR4_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR4_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR4_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR4_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR4_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR4_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR4_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR4_RgRd(v);\
		v = (v & (MTL_QUCR4_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR4_Mask_12))<<12);\
		v = ((v & MTL_QUCR4_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR4_UFPKTCNT_Mask)<<0));\
		MTL_QUCR4_RgWr(v);\
} while(0)

#define MTL_QUCR4_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR4_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR4_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR4_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR4_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR4_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR4_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR4_UFCNTOVF_Mask);\
} while(0)


#define MTL_QUCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdc4))

#define MTL_QUCR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR3_RgOffAddr);\
} while(0)

#define MTL_QUCR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR3_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR3_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR3_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR3_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR3_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR3_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR3_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR3_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR3_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR3_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR3_RgRd(v);\
		v = (v & (MTL_QUCR3_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR3_Mask_12))<<12);\
		v = ((v & MTL_QUCR3_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR3_UFPKTCNT_Mask)<<0));\
		MTL_QUCR3_RgWr(v);\
} while(0)

#define MTL_QUCR3_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR3_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR3_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR3_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR3_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR3_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR3_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR3_UFCNTOVF_Mask);\
} while(0)


#define MTL_QUCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd84))

#define MTL_QUCR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR2_RgOffAddr);\
} while(0)

#define MTL_QUCR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR2_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR2_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR2_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR2_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR2_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR2_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR2_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR2_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR2_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR2_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR2_RgRd(v);\
		v = (v & (MTL_QUCR2_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR2_Mask_12))<<12);\
		v = ((v & MTL_QUCR2_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR2_UFPKTCNT_Mask)<<0));\
		MTL_QUCR2_RgWr(v);\
} while(0)

#define MTL_QUCR2_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR2_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR2_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR2_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR2_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR2_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR2_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR2_UFCNTOVF_Mask);\
} while(0)


#define MTL_QUCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd44))

#define MTL_QUCR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QUCR1_RgOffAddr);\
} while(0)

#define MTL_QUCR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QUCR1_RgOffAddr);\
} while(0)

/*#define  MTL_QUCR1_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR1_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR1_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR1_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR1_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR1_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR1_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR1_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR1_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_QUCR1_RgRd(v);\
		v = (v & (MTL_QUCR1_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR1_Mask_12))<<12);\
		v = ((v & MTL_QUCR1_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR1_UFPKTCNT_Mask)<<0));\
		MTL_QUCR1_RgWr(v);\
} while(0)

#define MTL_QUCR1_UFPKTCNT_UdfRd(data) do {\
		MTL_QUCR1_RgRd(data);\
		data = ((data >> 0) & MTL_QUCR1_UFPKTCNT_Mask);\
} while(0)

/*#define MTL_QUCR1_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR1_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR1_UFCNTOVF_UdfRd(data) do {\
		MTL_QUCR1_RgRd(data);\
		data = ((data >> 11) & MTL_QUCR1_UFCNTOVF_Mask);\
} while(0)


#define MTL_QTOMR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xec0))

#define MTL_QTOMR7_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR7_RgOffAddr);\
} while(0)

#define MTL_QTOMR7_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR7_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR7_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR7_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR7_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR7_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR7_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR7_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR7_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR7_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR7_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR7_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR7_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR7_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR7_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR7_RgRd(v);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR7_Mask_7))<<7);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR7_Mask_26))<<26);\
		v = ((v & MTL_QTOMR7_FTQ_Wr_Mask) | ((data & MTL_QTOMR7_FTQ_Mask)<<0));\
		MTL_QTOMR7_RgWr(v);\
} while(0)

#define MTL_QTOMR7_FTQ_UdfRd(data) do {\
		MTL_QTOMR7_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR7_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR7_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR7_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR7_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR7_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR7_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR7_RgRd(v);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR7_Mask_7))<<7);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR7_Mask_26))<<26);\
		v = ((v & MTL_QTOMR7_TSF_Wr_Mask) | ((data & MTL_QTOMR7_TSF_Mask)<<1));\
		MTL_QTOMR7_RgWr(v);\
} while(0)

#define MTL_QTOMR7_TSF_UdfRd(data) do {\
		MTL_QTOMR7_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR7_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR7_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR7_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR7_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR7_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR7_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR7_RgRd(v);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR7_Mask_7))<<7);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR7_Mask_26))<<26);\
		v = ((v & MTL_QTOMR7_TXQEN_Wr_Mask) | ((data & MTL_QTOMR7_TXQEN_Mask)<<2));\
		MTL_QTOMR7_RgWr(v);\
} while(0)

#define MTL_QTOMR7_TXQEN_UdfRd(data) do {\
		MTL_QTOMR7_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR7_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR7_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR7_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR7_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR7_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR7_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR7_RgRd(v);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR7_Mask_7))<<7);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR7_Mask_26))<<26);\
		v = ((v & MTL_QTOMR7_TTC_Wr_Mask) | ((data & MTL_QTOMR7_TTC_Mask)<<4));\
		MTL_QTOMR7_RgWr(v);\
} while(0)

#define MTL_QTOMR7_TTC_UdfRd(data) do {\
		MTL_QTOMR7_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR7_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR7_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR7_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR7_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR7_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR7_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR7_RgRd(v);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR7_Mask_7))<<7);\
		v = (v & (MTL_QTOMR7_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR7_Mask_26))<<26);\
		v = ((v & MTL_QTOMR7_TQS_Wr_Mask) | ((data & MTL_QTOMR7_TQS_Mask)<<16));\
		MTL_QTOMR7_RgWr(v);\
} while(0)

#define MTL_QTOMR7_TQS_UdfRd(data) do {\
		MTL_QTOMR7_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR7_TQS_Mask);\
} while(0)


#define MTL_QTOMR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe80))

#define MTL_QTOMR6_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR6_RgOffAddr);\
} while(0)

#define MTL_QTOMR6_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR6_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR6_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR6_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR6_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR6_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR6_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR6_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR6_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR6_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR6_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR6_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR6_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR6_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR6_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR6_RgRd(v);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR6_Mask_7))<<7);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR6_Mask_26))<<26);\
		v = ((v & MTL_QTOMR6_FTQ_Wr_Mask) | ((data & MTL_QTOMR6_FTQ_Mask)<<0));\
		MTL_QTOMR6_RgWr(v);\
} while(0)

#define MTL_QTOMR6_FTQ_UdfRd(data) do {\
		MTL_QTOMR6_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR6_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR6_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR6_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR6_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR6_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR6_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR6_RgRd(v);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR6_Mask_7))<<7);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR6_Mask_26))<<26);\
		v = ((v & MTL_QTOMR6_TSF_Wr_Mask) | ((data & MTL_QTOMR6_TSF_Mask)<<1));\
		MTL_QTOMR6_RgWr(v);\
} while(0)

#define MTL_QTOMR6_TSF_UdfRd(data) do {\
		MTL_QTOMR6_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR6_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR6_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR6_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR6_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR6_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR6_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR6_RgRd(v);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR6_Mask_7))<<7);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR6_Mask_26))<<26);\
		v = ((v & MTL_QTOMR6_TXQEN_Wr_Mask) | ((data & MTL_QTOMR6_TXQEN_Mask)<<2));\
		MTL_QTOMR6_RgWr(v);\
} while(0)

#define MTL_QTOMR6_TXQEN_UdfRd(data) do {\
		MTL_QTOMR6_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR6_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR6_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR6_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR6_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR6_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR6_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR6_RgRd(v);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR6_Mask_7))<<7);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR6_Mask_26))<<26);\
		v = ((v & MTL_QTOMR6_TTC_Wr_Mask) | ((data & MTL_QTOMR6_TTC_Mask)<<4));\
		MTL_QTOMR6_RgWr(v);\
} while(0)

#define MTL_QTOMR6_TTC_UdfRd(data) do {\
		MTL_QTOMR6_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR6_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR6_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR6_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR6_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR6_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR6_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR6_RgRd(v);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR6_Mask_7))<<7);\
		v = (v & (MTL_QTOMR6_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR6_Mask_26))<<26);\
		v = ((v & MTL_QTOMR6_TQS_Wr_Mask) | ((data & MTL_QTOMR6_TQS_Mask)<<16));\
		MTL_QTOMR6_RgWr(v);\
} while(0)

#define MTL_QTOMR6_TQS_UdfRd(data) do {\
		MTL_QTOMR6_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR6_TQS_Mask);\
} while(0)


#define MTL_QTOMR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe40))

#define MTL_QTOMR5_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR5_RgOffAddr);\
} while(0)

#define MTL_QTOMR5_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR5_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR5_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR5_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR5_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR5_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR5_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR5_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR5_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR5_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR5_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR5_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR5_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR5_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR5_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR5_RgRd(v);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR5_Mask_7))<<7);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR5_Mask_26))<<26);\
		v = ((v & MTL_QTOMR5_FTQ_Wr_Mask) | ((data & MTL_QTOMR5_FTQ_Mask)<<0));\
		MTL_QTOMR5_RgWr(v);\
} while(0)

#define MTL_QTOMR5_FTQ_UdfRd(data) do {\
		MTL_QTOMR5_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR5_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR5_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR5_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR5_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR5_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR5_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR5_RgRd(v);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR5_Mask_7))<<7);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR5_Mask_26))<<26);\
		v = ((v & MTL_QTOMR5_TSF_Wr_Mask) | ((data & MTL_QTOMR5_TSF_Mask)<<1));\
		MTL_QTOMR5_RgWr(v);\
} while(0)

#define MTL_QTOMR5_TSF_UdfRd(data) do {\
		MTL_QTOMR5_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR5_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR5_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR5_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR5_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR5_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR5_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR5_RgRd(v);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR5_Mask_7))<<7);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR5_Mask_26))<<26);\
		v = ((v & MTL_QTOMR5_TXQEN_Wr_Mask) | ((data & MTL_QTOMR5_TXQEN_Mask)<<2));\
		MTL_QTOMR5_RgWr(v);\
} while(0)

#define MTL_QTOMR5_TXQEN_UdfRd(data) do {\
		MTL_QTOMR5_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR5_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR5_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR5_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR5_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR5_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR5_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR5_RgRd(v);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR5_Mask_7))<<7);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR5_Mask_26))<<26);\
		v = ((v & MTL_QTOMR5_TTC_Wr_Mask) | ((data & MTL_QTOMR5_TTC_Mask)<<4));\
		MTL_QTOMR5_RgWr(v);\
} while(0)

#define MTL_QTOMR5_TTC_UdfRd(data) do {\
		MTL_QTOMR5_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR5_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR5_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR5_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR5_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR5_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR5_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR5_RgRd(v);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR5_Mask_7))<<7);\
		v = (v & (MTL_QTOMR5_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR5_Mask_26))<<26);\
		v = ((v & MTL_QTOMR5_TQS_Wr_Mask) | ((data & MTL_QTOMR5_TQS_Mask)<<16));\
		MTL_QTOMR5_RgWr(v);\
} while(0)

#define MTL_QTOMR5_TQS_UdfRd(data) do {\
		MTL_QTOMR5_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR5_TQS_Mask);\
} while(0)


#define MTL_QTOMR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xe00))

#define MTL_QTOMR4_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR4_RgOffAddr);\
} while(0)

#define MTL_QTOMR4_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR4_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR4_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR4_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR4_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR4_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR4_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR4_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR4_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR4_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR4_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR4_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR4_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR4_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR4_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR4_RgRd(v);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR4_Mask_7))<<7);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR4_Mask_26))<<26);\
		v = ((v & MTL_QTOMR4_FTQ_Wr_Mask) | ((data & MTL_QTOMR4_FTQ_Mask)<<0));\
		MTL_QTOMR4_RgWr(v);\
} while(0)

#define MTL_QTOMR4_FTQ_UdfRd(data) do {\
		MTL_QTOMR4_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR4_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR4_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR4_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR4_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR4_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR4_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR4_RgRd(v);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR4_Mask_7))<<7);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR4_Mask_26))<<26);\
		v = ((v & MTL_QTOMR4_TSF_Wr_Mask) | ((data & MTL_QTOMR4_TSF_Mask)<<1));\
		MTL_QTOMR4_RgWr(v);\
} while(0)

#define MTL_QTOMR4_TSF_UdfRd(data) do {\
		MTL_QTOMR4_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR4_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR4_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR4_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR4_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR4_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR4_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR4_RgRd(v);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR4_Mask_7))<<7);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR4_Mask_26))<<26);\
		v = ((v & MTL_QTOMR4_TXQEN_Wr_Mask) | ((data & MTL_QTOMR4_TXQEN_Mask)<<2));\
		MTL_QTOMR4_RgWr(v);\
} while(0)

#define MTL_QTOMR4_TXQEN_UdfRd(data) do {\
		MTL_QTOMR4_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR4_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR4_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR4_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR4_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR4_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR4_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR4_RgRd(v);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR4_Mask_7))<<7);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR4_Mask_26))<<26);\
		v = ((v & MTL_QTOMR4_TTC_Wr_Mask) | ((data & MTL_QTOMR4_TTC_Mask)<<4));\
		MTL_QTOMR4_RgWr(v);\
} while(0)

#define MTL_QTOMR4_TTC_UdfRd(data) do {\
		MTL_QTOMR4_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR4_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR4_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR4_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR4_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR4_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR4_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR4_RgRd(v);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR4_Mask_7))<<7);\
		v = (v & (MTL_QTOMR4_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR4_Mask_26))<<26);\
		v = ((v & MTL_QTOMR4_TQS_Wr_Mask) | ((data & MTL_QTOMR4_TQS_Mask)<<16));\
		MTL_QTOMR4_RgWr(v);\
} while(0)

#define MTL_QTOMR4_TQS_UdfRd(data) do {\
		MTL_QTOMR4_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR4_TQS_Mask);\
} while(0)


#define MTL_QTOMR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xdc0))

#define MTL_QTOMR3_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR3_RgOffAddr);\
} while(0)

#define MTL_QTOMR3_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR3_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR3_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR3_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR3_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR3_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR3_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR3_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR3_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR3_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR3_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR3_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR3_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR3_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR3_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR3_RgRd(v);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR3_Mask_7))<<7);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR3_Mask_26))<<26);\
		v = ((v & MTL_QTOMR3_FTQ_Wr_Mask) | ((data & MTL_QTOMR3_FTQ_Mask)<<0));\
		MTL_QTOMR3_RgWr(v);\
} while(0)

#define MTL_QTOMR3_FTQ_UdfRd(data) do {\
		MTL_QTOMR3_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR3_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR3_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR3_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR3_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR3_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR3_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR3_RgRd(v);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR3_Mask_7))<<7);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR3_Mask_26))<<26);\
		v = ((v & MTL_QTOMR3_TSF_Wr_Mask) | ((data & MTL_QTOMR3_TSF_Mask)<<1));\
		MTL_QTOMR3_RgWr(v);\
} while(0)

#define MTL_QTOMR3_TSF_UdfRd(data) do {\
		MTL_QTOMR3_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR3_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR3_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR3_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR3_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR3_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR3_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR3_RgRd(v);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR3_Mask_7))<<7);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR3_Mask_26))<<26);\
		v = ((v & MTL_QTOMR3_TXQEN_Wr_Mask) | ((data & MTL_QTOMR3_TXQEN_Mask)<<2));\
		MTL_QTOMR3_RgWr(v);\
} while(0)

#define MTL_QTOMR3_TXQEN_UdfRd(data) do {\
		MTL_QTOMR3_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR3_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR3_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR3_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR3_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR3_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR3_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR3_RgRd(v);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR3_Mask_7))<<7);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR3_Mask_26))<<26);\
		v = ((v & MTL_QTOMR3_TTC_Wr_Mask) | ((data & MTL_QTOMR3_TTC_Mask)<<4));\
		MTL_QTOMR3_RgWr(v);\
} while(0)

#define MTL_QTOMR3_TTC_UdfRd(data) do {\
		MTL_QTOMR3_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR3_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR3_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR3_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR3_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR3_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR3_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR3_RgRd(v);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR3_Mask_7))<<7);\
		v = (v & (MTL_QTOMR3_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR3_Mask_26))<<26);\
		v = ((v & MTL_QTOMR3_TQS_Wr_Mask) | ((data & MTL_QTOMR3_TQS_Mask)<<16));\
		MTL_QTOMR3_RgWr(v);\
} while(0)

#define MTL_QTOMR3_TQS_UdfRd(data) do {\
		MTL_QTOMR3_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR3_TQS_Mask);\
} while(0)


#define MTL_QTOMR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd80))

#define MTL_QTOMR2_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR2_RgOffAddr);\
} while(0)

#define MTL_QTOMR2_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR2_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR2_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR2_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR2_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR2_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR2_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR2_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR2_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR2_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR2_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR2_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR2_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR2_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR2_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR2_RgRd(v);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR2_Mask_7))<<7);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR2_Mask_26))<<26);\
		v = ((v & MTL_QTOMR2_FTQ_Wr_Mask) | ((data & MTL_QTOMR2_FTQ_Mask)<<0));\
		MTL_QTOMR2_RgWr(v);\
} while(0)

#define MTL_QTOMR2_FTQ_UdfRd(data) do {\
		MTL_QTOMR2_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR2_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR2_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR2_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR2_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR2_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR2_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR2_RgRd(v);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR2_Mask_7))<<7);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR2_Mask_26))<<26);\
		v = ((v & MTL_QTOMR2_TSF_Wr_Mask) | ((data & MTL_QTOMR2_TSF_Mask)<<1));\
		MTL_QTOMR2_RgWr(v);\
} while(0)

#define MTL_QTOMR2_TSF_UdfRd(data) do {\
		MTL_QTOMR2_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR2_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR2_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR2_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR2_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR2_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR2_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR2_RgRd(v);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR2_Mask_7))<<7);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR2_Mask_26))<<26);\
		v = ((v & MTL_QTOMR2_TXQEN_Wr_Mask) | ((data & MTL_QTOMR2_TXQEN_Mask)<<2));\
		MTL_QTOMR2_RgWr(v);\
} while(0)

#define MTL_QTOMR2_TXQEN_UdfRd(data) do {\
		MTL_QTOMR2_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR2_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR2_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR2_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR2_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR2_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR2_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR2_RgRd(v);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR2_Mask_7))<<7);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR2_Mask_26))<<26);\
		v = ((v & MTL_QTOMR2_TTC_Wr_Mask) | ((data & MTL_QTOMR2_TTC_Mask)<<4));\
		MTL_QTOMR2_RgWr(v);\
} while(0)

#define MTL_QTOMR2_TTC_UdfRd(data) do {\
		MTL_QTOMR2_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR2_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR2_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR2_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR2_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR2_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR2_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR2_RgRd(v);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR2_Mask_7))<<7);\
		v = (v & (MTL_QTOMR2_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR2_Mask_26))<<26);\
		v = ((v & MTL_QTOMR2_TQS_Wr_Mask) | ((data & MTL_QTOMR2_TQS_Mask)<<16));\
		MTL_QTOMR2_RgWr(v);\
} while(0)

#define MTL_QTOMR2_TQS_UdfRd(data) do {\
		MTL_QTOMR2_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR2_TQS_Mask);\
} while(0)


#define MTL_QTOMR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd40))

#define MTL_QTOMR1_RgWr(data) do {\
		iowrite32(data, (void *)MTL_QTOMR1_RgOffAddr);\
} while(0)

#define MTL_QTOMR1_RgRd(data) do {\
		(data) = ioread32((void *)MTL_QTOMR1_RgOffAddr);\
} while(0)

/*#define  MTL_QTOMR1_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR1_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR1_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR1_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define  MTL_QTOMR1_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR1_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR1_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR1_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MTL_QTOMR1_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR1_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR1_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR1_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR1_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR1_RgRd(v);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR1_Mask_7))<<7);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR1_Mask_26))<<26);\
		v = ((v & MTL_QTOMR1_FTQ_Wr_Mask) | ((data & MTL_QTOMR1_FTQ_Mask)<<0));\
		MTL_QTOMR1_RgWr(v);\
} while(0)

#define MTL_QTOMR1_FTQ_UdfRd(data) do {\
		MTL_QTOMR1_RgRd(data);\
		data = ((data >> 0) & MTL_QTOMR1_FTQ_Mask);\
} while(0)

/*#define MTL_QTOMR1_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR1_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR1_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR1_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR1_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR1_RgRd(v);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR1_Mask_7))<<7);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR1_Mask_26))<<26);\
		v = ((v & MTL_QTOMR1_TSF_Wr_Mask) | ((data & MTL_QTOMR1_TSF_Mask)<<1));\
		MTL_QTOMR1_RgWr(v);\
} while(0)

#define MTL_QTOMR1_TSF_UdfRd(data) do {\
		MTL_QTOMR1_RgRd(data);\
		data = ((data >> 1) & MTL_QTOMR1_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR1_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR1_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR1_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR1_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR1_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR1_RgRd(v);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR1_Mask_7))<<7);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR1_Mask_26))<<26);\
		v = ((v & MTL_QTOMR1_TXQEN_Wr_Mask) | ((data & MTL_QTOMR1_TXQEN_Mask)<<2));\
		MTL_QTOMR1_RgWr(v);\
} while(0)

#define MTL_QTOMR1_TXQEN_UdfRd(data) do {\
		MTL_QTOMR1_RgRd(data);\
		data = ((data >> 2) & MTL_QTOMR1_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR1_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR1_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR1_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR1_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR1_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR1_RgRd(v);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR1_Mask_7))<<7);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR1_Mask_26))<<26);\
		v = ((v & MTL_QTOMR1_TTC_Wr_Mask) | ((data & MTL_QTOMR1_TTC_Mask)<<4));\
		MTL_QTOMR1_RgWr(v);\
} while(0)

#define MTL_QTOMR1_TTC_UdfRd(data) do {\
		MTL_QTOMR1_RgRd(data);\
		data = ((data >> 4) & MTL_QTOMR1_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR1_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR1_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR1_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR1_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR1_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_QTOMR1_RgRd(v);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR1_Mask_7))<<7);\
		v = (v & (MTL_QTOMR1_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR1_Mask_26))<<26);\
		v = ((v & MTL_QTOMR1_TQS_Wr_Mask) | ((data & MTL_QTOMR1_TQS_Mask)<<16));\
		MTL_QTOMR1_RgWr(v);\
} while(0)

#define MTL_QTOMR1_TQS_UdfRd(data) do {\
		MTL_QTOMR1_RgRd(data);\
		data = ((data >> 16) & MTL_QTOMR1_TQS_Mask);\
} while(0)


#define MAC_PMTCSR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc0))

#define MAC_PMTCSR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_PMTCSR_RgOffAddr);\
} while(0)

#define MAC_PMTCSR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_PMTCSR_RgOffAddr);\
} while(0)

/*#define  MAC_PMTCSR_Mask_27 (ULONG)(~(~0<<(4)))*/

#define  MAC_PMTCSR_Mask_27 (ULONG)(0xf)

/*#define MAC_PMTCSR_RES_Wr_Mask_27 (ULONG)(~((~(~0<<(4)))<<(27)))*/

#define MAC_PMTCSR_RES_Wr_Mask_27 (ULONG)(0x87ffffff)

/*#define  MAC_PMTCSR_Mask_10 (ULONG)(~(~0<<(14)))*/

#define  MAC_PMTCSR_Mask_10 (ULONG)(0x3fff)

/*#define MAC_PMTCSR_RES_Wr_Mask_10 (ULONG)(~((~(~0<<(14)))<<(10)))*/

#define MAC_PMTCSR_RES_Wr_Mask_10 (ULONG)(0xff0003ff)

/*#define  MAC_PMTCSR_Mask_7 (ULONG)(~(~0<<(2)))*/

#define  MAC_PMTCSR_Mask_7 (ULONG)(0x3)

/*#define MAC_PMTCSR_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(2)))<<(7)))*/

#define MAC_PMTCSR_RES_Wr_Mask_7 (ULONG)(0xfffffe7f)

/*#define  MAC_PMTCSR_Mask_3 (ULONG)(~(~0<<(2)))*/

#define  MAC_PMTCSR_Mask_3 (ULONG)(0x3)

/*#define MAC_PMTCSR_RES_Wr_Mask_3 (ULONG)(~((~(~0<<(2)))<<(3)))*/

#define MAC_PMTCSR_RES_Wr_Mask_3 (ULONG)(0xffffffe7)

/*#define MAC_PMTCSR_RWKFILTRST_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_RWKFILTRST_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_RWKFILTRST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_PMTCSR_RWKFILTRST_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_PMTCSR_RWKFILTRST_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_RWKFILTRST_Wr_Mask) | ((data & MAC_PMTCSR_RWKFILTRST_Mask)<<31));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_RWKFILTRST_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 31) & MAC_PMTCSR_RWKFILTRST_Mask);\
} while(0)

/*#define MAC_PMTCSR_RWKPTR_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_PMTCSR_RWKPTR_Mask (ULONG)(0x7)

#define MAC_PMTCSR_RWKPTR_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 24) & MAC_PMTCSR_RWKPTR_Mask);\
} while(0)

/*#define MAC_PMTCSR_GLBLUCAST_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_GLBLUCAST_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_GLBLUCAST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MAC_PMTCSR_GLBLUCAST_Wr_Mask (ULONG)(0xfffffdff)

#define MAC_PMTCSR_GLBLUCAST_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_GLBLUCAST_Wr_Mask) | ((data & MAC_PMTCSR_GLBLUCAST_Mask)<<9));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_GLBLUCAST_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 9) & MAC_PMTCSR_GLBLUCAST_Mask);\
} while(0)

/*#define MAC_PMTCSR_RWKPRCVD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_RWKPRCVD_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_RWKPRCVD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MAC_PMTCSR_RWKPRCVD_Wr_Mask (ULONG)(0xffffffbf)

#define MAC_PMTCSR_RWKPRCVD_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_RWKPRCVD_Wr_Mask) | ((data & MAC_PMTCSR_RWKPRCVD_Mask)<<6));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_RWKPRCVD_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 6) & MAC_PMTCSR_RWKPRCVD_Mask);\
} while(0)

/*#define MAC_PMTCSR_MGKPRCVD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_MGKPRCVD_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_MGKPRCVD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_PMTCSR_MGKPRCVD_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_PMTCSR_MGKPRCVD_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_MGKPRCVD_Wr_Mask) | ((data & MAC_PMTCSR_MGKPRCVD_Mask)<<5));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_MGKPRCVD_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 5) & MAC_PMTCSR_MGKPRCVD_Mask);\
} while(0)

/*#define MAC_PMTCSR_RWKPKTEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_RWKPKTEN_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_RWKPKTEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_PMTCSR_RWKPKTEN_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_PMTCSR_RWKPKTEN_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_RWKPKTEN_Wr_Mask) | ((data & MAC_PMTCSR_RWKPKTEN_Mask)<<2));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_RWKPKTEN_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 2) & MAC_PMTCSR_RWKPKTEN_Mask);\
} while(0)

/*#define MAC_PMTCSR_MGKPKTEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_MGKPKTEN_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_MGKPKTEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_PMTCSR_MGKPKTEN_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_PMTCSR_MGKPKTEN_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_MGKPKTEN_Wr_Mask) | ((data & MAC_PMTCSR_MGKPKTEN_Mask)<<1));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_MGKPKTEN_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 1) & MAC_PMTCSR_MGKPKTEN_Mask);\
} while(0)

/*#define MAC_PMTCSR_PWRDWN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PMTCSR_PWRDWN_Mask (ULONG)(0x1)

/*#define MAC_PMTCSR_PWRDWN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_PMTCSR_PWRDWN_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_PMTCSR_PWRDWN_UdfWr(data) do {\
		ULONG v;\
		MAC_PMTCSR_RgRd(v);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_27))|((( 0) & (MAC_PMTCSR_Mask_27))<<27);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_10))|((( 0) & (MAC_PMTCSR_Mask_10))<<10);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_7))|((( 0) & (MAC_PMTCSR_Mask_7))<<7);\
		v = (v & (MAC_PMTCSR_RES_Wr_Mask_3))|((( 0) & (MAC_PMTCSR_Mask_3))<<3);\
		v = ((v & MAC_PMTCSR_PWRDWN_Wr_Mask) | ((data & MAC_PMTCSR_PWRDWN_Mask)<<0));\
		MAC_PMTCSR_RgWr(v);\
} while(0)

#define MAC_PMTCSR_PWRDWN_UdfRd(data) do {\
		MAC_PMTCSR_RgRd(data);\
		data = ((data >> 0) & MAC_PMTCSR_PWRDWN_Mask);\
} while(0)


#define MMC_RXICMP_ERR_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x884))

#define MMC_RXICMP_ERR_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXICMP_ERR_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXICMP_ERR_OCTETS_RXICMP_ERR_OCTETS_UdfRd(data) do {\
		MMC_RXICMP_ERR_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXICMP_GD_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x880))

#define MMC_RXICMP_GD_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXICMP_GD_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXICMP_GD_OCTETS_RXICMP_GD_OCTETS_UdfRd(data) do {\
		MMC_RXICMP_GD_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXTCP_ERR_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x87c))

#define MMC_RXTCP_ERR_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXTCP_ERR_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXTCP_ERR_OCTETS_RXTCP_ERR_OCTETS_UdfRd(data) do {\
		MMC_RXTCP_ERR_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXTCP_GD_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x878))

#define MMC_RXTCP_GD_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXTCP_GD_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXTCP_GD_OCTETS_RXTCP_GD_OCTETS_UdfRd(data) do {\
		MMC_RXTCP_GD_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXUDP_ERR_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x874))

#define MMC_RXUDP_ERR_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXUDP_ERR_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXUDP_ERR_OCTETS_RXUDP_ERR_OCTETS_UdfRd(data) do {\
		MMC_RXUDP_ERR_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXUDP_GD_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x870))

#define MMC_RXUDP_GD_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXUDP_GD_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXUDP_GD_OCTETS_RXUDP_GD_OCTETS_UdfRd(data) do {\
		MMC_RXUDP_GD_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV6_NOPAY_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x86c))

#define MMC_RXIPV6_NOPAY_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV6_NOPAY_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV6_NOPAY_OCTETS_RXIPV6_NOPAY_OCTETS_UdfRd(data) do {\
		MMC_RXIPV6_NOPAY_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV6_HDRERR_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x868))

#define MMC_RXIPV6_HDRERR_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV6_HDRERR_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV6_HDRERR_OCTETS_RXIPV6_HDRERR_OCTETS_UdfRd(data) do {\
		MMC_RXIPV6_HDRERR_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV6_GD_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x864))

#define MMC_RXIPV6_GD_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV6_GD_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV6_GD_OCTETS_RXIPV6_GD_OCTETS_UdfRd(data) do {\
		MMC_RXIPV6_GD_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_UDSBL_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x860))

#define MMC_RXIPV4_UDSBL_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_UDSBL_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_UDSBL_OCTETS_RXIPV4_UDSBL_OCTETS_UdfRd(data) do {\
		MMC_RXIPV4_UDSBL_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_FRAG_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x85c))

#define MMC_RXIPV4_FRAG_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_FRAG_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_FRAG_OCTETS_RXIPV4_FRAG_OCTETS_UdfRd(data) do {\
		MMC_RXIPV4_FRAG_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_NOPAY_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x858))

#define MMC_RXIPV4_NOPAY_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_NOPAY_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_NOPAY_OCTETS_RXIPV4_NOPAY_OCTETS_UdfRd(data) do {\
		MMC_RXIPV4_NOPAY_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_HDRERR_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x854))

#define MMC_RXIPV4_HDRERR_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_HDRERR_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_HDRERR_OCTETS_RXIPV4_HDRERR_OCTETS_UdfRd(data) do {\
		MMC_RXIPV4_HDRERR_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_GD_OCTETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x850))

#define MMC_RXIPV4_GD_OCTETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_GD_OCTETS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_GD_OCTETS_RXIPV4_GD_OCTETS_UdfRd(data) do {\
		MMC_RXIPV4_GD_OCTETS_RgRd(data);\
} while(0)


#define MMC_RXICMP_ERR_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x844))

#define MMC_RXICMP_ERR_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXICMP_ERR_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXICMP_ERR_PKTS_RXICMP_ERR_PKTS_UdfRd(data) do {\
		MMC_RXICMP_ERR_PKTS_RgRd(data);\
} while(0)


#define MMC_RXICMP_GD_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x840))

#define MMC_RXICMP_GD_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXICMP_GD_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXICMP_GD_PKTS_RXICMP_GD_PKTS_UdfRd(data) do {\
		MMC_RXICMP_GD_PKTS_RgRd(data);\
} while(0)


#define MMC_RXTCP_ERR_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x83c))

#define MMC_RXTCP_ERR_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXTCP_ERR_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXTCP_ERR_PKTS_RXTCP_ERR_PKTS_UdfRd(data) do {\
		MMC_RXTCP_ERR_PKTS_RgRd(data);\
} while(0)


#define MMC_RXTCP_GD_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x838))

#define MMC_RXTCP_GD_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXTCP_GD_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXTCP_GD_PKTS_RXTCP_GD_PKTS_UdfRd(data) do {\
		MMC_RXTCP_GD_PKTS_RgRd(data);\
} while(0)


#define MMC_RXUDP_ERR_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x834))

#define MMC_RXUDP_ERR_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXUDP_ERR_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXUDP_ERR_PKTS_RXUDP_ERR_PKTS_UdfRd(data) do {\
		MMC_RXUDP_ERR_PKTS_RgRd(data);\
} while(0)


#define MMC_RXUDP_GD_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x830))

#define MMC_RXUDP_GD_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXUDP_GD_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXUDP_GD_PKTS_RXUDP_GD_PKTS_UdfRd(data) do {\
		MMC_RXUDP_GD_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV6_NOPAY_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x82c))

#define MMC_RXIPV6_NOPAY_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV6_NOPAY_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV6_NOPAY_PKTS_RXIPV6_NOPAY_PKTS_UdfRd(data) do {\
		MMC_RXIPV6_NOPAY_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV6_HDRERR_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x828))

#define MMC_RXIPV6_HDRERR_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV6_HDRERR_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV6_HDRERR_PKTS_RXIPV6_HDRERR_PKTS_UdfRd(data) do {\
		MMC_RXIPV6_HDRERR_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV6_GD_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x824))

#define MMC_RXIPV6_GD_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV6_GD_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV6_GD_PKTS_RXIPV6_GD_PKTS_UdfRd(data) do {\
		MMC_RXIPV6_GD_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_UBSBL_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x820))

#define MMC_RXIPV4_UBSBL_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_UBSBL_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_UBSBL_PKTS_RXIPV4_UBSBL_PKTS_UdfRd(data) do {\
		MMC_RXIPV4_UBSBL_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_FRAG_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x81c))

#define MMC_RXIPV4_FRAG_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_FRAG_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_FRAG_PKTS_RXIPV4_FRAG_PKTS_UdfRd(data) do {\
		MMC_RXIPV4_FRAG_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_NOPAY_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x818))

#define MMC_RXIPV4_NOPAY_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_NOPAY_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_NOPAY_PKTS_RXIPV4_NOPAY_PKTS_UdfRd(data) do {\
		MMC_RXIPV4_NOPAY_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_HDRERR_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x814))

#define MMC_RXIPV4_HDRERR_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_HDRERR_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_HDRERR_PKTS_RXIPV4_HDRERR_PKTS_UdfRd(data) do {\
		MMC_RXIPV4_HDRERR_PKTS_RgRd(data);\
} while(0)


#define MMC_RXIPV4_GD_PKTS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x810))

#define MMC_RXIPV4_GD_PKTS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXIPV4_GD_PKTS_RgOffAddr);\
} while(0)

#define MMC_RXIPV4_GD_PKTS_RXIPV4_GD_PKTS_UdfRd(data) do {\
		MMC_RXIPV4_GD_PKTS_RgRd(data);\
} while(0)


#define MMC_RXCTRLPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7e4))

#define MMC_RXCTRLPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXCTRLPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_RXCTRLPACKETS_G_RXCTRLPACKETS_G_UdfRd(data) do {\
		MMC_RXCTRLPACKETS_G_RgRd(data);\
} while(0)


#define MMC_RXRCVERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7e0))

#define MMC_RXRCVERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXRCVERROR_RgOffAddr);\
} while(0)

#define MMC_RXRCVERROR_RXRCVERROR_UdfRd(data) do {\
		MMC_RXRCVERROR_RgRd(data);\
} while(0)


#define MMC_RXWATCHDOGERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7dc))

#define MMC_RXWATCHDOGERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXWATCHDOGERROR_RgOffAddr);\
} while(0)

#define MMC_RXWATCHDOGERROR_RXWATCHDOGERROR_UdfRd(data) do {\
		MMC_RXWATCHDOGERROR_RgRd(data);\
} while(0)


#define MMC_RXVLANPACKETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7d8))

#define MMC_RXVLANPACKETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXVLANPACKETS_GB_RgOffAddr);\
} while(0)

#define MMC_RXVLANPACKETS_GB_RXVLANPACKETS_GB_UdfRd(data) do {\
		MMC_RXVLANPACKETS_GB_RgRd(data);\
} while(0)


#define MMC_RXFIFOOVERFLOW_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7d4))

#define MMC_RXFIFOOVERFLOW_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXFIFOOVERFLOW_RgOffAddr);\
} while(0)

#define MMC_RXFIFOOVERFLOW_RXFIFOOVERFLOW_UdfRd(data) do {\
		MMC_RXFIFOOVERFLOW_RgRd(data);\
} while(0)


#define MMC_RXPAUSEPACKETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7d0))

#define MMC_RXPAUSEPACKETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXPAUSEPACKETS_RgOffAddr);\
} while(0)

#define MMC_RXPAUSEPACKETS_RXPAUSEPACKETS_UdfRd(data) do {\
		MMC_RXPAUSEPACKETS_RgRd(data);\
} while(0)


#define MMC_RXOUTOFRANGETYPE_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7cc))

#define MMC_RXOUTOFRANGETYPE_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXOUTOFRANGETYPE_RgOffAddr);\
} while(0)

#define MMC_RXOUTOFRANGETYPE_RXOUTOFRANGETYPE_UdfRd(data) do {\
		MMC_RXOUTOFRANGETYPE_RgRd(data);\
} while(0)


#define MMC_RXLENGTHERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7c8))

#define MMC_RXLENGTHERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXLENGTHERROR_RgOffAddr);\
} while(0)

#define MMC_RXLENGTHERROR_RXLENGTHERROR_UdfRd(data) do {\
		MMC_RXLENGTHERROR_RgRd(data);\
} while(0)


#define MMC_RXUNICASTPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7c4))

#define MMC_RXUNICASTPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXUNICASTPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_RXUNICASTPACKETS_G_RXUNICASTPACKETS_G_UdfRd(data) do {\
		MMC_RXUNICASTPACKETS_G_RgRd(data);\
} while(0)


#define MMC_RX1024TOMAXOCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7c0))

#define MMC_RX1024TOMAXOCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RX1024TOMAXOCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_RX1024TOMAXOCTETS_GB_RX1024TOMAXOCTETS_GB_UdfRd(data) do {\
		MMC_RX1024TOMAXOCTETS_GB_RgRd(data);\
} while(0)


#define MMC_RX512TO1023OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7bc))

#define MMC_RX512TO1023OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RX512TO1023OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_RX512TO1023OCTETS_GB_RX512TO1023OCTETS_GB_UdfRd(data) do {\
		MMC_RX512TO1023OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_RX256TO511OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7b8))

#define MMC_RX256TO511OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RX256TO511OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_RX256TO511OCTETS_GB_RX256TO511OCTETS_GB_UdfRd(data) do {\
		MMC_RX256TO511OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_RX128TO255OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7b4))

#define MMC_RX128TO255OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RX128TO255OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_RX128TO255OCTETS_GB_RX128TO255OCTETS_GB_UdfRd(data) do {\
		MMC_RX128TO255OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_RX65TO127OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7b0))

#define MMC_RX65TO127OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RX65TO127OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_RX65TO127OCTETS_GB_RX65TO127OCTETS_GB_UdfRd(data) do {\
		MMC_RX65TO127OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_RX64OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7ac))

#define MMC_RX64OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RX64OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_RX64OCTETS_GB_RX64OCTETS_GB_UdfRd(data) do {\
		MMC_RX64OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_RXOVERSIZE_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7a8))

#define MMC_RXOVERSIZE_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXOVERSIZE_G_RgOffAddr);\
} while(0)

#define MMC_RXOVERSIZE_G_RXOVERSIZE_G_UdfRd(data) do {\
		MMC_RXOVERSIZE_G_RgRd(data);\
} while(0)


#define MMC_RXUNDERSIZE_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7a4))

#define MMC_RXUNDERSIZE_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXUNDERSIZE_G_RgOffAddr);\
} while(0)

#define MMC_RXUNDERSIZE_G_RXUNDERSIZE_G_UdfRd(data) do {\
		MMC_RXUNDERSIZE_G_RgRd(data);\
} while(0)


#define MMC_RXJABBERERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7a0))

#define MMC_RXJABBERERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXJABBERERROR_RgOffAddr);\
} while(0)

#define MMC_RXJABBERERROR_RXJABBERERROR_UdfRd(data) do {\
		MMC_RXJABBERERROR_RgRd(data);\
} while(0)


#define MMC_RXRUNTERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x79c))

#define MMC_RXRUNTERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXRUNTERROR_RgOffAddr);\
} while(0)

#define MMC_RXRUNTERROR_RXRUNTERROR_UdfRd(data) do {\
		MMC_RXRUNTERROR_RgRd(data);\
} while(0)


#define MMC_RXALIGNMENTERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x798))

#define MMC_RXALIGNMENTERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXALIGNMENTERROR_RgOffAddr);\
} while(0)

#define MMC_RXALIGNMENTERROR_RXALIGNMENTERROR_UdfRd(data) do {\
		MMC_RXALIGNMENTERROR_RgRd(data);\
} while(0)


#define MMC_RXCRCERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x794))

#define MMC_RXCRCERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXCRCERROR_RgOffAddr);\
} while(0)

#define MMC_RXCRCERROR_RXCRCERROR_UdfRd(data) do {\
		MMC_RXCRCERROR_RgRd(data);\
} while(0)


#define MMC_RXMULTICASTPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x790))

#define MMC_RXMULTICASTPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXMULTICASTPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_RXMULTICASTPACKETS_G_RXMULTICASTPACKETS_G_UdfRd(data) do {\
		MMC_RXMULTICASTPACKETS_G_RgRd(data);\
} while(0)


#define MMC_RXBROADCASTPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x78c))

#define MMC_RXBROADCASTPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXBROADCASTPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_RXBROADCASTPACKETS_G_RXBROADCASTPACKETS_G_UdfRd(data) do {\
		MMC_RXBROADCASTPACKETS_G_RgRd(data);\
} while(0)


#define MMC_RXOCTETCOUNT_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x788))

#define MMC_RXOCTETCOUNT_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXOCTETCOUNT_G_RgOffAddr);\
} while(0)

#define MMC_RXOCTETCOUNT_G_RXOCTETCOUNT_G_UdfRd(data) do {\
		MMC_RXOCTETCOUNT_G_RgRd(data);\
} while(0)


#define MMC_RXOCTETCOUNT_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x784))

#define MMC_RXOCTETCOUNT_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXOCTETCOUNT_GB_RgOffAddr);\
} while(0)

#define MMC_RXOCTETCOUNT_GB_RXOCTETCOUNT_GB_UdfRd(data) do {\
		MMC_RXOCTETCOUNT_GB_RgRd(data);\
} while(0)


#define MMC_RXPACKETCOUNT_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x780))

#define MMC_RXPACKETCOUNT_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_RXPACKETCOUNT_GB_RgOffAddr);\
} while(0)

#define MMC_RXPACKETCOUNT_GB_RXPACKETCOUNT_GB_UdfRd(data) do {\
		MMC_RXPACKETCOUNT_GB_RgRd(data);\
} while(0)


#define MMC_TXOVERSIZE_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x778))

#define MMC_TXOVERSIZE_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXOVERSIZE_G_RgOffAddr);\
} while(0)

#define MMC_TXOVERSIZE_G_TXOVERSIZE_G_UdfRd(data) do {\
		MMC_TXOVERSIZE_G_RgRd(data);\
} while(0)


#define MMC_TXVLANPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x774))

#define MMC_TXVLANPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXVLANPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_TXVLANPACKETS_G_TXVLANPACKETS_G_UdfRd(data) do {\
		MMC_TXVLANPACKETS_G_RgRd(data);\
} while(0)


#define MMC_TXPAUSEPACKETS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x770))

#define MMC_TXPAUSEPACKETS_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXPAUSEPACKETS_RgOffAddr);\
} while(0)

#define MMC_TXPAUSEPACKETS_TXPAUSEPACKETS_UdfRd(data) do {\
		MMC_TXPAUSEPACKETS_RgRd(data);\
} while(0)


#define MMC_TXEXCESSDEF_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x76c))

#define MMC_TXEXCESSDEF_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXEXCESSDEF_RgOffAddr);\
} while(0)

#define MMC_TXEXCESSDEF_TXEXCESSDEF_UdfRd(data) do {\
		MMC_TXEXCESSDEF_RgRd(data);\
} while(0)


#define MMC_TXPACKETSCOUNT_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x768))

#define MMC_TXPACKETSCOUNT_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXPACKETSCOUNT_G_RgOffAddr);\
} while(0)

#define MMC_TXPACKETSCOUNT_G_TXPACKETSCOUNT_G_UdfRd(data) do {\
		MMC_TXPACKETSCOUNT_G_RgRd(data);\
} while(0)


#define MMC_TXOCTETCOUNT_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x764))

#define MMC_TXOCTETCOUNT_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXOCTETCOUNT_G_RgOffAddr);\
} while(0)

#define MMC_TXOCTETCOUNT_G_TXOCTETCOUNT_G_UdfRd(data) do {\
		MMC_TXOCTETCOUNT_G_RgRd(data);\
} while(0)


#define MMC_TXCARRIERERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x760))

#define MMC_TXCARRIERERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXCARRIERERROR_RgOffAddr);\
} while(0)

#define MMC_TXCARRIERERROR_TXCARRIERERROR_UdfRd(data) do {\
		MMC_TXCARRIERERROR_RgRd(data);\
} while(0)


#define MMC_TXEXESSCOL_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x75c))

#define MMC_TXEXESSCOL_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXEXESSCOL_RgOffAddr);\
} while(0)

#define MMC_TXEXESSCOL_TXEXESSCOL_UdfRd(data) do {\
		MMC_TXEXESSCOL_RgRd(data);\
} while(0)


#define MMC_TXLATECOL_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x758))

#define MMC_TXLATECOL_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXLATECOL_RgOffAddr);\
} while(0)

#define MMC_TXLATECOL_TXLATECOL_UdfRd(data) do {\
		MMC_TXLATECOL_RgRd(data);\
} while(0)


#define MMC_TXDEFERRED_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x754))

#define MMC_TXDEFERRED_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXDEFERRED_RgOffAddr);\
} while(0)

#define MMC_TXDEFERRED_TXDEFERRED_UdfRd(data) do {\
		MMC_TXDEFERRED_RgRd(data);\
} while(0)


#define MMC_TXMULTICOL_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x750))

#define MMC_TXMULTICOL_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXMULTICOL_G_RgOffAddr);\
} while(0)

#define MMC_TXMULTICOL_G_TXMULTICOL_G_UdfRd(data) do {\
		MMC_TXMULTICOL_G_RgRd(data);\
} while(0)


#define MMC_TXSINGLECOL_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x74c))

#define MMC_TXSINGLECOL_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXSINGLECOL_G_RgOffAddr);\
} while(0)

#define MMC_TXSINGLECOL_G_TXSINGLECOL_G_UdfRd(data) do {\
		MMC_TXSINGLECOL_G_RgRd(data);\
} while(0)


#define MMC_TXUNDERFLOWERROR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x748))

#define MMC_TXUNDERFLOWERROR_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXUNDERFLOWERROR_RgOffAddr);\
} while(0)

#define MMC_TXUNDERFLOWERROR_TXUNDERFLOWERROR_UdfRd(data) do {\
		MMC_TXUNDERFLOWERROR_RgRd(data);\
} while(0)


#define MMC_TXBROADCASTPACKETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x744))

#define MMC_TXBROADCASTPACKETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXBROADCASTPACKETS_GB_RgOffAddr);\
} while(0)

#define MMC_TXBROADCASTPACKETS_GB_TXBROADCASTPACKETS_GB_UdfRd(data) do {\
		MMC_TXBROADCASTPACKETS_GB_RgRd(data);\
} while(0)


#define MMC_TXMULTICASTPACKETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x740))

#define MMC_TXMULTICASTPACKETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXMULTICASTPACKETS_GB_RgOffAddr);\
} while(0)

#define MMC_TXMULTICASTPACKETS_GB_TXMULTICASTPACKETS_GB_UdfRd(data) do {\
		MMC_TXMULTICASTPACKETS_GB_RgRd(data);\
} while(0)


#define MMC_TXUNICASTPACKETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x73c))

#define MMC_TXUNICASTPACKETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXUNICASTPACKETS_GB_RgOffAddr);\
} while(0)

#define MMC_TXUNICASTPACKETS_GB_TXUNICASTPACKETS_GB_UdfRd(data) do {\
		MMC_TXUNICASTPACKETS_GB_RgRd(data);\
} while(0)


#define MMC_TX1024TOMAXOCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x738))

#define MMC_TX1024TOMAXOCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TX1024TOMAXOCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_TX1024TOMAXOCTETS_GB_TX1024TOMAXOCTETS_GB_UdfRd(data) do {\
		MMC_TX1024TOMAXOCTETS_GB_RgRd(data);\
} while(0)


#define MMC_TX512TO1023OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x734))

#define MMC_TX512TO1023OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TX512TO1023OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_TX512TO1023OCTETS_GB_TX512TO1023OCTETS_GB_UdfRd(data) do {\
		MMC_TX512TO1023OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_TX256TO511OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x730))

#define MMC_TX256TO511OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TX256TO511OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_TX256TO511OCTETS_GB_TX256TO511OCTETS_GB_UdfRd(data) do {\
		MMC_TX256TO511OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_TX128TO255OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x72c))

#define MMC_TX128TO255OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TX128TO255OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_TX128TO255OCTETS_GB_TX128TO255OCTETS_GB_UdfRd(data) do {\
		MMC_TX128TO255OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_TX65TO127OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x728))

#define MMC_TX65TO127OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TX65TO127OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_TX65TO127OCTETS_GB_TX65TO127OCTETS_GB_UdfRd(data) do {\
		MMC_TX65TO127OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_TX64OCTETS_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x724))

#define MMC_TX64OCTETS_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TX64OCTETS_GB_RgOffAddr);\
} while(0)

#define MMC_TX64OCTETS_GB_TX64OCTETS_GB_UdfRd(data) do {\
		MMC_TX64OCTETS_GB_RgRd(data);\
} while(0)


#define MMC_TXMULTICASTPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x720))

#define MMC_TXMULTICASTPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXMULTICASTPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_TXMULTICASTPACKETS_G_TXMULTICASTPACKETS_G_UdfRd(data) do {\
		MMC_TXMULTICASTPACKETS_G_RgRd(data);\
} while(0)


#define MMC_TXBROADCASTPACKETS_G_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x71c))

#define MMC_TXBROADCASTPACKETS_G_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXBROADCASTPACKETS_G_RgOffAddr);\
} while(0)

#define MMC_TXBROADCASTPACKETS_G_TXBROADCASTPACKETS_G_UdfRd(data) do {\
		MMC_TXBROADCASTPACKETS_G_RgRd(data);\
} while(0)


#define MMC_TXPACKETCOUNT_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x718))

#define MMC_TXPACKETCOUNT_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXPACKETCOUNT_GB_RgOffAddr);\
} while(0)

#define MMC_TXPACKETCOUNT_GB_TXPACKETCOUNT_GB_UdfRd(data) do {\
		MMC_TXPACKETCOUNT_GB_RgRd(data);\
} while(0)


#define MMC_TXOCTETCOUNT_GB_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x714))

#define MMC_TXOCTETCOUNT_GB_RgRd(data) do {\
		(data) = ioread32((void *)MMC_TXOCTETCOUNT_GB_RgOffAddr);\
} while(0)

#define MMC_TXOCTETCOUNT_GB_TXOCTETCOUNT_GB_UdfRd(data) do {\
		MMC_TXOCTETCOUNT_GB_RgRd(data);\
} while(0)


#define MMC_IPC_INTR_RX_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x808))

#define MMC_IPC_INTR_RX_RgRd(data) do {\
		(data) = ioread32((void *)MMC_IPC_INTR_RX_RgOffAddr);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXICMP_ERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXICMP_ERR_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXICMP_ERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 29) & MMC_IPC_INTR_RX_RXICMP_ERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXICMP_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXICMP_GD_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXICMP_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 28) & MMC_IPC_INTR_RX_RXICMP_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXTCP_ERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXTCP_ERR_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXTCP_ERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 27) & MMC_IPC_INTR_RX_RXTCP_ERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXTCP_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXTCP_GD_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXTCP_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 26) & MMC_IPC_INTR_RX_RXTCP_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXUDP_ERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXUDP_ERR_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXUDP_ERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 25) & MMC_IPC_INTR_RX_RXUDP_ERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXUDP_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXUDP_GD_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXUDP_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 24) & MMC_IPC_INTR_RX_RXUDP_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV6_NOPAY_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV6_NOPAY_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV6_NOPAY_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 23) & MMC_IPC_INTR_RX_RXIPV6_NOPAY_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV6_HDRERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV6_HDRERR_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV6_HDRERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 22) & MMC_IPC_INTR_RX_RXIPV6_HDRERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV6_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV6_GD_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV6_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 21) & MMC_IPC_INTR_RX_RXIPV6_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_UDSBL_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_UDSBL_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_UDSBL_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 20) & MMC_IPC_INTR_RX_RXIPV4_UDSBL_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_FRAG_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_FRAG_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_FRAG_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 19) & MMC_IPC_INTR_RX_RXIPV4_FRAG_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_NOPAY_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_NOPAY_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_NOPAY_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 18) & MMC_IPC_INTR_RX_RXIPV4_NOPAY_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_HDRERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_HDRERR_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_HDRERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 17) & MMC_IPC_INTR_RX_RXIPV4_HDRERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_GD_OCTETS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 16) & MMC_IPC_INTR_RX_RXIPV4_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXICMP_ERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXICMP_ERR_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXICMP_ERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 13) & MMC_IPC_INTR_RX_RXICMP_ERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXICMP_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXICMP_GD_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXICMP_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 12) & MMC_IPC_INTR_RX_RXICMP_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXTCP_ERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXTCP_ERR_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXTCP_ERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 11) & MMC_IPC_INTR_RX_RXTCP_ERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXTCP_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXTCP_GD_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXTCP_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 10) & MMC_IPC_INTR_RX_RXTCP_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXUDP_ERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXUDP_ERR_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXUDP_ERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 9) & MMC_IPC_INTR_RX_RXUDP_ERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXUDP_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXUDP_GD_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXUDP_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 8) & MMC_IPC_INTR_RX_RXUDP_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV6_NOPAY_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV6_NOPAY_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV6_NOPAY_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 7) & MMC_IPC_INTR_RX_RXIPV6_NOPAY_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV6_HDRERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV6_HDRERR_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV6_HDRERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 6) & MMC_IPC_INTR_RX_RXIPV6_HDRERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV6_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV6_GD_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV6_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 5) & MMC_IPC_INTR_RX_RXIPV6_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_UDSBL_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_UDSBL_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_UDSBL_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 4) & MMC_IPC_INTR_RX_RXIPV4_UDSBL_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_FRAG_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_FRAG_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_FRAG_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 3) & MMC_IPC_INTR_RX_RXIPV4_FRAG_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_NOPAY_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_NOPAY_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_NOPAY_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 2) & MMC_IPC_INTR_RX_RXIPV4_NOPAY_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_HDRERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_HDRERR_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_HDRERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 1) & MMC_IPC_INTR_RX_RXIPV4_HDRERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_RX_RXIPV4_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_RX_RXIPV4_GD_FRMS_Mask (ULONG)(0x1)

#define MMC_IPC_INTR_RX_RXIPV4_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_RX_RgRd(data);\
		data = ((data >> 0) & MMC_IPC_INTR_RX_RXIPV4_GD_FRMS_Mask);\
} while(0)


#define MMC_IPC_INTR_MASK_RX_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x800))

#define MMC_IPC_INTR_MASK_RX_RgWr(data) do {\
		iowrite32(data, (void *)MMC_IPC_INTR_MASK_RX_RgOffAddr);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RgRd(data) do {\
		(data) = ioread32((void *)MMC_IPC_INTR_MASK_RX_RgOffAddr);\
} while(0)

/*#define  MMC_IPC_INTR_MASK_RX_Mask_30 (ULONG)(~(~0<<(2)))*/

#define  MMC_IPC_INTR_MASK_RX_Mask_30 (ULONG)(0x3)

/*#define MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30 (ULONG)(~((~(~0<<(2)))<<(30)))*/

#define MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30 (ULONG)(0x3fffffff)

/*#define  MMC_IPC_INTR_MASK_RX_Mask_14 (ULONG)(~(~0<<(2)))*/

#define  MMC_IPC_INTR_MASK_RX_Mask_14 (ULONG)(0x3)

/*#define MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(2)))<<(14)))*/

#define MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14 (ULONG)(0xffff3fff)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (29)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Wr_Mask (ULONG)(0xdfffffff)

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Mask)<<29));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 29) & MMC_IPC_INTR_MASK_RX_RXICMP_ERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (28)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Wr_Mask (ULONG)(0xefffffff)

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Mask)<<28));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 28) & MMC_IPC_INTR_MASK_RX_RXICMP_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Wr_Mask (ULONG)(0xf7ffffff)

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Mask)<<27));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 27) & MMC_IPC_INTR_MASK_RX_RXTCP_ERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (26)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Wr_Mask (ULONG)(0xfbffffff)

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Mask)<<26));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 26) & MMC_IPC_INTR_MASK_RX_RXTCP_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (25)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Wr_Mask (ULONG)(0xfdffffff)

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Mask)<<25));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 25) & MMC_IPC_INTR_MASK_RX_RXUDP_ERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Wr_Mask (ULONG)(0xfeffffff)

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Mask)<<24));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 24) & MMC_IPC_INTR_MASK_RX_RXUDP_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (23)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Wr_Mask (ULONG)(0xff7fffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Mask)<<23));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 23) & MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (22)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Wr_Mask (ULONG)(0xffbfffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Mask)<<22));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 22) & MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Wr_Mask (ULONG)(0xffdfffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Mask)<<21));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 21) & MMC_IPC_INTR_MASK_RX_RXIPV6_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Wr_Mask (ULONG)(0xffefffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Mask)<<20));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 20) & MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Wr_Mask (ULONG)(0xfff7ffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Mask)<<19));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 19) & MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Wr_Mask (ULONG)(0xfffbffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Mask)<<18));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 18) & MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Wr_Mask (ULONG)(0xfffdffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Mask)<<17));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 17) & MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Wr_Mask (ULONG)(0xfffeffff)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Mask)<<16));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 16) & MMC_IPC_INTR_MASK_RX_RXIPV4_GD_OCTETS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Wr_Mask (ULONG)(0xffffdfff)

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Mask)<<13));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 13) & MMC_IPC_INTR_MASK_RX_RXICMP_ERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Wr_Mask (ULONG)(0xffffefff)

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Mask)<<12));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 12) & MMC_IPC_INTR_MASK_RX_RXICMP_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Wr_Mask (ULONG)(0xfffff7ff)

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Mask)<<11));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 11) & MMC_IPC_INTR_MASK_RX_RXTCP_ERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Wr_Mask (ULONG)(0xfffffbff)

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Mask)<<10));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 10) & MMC_IPC_INTR_MASK_RX_RXTCP_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Wr_Mask (ULONG)(0xfffffdff)

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Mask)<<9));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 9) & MMC_IPC_INTR_MASK_RX_RXUDP_ERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Wr_Mask (ULONG)(0xfffffeff)

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Mask)<<8));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 8) & MMC_IPC_INTR_MASK_RX_RXUDP_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Wr_Mask (ULONG)(0xffffff7f)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Mask)<<7));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 7) & MMC_IPC_INTR_MASK_RX_RXIPV6_NOPAY_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Wr_Mask (ULONG)(0xffffffbf)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Mask)<<6));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 6) & MMC_IPC_INTR_MASK_RX_RXIPV6_HDRERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Wr_Mask (ULONG)(0xffffffdf)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Mask)<<5));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 5) & MMC_IPC_INTR_MASK_RX_RXIPV6_GD_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Wr_Mask (ULONG)(0xffffffef)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Mask)<<4));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 4) & MMC_IPC_INTR_MASK_RX_RXIPV4_UDSBL_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Wr_Mask (ULONG)(0xfffffff7)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Mask)<<3));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 3) & MMC_IPC_INTR_MASK_RX_RXIPV4_FRAG_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Wr_Mask (ULONG)(0xfffffffb)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Mask)<<2));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 2) & MMC_IPC_INTR_MASK_RX_RXIPV4_NOPAY_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Wr_Mask (ULONG)(0xfffffffd)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Mask)<<1));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 1) & MMC_IPC_INTR_MASK_RX_RXIPV4_HDRERR_FRMS_Mask);\
} while(0)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Mask (ULONG)(0x1)

/*#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Wr_Mask (ULONG)(0xfffffffe)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_UdfWr(data) do {\
		ULONG v;\
		MMC_IPC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_30))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_30))<<30);\
		v = (v & (MMC_IPC_INTR_MASK_RX_RES_Wr_Mask_14))|((( 0) & (MMC_IPC_INTR_MASK_RX_Mask_14))<<14);\
		v = ((v & MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Wr_Mask) | ((data & MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Mask)<<0));\
		MMC_IPC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_UdfRd(data) do {\
		MMC_IPC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 0) & MMC_IPC_INTR_MASK_RX_RXIPV4_GD_FRMS_Mask);\
} while(0)


#define MMC_INTR_MASK_TX_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x710))

#define MMC_INTR_MASK_TX_RgWr(data) do {\
		iowrite32(data, (void *)MMC_INTR_MASK_TX_RgOffAddr);\
} while(0)

#define MMC_INTR_MASK_TX_RgRd(data) do {\
		(data) = ioread32((void *)MMC_INTR_MASK_TX_RgOffAddr);\
} while(0)

/*#define  MMC_INTR_MASK_TX_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MMC_INTR_MASK_TX_Mask_25 (ULONG)(0x7f)

/*#define MMC_INTR_MASK_TX_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MMC_INTR_MASK_TX_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define MMC_INTR_MASK_TX_TXVLANFRAMES_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXVLANFRAMES_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXVLANFRAMES_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MMC_INTR_MASK_TX_TXVLANFRAMES_G_Wr_Mask (ULONG)(0xfeffffff)

#define MMC_INTR_MASK_TX_TXVLANFRAMES_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXVLANFRAMES_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXVLANFRAMES_G_Mask)<<24));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXVLANFRAMES_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 24) & MMC_INTR_MASK_TX_TXVLANFRAMES_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXPAUSEFRAMES_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXPAUSEFRAMES_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXPAUSEFRAMES_Wr_Mask (ULONG)(~((~(~0 << (1))) << (23)))*/

#define MMC_INTR_MASK_TX_TXPAUSEFRAMES_Wr_Mask (ULONG)(0xff7fffff)

#define MMC_INTR_MASK_TX_TXPAUSEFRAMES_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXPAUSEFRAMES_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXPAUSEFRAMES_Mask)<<23));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXPAUSEFRAMES_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 23) & MMC_INTR_MASK_TX_TXPAUSEFRAMES_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXEXCESSDEF_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXEXCESSDEF_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXEXCESSDEF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (22)))*/

#define MMC_INTR_MASK_TX_TXEXCESSDEF_Wr_Mask (ULONG)(0xffbfffff)

#define MMC_INTR_MASK_TX_TXEXCESSDEF_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXEXCESSDEF_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXEXCESSDEF_Mask)<<22));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXEXCESSDEF_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 22) & MMC_INTR_MASK_TX_TXEXCESSDEF_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Wr_Mask (ULONG)(0xffdfffff)

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Mask)<<21));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 21) & MMC_INTR_MASK_TX_TXFRAMECOUNT_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Wr_Mask (ULONG)(0xffefffff)

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Mask)<<20));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 20) & MMC_INTR_MASK_TX_TXOCTETCOUNT_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXCARRIERERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXCARRIERERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXCARRIERERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MMC_INTR_MASK_TX_TXCARRIERERROR_Wr_Mask (ULONG)(0xfff7ffff)

#define MMC_INTR_MASK_TX_TXCARRIERERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXCARRIERERROR_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXCARRIERERROR_Mask)<<19));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXCARRIERERROR_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 19) & MMC_INTR_MASK_TX_TXCARRIERERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXEXESSCOL_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXEXESSCOL_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXEXESSCOL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MMC_INTR_MASK_TX_TXEXESSCOL_Wr_Mask (ULONG)(0xfffbffff)

#define MMC_INTR_MASK_TX_TXEXESSCOL_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXEXESSCOL_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXEXESSCOL_Mask)<<18));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXEXESSCOL_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 18) & MMC_INTR_MASK_TX_TXEXESSCOL_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXLATECOL_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXLATECOL_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXLATECOL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MMC_INTR_MASK_TX_TXLATECOL_Wr_Mask (ULONG)(0xfffdffff)

#define MMC_INTR_MASK_TX_TXLATECOL_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXLATECOL_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXLATECOL_Mask)<<17));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXLATECOL_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 17) & MMC_INTR_MASK_TX_TXLATECOL_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXDEFERRED_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXDEFERRED_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXDEFERRED_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MMC_INTR_MASK_TX_TXDEFERRED_Wr_Mask (ULONG)(0xfffeffff)

#define MMC_INTR_MASK_TX_TXDEFERRED_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXDEFERRED_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXDEFERRED_Mask)<<16));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXDEFERRED_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 16) & MMC_INTR_MASK_TX_TXDEFERRED_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXMULTICOL_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXMULTICOL_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXMULTICOL_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MMC_INTR_MASK_TX_TXMULTICOL_G_Wr_Mask (ULONG)(0xffff7fff)

#define MMC_INTR_MASK_TX_TXMULTICOL_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXMULTICOL_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXMULTICOL_G_Mask)<<15));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXMULTICOL_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 15) & MMC_INTR_MASK_TX_TXMULTICOL_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXSINGLECOL_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXSINGLECOL_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXSINGLECOL_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MMC_INTR_MASK_TX_TXSINGLECOL_G_Wr_Mask (ULONG)(0xffffbfff)

#define MMC_INTR_MASK_TX_TXSINGLECOL_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXSINGLECOL_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXSINGLECOL_G_Mask)<<14));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXSINGLECOL_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 14) & MMC_INTR_MASK_TX_TXSINGLECOL_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Wr_Mask (ULONG)(0xffffdfff)

#define MMC_INTR_MASK_TX_TXUNDERFLOWERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Mask)<<13));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXUNDERFLOWERROR_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 13) & MMC_INTR_MASK_TX_TXUNDERFLOWERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Wr_Mask (ULONG)(0xffffefff)

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Mask)<<12));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 12) & MMC_INTR_MASK_TX_TXBROADCASTFRAMES_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Wr_Mask (ULONG)(0xfffff7ff)

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Mask)<<11));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 11) & MMC_INTR_MASK_TX_TXMULTICASTFRAMES_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Wr_Mask (ULONG)(0xfffffbff)

#define MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Mask)<<10));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 10) & MMC_INTR_MASK_TX_TXUNICASTFRAMES_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Wr_Mask (ULONG)(0xfffffdff)

#define MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Mask)<<9));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 9) & MMC_INTR_MASK_TX_TX1024TOMAXOCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Wr_Mask (ULONG)(0xfffffeff)

#define MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Mask)<<8));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 8) & MMC_INTR_MASK_TX_TX512TO1023OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Wr_Mask (ULONG)(0xffffff7f)

#define MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Mask)<<7));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 7) & MMC_INTR_MASK_TX_TX256TO511OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Wr_Mask (ULONG)(0xffffffbf)

#define MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Mask)<<6));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 6) & MMC_INTR_MASK_TX_TX128TO255OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Wr_Mask (ULONG)(0xffffffdf)

#define MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Mask)<<5));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 5) & MMC_INTR_MASK_TX_TX65TO127OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TX64OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TX64OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TX64OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MMC_INTR_MASK_TX_TX64OCTETS_GB_Wr_Mask (ULONG)(0xffffffef)

#define MMC_INTR_MASK_TX_TX64OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TX64OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TX64OCTETS_GB_Mask)<<4));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TX64OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 4) & MMC_INTR_MASK_TX_TX64OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Wr_Mask (ULONG)(0xfffffff7)

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Mask)<<3));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 3) & MMC_INTR_MASK_TX_TXMULTICASTFRAMES_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Wr_Mask (ULONG)(0xfffffffb)

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Mask)<<2));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 2) & MMC_INTR_MASK_TX_TXBROADCASTFRAMES_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Wr_Mask (ULONG)(0xfffffffd)

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Mask)<<1));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 1) & MMC_INTR_MASK_TX_TXFRAMECOUNT_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Wr_Mask (ULONG)(0xfffffffe)

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_TX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_TX_RES_Wr_Mask_25))|((( 0) & (MMC_INTR_MASK_TX_Mask_25))<<25);\
		v = ((v & MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Wr_Mask) | ((data & MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Mask)<<0));\
		MMC_INTR_MASK_TX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_UdfRd(data) do {\
		MMC_INTR_MASK_TX_RgRd(data);\
		data = ((data >> 0) & MMC_INTR_MASK_TX_TXOCTETCOUNT_GB_Mask);\
} while(0)


#define MMC_INTR_MASK_RX_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x70c))

#define MMC_INTR_MASK_RX_RgWr(data) do {\
		iowrite32(data, (void *)MMC_INTR_MASK_RX_RgOffAddr);\
} while(0)

#define MMC_INTR_MASK_RX_RgRd(data) do {\
		(data) = ioread32((void *)MMC_INTR_MASK_RX_RgOffAddr);\
} while(0)

/*#define  MMC_INTR_MASK_RX_Mask_24 (ULONG)(~(~0<<(8)))*/

#define  MMC_INTR_MASK_RX_Mask_24 (ULONG)(0xff)

/*#define MMC_INTR_MASK_RX_RES_Wr_Mask_24 (ULONG)(~((~(~0<<(8)))<<(24)))*/

#define MMC_INTR_MASK_RX_RES_Wr_Mask_24 (ULONG)(0xffffff)

/*#define MMC_INTR_MASK_RX_RXWATCHDOG_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXWATCHDOG_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXWATCHDOG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (23)))*/

#define MMC_INTR_MASK_RX_RXWATCHDOG_Wr_Mask (ULONG)(0xff7fffff)

#define MMC_INTR_MASK_RX_RXWATCHDOG_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXWATCHDOG_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXWATCHDOG_Mask)<<23));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXWATCHDOG_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 23) & MMC_INTR_MASK_RX_RXWATCHDOG_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (22)))*/

#define MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Wr_Mask (ULONG)(0xffbfffff)

#define MMC_INTR_MASK_RX_RXVLANFRAMES_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Mask)<<22));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXVLANFRAMES_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 22) & MMC_INTR_MASK_RX_RXVLANFRAMES_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Wr_Mask (ULONG)(0xffdfffff)

#define MMC_INTR_MASK_RX_RXFIFOOVERFLOW_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Mask)<<21));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXFIFOOVERFLOW_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 21) & MMC_INTR_MASK_RX_RXFIFOOVERFLOW_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXPAUSEFRAMES_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXPAUSEFRAMES_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXPAUSEFRAMES_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MMC_INTR_MASK_RX_RXPAUSEFRAMES_Wr_Mask (ULONG)(0xffefffff)

#define MMC_INTR_MASK_RX_RXPAUSEFRAMES_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXPAUSEFRAMES_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXPAUSEFRAMES_Mask)<<20));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXPAUSEFRAMES_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 20) & MMC_INTR_MASK_RX_RXPAUSEFRAMES_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Wr_Mask (ULONG)(0xfff7ffff)

#define MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Mask)<<19));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 19) & MMC_INTR_MASK_RX_RXOUTOFRANGETYPE_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXLENGTHERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXLENGTHERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXLENGTHERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MMC_INTR_MASK_RX_RXLENGTHERROR_Wr_Mask (ULONG)(0xfffbffff)

#define MMC_INTR_MASK_RX_RXLENGTHERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXLENGTHERROR_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXLENGTHERROR_Mask)<<18));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXLENGTHERROR_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 18) & MMC_INTR_MASK_RX_RXLENGTHERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Wr_Mask (ULONG)(0xfffdffff)

#define MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Mask)<<17));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 17) & MMC_INTR_MASK_RX_RXUNICASTFRAMES_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Wr_Mask (ULONG)(0xfffeffff)

#define MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Mask)<<16));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 16) & MMC_INTR_MASK_RX_RX1024TOMAXOCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Wr_Mask (ULONG)(0xffff7fff)

#define MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Mask)<<15));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 15) & MMC_INTR_MASK_RX_RX512TO1023OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Wr_Mask (ULONG)(0xffffbfff)

#define MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Mask)<<14));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 14) & MMC_INTR_MASK_RX_RX256TO511OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Wr_Mask (ULONG)(0xffffdfff)

#define MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Mask)<<13));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 13) & MMC_INTR_MASK_RX_RX128TO255OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Wr_Mask (ULONG)(0xffffefff)

#define MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Mask)<<12));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 12) & MMC_INTR_MASK_RX_RX65TO127OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RX64OCTETS_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RX64OCTETS_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RX64OCTETS_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MMC_INTR_MASK_RX_RX64OCTETS_GB_Wr_Mask (ULONG)(0xfffff7ff)

#define MMC_INTR_MASK_RX_RX64OCTETS_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RX64OCTETS_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RX64OCTETS_GB_Mask)<<11));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RX64OCTETS_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 11) & MMC_INTR_MASK_RX_RX64OCTETS_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXOVERSIZE_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXOVERSIZE_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXOVERSIZE_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MMC_INTR_MASK_RX_RXOVERSIZE_G_Wr_Mask (ULONG)(0xfffffbff)

#define MMC_INTR_MASK_RX_RXOVERSIZE_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXOVERSIZE_G_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXOVERSIZE_G_Mask)<<10));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXOVERSIZE_G_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 10) & MMC_INTR_MASK_RX_RXOVERSIZE_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXUNDERSIZE_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXUNDERSIZE_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXUNDERSIZE_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MMC_INTR_MASK_RX_RXUNDERSIZE_G_Wr_Mask (ULONG)(0xfffffdff)

#define MMC_INTR_MASK_RX_RXUNDERSIZE_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXUNDERSIZE_G_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXUNDERSIZE_G_Mask)<<9));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXUNDERSIZE_G_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 9) & MMC_INTR_MASK_RX_RXUNDERSIZE_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXJABBERERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXJABBERERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXJABBERERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MMC_INTR_MASK_RX_RXJABBERERROR_Wr_Mask (ULONG)(0xfffffeff)

#define MMC_INTR_MASK_RX_RXJABBERERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXJABBERERROR_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXJABBERERROR_Mask)<<8));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXJABBERERROR_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 8) & MMC_INTR_MASK_RX_RXJABBERERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXRUNTERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXRUNTERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXRUNTERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MMC_INTR_MASK_RX_RXRUNTERROR_Wr_Mask (ULONG)(0xffffff7f)

#define MMC_INTR_MASK_RX_RXRUNTERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXRUNTERROR_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXRUNTERROR_Mask)<<7));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXRUNTERROR_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 7) & MMC_INTR_MASK_RX_RXRUNTERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXALIGNMENTERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXALIGNMENTERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXALIGNMENTERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MMC_INTR_MASK_RX_RXALIGNMENTERROR_Wr_Mask (ULONG)(0xffffffbf)

#define MMC_INTR_MASK_RX_RXALIGNMENTERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXALIGNMENTERROR_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXALIGNMENTERROR_Mask)<<6));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXALIGNMENTERROR_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 6) & MMC_INTR_MASK_RX_RXALIGNMENTERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXCRCERROR_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXCRCERROR_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXCRCERROR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MMC_INTR_MASK_RX_RXCRCERROR_Wr_Mask (ULONG)(0xffffffdf)

#define MMC_INTR_MASK_RX_RXCRCERROR_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXCRCERROR_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXCRCERROR_Mask)<<5));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXCRCERROR_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 5) & MMC_INTR_MASK_RX_RXCRCERROR_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Wr_Mask (ULONG)(0xffffffef)

#define MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Mask)<<4));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 4) & MMC_INTR_MASK_RX_RXMULTICASTFRAMES_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Wr_Mask (ULONG)(0xfffffff7)

#define MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Mask)<<3));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 3) & MMC_INTR_MASK_RX_RXBROADCASTFRAMES_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Wr_Mask (ULONG)(0xfffffffb)

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_G_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Mask)<<2));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_G_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 2) & MMC_INTR_MASK_RX_RXOCTETCOUNT_G_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Wr_Mask (ULONG)(0xfffffffd)

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Mask)<<1));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 1) & MMC_INTR_MASK_RX_RXOCTETCOUNT_GB_Mask);\
} while(0)

/*#define MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Mask (ULONG)(0x1)

/*#define MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Wr_Mask (ULONG)(0xfffffffe)

#define MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_MASK_RX_RgRd(v);\
		v = (v & (MMC_INTR_MASK_RX_RES_Wr_Mask_24))|((( 0) & (MMC_INTR_MASK_RX_Mask_24))<<24);\
		v = ((v & MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Wr_Mask) | ((data & MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Mask)<<0));\
		MMC_INTR_MASK_RX_RgWr(v);\
} while(0)

#define MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_UdfRd(data) do {\
		MMC_INTR_MASK_RX_RgRd(data);\
		data = ((data >> 0) & MMC_INTR_MASK_RX_RXFRAMECOUNT_GB_Mask);\
} while(0)


#define MMC_INTR_TX_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x708))

#define MMC_INTR_TX_RgWr(data) do {\
		iowrite32(data, (void *)MMC_INTR_TX_RgOffAddr);\
} while(0)

#define MMC_INTR_TX_RgRd(data) do {\
		(data) = ioread32((void *)MMC_INTR_TX_RgOffAddr);\
} while(0)

/*#define  MMC_INTR_TX_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MMC_INTR_TX_Mask_26 (ULONG)(0x3f)

/*#define MMC_INTR_TX_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MMC_INTR_TX_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MMC_INTR_TX_TXOSIZEGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXOSIZEGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXOSIZEGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (25)))*/

#define MMC_INTR_TX_TXOSIZEGPIS_Wr_Mask (ULONG)(0xfdffffff)

#define MMC_INTR_TX_TXOSIZEGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXOSIZEGPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXOSIZEGPIS_Mask)<<25));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXOSIZEGPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 25) & MMC_INTR_TX_TXOSIZEGPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXVLANGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXVLANGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXVLANGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MMC_INTR_TX_TXVLANGPIS_Wr_Mask (ULONG)(0xfeffffff)

#define MMC_INTR_TX_TXVLANGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXVLANGPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXVLANGPIS_Mask)<<24));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXVLANGPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 24) & MMC_INTR_TX_TXVLANGPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXPAUSPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXPAUSPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXPAUSPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (23)))*/

#define MMC_INTR_TX_TXPAUSPIS_Wr_Mask (ULONG)(0xff7fffff)

#define MMC_INTR_TX_TXPAUSPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXPAUSPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXPAUSPIS_Mask)<<23));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXPAUSPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 23) & MMC_INTR_TX_TXPAUSPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXEXDEFPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXEXDEFPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXEXDEFPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (22)))*/

#define MMC_INTR_TX_TXEXDEFPIS_Wr_Mask (ULONG)(0xffbfffff)

#define MMC_INTR_TX_TXEXDEFPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXEXDEFPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXEXDEFPIS_Mask)<<22));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXEXDEFPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 22) & MMC_INTR_TX_TXEXDEFPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXGPKTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXGPKTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXGPKTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MMC_INTR_TX_TXGPKTIS_Wr_Mask (ULONG)(0xffdfffff)

#define MMC_INTR_TX_TXGPKTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXGPKTIS_Wr_Mask) | ((data & MMC_INTR_TX_TXGPKTIS_Mask)<<21));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXGPKTIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 21) & MMC_INTR_TX_TXGPKTIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXGOCTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXGOCTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXGOCTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MMC_INTR_TX_TXGOCTIS_Wr_Mask (ULONG)(0xffefffff)

#define MMC_INTR_TX_TXGOCTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXGOCTIS_Wr_Mask) | ((data & MMC_INTR_TX_TXGOCTIS_Mask)<<20));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXGOCTIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 20) & MMC_INTR_TX_TXGOCTIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXCARERPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXCARERPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXCARERPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MMC_INTR_TX_TXCARERPIS_Wr_Mask (ULONG)(0xfff7ffff)

#define MMC_INTR_TX_TXCARERPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXCARERPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXCARERPIS_Mask)<<19));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXCARERPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 19) & MMC_INTR_TX_TXCARERPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXEXCOLPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXEXCOLPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXEXCOLPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MMC_INTR_TX_TXEXCOLPIS_Wr_Mask (ULONG)(0xfffbffff)

#define MMC_INTR_TX_TXEXCOLPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXEXCOLPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXEXCOLPIS_Mask)<<18));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXEXCOLPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 18) & MMC_INTR_TX_TXEXCOLPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXLATCOLPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXLATCOLPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXLATCOLPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MMC_INTR_TX_TXLATCOLPIS_Wr_Mask (ULONG)(0xfffdffff)

#define MMC_INTR_TX_TXLATCOLPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXLATCOLPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXLATCOLPIS_Mask)<<17));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXLATCOLPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 17) & MMC_INTR_TX_TXLATCOLPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXDEFPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXDEFPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXDEFPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MMC_INTR_TX_TXDEFPIS_Wr_Mask (ULONG)(0xfffeffff)

#define MMC_INTR_TX_TXDEFPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXDEFPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXDEFPIS_Mask)<<16));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXDEFPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 16) & MMC_INTR_TX_TXDEFPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXMCOLGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXMCOLGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXMCOLGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MMC_INTR_TX_TXMCOLGPIS_Wr_Mask (ULONG)(0xffff7fff)

#define MMC_INTR_TX_TXMCOLGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXMCOLGPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXMCOLGPIS_Mask)<<15));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXMCOLGPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 15) & MMC_INTR_TX_TXMCOLGPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXSCOLGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXSCOLGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXSCOLGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MMC_INTR_TX_TXSCOLGPIS_Wr_Mask (ULONG)(0xffffbfff)

#define MMC_INTR_TX_TXSCOLGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXSCOLGPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXSCOLGPIS_Mask)<<14));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXSCOLGPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 14) & MMC_INTR_TX_TXSCOLGPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXUFLOWERFIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXUFLOWERFIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXUFLOWERFIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MMC_INTR_TX_TXUFLOWERFIS_Wr_Mask (ULONG)(0xffffdfff)

#define MMC_INTR_TX_TXUFLOWERFIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXUFLOWERFIS_Wr_Mask) | ((data & MMC_INTR_TX_TXUFLOWERFIS_Mask)<<13));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXUFLOWERFIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 13) & MMC_INTR_TX_TXUFLOWERFIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXBCGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXBCGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXBCGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MMC_INTR_TX_TXBCGBPIS_Wr_Mask (ULONG)(0xffffefff)

#define MMC_INTR_TX_TXBCGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXBCGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXBCGBPIS_Mask)<<12));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXBCGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 12) & MMC_INTR_TX_TXBCGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXMCGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXMCGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXMCGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MMC_INTR_TX_TXMCGBPIS_Wr_Mask (ULONG)(0xfffff7ff)

#define MMC_INTR_TX_TXMCGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXMCGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXMCGBPIS_Mask)<<11));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXMCGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 11) & MMC_INTR_TX_TXMCGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXUCGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXUCGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXUCGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MMC_INTR_TX_TXUCGBPIS_Wr_Mask (ULONG)(0xfffffbff)

#define MMC_INTR_TX_TXUCGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXUCGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXUCGBPIS_Mask)<<10));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXUCGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 10) & MMC_INTR_TX_TXUCGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TX1024TMAXOCTGBFIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TX1024TMAXOCTGBFIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TX1024TMAXOCTGBFIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MMC_INTR_TX_TX1024TMAXOCTGBFIS_Wr_Mask (ULONG)(0xfffffdff)

#define MMC_INTR_TX_TX1024TMAXOCTGBFIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TX1024TMAXOCTGBFIS_Wr_Mask) | ((data & MMC_INTR_TX_TX1024TMAXOCTGBFIS_Mask)<<9));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TX1024TMAXOCTGBFIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 9) & MMC_INTR_TX_TX1024TMAXOCTGBFIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TX512T1023OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TX512T1023OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TX512T1023OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MMC_INTR_TX_TX512T1023OCTGBPIS_Wr_Mask (ULONG)(0xfffffeff)

#define MMC_INTR_TX_TX512T1023OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TX512T1023OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TX512T1023OCTGBPIS_Mask)<<8));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TX512T1023OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 8) & MMC_INTR_TX_TX512T1023OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TX256T511OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TX256T511OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TX256T511OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MMC_INTR_TX_TX256T511OCTGBPIS_Wr_Mask (ULONG)(0xffffff7f)

#define MMC_INTR_TX_TX256T511OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TX256T511OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TX256T511OCTGBPIS_Mask)<<7));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TX256T511OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 7) & MMC_INTR_TX_TX256T511OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TX128T255OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TX128T255OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TX128T255OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MMC_INTR_TX_TX128T255OCTGBPIS_Wr_Mask (ULONG)(0xffffffbf)

#define MMC_INTR_TX_TX128T255OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TX128T255OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TX128T255OCTGBPIS_Mask)<<6));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TX128T255OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 6) & MMC_INTR_TX_TX128T255OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TX65T127OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TX65T127OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TX65T127OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MMC_INTR_TX_TX65T127OCTGBPIS_Wr_Mask (ULONG)(0xffffffdf)

#define MMC_INTR_TX_TX65T127OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TX65T127OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TX65T127OCTGBPIS_Mask)<<5));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TX65T127OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 5) & MMC_INTR_TX_TX65T127OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TX64OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TX64OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TX64OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MMC_INTR_TX_TX64OCTGBPIS_Wr_Mask (ULONG)(0xffffffef)

#define MMC_INTR_TX_TX64OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TX64OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_TX_TX64OCTGBPIS_Mask)<<4));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TX64OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 4) & MMC_INTR_TX_TX64OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXMCGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXMCGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXMCGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MMC_INTR_TX_TXMCGPIS_Wr_Mask (ULONG)(0xfffffff7)

#define MMC_INTR_TX_TXMCGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXMCGPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXMCGPIS_Mask)<<3));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXMCGPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 3) & MMC_INTR_TX_TXMCGPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXBCGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXBCGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXBCGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MMC_INTR_TX_TXBCGPIS_Wr_Mask (ULONG)(0xfffffffb)

#define MMC_INTR_TX_TXBCGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXBCGPIS_Wr_Mask) | ((data & MMC_INTR_TX_TXBCGPIS_Mask)<<2));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXBCGPIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 2) & MMC_INTR_TX_TXBCGPIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXGBPKTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXGBPKTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXGBPKTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MMC_INTR_TX_TXGBPKTIS_Wr_Mask (ULONG)(0xfffffffd)

#define MMC_INTR_TX_TXGBPKTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXGBPKTIS_Wr_Mask) | ((data & MMC_INTR_TX_TXGBPKTIS_Mask)<<1));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXGBPKTIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 1) & MMC_INTR_TX_TXGBPKTIS_Mask);\
} while(0)

/*#define MMC_INTR_TX_TXGBOCTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_TX_TXGBOCTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_TX_TXGBOCTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MMC_INTR_TX_TXGBOCTIS_Wr_Mask (ULONG)(0xfffffffe)

#define MMC_INTR_TX_TXGBOCTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_TX_RgRd(v);\
		v = (v & (MMC_INTR_TX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_TX_Mask_26))<<26);\
		v = ((v & MMC_INTR_TX_TXGBOCTIS_Wr_Mask) | ((data & MMC_INTR_TX_TXGBOCTIS_Mask)<<0));\
		MMC_INTR_TX_RgWr(v);\
} while(0)

#define MMC_INTR_TX_TXGBOCTIS_UdfRd(data) do {\
		MMC_INTR_TX_RgRd(data);\
		data = ((data >> 0) & MMC_INTR_TX_TXGBOCTIS_Mask);\
} while(0)


#define MMC_INTR_RX_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x704))

#define MMC_INTR_RX_RgWr(data) do {\
		iowrite32(data, (void *)MMC_INTR_RX_RgOffAddr);\
} while(0)

#define MMC_INTR_RX_RgRd(data) do {\
		(data) = ioread32((void *)MMC_INTR_RX_RgOffAddr);\
} while(0)

/*#define  MMC_INTR_RX_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MMC_INTR_RX_Mask_26 (ULONG)(0x3f)

/*#define MMC_INTR_RX_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MMC_INTR_RX_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define MMC_INTR_RX_RXCTRLPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXCTRLPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXCTRLPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (25)))*/

#define MMC_INTR_RX_RXCTRLPIS_Wr_Mask (ULONG)(0xfdffffff)

#define MMC_INTR_RX_RXCTRLPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXCTRLPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXCTRLPIS_Mask)<<25));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXCTRLPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 25) & MMC_INTR_RX_RXCTRLPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXRCVERRPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXRCVERRPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXRCVERRPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MMC_INTR_RX_RXRCVERRPIS_Wr_Mask (ULONG)(0xfeffffff)

#define MMC_INTR_RX_RXRCVERRPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXRCVERRPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXRCVERRPIS_Mask)<<24));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXRCVERRPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 24) & MMC_INTR_RX_RXRCVERRPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXWDOGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXWDOGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXWDOGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (23)))*/

#define MMC_INTR_RX_RXWDOGPIS_Wr_Mask (ULONG)(0xff7fffff)

#define MMC_INTR_RX_RXWDOGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXWDOGPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXWDOGPIS_Mask)<<23));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXWDOGPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 23) & MMC_INTR_RX_RXWDOGPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXVLANGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXVLANGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXVLANGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (22)))*/

#define MMC_INTR_RX_RXVLANGBPIS_Wr_Mask (ULONG)(0xffbfffff)

#define MMC_INTR_RX_RXVLANGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXVLANGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXVLANGBPIS_Mask)<<22));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXVLANGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 22) & MMC_INTR_RX_RXVLANGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXFOVPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXFOVPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXFOVPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MMC_INTR_RX_RXFOVPIS_Wr_Mask (ULONG)(0xffdfffff)

#define MMC_INTR_RX_RXFOVPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXFOVPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXFOVPIS_Mask)<<21));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXFOVPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 21) & MMC_INTR_RX_RXFOVPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXPAUSPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXPAUSPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXPAUSPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MMC_INTR_RX_RXPAUSPIS_Wr_Mask (ULONG)(0xffefffff)

#define MMC_INTR_RX_RXPAUSPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXPAUSPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXPAUSPIS_Mask)<<20));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXPAUSPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 20) & MMC_INTR_RX_RXPAUSPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXORANGEPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXORANGEPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXORANGEPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MMC_INTR_RX_RXORANGEPIS_Wr_Mask (ULONG)(0xfff7ffff)

#define MMC_INTR_RX_RXORANGEPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXORANGEPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXORANGEPIS_Mask)<<19));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXORANGEPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 19) & MMC_INTR_RX_RXORANGEPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXLENERPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXLENERPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXLENERPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MMC_INTR_RX_RXLENERPIS_Wr_Mask (ULONG)(0xfffbffff)

#define MMC_INTR_RX_RXLENERPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXLENERPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXLENERPIS_Mask)<<18));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXLENERPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 18) & MMC_INTR_RX_RXLENERPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXUCBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXUCBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXUCBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MMC_INTR_RX_RXUCBPIS_Wr_Mask (ULONG)(0xfffdffff)

#define MMC_INTR_RX_RXUCBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXUCBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXUCBPIS_Mask)<<17));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXUCBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 17) & MMC_INTR_RX_RXUCBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RX1024TMAXOCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RX1024TMAXOCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RX1024TMAXOCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MMC_INTR_RX_RX1024TMAXOCTGBPIS_Wr_Mask (ULONG)(0xfffeffff)

#define MMC_INTR_RX_RX1024TMAXOCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RX1024TMAXOCTGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RX1024TMAXOCTGBPIS_Mask)<<16));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RX1024TMAXOCTGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 16) & MMC_INTR_RX_RX1024TMAXOCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RX512T1023OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RX512T1023OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RX512T1023OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MMC_INTR_RX_RX512T1023OCTGBPIS_Wr_Mask (ULONG)(0xffff7fff)

#define MMC_INTR_RX_RX512T1023OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RX512T1023OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RX512T1023OCTGBPIS_Mask)<<15));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RX512T1023OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 15) & MMC_INTR_RX_RX512T1023OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RX256T511OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RX256T511OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RX256T511OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MMC_INTR_RX_RX256T511OCTGBPIS_Wr_Mask (ULONG)(0xffffbfff)

#define MMC_INTR_RX_RX256T511OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RX256T511OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RX256T511OCTGBPIS_Mask)<<14));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RX256T511OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 14) & MMC_INTR_RX_RX256T511OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RX128T255OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RX128T255OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RX128T255OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MMC_INTR_RX_RX128T255OCTGBPIS_Wr_Mask (ULONG)(0xffffdfff)

#define MMC_INTR_RX_RX128T255OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RX128T255OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RX128T255OCTGBPIS_Mask)<<13));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RX128T255OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 13) & MMC_INTR_RX_RX128T255OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RX65T127OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RX65T127OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RX65T127OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MMC_INTR_RX_RX65T127OCTGBPIS_Wr_Mask (ULONG)(0xffffefff)

#define MMC_INTR_RX_RX65T127OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RX65T127OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RX65T127OCTGBPIS_Mask)<<12));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RX65T127OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 12) & MMC_INTR_RX_RX65T127OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RX64OCTGBPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RX64OCTGBPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RX64OCTGBPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MMC_INTR_RX_RX64OCTGBPIS_Wr_Mask (ULONG)(0xfffff7ff)

#define MMC_INTR_RX_RX64OCTGBPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RX64OCTGBPIS_Wr_Mask) | ((data & MMC_INTR_RX_RX64OCTGBPIS_Mask)<<11));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RX64OCTGBPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 11) & MMC_INTR_RX_RX64OCTGBPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXOSIZEGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXOSIZEGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXOSIZEGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MMC_INTR_RX_RXOSIZEGPIS_Wr_Mask (ULONG)(0xfffffbff)

#define MMC_INTR_RX_RXOSIZEGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXOSIZEGPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXOSIZEGPIS_Mask)<<10));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXOSIZEGPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 10) & MMC_INTR_RX_RXOSIZEGPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXUSIZEGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXUSIZEGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXUSIZEGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MMC_INTR_RX_RXUSIZEGPIS_Wr_Mask (ULONG)(0xfffffdff)

#define MMC_INTR_RX_RXUSIZEGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXUSIZEGPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXUSIZEGPIS_Mask)<<9));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXUSIZEGPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 9) & MMC_INTR_RX_RXUSIZEGPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXJABERPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXJABERPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXJABERPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MMC_INTR_RX_RXJABERPIS_Wr_Mask (ULONG)(0xfffffeff)

#define MMC_INTR_RX_RXJABERPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXJABERPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXJABERPIS_Mask)<<8));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXJABERPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 8) & MMC_INTR_RX_RXJABERPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXRUNTRIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXRUNTRIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXRUNTRIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MMC_INTR_RX_RXRUNTRIS_Wr_Mask (ULONG)(0xffffff7f)

#define MMC_INTR_RX_RXRUNTRIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXRUNTRIS_Wr_Mask) | ((data & MMC_INTR_RX_RXRUNTRIS_Mask)<<7));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXRUNTRIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 7) & MMC_INTR_RX_RXRUNTRIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXALGNERPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXALGNERPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXALGNERPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MMC_INTR_RX_RXALGNERPIS_Wr_Mask (ULONG)(0xffffffbf)

#define MMC_INTR_RX_RXALGNERPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXALGNERPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXALGNERPIS_Mask)<<6));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXALGNERPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 6) & MMC_INTR_RX_RXALGNERPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXCRCERPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXCRCERPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXCRCERPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MMC_INTR_RX_RXCRCERPIS_Wr_Mask (ULONG)(0xffffffdf)

#define MMC_INTR_RX_RXCRCERPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXCRCERPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXCRCERPIS_Mask)<<5));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXCRCERPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 5) & MMC_INTR_RX_RXCRCERPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXMCGPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXMCGPIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXMCGPIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MMC_INTR_RX_RXMCGPIS_Wr_Mask (ULONG)(0xffffffef)

#define MMC_INTR_RX_RXMCGPIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXMCGPIS_Wr_Mask) | ((data & MMC_INTR_RX_RXMCGPIS_Mask)<<4));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXMCGPIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 4) & MMC_INTR_RX_RXMCGPIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXBCGTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXBCGTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXBCGTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MMC_INTR_RX_RXBCGTIS_Wr_Mask (ULONG)(0xfffffff7)

#define MMC_INTR_RX_RXBCGTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXBCGTIS_Wr_Mask) | ((data & MMC_INTR_RX_RXBCGTIS_Mask)<<3));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXBCGTIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 3) & MMC_INTR_RX_RXBCGTIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXGOCTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXGOCTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXGOCTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MMC_INTR_RX_RXGOCTIS_Wr_Mask (ULONG)(0xfffffffb)

#define MMC_INTR_RX_RXGOCTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXGOCTIS_Wr_Mask) | ((data & MMC_INTR_RX_RXGOCTIS_Mask)<<2));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXGOCTIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 2) & MMC_INTR_RX_RXGOCTIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXGBOCTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXGBOCTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXGBOCTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MMC_INTR_RX_RXGBOCTIS_Wr_Mask (ULONG)(0xfffffffd)

#define MMC_INTR_RX_RXGBOCTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXGBOCTIS_Wr_Mask) | ((data & MMC_INTR_RX_RXGBOCTIS_Mask)<<1));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXGBOCTIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 1) & MMC_INTR_RX_RXGBOCTIS_Mask);\
} while(0)

/*#define MMC_INTR_RX_RXGBPKTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_INTR_RX_RXGBPKTIS_Mask (ULONG)(0x1)

/*#define MMC_INTR_RX_RXGBPKTIS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MMC_INTR_RX_RXGBPKTIS_Wr_Mask (ULONG)(0xfffffffe)

#define MMC_INTR_RX_RXGBPKTIS_UdfWr(data) do {\
		ULONG v;\
		MMC_INTR_RX_RgRd(v);\
		v = (v & (MMC_INTR_RX_RES_Wr_Mask_26))|((( 0) & (MMC_INTR_RX_Mask_26))<<26);\
		v = ((v & MMC_INTR_RX_RXGBPKTIS_Wr_Mask) | ((data & MMC_INTR_RX_RXGBPKTIS_Mask)<<0));\
		MMC_INTR_RX_RgWr(v);\
} while(0)

#define MMC_INTR_RX_RXGBPKTIS_UdfRd(data) do {\
		MMC_INTR_RX_RgRd(data);\
		data = ((data >> 0) & MMC_INTR_RX_RXGBPKTIS_Mask);\
} while(0)


#define MMC_CNTRL_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x700))

#define MMC_CNTRL_RgWr(data) do {\
		iowrite32(data, (void *)MMC_CNTRL_RgOffAddr);\
} while(0)

#define MMC_CNTRL_RgRd(data) do {\
		(data) = ioread32((void *)MMC_CNTRL_RgOffAddr);\
} while(0)

/*#define  MMC_CNTRL_Mask_9 (ULONG)(~(~0<<(23)))*/

#define  MMC_CNTRL_Mask_9 (ULONG)(0x7fffff)

/*#define MMC_CNTRL_RES_Wr_Mask_9 (ULONG)(~((~(~0<<(23)))<<(9)))*/

#define MMC_CNTRL_RES_Wr_Mask_9 (ULONG)(0x1ff)

/*#define  MMC_CNTRL_Mask_6 (ULONG)(~(~0<<(2)))*/

#define  MMC_CNTRL_Mask_6 (ULONG)(0x3)

/*#define MMC_CNTRL_RES_Wr_Mask_6 (ULONG)(~((~(~0<<(2)))<<(6)))*/

#define MMC_CNTRL_RES_Wr_Mask_6 (ULONG)(0xffffff3f)

/*#define MMC_CNTRL_UCDBC_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_UCDBC_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_UCDBC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MMC_CNTRL_UCDBC_Wr_Mask (ULONG)(0xfffffeff)

#define MMC_CNTRL_UCDBC_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_UCDBC_Wr_Mask) | ((data & MMC_CNTRL_UCDBC_Mask)<<8));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_UCDBC_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 8) & MMC_CNTRL_UCDBC_Mask);\
} while(0)

/*#define MMC_CNTRL_CNPRSTLVL_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_CNPRSTLVL_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_CNPRSTLVL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MMC_CNTRL_CNPRSTLVL_Wr_Mask (ULONG)(0xffffffdf)

#define MMC_CNTRL_CNPRSTLVL_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_CNPRSTLVL_Wr_Mask) | ((data & MMC_CNTRL_CNPRSTLVL_Mask)<<5));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_CNPRSTLVL_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 5) & MMC_CNTRL_CNPRSTLVL_Mask);\
} while(0)

/*#define MMC_CNTRL_CNTPRST_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_CNTPRST_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_CNTPRST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MMC_CNTRL_CNTPRST_Wr_Mask (ULONG)(0xffffffef)

#define MMC_CNTRL_CNTPRST_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_CNTPRST_Wr_Mask) | ((data & MMC_CNTRL_CNTPRST_Mask)<<4));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_CNTPRST_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 4) & MMC_CNTRL_CNTPRST_Mask);\
} while(0)

/*#define MMC_CNTRL_CNTFREEZ_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_CNTFREEZ_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_CNTFREEZ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MMC_CNTRL_CNTFREEZ_Wr_Mask (ULONG)(0xfffffff7)

#define MMC_CNTRL_CNTFREEZ_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_CNTFREEZ_Wr_Mask) | ((data & MMC_CNTRL_CNTFREEZ_Mask)<<3));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_CNTFREEZ_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 3) & MMC_CNTRL_CNTFREEZ_Mask);\
} while(0)

/*#define MMC_CNTRL_RSTONRD_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_RSTONRD_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_RSTONRD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MMC_CNTRL_RSTONRD_Wr_Mask (ULONG)(0xfffffffb)

#define MMC_CNTRL_RSTONRD_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_RSTONRD_Wr_Mask) | ((data & MMC_CNTRL_RSTONRD_Mask)<<2));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_RSTONRD_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 2) & MMC_CNTRL_RSTONRD_Mask);\
} while(0)

/*#define MMC_CNTRL_CNTSTOPRO_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_CNTSTOPRO_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_CNTSTOPRO_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MMC_CNTRL_CNTSTOPRO_Wr_Mask (ULONG)(0xfffffffd)

#define MMC_CNTRL_CNTSTOPRO_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_CNTSTOPRO_Wr_Mask) | ((data & MMC_CNTRL_CNTSTOPRO_Mask)<<1));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_CNTSTOPRO_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 1) & MMC_CNTRL_CNTSTOPRO_Mask);\
} while(0)

/*#define MMC_CNTRL_CNTRST_Mask (ULONG)(~(~0<<(1)))*/

#define MMC_CNTRL_CNTRST_Mask (ULONG)(0x1)

/*#define MMC_CNTRL_CNTRST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MMC_CNTRL_CNTRST_Wr_Mask (ULONG)(0xfffffffe)

#define MMC_CNTRL_CNTRST_UdfWr(data) do {\
		ULONG v;\
		MMC_CNTRL_RgRd(v);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_9))|((( 0) & (MMC_CNTRL_Mask_9))<<9);\
		v = (v & (MMC_CNTRL_RES_Wr_Mask_6))|((( 0) & (MMC_CNTRL_Mask_6))<<6);\
		v = ((v & MMC_CNTRL_CNTRST_Wr_Mask) | ((data & MMC_CNTRL_CNTRST_Mask)<<0));\
		MMC_CNTRL_RgWr(v);\
} while(0)

#define MMC_CNTRL_CNTRST_UdfRd(data) do {\
		MMC_CNTRL_RgRd(data);\
		data = ((data >> 0) & MMC_CNTRL_CNTRST_Mask);\
} while(0)



#define MAC_MA2LR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x314))

#define MAC_MA2LR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MA2LR_RgOffAddr);\
} while(0)

#define MAC_MA2LR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MA2LR_RgOffAddr);\
} while(0)

#define MAC_MA2LR_ADDRLO_UdfWr(data) do {\
		MAC_MA2LR_RgWr(data);\
} while(0)

#define MAC_MA2LR_ADDRLO_UdfRd(data) do {\
		MAC_MA2LR_RgRd(data);\
} while(0)


#define MAC_MA2HR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x310))

#define MAC_MA2HR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MA2HR_RgOffAddr);\
} while(0)

#define MAC_MA2HR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MA2HR_RgOffAddr);\
} while(0)

/*#define  MAC_MA2:wHR_Mask_19 (ULONG)(~(~0<<(5)))*/

#define  MAC_MA2HR_Mask_19 (ULONG)(0x1f)

/*#define MAC_MA2HR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(5)))<<(19)))*/

#define MAC_MA2HR_RES_Wr_Mask_19 (ULONG)(0xff07ffff)

/*#define MAC_MA2HR_AE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA2HR_AE_Mask (ULONG)(0x1)

/*#define MAC_MA2HR_AE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_MA2HR_AE_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MA2HR_AE_UdfWr(data) do {\
		ULONG v;\
		MAC_MA2HR_RgRd(v);\
		v = (v & (MAC_MA2HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA2HR_Mask_19))<<19);\
		v = ((v & MAC_MA2HR_AE_Wr_Mask) | ((data & MAC_MA2HR_AE_Mask)<<31));\
		MAC_MA2HR_RgWr(v);\
} while(0)

#define MAC_MA2HR_AE_UdfRd(data) do {\
		MAC_MA2HR_RgRd(data);\
		data = ((data >> 31) & MAC_MA2HR_AE_Mask);\
} while(0)

/*#define MAC_MA2HR_SA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA2HR_SA_Mask (ULONG)(0x1)

/*#define MAC_MA2HR_SA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (30)))*/

#define MAC_MA2HR_SA_Wr_Mask (ULONG)(0xbfffffff)

#define MAC_MA2HR_SA_UdfWr(data) do {\
		ULONG v;\
		MAC_MA2HR_RgRd(v);\
		v = (v & (MAC_MA2HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA2HR_Mask_19))<<19);\
		v = ((v & MAC_MA2HR_SA_Wr_Mask) | ((data & MAC_MA2HR_SA_Mask)<<30));\
		MAC_MA2HR_RgWr(v);\
} while(0)

#define MAC_MA2HR_SA_UdfRd(data) do {\
		MAC_MA2HR_RgRd(data);\
		data = ((data >> 30) & MAC_MA2HR_SA_Mask);\
} while(0)

/*#define MAC_MA2HR_MBC_Mask (ULONG)(~(~0<<(6)))*/

#define MAC_MA2HR_MBC_Mask (ULONG)(0x3f)

/*#define MAC_MA2HR_MBC_Wr_Mask (ULONG)(~((~(~0 << (6))) << (24)))*/

#define MAC_MA2HR_MBC_Wr_Mask (ULONG)(0xc0ffffff)

#define MAC_MA2HR_MBC_UdfWr(data) do {\
		ULONG v;\
		MAC_MA2HR_RgRd(v);\
		v = (v & (MAC_MA2HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA2HR_Mask_19))<<19);\
		v = ((v & MAC_MA2HR_MBC_Wr_Mask) | ((data & MAC_MA2HR_MBC_Mask)<<24));\
		MAC_MA2HR_RgWr(v);\
} while(0)

#define MAC_MA2HR_MBC_UdfRd(data) do {\
		MAC_MA2HR_RgRd(data);\
		data = ((data >> 24) & MAC_MA2HR_MBC_Mask);\
} while(0)

/*#define MAC_MA2HR_DCS_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MA2HR_DCS_Mask (ULONG)(0x7)

/*#define MAC_MA2HR_DCS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MAC_MA2HR_DCS_Wr_Mask (ULONG)(0xfff8ffff)

#define MAC_MA2HR_DCS_UdfWr(data) do {\
		ULONG v;\
		MAC_MA2HR_RgRd(v);\
		v = (v & (MAC_MA2HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA2HR_Mask_19))<<19);\
		v = ((v & MAC_MA2HR_DCS_Wr_Mask) | ((data & MAC_MA2HR_DCS_Mask)<<16));\
		MAC_MA2HR_RgWr(v);\
} while(0)

#define MAC_MA2HR_DCS_UdfRd(data) do {\
		MAC_MA2HR_RgRd(data);\
		data = ((data >> 16) & MAC_MA2HR_DCS_Mask);\
} while(0)

/*#define MAC_MA2HR_ADDRHI_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_MA2HR_ADDRHI_Mask (ULONG)(0xffff)

/*#define MAC_MA2HR_ADDRHI_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_MA2HR_ADDRHI_Wr_Mask (ULONG)(0xffff0000)

#define MAC_MA2HR_ADDRHI_UdfWr(data) do {\
		ULONG v;\
		MAC_MA2HR_RgRd(v);\
		v = (v & (MAC_MA2HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA2HR_Mask_19))<<19);\
		v = ((v & MAC_MA2HR_ADDRHI_Wr_Mask) | ((data & MAC_MA2HR_ADDRHI_Mask)<<0));\
		MAC_MA2HR_RgWr(v);\
} while(0)

#define MAC_MA2HR_ADDRHI_UdfRd(data) do {\
		MAC_MA2HR_RgRd(data);\
		data = ((data >> 0) & MAC_MA2HR_ADDRHI_Mask);\
} while(0)




#define MAC_MA1LR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x30c))

#define MAC_MA1LR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MA1LR_RgOffAddr);\
} while(0)

#define MAC_MA1LR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MA1LR_RgOffAddr);\
} while(0)

#define MAC_MA1LR_ADDRLO_UdfWr(data) do {\
		MAC_MA1LR_RgWr(data);\
} while(0)

#define MAC_MA1LR_ADDRLO_UdfRd(data) do {\
		MAC_MA1LR_RgRd(data);\
} while(0)


#define MAC_MA1HR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x308))

#define MAC_MA1HR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MA1HR_RgOffAddr);\
} while(0)

#define MAC_MA1HR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MA1HR_RgOffAddr);\
} while(0)

/*#define  MAC_MA1HR_Mask_19 (ULONG)(~(~0<<(5)))*/

#define  MAC_MA1HR_Mask_19 (ULONG)(0x1f)

/*#define MAC_MA1HR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(5)))<<(19)))*/

#define MAC_MA1HR_RES_Wr_Mask_19 (ULONG)(0xff07ffff)

/*#define MAC_MA1HR_AE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA1HR_AE_Mask (ULONG)(0x1)

/*#define MAC_MA1HR_AE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_MA1HR_AE_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MA1HR_AE_UdfWr(data) do {\
		ULONG v;\
		MAC_MA1HR_RgRd(v);\
		v = (v & (MAC_MA1HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1HR_Mask_19))<<19);\
		v = ((v & MAC_MA1HR_AE_Wr_Mask) | ((data & MAC_MA1HR_AE_Mask)<<31));\
		MAC_MA1HR_RgWr(v);\
} while(0)

#define MAC_MA1HR_AE_UdfRd(data) do {\
		MAC_MA1HR_RgRd(data);\
		data = ((data >> 31) & MAC_MA1HR_AE_Mask);\
} while(0)

/*#define MAC_MA1HR_SA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA1HR_SA_Mask (ULONG)(0x1)

/*#define MAC_MA1HR_SA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (30)))*/

#define MAC_MA1HR_SA_Wr_Mask (ULONG)(0xbfffffff)

#define MAC_MA1HR_SA_UdfWr(data) do {\
		ULONG v;\
		MAC_MA1HR_RgRd(v);\
		v = (v & (MAC_MA1HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1HR_Mask_19))<<19);\
		v = ((v & MAC_MA1HR_SA_Wr_Mask) | ((data & MAC_MA1HR_SA_Mask)<<30));\
		MAC_MA1HR_RgWr(v);\
} while(0)

#define MAC_MA1HR_SA_UdfRd(data) do {\
		MAC_MA1HR_RgRd(data);\
		data = ((data >> 30) & MAC_MA1HR_SA_Mask);\
} while(0)

/*#define MAC_MA1HR_MBC_Mask (ULONG)(~(~0<<(6)))*/

#define MAC_MA1HR_MBC_Mask (ULONG)(0x3f)

/*#define MAC_MA1HR_MBC_Wr_Mask (ULONG)(~((~(~0 << (6))) << (24)))*/

#define MAC_MA1HR_MBC_Wr_Mask (ULONG)(0xc0ffffff)

#define MAC_MA1HR_MBC_UdfWr(data) do {\
		ULONG v;\
		MAC_MA1HR_RgRd(v);\
		v = (v & (MAC_MA1HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1HR_Mask_19))<<19);\
		v = ((v & MAC_MA1HR_MBC_Wr_Mask) | ((data & MAC_MA1HR_MBC_Mask)<<24));\
		MAC_MA1HR_RgWr(v);\
} while(0)

#define MAC_MA1HR_MBC_UdfRd(data) do {\
		MAC_MA1HR_RgRd(data);\
		data = ((data >> 24) & MAC_MA1HR_MBC_Mask);\
} while(0)

/*#define MAC_MA1HR_DCS_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MA1HR_DCS_Mask (ULONG)(0x7)

/*#define MAC_MA1HR_DCS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MAC_MA1HR_DCS_Wr_Mask (ULONG)(0xfff8ffff)

#define MAC_MA1HR_DCS_UdfWr(data) do {\
		ULONG v;\
		MAC_MA1HR_RgRd(v);\
		v = (v & (MAC_MA1HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1HR_Mask_19))<<19);\
		v = ((v & MAC_MA1HR_DCS_Wr_Mask) | ((data & MAC_MA1HR_DCS_Mask)<<16));\
		MAC_MA1HR_RgWr(v);\
} while(0)

#define MAC_MA1HR_DCS_UdfRd(data) do {\
		MAC_MA1HR_RgRd(data);\
		data = ((data >> 16) & MAC_MA1HR_DCS_Mask);\
} while(0)

/*#define MAC_MA1HR_ADDRHI_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_MA1HR_ADDRHI_Mask (ULONG)(0xffff)

/*#define MAC_MA1HR_ADDRHI_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_MA1HR_ADDRHI_Wr_Mask (ULONG)(0xffff0000)

#define MAC_MA1HR_ADDRHI_UdfWr(data) do {\
		ULONG v;\
		MAC_MA1HR_RgRd(v);\
		v = (v & (MAC_MA1HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1HR_Mask_19))<<19);\
		v = ((v & MAC_MA1HR_ADDRHI_Wr_Mask) | ((data & MAC_MA1HR_ADDRHI_Mask)<<0));\
		MAC_MA1HR_RgWr(v);\
} while(0)

#define MAC_MA1HR_ADDRHI_UdfRd(data) do {\
		MAC_MA1HR_RgRd(data);\
		data = ((data >> 0) & MAC_MA1HR_ADDRHI_Mask);\
} while(0)


#define MAC_MA0LR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x304))

#define MAC_MA0LR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MA0LR_RgOffAddr);\
} while(0)

#define MAC_MA0LR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MA0LR_RgOffAddr);\
} while(0)

#define MAC_MA0LR_ADDRLO_UdfWr(data) do {\
		MAC_MA0LR_RgWr(data);\
} while(0)

#define MAC_MA0LR_ADDRLO_UdfRd(data) do {\
		MAC_MA0LR_RgRd(data);\
} while(0)


#define MAC_MA0HR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x300))

#define MAC_MA0HR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MA0HR_RgOffAddr);\
} while(0)

#define MAC_MA0HR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MA0HR_RgOffAddr);\
} while(0)

/*#define  MAC_MA0HR_Mask_19 (ULONG)(~(~0<<(12)))*/

#define  MAC_MA0HR_Mask_19 (ULONG)(0xfff)

/*#define MAC_MA0HR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(12)))<<(19)))*/

#define MAC_MA0HR_RES_Wr_Mask_19 (ULONG)(0x8007ffff)

/*#define MAC_MA0HR_AE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA0HR_AE_Mask (ULONG)(0x1)

/*#define MAC_MA0HR_AE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_MA0HR_AE_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MA0HR_AE_UdfWr(data) do {\
		ULONG v;\
		MAC_MA0HR_RgRd(v);\
		v = (v & (MAC_MA0HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA0HR_Mask_19))<<19);\
		v = ((v & MAC_MA0HR_AE_Wr_Mask) | ((data & MAC_MA0HR_AE_Mask)<<31));\
		MAC_MA0HR_RgWr(v);\
} while(0)

#define MAC_MA0HR_AE_UdfRd(data) do {\
		MAC_MA0HR_RgRd(data);\
		data = ((data >> 31) & MAC_MA0HR_AE_Mask);\
} while(0)

/*#define MAC_MA0HR_DCS_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MA0HR_DCS_Mask (ULONG)(0x7)

/*#define MAC_MA0HR_DCS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MAC_MA0HR_DCS_Wr_Mask (ULONG)(0xfff8ffff)

#define MAC_MA0HR_DCS_UdfWr(data) do {\
		ULONG v;\
		MAC_MA0HR_RgRd(v);\
		v = (v & (MAC_MA0HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA0HR_Mask_19))<<19);\
		v = ((v & MAC_MA0HR_DCS_Wr_Mask) | ((data & MAC_MA0HR_DCS_Mask)<<16));\
		MAC_MA0HR_RgWr(v);\
} while(0)

#define MAC_MA0HR_DCS_UdfRd(data) do {\
		MAC_MA0HR_RgRd(data);\
		data = ((data >> 16) & MAC_MA0HR_DCS_Mask);\
} while(0)

/*#define MAC_MA0HR_ADDRHI_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_MA0HR_ADDRHI_Mask (ULONG)(0xffff)

/*#define MAC_MA0HR_ADDRHI_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_MA0HR_ADDRHI_Wr_Mask (ULONG)(0xffff0000)

#define MAC_MA0HR_ADDRHI_UdfWr(data) do {\
		ULONG v;\
		MAC_MA0HR_RgRd(v);\
		v = (v & (MAC_MA0HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA0HR_Mask_19))<<19);\
		v = ((v & MAC_MA0HR_ADDRHI_Wr_Mask) | ((data & MAC_MA0HR_ADDRHI_Mask)<<0));\
		MAC_MA0HR_RgWr(v);\
} while(0)

#define MAC_MA0HR_ADDRHI_UdfRd(data) do {\
		MAC_MA0HR_RgRd(data);\
		data = ((data >> 0) & MAC_MA0HR_ADDRHI_Mask);\
} while(0)


#define MAC_GPIOR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x208))

#define MAC_GPIOR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_GPIOR_RgOffAddr);\
} while(0)

#define MAC_GPIOR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_GPIOR_RgOffAddr);\
} while(0)

/*#define  MAC_GPIOR_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MAC_GPIOR_Mask_28 (ULONG)(0xf)

/*#define MAC_GPIOR_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MAC_GPIOR_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define  MAC_GPIOR_Mask_20 (ULONG)(~(~0<<(4)))*/

#define  MAC_GPIOR_Mask_20 (ULONG)(0xf)

/*#define MAC_GPIOR_RES_Wr_Mask_20 (ULONG)(~((~(~0<<(4)))<<(20)))*/

#define MAC_GPIOR_RES_Wr_Mask_20 (ULONG)(0xff0fffff)

/*#define  MAC_GPIOR_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MAC_GPIOR_Mask_12 (ULONG)(0xf)

/*#define MAC_GPIOR_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MAC_GPIOR_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MAC_GPIOR_Mask_4 (ULONG)(~(~0<<(4)))*/

#define  MAC_GPIOR_Mask_4 (ULONG)(0xf)

/*#define MAC_GPIOR_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(4)))<<(4)))*/

#define MAC_GPIOR_RES_Wr_Mask_4 (ULONG)(0xffffff0f)

/*#define MAC_GPIOR_GPIT_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_GPIOR_GPIT_Mask (ULONG)(0xf)

/*#define MAC_GPIOR_GPIT_Wr_Mask (ULONG)(~((~(~0 << (4))) << (24)))*/

#define MAC_GPIOR_GPIT_Wr_Mask (ULONG)(0xf0ffffff)

#define MAC_GPIOR_GPIT_UdfWr(data) do {\
		ULONG v;\
		MAC_GPIOR_RgRd(v);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_28))|((( 0) & (MAC_GPIOR_Mask_28))<<28);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_20))|((( 0) & (MAC_GPIOR_Mask_20))<<20);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_12))|((( 0) & (MAC_GPIOR_Mask_12))<<12);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_4))|((( 0) & (MAC_GPIOR_Mask_4))<<4);\
		v = ((v & MAC_GPIOR_GPIT_Wr_Mask) | ((data & MAC_GPIOR_GPIT_Mask)<<24));\
		MAC_GPIOR_RgWr(v);\
} while(0)

#define MAC_GPIOR_GPIT_UdfRd(data) do {\
		MAC_GPIOR_RgRd(data);\
		data = ((data >> 24) & MAC_GPIOR_GPIT_Mask);\
} while(0)

/*#define MAC_GPIOR_GPIE_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_GPIOR_GPIE_Mask (ULONG)(0xf)

/*#define MAC_GPIOR_GPIE_Wr_Mask (ULONG)(~((~(~0 << (4))) << (16)))*/

#define MAC_GPIOR_GPIE_Wr_Mask (ULONG)(0xfff0ffff)

#define MAC_GPIOR_GPIE_UdfWr(data) do {\
		ULONG v;\
		MAC_GPIOR_RgRd(v);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_28))|((( 0) & (MAC_GPIOR_Mask_28))<<28);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_20))|((( 0) & (MAC_GPIOR_Mask_20))<<20);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_12))|((( 0) & (MAC_GPIOR_Mask_12))<<12);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_4))|((( 0) & (MAC_GPIOR_Mask_4))<<4);\
		v = ((v & MAC_GPIOR_GPIE_Wr_Mask) | ((data & MAC_GPIOR_GPIE_Mask)<<16));\
		MAC_GPIOR_RgWr(v);\
} while(0)

#define MAC_GPIOR_GPIE_UdfRd(data) do {\
		MAC_GPIOR_RgRd(data);\
		data = ((data >> 16) & MAC_GPIOR_GPIE_Mask);\
} while(0)

/*#define MAC_GPIOR_GPO_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_GPIOR_GPO_Mask (ULONG)(0xf)

/*#define MAC_GPIOR_GPO_Wr_Mask (ULONG)(~((~(~0 << (4))) << (8)))*/

#define MAC_GPIOR_GPO_Wr_Mask (ULONG)(0xfffff0ff)

#define MAC_GPIOR_GPO_UdfWr(data) do {\
		ULONG v;\
		MAC_GPIOR_RgRd(v);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_28))|((( 0) & (MAC_GPIOR_Mask_28))<<28);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_20))|((( 0) & (MAC_GPIOR_Mask_20))<<20);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_12))|((( 0) & (MAC_GPIOR_Mask_12))<<12);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_4))|((( 0) & (MAC_GPIOR_Mask_4))<<4);\
		v = ((v & MAC_GPIOR_GPO_Wr_Mask) | ((data & MAC_GPIOR_GPO_Mask)<<8));\
		MAC_GPIOR_RgWr(v);\
} while(0)

#define MAC_GPIOR_GPO_UdfRd(data) do {\
		MAC_GPIOR_RgRd(data);\
		data = ((data >> 8) & MAC_GPIOR_GPO_Mask);\
} while(0)

/*#define MAC_GPIOR_GPIS_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_GPIOR_GPIS_Mask (ULONG)(0xf)

/*#define MAC_GPIOR_GPIS_Wr_Mask (ULONG)(~((~(~0 << (4))) << (0)))*/

#define MAC_GPIOR_GPIS_Wr_Mask (ULONG)(0xfffffff0)

#define MAC_GPIOR_GPIS_UdfWr(data) do {\
		ULONG v;\
		MAC_GPIOR_RgRd(v);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_28))|((( 0) & (MAC_GPIOR_Mask_28))<<28);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_20))|((( 0) & (MAC_GPIOR_Mask_20))<<20);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_12))|((( 0) & (MAC_GPIOR_Mask_12))<<12);\
		v = (v & (MAC_GPIOR_RES_Wr_Mask_4))|((( 0) & (MAC_GPIOR_Mask_4))<<4);\
		v = ((v & MAC_GPIOR_GPIS_Wr_Mask) | ((data & MAC_GPIOR_GPIS_Mask)<<0));\
		MAC_GPIOR_RgWr(v);\
} while(0)

#define MAC_GPIOR_GPIS_UdfRd(data) do {\
		MAC_GPIOR_RgRd(data);\
		data = ((data >> 0) & MAC_GPIOR_GPIS_Mask);\
} while(0)


#define MAC_GMIIDR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x204))

#define MAC_GMIIDR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_GMIIDR_RgOffAddr);\
} while(0)

#define MAC_GMIIDR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_GMIIDR_RgOffAddr);\
} while(0)

/*#define MAC_GMIIDR_RA_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_GMIIDR_RA_Mask (ULONG)(0xffff)

/*#define MAC_GMIIDR_RA_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_GMIIDR_RA_Wr_Mask (ULONG)(0xffff)

#define MAC_GMIIDR_RA_UdfWr(data) do{\
		ULONG v;\
		MAC_GMIIDR_RgRd(v);\
		v = ((v & MAC_GMIIDR_RA_Wr_Mask) | ((data & MAC_GMIIDR_RA_Mask)<<16));\
		MAC_GMIIDR_RgWr(v);\
} while(0)

#define MAC_GMIIDR_RA_UdfRd(data) do {\
		MAC_GMIIDR_RgRd(data);\
		data = ((data >> 16) & MAC_GMIIDR_RA_Mask);\
} while(0)

/*#define MAC_GMIIDR_GD_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_GMIIDR_GD_Mask (ULONG)(0xffff)

/*#define MAC_GMIIDR_GD_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_GMIIDR_GD_Wr_Mask (ULONG)(0xffff0000)

#define MAC_GMIIDR_GD_UdfWr(data) do{\
		ULONG v;\
		MAC_GMIIDR_RgRd(v);\
		v = ((v & MAC_GMIIDR_GD_Wr_Mask) | ((data & MAC_GMIIDR_GD_Mask)<<0));\
		MAC_GMIIDR_RgWr(v);\
} while(0)

#define MAC_GMIIDR_GD_UdfRd(data) do {\
		MAC_GMIIDR_RgRd(data);\
		data = ((data >> 0) & MAC_GMIIDR_GD_Mask);\
} while(0)


#define MAC_GMIIAR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x200))

#define MAC_GMIIAR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_GMIIAR_RgOffAddr);\
} while(0)

#define MAC_GMIIAR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_GMIIAR_RgOffAddr);\
} while(0)

/*#define  MAC_GMIIAR_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MAC_GMIIAR_Mask_26 (ULONG)(0x3f)

/*#define MAC_GMIIAR_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MAC_GMIIAR_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define  MAC_GMIIAR_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MAC_GMIIAR_Mask_12 (ULONG)(0xf)

/*#define MAC_GMIIAR_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MAC_GMIIAR_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define  MAC_GMIIAR_Mask_5 (ULONG)(~(~0<<(3)))*/

#define  MAC_GMIIAR_Mask_5 (ULONG)(0x7)

/*#define MAC_GMIIAR_RES_Wr_Mask_5 (ULONG)(~((~(~0<<(3)))<<(5)))*/

#define MAC_GMIIAR_RES_Wr_Mask_5 (ULONG)(0xffffff1f)

/*#define MAC_GMIIAR_PA_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_GMIIAR_PA_Mask (ULONG)(0x1f)

/*#define MAC_GMIIAR_PA_Wr_Mask (ULONG)(~((~(~0 << (5))) << (21)))*/

#define MAC_GMIIAR_PA_Wr_Mask (ULONG)(0xfc1fffff)

#define MAC_GMIIAR_PA_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_PA_Wr_Mask) | ((data & MAC_GMIIAR_PA_Mask)<<21));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_PA_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 21) & MAC_GMIIAR_PA_Mask);\
} while(0)

/*#define MAC_GMIIAR_GR_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_GMIIAR_GR_Mask (ULONG)(0x1f)

/*#define MAC_GMIIAR_GR_Wr_Mask (ULONG)(~((~(~0 << (5))) << (16)))*/

#define MAC_GMIIAR_GR_Wr_Mask (ULONG)(0xffe0ffff)

#define MAC_GMIIAR_GR_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_GR_Wr_Mask) | ((data & MAC_GMIIAR_GR_Mask)<<16));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_GR_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 16) & MAC_GMIIAR_GR_Mask);\
} while(0)

/*#define MAC_GMIIAR_CR_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_GMIIAR_CR_Mask (ULONG)(0xf)

/*#define MAC_GMIIAR_CR_Wr_Mask (ULONG)(~((~(~0 << (4))) << (8)))*/

#define MAC_GMIIAR_CR_Wr_Mask (ULONG)(0xfffff0ff)

#define MAC_GMIIAR_CR_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_CR_Wr_Mask) | ((data & MAC_GMIIAR_CR_Mask)<<8));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_CR_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 8) & MAC_GMIIAR_CR_Mask);\
} while(0)

/*#define MAC_GMIIAR_SKAP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_GMIIAR_SKAP_Mask (ULONG)(0x1)

/*#define MAC_GMIIAR_SKAP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_GMIIAR_SKAP_Wr_Mask (ULONG)(0xffffffef)

#define MAC_GMIIAR_SKAP_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_SKAP_Wr_Mask) | ((data & MAC_GMIIAR_SKAP_Mask)<<4));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_SKAP_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 4) & MAC_GMIIAR_SKAP_Mask);\
} while(0)

/*#define MAC_GMIIAR_GOC_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_GMIIAR_GOC_Mask (ULONG)(0x3)

/*#define MAC_GMIIAR_GOC_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MAC_GMIIAR_GOC_Wr_Mask (ULONG)(0xfffffff3)

#define MAC_GMIIAR_GOC_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_GOC_Wr_Mask) | ((data & MAC_GMIIAR_GOC_Mask)<<2));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_GOC_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 2) & MAC_GMIIAR_GOC_Mask);\
} while(0)

/*#define MAC_GMIIAR_C45E_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_GMIIAR_C45E_Mask (ULONG)(0x1)

/*#define MAC_GMIIAR_C45E_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_GMIIAR_C45E_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_GMIIAR_C45E_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_C45E_Wr_Mask) | ((data & MAC_GMIIAR_C45E_Mask)<<1));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_C45E_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 1) & MAC_GMIIAR_C45E_Mask);\
} while(0)

/*#define MAC_GMIIAR_GB_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_GMIIAR_GB_Mask (ULONG)(0x1)

/*#define MAC_GMIIAR_GB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_GMIIAR_GB_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_GMIIAR_GB_UdfWr(data) do {\
		ULONG v;\
		MAC_GMIIAR_RgRd(v);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_26))|((( 0) & (MAC_GMIIAR_Mask_26))<<26);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_12))|((( 0) & (MAC_GMIIAR_Mask_12))<<12);\
		v = (v & (MAC_GMIIAR_RES_Wr_Mask_5))|((( 0) & (MAC_GMIIAR_Mask_5))<<5);\
		v = ((v & MAC_GMIIAR_GB_Wr_Mask) | ((data & MAC_GMIIAR_GB_Mask)<<0));\
		MAC_GMIIAR_RgWr(v);\
} while(0)

#define MAC_GMIIAR_GB_UdfRd(data) do {\
		MAC_GMIIAR_RgRd(data);\
		data = ((data >> 0) & MAC_GMIIAR_GB_Mask);\
} while(0)



#define MAC_HFR2_NOPRV_RgOffAddr ((volatile ULONG *)(dwc_eth_ntn_reg_pci_base_addr + MAC_OFFSET + 0x124))

#define MAC_HFR2_NOPRV_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HFR2_NOPRV_RgOffAddr);\
} while(0)


#define MAC_HFR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x124))

#define MAC_HFR2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HFR2_RgOffAddr);\
} while(0)

/*#define MAC_HFR2_AUXSNAPNUM_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_HFR2_AUXSNAPNUM_Mask (ULONG)(0x7)

#define MAC_HFR2_AUXSNAPNUM_UdfRd(data) do {\
		MAC_HFR2_RgRd(data);\
		data = ((data >> 28) & MAC_HFR2_AUXSNAPNUM_Mask);\
} while(0)

/*#define MAC_HFR2_PPSOUTNUM_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_HFR2_PPSOUTNUM_Mask (ULONG)(0x7)

#define MAC_HFR2_PPSOUTNUM_UdfRd(data) do {\
		MAC_HFR2_RgRd(data);\
		data = ((data >> 24) & MAC_HFR2_PPSOUTNUM_Mask);\
} while(0)

/*#define MAC_HFR2_TXCHCNT_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_HFR2_TXCHCNT_Mask (ULONG)(0xf)

#define MAC_HFR2_TXCHCNT_UdfRd(data) do {\
		MAC_HFR2_RgRd(data);\
		data = ((data >> 18) & MAC_HFR2_TXCHCNT_Mask);\
} while(0)

/*#define MAC_HFR2_RXCHCNT_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_HFR2_RXCHCNT_Mask (ULONG)(0xf)

#define MAC_HFR2_RXCHCNT_UdfRd(data) do {\
		MAC_HFR2_RgRd(data);\
		data = ((data >> 12) & MAC_HFR2_RXCHCNT_Mask);\
} while(0)

/*#define MAC_HFR2_TXQCNT_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_HFR2_TXQCNT_Mask (ULONG)(0xf)

#define MAC_HFR2_TXQCNT_UdfRd(data) do {\
		MAC_HFR2_RgRd(data);\
		data = ((data >> 6) & MAC_HFR2_TXQCNT_Mask);\
} while(0)

/*#define MAC_HFR2_RXQCNT_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_HFR2_RXQCNT_Mask (ULONG)(0xf)

#define MAC_HFR2_RXQCNT_UdfRd(data) do {\
		MAC_HFR2_RgRd(data);\
		data = ((data >> 0) & MAC_HFR2_RXQCNT_Mask);\
} while(0)


#define MAC_HFR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x120))

#define MAC_HFR1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HFR1_RgOffAddr);\
} while(0)

/*#define MAC_HFR1_L3L4FILTERNUM_Mask (ULONG)(~(~0<<(4)))*/

#define MAC_HFR1_L3L4FILTERNUM_Mask (ULONG)(0xf)

#define MAC_HFR1_L3L4FILTERNUM_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 27) & MAC_HFR1_L3L4FILTERNUM_Mask);\
} while(0)

/*#define MAC_HFR1_HASHTBLSZ_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_HFR1_HASHTBLSZ_Mask (ULONG)(0x3)

#define MAC_HFR1_HASHTBLSZ_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 24) & MAC_HFR1_HASHTBLSZ_Mask);\
} while(0)

/*#define MAC_HFR1_LPMODEEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_LPMODEEN_Mask (ULONG)(0x1)

#define MAC_HFR1_LPMODEEN_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 23) & MAC_HFR1_LPMODEEN_Mask);\
} while(0)

/*#define MAC_HFR1_AVSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_AVSEL_Mask (ULONG)(0x1)

#define MAC_HFR1_AVSEL_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 20) & MAC_HFR1_AVSEL_Mask);\
} while(0)

/*#define MAC_HFR1_DMADEBUGEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_DMADEBUGEN_Mask (ULONG)(0x1)

#define MAC_HFR1_DMADEBUGEN_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 19) & MAC_HFR1_DMADEBUGEN_Mask);\
} while(0)

/*#define MAC_HFR1_TSOEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_TSOEN_Mask (ULONG)(0x1)

#define MAC_HFR1_TSOEN_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 18) & MAC_HFR1_TSOEN_Mask);\
} while(0)

/*#define MAC_HFR1_SPHEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_SPHEN_Mask (ULONG)(0x1)

#define MAC_HFR1_SPHEN_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 17) & MAC_HFR1_SPHEN_Mask);\
} while(0)

/*#define MAC_HFR1_DCBEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_DCBEN_Mask (ULONG)(0x1)

#define MAC_HFR1_DCBEN_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 16) & MAC_HFR1_DCBEN_Mask);\
} while(0)

/*#define MAC_HFR1_ADVTHWORD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR1_ADVTHWORD_Mask (ULONG)(0x1)

#define MAC_HFR1_ADVTHWORD_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 13) & MAC_HFR1_ADVTHWORD_Mask);\
} while(0)

/*#define MAC_HFR1_TXFIFOSIZE_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_HFR1_TXFIFOSIZE_Mask (ULONG)(0x1f)

#define MAC_HFR1_TXFIFOSIZE_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 6) & MAC_HFR1_TXFIFOSIZE_Mask);\
} while(0)

/*#define MAC_HFR1_RXFIFOSIZE_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_HFR1_RXFIFOSIZE_Mask (ULONG)(0x1f)

#define MAC_HFR1_RXFIFOSIZE_UdfRd(data) do {\
		MAC_HFR1_RgRd(data);\
		data = ((data >> 0) & MAC_HFR1_RXFIFOSIZE_Mask);\
} while(0)


#define MAC_HFR0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x11c))

#define MAC_HFR0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HFR0_RgOffAddr);\
} while(0)

/*#define MAC_HFR0_ACTPHYSEL_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_HFR0_ACTPHYSEL_Mask (ULONG)(0x7)

#define MAC_HFR0_ACTPHYSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 28) & MAC_HFR0_ACTPHYSEL_Mask);\
} while(0)

/*#define MAC_HFR0_SAVLANINS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_SAVLANINS_Mask (ULONG)(0x0)

#define MAC_HFR0_SAVLANINS_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 27) & MAC_HFR0_SAVLANINS_Mask);\
} while(0)

/*#define MAC_HFR0_TSINTSEL_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_HFR0_TSINTSEL_Mask (ULONG)(0x3)

#define MAC_HFR0_TSINTSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 25) & MAC_HFR0_TSINTSEL_Mask);\
} while(0)

/*#define MAC_HFR0_MACADR64SEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_MACADR64SEL_Mask (ULONG)(0x1)

#define MAC_HFR0_MACADR64SEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 24) & MAC_HFR0_MACADR64SEL_Mask);\
} while(0)

/*#define MAC_HFR0_MACADR32SEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_MACADR32SEL_Mask (ULONG)(0x1)

#define MAC_HFR0_MACADR32SEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 23) & MAC_HFR0_MACADR32SEL_Mask);\
} while(0)

/*#define MAC_HFR0_ADDMACADRSEL_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_HFR0_ADDMACADRSEL_Mask (ULONG)(0x1f)

#define MAC_HFR0_ADDMACADRSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 18) & MAC_HFR0_ADDMACADRSEL_Mask);\
} while(0)

/*#define MAC_HFR0_RXCOE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_RXCOE_Mask (ULONG)(0x1)

#define MAC_HFR0_RXCOE_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 16) & MAC_HFR0_RXCOE_Mask);\
} while(0)

/*#define MAC_HFR0_TXCOESEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_TXCOESEL_Mask (ULONG)(0x1)

#define MAC_HFR0_TXCOESEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 14) & MAC_HFR0_TXCOESEL_Mask);\
} while(0)

/*#define MAC_HFR0_EEESEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_EEESEL_Mask (ULONG)(0x1)

#define MAC_HFR0_EEESEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 13) & MAC_HFR0_EEESEL_Mask);\
} while(0)

/*#define MAC_HFR0_TSSSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_TSSSEL_Mask (ULONG)(0x1)

#define MAC_HFR0_TSSSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 12) & MAC_HFR0_TSSSEL_Mask);\
} while(0)

/*#define MAC_HFR0_ARPOFFLDEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_ARPOFFLDEN_Mask (ULONG)(0x1)

#define MAC_HFR0_ARPOFFLDEN_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 9) & MAC_HFR0_ARPOFFLDEN_Mask);\
} while(0)

/*#define MAC_HFR0_MMCSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_MMCSEL_Mask (ULONG)(0x1)

#define MAC_HFR0_MMCSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 8) & MAC_HFR0_MMCSEL_Mask);\
} while(0)

/*#define MAC_HFR0_MGKSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_MGKSEL_Mask (ULONG)(0x1)

#define MAC_HFR0_MGKSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 7) & MAC_HFR0_MGKSEL_Mask);\
} while(0)

/*#define MAC_HFR0_RWKSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_RWKSEL_Mask (ULONG)(0x1)

#define MAC_HFR0_RWKSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 6) & MAC_HFR0_RWKSEL_Mask);\
} while(0)

/*#define MAC_HFR0_SMASEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_SMASEL_Mask (ULONG)(0x1)

#define MAC_HFR0_SMASEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 5) & MAC_HFR0_SMASEL_Mask);\
} while(0)

/*#define MAC_HFR0_VLANHASEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_VLANHASEL_Mask (ULONG)(0x1)

#define MAC_HFR0_VLANHASEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 4) & MAC_HFR0_VLANHASEL_Mask);\
} while(0)

/*#define MAC_HFR0_PCSSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_PCSSEL_Mask (ULONG)(0x1)

#define MAC_HFR0_PCSSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 3) & MAC_HFR0_PCSSEL_Mask);\
} while(0)

/*#define MAC_HFR0_HDSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_HDSEL_Mask (ULONG)(0x1)

#define MAC_HFR0_HDSEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 2) & MAC_HFR0_HDSEL_Mask);\
} while(0)

/*#define MAC_HFR0_GMIISEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_GMIISEL_Mask (ULONG)(0x1)

#define MAC_HFR0_GMIISEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 1) & MAC_HFR0_GMIISEL_Mask);\
} while(0)

/*#define MAC_HFR0_MIISEL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_HFR0_MIISEL_Mask (ULONG)(0x1)

#define MAC_HFR0_MIISEL_UdfRd(data) do {\
		MAC_HFR0_RgRd(data);\
		data = ((data >> 0) & MAC_HFR0_MIISEL_Mask);\
} while(0)


#define MAC_MDR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x114))

#define MAC_MDR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MDR_RgOffAddr);\
} while(0)

/*#define MAC_MDR_TFCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_MDR_TFCSTS_Mask (ULONG)(0x3)

#define MAC_MDR_TFCSTS_UdfRd(data) do {\
		MAC_MDR_RgRd(data);\
		data = ((data >> 17) & MAC_MDR_TFCSTS_Mask);\
} while(0)

/*#define MAC_MDR_TPESTS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MDR_TPESTS_Mask (ULONG)(0x1)

#define MAC_MDR_TPESTS_UdfRd(data) do {\
		MAC_MDR_RgRd(data);\
		data = ((data >> 16) & MAC_MDR_TPESTS_Mask);\
} while(0)

/*#define MAC_MDR_RFCFCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_MDR_RFCFCSTS_Mask (ULONG)(0x3)

#define MAC_MDR_RFCFCSTS_UdfRd(data) do {\
		MAC_MDR_RgRd(data);\
		data = ((data >> 1) & MAC_MDR_RFCFCSTS_Mask);\
} while(0)

/*#define MAC_MDR_RPESTS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MDR_RPESTS_Mask (ULONG)(0x1)

#define MAC_MDR_RPESTS_UdfRd(data) do {\
		MAC_MDR_RgRd(data);\
		data = ((data >> 0) & MAC_MDR_RPESTS_Mask);\
} while(0)


#define MAC_VR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x110))

#define MAC_VR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_VR_RgOffAddr);\
} while(0)

/*#define MAC_VR_USERVER_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_VR_USERVER_Mask (ULONG)(0xff)

#define MAC_VR_USERVER_UdfRd(data) do {\
		MAC_VR_RgRd(data);\
		data = ((data >> 8) & MAC_VR_USERVER_Mask);\
} while(0)

/*#define MAC_VR_SNPSVER_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_VR_SNPSVER_Mask (ULONG)(0xff)

#define MAC_VR_SNPSVER_UdfRd(data) do {\
		MAC_VR_RgRd(data);\
		data = ((data >> 0) & MAC_VR_SNPSVER_Mask);\
} while(0)


#define MAC_HTR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x2c))

#define MAC_HTR7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR7_RgOffAddr);\
} while(0)

#define MAC_HTR7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR7_RgOffAddr);\
} while(0)

#define MAC_HTR7_HT_UdfWr(data) do {\
		MAC_HTR7_RgWr(data);\
} while(0)

#define MAC_HTR7_HT_UdfRd(data) do {\
		MAC_HTR7_RgRd(data);\
} while(0)


#define MAC_HTR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x28))

#define MAC_HTR6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR6_RgOffAddr);\
} while(0)

#define MAC_HTR6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR6_RgOffAddr);\
} while(0)

#define MAC_HTR6_HT_UdfWr(data) do {\
		MAC_HTR6_RgWr(data);\
} while(0)

#define MAC_HTR6_HT_UdfRd(data) do {\
		MAC_HTR6_RgRd(data);\
} while(0)


#define MAC_HTR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x24))

#define MAC_HTR5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR5_RgOffAddr);\
} while(0)

#define MAC_HTR5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR5_RgOffAddr);\
} while(0)

#define MAC_HTR5_HT_UdfWr(data) do {\
		MAC_HTR5_RgWr(data);\
} while(0)

#define MAC_HTR5_HT_UdfRd(data) do {\
		MAC_HTR5_RgRd(data);\
} while(0)


#define MAC_HTR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x20))

#define MAC_HTR4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR4_RgOffAddr);\
} while(0)

#define MAC_HTR4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR4_RgOffAddr);\
} while(0)

#define MAC_HTR4_HT_UdfWr(data) do {\
		MAC_HTR4_RgWr(data);\
} while(0)

#define MAC_HTR4_HT_UdfRd(data) do {\
		MAC_HTR4_RgRd(data);\
} while(0)


#define MAC_HTR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x1c))

#define MAC_HTR3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR3_RgOffAddr);\
} while(0)

#define MAC_HTR3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR3_RgOffAddr);\
} while(0)

#define MAC_HTR3_HT_UdfWr(data) do {\
		MAC_HTR3_RgWr(data);\
} while(0)

#define MAC_HTR3_HT_UdfRd(data) do {\
		MAC_HTR3_RgRd(data);\
} while(0)


#define MAC_HTR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x18))

#define MAC_HTR2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR2_RgOffAddr);\
} while(0)

#define MAC_HTR2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR2_RgOffAddr);\
} while(0)

#define MAC_HTR2_HT_UdfWr(data) do {\
		MAC_HTR2_RgWr(data);\
} while(0)

#define MAC_HTR2_HT_UdfRd(data) do {\
		MAC_HTR2_RgRd(data);\
} while(0)


#define MAC_HTR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x14))

#define MAC_HTR1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR1_RgOffAddr);\
} while(0)

#define MAC_HTR1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR1_RgOffAddr);\
} while(0)

#define MAC_HTR1_HT_UdfWr(data) do {\
		MAC_HTR1_RgWr(data);\
} while(0)

#define MAC_HTR1_HT_UdfRd(data) do {\
		MAC_HTR1_RgRd(data);\
} while(0)


#define MAC_HTR0_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x10))

#define MAC_HTR0_RgWr(data) do {\
		iowrite32(data, (void *)MAC_HTR0_RgOffAddr);\
} while(0)

#define MAC_HTR0_RgRd(data) do {\
		(data) = ioread32((void *)MAC_HTR0_RgOffAddr);\
} while(0)

#define MAC_HTR0_HT_UdfWr(data) do {\
		MAC_HTR0_RgWr(data);\
} while(0)

#define MAC_HTR0_HT_UdfRd(data) do {\
		MAC_HTR0_RgRd(data);\
} while(0)



#define MAC_IMR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb4))

#define MAC_IMR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_IMR_RgOffAddr);\
} while(0)

#define MAC_IMR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_IMR_RgOffAddr);\
} while(0)

/*#define  MAC_IMR_Mask_13 (ULONG)(~(~0<<(19)))*/

#define  MAC_IMR_Mask_13 (ULONG)(0x7ffff)

/*#define MAC_IMR_RES_Wr_Mask_13 (ULONG)(~((~(~0<<(19)))<<(13)))*/

#define MAC_IMR_RES_Wr_Mask_13 (ULONG)(0x1fff)

/*#define  MAC_IMR_Mask_6 (ULONG)(~(~0<<(6)))*/

#define  MAC_IMR_Mask_6 (ULONG)(0x3f)

/*#define MAC_IMR_RES_Wr_Mask_6 (ULONG)(~((~(~0<<(6)))<<(6)))*/

#define MAC_IMR_RES_Wr_Mask_6 (ULONG)(0xfffff03f)

/*#define MAC_IMR_TSIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_TSIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_TSIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MAC_IMR_TSIM_Wr_Mask (ULONG)(0xffffefff)

#define MAC_IMR_TSIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_TSIM_Wr_Mask) | ((data & MAC_IMR_TSIM_Mask)<<12));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_TSIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 12) & MAC_IMR_TSIM_Mask);\
} while(0)

/*#define MAC_IMR_LPIIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_LPIIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_LPIIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_IMR_LPIIM_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_IMR_LPIIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_LPIIM_Wr_Mask) | ((data & MAC_IMR_LPIIM_Mask)<<5));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_LPIIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 5) & MAC_IMR_LPIIM_Mask);\
} while(0)

/*#define MAC_IMR_PMTIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_PMTIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_PMTIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_IMR_PMTIM_Wr_Mask (ULONG)(0xffffffef)

#define MAC_IMR_PMTIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_PMTIM_Wr_Mask) | ((data & MAC_IMR_PMTIM_Mask)<<4));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_PMTIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 4) & MAC_IMR_PMTIM_Mask);\
} while(0)

/*#define MAC_IMR_PHYIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_PHYIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_PHYIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_IMR_PHYIM_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_IMR_PHYIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_PHYIM_Wr_Mask) | ((data & MAC_IMR_PHYIM_Mask)<<3));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_PHYIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 3) & MAC_IMR_PHYIM_Mask);\
} while(0)

/*#define MAC_IMR_PCSANCIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_PCSANCIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_PCSANCIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_IMR_PCSANCIM_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_IMR_PCSANCIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_PCSANCIM_Wr_Mask) | ((data & MAC_IMR_PCSANCIM_Mask)<<2));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_PCSANCIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 2) & MAC_IMR_PCSANCIM_Mask);\
} while(0)

/*#define MAC_IMR_PCSLCHGIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_PCSLCHGIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_PCSLCHGIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_IMR_PCSLCHGIM_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_IMR_PCSLCHGIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_PCSLCHGIM_Wr_Mask) | ((data & MAC_IMR_PCSLCHGIM_Mask)<<1));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_PCSLCHGIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 1) & MAC_IMR_PCSLCHGIM_Mask);\
} while(0)

/*#define MAC_IMR_RGSMIIIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IMR_RGSMIIIM_Mask (ULONG)(0x1)

/*#define MAC_IMR_RGSMIIIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_IMR_RGSMIIIM_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_IMR_RGSMIIIM_UdfWr(data) do {\
		ULONG v;\
		MAC_IMR_RgRd(v);\
		v = (v & (MAC_IMR_RES_Wr_Mask_13))|((( 0) & (MAC_IMR_Mask_13))<<13);\
		v = (v & (MAC_IMR_RES_Wr_Mask_6))|((( 0) & (MAC_IMR_Mask_6))<<6);\
		v = ((v & MAC_IMR_RGSMIIIM_Wr_Mask) | ((data & MAC_IMR_RGSMIIIM_Mask)<<0));\
		MAC_IMR_RgWr(v);\
} while(0)

#define MAC_IMR_RGSMIIIM_UdfRd(data) do {\
		MAC_IMR_RgRd(data);\
		data = ((data >> 0) & MAC_IMR_RGSMIIIM_Mask);\
} while(0)


#define MAC_ISR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xb0))

#define MAC_ISR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_ISR_RgOffAddr);\
} while(0)

/*#define MAC_ISR_RWT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_RWT_Mask (ULONG)(0x1)

#define MAC_ISR_RWT_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 14) & MAC_ISR_RWT_Mask);\
} while(0)

/*#define MAC_ISR_TJT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_TJT_Mask (ULONG)(0x1)

#define MAC_ISR_TJT_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 13) & MAC_ISR_TJT_Mask);\
} while(0)

/*#define MAC_ISR_TSIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_TSIS_Mask (ULONG)(0x1)

#define MAC_ISR_TSIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 12) & MAC_ISR_TSIS_Mask);\
} while(0)

/*#define MAC_ISR_MMCRXIPIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_MMCRXIPIS_Mask (ULONG)(0x1)

#define MAC_ISR_MMCRXIPIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 11) & MAC_ISR_MMCRXIPIS_Mask);\
} while(0)

/*#define MAC_ISR_MMCTXIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_MMCTXIS_Mask (ULONG)(0x1)

#define MAC_ISR_MMCTXIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 10) & MAC_ISR_MMCTXIS_Mask);\
} while(0)

/*#define MAC_ISR_MMCRXIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_MMCRXIS_Mask (ULONG)(0x1)

#define MAC_ISR_MMCRXIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 9) & MAC_ISR_MMCRXIS_Mask);\
} while(0)

/*#define MAC_ISR_MMCIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_MMCIS_Mask (ULONG)(0x1)

#define MAC_ISR_MMCIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 8) & MAC_ISR_MMCIS_Mask);\
} while(0)

/*#define MAC_ISR_LPIIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_LPIIS_Mask (ULONG)(0x1)

#define MAC_ISR_LPIIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 5) & MAC_ISR_LPIIS_Mask);\
} while(0)

/*#define MAC_ISR_PMTIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_PMTIS_Mask (ULONG)(0x1)

#define MAC_ISR_PMTIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 4) & MAC_ISR_PMTIS_Mask);\
} while(0)

/*#define MAC_ISR_PHYIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_PHYIS_Mask (ULONG)(0x1)

#define MAC_ISR_PHYIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 3) & MAC_ISR_PHYIS_Mask);\
} while(0)

/*#define MAC_ISR_PCSANCIA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_PCSANCIA_Mask (ULONG)(0x1)

#define MAC_ISR_PCSANCIA_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 2) & MAC_ISR_PCSANCIA_Mask);\
} while(0)

/*#define MAC_ISR_PCSLCHGIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_PCSLCHGIS_Mask (ULONG)(0x1)

#define MAC_ISR_PCSLCHGIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 1) & MAC_ISR_PCSLCHGIS_Mask);\
} while(0)

/*#define MAC_ISR_RGSMIIIS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_ISR_RGSMIIIS_Mask (ULONG)(0x1)

#define MAC_ISR_RGSMIIIS_UdfRd(data) do {\
		MAC_ISR_RgRd(data);\
		data = ((data >> 0) & MAC_ISR_RGSMIIIS_Mask);\
} while(0)


#define MTL_ISR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc20))

#define MTL_ISR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_ISR_RgOffAddr);\
} while(0)

/*#define MTL_ISR_MACIS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_MACIS_Mask (ULONG)(0x1)

#define MTL_ISR_MACIS_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 16) & MTL_ISR_MACIS_Mask);\
} while(0)

/*#define MTL_ISR_Q7RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q7RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q7RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 15) & MTL_ISR_Q7RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q7TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q7TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q7TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 14) & MTL_ISR_Q7TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q6RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q6RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q6RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 13) & MTL_ISR_Q6RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q6TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q6TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q6TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 12) & MTL_ISR_Q6TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q5RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q5RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q5RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 11) & MTL_ISR_Q5RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q5TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q5TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q5TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 10) & MTL_ISR_Q5TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q4RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q4RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q4RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 9) & MTL_ISR_Q4RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q4TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q4TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q4TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 8) & MTL_ISR_Q4TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q3RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q3RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q3RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 7) & MTL_ISR_Q3RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q3TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q3TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q3TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 6) & MTL_ISR_Q3TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q2RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q2RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q2RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 5) & MTL_ISR_Q2RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q2TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q2TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q2TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 4) & MTL_ISR_Q2TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q1RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q1RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q1RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 3) & MTL_ISR_Q1RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q1TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q1TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q1TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 2) & MTL_ISR_Q1TXU_Mask);\
} while(0)

/*#define MTL_ISR_Q0RXO_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q0RXO_Mask (ULONG)(0x1)

#define MTL_ISR_Q0RXO_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 1) & MTL_ISR_Q0RXO_Mask);\
} while(0)

/*#define MTL_ISR_Q0TXU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_ISR_Q0TXU_Mask (ULONG)(0x1)

#define MTL_ISR_Q0TXU_UdfRd(data) do {\
		MTL_ISR_RgRd(data);\
		data = ((data >> 0) & MTL_ISR_Q0TXU_Mask);\
} while(0)




#define MTL_Q0RDR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd38))

#define MTL_Q0RDR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0RDR_RgOffAddr);\
} while(0)

/*#define MTL_Q0RDR_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_Q0RDR_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_Q0RDR_PRXQ_UdfRd(data) do {\
		MTL_Q0RDR_RgRd(data);\
		data = ((data >> 16) & MTL_Q0RDR_PRXQ_Mask);\
} while(0)

/*#define MTL_Q0RDR_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_Q0RDR_RXQSTS_Mask (ULONG)(0x3)

#define MTL_Q0RDR_RXQSTS_UdfRd(data) do {\
		MTL_Q0RDR_RgRd(data);\
		data = ((data >> 4) & MTL_Q0RDR_RXQSTS_Mask);\
} while(0)

/*#define MTL_Q0RDR_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_Q0RDR_RRCSTS_Mask (ULONG)(0x3)

#define MTL_Q0RDR_RRCSTS_UdfRd(data) do {\
		MTL_Q0RDR_RgRd(data);\
		data = ((data >> 1) & MTL_Q0RDR_RRCSTS_Mask);\
} while(0)

/*#define MTL_Q0RDR_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0RDR_RWCSTS_Mask (ULONG)(0x1)

#define MTL_Q0RDR_RWCSTS_UdfRd(data) do {\
		MTL_Q0RDR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0RDR_RWCSTS_Mask);\
} while(0)


#define MTL_Q0ESR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd14))

#define MTL_Q0ESR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0ESR_RgOffAddr);\
} while(0)

/*#define MTL_Q0ESR_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ESR_ABSU_Mask (ULONG)(0x1)

#define MTL_Q0ESR_ABSU_UdfRd(data) do {\
		MTL_Q0ESR_RgRd(data);\
		data = ((data >> 24) & MTL_Q0ESR_ABSU_Mask);\
} while(0)

/*#define MTL_Q0ESR_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_Q0ESR_ABS_Mask (ULONG)(0xffffff)

#define MTL_Q0ESR_ABS_UdfRd(data) do {\
		MTL_Q0ESR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0ESR_ABS_Mask);\
} while(0)


#define MTL_Q0TDR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd08))

#define MTL_Q0TDR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0TDR_RgOffAddr);\
} while(0)

/*#define MTL_Q0TDR_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_Q0TDR_STXSTSF_Mask (ULONG)(0x7)

#define MTL_Q0TDR_STXSTSF_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 20) & MTL_Q0TDR_STXSTSF_Mask);\
} while(0)

/*#define MTL_Q0TDR_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_Q0TDR_PTXQ_Mask (ULONG)(0x7)

#define MTL_Q0TDR_PTXQ_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 16) & MTL_Q0TDR_PTXQ_Mask);\
} while(0)

/*#define MTL_Q0TDR_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0TDR_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_Q0TDR_TXSTSFSTS_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 5) & MTL_Q0TDR_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_Q0TDR_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0TDR_TXQSTS_Mask (ULONG)(0x1)

#define MTL_Q0TDR_TXQSTS_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 4) & MTL_Q0TDR_TXQSTS_Mask);\
} while(0)

/*#define MTL_Q0TDR_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0TDR_TWCSTS_Mask (ULONG)(0x1)

#define MTL_Q0TDR_TWCSTS_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 3) & MTL_Q0TDR_TWCSTS_Mask);\
} while(0)

/*#define MTL_Q0TDR_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_Q0TDR_TRCSTS_Mask (ULONG)(0x3)

#define MTL_Q0TDR_TRCSTS_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 1) & MTL_Q0TDR_TRCSTS_Mask);\
} while(0)

/*#define MTL_Q0TDR_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0TDR_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_Q0TDR_TXQPAUSED_UdfRd(data) do {\
		MTL_Q0TDR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0TDR_TXQPAUSED_Mask);\
} while(0)




#define MAC_IVLANTIRR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x64))

#define MAC_IVLANTIRR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_IVLANTIRR_RgOffAddr);\
} while(0)

#define MAC_IVLANTIRR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_IVLANTIRR_RgOffAddr);\
} while(0)

/*#define  MAC_IVLANTIRR_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MAC_IVLANTIRR_Mask_21 (ULONG)(0x7ff)

/*#define MAC_IVLANTIRR_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MAC_IVLANTIRR_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MAC_IVLANTIRR_VLTI_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IVLANTIRR_VLTI_Mask (ULONG)(0x1)

/*#define MAC_IVLANTIRR_VLTI_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_IVLANTIRR_VLTI_Wr_Mask (ULONG)(0xffefffff)

#define MAC_IVLANTIRR_VLTI_UdfWr(data) do {\
		ULONG v;\
		MAC_IVLANTIRR_RgRd(v);\
		v = (v & (MAC_IVLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_IVLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_IVLANTIRR_VLTI_Wr_Mask) | ((data & MAC_IVLANTIRR_VLTI_Mask)<<20));\
		MAC_IVLANTIRR_RgWr(v);\
} while(0)

#define MAC_IVLANTIRR_VLTI_UdfRd(data) do {\
		MAC_IVLANTIRR_RgRd(data);\
		data = ((data >> 20) & MAC_IVLANTIRR_VLTI_Mask);\
} while(0)

/*#define MAC_IVLANTIRR_CSVL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IVLANTIRR_CSVL_Mask (ULONG)(0x1)

/*#define MAC_IVLANTIRR_CSVL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_IVLANTIRR_CSVL_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_IVLANTIRR_CSVL_UdfWr(data) do {\
		ULONG v;\
		MAC_IVLANTIRR_RgRd(v);\
		v = (v & (MAC_IVLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_IVLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_IVLANTIRR_CSVL_Wr_Mask) | ((data & MAC_IVLANTIRR_CSVL_Mask)<<19));\
		MAC_IVLANTIRR_RgWr(v);\
} while(0)

#define MAC_IVLANTIRR_CSVL_UdfRd(data) do {\
		MAC_IVLANTIRR_RgRd(data);\
		data = ((data >> 19) & MAC_IVLANTIRR_CSVL_Mask);\
} while(0)

/*#define MAC_IVLANTIRR_VLP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_IVLANTIRR_VLP_Mask (ULONG)(0x1)

/*#define MAC_IVLANTIRR_VLP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_IVLANTIRR_VLP_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_IVLANTIRR_VLP_UdfWr(data) do {\
		ULONG v;\
		MAC_IVLANTIRR_RgRd(v);\
		v = (v & (MAC_IVLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_IVLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_IVLANTIRR_VLP_Wr_Mask) | ((data & MAC_IVLANTIRR_VLP_Mask)<<18));\
		MAC_IVLANTIRR_RgWr(v);\
} while(0)

#define MAC_IVLANTIRR_VLP_UdfRd(data) do {\
		MAC_IVLANTIRR_RgRd(data);\
		data = ((data >> 18) & MAC_IVLANTIRR_VLP_Mask);\
} while(0)

/*#define MAC_IVLANTIRR_VLC_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_IVLANTIRR_VLC_Mask (ULONG)(0x3)

/*#define MAC_IVLANTIRR_VLC_Wr_Mask (ULONG)(~((~(~0 << (2))) << (16)))*/

#define MAC_IVLANTIRR_VLC_Wr_Mask (ULONG)(0xfffcffff)

#define MAC_IVLANTIRR_VLC_UdfWr(data) do {\
		ULONG v;\
		MAC_IVLANTIRR_RgRd(v);\
		v = (v & (MAC_IVLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_IVLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_IVLANTIRR_VLC_Wr_Mask) | ((data & MAC_IVLANTIRR_VLC_Mask)<<16));\
		MAC_IVLANTIRR_RgWr(v);\
} while(0)

#define MAC_IVLANTIRR_VLC_UdfRd(data) do {\
		MAC_IVLANTIRR_RgRd(data);\
		data = ((data >> 16) & MAC_IVLANTIRR_VLC_Mask);\
} while(0)

/*#define MAC_IVLANTIRR_VLT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_IVLANTIRR_VLT_Mask (ULONG)(0xffff)

/*#define MAC_IVLANTIRR_VLT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_IVLANTIRR_VLT_Wr_Mask (ULONG)(0xffff0000)

#define MAC_IVLANTIRR_VLT_UdfWr(data) do {\
		ULONG v;\
		MAC_IVLANTIRR_RgRd(v);\
		v = (v & (MAC_IVLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_IVLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_IVLANTIRR_VLT_Wr_Mask) | ((data & MAC_IVLANTIRR_VLT_Mask)<<0));\
		MAC_IVLANTIRR_RgWr(v);\
} while(0)

#define MAC_IVLANTIRR_VLT_UdfRd(data) do {\
		MAC_IVLANTIRR_RgRd(data);\
		data = ((data >> 0) & MAC_IVLANTIRR_VLT_Mask);\
} while(0)


#define MAC_VLANTIRR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x60))

#define MAC_VLANTIRR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_VLANTIRR_RgOffAddr);\
} while(0)

#define MAC_VLANTIRR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_VLANTIRR_RgOffAddr);\
} while(0)

/*#define  MAC_VLANTIRR_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MAC_VLANTIRR_Mask_21 (ULONG)(0x7ff)

/*#define MAC_VLANTIRR_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MAC_VLANTIRR_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MAC_VLANTIRR_VLTI_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTIRR_VLTI_Mask (ULONG)(0x1)

/*#define MAC_VLANTIRR_VLTI_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_VLANTIRR_VLTI_Wr_Mask (ULONG)(0xffefffff)

#define MAC_VLANTIRR_VLTI_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTIRR_RgRd(v);\
		v = (v & (MAC_VLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_VLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_VLANTIRR_VLTI_Wr_Mask) | ((data & MAC_VLANTIRR_VLTI_Mask)<<20));\
		MAC_VLANTIRR_RgWr(v);\
} while(0)

#define MAC_VLANTIRR_VLTI_UdfRd(data) do {\
		MAC_VLANTIRR_RgRd(data);\
		data = ((data >> 20) & MAC_VLANTIRR_VLTI_Mask);\
} while(0)

/*#define MAC_VLANTIRR_CSVL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTIRR_CSVL_Mask (ULONG)(0x1)

/*#define MAC_VLANTIRR_CSVL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_VLANTIRR_CSVL_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_VLANTIRR_CSVL_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTIRR_RgRd(v);\
		v = (v & (MAC_VLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_VLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_VLANTIRR_CSVL_Wr_Mask) | ((data & MAC_VLANTIRR_CSVL_Mask)<<19));\
		MAC_VLANTIRR_RgWr(v);\
} while(0)

#define MAC_VLANTIRR_CSVL_UdfRd(data) do {\
		MAC_VLANTIRR_RgRd(data);\
		data = ((data >> 19) & MAC_VLANTIRR_CSVL_Mask);\
} while(0)

/*#define MAC_VLANTIRR_VLP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTIRR_VLP_Mask (ULONG)(0x1)

/*#define MAC_VLANTIRR_VLP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_VLANTIRR_VLP_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_VLANTIRR_VLP_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTIRR_RgRd(v);\
		v = (v & (MAC_VLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_VLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_VLANTIRR_VLP_Wr_Mask) | ((data & MAC_VLANTIRR_VLP_Mask)<<18));\
		MAC_VLANTIRR_RgWr(v);\
} while(0)

#define MAC_VLANTIRR_VLP_UdfRd(data) do {\
		MAC_VLANTIRR_RgRd(data);\
		data = ((data >> 18) & MAC_VLANTIRR_VLP_Mask);\
} while(0)

/*#define MAC_VLANTIRR_VLC_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_VLANTIRR_VLC_Mask (ULONG)(0x3)

/*#define MAC_VLANTIRR_VLC_Wr_Mask (ULONG)(~((~(~0 << (2))) << (16)))*/

#define MAC_VLANTIRR_VLC_Wr_Mask (ULONG)(0xfffcffff)

#define MAC_VLANTIRR_VLC_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTIRR_RgRd(v);\
		v = (v & (MAC_VLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_VLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_VLANTIRR_VLC_Wr_Mask) | ((data & MAC_VLANTIRR_VLC_Mask)<<16));\
		MAC_VLANTIRR_RgWr(v);\
} while(0)

#define MAC_VLANTIRR_VLC_UdfRd(data) do {\
		MAC_VLANTIRR_RgRd(data);\
		data = ((data >> 16) & MAC_VLANTIRR_VLC_Mask);\
} while(0)

/*#define MAC_VLANTIRR_VLT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_VLANTIRR_VLT_Mask (ULONG)(0xffff)

/*#define MAC_VLANTIRR_VLT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_VLANTIRR_VLT_Wr_Mask (ULONG)(0xffff0000)

#define MAC_VLANTIRR_VLT_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTIRR_RgRd(v);\
		v = (v & (MAC_VLANTIRR_RES_Wr_Mask_21))|((( 0) & (MAC_VLANTIRR_Mask_21))<<21);\
		v = ((v & MAC_VLANTIRR_VLT_Wr_Mask) | ((data & MAC_VLANTIRR_VLT_Mask)<<0));\
		MAC_VLANTIRR_RgWr(v);\
} while(0)

#define MAC_VLANTIRR_VLT_UdfRd(data) do {\
		MAC_VLANTIRR_RgRd(data);\
		data = ((data >> 0) & MAC_VLANTIRR_VLT_Mask);\
} while(0)


#define MAC_VLANHTR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x58))

#define MAC_VLANHTR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_VLANHTR_RgOffAddr);\
} while(0)

#define MAC_VLANHTR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_VLANHTR_RgOffAddr);\
} while(0)

/*#define  MAC_VLANHTR_Mask_16 (ULONG)(~(~0<<(16)))*/

#define  MAC_VLANHTR_Mask_16 (ULONG)(0xffff)

/*#define MAC_VLANHTR_RES_Wr_Mask_16 (ULONG)(~((~(~0<<(16)))<<(16)))*/

#define MAC_VLANHTR_RES_Wr_Mask_16 (ULONG)(0xffff)

/*#define MAC_VLANHTR_VLHT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_VLANHTR_VLHT_Mask (ULONG)(0xffff)

/*#define MAC_VLANHTR_VLHT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_VLANHTR_VLHT_Wr_Mask (ULONG)(0xffff0000)

#define MAC_VLANHTR_VLHT_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MAC_VLANHTR_RES_Wr_Mask_16))|((( 0) & (MAC_VLANHTR_Mask_16))<<16);\
		(v) = ((v & MAC_VLANHTR_VLHT_Wr_Mask) | ((data & MAC_VLANHTR_VLHT_Mask)<<0));\
		MAC_VLANHTR_RgWr(v);\
} while(0)

#define MAC_VLANHTR_VLHT_UdfRd(data) do {\
		MAC_VLANHTR_RgRd(data);\
		data = ((data >> 0) & MAC_VLANHTR_VLHT_Mask);\
} while(0)


#define MAC_VLANTR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x50))

#define MAC_VLANTR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_VLANTR_RgOffAddr);\
} while(0)

#define MAC_VLANTR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_VLANTR_RgOffAddr);\
} while(0)

/*#define  MAC_VLANTR_Mask_30 (ULONG)(~(~0<<(1)))*/

#define  MAC_VLANTR_Mask_30 (ULONG)(0x1)

/*#define MAC_VLANTR_RES_Wr_Mask_30 (ULONG)(~((~(~0<<(1)))<<(30)))*/

#define MAC_VLANTR_RES_Wr_Mask_30 (ULONG)(0xbfffffff)

/*#define  MAC_VLANTR_Mask_23 (ULONG)(~(~0<<(1)))*/

#define  MAC_VLANTR_Mask_23 (ULONG)(0x1)

/*#define MAC_VLANTR_RES_Wr_Mask_23 (ULONG)(~((~(~0<<(1)))<<(23)))*/

#define MAC_VLANTR_RES_Wr_Mask_23 (ULONG)(0xff7fffff)

/*#define MAC_VLANTR_EIVLRXS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_EIVLRXS_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_EIVLRXS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_VLANTR_EIVLRXS_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_VLANTR_EIVLRXS_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_EIVLRXS_Wr_Mask) | ((data & MAC_VLANTR_EIVLRXS_Mask)<<31));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_EIVLRXS_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 31) & MAC_VLANTR_EIVLRXS_Mask);\
} while(0)

/*#define MAC_VLANTR_EIVLS_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_VLANTR_EIVLS_Mask (ULONG)(0x3)

/*#define MAC_VLANTR_EIVLS_Wr_Mask (ULONG)(~((~(~0 << (2))) << (28)))*/

#define MAC_VLANTR_EIVLS_Wr_Mask (ULONG)(0xcfffffff)

#define MAC_VLANTR_EIVLS_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_EIVLS_Wr_Mask) | ((data & MAC_VLANTR_EIVLS_Mask)<<28));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_EIVLS_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 28) & MAC_VLANTR_EIVLS_Mask);\
} while(0)

/*#define MAC_VLANTR_ERIVLT_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_ERIVLT_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_ERIVLT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MAC_VLANTR_ERIVLT_Wr_Mask (ULONG)(0xf7ffffff)

#define MAC_VLANTR_ERIVLT_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_ERIVLT_Wr_Mask) | ((data & MAC_VLANTR_ERIVLT_Mask)<<27));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_ERIVLT_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 27) & MAC_VLANTR_ERIVLT_Mask);\
} while(0)

/*#define MAC_VLANTR_EDVLP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_EDVLP_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_EDVLP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (26)))*/

#define MAC_VLANTR_EDVLP_Wr_Mask (ULONG)(0xfbffffff)

#define MAC_VLANTR_EDVLP_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_EDVLP_Wr_Mask) | ((data & MAC_VLANTR_EDVLP_Mask)<<26));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_EDVLP_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 26) & MAC_VLANTR_EDVLP_Mask);\
} while(0)

/*#define MAC_VLANTR_VTHM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_VTHM_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_VTHM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (25)))*/

#define MAC_VLANTR_VTHM_Wr_Mask (ULONG)(0xfdffffff)

#define MAC_VLANTR_VTHM_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_VTHM_Wr_Mask) | ((data & MAC_VLANTR_VTHM_Mask)<<25));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_VTHM_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 25) & MAC_VLANTR_VTHM_Mask);\
} while(0)

/*#define MAC_VLANTR_EVLRXS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_EVLRXS_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_EVLRXS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MAC_VLANTR_EVLRXS_Wr_Mask (ULONG)(0xfeffffff)

#define MAC_VLANTR_EVLRXS_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_EVLRXS_Wr_Mask) | ((data & MAC_VLANTR_EVLRXS_Mask)<<24));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_EVLRXS_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 24) & MAC_VLANTR_EVLRXS_Mask);\
} while(0)

/*#define MAC_VLANTR_EVLS_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_VLANTR_EVLS_Mask (ULONG)(0x3)

/*#define MAC_VLANTR_EVLS_Wr_Mask (ULONG)(~((~(~0 << (2))) << (21)))*/

#define MAC_VLANTR_EVLS_Wr_Mask (ULONG)(0xff9fffff)

#define MAC_VLANTR_EVLS_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_EVLS_Wr_Mask) | ((data & MAC_VLANTR_EVLS_Mask)<<21));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_EVLS_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 21) & MAC_VLANTR_EVLS_Mask);\
} while(0)

/*#define MAC_VLANTR_DOVLTC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_DOVLTC_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_DOVLTC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_VLANTR_DOVLTC_Wr_Mask (ULONG)(0xffefffff)

#define MAC_VLANTR_DOVLTC_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_DOVLTC_Wr_Mask) | ((data & MAC_VLANTR_DOVLTC_Mask)<<20));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_DOVLTC_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 20) & MAC_VLANTR_DOVLTC_Mask);\
} while(0)

/*#define MAC_VLANTR_ERSVLM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_ERSVLM_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_ERSVLM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_VLANTR_ERSVLM_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_VLANTR_ERSVLM_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_ERSVLM_Wr_Mask) | ((data & MAC_VLANTR_ERSVLM_Mask)<<19));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_ERSVLM_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 19) & MAC_VLANTR_ERSVLM_Mask);\
} while(0)

/*#define MAC_VLANTR_ESVL_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_ESVL_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_ESVL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_VLANTR_ESVL_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_VLANTR_ESVL_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_ESVL_Wr_Mask) | ((data & MAC_VLANTR_ESVL_Mask)<<18));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_ESVL_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 18) & MAC_VLANTR_ESVL_Mask);\
} while(0)

/*#define MAC_VLANTR_VTIM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_VTIM_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_VTIM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MAC_VLANTR_VTIM_Wr_Mask (ULONG)(0xfffdffff)

#define MAC_VLANTR_VTIM_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_VTIM_Wr_Mask) | ((data & MAC_VLANTR_VTIM_Mask)<<17));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_VTIM_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 17) & MAC_VLANTR_VTIM_Mask);\
} while(0)

/*#define MAC_VLANTR_ETV_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_VLANTR_ETV_Mask (ULONG)(0x1)

/*#define MAC_VLANTR_ETV_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_VLANTR_ETV_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_VLANTR_ETV_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_ETV_Wr_Mask) | ((data & MAC_VLANTR_ETV_Mask)<<16));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_ETV_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 16) & MAC_VLANTR_ETV_Mask);\
} while(0)

/*#define MAC_VLANTR_VL_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_VLANTR_VL_Mask (ULONG)(0xffff)

/*#define MAC_VLANTR_VL_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_VLANTR_VL_Wr_Mask (ULONG)(0xffff0000)

#define MAC_VLANTR_VL_UdfWr(data) do {\
		ULONG v;\
		MAC_VLANTR_RgRd(v);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_30))|((( 0) & (MAC_VLANTR_Mask_30))<<30);\
		v = (v & (MAC_VLANTR_RES_Wr_Mask_23))|((( 0) & (MAC_VLANTR_Mask_23))<<23);\
		v = ((v & MAC_VLANTR_VL_Wr_Mask) | ((data & MAC_VLANTR_VL_Mask)<<0));\
		MAC_VLANTR_RgWr(v);\
} while(0)

#define MAC_VLANTR_VL_UdfRd(data) do {\
		MAC_VLANTR_RgRd(data);\
		data = ((data >> 0) & MAC_VLANTR_VL_Mask);\
} while(0)




#define MTL_Q0RCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd3c))

#define MTL_Q0RCR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_Q0RCR_RgOffAddr);\
} while(0)

#define MTL_Q0RCR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0RCR_RgOffAddr);\
} while(0)

/*#define  MTL_Q0RCR_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_Q0RCR_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_Q0RCR_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_Q0RCR_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_Q0RCR_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0RCR_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_Q0RCR_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_Q0RCR_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_Q0RCR_RXQ_PKT_ARBIT_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0RCR_RgRd(v);\
		v = (v & (MTL_Q0RCR_RES_Wr_Mask_4))|((( 0) & (MTL_Q0RCR_Mask_4))<<4);\
		v = ((v & MTL_Q0RCR_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_Q0RCR_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_Q0RCR_RgWr(v);\
} while(0)

#define MTL_Q0RCR_RXQ_PKT_ARBIT_UdfRd(data) do {\
		MTL_Q0RCR_RgRd(data);\
		data = ((data >> 3) & MTL_Q0RCR_RXQ_PKT_ARBIT_Mask);\
} while(0)

/*#define MTL_Q0RCR_RQW_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_Q0RCR_RQW_Mask (ULONG)(0x7)

/*#define MTL_Q0RCR_RQW_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_Q0RCR_RQW_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_Q0RCR_RQW_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0RCR_RgRd(v);\
		v = (v & (MTL_Q0RCR_RES_Wr_Mask_4))|((( 0) & (MTL_Q0RCR_Mask_4))<<4);\
		v = ((v & MTL_Q0RCR_RQW_Wr_Mask) | ((data & MTL_Q0RCR_RQW_Mask)<<0));\
		MTL_Q0RCR_RgWr(v);\
} while(0)

#define MTL_Q0RCR_RQW_UdfRd(data) do {\
		MTL_Q0RCR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0RCR_RQW_Mask);\
} while(0)


#define MTL_Q0OCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd34))

#define MTL_Q0OCR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0OCR_RgOffAddr);\
} while(0)

/*#define MTL_Q0OCR_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0OCR_MISCNTOVF_Mask (ULONG)(0x1)

#define MTL_Q0OCR_MISCNTOVF_UdfRd(data) do {\
		MTL_Q0OCR_RgRd(data);\
		data = ((data >> 27) & MTL_Q0OCR_MISCNTOVF_Mask);\
} while(0)

/*#define MTL_Q0OCR_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_Q0OCR_MISPKTCNT_Mask (ULONG)(0x7ff)

#define MTL_Q0OCR_MISPKTCNT_UdfRd(data) do {\
		MTL_Q0OCR_RgRd(data);\
		data = ((data >> 16) & MTL_Q0OCR_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_Q0OCR_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0OCR_OVFCNTOVF_Mask (ULONG)(0x1)

#define MTL_Q0OCR_OVFCNTOVF_UdfRd(data) do {\
		MTL_Q0OCR_RgRd(data);\
		data = ((data >> 11) & MTL_Q0OCR_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_Q0OCR_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_Q0OCR_OVFPKTCNT_Mask (ULONG)(0x7ff)

#define MTL_Q0OCR_OVFPKTCNT_UdfRd(data) do {\
		MTL_Q0OCR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0OCR_OVFPKTCNT_Mask);\
} while(0)


#define MTL_Q0ROMR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd30))

#define MTL_Q0ROMR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_Q0ROMR_RgOffAddr);\
} while(0)

#define MTL_Q0ROMR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0ROMR_RgOffAddr);\
} while(0)

/*#define  MTL_Q0ROMR_Mask_30 (ULONG)(~(~0<<(2)))*/

#define  MTL_Q0ROMR_Mask_30 (ULONG)(0x3)

/*#define MTL_Q0ROMR_RES_Wr_Mask_30 (ULONG)(~((~(~0<<(2)))<<(30)))*/

#define MTL_Q0ROMR_RES_Wr_Mask_30 (ULONG)(0x3fffffff)

/*#define  MTL_Q0ROMR_Mask_16 (ULONG)(~(~0<<(4)))*/

#define  MTL_Q0ROMR_Mask_16 (ULONG)(0xf)

/*#define MTL_Q0ROMR_RES_Wr_Mask_16 (ULONG)(~((~(~0<<(4)))<<(16)))*/

#define MTL_Q0ROMR_RES_Wr_Mask_16 (ULONG)(0xfff0ffff)

/*#define  MTL_Q0ROMR_Mask_11 (ULONG)(~(~0<<(2)))*/

#define  MTL_Q0ROMR_Mask_11 (ULONG)(0x3)

/*#define MTL_Q0ROMR_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(2)))<<(11)))*/

#define MTL_Q0ROMR_RES_Wr_Mask_11 (ULONG)(0xffffe7ff)

/*#define  MTL_Q0ROMR_Mask_2 (ULONG)(~(~0<<(1)))*/

#define  MTL_Q0ROMR_Mask_2 (ULONG)(0x1)

/*#define MTL_Q0ROMR_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(1)))<<(2)))*/

#define MTL_Q0ROMR_RES_Wr_Mask_2 (ULONG)(0xfffffffb)

/*#define MTL_Q0ROMR_RQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_Q0ROMR_RQS_Mask (ULONG)(0x3ff)

/*#define MTL_Q0ROMR_RQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (20)))*/

#define MTL_Q0ROMR_RQS_Wr_Mask (ULONG)(0xc00fffff)

#define MTL_Q0ROMR_RQS_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_RQS_Wr_Mask) | ((data & MTL_Q0ROMR_RQS_Mask)<<20));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_RQS_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 20) & MTL_Q0ROMR_RQS_Mask);\
} while(0)

/*#define MTL_Q0ROMR_RFD_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_Q0ROMR_RFD_Mask (ULONG)(0x7)

/*#define MTL_Q0ROMR_RFD_Wr_Mask (ULONG)(~((~(~0 << (3))) << (13)))*/

#define MTL_Q0ROMR_RFD_Wr_Mask (ULONG)(0xffff1fff)

#define MTL_Q0ROMR_RFD_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_RFD_Wr_Mask) | ((data & MTL_Q0ROMR_RFD_Mask)<<13));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_RFD_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 13) & MTL_Q0ROMR_RFD_Mask);\
} while(0)

/*#define MTL_Q0ROMR_RFA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_Q0ROMR_RFA_Mask (ULONG)(0x7)

/*#define MTL_Q0ROMR_RFA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (8)))*/

#define MTL_Q0ROMR_RFA_Wr_Mask (ULONG)(0xfffff8ff)

#define MTL_Q0ROMR_RFA_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_RFA_Wr_Mask) | ((data & MTL_Q0ROMR_RFA_Mask)<<8));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_RFA_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 8) & MTL_Q0ROMR_RFA_Mask);\
} while(0)

/*#define MTL_Q0ROMR_EFC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ROMR_EFC_Mask (ULONG)(0x1)

/*#define MTL_Q0ROMR_EFC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MTL_Q0ROMR_EFC_Wr_Mask (ULONG)(0xffffff7f)

#define MTL_Q0ROMR_EFC_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_EFC_Wr_Mask) | ((data & MTL_Q0ROMR_EFC_Mask)<<7));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_EFC_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 7) & MTL_Q0ROMR_EFC_Mask);\
} while(0)

/*#define MTL_Q0ROMR_DT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ROMR_DT_Mask (ULONG)(0x1)

/*#define MTL_Q0ROMR_DT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MTL_Q0ROMR_DT_Wr_Mask (ULONG)(0xffffffbf)

#define MTL_Q0ROMR_DT_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_DT_Wr_Mask) | ((data & MTL_Q0ROMR_DT_Mask)<<6));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_DT_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 6) & MTL_Q0ROMR_DT_Mask);\
} while(0)

/*#define MTL_Q0ROMR_RSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ROMR_RSF_Mask (ULONG)(0x1)

/*#define MTL_Q0ROMR_RSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MTL_Q0ROMR_RSF_Wr_Mask (ULONG)(0xffffffdf)

#define MTL_Q0ROMR_RSF_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_RSF_Wr_Mask) | ((data & MTL_Q0ROMR_RSF_Mask)<<5));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_RSF_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 5) & MTL_Q0ROMR_RSF_Mask);\
} while(0)

/*#define MTL_Q0ROMR_FEP_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ROMR_FEP_Mask (ULONG)(0x1)

/*#define MTL_Q0ROMR_FEP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MTL_Q0ROMR_FEP_Wr_Mask (ULONG)(0xffffffef)

#define MTL_Q0ROMR_FEP_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_FEP_Wr_Mask) | ((data & MTL_Q0ROMR_FEP_Mask)<<4));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_FEP_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 4) & MTL_Q0ROMR_FEP_Mask);\
} while(0)

/*#define MTL_Q0ROMR_FUP_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ROMR_FUP_Mask (ULONG)(0x1)

/*#define MTL_Q0ROMR_FUP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_Q0ROMR_FUP_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_Q0ROMR_FUP_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_FUP_Wr_Mask) | ((data & MTL_Q0ROMR_FUP_Mask)<<3));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_FUP_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 3) & MTL_Q0ROMR_FUP_Mask);\
} while(0)

/*#define MTL_Q0ROMR_RTC_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_Q0ROMR_RTC_Mask (ULONG)(0x3)

/*#define MTL_Q0ROMR_RTC_Wr_Mask (ULONG)(~((~(~0 << (2))) << (0)))*/

#define MTL_Q0ROMR_RTC_Wr_Mask (ULONG)(0xfffffffc)

#define MTL_Q0ROMR_RTC_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0ROMR_RgRd(v);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_30))|((( 0) & (MTL_Q0ROMR_Mask_30))<<30);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_16))|((( 0) & (MTL_Q0ROMR_Mask_16))<<16);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_11))|((( 0) & (MTL_Q0ROMR_Mask_11))<<11);\
		v = (v & (MTL_Q0ROMR_RES_Wr_Mask_2))|((( 0) & (MTL_Q0ROMR_Mask_2))<<2);\
		v = ((v & MTL_Q0ROMR_RTC_Wr_Mask) | ((data & MTL_Q0ROMR_RTC_Mask)<<0));\
		MTL_Q0ROMR_RgWr(v);\
} while(0)

#define MTL_Q0ROMR_RTC_UdfRd(data) do {\
		MTL_Q0ROMR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0ROMR_RTC_Mask);\
} while(0)


#define MTL_Q0QR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd18))

#define MTL_Q0QR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_Q0QR_RgOffAddr);\
} while(0)

#define MTL_Q0QR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0QR_RgOffAddr);\
} while(0)

/*#define  MTL_Q0QR_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_Q0QR_Mask_21 (ULONG)(0x7ff)

/*#define MTL_Q0QR_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_Q0QR_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_Q0QR_QW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_Q0QR_QW_Mask (ULONG)(0x1fffff)

/*#define MTL_Q0QR_QW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_Q0QR_QW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_Q0QR_QW_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_Q0QR_RES_Wr_Mask_21))|((( 0) & (MTL_Q0QR_Mask_21))<<21);\
		(v) = ((v & MTL_Q0QR_QW_Wr_Mask) | ((data & MTL_Q0QR_QW_Mask)<<0));\
		MTL_Q0QR_RgWr(v);\
} while(0)

#define MTL_Q0QR_QW_UdfRd(data) do {\
		MTL_Q0QR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0QR_QW_Mask);\
} while(0)


#define MTL_Q0ECR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd10))

#define MTL_Q0ECR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_Q0ECR_RgOffAddr);\
} while(0)

#define MTL_Q0ECR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0ECR_RgOffAddr);\
} while(0)

/*#define  MTL_Q0ECR_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_Q0ECR_Mask_25 (ULONG)(0x7f)

/*#define MTL_Q0ECR_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_Q0ECR_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define  MTL_Q0ECR_Mask_0 (ULONG)(~(~0<<(24)))*/

#define  MTL_Q0ECR_Mask_0 (ULONG)(0xffffff)

/*#define MTL_Q0ECR_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(24)))<<(0)))*/

#define MTL_Q0ECR_RES_Wr_Mask_0 (ULONG)(0xff000000)

/*#define MTL_Q0ECR_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0ECR_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_Q0ECR_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_Q0ECR_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_Q0ECR_ABPSSIE_UdfWr(data) do {\
		ULONG v = 0; \
		v = (v & (MTL_Q0ECR_RES_Wr_Mask_25))|((( 0) & (MTL_Q0ECR_Mask_25))<<25);\
		v = (v & (MTL_Q0ECR_RES_Wr_Mask_0))|((( 0) & (MTL_Q0ECR_Mask_0))<<0);\
		(v) = ((v & MTL_Q0ECR_ABPSSIE_Wr_Mask) | ((data & MTL_Q0ECR_ABPSSIE_Mask)<<24));\
		MTL_Q0ECR_RgWr(v);\
} while(0)

#define MTL_Q0ECR_ABPSSIE_UdfRd(data) do {\
		MTL_Q0ECR_RgRd(data);\
		data = ((data >> 24) & MTL_Q0ECR_ABPSSIE_Mask);\
} while(0)


#define MTL_Q0UCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd04))

#define MTL_Q0UCR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_Q0UCR_RgOffAddr);\
} while(0)

#define MTL_Q0UCR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0UCR_RgOffAddr);\
} while(0)

/*#define  MTL_Q0UCR_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_Q0UCR_Mask_12 (ULONG)(0xfffff)

/*#define MTL_Q0UCR_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_Q0UCR_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_Q0UCR_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0UCR_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_Q0UCR_UFCNTOVF_UdfRd(data) do {\
		MTL_Q0UCR_RgRd(data);\
		data = ((data >> 11) & MTL_Q0UCR_UFCNTOVF_Mask);\
} while(0)

/*#define MTL_Q0UCR_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_Q0UCR_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_Q0UCR_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_Q0UCR_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_Q0UCR_UFPKTCNT_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0UCR_RgRd(v);\
		v = (v & (MTL_Q0UCR_RES_Wr_Mask_12))|((( 0) & (MTL_Q0UCR_Mask_12))<<12);\
		v = ((v & MTL_Q0UCR_UFPKTCNT_Wr_Mask) | ((data & MTL_Q0UCR_UFPKTCNT_Mask)<<0));\
		MTL_Q0UCR_RgWr(v);\
} while(0)

#define MTL_Q0UCR_UFPKTCNT_UdfRd(data) do {\
		MTL_Q0UCR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0UCR_UFPKTCNT_Mask);\
} while(0)


#define MTL_Q0TOMR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xd00))

#define MTL_Q0TOMR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_Q0TOMR_RgOffAddr);\
} while(0)

#define MTL_Q0TOMR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_Q0TOMR_RgOffAddr);\
} while(0)

/*#define  MTL_Q0TOMR_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_Q0TOMR_Mask_26 (ULONG)(0x3f)

/*#define MTL_Q0TOMR_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_Q0TOMR_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define  MTL_Q0TOMR_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_Q0TOMR_Mask_7 (ULONG)(0x1ff)

/*#define MTL_Q0TOMR_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_Q0TOMR_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define MTL_Q0TOMR_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_Q0TOMR_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_Q0TOMR_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_Q0TOMR_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_Q0TOMR_TQS_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0TOMR_RgRd(v);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_26))|((( 0) & (MTL_Q0TOMR_Mask_26))<<26);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_7))|((( 0) & (MTL_Q0TOMR_Mask_7))<<7);\
		v = ((v & MTL_Q0TOMR_TQS_Wr_Mask) | ((data & MTL_Q0TOMR_TQS_Mask)<<16));\
		MTL_Q0TOMR_RgWr(v);\
} while(0)

#define MTL_Q0TOMR_TQS_UdfRd(data) do {\
		MTL_Q0TOMR_RgRd(data);\
		data = ((data >> 16) & MTL_Q0TOMR_TQS_Mask);\
} while(0)

/*#define MTL_Q0TOMR_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_Q0TOMR_TTC_Mask (ULONG)(0x7)

/*#define MTL_Q0TOMR_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_Q0TOMR_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_Q0TOMR_TTC_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0TOMR_RgRd(v);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_26))|((( 0) & (MTL_Q0TOMR_Mask_26))<<26);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_7))|((( 0) & (MTL_Q0TOMR_Mask_7))<<7);\
		v = ((v & MTL_Q0TOMR_TTC_Wr_Mask) | ((data & MTL_Q0TOMR_TTC_Mask)<<4));\
		MTL_Q0TOMR_RgWr(v);\
} while(0)

#define MTL_Q0TOMR_TTC_UdfRd(data) do {\
		MTL_Q0TOMR_RgRd(data);\
		data = ((data >> 4) & MTL_Q0TOMR_TTC_Mask);\
} while(0)

/*#define MTL_Q0TOMR_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_Q0TOMR_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_Q0TOMR_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_Q0TOMR_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_Q0TOMR_TXQEN_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0TOMR_RgRd(v);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_26))|((( 0) & (MTL_Q0TOMR_Mask_26))<<26);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_7))|((( 0) & (MTL_Q0TOMR_Mask_7))<<7);\
		v = ((v & MTL_Q0TOMR_TXQEN_Wr_Mask) | ((data & MTL_Q0TOMR_TXQEN_Mask)<<2));\
		MTL_Q0TOMR_RgWr(v);\
} while(0)

#define MTL_Q0TOMR_TXQEN_UdfRd(data) do {\
		MTL_Q0TOMR_RgRd(data);\
		data = ((data >> 2) & MTL_Q0TOMR_TXQEN_Mask);\
} while(0)

/*#define MTL_Q0TOMR_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0TOMR_TSF_Mask (ULONG)(0x1)

/*#define MTL_Q0TOMR_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_Q0TOMR_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_Q0TOMR_TSF_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0TOMR_RgRd(v);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_26))|((( 0) & (MTL_Q0TOMR_Mask_26))<<26);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_7))|((( 0) & (MTL_Q0TOMR_Mask_7))<<7);\
		v = ((v & MTL_Q0TOMR_TSF_Wr_Mask) | ((data & MTL_Q0TOMR_TSF_Mask)<<1));\
		MTL_Q0TOMR_RgWr(v);\
} while(0)

#define MTL_Q0TOMR_TSF_UdfRd(data) do {\
		MTL_Q0TOMR_RgRd(data);\
		data = ((data >> 1) & MTL_Q0TOMR_TSF_Mask);\
} while(0)

/*#define MTL_Q0TOMR_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_Q0TOMR_FTQ_Mask (ULONG)(0x1)

/*#define MTL_Q0TOMR_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_Q0TOMR_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_Q0TOMR_FTQ_UdfWr(data) do {\
		ULONG v;\
		MTL_Q0TOMR_RgRd(v);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_26))|((( 0) & (MTL_Q0TOMR_Mask_26))<<26);\
		v = (v & (MTL_Q0TOMR_RES_Wr_Mask_7))|((( 0) & (MTL_Q0TOMR_Mask_7))<<7);\
		v = ((v & MTL_Q0TOMR_FTQ_Wr_Mask) | ((data & MTL_Q0TOMR_FTQ_Mask)<<0));\
		MTL_Q0TOMR_RgWr(v);\
} while(0)

#define MTL_Q0TOMR_FTQ_UdfRd(data) do {\
		MTL_Q0TOMR_RgRd(data);\
		data = ((data >> 0) & MTL_Q0TOMR_FTQ_Mask);\
} while(0)


#define MTL_RQDCM1R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc34))

#define MTL_RQDCM1R_RgWr(data) do {\
		iowrite32(data, (void *)MTL_RQDCM1R_RgOffAddr);\
} while(0)

#define MTL_RQDCM1R_RgRd(data) do {\
		(data) = ioread32((void *)MTL_RQDCM1R_RgOffAddr);\
} while(0)

/*#define  MTL_RQDCM1R_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM1R_Mask_29 (ULONG)(0x7)

/*#define MTL_RQDCM1R_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define  MTL_RQDCM1R_Mask_27 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM1R_Mask_27 (ULONG)(0x1)

/*#define MTL_RQDCM1R_RES_Wr_Mask_27 (ULONG)(~((~(~0<<(1)))<<(27)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_27 (ULONG)(0xf7ffffff)

/*#define  MTL_RQDCM1R_Mask_21 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM1R_Mask_21 (ULONG)(0x7)

/*#define MTL_RQDCM1R_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(3)))<<(21)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_21 (ULONG)(0xff1fffff)

/*#define  MTL_RQDCM1R_Mask_19 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM1R_Mask_19 (ULONG)(0x1)

/*#define MTL_RQDCM1R_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(1)))<<(19)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_19 (ULONG)(0xfff7ffff)

/*#define  MTL_RQDCM1R_Mask_13 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM1R_Mask_13 (ULONG)(0x7)

/*#define MTL_RQDCM1R_RES_Wr_Mask_13 (ULONG)(~((~(~0<<(3)))<<(13)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_13 (ULONG)(0xffff1fff)

/*#define  MTL_RQDCM1R_Mask_11 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM1R_Mask_11 (ULONG)(0x1)

/*#define MTL_RQDCM1R_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(1)))<<(11)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_11 (ULONG)(0xfffff7ff)

/*#define  MTL_RQDCM1R_Mask_5 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM1R_Mask_5 (ULONG)(0x7)

/*#define MTL_RQDCM1R_RES_Wr_Mask_5 (ULONG)(~((~(~0<<(3)))<<(5)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_5 (ULONG)(0xffffff1f)

/*#define  MTL_RQDCM1R_Mask_3 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM1R_Mask_3 (ULONG)(0x1)

/*#define MTL_RQDCM1R_RES_Wr_Mask_3 (ULONG)(~((~(~0<<(1)))<<(3)))*/

#define MTL_RQDCM1R_RES_Wr_Mask_3 (ULONG)(0xfffffff7)

/*#define MTL_RQDCM1R_RXQ7DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM1R_RXQ7DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM1R_RXQ7DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (28)))*/

#define MTL_RQDCM1R_RXQ7DADMACH_Wr_Mask (ULONG)(0xefffffff)

#define MTL_RQDCM1R_RXQ7DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ7DADMACH_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ7DADMACH_Mask)<<28));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ7DADMACH_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 28) & MTL_RQDCM1R_RXQ7DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ72DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM1R_RXQ72DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM1R_RXQ72DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (24)))*/

#define MTL_RQDCM1R_RXQ72DMA_Wr_Mask (ULONG)(0xf8ffffff)

#define MTL_RQDCM1R_RXQ72DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ72DMA_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ72DMA_Mask)<<24));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ72DMA_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 24) & MTL_RQDCM1R_RXQ72DMA_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ6DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM1R_RXQ6DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM1R_RXQ6DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MTL_RQDCM1R_RXQ6DADMACH_Wr_Mask (ULONG)(0xffefffff)

#define MTL_RQDCM1R_RXQ6DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ6DADMACH_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ6DADMACH_Mask)<<20));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ6DADMACH_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 20) & MTL_RQDCM1R_RXQ6DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ26DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM1R_RXQ26DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM1R_RXQ26DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MTL_RQDCM1R_RXQ26DMA_Wr_Mask (ULONG)(0xfff8ffff)

#define MTL_RQDCM1R_RXQ26DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ26DMA_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ26DMA_Mask)<<16));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ26DMA_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 16) & MTL_RQDCM1R_RXQ26DMA_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ5DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM1R_RXQ5DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM1R_RXQ5DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MTL_RQDCM1R_RXQ5DADMACH_Wr_Mask (ULONG)(0xffffefff)

#define MTL_RQDCM1R_RXQ5DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ5DADMACH_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ5DADMACH_Mask)<<12));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ5DADMACH_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 12) & MTL_RQDCM1R_RXQ5DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ25DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM1R_RXQ25DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM1R_RXQ25DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (8)))*/

#define MTL_RQDCM1R_RXQ25DMA_Wr_Mask (ULONG)(0xfffff8ff)

#define MTL_RQDCM1R_RXQ25DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ25DMA_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ25DMA_Mask)<<8));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ25DMA_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 8) & MTL_RQDCM1R_RXQ25DMA_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ4DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM1R_RXQ4DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM1R_RXQ4DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MTL_RQDCM1R_RXQ4DADMACH_Wr_Mask (ULONG)(0xffffffef)

#define MTL_RQDCM1R_RXQ4DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ4DADMACH_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ4DADMACH_Mask)<<4));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ4DADMACH_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 4) & MTL_RQDCM1R_RXQ4DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM1R_RXQ42DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM1R_RXQ42DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM1R_RXQ42DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_RQDCM1R_RXQ42DMA_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_RQDCM1R_RXQ42DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM1R_RgRd(v);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM1R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM1R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM1R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM1R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM1R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM1R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM1R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM1R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM1R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM1R_RXQ42DMA_Wr_Mask) | ((data & MTL_RQDCM1R_RXQ42DMA_Mask)<<0));\
		MTL_RQDCM1R_RgWr(v);\
} while(0)

#define MTL_RQDCM1R_RXQ42DMA_UdfRd(data) do {\
		MTL_RQDCM1R_RgRd(data);\
		data = ((data >> 0) & MTL_RQDCM1R_RXQ42DMA_Mask);\
} while(0)


#define MTL_RQDCM0R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc30))

#define MTL_RQDCM0R_RgWr(data) do {\
		iowrite32(data, (void *)MTL_RQDCM0R_RgOffAddr);\
} while(0)

#define MTL_RQDCM0R_RgRd(data) do {\
		(data) = ioread32((void *)MTL_RQDCM0R_RgOffAddr);\
} while(0)

/*#define  MTL_RQDCM0R_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM0R_Mask_29 (ULONG)(0x7)

/*#define MTL_RQDCM0R_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define  MTL_RQDCM0R_Mask_27 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM0R_Mask_27 (ULONG)(0x1)

/*#define MTL_RQDCM0R_RES_Wr_Mask_27 (ULONG)(~((~(~0<<(1)))<<(27)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_27 (ULONG)(0xf7ffffff)

/*#define  MTL_RQDCM0R_Mask_21 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM0R_Mask_21 (ULONG)(0x7)

/*#define MTL_RQDCM0R_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(3)))<<(21)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_21 (ULONG)(0xff1fffff)

/*#define  MTL_RQDCM0R_Mask_19 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM0R_Mask_19 (ULONG)(0x1)

/*#define MTL_RQDCM0R_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(1)))<<(19)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_19 (ULONG)(0xfff7ffff)

/*#define  MTL_RQDCM0R_Mask_13 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM0R_Mask_13 (ULONG)(0x7)

/*#define MTL_RQDCM0R_RES_Wr_Mask_13 (ULONG)(~((~(~0<<(3)))<<(13)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_13 (ULONG)(0xffff1fff)

/*#define  MTL_RQDCM0R_Mask_11 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM0R_Mask_11 (ULONG)(0x1)

/*#define MTL_RQDCM0R_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(1)))<<(11)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_11 (ULONG)(0xfffff7ff)

/*#define  MTL_RQDCM0R_Mask_5 (ULONG)(~(~0<<(3)))*/

#define  MTL_RQDCM0R_Mask_5 (ULONG)(0x7)

/*#define MTL_RQDCM0R_RES_Wr_Mask_5 (ULONG)(~((~(~0<<(3)))<<(5)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_5 (ULONG)(0xffffff1f)

/*#define  MTL_RQDCM0R_Mask_3 (ULONG)(~(~0<<(1)))*/

#define  MTL_RQDCM0R_Mask_3 (ULONG)(0x1)

/*#define MTL_RQDCM0R_RES_Wr_Mask_3 (ULONG)(~((~(~0<<(1)))<<(3)))*/

#define MTL_RQDCM0R_RES_Wr_Mask_3 (ULONG)(0xfffffff7)

/*#define MTL_RQDCM0R_RXQ3DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM0R_RXQ3DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM0R_RXQ3DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (28)))*/

#define MTL_RQDCM0R_RXQ3DADMACH_Wr_Mask (ULONG)(0xefffffff)

#define MTL_RQDCM0R_RXQ3DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ3DADMACH_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ3DADMACH_Mask)<<28));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ3DADMACH_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 28) & MTL_RQDCM0R_RXQ3DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ32DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM0R_RXQ32DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM0R_RXQ32DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (24)))*/

#define MTL_RQDCM0R_RXQ32DMA_Wr_Mask (ULONG)(0xf8ffffff)

#define MTL_RQDCM0R_RXQ32DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ32DMA_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ32DMA_Mask)<<24));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ32DMA_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 24) & MTL_RQDCM0R_RXQ32DMA_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ2DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM0R_RXQ2DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM0R_RXQ2DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MTL_RQDCM0R_RXQ2DADMACH_Wr_Mask (ULONG)(0xffefffff)

#define MTL_RQDCM0R_RXQ2DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ2DADMACH_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ2DADMACH_Mask)<<20));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ2DADMACH_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 20) & MTL_RQDCM0R_RXQ2DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ22DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM0R_RXQ22DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM0R_RXQ22DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MTL_RQDCM0R_RXQ22DMA_Wr_Mask (ULONG)(0xfff8ffff)

#define MTL_RQDCM0R_RXQ22DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ22DMA_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ22DMA_Mask)<<16));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ22DMA_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 16) & MTL_RQDCM0R_RXQ22DMA_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ1DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM0R_RXQ1DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM0R_RXQ1DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MTL_RQDCM0R_RXQ1DADMACH_Wr_Mask (ULONG)(0xffffefff)

#define MTL_RQDCM0R_RXQ1DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ1DADMACH_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ1DADMACH_Mask)<<12));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ1DADMACH_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 12) & MTL_RQDCM0R_RXQ1DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ12DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM0R_RXQ12DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM0R_RXQ12DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (8)))*/

#define MTL_RQDCM0R_RXQ12DMA_Wr_Mask (ULONG)(0xfffff8ff)

#define MTL_RQDCM0R_RXQ12DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ12DMA_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ12DMA_Mask)<<8));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ12DMA_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 8) & MTL_RQDCM0R_RXQ12DMA_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ0DADMACH_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_RQDCM0R_RXQ0DADMACH_Mask (ULONG)(0x1)

/*#define MTL_RQDCM0R_RXQ0DADMACH_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MTL_RQDCM0R_RXQ0DADMACH_Wr_Mask (ULONG)(0xffffffef)

#define MTL_RQDCM0R_RXQ0DADMACH_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ0DADMACH_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ0DADMACH_Mask)<<4));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ0DADMACH_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 4) & MTL_RQDCM0R_RXQ0DADMACH_Mask);\
} while(0)

/*#define MTL_RQDCM0R_RXQ02DMA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_RQDCM0R_RXQ02DMA_Mask (ULONG)(0x7)

/*#define MTL_RQDCM0R_RXQ02DMA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_RQDCM0R_RXQ02DMA_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_RQDCM0R_RXQ02DMA_UdfWr(data) do {\
		ULONG v;\
		MTL_RQDCM0R_RgRd(v);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_29))|((( 0) & (MTL_RQDCM0R_Mask_29))<<29);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_27))|((( 0) & (MTL_RQDCM0R_Mask_27))<<27);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_21))|((( 0) & (MTL_RQDCM0R_Mask_21))<<21);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_19))|((( 0) & (MTL_RQDCM0R_Mask_19))<<19);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_13))|((( 0) & (MTL_RQDCM0R_Mask_13))<<13);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_11))|((( 0) & (MTL_RQDCM0R_Mask_11))<<11);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_5))|((( 0) & (MTL_RQDCM0R_Mask_5))<<5);\
		v = (v & (MTL_RQDCM0R_RES_Wr_Mask_3))|((( 0) & (MTL_RQDCM0R_Mask_3))<<3);\
		v = ((v & MTL_RQDCM0R_RXQ02DMA_Wr_Mask) | ((data & MTL_RQDCM0R_RXQ02DMA_Mask)<<0));\
		MTL_RQDCM0R_RgWr(v);\
} while(0)

#define MTL_RQDCM0R_RXQ02DMA_UdfRd(data) do {\
		MTL_RQDCM0R_RgRd(data);\
		data = ((data >> 0) & MTL_RQDCM0R_RXQ02DMA_Mask);\
} while(0)


#define MTL_FDDR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc10))

#define MTL_FDDR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_FDDR_RgOffAddr);\
} while(0)

#define MTL_FDDR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_FDDR_RgOffAddr);\
} while(0)

#define MTL_FDDR_FDBGDATA_UdfWr(data) do {\
		MTL_FDDR_RgWr(data);\
} while(0)

#define MTL_FDDR_FDBGDATA_UdfRd(data) do {\
		MTL_FDDR_RgRd(data);\
} while(0)


#define MTL_FDACS_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc08))

#define MTL_FDACS_RgWr(data) do {\
		iowrite32(data, (void *)MTL_FDACS_RgOffAddr);\
} while(0)

#define MTL_FDACS_RgRd(data) do {\
		(data) = ioread32((void *)MTL_FDACS_RgOffAddr);\
} while(0)

/*#define  MTL_FDACS_Mask_16 (ULONG)(~(~0<<(16)))*/

#define  MTL_FDACS_Mask_16 (ULONG)(0xffff)

/*#define MTL_FDACS_RES_Wr_Mask_16 (ULONG)(~((~(~0<<(16)))<<(16)))*/

#define MTL_FDACS_RES_Wr_Mask_16 (ULONG)(0xffff)

/*#define  MTL_FDACS_Mask_7 (ULONG)(~(~0<<(1)))*/

#define  MTL_FDACS_Mask_7 (ULONG)(0x1)

/*#define MTL_FDACS_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(1)))<<(7)))*/

#define MTL_FDACS_RES_Wr_Mask_7 (ULONG)(0xffffff7f)

/*#define  MTL_FDACS_Mask_4 (ULONG)(~(~0<<(1)))*/

#define  MTL_FDACS_Mask_4 (ULONG)(0x1)

/*#define MTL_FDACS_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(1)))<<(4)))*/

#define MTL_FDACS_RES_Wr_Mask_4 (ULONG)(0xffffffef)

/*#define MTL_FDACS_STSE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_STSE_Mask (ULONG)(0x1)

/*#define MTL_FDACS_STSE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MTL_FDACS_STSE_Wr_Mask (ULONG)(0xffff7fff)

#define MTL_FDACS_STSE_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_STSE_Wr_Mask) | ((data & MTL_FDACS_STSE_Mask)<<15));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_STSE_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 15) & MTL_FDACS_STSE_Mask);\
} while(0)

/*#define MTL_FDACS_PKTE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_PKTE_Mask (ULONG)(0x1)

/*#define MTL_FDACS_PKTE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MTL_FDACS_PKTE_Wr_Mask (ULONG)(0xffffbfff)

#define MTL_FDACS_PKTE_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_PKTE_Wr_Mask) | ((data & MTL_FDACS_PKTE_Mask)<<14));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_PKTE_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 14) & MTL_FDACS_PKTE_Mask);\
} while(0)

/*#define MTL_FDACS_FIFOSEL_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_FDACS_FIFOSEL_Mask (ULONG)(0x3)

/*#define MTL_FDACS_FIFOSEL_Wr_Mask (ULONG)(~((~(~0 << (2))) << (12)))*/

#define MTL_FDACS_FIFOSEL_Wr_Mask (ULONG)(0xffffcfff)

#define MTL_FDACS_FIFOSEL_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_FIFOSEL_Wr_Mask) | ((data & MTL_FDACS_FIFOSEL_Mask)<<12));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_FIFOSEL_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 12) & MTL_FDACS_FIFOSEL_Mask);\
} while(0)

/*#define MTL_FDACS_FIFOWREN_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_FIFOWREN_Mask (ULONG)(0x1)

/*#define MTL_FDACS_FIFOWREN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_FDACS_FIFOWREN_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_FDACS_FIFOWREN_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_FIFOWREN_Wr_Mask) | ((data & MTL_FDACS_FIFOWREN_Mask)<<11));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_FIFOWREN_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 11) & MTL_FDACS_FIFOWREN_Mask);\
} while(0)

/*#define MTL_FDACS_FIFORDEN_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_FIFORDEN_Mask (ULONG)(0x1)

/*#define MTL_FDACS_FIFORDEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MTL_FDACS_FIFORDEN_Wr_Mask (ULONG)(0xfffffbff)

#define MTL_FDACS_FIFORDEN_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_FIFORDEN_Wr_Mask) | ((data & MTL_FDACS_FIFORDEN_Mask)<<10));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_FIFORDEN_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 10) & MTL_FDACS_FIFORDEN_Mask);\
} while(0)

/*#define MTL_FDACS_RSTSEL_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_RSTSEL_Mask (ULONG)(0x1)

/*#define MTL_FDACS_RSTSEL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MTL_FDACS_RSTSEL_Wr_Mask (ULONG)(0xfffffdff)

#define MTL_FDACS_RSTSEL_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_RSTSEL_Wr_Mask) | ((data & MTL_FDACS_RSTSEL_Mask)<<9));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_RSTSEL_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 9) & MTL_FDACS_RSTSEL_Mask);\
} while(0)

/*#define MTL_FDACS_RSTALL_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_RSTALL_Mask (ULONG)(0x1)

/*#define MTL_FDACS_RSTALL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MTL_FDACS_RSTALL_Wr_Mask (ULONG)(0xfffffeff)

#define MTL_FDACS_RSTALL_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_RSTALL_Wr_Mask) | ((data & MTL_FDACS_RSTALL_Mask)<<8));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_RSTALL_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 8) & MTL_FDACS_RSTALL_Mask);\
} while(0)

/*#define MTL_FDACS_PKTSTATE_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_FDACS_PKTSTATE_Mask (ULONG)(0x3)

/*#define MTL_FDACS_PKTSTATE_Wr_Mask (ULONG)(~((~(~0 << (2))) << (5)))*/

#define MTL_FDACS_PKTSTATE_Wr_Mask (ULONG)(0xffffff9f)

#define MTL_FDACS_PKTSTATE_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_PKTSTATE_Wr_Mask) | ((data & MTL_FDACS_PKTSTATE_Mask)<<5));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_PKTSTATE_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 5) & MTL_FDACS_PKTSTATE_Mask);\
} while(0)

/*#define MTL_FDACS_BYTEEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_FDACS_BYTEEN_Mask (ULONG)(0x3)

/*#define MTL_FDACS_BYTEEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_FDACS_BYTEEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_FDACS_BYTEEN_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_BYTEEN_Wr_Mask) | ((data & MTL_FDACS_BYTEEN_Mask)<<2));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_BYTEEN_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 2) & MTL_FDACS_BYTEEN_Mask);\
} while(0)

/*#define MTL_FDACS_DEGMOD_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_DEGMOD_Mask (ULONG)(0x1)

/*#define MTL_FDACS_DEGMOD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_FDACS_DEGMOD_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_FDACS_DEGMOD_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_DEGMOD_Wr_Mask) | ((data & MTL_FDACS_DEGMOD_Mask)<<1));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_DEGMOD_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 1) & MTL_FDACS_DEGMOD_Mask);\
} while(0)

/*#define MTL_FDACS_FDBGEN_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_FDACS_FDBGEN_Mask (ULONG)(0x1)

/*#define MTL_FDACS_FDBGEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_FDACS_FDBGEN_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_FDACS_FDBGEN_UdfWr(data) do {\
		ULONG v;\
		MTL_FDACS_RgRd(v);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_16))|((( 0) & (MTL_FDACS_Mask_16))<<16);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_7))|((( 0) & (MTL_FDACS_Mask_7))<<7);\
		v = (v & (MTL_FDACS_RES_Wr_Mask_4))|((( 0) & (MTL_FDACS_Mask_4))<<4);\
		v = ((v & MTL_FDACS_FDBGEN_Wr_Mask) | ((data & MTL_FDACS_FDBGEN_Mask)<<0));\
		MTL_FDACS_RgWr(v);\
} while(0)

#define MTL_FDACS_FDBGEN_UdfRd(data) do {\
		MTL_FDACS_RgRd(data);\
		data = ((data >> 0) & MTL_FDACS_FDBGEN_Mask);\
} while(0)


#define MTL_OMR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc00))

#define MTL_OMR_RgWr(data) do {\
		iowrite32(data, (void *)MTL_OMR_RgOffAddr);\
} while(0)

#define MTL_OMR_RgRd(data) do {\
		(data) = ioread32((void *)MTL_OMR_RgOffAddr);\
} while(0)

/*#define  MTL_OMR_Mask_7 (ULONG)(~(~0<<(25)))*/

#define  MTL_OMR_Mask_7 (ULONG)(0x1ffffff)

/*#define MTL_OMR_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(25)))<<(7)))*/

#define MTL_OMR_RES_Wr_Mask_7 (ULONG)(0x7f)

/*#define  MTL_OMR_Mask_0 (ULONG)(~(~0<<(1)))*/

#define  MTL_OMR_Mask_0 (ULONG)(0x1)

/*#define MTL_OMR_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(1)))<<(0)))*/

#define MTL_OMR_RES_Wr_Mask_0 (ULONG)(0xfffffffe)

/*#define MTL_OMR_SCHALG_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_OMR_SCHALG_Mask (ULONG)(0x3)

/*#define MTL_OMR_SCHALG_Wr_Mask (ULONG)(~((~(~0 << (2))) << (5)))*/

#define MTL_OMR_SCHALG_Wr_Mask (ULONG)(0xffffff9f)

#define MTL_OMR_SCHALG_UdfWr(data) do {\
		ULONG v;\
		MTL_OMR_RgRd(v);\
		v = (v & (MTL_OMR_RES_Wr_Mask_7))|((( 0) & (MTL_OMR_Mask_7))<<7);\
		v = (v & (MTL_OMR_RES_Wr_Mask_0))|((( 0) & (MTL_OMR_Mask_0))<<0);\
		v = ((v & MTL_OMR_SCHALG_Wr_Mask) | ((data & MTL_OMR_SCHALG_Mask)<<5));\
		MTL_OMR_RgWr(v);\
} while(0)

#define MTL_OMR_SCHALG_UdfRd(data) do {\
		MTL_OMR_RgRd(data);\
		data = ((data >> 5) & MTL_OMR_SCHALG_Mask);\
} while(0)

/*#define MTL_OMR_RAA_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_OMR_RAA_Mask (ULONG)(0x7)

/*#define MTL_OMR_RAA_Wr_Mask (ULONG)(~((~(~0 << (3))) << (2)))*/

#define MTL_OMR_RAA_Wr_Mask (ULONG)(0xffffffe3)

#define MTL_OMR_RAA_UdfWr(data) do {\
		ULONG v;\
		MTL_OMR_RgRd(v);\
		v = (v & (MTL_OMR_RES_Wr_Mask_7))|((( 0) & (MTL_OMR_Mask_7))<<7);\
		v = (v & (MTL_OMR_RES_Wr_Mask_0))|((( 0) & (MTL_OMR_Mask_0))<<0);\
		v = ((v & MTL_OMR_RAA_Wr_Mask) | ((data & MTL_OMR_RAA_Mask)<<2));\
		MTL_OMR_RgWr(v);\
} while(0)

#define MTL_OMR_RAA_UdfRd(data) do {\
		MTL_OMR_RgRd(data);\
		data = ((data >> 2) & MTL_OMR_RAA_Mask);\
} while(0)

/*#define MTL_OMR_DTXSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_OMR_DTXSTS_Mask (ULONG)(0x1)

/*#define MTL_OMR_DTXSTS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_OMR_DTXSTS_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_OMR_DTXSTS_UdfWr(data) do {\
		ULONG v;\
		MTL_OMR_RgRd(v);\
		v = (v & (MTL_OMR_RES_Wr_Mask_7))|((( 0) & (MTL_OMR_Mask_7))<<7);\
		v = (v & (MTL_OMR_RES_Wr_Mask_0))|((( 0) & (MTL_OMR_Mask_0))<<0);\
		v = ((v & MTL_OMR_DTXSTS_Wr_Mask) | ((data & MTL_OMR_DTXSTS_Mask)<<1));\
		MTL_OMR_RgWr(v);\
} while(0)

#define MTL_OMR_DTXSTS_UdfRd(data) do {\
		MTL_OMR_RgRd(data);\
		data = ((data >> 1) & MTL_OMR_DTXSTS_Mask);\
} while(0)


#define MAC_RQC3R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xac))

#define MAC_RQC3R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_RQC3R_RgOffAddr);\
} while(0)

#define MAC_RQC3R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RQC3R_RgOffAddr);\
} while(0)

/*#define MAC_RQC3R_PSRQ7_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC3R_PSRQ7_Mask (ULONG)(0xff)

/*#define MAC_RQC3R_PSRQ7_Wr_Mask (ULONG)(~((~(~0 << (8))) << (24)))*/

#define MAC_RQC3R_PSRQ7_Wr_Mask (ULONG)(0xffffff)

#define MAC_RQC3R_PSRQ7_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC3R_RgRd(v);\
		v = ((v & MAC_RQC3R_PSRQ7_Wr_Mask) | ((data & MAC_RQC3R_PSRQ7_Mask)<<24));\
		MAC_RQC3R_RgWr(v);\
} while(0)

#define MAC_RQC3R_PSRQ7_UdfRd(data) do {\
		MAC_RQC3R_RgRd(data);\
		data = ((data >> 24) & MAC_RQC3R_PSRQ7_Mask);\
} while(0)

/*#define MAC_RQC3R_PSRQ6_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC3R_PSRQ6_Mask (ULONG)(0xff)

/*#define MAC_RQC3R_PSRQ6_Wr_Mask (ULONG)(~((~(~0 << (8))) << (16)))*/

#define MAC_RQC3R_PSRQ6_Wr_Mask (ULONG)(0xff00ffff)

#define MAC_RQC3R_PSRQ6_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC3R_RgRd(v);\
		v = ((v & MAC_RQC3R_PSRQ6_Wr_Mask) | ((data & MAC_RQC3R_PSRQ6_Mask)<<16));\
		MAC_RQC3R_RgWr(v);\
} while(0)

#define MAC_RQC3R_PSRQ6_UdfRd(data) do {\
		MAC_RQC3R_RgRd(data);\
		data = ((data >> 16) & MAC_RQC3R_PSRQ6_Mask);\
} while(0)

/*#define MAC_RQC3R_PSRQ5_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC3R_PSRQ5_Mask (ULONG)(0xff)

/*#define MAC_RQC3R_PSRQ5_Wr_Mask (ULONG)(~((~(~0 << (8))) << (8)))*/

#define MAC_RQC3R_PSRQ5_Wr_Mask (ULONG)(0xffff00ff)

#define MAC_RQC3R_PSRQ5_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC3R_RgRd(v);\
		v = ((v & MAC_RQC3R_PSRQ5_Wr_Mask) | ((data & MAC_RQC3R_PSRQ5_Mask)<<8));\
		MAC_RQC3R_RgWr(v);\
} while(0)

#define MAC_RQC3R_PSRQ5_UdfRd(data) do {\
		MAC_RQC3R_RgRd(data);\
		data = ((data >> 8) & MAC_RQC3R_PSRQ5_Mask);\
} while(0)

/*#define MAC_RQC3R_PSRQ4_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC3R_PSRQ4_Mask (ULONG)(0xff)

/*#define MAC_RQC3R_PSRQ4_Wr_Mask (ULONG)(~((~(~0 << (8))) << (0)))*/

#define MAC_RQC3R_PSRQ4_Wr_Mask (ULONG)(0xffffff00)

#define MAC_RQC3R_PSRQ4_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC3R_RgRd(v);\
		v = ((v & MAC_RQC3R_PSRQ4_Wr_Mask) | ((data & MAC_RQC3R_PSRQ4_Mask)<<0));\
		MAC_RQC3R_RgWr(v);\
} while(0)

#define MAC_RQC3R_PSRQ4_UdfRd(data) do {\
		MAC_RQC3R_RgRd(data);\
		data = ((data >> 0) & MAC_RQC3R_PSRQ4_Mask);\
} while(0)


#define MAC_RQC2R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa8))

#define MAC_RQC2R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_RQC2R_RgOffAddr);\
} while(0)

#define MAC_RQC2R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RQC2R_RgOffAddr);\
} while(0)

/*#define MAC_RQC2R_PSRQ3_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC2R_PSRQ3_Mask (ULONG)(0xff)

/*#define MAC_RQC2R_PSRQ3_Wr_Mask (ULONG)(~((~(~0 << (8))) << (24)))*/

#define MAC_RQC2R_PSRQ3_Wr_Mask (ULONG)(0xffffff)

#define MAC_RQC2R_PSRQ3_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC2R_RgRd(v);\
		v = ((v & MAC_RQC2R_PSRQ3_Wr_Mask) | ((data & MAC_RQC2R_PSRQ3_Mask)<<24));\
		MAC_RQC2R_RgWr(v);\
} while(0)

#define MAC_RQC2R_PSRQ3_UdfRd(data) do {\
		MAC_RQC2R_RgRd(data);\
		data = ((data >> 24) & MAC_RQC2R_PSRQ3_Mask);\
} while(0)

/*#define MAC_RQC2R_PSRQ2_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC2R_PSRQ2_Mask (ULONG)(0xff)

/*#define MAC_RQC2R_PSRQ2_Wr_Mask (ULONG)(~((~(~0 << (8))) << (16)))*/

#define MAC_RQC2R_PSRQ2_Wr_Mask (ULONG)(0xff00ffff)

#define MAC_RQC2R_PSRQ2_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC2R_RgRd(v);\
		v = ((v & MAC_RQC2R_PSRQ2_Wr_Mask) | ((data & MAC_RQC2R_PSRQ2_Mask)<<16));\
		MAC_RQC2R_RgWr(v);\
} while(0)

#define MAC_RQC2R_PSRQ2_UdfRd(data) do {\
		MAC_RQC2R_RgRd(data);\
		data = ((data >> 16) & MAC_RQC2R_PSRQ2_Mask);\
} while(0)

/*#define MAC_RQC2R_PSRQ1_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC2R_PSRQ1_Mask (ULONG)(0xff)

/*#define MAC_RQC2R_PSRQ1_Wr_Mask (ULONG)(~((~(~0 << (8))) << (8)))*/

#define MAC_RQC2R_PSRQ1_Wr_Mask (ULONG)(0xffff00ff)

#define MAC_RQC2R_PSRQ1_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC2R_RgRd(v);\
		v = ((v & MAC_RQC2R_PSRQ1_Wr_Mask) | ((data & MAC_RQC2R_PSRQ1_Mask)<<8));\
		MAC_RQC2R_RgWr(v);\
} while(0)

#define MAC_RQC2R_PSRQ1_UdfRd(data) do {\
		MAC_RQC2R_RgRd(data);\
		data = ((data >> 8) & MAC_RQC2R_PSRQ1_Mask);\
} while(0)

/*#define MAC_RQC2R_PSRQ0_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_RQC2R_PSRQ0_Mask (ULONG)(0xff)

/*#define MAC_RQC2R_PSRQ0_Wr_Mask (ULONG)(~((~(~0 << (8))) << (0)))*/

#define MAC_RQC2R_PSRQ0_Wr_Mask (ULONG)(0xffffff00)

#define MAC_RQC2R_PSRQ0_UdfWr(data) do{\
		ULONG v;\
		MAC_RQC2R_RgRd(v);\
		v = ((v & MAC_RQC2R_PSRQ0_Wr_Mask) | ((data & MAC_RQC2R_PSRQ0_Mask)<<0));\
		MAC_RQC2R_RgWr(v);\
} while(0)

#define MAC_RQC2R_PSRQ0_UdfRd(data) do {\
		MAC_RQC2R_RgRd(data);\
		data = ((data >> 0) & MAC_RQC2R_PSRQ0_Mask);\
} while(0)


#define MAC_RQC1R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa4))

#define MAC_RQC1R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_RQC1R_RgOffAddr);\
} while(0)

#define MAC_RQC1R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RQC1R_RgOffAddr);\
} while(0)

/*#define  MAC_RQC1R_Mask_11 (ULONG)(~(~0<<(21)))*/

#define  MAC_RQC1R_Mask_11 (ULONG)(0x1fffff)

/*#define MAC_RQC1R_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(21)))<<(11)))*/

#define MAC_RQC1R_RES_Wr_Mask_11 (ULONG)(0x7ff)

/*#define  MAC_RQC1R_Mask_7 (ULONG)(~(~0<<(1)))*/

#define  MAC_RQC1R_Mask_7 (ULONG)(0x1)

/*#define MAC_RQC1R_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(1)))<<(7)))*/

#define MAC_RQC1R_RES_Wr_Mask_7 (ULONG)(0xffffff7f)

/*#define  MAC_RQC1R_Mask_3 (ULONG)(~(~0<<(1)))*/

#define  MAC_RQC1R_Mask_3 (ULONG)(0x1)

/*#define MAC_RQC1R_RES_Wr_Mask_3 (ULONG)(~((~(~0<<(1)))<<(3)))*/

#define MAC_RQC1R_RES_Wr_Mask_3 (ULONG)(0xfffffff7)

/*#define MAC_RQC1R_DCBCPQ_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_RQC1R_DCBCPQ_Mask (ULONG)(0x7)

/*#define MAC_RQC1R_DCBCPQ_Wr_Mask (ULONG)(~((~(~0 << (3))) << (8)))*/

#define MAC_RQC1R_DCBCPQ_Wr_Mask (ULONG)(0xfffff8ff)

#define MAC_RQC1R_DCBCPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC1R_RgRd(v);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_11))|((( 0) & (MAC_RQC1R_Mask_11))<<11);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_7))|((( 0) & (MAC_RQC1R_Mask_7))<<7);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_3))|((( 0) & (MAC_RQC1R_Mask_3))<<3);\
		v = ((v & MAC_RQC1R_DCBCPQ_Wr_Mask) | ((data & MAC_RQC1R_DCBCPQ_Mask)<<8));\
		MAC_RQC1R_RgWr(v);\
} while(0)

#define MAC_RQC1R_DCBCPQ_UdfRd(data) do {\
		MAC_RQC1R_RgRd(data);\
		data = ((data >> 8) & MAC_RQC1R_DCBCPQ_Mask);\
} while(0)

/*#define MAC_RQC1R_AVPTPQ_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_RQC1R_AVPTPQ_Mask (ULONG)(0x7)

/*#define MAC_RQC1R_AVPTPQ_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_RQC1R_AVPTPQ_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_RQC1R_AVPTPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC1R_RgRd(v);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_11))|((( 0) & (MAC_RQC1R_Mask_11))<<11);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_7))|((( 0) & (MAC_RQC1R_Mask_7))<<7);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_3))|((( 0) & (MAC_RQC1R_Mask_3))<<3);\
		v = ((v & MAC_RQC1R_AVPTPQ_Wr_Mask) | ((data & MAC_RQC1R_AVPTPQ_Mask)<<4));\
		MAC_RQC1R_RgWr(v);\
} while(0)

#define MAC_RQC1R_AVPTPQ_UdfRd(data) do {\
		MAC_RQC1R_RgRd(data);\
		data = ((data >> 4) & MAC_RQC1R_AVPTPQ_Mask);\
} while(0)

/*#define MAC_RQC1R_AVUCPQ_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_RQC1R_AVUCPQ_Mask (ULONG)(0x7)

/*#define MAC_RQC1R_AVUCPQ_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MAC_RQC1R_AVUCPQ_Wr_Mask (ULONG)(0xfffffff8)

#define MAC_RQC1R_AVUCPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC1R_RgRd(v);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_11))|((( 0) & (MAC_RQC1R_Mask_11))<<11);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_7))|((( 0) & (MAC_RQC1R_Mask_7))<<7);\
		v = (v & (MAC_RQC1R_RES_Wr_Mask_3))|((( 0) & (MAC_RQC1R_Mask_3))<<3);\
		v = ((v & MAC_RQC1R_AVUCPQ_Wr_Mask) | ((data & MAC_RQC1R_AVUCPQ_Mask)<<0));\
		MAC_RQC1R_RgWr(v);\
} while(0)

#define MAC_RQC1R_AVUCPQ_UdfRd(data) do {\
		MAC_RQC1R_RgRd(data);\
		data = ((data >> 0) & MAC_RQC1R_AVUCPQ_Mask);\
} while(0)


#define MAC_RQC0R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xa0))

#define MAC_RQC0R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_RQC0R_RgOffAddr);\
} while(0)

#define MAC_RQC0R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RQC0R_RgOffAddr);\
} while(0)

/*#define  MAC_RQC0R_Mask_16 (ULONG)(~(~0<<(16)))*/

#define  MAC_RQC0R_Mask_16 (ULONG)(0xffff)

/*#define MAC_RQC0R_RES_Wr_Mask_16 (ULONG)(~((~(~0<<(16)))<<(16)))*/

#define MAC_RQC0R_RES_Wr_Mask_16 (ULONG)(0xffff)

/*#define MAC_RQC0R_RXQEN7_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN7_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN7_Wr_Mask (ULONG)(~((~(~0 << (2))) << (14)))*/

#define MAC_RQC0R_RXQEN7_Wr_Mask (ULONG)(0xffff3fff)

#define MAC_RQC0R_RXQEN7_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN7_Wr_Mask) | ((data & MAC_RQC0R_RXQEN7_Mask)<<14));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN7_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 14) & MAC_RQC0R_RXQEN7_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN6_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN6_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN6_Wr_Mask (ULONG)(~((~(~0 << (2))) << (12)))*/

#define MAC_RQC0R_RXQEN6_Wr_Mask (ULONG)(0xffffcfff)

#define MAC_RQC0R_RXQEN6_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN6_Wr_Mask) | ((data & MAC_RQC0R_RXQEN6_Mask)<<12));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN6_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 12) & MAC_RQC0R_RXQEN6_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN5_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN5_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN5_Wr_Mask (ULONG)(~((~(~0 << (2))) << (10)))*/

#define MAC_RQC0R_RXQEN5_Wr_Mask (ULONG)(0xfffff3ff)

#define MAC_RQC0R_RXQEN5_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN5_Wr_Mask) | ((data & MAC_RQC0R_RXQEN5_Mask)<<10));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN5_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 10) & MAC_RQC0R_RXQEN5_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN4_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN4_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN4_Wr_Mask (ULONG)(~((~(~0 << (2))) << (8)))*/

#define MAC_RQC0R_RXQEN4_Wr_Mask (ULONG)(0xfffffcff)

#define MAC_RQC0R_RXQEN4_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN4_Wr_Mask) | ((data & MAC_RQC0R_RXQEN4_Mask)<<8));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN4_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 8) & MAC_RQC0R_RXQEN4_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN3_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN3_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN3_Wr_Mask (ULONG)(~((~(~0 << (2))) << (6)))*/

#define MAC_RQC0R_RXQEN3_Wr_Mask (ULONG)(0xffffff3f)

#define MAC_RQC0R_RXQEN3_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN3_Wr_Mask) | ((data & MAC_RQC0R_RXQEN3_Mask)<<6));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN3_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 6) & MAC_RQC0R_RXQEN3_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN2_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN2_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN2_Wr_Mask (ULONG)(~((~(~0 << (2))) << (4)))*/

#define MAC_RQC0R_RXQEN2_Wr_Mask (ULONG)(0xffffffcf)

#define MAC_RQC0R_RXQEN2_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN2_Wr_Mask) | ((data & MAC_RQC0R_RXQEN2_Mask)<<4));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN2_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 4) & MAC_RQC0R_RXQEN2_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN1_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN1_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN1_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MAC_RQC0R_RXQEN1_Wr_Mask (ULONG)(0xfffffff3)

#define MAC_RQC0R_RXQEN1_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN1_Wr_Mask) | ((data & MAC_RQC0R_RXQEN1_Mask)<<2));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN1_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 2) & MAC_RQC0R_RXQEN1_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN0_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN0_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN0_Wr_Mask (ULONG)(~((~(~0 << (2))) << (0)))*/

#define MAC_RQC0R_RXQEN0_Wr_Mask (ULONG)(0xfffffffc)

#define MAC_RQC0R_RXQEN0_UdfWr(data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN0_Wr_Mask) | ((data & MAC_RQC0R_RXQEN0_Mask)<<0));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN0_UdfRd(data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> 0) & MAC_RQC0R_RXQEN0_Mask);\
} while(0)

/*#define MAC_RQC0R_RXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_RQC0R_RXQEN_Mask (ULONG)(0x3)

/*#define MAC_RQC0R_RXQEN_Wr_Mask(i) (ULONG)(~((~(~0 << (2))) << (0 + (i * 2))))*/

#define MAC_RQC0R_RXQEN_Wr_Mask(i)  (ULONG)(~((~(~0 << (2))) << (0 + (i * 2))))

#define MAC_RQC0R_RXQEN_UdfWr(i, data) do {\
		ULONG v;\
		MAC_RQC0R_RgRd(v);\
		v = (v & (MAC_RQC0R_RES_Wr_Mask_16))|((( 0) & (MAC_RQC0R_Mask_16))<<16);\
		v = ((v & MAC_RQC0R_RXQEN_Wr_Mask(i)) | ((data & MAC_RQC0R_RXQEN_Mask)<<(0 + i*2)));\
		MAC_RQC0R_RgWr(v);\
} while(0)

#define MAC_RQC0R_RXQEN_UdfRd(i, data) do {\
		MAC_RQC0R_RgRd(data);\
		data = ((data >> (0 + (i*2))) & MAC_RQC0R_RXQEN_Mask);\
} while(0)


#define MAC_TQPM1R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x9c))

#define MAC_TQPM1R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_TQPM1R_RgOffAddr);\
} while(0)

#define MAC_TQPM1R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TQPM1R_RgOffAddr);\
} while(0)

/*#define MAC_TQPM1R_PSTQ7_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM1R_PSTQ7_Mask (ULONG)(0xff)

/*#define MAC_TQPM1R_PSTQ7_Wr_Mask (ULONG)(~((~(~0 << (8))) << (24)))*/

#define MAC_TQPM1R_PSTQ7_Wr_Mask (ULONG)(0xffffff)

#define MAC_TQPM1R_PSTQ7_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM1R_RgRd(v);\
		v = ((v & MAC_TQPM1R_PSTQ7_Wr_Mask) | ((data & MAC_TQPM1R_PSTQ7_Mask)<<24));\
		MAC_TQPM1R_RgWr(v);\
} while(0)

#define MAC_TQPM1R_PSTQ7_UdfRd(data) do {\
		MAC_TQPM1R_RgRd(data);\
		data = ((data >> 24) & MAC_TQPM1R_PSTQ7_Mask);\
} while(0)

/*#define MAC_TQPM1R_PSTQ6_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM1R_PSTQ6_Mask (ULONG)(0xff)

/*#define MAC_TQPM1R_PSTQ6_Wr_Mask (ULONG)(~((~(~0 << (8))) << (16)))*/

#define MAC_TQPM1R_PSTQ6_Wr_Mask (ULONG)(0xff00ffff)

#define MAC_TQPM1R_PSTQ6_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM1R_RgRd(v);\
		v = ((v & MAC_TQPM1R_PSTQ6_Wr_Mask) | ((data & MAC_TQPM1R_PSTQ6_Mask)<<16));\
		MAC_TQPM1R_RgWr(v);\
} while(0)

#define MAC_TQPM1R_PSTQ6_UdfRd(data) do {\
		MAC_TQPM1R_RgRd(data);\
		data = ((data >> 16) & MAC_TQPM1R_PSTQ6_Mask);\
} while(0)

/*#define MAC_TQPM1R_PSTQ5_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM1R_PSTQ5_Mask (ULONG)(0xff)

/*#define MAC_TQPM1R_PSTQ5_Wr_Mask (ULONG)(~((~(~0 << (8))) << (8)))*/

#define MAC_TQPM1R_PSTQ5_Wr_Mask (ULONG)(0xffff00ff)

#define MAC_TQPM1R_PSTQ5_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM1R_RgRd(v);\
		v = ((v & MAC_TQPM1R_PSTQ5_Wr_Mask) | ((data & MAC_TQPM1R_PSTQ5_Mask)<<8));\
		MAC_TQPM1R_RgWr(v);\
} while(0)

#define MAC_TQPM1R_PSTQ5_UdfRd(data) do {\
		MAC_TQPM1R_RgRd(data);\
		data = ((data >> 8) & MAC_TQPM1R_PSTQ5_Mask);\
} while(0)

/*#define MAC_TQPM1R_PSTQ4_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM1R_PSTQ4_Mask (ULONG)(0xff)

/*#define MAC_TQPM1R_PSTQ4_Wr_Mask (ULONG)(~((~(~0 << (8))) << (0)))*/

#define MAC_TQPM1R_PSTQ4_Wr_Mask (ULONG)(0xffffff00)

#define MAC_TQPM1R_PSTQ4_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM1R_RgRd(v);\
		v = ((v & MAC_TQPM1R_PSTQ4_Wr_Mask) | ((data & MAC_TQPM1R_PSTQ4_Mask)<<0));\
		MAC_TQPM1R_RgWr(v);\
} while(0)

#define MAC_TQPM1R_PSTQ4_UdfRd(data) do {\
		MAC_TQPM1R_RgRd(data);\
		data = ((data >> 0) & MAC_TQPM1R_PSTQ4_Mask);\
} while(0)


#define MAC_TQPM0R_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x98))

#define MAC_TQPM0R_RgWr(data) do {\
		iowrite32(data, (void *)MAC_TQPM0R_RgOffAddr);\
} while(0)

#define MAC_TQPM0R_RgRd(data) do {\
		(data) = ioread32((void *)MAC_TQPM0R_RgOffAddr);\
} while(0)

/*#define MAC_TQPM0R_PSTQ3_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM0R_PSTQ3_Mask (ULONG)(0xff)

/*#define MAC_TQPM0R_PSTQ3_Wr_Mask (ULONG)(~((~(~0 << (8))) << (24)))*/

#define MAC_TQPM0R_PSTQ3_Wr_Mask (ULONG)(0xffffff)

#define MAC_TQPM0R_PSTQ3_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM0R_RgRd(v);\
		v = ((v & MAC_TQPM0R_PSTQ3_Wr_Mask) | ((data & MAC_TQPM0R_PSTQ3_Mask)<<24));\
		MAC_TQPM0R_RgWr(v);\
} while(0)

#define MAC_TQPM0R_PSTQ3_UdfRd(data) do {\
		MAC_TQPM0R_RgRd(data);\
		data = ((data >> 24) & MAC_TQPM0R_PSTQ3_Mask);\
} while(0)

/*#define MAC_TQPM0R_PSTQ2_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM0R_PSTQ2_Mask (ULONG)(0xff)

/*#define MAC_TQPM0R_PSTQ2_Wr_Mask (ULONG)(~((~(~0 << (8))) << (16)))*/

#define MAC_TQPM0R_PSTQ2_Wr_Mask (ULONG)(0xff00ffff)

#define MAC_TQPM0R_PSTQ2_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM0R_RgRd(v);\
		v = ((v & MAC_TQPM0R_PSTQ2_Wr_Mask) | ((data & MAC_TQPM0R_PSTQ2_Mask)<<16));\
		MAC_TQPM0R_RgWr(v);\
} while(0)

#define MAC_TQPM0R_PSTQ2_UdfRd(data) do {\
		MAC_TQPM0R_RgRd(data);\
		data = ((data >> 16) & MAC_TQPM0R_PSTQ2_Mask);\
} while(0)

/*#define MAC_TQPM0R_PSTQ1_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM0R_PSTQ1_Mask (ULONG)(0xff)

/*#define MAC_TQPM0R_PSTQ1_Wr_Mask (ULONG)(~((~(~0 << (8))) << (8)))*/

#define MAC_TQPM0R_PSTQ1_Wr_Mask (ULONG)(0xffff00ff)

#define MAC_TQPM0R_PSTQ1_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM0R_RgRd(v);\
		v = ((v & MAC_TQPM0R_PSTQ1_Wr_Mask) | ((data & MAC_TQPM0R_PSTQ1_Mask)<<8));\
		MAC_TQPM0R_RgWr(v);\
} while(0)

#define MAC_TQPM0R_PSTQ1_UdfRd(data) do {\
		MAC_TQPM0R_RgRd(data);\
		data = ((data >> 8) & MAC_TQPM0R_PSTQ1_Mask);\
} while(0)

/*#define MAC_TQPM0R_PSTQ0_Mask (ULONG)(~(~0<<(8)))*/

#define MAC_TQPM0R_PSTQ0_Mask (ULONG)(0xff)

/*#define MAC_TQPM0R_PSTQ0_Wr_Mask (ULONG)(~((~(~0 << (8))) << (0)))*/

#define MAC_TQPM0R_PSTQ0_Wr_Mask (ULONG)(0xffffff00)

#define MAC_TQPM0R_PSTQ0_UdfWr(data) do{\
		ULONG v;\
		MAC_TQPM0R_RgRd(v);\
		v = ((v & MAC_TQPM0R_PSTQ0_Wr_Mask) | ((data & MAC_TQPM0R_PSTQ0_Mask)<<0));\
		MAC_TQPM0R_RgWr(v);\
} while(0)

#define MAC_TQPM0R_PSTQ0_UdfRd(data) do {\
		MAC_TQPM0R_RgRd(data);\
		data = ((data >> 0) & MAC_TQPM0R_PSTQ0_Mask);\
} while(0)


#define MAC_RFCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x90))

#define MAC_RFCR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_RFCR_RgOffAddr);\
} while(0)

#define MAC_RFCR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_RFCR_RgOffAddr);\
} while(0)

/*#define  MAC_RFCR_Mask_9 (ULONG)(~(~0<<(23)))*/

#define  MAC_RFCR_Mask_9 (ULONG)(0x7fffff)

/*#define MAC_RFCR_RES_Wr_Mask_9 (ULONG)(~((~(~0<<(23)))<<(9)))*/

#define MAC_RFCR_RES_Wr_Mask_9 (ULONG)(0x1ff)

/*#define  MAC_RFCR_Mask_2 (ULONG)(~(~0<<(6)))*/

#define  MAC_RFCR_Mask_2 (ULONG)(0x3f)

/*#define MAC_RFCR_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(6)))<<(2)))*/

#define MAC_RFCR_RES_Wr_Mask_2 (ULONG)(0xffffff03)

/*#define MAC_RFCR_PFCE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RFCR_PFCE_Mask (ULONG)(0x1)

/*#define MAC_RFCR_PFCE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MAC_RFCR_PFCE_Wr_Mask (ULONG)(0xfffffeff)

#define MAC_RFCR_PFCE_UdfWr(data) do {\
		ULONG v;\
		MAC_RFCR_RgRd(v);\
		v = (v & (MAC_RFCR_RES_Wr_Mask_9))|((( 0) & (MAC_RFCR_Mask_9))<<9);\
		v = (v & (MAC_RFCR_RES_Wr_Mask_2))|((( 0) & (MAC_RFCR_Mask_2))<<2);\
		v = ((v & MAC_RFCR_PFCE_Wr_Mask) | ((data & MAC_RFCR_PFCE_Mask)<<8));\
		MAC_RFCR_RgWr(v);\
} while(0)

#define MAC_RFCR_PFCE_UdfRd(data) do {\
		MAC_RFCR_RgRd(data);\
		data = ((data >> 8) & MAC_RFCR_PFCE_Mask);\
} while(0)

/*#define MAC_RFCR_UP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RFCR_UP_Mask (ULONG)(0x1)

/*#define MAC_RFCR_UP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_RFCR_UP_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_RFCR_UP_UdfWr(data) do {\
		ULONG v;\
		MAC_RFCR_RgRd(v);\
		v = (v & (MAC_RFCR_RES_Wr_Mask_9))|((( 0) & (MAC_RFCR_Mask_9))<<9);\
		v = (v & (MAC_RFCR_RES_Wr_Mask_2))|((( 0) & (MAC_RFCR_Mask_2))<<2);\
		v = ((v & MAC_RFCR_UP_Wr_Mask) | ((data & MAC_RFCR_UP_Mask)<<1));\
		MAC_RFCR_RgWr(v);\
} while(0)

#define MAC_RFCR_UP_UdfRd(data) do {\
		MAC_RFCR_RgRd(data);\
		data = ((data >> 1) & MAC_RFCR_UP_Mask);\
} while(0)

/*#define MAC_RFCR_RFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_RFCR_RFE_Mask (ULONG)(0x1)

/*#define MAC_RFCR_RFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_RFCR_RFE_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_RFCR_RFE_UdfWr(data) do {\
		ULONG v;\
		MAC_RFCR_RgRd(v);\
		v = (v & (MAC_RFCR_RES_Wr_Mask_9))|((( 0) & (MAC_RFCR_Mask_9))<<9);\
		v = (v & (MAC_RFCR_RES_Wr_Mask_2))|((( 0) & (MAC_RFCR_Mask_2))<<2);\
		v = ((v & MAC_RFCR_RFE_Wr_Mask) | ((data & MAC_RFCR_RFE_Mask)<<0));\
		MAC_RFCR_RgWr(v);\
} while(0)

#define MAC_RFCR_RFE_UdfRd(data) do {\
		MAC_RFCR_RgRd(data);\
		data = ((data >> 0) & MAC_RFCR_RFE_Mask);\
} while(0)


#define MAC_QTFCR7_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x8c))

#define MAC_QTFCR7_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR7_RgOffAddr);\
} while(0)

#define MAC_QTFCR7_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR7_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR7_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR7_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR7_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR7_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR7_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR7_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR7_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR7_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR7_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR7_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR7_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR7_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR7_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR7_RgRd(v);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR7_Mask_8))<<8);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR7_Mask_2))<<2);\
		v = ((v & MAC_QTFCR7_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR7_FCB_BPA_Mask)<<0));\
		MAC_QTFCR7_RgWr(v);\
} while(0)

#define MAC_QTFCR7_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR7_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR7_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR7_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR7_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR7_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR7_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR7_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR7_RgRd(v);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR7_Mask_8))<<8);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR7_Mask_2))<<2);\
		v = ((v & MAC_QTFCR7_TFE_Wr_Mask) | ((data & MAC_QTFCR7_TFE_Mask)<<1));\
		MAC_QTFCR7_RgWr(v);\
} while(0)

#define MAC_QTFCR7_TFE_UdfRd(data) do {\
		MAC_QTFCR7_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR7_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR7_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR7_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR7_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR7_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR7_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR7_RgRd(v);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR7_Mask_8))<<8);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR7_Mask_2))<<2);\
		v = ((v & MAC_QTFCR7_PLT_Wr_Mask) | ((data & MAC_QTFCR7_PLT_Mask)<<4));\
		MAC_QTFCR7_RgWr(v);\
} while(0)

#define MAC_QTFCR7_PLT_UdfRd(data) do {\
		MAC_QTFCR7_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR7_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR7_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR7_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR7_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR7_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR7_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR7_RgRd(v);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR7_Mask_8))<<8);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR7_Mask_2))<<2);\
		v = ((v & MAC_QTFCR7_DZPQ_Wr_Mask) | ((data & MAC_QTFCR7_DZPQ_Mask)<<7));\
		MAC_QTFCR7_RgWr(v);\
} while(0)

#define MAC_QTFCR7_DZPQ_UdfRd(data) do {\
		MAC_QTFCR7_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR7_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR7_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR7_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR7_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR7_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR7_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR7_RgRd(v);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR7_Mask_8))<<8);\
		v = (v & (MAC_QTFCR7_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR7_Mask_2))<<2);\
		v = ((v & MAC_QTFCR7_PT_Wr_Mask) | ((data & MAC_QTFCR7_PT_Mask)<<16));\
		MAC_QTFCR7_RgWr(v);\
} while(0)

#define MAC_QTFCR7_PT_UdfRd(data) do {\
		MAC_QTFCR7_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR7_PT_Mask);\
} while(0)


#define MAC_QTFCR6_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x88))

#define MAC_QTFCR6_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR6_RgOffAddr);\
} while(0)

#define MAC_QTFCR6_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR6_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR6_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR6_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR6_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR6_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR6_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR6_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR6_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR6_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR6_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR6_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR6_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR6_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR6_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR6_RgRd(v);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR6_Mask_8))<<8);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR6_Mask_2))<<2);\
		v = ((v & MAC_QTFCR6_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR6_FCB_BPA_Mask)<<0));\
		MAC_QTFCR6_RgWr(v);\
} while(0)

#define MAC_QTFCR6_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR6_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR6_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR6_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR6_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR6_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR6_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR6_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR6_RgRd(v);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR6_Mask_8))<<8);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR6_Mask_2))<<2);\
		v = ((v & MAC_QTFCR6_TFE_Wr_Mask) | ((data & MAC_QTFCR6_TFE_Mask)<<1));\
		MAC_QTFCR6_RgWr(v);\
} while(0)

#define MAC_QTFCR6_TFE_UdfRd(data) do {\
		MAC_QTFCR6_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR6_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR6_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR6_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR6_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR6_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR6_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR6_RgRd(v);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR6_Mask_8))<<8);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR6_Mask_2))<<2);\
		v = ((v & MAC_QTFCR6_PLT_Wr_Mask) | ((data & MAC_QTFCR6_PLT_Mask)<<4));\
		MAC_QTFCR6_RgWr(v);\
} while(0)

#define MAC_QTFCR6_PLT_UdfRd(data) do {\
		MAC_QTFCR6_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR6_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR6_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR6_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR6_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR6_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR6_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR6_RgRd(v);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR6_Mask_8))<<8);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR6_Mask_2))<<2);\
		v = ((v & MAC_QTFCR6_DZPQ_Wr_Mask) | ((data & MAC_QTFCR6_DZPQ_Mask)<<7));\
		MAC_QTFCR6_RgWr(v);\
} while(0)

#define MAC_QTFCR6_DZPQ_UdfRd(data) do {\
		MAC_QTFCR6_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR6_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR6_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR6_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR6_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR6_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR6_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR6_RgRd(v);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR6_Mask_8))<<8);\
		v = (v & (MAC_QTFCR6_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR6_Mask_2))<<2);\
		v = ((v & MAC_QTFCR6_PT_Wr_Mask) | ((data & MAC_QTFCR6_PT_Mask)<<16));\
		MAC_QTFCR6_RgWr(v);\
} while(0)

#define MAC_QTFCR6_PT_UdfRd(data) do {\
		MAC_QTFCR6_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR6_PT_Mask);\
} while(0)


#define MAC_QTFCR5_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x84))

#define MAC_QTFCR5_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR5_RgOffAddr);\
} while(0)

#define MAC_QTFCR5_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR5_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR5_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR5_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR5_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR5_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR5_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR5_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR5_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR5_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR5_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR5_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR5_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR5_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR5_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR5_RgRd(v);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR5_Mask_8))<<8);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR5_Mask_2))<<2);\
		v = ((v & MAC_QTFCR5_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR5_FCB_BPA_Mask)<<0));\
		MAC_QTFCR5_RgWr(v);\
} while(0)

#define MAC_QTFCR5_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR5_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR5_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR5_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR5_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR5_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR5_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR5_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR5_RgRd(v);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR5_Mask_8))<<8);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR5_Mask_2))<<2);\
		v = ((v & MAC_QTFCR5_TFE_Wr_Mask) | ((data & MAC_QTFCR5_TFE_Mask)<<1));\
		MAC_QTFCR5_RgWr(v);\
} while(0)

#define MAC_QTFCR5_TFE_UdfRd(data) do {\
		MAC_QTFCR5_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR5_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR5_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR5_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR5_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR5_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR5_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR5_RgRd(v);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR5_Mask_8))<<8);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR5_Mask_2))<<2);\
		v = ((v & MAC_QTFCR5_PLT_Wr_Mask) | ((data & MAC_QTFCR5_PLT_Mask)<<4));\
		MAC_QTFCR5_RgWr(v);\
} while(0)

#define MAC_QTFCR5_PLT_UdfRd(data) do {\
		MAC_QTFCR5_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR5_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR5_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR5_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR5_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR5_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR5_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR5_RgRd(v);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR5_Mask_8))<<8);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR5_Mask_2))<<2);\
		v = ((v & MAC_QTFCR5_DZPQ_Wr_Mask) | ((data & MAC_QTFCR5_DZPQ_Mask)<<7));\
		MAC_QTFCR5_RgWr(v);\
} while(0)

#define MAC_QTFCR5_DZPQ_UdfRd(data) do {\
		MAC_QTFCR5_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR5_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR5_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR5_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR5_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR5_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR5_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR5_RgRd(v);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR5_Mask_8))<<8);\
		v = (v & (MAC_QTFCR5_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR5_Mask_2))<<2);\
		v = ((v & MAC_QTFCR5_PT_Wr_Mask) | ((data & MAC_QTFCR5_PT_Mask)<<16));\
		MAC_QTFCR5_RgWr(v);\
} while(0)

#define MAC_QTFCR5_PT_UdfRd(data) do {\
		MAC_QTFCR5_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR5_PT_Mask);\
} while(0)


#define MAC_QTFCR4_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x80))

#define MAC_QTFCR4_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR4_RgOffAddr);\
} while(0)

#define MAC_QTFCR4_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR4_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR4_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR4_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR4_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR4_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR4_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR4_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR4_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR4_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR4_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR4_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR4_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR4_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR4_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR4_RgRd(v);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR4_Mask_8))<<8);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR4_Mask_2))<<2);\
		v = ((v & MAC_QTFCR4_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR4_FCB_BPA_Mask)<<0));\
		MAC_QTFCR4_RgWr(v);\
} while(0)

#define MAC_QTFCR4_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR4_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR4_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR4_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR4_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR4_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR4_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR4_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR4_RgRd(v);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR4_Mask_8))<<8);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR4_Mask_2))<<2);\
		v = ((v & MAC_QTFCR4_TFE_Wr_Mask) | ((data & MAC_QTFCR4_TFE_Mask)<<1));\
		MAC_QTFCR4_RgWr(v);\
} while(0)

#define MAC_QTFCR4_TFE_UdfRd(data) do {\
		MAC_QTFCR4_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR4_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR4_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR4_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR4_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR4_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR4_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR4_RgRd(v);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR4_Mask_8))<<8);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR4_Mask_2))<<2);\
		v = ((v & MAC_QTFCR4_PLT_Wr_Mask) | ((data & MAC_QTFCR4_PLT_Mask)<<4));\
		MAC_QTFCR4_RgWr(v);\
} while(0)

#define MAC_QTFCR4_PLT_UdfRd(data) do {\
		MAC_QTFCR4_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR4_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR4_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR4_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR4_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR4_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR4_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR4_RgRd(v);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR4_Mask_8))<<8);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR4_Mask_2))<<2);\
		v = ((v & MAC_QTFCR4_DZPQ_Wr_Mask) | ((data & MAC_QTFCR4_DZPQ_Mask)<<7));\
		MAC_QTFCR4_RgWr(v);\
} while(0)

#define MAC_QTFCR4_DZPQ_UdfRd(data) do {\
		MAC_QTFCR4_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR4_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR4_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR4_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR4_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR4_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR4_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR4_RgRd(v);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR4_Mask_8))<<8);\
		v = (v & (MAC_QTFCR4_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR4_Mask_2))<<2);\
		v = ((v & MAC_QTFCR4_PT_Wr_Mask) | ((data & MAC_QTFCR4_PT_Mask)<<16));\
		MAC_QTFCR4_RgWr(v);\
} while(0)

#define MAC_QTFCR4_PT_UdfRd(data) do {\
		MAC_QTFCR4_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR4_PT_Mask);\
} while(0)


#define MAC_QTFCR3_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x7c))

#define MAC_QTFCR3_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR3_RgOffAddr);\
} while(0)

#define MAC_QTFCR3_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR3_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR3_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR3_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR3_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR3_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR3_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR3_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR3_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR3_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR3_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR3_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR3_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR3_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR3_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR3_RgRd(v);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR3_Mask_8))<<8);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR3_Mask_2))<<2);\
		v = ((v & MAC_QTFCR3_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR3_FCB_BPA_Mask)<<0));\
		MAC_QTFCR3_RgWr(v);\
} while(0)

#define MAC_QTFCR3_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR3_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR3_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR3_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR3_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR3_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR3_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR3_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR3_RgRd(v);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR3_Mask_8))<<8);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR3_Mask_2))<<2);\
		v = ((v & MAC_QTFCR3_TFE_Wr_Mask) | ((data & MAC_QTFCR3_TFE_Mask)<<1));\
		MAC_QTFCR3_RgWr(v);\
} while(0)

#define MAC_QTFCR3_TFE_UdfRd(data) do {\
		MAC_QTFCR3_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR3_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR3_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR3_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR3_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR3_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR3_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR3_RgRd(v);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR3_Mask_8))<<8);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR3_Mask_2))<<2);\
		v = ((v & MAC_QTFCR3_PLT_Wr_Mask) | ((data & MAC_QTFCR3_PLT_Mask)<<4));\
		MAC_QTFCR3_RgWr(v);\
} while(0)

#define MAC_QTFCR3_PLT_UdfRd(data) do {\
		MAC_QTFCR3_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR3_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR3_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR3_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR3_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR3_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR3_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR3_RgRd(v);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR3_Mask_8))<<8);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR3_Mask_2))<<2);\
		v = ((v & MAC_QTFCR3_DZPQ_Wr_Mask) | ((data & MAC_QTFCR3_DZPQ_Mask)<<7));\
		MAC_QTFCR3_RgWr(v);\
} while(0)

#define MAC_QTFCR3_DZPQ_UdfRd(data) do {\
		MAC_QTFCR3_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR3_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR3_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR3_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR3_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR3_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR3_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR3_RgRd(v);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR3_Mask_8))<<8);\
		v = (v & (MAC_QTFCR3_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR3_Mask_2))<<2);\
		v = ((v & MAC_QTFCR3_PT_Wr_Mask) | ((data & MAC_QTFCR3_PT_Mask)<<16));\
		MAC_QTFCR3_RgWr(v);\
} while(0)

#define MAC_QTFCR3_PT_UdfRd(data) do {\
		MAC_QTFCR3_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR3_PT_Mask);\
} while(0)


#define MAC_QTFCR2_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x78))

#define MAC_QTFCR2_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR2_RgOffAddr);\
} while(0)

#define MAC_QTFCR2_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR2_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR2_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR2_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR2_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR2_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR2_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR2_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR2_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR2_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR2_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR2_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR2_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR2_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR2_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR2_RgRd(v);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR2_Mask_8))<<8);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR2_Mask_2))<<2);\
		v = ((v & MAC_QTFCR2_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR2_FCB_BPA_Mask)<<0));\
		MAC_QTFCR2_RgWr(v);\
} while(0)

#define MAC_QTFCR2_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR2_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR2_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR2_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR2_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR2_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR2_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR2_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR2_RgRd(v);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR2_Mask_8))<<8);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR2_Mask_2))<<2);\
		v = ((v & MAC_QTFCR2_TFE_Wr_Mask) | ((data & MAC_QTFCR2_TFE_Mask)<<1));\
		MAC_QTFCR2_RgWr(v);\
} while(0)

#define MAC_QTFCR2_TFE_UdfRd(data) do {\
		MAC_QTFCR2_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR2_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR2_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR2_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR2_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR2_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR2_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR2_RgRd(v);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR2_Mask_8))<<8);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR2_Mask_2))<<2);\
		v = ((v & MAC_QTFCR2_PLT_Wr_Mask) | ((data & MAC_QTFCR2_PLT_Mask)<<4));\
		MAC_QTFCR2_RgWr(v);\
} while(0)

#define MAC_QTFCR2_PLT_UdfRd(data) do {\
		MAC_QTFCR2_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR2_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR2_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR2_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR2_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR2_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR2_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR2_RgRd(v);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR2_Mask_8))<<8);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR2_Mask_2))<<2);\
		v = ((v & MAC_QTFCR2_DZPQ_Wr_Mask) | ((data & MAC_QTFCR2_DZPQ_Mask)<<7));\
		MAC_QTFCR2_RgWr(v);\
} while(0)

#define MAC_QTFCR2_DZPQ_UdfRd(data) do {\
		MAC_QTFCR2_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR2_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR2_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR2_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR2_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR2_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR2_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR2_RgRd(v);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR2_Mask_8))<<8);\
		v = (v & (MAC_QTFCR2_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR2_Mask_2))<<2);\
		v = ((v & MAC_QTFCR2_PT_Wr_Mask) | ((data & MAC_QTFCR2_PT_Mask)<<16));\
		MAC_QTFCR2_RgWr(v);\
} while(0)

#define MAC_QTFCR2_PT_UdfRd(data) do {\
		MAC_QTFCR2_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR2_PT_Mask);\
} while(0)


#define MAC_QTFCR1_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x74))

#define MAC_QTFCR1_RgWr(data) do {\
		iowrite32(data, (void *)MAC_QTFCR1_RgOffAddr);\
} while(0)

#define MAC_QTFCR1_RgRd(data) do {\
		(data) = ioread32((void *)MAC_QTFCR1_RgOffAddr);\
} while(0)

/*#define  MAC_QTFCR1_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR1_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR1_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR1_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR1_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR1_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR1_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR1_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR1_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR1_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR1_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR1_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR1_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR1_RgRd(v);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR1_Mask_8))<<8);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR1_Mask_2))<<2);\
		v = ((v & MAC_QTFCR1_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR1_FCB_BPA_Mask)<<0));\
		MAC_QTFCR1_RgWr(v);\
} while(0)

#define MAC_QTFCR1_FCB_BPA_UdfRd(data) do {\
		MAC_QTFCR1_RgRd(data);\
		data = ((data >> 0) & MAC_QTFCR1_FCB_BPA_Mask);\
} while(0)

/*#define MAC_QTFCR1_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR1_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR1_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR1_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR1_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR1_RgRd(v);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR1_Mask_8))<<8);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR1_Mask_2))<<2);\
		v = ((v & MAC_QTFCR1_TFE_Wr_Mask) | ((data & MAC_QTFCR1_TFE_Mask)<<1));\
		MAC_QTFCR1_RgWr(v);\
} while(0)

#define MAC_QTFCR1_TFE_UdfRd(data) do {\
		MAC_QTFCR1_RgRd(data);\
		data = ((data >> 1) & MAC_QTFCR1_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR1_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR1_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR1_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR1_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR1_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR1_RgRd(v);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR1_Mask_8))<<8);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR1_Mask_2))<<2);\
		v = ((v & MAC_QTFCR1_PLT_Wr_Mask) | ((data & MAC_QTFCR1_PLT_Mask)<<4));\
		MAC_QTFCR1_RgWr(v);\
} while(0)

#define MAC_QTFCR1_PLT_UdfRd(data) do {\
		MAC_QTFCR1_RgRd(data);\
		data = ((data >> 4) & MAC_QTFCR1_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR1_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR1_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR1_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR1_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR1_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR1_RgRd(v);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR1_Mask_8))<<8);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR1_Mask_2))<<2);\
		v = ((v & MAC_QTFCR1_DZPQ_Wr_Mask) | ((data & MAC_QTFCR1_DZPQ_Mask)<<7));\
		MAC_QTFCR1_RgWr(v);\
} while(0)

#define MAC_QTFCR1_DZPQ_UdfRd(data) do {\
		MAC_QTFCR1_RgRd(data);\
		data = ((data >> 7) & MAC_QTFCR1_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR1_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR1_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR1_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR1_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR1_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_QTFCR1_RgRd(v);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR1_Mask_8))<<8);\
		v = (v & (MAC_QTFCR1_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR1_Mask_2))<<2);\
		v = ((v & MAC_QTFCR1_PT_Wr_Mask) | ((data & MAC_QTFCR1_PT_Mask)<<16));\
		MAC_QTFCR1_RgWr(v);\
} while(0)

#define MAC_QTFCR1_PT_UdfRd(data) do {\
		MAC_QTFCR1_RgRd(data);\
		data = ((data >> 16) & MAC_QTFCR1_PT_Mask);\
} while(0)


#define MAC_Q0TFCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x70))

#define MAC_Q0TFCR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_Q0TFCR_RgOffAddr);\
} while(0)

#define MAC_Q0TFCR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_Q0TFCR_RgOffAddr);\
} while(0)

/*#define  MAC_Q0TFCR_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_Q0TFCR_Mask_8 (ULONG)(0xff)

/*#define MAC_Q0TFCR_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_Q0TFCR_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_Q0TFCR_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_Q0TFCR_Mask_2 (ULONG)(0x3)

/*#define MAC_Q0TFCR_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_Q0TFCR_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_Q0TFCR_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_Q0TFCR_PT_Mask (ULONG)(0xffff)

/*#define MAC_Q0TFCR_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_Q0TFCR_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_Q0TFCR_PT_UdfWr(data) do {\
		ULONG v;\
		MAC_Q0TFCR_RgRd(v);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_8))|((( 0) & (MAC_Q0TFCR_Mask_8))<<8);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_2))|((( 0) & (MAC_Q0TFCR_Mask_2))<<2);\
		v = ((v & MAC_Q0TFCR_PT_Wr_Mask) | ((data & MAC_Q0TFCR_PT_Mask)<<16));\
		MAC_Q0TFCR_RgWr(v);\
} while(0)

#define MAC_Q0TFCR_PT_UdfRd(data) do {\
		MAC_Q0TFCR_RgRd(data);\
		data = ((data >> 16) & MAC_Q0TFCR_PT_Mask);\
} while(0)

/*#define MAC_Q0TFCR_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_Q0TFCR_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_Q0TFCR_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_Q0TFCR_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_Q0TFCR_DZPQ_UdfWr(data) do {\
		ULONG v;\
		MAC_Q0TFCR_RgRd(v);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_8))|((( 0) & (MAC_Q0TFCR_Mask_8))<<8);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_2))|((( 0) & (MAC_Q0TFCR_Mask_2))<<2);\
		v = ((v & MAC_Q0TFCR_DZPQ_Wr_Mask) | ((data & MAC_Q0TFCR_DZPQ_Mask)<<7));\
		MAC_Q0TFCR_RgWr(v);\
} while(0)

#define MAC_Q0TFCR_DZPQ_UdfRd(data) do {\
		MAC_Q0TFCR_RgRd(data);\
		data = ((data >> 7) & MAC_Q0TFCR_DZPQ_Mask);\
} while(0)

/*#define MAC_Q0TFCR_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_Q0TFCR_PLT_Mask (ULONG)(0x7)

/*#define MAC_Q0TFCR_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_Q0TFCR_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_Q0TFCR_PLT_UdfWr(data) do {\
		ULONG v;\
		MAC_Q0TFCR_RgRd(v);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_8))|((( 0) & (MAC_Q0TFCR_Mask_8))<<8);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_2))|((( 0) & (MAC_Q0TFCR_Mask_2))<<2);\
		v = ((v & MAC_Q0TFCR_PLT_Wr_Mask) | ((data & MAC_Q0TFCR_PLT_Mask)<<4));\
		MAC_Q0TFCR_RgWr(v);\
} while(0)

#define MAC_Q0TFCR_PLT_UdfRd(data) do {\
		MAC_Q0TFCR_RgRd(data);\
		data = ((data >> 4) & MAC_Q0TFCR_PLT_Mask);\
} while(0)

/*#define MAC_Q0TFCR_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_Q0TFCR_TFE_Mask (ULONG)(0x1)

/*#define MAC_Q0TFCR_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_Q0TFCR_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_Q0TFCR_TFE_UdfWr(data) do {\
		ULONG v;\
		MAC_Q0TFCR_RgRd(v);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_8))|((( 0) & (MAC_Q0TFCR_Mask_8))<<8);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_2))|((( 0) & (MAC_Q0TFCR_Mask_2))<<2);\
		v = ((v & MAC_Q0TFCR_TFE_Wr_Mask) | ((data & MAC_Q0TFCR_TFE_Mask)<<1));\
		MAC_Q0TFCR_RgWr(v);\
} while(0)

#define MAC_Q0TFCR_TFE_UdfRd(data) do {\
		MAC_Q0TFCR_RgRd(data);\
		data = ((data >> 1) & MAC_Q0TFCR_TFE_Mask);\
} while(0)

/*#define MAC_Q0TFCR_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_Q0TFCR_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_Q0TFCR_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_Q0TFCR_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_Q0TFCR_FCB_BPA_UdfWr(data) do {\
		ULONG v;\
		MAC_Q0TFCR_RgRd(v);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_8))|((( 0) & (MAC_Q0TFCR_Mask_8))<<8);\
		v = (v & (MAC_Q0TFCR_RES_Wr_Mask_2))|((( 0) & (MAC_Q0TFCR_Mask_2))<<2);\
		v = ((v & MAC_Q0TFCR_FCB_BPA_Wr_Mask) | ((data & MAC_Q0TFCR_FCB_BPA_Mask)<<0));\
		MAC_Q0TFCR_RgWr(v);\
} while(0)

#define MAC_Q0TFCR_FCB_BPA_UdfRd(data) do {\
		MAC_Q0TFCR_RgRd(data);\
		data = ((data >> 0) & MAC_Q0TFCR_FCB_BPA_Mask);\
} while(0)



#define MAC_WTR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0xc))

#define MAC_WTR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_WTR_RgOffAddr);\
} while(0)

#define MAC_WTR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_WTR_RgOffAddr);\
} while(0)

/*#define  MAC_WTR_Mask_17 (ULONG)(~(~0<<(15)))*/

#define  MAC_WTR_Mask_17 (ULONG)(0x7fff)

/*#define MAC_WTR_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(15)))<<(17)))*/

#define MAC_WTR_RES_Wr_Mask_17 (ULONG)(0x1ffff)

/*#define  MAC_WTR_Mask_14 (ULONG)(~(~0<<(2)))*/

#define  MAC_WTR_Mask_14 (ULONG)(0x3)

/*#define MAC_WTR_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(2)))<<(14)))*/

#define MAC_WTR_RES_Wr_Mask_14 (ULONG)(0xffff3fff)

/*#define MAC_WTR_PWE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_WTR_PWE_Mask (ULONG)(0x1)

/*#define MAC_WTR_PWE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_WTR_PWE_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_WTR_PWE_UdfWr(data) do {\
		ULONG v;\
		MAC_WTR_RgRd(v);\
		v = (v & (MAC_WTR_RES_Wr_Mask_17))|((( 0) & (MAC_WTR_Mask_17))<<17);\
		v = (v & (MAC_WTR_RES_Wr_Mask_14))|((( 0) & (MAC_WTR_Mask_14))<<14);\
		v = ((v & MAC_WTR_PWE_Wr_Mask) | ((data & MAC_WTR_PWE_Mask)<<16));\
		MAC_WTR_RgWr(v);\
} while(0)

#define MAC_WTR_PWE_UdfRd(data) do {\
		MAC_WTR_RgRd(data);\
		data = ((data >> 16) & MAC_WTR_PWE_Mask);\
} while(0)

/*#define MAC_WTR_WTO_Mask (ULONG)(~(~0<<(14)))*/

#define MAC_WTR_WTO_Mask (ULONG)(0x3fff)

/*#define MAC_WTR_WTO_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MAC_WTR_WTO_Wr_Mask (ULONG)(0xffffc000)

#define MAC_WTR_WTO_UdfWr(data) do {\
		ULONG v;\
		MAC_WTR_RgRd(v);\
		v = (v & (MAC_WTR_RES_Wr_Mask_17))|((( 0) & (MAC_WTR_Mask_17))<<17);\
		v = (v & (MAC_WTR_RES_Wr_Mask_14))|((( 0) & (MAC_WTR_Mask_14))<<14);\
		v = ((v & MAC_WTR_WTO_Wr_Mask) | ((data & MAC_WTR_WTO_Mask)<<0));\
		MAC_WTR_RgWr(v);\
} while(0)

#define MAC_WTR_WTO_UdfRd(data) do {\
		MAC_WTR_RgRd(data);\
		data = ((data >> 0) & MAC_WTR_WTO_Mask);\
} while(0)


#define MAC_MPFR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x8))

#define MAC_MPFR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MPFR_RgOffAddr);\
} while(0)

#define MAC_MPFR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MPFR_RgOffAddr);\
} while(0)

/*#define  MAC_MPFR_Mask_22 (ULONG)(~(~0<<(9)))*/

#define  MAC_MPFR_Mask_22 (ULONG)(0x1ff)

/*#define MAC_MPFR_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(9)))<<(22)))*/

#define MAC_MPFR_RES_Wr_Mask_22 (ULONG)(0x803fffff)

/*#define  MAC_MPFR_Mask_17 (ULONG)(~(~0<<(3)))*/

#define  MAC_MPFR_Mask_17 (ULONG)(0x7)

/*#define MAC_MPFR_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(3)))<<(17)))*/

#define MAC_MPFR_RES_Wr_Mask_17 (ULONG)(0xfff1ffff)

/*#define  MAC_MPFR_Mask_11 (ULONG)(~(~0<<(5)))*/

#define  MAC_MPFR_Mask_11 (ULONG)(0x1f)

/*#define MAC_MPFR_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(5)))<<(11)))*/

#define MAC_MPFR_RES_Wr_Mask_11 (ULONG)(0xffff07ff)

/*#define MAC_MPFR_RA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_RA_Mask (ULONG)(0x1)

/*#define MAC_MPFR_RA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_MPFR_RA_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MPFR_RA_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_RA_Wr_Mask) | ((data & MAC_MPFR_RA_Mask)<<31));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_RA_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 31) & MAC_MPFR_RA_Mask);\
} while(0)

/*#define MAC_MPFR_DNTU_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_DNTU_Mask (ULONG)(0x1)

/*#define MAC_MPFR_DNTU_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_MPFR_DNTU_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_MPFR_DNTU_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_DNTU_Wr_Mask) | ((data & MAC_MPFR_DNTU_Mask)<<21));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_DNTU_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 21) & MAC_MPFR_DNTU_Mask);\
} while(0)

/*#define MAC_MPFR_IPFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_IPFE_Mask (ULONG)(0x1)

/*#define MAC_MPFR_IPFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_MPFR_IPFE_Wr_Mask (ULONG)(0xffefffff)

#define MAC_MPFR_IPFE_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_IPFE_Wr_Mask) | ((data & MAC_MPFR_IPFE_Mask)<<20));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_IPFE_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 20) & MAC_MPFR_IPFE_Mask);\
} while(0)

/*#define MAC_MPFR_VTFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_VTFE_Mask (ULONG)(0x1)

/*#define MAC_MPFR_VTFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_MPFR_VTFE_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_MPFR_VTFE_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_VTFE_Wr_Mask) | ((data & MAC_MPFR_VTFE_Mask)<<16));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_VTFE_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 16) & MAC_MPFR_VTFE_Mask);\
} while(0)

/*#define MAC_MPFR_HPF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_HPF_Mask (ULONG)(0x1)

/*#define MAC_MPFR_HPF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MAC_MPFR_HPF_Wr_Mask (ULONG)(0xfffffbff)

#define MAC_MPFR_HPF_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_HPF_Wr_Mask) | ((data & MAC_MPFR_HPF_Mask)<<10));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_HPF_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 10) & MAC_MPFR_HPF_Mask);\
} while(0)

/*#define MAC_MPFR_SAF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_SAF_Mask (ULONG)(0x1)

/*#define MAC_MPFR_SAF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MAC_MPFR_SAF_Wr_Mask (ULONG)(0xfffffdff)

#define MAC_MPFR_SAF_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_SAF_Wr_Mask) | ((data & MAC_MPFR_SAF_Mask)<<9));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_SAF_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 9) & MAC_MPFR_SAF_Mask);\
} while(0)

/*#define MAC_MPFR_SAIF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_SAIF_Mask (ULONG)(0x1)

/*#define MAC_MPFR_SAIF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MAC_MPFR_SAIF_Wr_Mask (ULONG)(0xfffffeff)

#define MAC_MPFR_SAIF_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_SAIF_Wr_Mask) | ((data & MAC_MPFR_SAIF_Mask)<<8));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_SAIF_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 8) & MAC_MPFR_SAIF_Mask);\
} while(0)

/*#define MAC_MPFR_PCF_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_MPFR_PCF_Mask (ULONG)(0x3)

/*#define MAC_MPFR_PCF_Wr_Mask (ULONG)(~((~(~0 << (2))) << (6)))*/

#define MAC_MPFR_PCF_Wr_Mask (ULONG)(0xffffff3f)

#define MAC_MPFR_PCF_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_PCF_Wr_Mask) | ((data & MAC_MPFR_PCF_Mask)<<6));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_PCF_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 6) & MAC_MPFR_PCF_Mask);\
} while(0)

/*#define MAC_MPFR_DBF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_DBF_Mask (ULONG)(0x1)

/*#define MAC_MPFR_DBF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_MPFR_DBF_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_MPFR_DBF_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_DBF_Wr_Mask) | ((data & MAC_MPFR_DBF_Mask)<<5));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_DBF_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 5) & MAC_MPFR_DBF_Mask);\
} while(0)

/*#define MAC_MPFR_PM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_PM_Mask (ULONG)(0x1)

/*#define MAC_MPFR_PM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_MPFR_PM_Wr_Mask (ULONG)(0xffffffef)

#define MAC_MPFR_PM_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_PM_Wr_Mask) | ((data & MAC_MPFR_PM_Mask)<<4));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_PM_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 4) & MAC_MPFR_PM_Mask);\
} while(0)

/*#define MAC_MPFR_DAIF_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_DAIF_Mask (ULONG)(0x1)

/*#define MAC_MPFR_DAIF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_MPFR_DAIF_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_MPFR_DAIF_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_DAIF_Wr_Mask) | ((data & MAC_MPFR_DAIF_Mask)<<3));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_DAIF_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 3) & MAC_MPFR_DAIF_Mask);\
} while(0)

/*#define MAC_MPFR_HMC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_HMC_Mask (ULONG)(0x1)

/*#define MAC_MPFR_HMC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_MPFR_HMC_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_MPFR_HMC_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_HMC_Wr_Mask) | ((data & MAC_MPFR_HMC_Mask)<<2));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_HMC_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 2) & MAC_MPFR_HMC_Mask);\
} while(0)

/*#define MAC_MPFR_HUC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_HUC_Mask (ULONG)(0x1)

/*#define MAC_MPFR_HUC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_MPFR_HUC_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_MPFR_HUC_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_HUC_Wr_Mask) | ((data & MAC_MPFR_HUC_Mask)<<1));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_HUC_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 1) & MAC_MPFR_HUC_Mask);\
} while(0)

/*#define MAC_MPFR_PR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MPFR_PR_Mask (ULONG)(0x1)

/*#define MAC_MPFR_PR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_MPFR_PR_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_MPFR_PR_UdfWr(data) do {\
		ULONG v;\
		MAC_MPFR_RgRd(v);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_22))|((( 0) & (MAC_MPFR_Mask_22))<<22);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_17))|((( 0) & (MAC_MPFR_Mask_17))<<17);\
		v = (v & (MAC_MPFR_RES_Wr_Mask_11))|((( 0) & (MAC_MPFR_Mask_11))<<11);\
		v = ((v & MAC_MPFR_PR_Wr_Mask) | ((data & MAC_MPFR_PR_Mask)<<0));\
		MAC_MPFR_RgWr(v);\
} while(0)

#define MAC_MPFR_PR_UdfRd(data) do {\
		MAC_MPFR_RgRd(data);\
		data = ((data >> 0) & MAC_MPFR_PR_Mask);\
} while(0)


#define MAC_MECR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0x4))

#define MAC_MECR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MECR_RgOffAddr);\
} while(0)

#define MAC_MECR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MECR_RgOffAddr);\
} while(0)

/*#define  MAC_MECR_Mask_23 (ULONG)(~(~0<<(9)))*/

#define  MAC_MECR_Mask_23 (ULONG)(0x1ff)

/*#define MAC_MECR_RES_Wr_Mask_23 (ULONG)(~((~(~0<<(9)))<<(23)))*/

#define MAC_MECR_RES_Wr_Mask_23 (ULONG)(0x7fffff)

/*#define  MAC_MECR_Mask_19 (ULONG)(~(~0<<(1)))*/

#define  MAC_MECR_Mask_19 (ULONG)(0x1)

/*#define MAC_MECR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(1)))<<(19)))*/

#define MAC_MECR_RES_Wr_Mask_19 (ULONG)(0xfff7ffff)

/*#define  MAC_MECR_Mask_15 (ULONG)(~(~0<<(1)))*/

#define  MAC_MECR_Mask_15 (ULONG)(0x1)

/*#define MAC_MECR_RES_Wr_Mask_15 (ULONG)(~((~(~0<<(1)))<<(15)))*/

#define MAC_MECR_RES_Wr_Mask_15 (ULONG)(0xffff7fff)

/*#define MAC_MECR_HDSMS_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MECR_HDSMS_Mask (ULONG)(0x7)

/*#define MAC_MECR_HDSMS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (20)))*/

#define MAC_MECR_HDSMS_Wr_Mask (ULONG)(0xff8fffff)

#define MAC_MECR_HDSMS_UdfWr(data) do {\
		ULONG v;\
		MAC_MECR_RgRd(v);\
		v = (v & (MAC_MECR_RES_Wr_Mask_23))|((( 0) & (MAC_MECR_Mask_23))<<23);\
		v = (v & (MAC_MECR_RES_Wr_Mask_19))|((( 0) & (MAC_MECR_Mask_19))<<19);\
		v = (v & (MAC_MECR_RES_Wr_Mask_15))|((( 0) & (MAC_MECR_Mask_15))<<15);\
		v = ((v & MAC_MECR_HDSMS_Wr_Mask) | ((data & MAC_MECR_HDSMS_Mask)<<20));\
		MAC_MECR_RgWr(v);\
} while(0)

#define MAC_MECR_HDSMS_UdfRd(data) do {\
		MAC_MECR_RgRd(data);\
		data = ((data >> 20) & MAC_MECR_HDSMS_Mask);\
} while(0)

/*#define MAC_MECR_USP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MECR_USP_Mask (ULONG)(0x1)

/*#define MAC_MECR_USP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_MECR_USP_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_MECR_USP_UdfWr(data) do {\
		ULONG v;\
		MAC_MECR_RgRd(v);\
		v = (v & (MAC_MECR_RES_Wr_Mask_23))|((( 0) & (MAC_MECR_Mask_23))<<23);\
		v = (v & (MAC_MECR_RES_Wr_Mask_19))|((( 0) & (MAC_MECR_Mask_19))<<19);\
		v = (v & (MAC_MECR_RES_Wr_Mask_15))|((( 0) & (MAC_MECR_Mask_15))<<15);\
		v = ((v & MAC_MECR_USP_Wr_Mask) | ((data & MAC_MECR_USP_Mask)<<18));\
		MAC_MECR_RgWr(v);\
} while(0)

#define MAC_MECR_USP_UdfRd(data) do {\
		MAC_MECR_RgRd(data);\
		data = ((data >> 18) & MAC_MECR_USP_Mask);\
} while(0)

/*#define MAC_MECR_SPEN_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MECR_SPEN_Mask (ULONG)(0x1)

/*#define MAC_MECR_SPEN_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MAC_MECR_SPEN_Wr_Mask (ULONG)(0xfffdffff)

#define MAC_MECR_SPEN_UdfWr(data) do {\
		ULONG v;\
		MAC_MECR_RgRd(v);\
		v = (v & (MAC_MECR_RES_Wr_Mask_23))|((( 0) & (MAC_MECR_Mask_23))<<23);\
		v = (v & (MAC_MECR_RES_Wr_Mask_19))|((( 0) & (MAC_MECR_Mask_19))<<19);\
		v = (v & (MAC_MECR_RES_Wr_Mask_15))|((( 0) & (MAC_MECR_Mask_15))<<15);\
		v = ((v & MAC_MECR_SPEN_Wr_Mask) | ((data & MAC_MECR_SPEN_Mask)<<17));\
		MAC_MECR_RgWr(v);\
} while(0)

#define MAC_MECR_SPEN_UdfRd(data) do {\
		MAC_MECR_RgRd(data);\
		data = ((data >> 17) & MAC_MECR_SPEN_Mask);\
} while(0)

/*#define MAC_MECR_DCRCC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MECR_DCRCC_Mask (ULONG)(0x1)

/*#define MAC_MECR_DCRCC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_MECR_DCRCC_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_MECR_DCRCC_UdfWr(data) do {\
		ULONG v;\
		MAC_MECR_RgRd(v);\
		v = (v & (MAC_MECR_RES_Wr_Mask_23))|((( 0) & (MAC_MECR_Mask_23))<<23);\
		v = (v & (MAC_MECR_RES_Wr_Mask_19))|((( 0) & (MAC_MECR_Mask_19))<<19);\
		v = (v & (MAC_MECR_RES_Wr_Mask_15))|((( 0) & (MAC_MECR_Mask_15))<<15);\
		v = ((v & MAC_MECR_DCRCC_Wr_Mask) | ((data & MAC_MECR_DCRCC_Mask)<<16));\
		MAC_MECR_RgWr(v);\
} while(0)

#define MAC_MECR_DCRCC_UdfRd(data) do {\
		MAC_MECR_RgRd(data);\
		data = ((data >> 16) & MAC_MECR_DCRCC_Mask);\
} while(0)

/*#define MAC_MECR_GPSL_Mask (ULONG)(~(~0<<(15)))*/

#define MAC_MECR_GPSL_Mask (ULONG)(0x7fff)

/*#define MAC_MECR_GPSL_Wr_Mask (ULONG)(~((~(~0 << (15))) << (0)))*/

#define MAC_MECR_GPSL_Wr_Mask (ULONG)(0xffff8000)

#define MAC_MECR_GPSL_UdfWr(data) do {\
		ULONG v;\
		MAC_MECR_RgRd(v);\
		v = (v & (MAC_MECR_RES_Wr_Mask_23))|((( 0) & (MAC_MECR_Mask_23))<<23);\
		v = (v & (MAC_MECR_RES_Wr_Mask_19))|((( 0) & (MAC_MECR_Mask_19))<<19);\
		v = (v & (MAC_MECR_RES_Wr_Mask_15))|((( 0) & (MAC_MECR_Mask_15))<<15);\
		v = ((v & MAC_MECR_GPSL_Wr_Mask) | ((data & MAC_MECR_GPSL_Mask)<<0));\
		MAC_MECR_RgWr(v);\
} while(0)

#define MAC_MECR_GPSL_UdfRd(data) do {\
		MAC_MECR_RgRd(data);\
		data = ((data >> 0) & MAC_MECR_GPSL_Mask);\
} while(0)


#define MAC_MCR_RgOffAddr ((volatile ULONG *)(BASE_ADDRESS + 0))

#define MAC_MCR_RgWr(data) do {\
		iowrite32(data, (void *)MAC_MCR_RgOffAddr);\
} while(0)

#define MAC_MCR_RgRd(data) do {\
		(data) = ioread32((void *)MAC_MCR_RgOffAddr);\
} while(0)

/*#define  MAC_MCR_Mask_7 (ULONG)(~(~0<<(1)))*/

#define  MAC_MCR_Mask_7 (ULONG)(0x1)

/*#define MAC_MCR_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(1)))<<(7)))*/

#define MAC_MCR_RES_Wr_Mask_7 (ULONG)(0xffffff7f)

/*#define MAC_MCR_ARPEN_Mask (unsigned long)(~(~0<<(1)))*/

#define MAC_MCR_ARPEN_Mask (ULONG)(0x1)

/*#define MAC_MCR_ARPEN_Wr_Mask (unsigned long)(~((~(~0 << (1))) << (31)))*/

#define MAC_MCR_ARPEN_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MCR_ARPEN_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_ARPEN_Wr_Mask) | ((data & MAC_MCR_ARPEN_Mask)<<31));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_ARPEN_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 31) & MAC_MCR_ARPEN_Mask);\
} while(0)

/*#define MAC_MCR_SARC_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MCR_SARC_Mask (ULONG)(0x7)

/*#define MAC_MCR_SARC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (28)))*/

#define MAC_MCR_SARC_Wr_Mask (ULONG)(0x8fffffff)

#define MAC_MCR_SARC_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_SARC_Wr_Mask) | ((data & MAC_MCR_SARC_Mask)<<28));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_SARC_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 28) & MAC_MCR_SARC_Mask);\
} while(0)

/*#define MAC_MCR_IPC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_IPC_Mask (ULONG)(0x1)

/*#define MAC_MCR_IPC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MAC_MCR_IPC_Wr_Mask (ULONG)(0xf7ffffff)

#define MAC_MCR_IPC_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_IPC_Wr_Mask) | ((data & MAC_MCR_IPC_Mask)<<27));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_IPC_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 27) & MAC_MCR_IPC_Mask);\
} while(0)

/*#define MAC_MCR_IFG_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MCR_IFG_Mask (ULONG)(0x7)

/*#define MAC_MCR_IFG_Wr_Mask (ULONG)(~((~(~0 << (3))) << (24)))*/

#define MAC_MCR_IFG_Wr_Mask (ULONG)(0xf8ffffff)

#define MAC_MCR_IFG_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_IFG_Wr_Mask) | ((data & MAC_MCR_IFG_Mask)<<24));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_IFG_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 24) & MAC_MCR_IFG_Mask);\
} while(0)

/*#define MAC_MCR_GPSLCE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_GPSLCE_Mask (ULONG)(0x1)

/*#define MAC_MCR_GPSLCE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (23)))*/

#define MAC_MCR_GPSLCE_Wr_Mask (ULONG)(0xff7fffff)

#define MAC_MCR_GPSLCE_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_GPSLCE_Wr_Mask) | ((data & MAC_MCR_GPSLCE_Mask)<<23));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_GPSLCE_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 23) & MAC_MCR_GPSLCE_Mask);\
} while(0)

/*#define MAC_MCR_S2KP_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_S2KP_Mask (ULONG)(0x1)

/*#define MAC_MCR_S2KP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (22)))*/

#define MAC_MCR_S2KP_Wr_Mask (ULONG)(0xffbfffff)

#define MAC_MCR_S2KP_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_S2KP_Wr_Mask) | ((data & MAC_MCR_S2KP_Mask)<<22));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_S2KP_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 22) & MAC_MCR_S2KP_Mask);\
} while(0)

/*#define MAC_MCR_CST_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_CST_Mask (ULONG)(0x1)

/*#define MAC_MCR_CST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_MCR_CST_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_MCR_CST_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_CST_Wr_Mask) | ((data & MAC_MCR_CST_Mask)<<21));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_CST_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 21) & MAC_MCR_CST_Mask);\
} while(0)

/*#define MAC_MCR_ACS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_ACS_Mask (ULONG)(0x1)

/*#define MAC_MCR_ACS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_MCR_ACS_Wr_Mask (ULONG)(0xffefffff)

#define MAC_MCR_ACS_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_ACS_Wr_Mask) | ((data & MAC_MCR_ACS_Mask)<<20));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_ACS_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 20) & MAC_MCR_ACS_Mask);\
} while(0)

/*#define MAC_MCR_WD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_WD_Mask (ULONG)(0x1)

/*#define MAC_MCR_WD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_MCR_WD_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_MCR_WD_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_WD_Wr_Mask) | ((data & MAC_MCR_WD_Mask)<<19));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_WD_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 19) & MAC_MCR_WD_Mask);\
} while(0)

/*#define MAC_MCR_BE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_BE_Mask (ULONG)(0x1)

/*#define MAC_MCR_BE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_MCR_BE_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_MCR_BE_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_BE_Wr_Mask) | ((data & MAC_MCR_BE_Mask)<<18));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_BE_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 18) & MAC_MCR_BE_Mask);\
} while(0)

/*#define MAC_MCR_JD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_JD_Mask (ULONG)(0x1)

/*#define MAC_MCR_JD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (17)))*/

#define MAC_MCR_JD_Wr_Mask (ULONG)(0xfffdffff)

#define MAC_MCR_JD_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_JD_Wr_Mask) | ((data & MAC_MCR_JD_Mask)<<17));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_JD_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 17) & MAC_MCR_JD_Mask);\
} while(0)

/*#define MAC_MCR_JE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_JE_Mask (ULONG)(0x1)

/*#define MAC_MCR_JE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_MCR_JE_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_MCR_JE_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_JE_Wr_Mask) | ((data & MAC_MCR_JE_Mask)<<16));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_JE_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 16) & MAC_MCR_JE_Mask);\
} while(0)

/*#define MAC_MCR_PS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_PS_Mask (ULONG)(0x1)

/*#define MAC_MCR_PS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/

#define MAC_MCR_PS_Wr_Mask (ULONG)(0xffff7fff)

#define MAC_MCR_PS_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_PS_Wr_Mask) | ((data & MAC_MCR_PS_Mask)<<15));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_PS_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 15) & MAC_MCR_PS_Mask);\
} while(0)

/*#define MAC_MCR_FES_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_FES_Mask (ULONG)(0x1)

/*#define MAC_MCR_FES_Wr_Mask (ULONG)(~((~(~0 << (1))) << (14)))*/

#define MAC_MCR_FES_Wr_Mask (ULONG)(0xffffbfff)

#define MAC_MCR_FES_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_FES_Wr_Mask) | ((data & MAC_MCR_FES_Mask)<<14));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_FES_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 14) & MAC_MCR_FES_Mask);\
} while(0)

/*#define MAC_MCR_DM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_DM_Mask (ULONG)(0x1)

/*#define MAC_MCR_DM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (13)))*/

#define MAC_MCR_DM_Wr_Mask (ULONG)(0xffffdfff)

#define MAC_MCR_DM_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_DM_Wr_Mask) | ((data & MAC_MCR_DM_Mask)<<13));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_DM_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 13) & MAC_MCR_DM_Mask);\
} while(0)

/*#define MAC_MCR_LM_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_LM_Mask (ULONG)(0x1)

/*#define MAC_MCR_LM_Wr_Mask (ULONG)(~((~(~0 << (1))) << (12)))*/

#define MAC_MCR_LM_Wr_Mask (ULONG)(0xffffefff)

#define MAC_MCR_LM_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_LM_Wr_Mask) | ((data & MAC_MCR_LM_Mask)<<12));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_LM_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 12) & MAC_MCR_LM_Mask);\
} while(0)

/*#define MAC_MCR_ECRSFD_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_ECRSFD_Mask (ULONG)(0x1)

/*#define MAC_MCR_ECRSFD_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MAC_MCR_ECRSFD_Wr_Mask (ULONG)(0xfffff7ff)

#define MAC_MCR_ECRSFD_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_ECRSFD_Wr_Mask) | ((data & MAC_MCR_ECRSFD_Mask)<<11));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_ECRSFD_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 11) & MAC_MCR_ECRSFD_Mask);\
} while(0)

/*#define MAC_MCR_DRO_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_DRO_Mask (ULONG)(0x1)

/*#define MAC_MCR_DRO_Wr_Mask (ULONG)(~((~(~0 << (1))) << (10)))*/

#define MAC_MCR_DRO_Wr_Mask (ULONG)(0xfffffbff)

#define MAC_MCR_DRO_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_DRO_Wr_Mask) | ((data & MAC_MCR_DRO_Mask)<<10));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_DRO_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 10) & MAC_MCR_DRO_Mask);\
} while(0)

/*#define MAC_MCR_DCRS_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_DCRS_Mask (ULONG)(0x1)

/*#define MAC_MCR_DCRS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/

#define MAC_MCR_DCRS_Wr_Mask (ULONG)(0xfffffdff)

#define MAC_MCR_DCRS_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_DCRS_Wr_Mask) | ((data & MAC_MCR_DCRS_Mask)<<9));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_DCRS_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 9) & MAC_MCR_DCRS_Mask);\
} while(0)

/*#define MAC_MCR_DR_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_DR_Mask (ULONG)(0x1)

/*#define MAC_MCR_DR_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/

#define MAC_MCR_DR_Wr_Mask (ULONG)(0xfffffeff)

#define MAC_MCR_DR_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_DR_Wr_Mask) | ((data & MAC_MCR_DR_Mask)<<8));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_DR_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 8) & MAC_MCR_DR_Mask);\
} while(0)

/*#define MAC_MCR_BL_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_MCR_BL_Mask (ULONG)(0x3)

/*#define MAC_MCR_BL_Wr_Mask (ULONG)(~((~(~0 << (2))) << (5)))*/

#define MAC_MCR_BL_Wr_Mask (ULONG)(0xffffff9f)

#define MAC_MCR_BL_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_BL_Wr_Mask) | ((data & MAC_MCR_BL_Mask)<<5));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_BL_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 5) & MAC_MCR_BL_Mask);\
} while(0)

/*#define MAC_MCR_DEFC_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_DEFC_Mask (ULONG)(0x1)

/*#define MAC_MCR_DEFC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_MCR_DEFC_Wr_Mask (ULONG)(0xffffffef)

#define MAC_MCR_DEFC_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_DEFC_Wr_Mask) | ((data & MAC_MCR_DEFC_Mask)<<4));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_DEFC_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 4) & MAC_MCR_DEFC_Mask);\
} while(0)

/*#define MAC_MCR_PRELEN_Mask (ULONG)(~(~0<<(2)))*/

#define MAC_MCR_PRELEN_Mask (ULONG)(0x3)

/*#define MAC_MCR_PRELEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MAC_MCR_PRELEN_Wr_Mask (ULONG)(0xfffffff3)

#define MAC_MCR_PRELEN_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_PRELEN_Wr_Mask) | ((data & MAC_MCR_PRELEN_Mask)<<2));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_PRELEN_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 2) & MAC_MCR_PRELEN_Mask);\
} while(0)

/*#define MAC_MCR_TE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_TE_Mask (ULONG)(0x1)

/*#define MAC_MCR_TE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_MCR_TE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_MCR_TE_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_TE_Wr_Mask) | ((data & MAC_MCR_TE_Mask)<<1));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_TE_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 1) & MAC_MCR_TE_Mask);\
} while(0)

/*#define MAC_MCR_RE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MCR_RE_Mask (ULONG)(0x1)

/*#define MAC_MCR_RE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_MCR_RE_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_MCR_RE_UdfWr(data) do {\
		ULONG v;\
		MAC_MCR_RgRd(v);\
		v = (v & (MAC_MCR_RES_Wr_Mask_7))|((( 0) & (MAC_MCR_Mask_7))<<7);\
		v = ((v & MAC_MCR_RE_Wr_Mask) | ((data & MAC_MCR_RE_Mask)<<0));\
		MAC_MCR_RgWr(v);\
} while(0)

#define MAC_MCR_RE_UdfRd(data) do {\
		MAC_MCR_RgRd(data);\
		data = ((data >> 0) & MAC_MCR_RE_Mask);\
} while(0)


#define MAC_MA32_127LR_RgOffAddr (BASE_ADDRESS + 0x404)

#define MAC_MA32_127LR_RgOffAddress(i) ((volatile ULONG *)(MAC_MA32_127LR_RgOffAddr + ((i-32)*8)))

#define MAC_MA32_127LR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_MA32_127LR_RgOffAddress(i));\
} while(0)

#define MAC_MA32_127LR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_MA32_127LR_RgOffAddress(i));\
} while(0)

#define MAC_MA32_127LR_ADDRLO_UdfWr(i,data) do {\
		MAC_MA32_127LR_RgWr(i,data);\
} while(0)

#define MAC_MA32_127LR_ADDRLO_UdfRd(i,data) do {\
		MAC_MA32_127LR_RgRd(i,data);\
} while(0)


#define MAC_MA32_127HR_RgOffAddr (BASE_ADDRESS + 0x400)

#define MAC_MA32_127HR_RgOffAddress(i) ((volatile ULONG *)(MAC_MA32_127HR_RgOffAddr + ((i-32)*8)))

#define MAC_MA32_127HR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_MA32_127HR_RgOffAddress(i));\
} while(0)

#define MAC_MA32_127HR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_MA32_127HR_RgOffAddress(i));\
} while(0)

/*#define  MAC_MA32_127HR_Mask_19 (ULONG)(~(~0<<(12)))*/

#define  MAC_MA32_127HR_Mask_19 (ULONG)(0xfff)

/*#define MAC_MA32_127HR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(12)))<<(19)))*/

#define MAC_MA32_127HR_RES_Wr_Mask_19 (ULONG)(0x8007ffff)

/*#define MAC_MA32_127HR_AE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA32_127HR_AE_Mask (ULONG)(0x1)

/*#define MAC_MA32_127HR_AE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_MA32_127HR_AE_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MA32_127HR_AE_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA32_127HR_RgRd(i,v);\
		v = (v & (MAC_MA32_127HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA32_127HR_Mask_19))<<19);\
		v = ((v & MAC_MA32_127HR_AE_Wr_Mask) | ((data & MAC_MA32_127HR_AE_Mask)<<31));\
		MAC_MA32_127HR_RgWr(i,v);\
} while(0)

#define MAC_MA32_127HR_AE_UdfRd(i,data) do {\
		MAC_MA32_127HR_RgRd(i,data);\
		data = ((data >> 31) & MAC_MA32_127HR_AE_Mask);\
} while(0)

/*#define MAC_MA32_127HR_DCS_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MA32_127HR_DCS_Mask (ULONG)(0x7)

/*#define MAC_MA32_127HR_DCS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MAC_MA32_127HR_DCS_Wr_Mask (ULONG)(0xfff8ffff)

#define MAC_MA32_127HR_DCS_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA32_127HR_RgRd(i,v);\
		v = (v & (MAC_MA32_127HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA32_127HR_Mask_19))<<19);\
		v = ((v & MAC_MA32_127HR_DCS_Wr_Mask) | ((data & MAC_MA32_127HR_DCS_Mask)<<16));\
		MAC_MA32_127HR_RgWr(i,v);\
} while(0)

#define MAC_MA32_127HR_DCS_UdfRd(i,data) do {\
		MAC_MA32_127HR_RgRd(i,data);\
		data = ((data >> 16) & MAC_MA32_127HR_DCS_Mask);\
} while(0)

/*#define MAC_MA32_127HR_ADDRHI_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_MA32_127HR_ADDRHI_Mask (ULONG)(0xffff)

/*#define MAC_MA32_127HR_ADDRHI_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_MA32_127HR_ADDRHI_Wr_Mask (ULONG)(0xffff0000)

#define MAC_MA32_127HR_ADDRHI_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA32_127HR_RgRd(i,v);\
		v = (v & (MAC_MA32_127HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA32_127HR_Mask_19))<<19);\
		v = ((v & MAC_MA32_127HR_ADDRHI_Wr_Mask) | ((data & MAC_MA32_127HR_ADDRHI_Mask)<<0));\
		MAC_MA32_127HR_RgWr(i,v);\
} while(0)

#define MAC_MA32_127HR_ADDRHI_UdfRd(i,data) do {\
		MAC_MA32_127HR_RgRd(i,data);\
		data = ((data >> 0) & MAC_MA32_127HR_ADDRHI_Mask);\
} while(0)


#define MAC_MA1_31LR_RgOffAddr (BASE_ADDRESS + 0x30c)

#define MAC_MA1_31LR_RgOffAddress(i) ((volatile ULONG *)(MAC_MA1_31LR_RgOffAddr + ((i-1)*8)))

#define MAC_MA1_31LR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_MA1_31LR_RgOffAddress(i));\
} while(0)

#define MAC_MA1_31LR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_MA1_31LR_RgOffAddress(i));\
} while(0)

#define MAC_MA1_31LR_ADDRLO_UdfWr(i,data) do {\
		MAC_MA1_31LR_RgWr(i,data);\
} while(0)

#define MAC_MA1_31LR_ADDRLO_UdfRd(i,data) do {\
		MAC_MA1_31LR_RgRd(i,data);\
} while(0)


#define MAC_MA1_31HR_RgOffAddr (BASE_ADDRESS + 0x308)

#define MAC_MA1_31HR_RgOffAddress(i) ((volatile ULONG *)(MAC_MA1_31HR_RgOffAddr + ((i-1)*8)))

#define MAC_MA1_31HR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_MA1_31HR_RgOffAddress(i));\
} while(0)

#define MAC_MA1_31HR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_MA1_31HR_RgOffAddress(i));\
} while(0)

/*#define  MAC_MA1_31HR_Mask_19 (ULONG)(~(~0<<(5)))*/

#define  MAC_MA1_31HR_Mask_19 (ULONG)(0x1f)

/*#define MAC_MA1_31HR_RES_Wr_Mask_19 (ULONG)(~((~(~0<<(5)))<<(19)))*/

#define MAC_MA1_31HR_RES_Wr_Mask_19 (ULONG)(0xff07ffff)

/*#define MAC_MA1_31HR_AE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA1_31HR_AE_Mask (ULONG)(0x1)

/*#define MAC_MA1_31HR_AE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_MA1_31HR_AE_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_MA1_31HR_AE_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA1_31HR_RgRd(i,v);\
		v = (v & (MAC_MA1_31HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1_31HR_Mask_19))<<19);\
		v = ((v & MAC_MA1_31HR_AE_Wr_Mask) | ((data & MAC_MA1_31HR_AE_Mask)<<31));\
		MAC_MA1_31HR_RgWr(i,v);\
} while(0)

#define MAC_MA1_31HR_AE_UdfRd(i,data) do {\
		MAC_MA1_31HR_RgRd(i,data);\
		data = ((data >> 31) & MAC_MA1_31HR_AE_Mask);\
} while(0)

/*#define MAC_MA1_31HR_SA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_MA1_31HR_SA_Mask (ULONG)(0x1)

/*#define MAC_MA1_31HR_SA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (30)))*/

#define MAC_MA1_31HR_SA_Wr_Mask (ULONG)(0xbfffffff)

#define MAC_MA1_31HR_SA_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA1_31HR_RgRd(i,v);\
		v = (v & (MAC_MA1_31HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1_31HR_Mask_19))<<19);\
		v = ((v & MAC_MA1_31HR_SA_Wr_Mask) | ((data & MAC_MA1_31HR_SA_Mask)<<30));\
		MAC_MA1_31HR_RgWr(i,v);\
} while(0)

#define MAC_MA1_31HR_SA_UdfRd(i,data) do {\
		MAC_MA1_31HR_RgRd(i,data);\
		data = ((data >> 30) & MAC_MA1_31HR_SA_Mask);\
} while(0)

/*#define MAC_MA1_31HR_MBC_Mask (ULONG)(~(~0<<(6)))*/

#define MAC_MA1_31HR_MBC_Mask (ULONG)(0x3f)

/*#define MAC_MA1_31HR_MBC_Wr_Mask (ULONG)(~((~(~0 << (6))) << (24)))*/

#define MAC_MA1_31HR_MBC_Wr_Mask (ULONG)(0xc0ffffff)

#define MAC_MA1_31HR_MBC_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA1_31HR_RgRd(i,v);\
		v = (v & (MAC_MA1_31HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1_31HR_Mask_19))<<19);\
		v = ((v & MAC_MA1_31HR_MBC_Wr_Mask) | ((data & MAC_MA1_31HR_MBC_Mask)<<24));\
		MAC_MA1_31HR_RgWr(i,v);\
} while(0)

#define MAC_MA1_31HR_MBC_UdfRd(i,data) do {\
		MAC_MA1_31HR_RgRd(i,data);\
		data = ((data >> 24) & MAC_MA1_31HR_MBC_Mask);\
} while(0)

/*#define MAC_MA1_31HR_DCS_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_MA1_31HR_DCS_Mask (ULONG)(0x7)

/*#define MAC_MA1_31HR_DCS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (16)))*/

#define MAC_MA1_31HR_DCS_Wr_Mask (ULONG)(0xfff8ffff)

#define MAC_MA1_31HR_DCS_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA1_31HR_RgRd(i,v);\
		v = (v & (MAC_MA1_31HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1_31HR_Mask_19))<<19);\
		v = ((v & MAC_MA1_31HR_DCS_Wr_Mask) | ((data & MAC_MA1_31HR_DCS_Mask)<<16));\
		MAC_MA1_31HR_RgWr(i,v);\
} while(0)

#define MAC_MA1_31HR_DCS_UdfRd(i,data) do {\
		MAC_MA1_31HR_RgRd(i,data);\
		data = ((data >> 16) & MAC_MA1_31HR_DCS_Mask);\
} while(0)

/*#define MAC_MA1_31HR_ADDRHI_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_MA1_31HR_ADDRHI_Mask (ULONG)(0xffff)

/*#define MAC_MA1_31HR_ADDRHI_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_MA1_31HR_ADDRHI_Wr_Mask (ULONG)(0xffff0000)

#define MAC_MA1_31HR_ADDRHI_UdfWr(i,data) do {\
		ULONG v;\
		MAC_MA1_31HR_RgRd(i,v);\
		v = (v & (MAC_MA1_31HR_RES_Wr_Mask_19))|((( 0) & (MAC_MA1_31HR_Mask_19))<<19);\
		v = ((v & MAC_MA1_31HR_ADDRHI_Wr_Mask) | ((data & MAC_MA1_31HR_ADDRHI_Mask)<<0));\
		MAC_MA1_31HR_RgWr(i,v);\
} while(0)

#define MAC_MA1_31HR_ADDRHI_UdfRd(i,data) do {\
		MAC_MA1_31HR_RgRd(i,data);\
		data = ((data >> 0) & MAC_MA1_31HR_ADDRHI_Mask);\
} while(0)


#define MAC_L3A3R_RgOffAddr (BASE_ADDRESS + 0x91c)

#define MAC_L3A3R_RgOffAddress(i) ((volatile ULONG *)(MAC_L3A3R_RgOffAddr + ((i-0)*48)))

#define MAC_L3A3R_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_L3A3R_RgOffAddress(i));\
} while(0)

#define MAC_L3A3R_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_L3A3R_RgOffAddress(i));\
} while(0)

#define MAC_L3A3R_L3A30_UdfWr(i,data) do {\
		MAC_L3A3R_RgWr(i,data);\
} while(0)

#define MAC_L3A3R_L3A30_UdfRd(i,data) do {\
		MAC_L3A3R_RgRd(i,data);\
} while(0)


#define MAC_L3A2R_RgOffAddr (BASE_ADDRESS + 0x918)

#define MAC_L3A2R_RgOffAddress(i) ((volatile ULONG *)(MAC_L3A2R_RgOffAddr + ((i-0)*48)))

#define MAC_L3A2R_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_L3A2R_RgOffAddress(i));\
} while(0)

#define MAC_L3A2R_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_L3A2R_RgOffAddress(i));\
} while(0)

#define MAC_L3A2R_L3A20_UdfWr(i,data) do {\
		MAC_L3A2R_RgWr(i,data);\
} while(0)

#define MAC_L3A2R_L3A20_UdfRd(i,data) do {\
		MAC_L3A2R_RgRd(i,data);\
} while(0)


#define MAC_L3A1R_RgOffAddr (BASE_ADDRESS + 0x914)

#define MAC_L3A1R_RgOffAddress(i) ((volatile ULONG *)(MAC_L3A1R_RgOffAddr + ((i-0)*48)))

#define MAC_L3A1R_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_L3A1R_RgOffAddress(i));\
} while(0)

#define MAC_L3A1R_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_L3A1R_RgOffAddress(i));\
} while(0)

#define MAC_L3A1R_L3A10_UdfWr(i,data) do {\
		MAC_L3A1R_RgWr(i,data);\
} while(0)

#define MAC_L3A1R_L3A10_UdfRd(i,data) do {\
		MAC_L3A1R_RgRd(i,data);\
} while(0)


#define MAC_L3A0R_RgOffAddr (BASE_ADDRESS + 0x910)

#define MAC_L3A0R_RgOffAddress(i) ((volatile ULONG *)(MAC_L3A0R_RgOffAddr + ((i-0)*48)))

#define MAC_L3A0R_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_L3A0R_RgOffAddress(i));\
} while(0)

#define MAC_L3A0R_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_L3A0R_RgOffAddress(i));\
} while(0)

#define MAC_L3A0R_L3A00_UdfWr(i,data) do {\
		MAC_L3A0R_RgWr(i,data);\
} while(0)

#define MAC_L3A0R_L3A00_UdfRd(i,data) do {\
		MAC_L3A0R_RgRd(i,data);\
} while(0)


#define MAC_L4AR_RgOffAddr (BASE_ADDRESS + 0x904)

#define MAC_L4AR_RgOffAddress(i) ((volatile ULONG *)(MAC_L4AR_RgOffAddr + ((i-0)*48)))

#define MAC_L4AR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_L4AR_RgOffAddress(i));\
} while(0)

#define MAC_L4AR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_L4AR_RgOffAddress(i));\
} while(0)

/*#define MAC_L4AR_L4DP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR_L4DP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR_L4DP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_L4AR_L4DP0_Wr_Mask (ULONG)(0xffff)

#define MAC_L4AR_L4DP0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L4AR_RgRd(i,v);\
		v = ((v & MAC_L4AR_L4DP0_Wr_Mask) | ((data & MAC_L4AR_L4DP0_Mask)<<16));\
		MAC_L4AR_RgWr(i,v);\
} while(0)

#define MAC_L4AR_L4DP0_UdfRd(i,data) do {\
		MAC_L4AR_RgRd(i,data);\
		data = ((data >> 16) & MAC_L4AR_L4DP0_Mask);\
} while(0)

/*#define MAC_L4AR_L4SP0_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_L4AR_L4SP0_Mask (ULONG)(0xffff)

/*#define MAC_L4AR_L4SP0_Wr_Mask (ULONG)(~((~(~0 << (16))) << (0)))*/

#define MAC_L4AR_L4SP0_Wr_Mask (ULONG)(0xffff0000)

#define MAC_L4AR_L4SP0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L4AR_RgRd(i,v);\
		v = ((v & MAC_L4AR_L4SP0_Wr_Mask) | ((data & MAC_L4AR_L4SP0_Mask)<<0));\
		MAC_L4AR_RgWr(i,v);\
} while(0)

#define MAC_L4AR_L4SP0_UdfRd(i,data) do {\
		MAC_L4AR_RgRd(i,data);\
		data = ((data >> 0) & MAC_L4AR_L4SP0_Mask);\
} while(0)


#define MAC_L3L4CR_RgOffAddr (BASE_ADDRESS + 0x900)

#define MAC_L3L4CR_RgOffAddress(i) ((volatile ULONG *)(MAC_L3L4CR_RgOffAddr + ((i-0)*48)))

#define MAC_L3L4CR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_L3L4CR_RgOffAddress(i));\
} while(0)

#define MAC_L3L4CR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_L3L4CR_RgOffAddress(i));\
} while(0)

/*#define  MAC_L3L4CR_Mask_22 (ULONG)(~(~0<<(10)))*/

#define  MAC_L3L4CR_Mask_22 (ULONG)(0x3ff)

/*#define MAC_L3L4CR_RES_Wr_Mask_22 (ULONG)(~((~(~0<<(10)))<<(22)))*/

#define MAC_L3L4CR_RES_Wr_Mask_22 (ULONG)(0x3fffff)

/*#define  MAC_L3L4CR_Mask_17 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR_Mask_17 (ULONG)(0x1)

/*#define MAC_L3L4CR_RES_Wr_Mask_17 (ULONG)(~((~(~0<<(1)))<<(17)))*/

#define MAC_L3L4CR_RES_Wr_Mask_17 (ULONG)(0xfffdffff)

/*#define  MAC_L3L4CR_Mask_1 (ULONG)(~(~0<<(1)))*/

#define  MAC_L3L4CR_Mask_1 (ULONG)(0x1)

/*#define MAC_L3L4CR_RES_Wr_Mask_1 (ULONG)(~((~(~0<<(1)))<<(1)))*/

#define MAC_L3L4CR_RES_Wr_Mask_1 (ULONG)(0xfffffffd)

/*#define MAC_L3L4CR_L4DPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L4DPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L4DPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (21)))*/

#define MAC_L3L4CR_L4DPIM0_Wr_Mask (ULONG)(0xffdfffff)

#define MAC_L3L4CR_L4DPIM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L4DPIM0_Wr_Mask) | ((data & MAC_L3L4CR_L4DPIM0_Mask)<<21));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L4DPIM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 21) & MAC_L3L4CR_L4DPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L4DPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L4DPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L4DPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (20)))*/

#define MAC_L3L4CR_L4DPM0_Wr_Mask (ULONG)(0xffefffff)

#define MAC_L3L4CR_L4DPM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L4DPM0_Wr_Mask) | ((data & MAC_L3L4CR_L4DPM0_Mask)<<20));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L4DPM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 20) & MAC_L3L4CR_L4DPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L4SPIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L4SPIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L4SPIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (19)))*/

#define MAC_L3L4CR_L4SPIM0_Wr_Mask (ULONG)(0xfff7ffff)

#define MAC_L3L4CR_L4SPIM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L4SPIM0_Wr_Mask) | ((data & MAC_L3L4CR_L4SPIM0_Mask)<<19));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L4SPIM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 19) & MAC_L3L4CR_L4SPIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L4SPM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L4SPM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L4SPM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (18)))*/

#define MAC_L3L4CR_L4SPM0_Wr_Mask (ULONG)(0xfffbffff)

#define MAC_L3L4CR_L4SPM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L4SPM0_Wr_Mask) | ((data & MAC_L3L4CR_L4SPM0_Mask)<<18));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L4SPM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 18) & MAC_L3L4CR_L4SPM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L4PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L4PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L4PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (16)))*/

#define MAC_L3L4CR_L4PEN0_Wr_Mask (ULONG)(0xfffeffff)

#define MAC_L3L4CR_L4PEN0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L4PEN0_Wr_Mask) | ((data & MAC_L3L4CR_L4PEN0_Mask)<<16));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L4PEN0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 16) & MAC_L3L4CR_L4PEN0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3HDBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR_L3HDBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR_L3HDBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (11)))*/

#define MAC_L3L4CR_L3HDBM0_Wr_Mask (ULONG)(0xffff07ff)

#define MAC_L3L4CR_L3HDBM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3HDBM0_Wr_Mask) | ((data & MAC_L3L4CR_L3HDBM0_Mask)<<11));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3HDBM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 11) & MAC_L3L4CR_L3HDBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3HSBM0_Mask (ULONG)(~(~0<<(5)))*/

#define MAC_L3L4CR_L3HSBM0_Mask (ULONG)(0x1f)

/*#define MAC_L3L4CR_L3HSBM0_Wr_Mask (ULONG)(~((~(~0 << (5))) << (6)))*/

#define MAC_L3L4CR_L3HSBM0_Wr_Mask (ULONG)(0xfffff83f)

#define MAC_L3L4CR_L3HSBM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3HSBM0_Wr_Mask) | ((data & MAC_L3L4CR_L3HSBM0_Mask)<<6));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3HSBM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 6) & MAC_L3L4CR_L3HSBM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3DAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L3DAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L3DAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MAC_L3L4CR_L3DAIM0_Wr_Mask (ULONG)(0xffffffdf)

#define MAC_L3L4CR_L3DAIM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3DAIM0_Wr_Mask) | ((data & MAC_L3L4CR_L3DAIM0_Mask)<<5));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3DAIM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 5) & MAC_L3L4CR_L3DAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3DAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L3DAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L3DAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MAC_L3L4CR_L3DAM0_Wr_Mask (ULONG)(0xffffffef)

#define MAC_L3L4CR_L3DAM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3DAM0_Wr_Mask) | ((data & MAC_L3L4CR_L3DAM0_Mask)<<4));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3DAM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 4) & MAC_L3L4CR_L3DAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3SAIM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L3SAIM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L3SAIM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MAC_L3L4CR_L3SAIM0_Wr_Mask (ULONG)(0xfffffff7)

#define MAC_L3L4CR_L3SAIM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3SAIM0_Wr_Mask) | ((data & MAC_L3L4CR_L3SAIM0_Mask)<<3));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3SAIM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 3) & MAC_L3L4CR_L3SAIM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3SAM0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L3SAM0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L3SAM0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MAC_L3L4CR_L3SAM0_Wr_Mask (ULONG)(0xfffffffb)

#define MAC_L3L4CR_L3SAM0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3SAM0_Wr_Mask) | ((data & MAC_L3L4CR_L3SAM0_Mask)<<2));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3SAM0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 2) & MAC_L3L4CR_L3SAM0_Mask);\
} while(0)

/*#define MAC_L3L4CR_L3PEN0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_L3L4CR_L3PEN0_Mask (ULONG)(0x1)

/*#define MAC_L3L4CR_L3PEN0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_L3L4CR_L3PEN0_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_L3L4CR_L3PEN0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_L3L4CR_RgRd(i,v);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_22))|((( 0) & (MAC_L3L4CR_Mask_22))<<22);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_17))|((( 0) & (MAC_L3L4CR_Mask_17))<<17);\
		v = (v & (MAC_L3L4CR_RES_Wr_Mask_1))|((( 0) & (MAC_L3L4CR_Mask_1))<<1);\
		v = ((v & MAC_L3L4CR_L3PEN0_Wr_Mask) | ((data & MAC_L3L4CR_L3PEN0_Mask)<<0));\
		MAC_L3L4CR_RgWr(i,v);\
} while(0)

#define MAC_L3L4CR_L3PEN0_UdfRd(i,data) do {\
		MAC_L3L4CR_RgRd(i,data);\
		data = ((data >> 0) & MAC_L3L4CR_L3PEN0_Mask);\
} while(0)


#define MAC_PPS_WIDTH_RgOffAddr (BASE_ADDRESS + 0xb8c)

#define MAC_PPS_WIDTH_RgOffAddress(i) ((volatile ULONG *)(MAC_PPS_WIDTH_RgOffAddr + ((i-0)*16)))

#define MAC_PPS_WIDTH_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_PPS_WIDTH_RgOffAddress(i));\
} while(0)

#define MAC_PPS_WIDTH_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_PPS_WIDTH_RgOffAddress(i));\
} while(0)

#define MAC_PPS_WIDTH_PPSWIDTH0_UdfWr(i,data) do {\
		MAC_PPS_WIDTH_RgWr(i,data);\
} while(0)

#define MAC_PPS_WIDTH_PPSWIDTH0_UdfRd(i,data) do {\
		MAC_PPS_WIDTH_RgRd(i,data);\
} while(0)


#define MAC_PPS_INTVAL_RgOffAddr (BASE_ADDRESS + 0xb88)

#define MAC_PPS_INTVAL_RgOffAddress(i) ((volatile ULONG *)(MAC_PPS_INTVAL_RgOffAddr + ((i-0)*16)))

#define MAC_PPS_INTVAL_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_PPS_INTVAL_RgOffAddress(i));\
} while(0)

#define MAC_PPS_INTVAL_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_PPS_INTVAL_RgOffAddress(i));\
} while(0)

#define MAC_PPS_INTVAL_PPSINT0_UdfWr(i,data) do {\
		MAC_PPS_INTVAL_RgWr(i,data);\
} while(0)

#define MAC_PPS_INTVAL_PPSINT0_UdfRd(i,data) do {\
		MAC_PPS_INTVAL_RgRd(i,data);\
} while(0)


#define MAC_PPS_TTNS_RgOffAddr (BASE_ADDRESS + 0xb84)

#define MAC_PPS_TTNS_RgOffAddress(i) ((volatile ULONG *)(MAC_PPS_TTNS_RgOffAddr + ((i-0)*16)))

#define MAC_PPS_TTNS_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_PPS_TTNS_RgOffAddress(i));\
} while(0)

#define MAC_PPS_TTNS_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_PPS_TTNS_RgOffAddress(i));\
} while(0)

/*#define MAC_PPS_TTNS_TRGTBUSY0_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_PPS_TTNS_TRGTBUSY0_Mask (ULONG)(0x1)

/*#define MAC_PPS_TTNS_TRGTBUSY0_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/

#define MAC_PPS_TTNS_TRGTBUSY0_Wr_Mask (ULONG)(0x7fffffff)

#define MAC_PPS_TTNS_TRGTBUSY0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_PPS_TTNS_RgRd(i,v);\
		v = ((v & MAC_PPS_TTNS_TRGTBUSY0_Wr_Mask) | ((data & MAC_PPS_TTNS_TRGTBUSY0_Mask)<<31));\
		MAC_PPS_TTNS_RgWr(i,v);\
} while(0)

#define MAC_PPS_TTNS_TRGTBUSY0_UdfRd(i,data) do {\
		MAC_PPS_TTNS_RgRd(i,data);\
		data = ((data >> 31) & MAC_PPS_TTNS_TRGTBUSY0_Mask);\
} while(0)

/*#define MAC_PPS_TTNS_TTSL0_Mask (ULONG)(~(~0<<(31)))*/

#define MAC_PPS_TTNS_TTSL0_Mask (ULONG)(0x7fffffff)

/*#define MAC_PPS_TTNS_TTSL0_Wr_Mask (ULONG)(~((~(~0 << (31))) << (0)))*/

#define MAC_PPS_TTNS_TTSL0_Wr_Mask (ULONG)(0x80000000)

#define MAC_PPS_TTNS_TTSL0_UdfWr(i,data) do {\
		ULONG v;\
		MAC_PPS_TTNS_RgRd(i,v);\
		v = ((v & MAC_PPS_TTNS_TTSL0_Wr_Mask) | ((data & MAC_PPS_TTNS_TTSL0_Mask)<<0));\
		MAC_PPS_TTNS_RgWr(i,v);\
} while(0)

#define MAC_PPS_TTNS_TTSL0_UdfRd(i,data) do {\
		MAC_PPS_TTNS_RgRd(i,data);\
		data = ((data >> 0) & MAC_PPS_TTNS_TTSL0_Mask);\
} while(0)


#define MAC_PPS_TTS_RgOffAddr (BASE_ADDRESS + 0xb80)

#define MAC_PPS_TTS_RgOffAddress(i) ((volatile ULONG *)(MAC_PPS_TTS_RgOffAddr + ((i-0)*16)))

#define MAC_PPS_TTS_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_PPS_TTS_RgOffAddress(i));\
} while(0)

#define MAC_PPS_TTS_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_PPS_TTS_RgOffAddress(i));\
} while(0)

#define MAC_PPS_TTS_TSTRH0_UdfWr(i,data) do {\
		MAC_PPS_TTS_RgWr(i,data);\
} while(0)

#define MAC_PPS_TTS_TSTRH0_UdfRd(i,data) do {\
		MAC_PPS_TTS_RgRd(i,data);\
} while(0)


#define MTL_QRCR_RgOffAddr (BASE_ADDRESS + 0xd3c)

#define MTL_QRCR_RgOffAddress(i) ((volatile ULONG *)(MTL_QRCR_RgOffAddr + ((i-0)*64)))

#define MTL_QRCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QRCR_RgOffAddress(i));\
} while(0)

#define MTL_QRCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QRCR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QRCR_Mask_4 (ULONG)(~(~0<<(28)))*/

#define  MTL_QRCR_Mask_4 (ULONG)(0xfffffff)

/*#define MTL_QRCR_RES_Wr_Mask_4 (ULONG)(~((~(~0<<(28)))<<(4)))*/

#define MTL_QRCR_RES_Wr_Mask_4 (ULONG)(0xf)

/*#define MTL_QRCR_RXQ_PKT_ARBIT_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRCR_RXQ_PKT_ARBIT_Mask (ULONG)(0x1)

/*#define MTL_QRCR_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QRCR_RXQ_PKT_ARBIT_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QRCR_RXQ_PKT_ARBIT_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QRCR_RgRd(i,v);\
		v = (v & (MTL_QRCR_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR_Mask_4))<<4);\
		v = ((v & MTL_QRCR_RXQ_PKT_ARBIT_Wr_Mask) | ((data & MTL_QRCR_RXQ_PKT_ARBIT_Mask)<<3));\
		MTL_QRCR_RgWr(i,v);\
} while(0)

#define MTL_QRCR_RXQ_PKT_ARBIT_UdfRd(i,data) do {\
		MTL_QRCR_RgRd(i,data);\
		data = ((data >> 3) & MTL_QRCR_RXQ_PKT_ARBIT_Mask);\
} while(0)

/*#define MTL_QRCR_RXQ_WEGT_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QRCR_RXQ_WEGT_Mask (ULONG)(0x7)

/*#define MTL_QRCR_RXQ_WEGT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (0)))*/

#define MTL_QRCR_RXQ_WEGT_Wr_Mask (ULONG)(0xfffffff8)

#define MTL_QRCR_RXQ_WEGT_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QRCR_RgRd(i,v);\
		v = (v & (MTL_QRCR_RES_Wr_Mask_4))|((( 0) & (MTL_QRCR_Mask_4))<<4);\
		v = ((v & MTL_QRCR_RXQ_WEGT_Wr_Mask) | ((data & MTL_QRCR_RXQ_WEGT_Mask)<<0));\
		MTL_QRCR_RgWr(i,v);\
} while(0)

#define MTL_QRCR_RXQ_WEGT_UdfRd(i,data) do {\
		MTL_QRCR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QRCR_RXQ_WEGT_Mask);\
} while(0)


#define MTL_QRDR_RgOffAddr (BASE_ADDRESS + 0xd38)

#define MTL_QRDR_RgOffAddress(i) ((volatile ULONG *)(MTL_QRDR_RgOffAddr + ((i-3)*64)))

#define MTL_QRDR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QRDR_RgOffAddress(i));\
} while(0)

/*#define MTL_QRDR_PRXQ_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QRDR_PRXQ_Mask (ULONG)(0x3fff)

#define MTL_QRDR_PRXQ_UdfRd(i,data) do {\
		MTL_QRDR_RgRd(i,data);\
		data = ((data >> 16) & MTL_QRDR_PRXQ_Mask);\
} while(0)

/*#define MTL_QRDR_RXQSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR_RXQSTS_Mask (ULONG)(0x3)

#define MTL_QRDR_RXQSTS_UdfRd(i,data) do {\
		MTL_QRDR_RgRd(i,data);\
		data = ((data >> 4) & MTL_QRDR_RXQSTS_Mask);\
} while(0)

/*#define MTL_QRDR_RRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QRDR_RRCSTS_Mask (ULONG)(0x3)

#define MTL_QRDR_RRCSTS_UdfRd(i,data) do {\
		MTL_QRDR_RgRd(i,data);\
		data = ((data >> 1) & MTL_QRDR_RRCSTS_Mask);\
} while(0)

/*#define MTL_QRDR_RWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QRDR_RWCSTS_Mask (ULONG)(0x1)

#define MTL_QRDR_RWCSTS_UdfRd(i,data) do {\
		MTL_QRDR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QRDR_RWCSTS_Mask);\
} while(0)


#define MTL_QOCR_RgOffAddr (BASE_ADDRESS + 0xd34)

#define MTL_QOCR_RgOffAddress(i) ((volatile ULONG *)(MTL_QOCR_RgOffAddr + ((i-0)*64)))

#define MTL_QOCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QOCR_RgOffAddress(i));\
} while(0)

#define MTL_QOCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QOCR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QOCR_Mask_28 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR_Mask_28 (ULONG)(0xf)

/*#define MTL_QOCR_RES_Wr_Mask_28 (ULONG)(~((~(~0<<(4)))<<(28)))*/

#define MTL_QOCR_RES_Wr_Mask_28 (ULONG)(0xfffffff)

/*#define  MTL_QOCR_Mask_12 (ULONG)(~(~0<<(4)))*/

#define  MTL_QOCR_Mask_12 (ULONG)(0xf)

/*#define MTL_QOCR_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(4)))<<(12)))*/

#define MTL_QOCR_RES_Wr_Mask_12 (ULONG)(0xffff0fff)

/*#define MTL_QOCR_MISCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR_MISCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR_MISCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (27)))*/

#define MTL_QOCR_MISCNTOVF_Wr_Mask (ULONG)(0xf7ffffff)

#define MTL_QOCR_MISCNTOVF_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QOCR_RgRd(i,v);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR_Mask_28))<<28);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR_Mask_12))<<12);\
		v = ((v & MTL_QOCR_MISCNTOVF_Wr_Mask) | ((data & MTL_QOCR_MISCNTOVF_Mask)<<27));\
		MTL_QOCR_RgWr(i,v);\
} while(0)

#define MTL_QOCR_MISCNTOVF_UdfRd(i,data) do {\
		MTL_QOCR_RgRd(i,data);\
		data = ((data >> 27) & MTL_QOCR_MISCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR_MISPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR_MISPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR_MISPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (16)))*/

#define MTL_QOCR_MISPKTCNT_Wr_Mask (ULONG)(0xf800ffff)

#define MTL_QOCR_MISPKTCNT_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QOCR_RgRd(i,v);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR_Mask_28))<<28);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR_Mask_12))<<12);\
		v = ((v & MTL_QOCR_MISPKTCNT_Wr_Mask) | ((data & MTL_QOCR_MISPKTCNT_Mask)<<16));\
		MTL_QOCR_RgWr(i,v);\
} while(0)

#define MTL_QOCR_MISPKTCNT_UdfRd(i,data) do {\
		MTL_QOCR_RgRd(i,data);\
		data = ((data >> 16) & MTL_QOCR_MISPKTCNT_Mask);\
} while(0)

/*#define MTL_QOCR_OVFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QOCR_OVFCNTOVF_Mask (ULONG)(0x1)

/*#define MTL_QOCR_OVFCNTOVF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (11)))*/

#define MTL_QOCR_OVFCNTOVF_Wr_Mask (ULONG)(0xfffff7ff)

#define MTL_QOCR_OVFCNTOVF_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QOCR_RgRd(i,v);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR_Mask_28))<<28);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR_Mask_12))<<12);\
		v = ((v & MTL_QOCR_OVFCNTOVF_Wr_Mask) | ((data & MTL_QOCR_OVFCNTOVF_Mask)<<11));\
		MTL_QOCR_RgWr(i,v);\
} while(0)

#define MTL_QOCR_OVFCNTOVF_UdfRd(i,data) do {\
		MTL_QOCR_RgRd(i,data);\
		data = ((data >> 11) & MTL_QOCR_OVFCNTOVF_Mask);\
} while(0)

/*#define MTL_QOCR_OVFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QOCR_OVFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QOCR_OVFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QOCR_OVFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QOCR_OVFPKTCNT_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QOCR_RgRd(i,v);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_28))|((( 0) & (MTL_QOCR_Mask_28))<<28);\
		v = (v & (MTL_QOCR_RES_Wr_Mask_12))|((( 0) & (MTL_QOCR_Mask_12))<<12);\
		v = ((v & MTL_QOCR_OVFPKTCNT_Wr_Mask) | ((data & MTL_QOCR_OVFPKTCNT_Mask)<<0));\
		MTL_QOCR_RgWr(i,v);\
} while(0)

#define MTL_QOCR_OVFPKTCNT_UdfRd(i,data) do {\
		MTL_QOCR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QOCR_OVFPKTCNT_Mask);\
} while(0)


#define MTL_QROMR_RgOffAddr (BASE_ADDRESS + 0xd30)

#define MTL_QROMR_RgOffAddress(i) ((volatile ULONG *)(MTL_QROMR_RgOffAddr + ((i-0)*64)))

#define MTL_QROMR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QROMR_RgOffAddress(i));\
} while(0)

#define MTL_QROMR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QROMR_RgOffAddress(i));\
} while(0)



/*#define  MTL_QROMR_Mask_30 (ULONG)(~(~0<<(2)))*/

#define  MTL_QROMR_Mask_30 (ULONG)(0x3)

/*#define MTL_QROMR_RES_Wr_Mask_30 (ULONG)(~((~(~0<<(2)))<<(30)))*/

#define MTL_QROMR_RES_Wr_Mask_30 (ULONG)(0x3fffffff)

/*#define  MTL_QROMR_Mask_11 (ULONG)(~(~0<<(2)))*/

#define  MTL_QROMR_Mask_11 (ULONG)(0x3)

/*#define MTL_QROMR_RES_Wr_Mask_11 (ULONG)(~((~(~0<<(2)))<<(11)))*/

#define MTL_QROMR_RES_Wr_Mask_11 (ULONG)(0xffffe7ff)

/*#define  MTL_QROMR_Mask_2 (ULONG)(~(~0<<(1)))*/

#define  MTL_QROMR_Mask_2 (ULONG)(0x1)

/*#define MTL_QROMR_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(1)))<<(2)))*/

#define MTL_QROMR_RES_Wr_Mask_2 (ULONG)(0xfffffffb)

/*#define MTL_QROMR_RQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QROMR_RQS_Mask (ULONG)(0x3ff)

/*#define MTL_QROMR_RQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (20)))*/

#define MTL_QROMR_RQS_Wr_Mask (ULONG)(0xc00fffff)

#define MTL_QROMR_RQS_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_RQS_Wr_Mask) | ((data & MTL_QROMR_RQS_Mask)<<20));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_RQS_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 20) & MTL_QROMR_RQS_Mask);\
} while(0)


#define MTL_QROMR_RFD_Mask (ULONG)(0x3f)
#define MTL_QROMR_RFD_Wr_Mask (ULONG)(0xfff03fff)

#define MTL_QROMR_RFD_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_RFD_Wr_Mask) | ((data & MTL_QROMR_RFD_Mask)<<14));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_RFD_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 14) & MTL_QROMR_RFD_Mask);\
} while(0)


#define MTL_QROMR_RFA_Mask (ULONG)(0x3f)
#define MTL_QROMR_RFA_Wr_Mask (ULONG)(0xffffc0ff)

#define MTL_QROMR_RFA_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_RFA_Wr_Mask) | ((data & MTL_QROMR_RFA_Mask)<<8));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_RFA_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 8) & MTL_QROMR_RFA_Mask);\
} while(0)

/*#define MTL_QROMR_EHFC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QROMR_EHFC_Mask (ULONG)(0x1)

/*#define MTL_QROMR_EHFC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MTL_QROMR_EHFC_Wr_Mask (ULONG)(0xffffff7f)

#define MTL_QROMR_EHFC_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_EHFC_Wr_Mask) | ((data & MTL_QROMR_EHFC_Mask)<<7));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_EHFC_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 7) & MTL_QROMR_EHFC_Mask);\
} while(0)

/*#define MTL_QROMR_DIS_TCP_EF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QROMR_DIS_TCP_EF_Mask (ULONG)(0x1)

/*#define MTL_QROMR_DIS_TCP_EF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (6)))*/

#define MTL_QROMR_DIS_TCP_EF_Wr_Mask (ULONG)(0xffffffbf)

#define MTL_QROMR_DIS_TCP_EF_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_DIS_TCP_EF_Wr_Mask) | ((data & MTL_QROMR_DIS_TCP_EF_Mask)<<6));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_DIS_TCP_EF_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 6) & MTL_QROMR_DIS_TCP_EF_Mask);\
} while(0)

/*#define MTL_QROMR_RSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QROMR_RSF_Mask (ULONG)(0x1)

/*#define MTL_QROMR_RSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (5)))*/

#define MTL_QROMR_RSF_Wr_Mask (ULONG)(0xffffffdf)

#define MTL_QROMR_RSF_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_RSF_Wr_Mask) | ((data & MTL_QROMR_RSF_Mask)<<5));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_RSF_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 5) & MTL_QROMR_RSF_Mask);\
} while(0)

/*#define MTL_QROMR_FEP_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QROMR_FEP_Mask (ULONG)(0x1)

/*#define MTL_QROMR_FEP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/

#define MTL_QROMR_FEP_Wr_Mask (ULONG)(0xffffffef)

#define MTL_QROMR_FEP_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_FEP_Wr_Mask) | ((data & MTL_QROMR_FEP_Mask)<<4));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_FEP_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 4) & MTL_QROMR_FEP_Mask);\
} while(0)

/*#define MTL_QROMR_FUP_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QROMR_FUP_Mask (ULONG)(0x1)

/*#define MTL_QROMR_FUP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QROMR_FUP_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QROMR_FUP_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_FUP_Wr_Mask) | ((data & MTL_QROMR_FUP_Mask)<<3));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_FUP_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 3) & MTL_QROMR_FUP_Mask);\
} while(0)

/*#define MTL_QROMR_RTC_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QROMR_RTC_Mask (ULONG)(0x3)

/*#define MTL_QROMR_RTC_Wr_Mask (ULONG)(~((~(~0 << (2))) << (0)))*/

#define MTL_QROMR_RTC_Wr_Mask (ULONG)(0xfffffffc)

#define MTL_QROMR_RTC_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QROMR_RgRd(i,v);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_30))|((( 0) & (MTL_QROMR_Mask_30))<<30);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_11))|((( 0) & (MTL_QROMR_Mask_11))<<11);\
		v = (v & (MTL_QROMR_RES_Wr_Mask_2))|((( 0) & (MTL_QROMR_Mask_2))<<2);\
		v = ((v & MTL_QROMR_RTC_Wr_Mask) | ((data & MTL_QROMR_RTC_Mask)<<0));\
		MTL_QROMR_RgWr(i,v);\
} while(0)

#define MTL_QROMR_RTC_UdfRd(i,data) do {\
		MTL_QROMR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QROMR_RTC_Mask);\
} while(0)





#define MTL_QLCR_RgOffAddr (BASE_ADDRESS + 0xd24)

#define MTL_QLCR_RgOffAddress(i) ((volatile ULONG *)(MTL_QLCR_RgOffAddr + ((i-0)*64)))

#define MTL_QLCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QLCR_RgOffAddress(i));\
} while(0)

#define MTL_QLCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QLCR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QLCR_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QLCR_Mask_29 (ULONG)(0x7)

/*#define MTL_QLCR_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QLCR_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QLCR_LC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QLCR_LC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QLCR_LC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QLCR_LC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QLCR_LC_UdfWr(i,data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QLCR_RES_Wr_Mask_29))|((( 0) & (MTL_QLCR_Mask_29))<<29);\
		(v) = ((v & MTL_QLCR_LC_Wr_Mask) | ((data & MTL_QLCR_LC_Mask)<<0));\
		MTL_QLCR_RgWr(i,v);\
} while(0)

#define MTL_QLCR_LC_UdfRd(i,data) do {\
		MTL_QLCR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QLCR_LC_Mask);\
} while(0)


#define MTL_QHCR_RgOffAddr (BASE_ADDRESS + 0xd20)

#define MTL_QHCR_RgOffAddress(i) ((volatile ULONG *)(MTL_QHCR_RgOffAddr + ((i-0)*64)))

#define MTL_QHCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QHCR_RgOffAddress(i));\
} while(0)

#define MTL_QHCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QHCR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QHCR_Mask_29 (ULONG)(~(~0<<(3)))*/

#define  MTL_QHCR_Mask_29 (ULONG)(0x7)

/*#define MTL_QHCR_RES_Wr_Mask_29 (ULONG)(~((~(~0<<(3)))<<(29)))*/

#define MTL_QHCR_RES_Wr_Mask_29 (ULONG)(0x1fffffff)

/*#define MTL_QHCR_HC_Mask (ULONG)(~(~0<<(29)))*/

#define MTL_QHCR_HC_Mask (ULONG)(0x1fffffff)

/*#define MTL_QHCR_HC_Wr_Mask (ULONG)(~((~(~0 << (29))) << (0)))*/

#define MTL_QHCR_HC_Wr_Mask (ULONG)(0xe0000000)

#define MTL_QHCR_HC_UdfWr(i,data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QHCR_RES_Wr_Mask_29))|((( 0) & (MTL_QHCR_Mask_29))<<29);\
		(v) = ((v & MTL_QHCR_HC_Wr_Mask) | ((data & MTL_QHCR_HC_Mask)<<0));\
		MTL_QHCR_RgWr(i,v);\
} while(0)

#define MTL_QHCR_HC_UdfRd(i,data) do {\
		MTL_QHCR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QHCR_HC_Mask);\
} while(0)


#define MTL_QSSCR_RgOffAddr (BASE_ADDRESS + 0xd1c)

#define MTL_QSSCR_RgOffAddress(i) ((volatile ULONG *)(MTL_QSSCR_RgOffAddr + ((i-0)*64)))

#define MTL_QSSCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QSSCR_RgOffAddress(i));\
} while(0)

#define MTL_QSSCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QSSCR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QSSCR_Mask_14 (ULONG)(~(~0<<(18)))*/

#define  MTL_QSSCR_Mask_14 (ULONG)(0x3ffff)

/*#define MTL_QSSCR_RES_Wr_Mask_14 (ULONG)(~((~(~0<<(18)))<<(14)))*/

#define MTL_QSSCR_RES_Wr_Mask_14 (ULONG)(0x3fff)

/*#define MTL_QSSCR_SSC_Mask (ULONG)(~(~0<<(14)))*/

#define MTL_QSSCR_SSC_Mask (ULONG)(0x3fff)

/*#define MTL_QSSCR_SSC_Wr_Mask (ULONG)(~((~(~0 << (14))) << (0)))*/

#define MTL_QSSCR_SSC_Wr_Mask (ULONG)(0xffffc000)

#define MTL_QSSCR_SSC_UdfWr(i,data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QSSCR_RES_Wr_Mask_14))|((( 0) & (MTL_QSSCR_Mask_14))<<14);\
		(v) = ((v & MTL_QSSCR_SSC_Wr_Mask) | ((data & MTL_QSSCR_SSC_Mask)<<0));\
		MTL_QSSCR_RgWr(i,v);\
} while(0)

#define MTL_QSSCR_SSC_UdfRd(i,data) do {\
		MTL_QSSCR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QSSCR_SSC_Mask);\
} while(0)


#define MTL_QW_RgOffAddr (BASE_ADDRESS + 0xd18)

#define MTL_QW_RgOffAddress(i) ((volatile ULONG *)(MTL_QW_RgOffAddr + ((i-0)*64)))

#define MTL_QW_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QW_RgOffAddress(i));\
} while(0)

#define MTL_QW_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QW_RgOffAddress(i));\
} while(0)

/*#define  MTL_QW_Mask_21 (ULONG)(~(~0<<(11)))*/

#define  MTL_QW_Mask_21 (ULONG)(0x7ff)

/*#define MTL_QW_RES_Wr_Mask_21 (ULONG)(~((~(~0<<(11)))<<(21)))*/

#define MTL_QW_RES_Wr_Mask_21 (ULONG)(0x1fffff)

/*#define MTL_QW_ISCQW_Mask (ULONG)(~(~0<<(21)))*/

#define MTL_QW_ISCQW_Mask (ULONG)(0x1fffff)

/*#define MTL_QW_ISCQW_Wr_Mask (ULONG)(~((~(~0 << (21))) << (0)))*/

#define MTL_QW_ISCQW_Wr_Mask (ULONG)(0xffe00000)

#define MTL_QW_ISCQW_UdfWr(i,data) do {\
		ULONG v = 0; \
		v = (v & (MTL_QW_RES_Wr_Mask_21))|((( 0) & (MTL_QW_Mask_21))<<21);\
		(v) = ((v & MTL_QW_ISCQW_Wr_Mask) | ((data & MTL_QW_ISCQW_Mask)<<0));\
		MTL_QW_RgWr(i,v);\
} while(0)

#define MTL_QW_ISCQW_UdfRd(i,data) do {\
		MTL_QW_RgRd(i,data);\
		data = ((data >> 0) & MTL_QW_ISCQW_Mask);\
} while(0)


#define MTL_QESR_RgOffAddr (BASE_ADDRESS + 0xd14)

#define MTL_QESR_RgOffAddress(i) ((volatile ULONG *)(MTL_QESR_RgOffAddr + ((i-0)*64)))

#define MTL_QESR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QESR_RgOffAddress(i));\
} while(0)

/*#define MTL_QESR_ABSU_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QESR_ABSU_Mask (ULONG)(0x1)

#define MTL_QESR_ABSU_UdfRd(i,data) do {\
		MTL_QESR_RgRd(i,data);\
		data = ((data >> 24) & MTL_QESR_ABSU_Mask);\
} while(0)

/*#define MTL_QESR_ABS_Mask (ULONG)(~(~0<<(24)))*/

#define MTL_QESR_ABS_Mask (ULONG)(0xffffff)

#define MTL_QESR_ABS_UdfRd(i,data) do {\
		MTL_QESR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QESR_ABS_Mask);\
} while(0)


#define MTL_QECR_RgOffAddr (BASE_ADDRESS + 0xd10)

#define MTL_QECR_RgOffAddress(i) ((volatile ULONG *)(MTL_QECR_RgOffAddr + ((i-0)*64)))

#define MTL_QECR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QECR_RgOffAddress(i));\
} while(0)

#define MTL_QECR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QECR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QECR_Mask_25 (ULONG)(~(~0<<(7)))*/

#define  MTL_QECR_Mask_25 (ULONG)(0x7f)

/*#define MTL_QECR_RES_Wr_Mask_25 (ULONG)(~((~(~0<<(7)))<<(25)))*/

#define MTL_QECR_RES_Wr_Mask_25 (ULONG)(0x1ffffff)

/*#define  MTL_QECR_Mask_7 (ULONG)(~(~0<<(17)))*/

#define  MTL_QECR_Mask_7 (ULONG)(0x1ffff)

/*#define MTL_QECR_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(17)))<<(7)))*/

#define MTL_QECR_RES_Wr_Mask_7 (ULONG)(0xff00007f)

/*#define  MTL_QECR_Mask_0 (ULONG)(~(~0<<(2)))*/

#define  MTL_QECR_Mask_0 (ULONG)(0x3)

/*#define MTL_QECR_RES_Wr_Mask_0 (ULONG)(~((~(~0<<(2)))<<(0)))*/

#define MTL_QECR_RES_Wr_Mask_0 (ULONG)(0xfffffffc)

/*#define MTL_QECR_ABPSSIE_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR_ABPSSIE_Mask (ULONG)(0x1)

/*#define MTL_QECR_ABPSSIE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (24)))*/

#define MTL_QECR_ABPSSIE_Wr_Mask (ULONG)(0xfeffffff)

#define MTL_QECR_ABPSSIE_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QECR_RgRd(i,v);\
		v = (v & (MTL_QECR_RES_Wr_Mask_25))|((( 0) & (MTL_QECR_Mask_25))<<25);\
		v = (v & (MTL_QECR_RES_Wr_Mask_7))|((( 0) & (MTL_QECR_Mask_7))<<7);\
		v = (v & (MTL_QECR_RES_Wr_Mask_0))|((( 0) & (MTL_QECR_Mask_0))<<0);\
		v = ((v & MTL_QECR_ABPSSIE_Wr_Mask) | ((data & MTL_QECR_ABPSSIE_Mask)<<24));\
		MTL_QECR_RgWr(i,v);\
} while(0)

#define MTL_QECR_ABPSSIE_UdfRd(i,data) do {\
		MTL_QECR_RgRd(i,data);\
		data = ((data >> 24) & MTL_QECR_ABPSSIE_Mask);\
} while(0)

/*#define MTL_QECR_SLC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QECR_SLC_Mask (ULONG)(0x7)

/*#define MTL_QECR_SLC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QECR_SLC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QECR_SLC_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QECR_RgRd(i,v);\
		v = (v & (MTL_QECR_RES_Wr_Mask_25))|((( 0) & (MTL_QECR_Mask_25))<<25);\
		v = (v & (MTL_QECR_RES_Wr_Mask_7))|((( 0) & (MTL_QECR_Mask_7))<<7);\
		v = (v & (MTL_QECR_RES_Wr_Mask_0))|((( 0) & (MTL_QECR_Mask_0))<<0);\
		v = ((v & MTL_QECR_SLC_Wr_Mask) | ((data & MTL_QECR_SLC_Mask)<<4));\
		MTL_QECR_RgWr(i,v);\
} while(0)

#define MTL_QECR_SLC_UdfRd(i,data) do {\
		MTL_QECR_RgRd(i,data);\
		data = ((data >> 4) & MTL_QECR_SLC_Mask);\
} while(0)

/*#define MTL_QECR_CC_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR_CC_Mask (ULONG)(0x1)

/*#define MTL_QECR_CC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/

#define MTL_QECR_CC_Wr_Mask (ULONG)(0xfffffff7)

#define MTL_QECR_CC_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QECR_RgRd(i,v);\
		v = (v & (MTL_QECR_RES_Wr_Mask_25))|((( 0) & (MTL_QECR_Mask_25))<<25);\
		v = (v & (MTL_QECR_RES_Wr_Mask_7))|((( 0) & (MTL_QECR_Mask_7))<<7);\
		v = (v & (MTL_QECR_RES_Wr_Mask_0))|((( 0) & (MTL_QECR_Mask_0))<<0);\
		v = ((v & MTL_QECR_CC_Wr_Mask) | ((data & MTL_QECR_CC_Mask)<<3));\
		MTL_QECR_RgWr(i,v);\
} while(0)

#define MTL_QECR_CC_UdfRd(i,data) do {\
		MTL_QECR_RgRd(i,data);\
		data = ((data >> 3) & MTL_QECR_CC_Mask);\
} while(0)

/*#define MTL_QECR_AVALG_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QECR_AVALG_Mask (ULONG)(0x1)

/*#define MTL_QECR_AVALG_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/

#define MTL_QECR_AVALG_Wr_Mask (ULONG)(0xfffffffb)

#define MTL_QECR_AVALG_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QECR_RgRd(i,v);\
		v = (v & (MTL_QECR_RES_Wr_Mask_25))|((( 0) & (MTL_QECR_Mask_25))<<25);\
		v = (v & (MTL_QECR_RES_Wr_Mask_7))|((( 0) & (MTL_QECR_Mask_7))<<7);\
		v = (v & (MTL_QECR_RES_Wr_Mask_0))|((( 0) & (MTL_QECR_Mask_0))<<0);\
		v = ((v & MTL_QECR_AVALG_Wr_Mask) | ((data & MTL_QECR_AVALG_Mask)<<2));\
		MTL_QECR_RgWr(i,v);\
} while(0)

#define MTL_QECR_AVALG_UdfRd(i,data) do {\
		MTL_QECR_RgRd(i,data);\
		data = ((data >> 2) & MTL_QECR_AVALG_Mask);\
} while(0)


#define MTL_QTDR_RgOffAddr (BASE_ADDRESS + 0xd08)

#define MTL_QTDR_RgOffAddress(i) ((volatile ULONG *)(MTL_QTDR_RgOffAddr + ((i-0)*64)))

#define MTL_QTDR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QTDR_RgOffAddress(i));\
} while(0)

/*#define MTL_QTDR_STXSTSF_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR_STXSTSF_Mask (ULONG)(0x7)

#define MTL_QTDR_STXSTSF_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 20) & MTL_QTDR_STXSTSF_Mask);\
} while(0)

/*#define MTL_QTDR_PTXQ_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTDR_PTXQ_Mask (ULONG)(0x7)

#define MTL_QTDR_PTXQ_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 16) & MTL_QTDR_PTXQ_Mask);\
} while(0)

/*#define MTL_QTDR_TXSTSFSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR_TXSTSFSTS_Mask (ULONG)(0x1)

#define MTL_QTDR_TXSTSFSTS_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 5) & MTL_QTDR_TXSTSFSTS_Mask);\
} while(0)

/*#define MTL_QTDR_TXQSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR_TXQSTS_Mask (ULONG)(0x1)

#define MTL_QTDR_TXQSTS_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 4) & MTL_QTDR_TXQSTS_Mask);\
} while(0)

/*#define MTL_QTDR_TWCSTS_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR_TWCSTS_Mask (ULONG)(0x1)

#define MTL_QTDR_TWCSTS_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 3) & MTL_QTDR_TWCSTS_Mask);\
} while(0)

/*#define MTL_QTDR_TRCSTS_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTDR_TRCSTS_Mask (ULONG)(0x3)

#define MTL_QTDR_TRCSTS_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 1) & MTL_QTDR_TRCSTS_Mask);\
} while(0)

/*#define MTL_QTDR_TXQPAUSED_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTDR_TXQPAUSED_Mask (ULONG)(0x1)

#define MTL_QTDR_TXQPAUSED_UdfRd(i,data) do {\
		MTL_QTDR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QTDR_TXQPAUSED_Mask);\
} while(0)


#define MTL_QUCR_RgOffAddr (BASE_ADDRESS + 0xd04)

#define MTL_QUCR_RgOffAddress(i) ((volatile ULONG *)(MTL_QUCR_RgOffAddr + ((i-0)*64)))

#define MTL_QUCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QUCR_RgOffAddress(i));\
} while(0)

#define MTL_QUCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QUCR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QUCR_Mask_12 (ULONG)(~(~0<<(20)))*/

#define  MTL_QUCR_Mask_12 (ULONG)(0xfffff)

/*#define MTL_QUCR_RES_Wr_Mask_12 (ULONG)(~((~(~0<<(20)))<<(12)))*/

#define MTL_QUCR_RES_Wr_Mask_12 (ULONG)(0xfff)

/*#define MTL_QUCR_UFCNTOVF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QUCR_UFCNTOVF_Mask (ULONG)(0x1)

#define MTL_QUCR_UFCNTOVF_UdfRd(i,data) do {\
		MTL_QUCR_RgRd(i,data);\
		data = ((data >> 11) & MTL_QUCR_UFCNTOVF_Mask);\
} while(0)

/*#define MTL_QUCR_UFPKTCNT_Mask (ULONG)(~(~0<<(11)))*/

#define MTL_QUCR_UFPKTCNT_Mask (ULONG)(0x7ff)

/*#define MTL_QUCR_UFPKTCNT_Wr_Mask (ULONG)(~((~(~0 << (11))) << (0)))*/

#define MTL_QUCR_UFPKTCNT_Wr_Mask (ULONG)(0xfffff800)

#define MTL_QUCR_UFPKTCNT_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QUCR_RgRd(i,v);\
		v = (v & (MTL_QUCR_RES_Wr_Mask_12))|((( 0) & (MTL_QUCR_Mask_12))<<12);\
		v = ((v & MTL_QUCR_UFPKTCNT_Wr_Mask) | ((data & MTL_QUCR_UFPKTCNT_Mask)<<0));\
		MTL_QUCR_RgWr(i,v);\
} while(0)

#define MTL_QUCR_UFPKTCNT_UdfRd(i,data) do {\
		MTL_QUCR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QUCR_UFPKTCNT_Mask);\
} while(0)


#define MTL_QTOMR_RgOffAddr (BASE_ADDRESS + 0xd00)

#define MTL_QTOMR_RgOffAddress(i) ((volatile ULONG *)(MTL_QTOMR_RgOffAddr + ((i-0)*64)))

#define MTL_QTOMR_RgWr(i,data) do {\
		iowrite32(data, (void *)MTL_QTOMR_RgOffAddress(i));\
} while(0)

#define MTL_QTOMR_RgRd(i,data) do {\
		(data) = ioread32((void *)MTL_QTOMR_RgOffAddress(i));\
} while(0)

/*#define  MTL_QTOMR_Mask_26 (ULONG)(~(~0<<(6)))*/

#define  MTL_QTOMR_Mask_26 (ULONG)(0x3f)

/*#define MTL_QTOMR_RES_Wr_Mask_26 (ULONG)(~((~(~0<<(6)))<<(26)))*/

#define MTL_QTOMR_RES_Wr_Mask_26 (ULONG)(0x3ffffff)

/*#define  MTL_QTOMR_Mask_7 (ULONG)(~(~0<<(9)))*/

#define  MTL_QTOMR_Mask_7 (ULONG)(0x1ff)

/*#define MTL_QTOMR_RES_Wr_Mask_7 (ULONG)(~((~(~0<<(9)))<<(7)))*/

#define MTL_QTOMR_RES_Wr_Mask_7 (ULONG)(0xffff007f)

/*#define MTL_QTOMR_TQS_Mask (ULONG)(~(~0<<(10)))*/

#define MTL_QTOMR_TQS_Mask (ULONG)(0x3ff)

/*#define MTL_QTOMR_TQS_Wr_Mask (ULONG)(~((~(~0 << (10))) << (16)))*/

#define MTL_QTOMR_TQS_Wr_Mask (ULONG)(0xfc00ffff)

#define MTL_QTOMR_TQS_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QTOMR_RgRd(i,v);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR_Mask_26))<<26);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR_Mask_7))<<7);\
		v = ((v & MTL_QTOMR_TQS_Wr_Mask) | ((data & MTL_QTOMR_TQS_Mask)<<16));\
		MTL_QTOMR_RgWr(i,v);\
} while(0)

#define MTL_QTOMR_TQS_UdfRd(i,data) do {\
		MTL_QTOMR_RgRd(i,data);\
		data = ((data >> 16) & MTL_QTOMR_TQS_Mask);\
} while(0)

/*#define MTL_QTOMR_TTC_Mask (ULONG)(~(~0<<(3)))*/

#define MTL_QTOMR_TTC_Mask (ULONG)(0x7)

/*#define MTL_QTOMR_TTC_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MTL_QTOMR_TTC_Wr_Mask (ULONG)(0xffffff8f)

#define MTL_QTOMR_TTC_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QTOMR_RgRd(i,v);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR_Mask_26))<<26);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR_Mask_7))<<7);\
		v = ((v & MTL_QTOMR_TTC_Wr_Mask) | ((data & MTL_QTOMR_TTC_Mask)<<4));\
		MTL_QTOMR_RgWr(i,v);\
} while(0)

#define MTL_QTOMR_TTC_UdfRd(i,data) do {\
		MTL_QTOMR_RgRd(i,data);\
		data = ((data >> 4) & MTL_QTOMR_TTC_Mask);\
} while(0)

/*#define MTL_QTOMR_TXQEN_Mask (ULONG)(~(~0<<(2)))*/

#define MTL_QTOMR_TXQEN_Mask (ULONG)(0x3)

/*#define MTL_QTOMR_TXQEN_Wr_Mask (ULONG)(~((~(~0 << (2))) << (2)))*/

#define MTL_QTOMR_TXQEN_Wr_Mask (ULONG)(0xfffffff3)

#define MTL_QTOMR_TXQEN_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QTOMR_RgRd(i,v);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR_Mask_26))<<26);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR_Mask_7))<<7);\
		v = ((v & MTL_QTOMR_TXQEN_Wr_Mask) | ((data & MTL_QTOMR_TXQEN_Mask)<<2));\
		MTL_QTOMR_RgWr(i,v);\
} while(0)

#define MTL_QTOMR_TXQEN_UdfRd(i,data) do {\
		MTL_QTOMR_RgRd(i,data);\
		data = ((data >> 2) & MTL_QTOMR_TXQEN_Mask);\
} while(0)

/*#define MTL_QTOMR_TSF_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR_TSF_Mask (ULONG)(0x1)

/*#define MTL_QTOMR_TSF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MTL_QTOMR_TSF_Wr_Mask (ULONG)(0xfffffffd)

#define MTL_QTOMR_TSF_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QTOMR_RgRd(i,v);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR_Mask_26))<<26);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR_Mask_7))<<7);\
		v = ((v & MTL_QTOMR_TSF_Wr_Mask) | ((data & MTL_QTOMR_TSF_Mask)<<1));\
		MTL_QTOMR_RgWr(i,v);\
} while(0)

#define MTL_QTOMR_TSF_UdfRd(i,data) do {\
		MTL_QTOMR_RgRd(i,data);\
		data = ((data >> 1) & MTL_QTOMR_TSF_Mask);\
} while(0)

/*#define MTL_QTOMR_FTQ_Mask (ULONG)(~(~0<<(1)))*/

#define MTL_QTOMR_FTQ_Mask (ULONG)(0x1)

/*#define MTL_QTOMR_FTQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MTL_QTOMR_FTQ_Wr_Mask (ULONG)(0xfffffffe)

#define MTL_QTOMR_FTQ_UdfWr(i,data) do {\
		ULONG v;\
		MTL_QTOMR_RgRd(i,v);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_26))|((( 0) & (MTL_QTOMR_Mask_26))<<26);\
		v = (v & (MTL_QTOMR_RES_Wr_Mask_7))|((( 0) & (MTL_QTOMR_Mask_7))<<7);\
		v = ((v & MTL_QTOMR_FTQ_Wr_Mask) | ((data & MTL_QTOMR_FTQ_Mask)<<0));\
		MTL_QTOMR_RgWr(i,v);\
} while(0)

#define MTL_QTOMR_FTQ_UdfRd(i,data) do {\
		MTL_QTOMR_RgRd(i,data);\
		data = ((data >> 0) & MTL_QTOMR_FTQ_Mask);\
} while(0)


#define MAC_HTR_RgOffAddr (BASE_ADDRESS + 0x10)

#define MAC_HTR_RgOffAddress(i) ((volatile ULONG *)(MAC_HTR_RgOffAddr + ((i-0)*4)))

#define MAC_HTR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_HTR_RgOffAddress(i));\
} while(0)

#define MAC_HTR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_HTR_RgOffAddress(i));\
} while(0)

#define MAC_HTR_HT_UdfWr(i,data) do {\
		MAC_HTR_RgWr(i,data);\
} while(0)

#define MAC_HTR_HT_UdfRd(i,data) do {\
		MAC_HTR_RgRd(i,data);\
} while(0)




#define MAC_QTFCR_RgOffAddr (BASE_ADDRESS + 0x70)

#define MAC_QTFCR_RgOffAddress(i) ((volatile ULONG *)(MAC_QTFCR_RgOffAddr + ((i-0)*4)))

#define MAC_QTFCR_RgWr(i,data) do {\
		iowrite32(data, (void *)MAC_QTFCR_RgOffAddress(i));\
} while(0)

#define MAC_QTFCR_RgRd(i,data) do {\
		(data) = ioread32((void *)MAC_QTFCR_RgOffAddress(i));\
} while(0)

/*#define  MAC_QTFCR_Mask_8 (ULONG)(~(~0<<(8)))*/

#define  MAC_QTFCR_Mask_8 (ULONG)(0xff)

/*#define MAC_QTFCR_RES_Wr_Mask_8 (ULONG)(~((~(~0<<(8)))<<(8)))*/

#define MAC_QTFCR_RES_Wr_Mask_8 (ULONG)(0xffff00ff)

/*#define  MAC_QTFCR_Mask_2 (ULONG)(~(~0<<(2)))*/

#define  MAC_QTFCR_Mask_2 (ULONG)(0x3)

/*#define MAC_QTFCR_RES_Wr_Mask_2 (ULONG)(~((~(~0<<(2)))<<(2)))*/

#define MAC_QTFCR_RES_Wr_Mask_2 (ULONG)(0xfffffff3)

/*#define MAC_QTFCR_PT_Mask (ULONG)(~(~0<<(16)))*/

#define MAC_QTFCR_PT_Mask (ULONG)(0xffff)

/*#define MAC_QTFCR_PT_Wr_Mask (ULONG)(~((~(~0 << (16))) << (16)))*/

#define MAC_QTFCR_PT_Wr_Mask (ULONG)(0xffff)

#define MAC_QTFCR_PT_UdfWr(i,data) do {\
		ULONG v;\
		MAC_QTFCR_RgRd(i,v);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR_Mask_8))<<8);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR_Mask_2))<<2);\
		v = ((v & MAC_QTFCR_PT_Wr_Mask) | ((data & MAC_QTFCR_PT_Mask)<<16));\
		MAC_QTFCR_RgWr(i,v);\
} while(0)

#define MAC_QTFCR_PT_UdfRd(i,data) do {\
		MAC_QTFCR_RgRd(i,data);\
		data = ((data >> 16) & MAC_QTFCR_PT_Mask);\
} while(0)

/*#define MAC_QTFCR_DZPQ_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR_DZPQ_Mask (ULONG)(0x1)

/*#define MAC_QTFCR_DZPQ_Wr_Mask (ULONG)(~((~(~0 << (1))) << (7)))*/

#define MAC_QTFCR_DZPQ_Wr_Mask (ULONG)(0xffffff7f)

#define MAC_QTFCR_DZPQ_UdfWr(i,data) do {\
		ULONG v;\
		MAC_QTFCR_RgRd(i,v);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR_Mask_8))<<8);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR_Mask_2))<<2);\
		v = ((v & MAC_QTFCR_DZPQ_Wr_Mask) | ((data & MAC_QTFCR_DZPQ_Mask)<<7));\
		MAC_QTFCR_RgWr(i,v);\
} while(0)

#define MAC_QTFCR_DZPQ_UdfRd(i,data) do {\
		MAC_QTFCR_RgRd(i,data);\
		data = ((data >> 7) & MAC_QTFCR_DZPQ_Mask);\
} while(0)

/*#define MAC_QTFCR_PLT_Mask (ULONG)(~(~0<<(3)))*/

#define MAC_QTFCR_PLT_Mask (ULONG)(0x7)

/*#define MAC_QTFCR_PLT_Wr_Mask (ULONG)(~((~(~0 << (3))) << (4)))*/

#define MAC_QTFCR_PLT_Wr_Mask (ULONG)(0xffffff8f)

#define MAC_QTFCR_PLT_UdfWr(i,data) do {\
		ULONG v;\
		MAC_QTFCR_RgRd(i,v);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR_Mask_8))<<8);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR_Mask_2))<<2);\
		v = ((v & MAC_QTFCR_PLT_Wr_Mask) | ((data & MAC_QTFCR_PLT_Mask)<<4));\
		MAC_QTFCR_RgWr(i,v);\
} while(0)

#define MAC_QTFCR_PLT_UdfRd(i,data) do {\
		MAC_QTFCR_RgRd(i,data);\
		data = ((data >> 4) & MAC_QTFCR_PLT_Mask);\
} while(0)

/*#define MAC_QTFCR_TFE_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR_TFE_Mask (ULONG)(0x1)

/*#define MAC_QTFCR_TFE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/

#define MAC_QTFCR_TFE_Wr_Mask (ULONG)(0xfffffffd)

#define MAC_QTFCR_TFE_UdfWr(i,data) do {\
		ULONG v;\
		MAC_QTFCR_RgRd(i,v);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR_Mask_8))<<8);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR_Mask_2))<<2);\
		v = ((v & MAC_QTFCR_TFE_Wr_Mask) | ((data & MAC_QTFCR_TFE_Mask)<<1));\
		MAC_QTFCR_RgWr(i,v);\
} while(0)

#define MAC_QTFCR_TFE_UdfRd(i,data) do {\
		MAC_QTFCR_RgRd(i,data);\
		data = ((data >> 1) & MAC_QTFCR_TFE_Mask);\
} while(0)

/*#define MAC_QTFCR_FCB_BPA_Mask (ULONG)(~(~0<<(1)))*/

#define MAC_QTFCR_FCB_BPA_Mask (ULONG)(0x1)

/*#define MAC_QTFCR_FCB_BPA_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/

#define MAC_QTFCR_FCB_BPA_Wr_Mask (ULONG)(0xfffffffe)

#define MAC_QTFCR_FCB_BPA_UdfWr(i,data) do {\
		ULONG v;\
		MAC_QTFCR_RgRd(i,v);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_8))|((( 0) & (MAC_QTFCR_Mask_8))<<8);\
		v = (v & (MAC_QTFCR_RES_Wr_Mask_2))|((( 0) & (MAC_QTFCR_Mask_2))<<2);\
		v = ((v & MAC_QTFCR_FCB_BPA_Wr_Mask) | ((data & MAC_QTFCR_FCB_BPA_Mask)<<0));\
		MAC_QTFCR_RgWr(i,v);\
} while(0)

#define MAC_QTFCR_FCB_BPA_UdfRd(i,data) do {\
		MAC_QTFCR_RgRd(i,data);\
		data = ((data >> 0) & MAC_QTFCR_FCB_BPA_Mask);\
} while(0)



#define MAC_ALPA_PSE_LPOS 7
#define MAC_ALPA_PSE_HPOS 8

#define MAC_ALPA_FD_LPOS 5
#define MAC_ALPA_FD_HPOS 5

#define MAC_AAD_PSE_LPOS 7
#define MAC_AAD_PSE_HPOS 8

#define MAC_AAD_FD_LPOS 5
#define MAC_AAD_FD_HPOS 5

#define MAC_TTN_TXTSSTSHI_LPOS 0
#define MAC_TTN_TXTSSTSHI_HPOS 31

#define MAC_TTSN_TXTSSTSMIS_LPOS 31
#define MAC_TTSN_TXTSSTSMIS_HPOS 31

#define MAC_TTSN_TXTSSTSLO_LPOS 0
#define MAC_TTSN_TXTSSTSLO_HPOS 30

#define MAC_STNSR_TSSS_LPOS 0
#define MAC_STNSR_TSSS_HPOS 30

#define MAC_TCR_TXTSSTSM_LPOS 24
#define MAC_TCR_TXTSSTSM_HPOS 24

#define MAC_TCR_TSCTRLSSR_LPOS 9
#define MAC_TCR_TSCTRLSSR_HPOS 9

#define MAC_TCR_TSADDREG_LPOS 5
#define MAC_TCR_TSADDREG_HPOS 5

#define MAC_TCR_TSUPDT_LPOS 3
#define MAC_TCR_TSUPDT_HPOS 3

#define MAC_TCR_TSINIT_LPOS 2
#define MAC_TCR_TSINIT_HPOS 2

#define MAC_TCR_TSCFUPDT_LPOS 1
#define MAC_TCR_TSCFUPDT_HPOS 1

#define MAC_GMIIDR_GD_LPOS 0
#define MAC_GMIIDR_GD_HPOS 15

#define MAC_GMIIAR_GB_LPOS 0
#define MAC_GMIIAR_GB_HPOS 0

#define MAC_HFR2_TXQCNT_LPOS 6
#define MAC_HFR2_TXQCNT_HPOS 9

#define MAC_HFR2_RXQCNT_LPOS 0
#define MAC_HFR2_RXQCNT_HPOS 3

#define MAC_VLANHTR_VLHT_LPOS 0
#define MAC_VLANHTR_VLHT_HPOS 15

#define MTL_Q0TOMR_FTQ_LPOS 0
#define MTL_Q0TOMR_FTQ_HPOS 0

#define MTL_QTOMR_FTQ_LPOS 0
#define MTL_QTOMR_FTQ_HPOS 0

#define MTL_OMR_DTXSTS_LPOS 1
#define MTL_OMR_DTXSTS_HPOS 1

#define MAC_MCR_IPC_LPOS 27
#define MAC_MCR_IPC_HPOS 27

#define MAC_ISR_PMTIS_LPOS 4
#define MAC_ISR_PMTIS_HPOS 4

#define MAC_ISR_PCSANCIS_LPOS 2
#define MAC_ISR_PCSANCIS_HPOS 2

#define MAC_ISR_PCSLCHGIS_LPOS 1
#define MAC_ISR_PCSLCHGIS_HPOS 1

#define MAC_ISR_RGSMIIS_LPOS 0
#define MAC_ISR_RGSMIIS_HPOS 0

#define MTL_QECR_ABPSSIE_LPOS 24
#define MTL_QECR_ABPSSIE_HPOS 24

#define MAC_ANS_LS_LPOS 2
#define MAC_ANS_LS_HPOS 2

#define MAC_ISR_LPI_LPOS 5
#define MAC_ISR_LPI_HPOS 5

#define GET_VALUE(data, lbit, hbit) ((data >>lbit) & (~(~0<<(hbit-lbit+1))))

#define GET_INDEXED_VALUE(data, lbit, hbit,index)\
  (GET_VALUE(data,(lbit+(index)*(hbit-lbit+1)),(hbit+(index)*(hbit-lbit+1))))
#endif


/******************************************************************************/
/**                  Neutrino IP DMA & Wrapper DMA registers                 **/
/******************************************************************************/
/* virtual_registers Low Bit Macro Name's */
#define RX_CONTEXT_DESC_RDES3_OWN_LBIT_POS  0x1f
#define RX_CONTEXT_DESC_RDES3_CTXT_LBIT_POS  0x1e
#define RX_CONTEXT_DESC_RDES2_RESERVED_BITS_LBIT_POS  0
#define RX_CONTEXT_DESC_RDES1_RTSH_LBIT_POS  0
#define RX_CONTEXT_DESC_RDES0_RTSL_LBIT_POS  0
#define TX_CONTEXT_DESC_TDES3_OWN_LBIT_POS  0x1f
#define TX_CONTEXT_DESC_TDES3_CTXT_LBIT_POS  0x1e
#define TX_CONTEXT_DESC_TDES3_OSTC_LBIT_POS  0x1b
#define TX_CONTEXT_DESC_TDES3_TCMSSV_LBIT_POS  0x1a
#define TX_CONTEXT_DESC_TDES3_CDX_LBIT_POS  0x17
#define TX_CONTEXT_DESC_TDES3_IVTIR_LBIT_POS  0x12
#define TX_CONTEXT_DESC_TDES3_SVLTV_LBIT_POS  0x11
#define TX_CONTEXT_DESC_TDES3_IVLTV_LBIT_POS  0x11
#define TX_CONTEXT_DESC_TDES3_VLTV_LBIT_POS  0x10
#define TX_CONTEXT_DESC_TDES2_IVT_LBIT_POS  0x10
#define TX_CONTEXT_DESC_TDES3_VT_LBIT_POS  0
#define TX_CONTEXT_DESC_TDES2_SVT_LBIT_POS  0xf
#define TX_CONTEXT_DESC_TDES2_MSS_LBIT_POS  0
#define TX_CONTEXT_DESC_TDES1_NDAP_LBIT_POS  0
#define TX_CONTEXT_DESC_TDES0_TTSL_LBIT_POS  0
#define RX_NORMAL_DESC_RDES3_OWN_LBIT_POS  0x1f
#define RX_NORMAL_DESC_RDES3_CTXT_LBIT_POS  0x1e
#define RX_NORMAL_DESC_RDES3_FD_LBIT_POS  0x1d
#define RX_NORMAL_DESC_RDES3_LD_LBIT_POS  0x1c
#define RX_NORMAL_DESC_RDES3_RS2V_LBIT_POS  0x1b
#define RX_NORMAL_DESC_RDES3_RS1V_LBIT_POS  0x1a
#define RX_NORMAL_DESC_RDES3_RS0V_LBIT_POS  0x19
#define RX_NORMAL_DESC_RDES3_CE_LBIT_POS  0x18
#define RX_NORMAL_DESC_RDES3_GP_LBIT_POS  0x17
#define RX_NORMAL_DESC_RDES3_RWT_LBIT_POS  0x16
#define RX_NORMAL_DESC_RDES3_OE_LBIT_POS  0x15
#define RX_NORMAL_DESC_RDES3_RE_LBIT_POS  0x14
#define RX_NORMAL_DESC_RDES3_DE_LBIT_POS  0x13
#define RX_NORMAL_DESC_RDES3_LT_LBIT_POS  0x10
#define RX_NORMAL_DESC_RDES3_ES_LBIT_POS  0xf
#define RX_NORMAL_DESC_RDES3_FL_LBIT_POS  0
#define RX_NORMAL_DESC_RDES2_B2AP_NDA_LBIT_POS  0
#define RX_NORMAL_DESC_RDES1_COP_LBIT_POS  0x10
#define RX_NORMAL_DESC_RDES1_TD_LBIT_POS  0xf
#define RX_NORMAL_DESC_RDES1_TSA_LBIT_POS  0xe
#define RX_NORMAL_DESC_RDES1_PV_LBIT_POS  0xd
#define RX_NORMAL_DESC_RDES1_PFT_LBIT_POS  0xc
#define RX_NORMAL_DESC_RDES1_PMT_LBIT_POS  0x8
#define RX_NORMAL_DESC_RDES1_IPPE_LBIT_POS  0x7
#define RX_NORMAL_DESC_RDES1_IPCB_LBIT_POS  0x6
#define RX_NORMAL_DESC_RDES1_IPV6_LBIT_POS  0x5
#define RX_NORMAL_DESC_RDES1_IPV4_LBIT_POS  0x4
#define RX_NORMAL_DESC_RDES1_IPHE_LBIT_POS  0x3
#define RX_NORMAL_DESC_RDES1_PT_LBIT_POS  0
#define RX_NORMAL_DESC_RDES0_HDR_B1AP_LBIT_POS  0
#define TX_NORMAL_DESC_TDES3_OWN_LBIT_POS  0x1f
#define TX_NORMAL_DESC_TDES3_CTXT_LBIT_POS  0x1e
#define TX_NORMAL_DESC_TDES3_FD_LBIT_POS  0x1d
#define TX_NORMAL_DESC_TDES3_LD_LBIT_POS  0x1c
#define TX_NORMAL_DESC_TDES3_CPC_LBIT_POS  0x1a
#define TX_NORMAL_DESC_TDES3_SAIC_LBIT_POS  0x17
#define TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_LBIT_POS  0x13
#define TX_NORMAL_DESC_TDES3_TSE_LBIT_POS  0x12
#define TX_NORMAL_DESC_TDES3_CIC_LBIT_POS  0x10
#define TX_NORMAL_DESC_TDES3_TIPLH_LBIT_POS  0xf
#define TX_NORMAL_DESC_TDES3_FL_LBIT_POS  0
#define TX_NORMAL_DESC_TDES2_IC_LBIT_POS  0x1f
#define TX_NORMAL_DESC_TDES2_TTSE_LBIT_POS  0x1e
#define TX_NORMAL_DESC_TDES2_B2L_LBIT_POS  0x10
#define TX_NORMAL_DESC_TDES2_VTIR_LBIT_POS  0xe
#define TX_NORMAL_DESC_TDES2_HL_B1L_LBIT_POS  0
#define TX_NORMAL_DESC_TDES1_B2A_NDA_LBIT_POS  0
#define TX_NORMAL_DESC_TDES0_B1A_HAP_LBIT_POS  0

/* virtual_registers High Bit Macro Name's */
#define RX_CONTEXT_DESC_RDES3_OWN_HBIT_POS  0x1f
#define RX_CONTEXT_DESC_RDES3_CTXT_HBIT_POS  0x1e
#define RX_CONTEXT_DESC_RDES2_RESERVED_BITS_HBIT_POS  0x1f
#define RX_CONTEXT_DESC_RDES1_RTSH_HBIT_POS  0x1f
#define RX_CONTEXT_DESC_RDES0_RTSL_HBIT_POS  0x1f
#define TX_CONTEXT_DESC_TDES3_OWN_HBIT_POS  0x1f
#define TX_CONTEXT_DESC_TDES3_CTXT_HBIT_POS  0x1e
#define TX_CONTEXT_DESC_TDES3_OSTC_HBIT_POS  0x1b
#define TX_CONTEXT_DESC_TDES3_TCMSSV_HBIT_POS  0x1a
#define TX_CONTEXT_DESC_TDES3_CDX_HBIT_POS  0x17
#define TX_CONTEXT_DESC_TDES3_IVTIR_HBIT_POS  0x13
#define TX_CONTEXT_DESC_TDES3_SVLTV_HBIT_POS  0x11
#define TX_CONTEXT_DESC_TDES3_IVLTV_HBIT_POS  0x11
#define TX_CONTEXT_DESC_TDES3_VLTV_HBIT_POS  0x10
#define TX_CONTEXT_DESC_TDES3_VT_HBIT_POS  0xf
#define TX_CONTEXT_DESC_TDES2_IVT_HBIT_POS  0x1f
#define TX_CONTEXT_DESC_TDES2_SVT_HBIT_POS  0x1f
#define TX_CONTEXT_DESC_TDES2_MSS_HBIT_POS  0xe
#define TX_CONTEXT_DESC_TDES1_NDAP_HBIT_POS  0x1f
#define TX_CONTEXT_DESC_TDES0_TTSL_HBIT_POS  0x1f
#define RX_NORMAL_DESC_RDES3_OWN_HBIT_POS  0x1f
#define RX_NORMAL_DESC_RDES3_CTXT_HBIT_POS  0x1e
#define RX_NORMAL_DESC_RDES3_FD_HBIT_POS  0x1d
#define RX_NORMAL_DESC_RDES3_LD_HBIT_POS  0x1c
#define RX_NORMAL_DESC_RDES3_RS2V_HBIT_POS  0x1b
#define RX_NORMAL_DESC_RDES3_RS1V_HBIT_POS  0x1a
#define RX_NORMAL_DESC_RDES3_RS0V_HBIT_POS  0x19
#define RX_NORMAL_DESC_RDES3_CE_HBIT_POS  0x18
#define RX_NORMAL_DESC_RDES3_GP_HBIT_POS  0x17
#define RX_NORMAL_DESC_RDES3_RWT_HBIT_POS  0x16
#define RX_NORMAL_DESC_RDES3_OE_HBIT_POS  0x15
#define RX_NORMAL_DESC_RDES3_RE_HBIT_POS  0x14
#define RX_NORMAL_DESC_RDES3_DE_HBIT_POS  0x13
#define RX_NORMAL_DESC_RDES3_LT_HBIT_POS  0x12
#define RX_NORMAL_DESC_RDES3_ES_HBIT_POS  0xf
#define RX_NORMAL_DESC_RDES3_FL_HBIT_POS  0xe
#define RX_NORMAL_DESC_RDES2_B2AP_NDA_HBIT_POS  0x1f
#define RX_NORMAL_DESC_RDES1_COP_HBIT_POS  0x1f
#define RX_NORMAL_DESC_RDES1_TD_HBIT_POS  0xf
#define RX_NORMAL_DESC_RDES1_TSA_HBIT_POS  0xe
#define RX_NORMAL_DESC_RDES1_PV_HBIT_POS  0xd
#define RX_NORMAL_DESC_RDES1_PFT_HBIT_POS  0xc
#define RX_NORMAL_DESC_RDES1_PMT_HBIT_POS  0xb
#define RX_NORMAL_DESC_RDES1_IPPE_HBIT_POS  0x7
#define RX_NORMAL_DESC_RDES1_IPCB_HBIT_POS  0x6
#define RX_NORMAL_DESC_RDES1_IPV6_HBIT_POS  0x5
#define RX_NORMAL_DESC_RDES1_IPV4_HBIT_POS  0x4
#define RX_NORMAL_DESC_RDES1_IPHE_HBIT_POS  0x3
#define RX_NORMAL_DESC_RDES1_PT_HBIT_POS  0x2
#define RX_NORMAL_DESC_RDES0_HDR_B1AP_HBIT_POS  0x1f
#define TX_NORMAL_DESC_TDES3_OWN_HBIT_POS  0x1f
#define TX_NORMAL_DESC_TDES3_CTXT_HBIT_POS  0x1e
#define TX_NORMAL_DESC_TDES3_FD_HBIT_POS  0x1d
#define TX_NORMAL_DESC_TDES3_LD_HBIT_POS  0x1c
#define TX_NORMAL_DESC_TDES3_CPC_HBIT_POS  0x1b
#define TX_NORMAL_DESC_TDES3_SAIC_HBIT_POS  0x19
#define TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_HBIT_POS  0x16
#define TX_NORMAL_DESC_TDES3_TSE_HBIT_POS  0x12
#define TX_NORMAL_DESC_TDES3_CIC_HBIT_POS  0x11
#define TX_NORMAL_DESC_TDES3_TIPLH_HBIT_POS  0xf
#define TX_NORMAL_DESC_TDES3_FL_HBIT_POS  0xe
#define TX_NORMAL_DESC_TDES2_IC_HBIT_POS  0x1f
#define TX_NORMAL_DESC_TDES2_TTSE_HBIT_POS  0x1e
#define TX_NORMAL_DESC_TDES2_B2L_HBIT_POS  0x1d
#define TX_NORMAL_DESC_TDES2_VTIR_HBIT_POS  0xf
#define TX_NORMAL_DESC_TDES2_HL_B1L_HBIT_POS  0xd
#define TX_NORMAL_DESC_TDES1_B2A_NDA_HBIT_POS  0x1f
#define TX_NORMAL_DESC_TDES0_B1A_HAP_HBIT_POS  0x1f

/* virtual_registers Register-Field Read-Write Macros */
#define RX_CONTEXT_DESC_RDES3_OWN_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_CONTEXT_DESC_RDES3_OWN_HBIT_POS, RX_CONTEXT_DESC_RDES3_OWN_LBIT_POS, ptr, data); \
} while(0)

#define RX_CONTEXT_DESC_RDES3_OWN_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_CONTEXT_DESC_RDES3_OWN_HBIT_POS, RX_CONTEXT_DESC_RDES3_OWN_LBIT_POS, ptr, data); \
} while(0)


#define RX_CONTEXT_DESC_RDES3_CTXT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_CONTEXT_DESC_RDES3_CTXT_HBIT_POS, RX_CONTEXT_DESC_RDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)

#define RX_CONTEXT_DESC_RDES3_CTXT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_CONTEXT_DESC_RDES3_CTXT_HBIT_POS, RX_CONTEXT_DESC_RDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)


#define RX_CONTEXT_DESC_RDES2_RESERVED_BITS_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_CONTEXT_DESC_RDES2_RESERVED_BITS_HBIT_POS, RX_CONTEXT_DESC_RDES2_RESERVED_BITS_LBIT_POS, ptr, data); \
} while(0)

#define RX_CONTEXT_DESC_RDES2_RESERVED_BITS_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_CONTEXT_DESC_RDES2_RESERVED_BITS_HBIT_POS, RX_CONTEXT_DESC_RDES2_RESERVED_BITS_LBIT_POS, ptr, data); \
} while(0)


#define RX_CONTEXT_DESC_RDES1_RTSH_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_CONTEXT_DESC_RDES1_RTSH_HBIT_POS, RX_CONTEXT_DESC_RDES1_RTSH_LBIT_POS, ptr, data); \
} while(0)

#define RX_CONTEXT_DESC_RDES1_RTSH_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_CONTEXT_DESC_RDES1_RTSH_HBIT_POS, RX_CONTEXT_DESC_RDES1_RTSH_LBIT_POS, ptr, data); \
} while(0)


#define RX_CONTEXT_DESC_RDES0_RTSL_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_CONTEXT_DESC_RDES0_RTSL_HBIT_POS, RX_CONTEXT_DESC_RDES0_RTSL_LBIT_POS, ptr, data); \
} while(0)

#define RX_CONTEXT_DESC_RDES0_RTSL_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_CONTEXT_DESC_RDES0_RTSL_HBIT_POS, RX_CONTEXT_DESC_RDES0_RTSL_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_OWN_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_OWN_HBIT_POS, TX_CONTEXT_DESC_TDES3_OWN_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_OWN_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_OWN_HBIT_POS, TX_CONTEXT_DESC_TDES3_OWN_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_CTXT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_CTXT_HBIT_POS, TX_CONTEXT_DESC_TDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_CTXT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_CTXT_HBIT_POS, TX_CONTEXT_DESC_TDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_OSTC_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_OSTC_HBIT_POS, TX_CONTEXT_DESC_TDES3_OSTC_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_OSTC_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_OSTC_HBIT_POS, TX_CONTEXT_DESC_TDES3_OSTC_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_TCMSSV_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_TCMSSV_HBIT_POS, TX_CONTEXT_DESC_TDES3_TCMSSV_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_TCMSSV_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_TCMSSV_HBIT_POS, TX_CONTEXT_DESC_TDES3_TCMSSV_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_CDX_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_CDX_HBIT_POS, TX_CONTEXT_DESC_TDES3_CDX_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_CDX_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_CDX_HBIT_POS, TX_CONTEXT_DESC_TDES3_CDX_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_IVTIR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_IVTIR_HBIT_POS, TX_CONTEXT_DESC_TDES3_IVTIR_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_IVTIR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_IVTIR_HBIT_POS, TX_CONTEXT_DESC_TDES3_IVTIR_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_SVLTV_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_SVLTV_HBIT_POS, TX_CONTEXT_DESC_TDES3_SVLTV_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_SVLTV_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_SVLTV_HBIT_POS, TX_CONTEXT_DESC_TDES3_SVLTV_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_IVLTV_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_IVLTV_HBIT_POS, TX_CONTEXT_DESC_TDES3_IVLTV_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_IVLTV_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_IVLTV_HBIT_POS, TX_CONTEXT_DESC_TDES3_IVLTV_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_VLTV_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_VLTV_HBIT_POS, TX_CONTEXT_DESC_TDES3_VLTV_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_VLTV_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_VLTV_HBIT_POS, TX_CONTEXT_DESC_TDES3_VLTV_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES3_VT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES3_VT_HBIT_POS, TX_CONTEXT_DESC_TDES3_VT_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES3_VT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES3_VT_HBIT_POS, TX_CONTEXT_DESC_TDES3_VT_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES2_IVT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES2_IVT_HBIT_POS, TX_CONTEXT_DESC_TDES2_IVT_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES2_IVT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES2_IVT_HBIT_POS, TX_CONTEXT_DESC_TDES2_IVT_LBIT_POS, ptr, data); \
} while(0)



#define TX_CONTEXT_DESC_TDES2_SVT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES2_SVT_HBIT_POS, TX_CONTEXT_DESC_TDES2_SVT_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES2_SVT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES2_SVT_HBIT_POS, TX_CONTEXT_DESC_TDES2_SVT_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES2_MSS_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES2_MSS_HBIT_POS, TX_CONTEXT_DESC_TDES2_MSS_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES2_MSS_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES2_MSS_HBIT_POS, TX_CONTEXT_DESC_TDES2_MSS_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES1_NDAP_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES1_NDAP_HBIT_POS, TX_CONTEXT_DESC_TDES1_NDAP_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES1_NDAP_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES1_NDAP_HBIT_POS, TX_CONTEXT_DESC_TDES1_NDAP_LBIT_POS, ptr, data); \
} while(0)


#define TX_CONTEXT_DESC_TDES0_TTSL_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_CONTEXT_DESC_TDES0_TTSL_HBIT_POS, TX_CONTEXT_DESC_TDES0_TTSL_LBIT_POS, ptr, data); \
} while(0)

#define TX_CONTEXT_DESC_TDES0_TTSL_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_CONTEXT_DESC_TDES0_TTSL_HBIT_POS, TX_CONTEXT_DESC_TDES0_TTSL_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_OWN_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_OWN_HBIT_POS, RX_NORMAL_DESC_RDES3_OWN_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_OWN_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_OWN_HBIT_POS, RX_NORMAL_DESC_RDES3_OWN_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_CTXT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_CTXT_HBIT_POS, RX_NORMAL_DESC_RDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_CTXT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_CTXT_HBIT_POS, RX_NORMAL_DESC_RDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_FD_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_FD_HBIT_POS, RX_NORMAL_DESC_RDES3_FD_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_FD_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_FD_HBIT_POS, RX_NORMAL_DESC_RDES3_FD_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_LD_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_LD_HBIT_POS, RX_NORMAL_DESC_RDES3_LD_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_LD_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_LD_HBIT_POS, RX_NORMAL_DESC_RDES3_LD_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_RS2V_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_RS2V_HBIT_POS, RX_NORMAL_DESC_RDES3_RS2V_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_RS2V_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_RS2V_HBIT_POS, RX_NORMAL_DESC_RDES3_RS2V_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_RS1V_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_RS1V_HBIT_POS, RX_NORMAL_DESC_RDES3_RS1V_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_RS1V_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_RS1V_HBIT_POS, RX_NORMAL_DESC_RDES3_RS1V_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_RS0V_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_RS0V_HBIT_POS, RX_NORMAL_DESC_RDES3_RS0V_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_RS0V_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_RS0V_HBIT_POS, RX_NORMAL_DESC_RDES3_RS0V_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_CE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_CE_HBIT_POS, RX_NORMAL_DESC_RDES3_CE_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_CE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_CE_HBIT_POS, RX_NORMAL_DESC_RDES3_CE_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_GP_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_GP_HBIT_POS, RX_NORMAL_DESC_RDES3_GP_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_GP_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_GP_HBIT_POS, RX_NORMAL_DESC_RDES3_GP_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_RWT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_RWT_HBIT_POS, RX_NORMAL_DESC_RDES3_RWT_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_RWT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_RWT_HBIT_POS, RX_NORMAL_DESC_RDES3_RWT_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_OE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_OE_HBIT_POS, RX_NORMAL_DESC_RDES3_OE_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_OE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_OE_HBIT_POS, RX_NORMAL_DESC_RDES3_OE_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_RE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_RE_HBIT_POS, RX_NORMAL_DESC_RDES3_RE_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_RE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_RE_HBIT_POS, RX_NORMAL_DESC_RDES3_RE_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_DE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_DE_HBIT_POS, RX_NORMAL_DESC_RDES3_DE_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_DE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_DE_HBIT_POS, RX_NORMAL_DESC_RDES3_DE_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_LT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_LT_HBIT_POS, RX_NORMAL_DESC_RDES3_LT_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_LT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_LT_HBIT_POS, RX_NORMAL_DESC_RDES3_LT_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_ES_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_ES_HBIT_POS, RX_NORMAL_DESC_RDES3_ES_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_ES_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_ES_HBIT_POS, RX_NORMAL_DESC_RDES3_ES_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES3_FL_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES3_FL_HBIT_POS, RX_NORMAL_DESC_RDES3_FL_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES3_FL_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES3_FL_HBIT_POS, RX_NORMAL_DESC_RDES3_FL_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES2_B2AP_NDA_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES2_B2AP_NDA_HBIT_POS, RX_NORMAL_DESC_RDES2_B2AP_NDA_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES2_B2AP_NDA_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES2_B2AP_NDA_HBIT_POS, RX_NORMAL_DESC_RDES2_B2AP_NDA_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_COP_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_COP_HBIT_POS, RX_NORMAL_DESC_RDES1_COP_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_COP_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_COP_HBIT_POS, RX_NORMAL_DESC_RDES1_COP_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_TD_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_TD_HBIT_POS, RX_NORMAL_DESC_RDES1_TD_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_TD_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_TD_HBIT_POS, RX_NORMAL_DESC_RDES1_TD_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_TSA_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_TSA_HBIT_POS, RX_NORMAL_DESC_RDES1_TSA_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_TSA_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_TSA_HBIT_POS, RX_NORMAL_DESC_RDES1_TSA_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_PV_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_PV_HBIT_POS, RX_NORMAL_DESC_RDES1_PV_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_PV_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_PV_HBIT_POS, RX_NORMAL_DESC_RDES1_PV_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_PFT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_PFT_HBIT_POS, RX_NORMAL_DESC_RDES1_PFT_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_PFT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_PFT_HBIT_POS, RX_NORMAL_DESC_RDES1_PFT_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_PMT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_PMT_HBIT_POS, RX_NORMAL_DESC_RDES1_PMT_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_PMT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_PMT_HBIT_POS, RX_NORMAL_DESC_RDES1_PMT_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_IPPE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_IPPE_HBIT_POS, RX_NORMAL_DESC_RDES1_IPPE_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_IPPE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_IPPE_HBIT_POS, RX_NORMAL_DESC_RDES1_IPPE_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_IPCB_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_IPCB_HBIT_POS, RX_NORMAL_DESC_RDES1_IPCB_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_IPCB_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_IPCB_HBIT_POS, RX_NORMAL_DESC_RDES1_IPCB_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_IPV6_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_IPV6_HBIT_POS, RX_NORMAL_DESC_RDES1_IPV6_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_IPV6_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_IPV6_HBIT_POS, RX_NORMAL_DESC_RDES1_IPV6_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_IPV4_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_IPV4_HBIT_POS, RX_NORMAL_DESC_RDES1_IPV4_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_IPV4_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_IPV4_HBIT_POS, RX_NORMAL_DESC_RDES1_IPV4_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_IPHE_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_IPHE_HBIT_POS, RX_NORMAL_DESC_RDES1_IPHE_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_IPHE_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_IPHE_HBIT_POS, RX_NORMAL_DESC_RDES1_IPHE_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES1_PT_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES1_PT_HBIT_POS, RX_NORMAL_DESC_RDES1_PT_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES1_PT_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES1_PT_HBIT_POS, RX_NORMAL_DESC_RDES1_PT_LBIT_POS, ptr, data); \
} while(0)


#define RX_NORMAL_DESC_RDES0_HDR_B1AP_Mlf_Rd(ptr, data) do { \
  GET_BITS(RX_NORMAL_DESC_RDES0_HDR_B1AP_HBIT_POS, RX_NORMAL_DESC_RDES0_HDR_B1AP_LBIT_POS, ptr, data); \
} while(0)

#define RX_NORMAL_DESC_RDES0_HDR_B1AP_Mlf_Wr(ptr, data) do { \
  SET_BITS(RX_NORMAL_DESC_RDES0_HDR_B1AP_HBIT_POS, RX_NORMAL_DESC_RDES0_HDR_B1AP_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_OWN_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_OWN_HBIT_POS, TX_NORMAL_DESC_TDES3_OWN_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_OWN_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_OWN_HBIT_POS, TX_NORMAL_DESC_TDES3_OWN_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_CTXT_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_CTXT_HBIT_POS, TX_NORMAL_DESC_TDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_CTXT_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_CTXT_HBIT_POS, TX_NORMAL_DESC_TDES3_CTXT_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_FD_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_FD_HBIT_POS, TX_NORMAL_DESC_TDES3_FD_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_FD_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_FD_HBIT_POS, TX_NORMAL_DESC_TDES3_FD_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_LD_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_LD_HBIT_POS, TX_NORMAL_DESC_TDES3_LD_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_LD_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_LD_HBIT_POS, TX_NORMAL_DESC_TDES3_LD_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_CPC_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_CPC_HBIT_POS, TX_NORMAL_DESC_TDES3_CPC_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_CPC_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_CPC_HBIT_POS, TX_NORMAL_DESC_TDES3_CPC_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_SAIC_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_SAIC_HBIT_POS, TX_NORMAL_DESC_TDES3_SAIC_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_SAIC_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_SAIC_HBIT_POS, TX_NORMAL_DESC_TDES3_SAIC_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_HBIT_POS, TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_HBIT_POS, TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_TSE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_TSE_HBIT_POS, TX_NORMAL_DESC_TDES3_TSE_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_TSE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_TSE_HBIT_POS, TX_NORMAL_DESC_TDES3_TSE_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_CIC_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_CIC_HBIT_POS, TX_NORMAL_DESC_TDES3_CIC_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_CIC_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_CIC_HBIT_POS, TX_NORMAL_DESC_TDES3_CIC_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_TIPLH_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_TIPLH_HBIT_POS, TX_NORMAL_DESC_TDES3_TIPLH_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_TIPLH_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_TIPLH_HBIT_POS, TX_NORMAL_DESC_TDES3_TIPLH_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES3_FL_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES3_FL_HBIT_POS, TX_NORMAL_DESC_TDES3_FL_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES3_FL_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES3_FL_HBIT_POS, TX_NORMAL_DESC_TDES3_FL_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES2_IC_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES2_IC_HBIT_POS, TX_NORMAL_DESC_TDES2_IC_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES2_IC_HBIT_POS, TX_NORMAL_DESC_TDES2_IC_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES2_TTSE_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES2_TTSE_HBIT_POS, TX_NORMAL_DESC_TDES2_TTSE_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_TTSE_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES2_TTSE_HBIT_POS, TX_NORMAL_DESC_TDES2_TTSE_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES2_B2L_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES2_B2L_HBIT_POS, TX_NORMAL_DESC_TDES2_B2L_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_B2L_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES2_B2L_HBIT_POS, TX_NORMAL_DESC_TDES2_B2L_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES2_VTIR_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES2_VTIR_HBIT_POS, TX_NORMAL_DESC_TDES2_VTIR_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_VTIR_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES2_VTIR_HBIT_POS, TX_NORMAL_DESC_TDES2_VTIR_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES2_HL_B1L_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES2_HL_B1L_HBIT_POS, TX_NORMAL_DESC_TDES2_HL_B1L_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_HL_B1L_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES2_HL_B1L_HBIT_POS, TX_NORMAL_DESC_TDES2_HL_B1L_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES1_B2A_NDA_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES1_B2A_NDA_HBIT_POS, TX_NORMAL_DESC_TDES1_B2A_NDA_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES1_B2A_NDA_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES1_B2A_NDA_HBIT_POS, TX_NORMAL_DESC_TDES1_B2A_NDA_LBIT_POS, ptr, data); \
} while(0)


#define TX_NORMAL_DESC_TDES0_B1A_HAP_Mlf_Rd(ptr, data) do { \
  GET_BITS(TX_NORMAL_DESC_TDES0_B1A_HAP_HBIT_POS, TX_NORMAL_DESC_TDES0_B1A_HAP_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES0_B1A_HAP_Mlf_Wr(ptr, data) do { \
  SET_BITS(TX_NORMAL_DESC_TDES0_B1A_HAP_HBIT_POS, TX_NORMAL_DESC_TDES0_B1A_HAP_LBIT_POS, ptr, data); \
} while(0)

/* virtual_registers Register Read-Write Macros */
#define RX_CONTEXT_DESC_RDES3_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_CONTEXT_DESC_RDES3_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_CONTEXT_DESC_RDES2_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_CONTEXT_DESC_RDES2_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_CONTEXT_DESC_RDES1_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_CONTEXT_DESC_RDES1_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_CONTEXT_DESC_RDES0_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_CONTEXT_DESC_RDES0_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_CONTEXT_DESC_TDES3_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_CONTEXT_DESC_TDES3_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_CONTEXT_DESC_TDES2_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_CONTEXT_DESC_TDES2_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_CONTEXT_DESC_TDES1_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_CONTEXT_DESC_TDES1_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_CONTEXT_DESC_TDES0_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_CONTEXT_DESC_TDES0_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_RDES3_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_RDES3_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_RDES2_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_RDES2_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_RDES1_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_RDES1_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_RDES0_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_RDES0_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_TX_ERRORS_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_TX_ERRORS_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_RX_BUF_PTR_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_RX_BUF_PTR_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_NEXT_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_NEXT_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define RX_NORMAL_DESC_SKB_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define RX_NORMAL_DESC_SKB_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_NORMAL_DESC_TDES3_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_NORMAL_DESC_TDES3_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_NORMAL_DESC_TDES2_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_NORMAL_DESC_TDES2_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_NORMAL_DESC_TDES1_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_NORMAL_DESC_TDES1_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_NORMAL_DESC_TDES0_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_NORMAL_DESC_TDES0_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_NORMAL_DESC_TX_BUF_PTR_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_NORMAL_DESC_TX_BUF_PTR_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)


#define TX_NORMAL_DESC_NEXT_Ml_Rd(ptr, data) do { \
	data = ptr; \
} while(0)

#define TX_NORMAL_DESC_NEXT_Ml_Wr(ptr, data) do { \
	ptr = data; \
} while(0)

/******************************************************************************/
/**                        Neutrino DMA Descriptor                           **/
/******************************************************************************/
/* virtual_registers Low Bit Macro Name's */
#define TX_NORMAL_DESC_TDES2_TMODE_LBIT_POS  0x15
#define TX_NORMAL_DESC_TDES2_BUFHA_LBIT_POS  16
#define TX_NORMAL_DESC_TDES1_LAUNCHTIME_LBIT_POS  0

/* virtual_registers High Bit Macro Name's */
#define TX_NORMAL_DESC_TDES2_TMODE_HBIT_POS  0x16
#define TX_NORMAL_DESC_TDES2_BUFHA_HBIT_POS  20
#define TX_NORMAL_DESC_TDES1_LAUNCHTIME_HBIT_POS  0x1f

/* virtual_registers Register-Field Read-Write Macros */
#define TX_NORMAL_DESC_TDES2_TMODE_Mlf_Rd(ptr, data) do { \
    GET_BITS(TX_NORMAL_DESC_TDES2_TMODE_HBIT_POS, TX_NORMAL_DESC_TDES2_TMODE_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_BUFHA_Mlf_Rd(ptr, data) do { \
    GET_BITS(TX_NORMAL_DESC_TDES2_BUFHA_HBIT_POS, TX_NORMAL_DESC_TDES2_BUFHA_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES1_LAUNCHTIME_Mlf_Rd(ptr, data) do { \
    GET_BITS(TX_NORMAL_DESC_TDES1_LAUNCHTIME_HBIT_POS, TX_NORMAL_DESC_TDES1_LAUNCHTIME_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_TMODE_Mlf_Wr(ptr, data) do { \
    SET_BITS(TX_NORMAL_DESC_TDES2_TMODE_HBIT_POS, TX_NORMAL_DESC_TDES2_TMODE_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES2_BUFHA_Mlf_Wr(ptr, data) do { \
    SET_BITS(TX_NORMAL_DESC_TDES2_BUFHA_HBIT_POS, TX_NORMAL_DESC_TDES2_BUFHA_LBIT_POS, ptr, data); \
} while(0)

#define TX_NORMAL_DESC_TDES1_LAUNCHTIME_Mlf_Wr(ptr, data) do { \
    SET_BITS(TX_NORMAL_DESC_TDES1_LAUNCHTIME_HBIT_POS, TX_NORMAL_DESC_TDES1_LAUNCHTIME_LBIT_POS, ptr, data); \
} while(0)

/* virtual_registers Register-Field value Macros */
#define	TX_NORMAL_DESC_TDES2_TMODE_IGNORE			0
#define	TX_NORMAL_DESC_TDES2_TMODE_EMBEDDED_IN_PKT	1
#define	TX_NORMAL_DESC_TDES2_TMODE_TDES1_VAL1		2
#define	TX_NORMAL_DESC_TDES2_TMODE_TDES1_VAL2		3



/******************************************************************************/
/**                       Neutrino Wrapper DMA registers                     **/
/******************************************************************************/

#define DMA_BUSCFG_RgOffAddr ((volatile ULONG *)(DMA_BASE_ADDRESS))

#define DMA_BUSCFG_RgWr(data) do {\
        iowrite32(data, (void *)DMA_BUSCFG_RgOffAddr);\
} while(0)

#define DMA_BUSCFG_RgRd(data) do {\
        (data) = ioread32((void *)DMA_BUSCFG_RgOffAddr);\
} while(0)

/*#define DMA_BUSCFG_WR_OSR_LMT_Mask (ULONG)(~(~0<<(5)))*/
#define DMA_BUSCFG_WR_OSR_LMT_Mask (ULONG)(0x1f)
/*#define DMA_BUSCFG_WR_OSR_LMT_Wr_Mask (ULONG)(~((~(~0 << (5))) << (24)))*/
#define DMA_BUSCFG_WR_OSR_LMT_Wr_Mask (ULONG)(0xe0ffffff)

#define DMA_BUSCFG_WR_OSR_LMT_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_WR_OSR_LMT_Wr_Mask) | ((data & DMA_BUSCFG_WR_OSR_LMT_Mask)<<24));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_WR_OSR_LMT_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 24) & DMA_BUSCFG_WR_OSR_LMT_Mask);\
} while(0)

/*#define DMA_BUSCFG_RD_OSR_LMT_Mask (ULONG)(~(~0<<(5)))*/
#define DMA_BUSCFG_RD_OSR_LMT_Mask (ULONG)(0x1f)
/*#define DMA_BUSCFG_RD_OSR_LMT_Wr_Mask (ULONG)(~((~(~0 << (5))) << (16)))*/
#define DMA_BUSCFG_RD_OSR_LMT_Wr_Mask (ULONG)(0xffe0ffff)

#define DMA_BUSCFG_RD_OSR_LMT_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_RD_OSR_LMT_Wr_Mask) | ((data & DMA_BUSCFG_RD_OSR_LMT_Mask)<<16));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_RD_OSR_LMT_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 16) & DMA_BUSCFG_RD_OSR_LMT_Mask);\
} while(0)

/*#define DMA_BUSCFG_WCHSTS_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_BUSCFG_WCHSTS_Mask (ULONG)(0x1)
/*#define DMA_BUSCFG_WCHSTS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/
#define DMA_BUSCFG_WCHSTS_Wr_Mask (ULONG)(0xfffffdff)

#define DMA_BUSCFG_WCHSTS_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_WCHSTS_Wr_Mask) | ((data & DMA_BUSCFG_WCHSTS_Mask)<<9));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_WCHSTS_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 9) & DMA_BUSCFG_WCHSTS_Mask);\
} while(0)


/*#define DMA_BUSCFG_RCHSTS_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_BUSCFG_RCHSTS_Mask (ULONG)(0x1)
/*#define DMA_BUSCFG_RCHSTS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (8)))*/
#define DMA_BUSCFG_RCHSTS_Wr_Mask (ULONG)(0xfffffeff)

#define DMA_BUSCFG_RCHSTS_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_RCHSTS_Wr_Mask) | ((data & DMA_BUSCFG_RCHSTS_Mask)<<8));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_RCHSTS_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 8) & DMA_BUSCFG_RCHSTS_Mask);\
} while(0)


/*#define DMA_BUSCFG_BLEN16_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_BUSCFG_BLEN16_Mask (ULONG)(0x1)
/*#define DMA_BUSCFG_BLEN16_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/
#define DMA_BUSCFG_BLEN16_Wr_Mask (ULONG)(0xfffffff7)

#define DMA_BUSCFG_BLEN16_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_BLEN16_Wr_Mask) | ((data & DMA_BUSCFG_BLEN16_Mask)<<3));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_BLEN16_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 3) & DMA_BUSCFG_BLEN16_Mask);\
} while(0)

/*#define DMA_BUSCFG_BLEN8_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_BUSCFG_BLEN8_Mask (ULONG)(0x1)
/*#define DMA_BUSCFG_BLEN8_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/
#define DMA_BUSCFG_BLEN8_Wr_Mask (ULONG)(0xfffffffb)

#define DMA_BUSCFG_BLEN8_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_BLEN8_Wr_Mask) | ((data & DMA_BUSCFG_BLEN8_Mask)<<2));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_BLEN8_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 2) & DMA_BUSCFG_BLEN8_Mask);\
} while(0)

/*#define DMA_BUSCFG_BLEN4_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_BUSCFG_BLEN4_Mask (ULONG)(0x1)
/*#define DMA_BUSCFG_BLEN4_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/
#define DMA_BUSCFG_BLEN4_Wr_Mask (ULONG)(0xfffffffd)

#define DMA_BUSCFG_BLEN4_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_BLEN4_Wr_Mask) | ((data & DMA_BUSCFG_BLEN4_Mask)<<1));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_BLEN4_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 1) & DMA_BUSCFG_BLEN4_Mask);\
} while(0)


/*#define DMA_BUSCFG_FB_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_BUSCFG_FB_Mask (ULONG)(0x1)
/*#define DMA_BUSCFG_FB_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/
#define DMA_BUSCFG_FB_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_BUSCFG_FB_UdfWr(data) do {\
        ULONG v;\
        DMA_BUSCFG_RgRd(v);\
        v = ((v & DMA_BUSCFG_FB_Wr_Mask) | ((data & DMA_BUSCFG_FB_Mask)<<0));\
        DMA_BUSCFG_RgWr(v);\
} while(0)

#define DMA_BUSCFG_FB_UdfRd(data) do {\
        DMA_BUSCFG_RgRd(data);\
        data = ((data >> 0) & DMA_BUSCFG_FB_Mask);\
} while(0)

#define DMA_TXCHSTS_RgOffAddr(n)                    ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x00 ))
#define DMA_TXCHINTMASK_RgOffAddr(n)                ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x04 ))
#define DMA_TXCHCTL_RgOffAddr(n)                    ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x08 ))
#define DMA_TXCH_DESC_LISTHADDR_RgOffAddr(n)        ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x0C ))
#define DMA_TXCH_DESC_LISTLADDR_RgOffAddr(n)        ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x10 ))
#define DMA_TXCH_DESC_TAILPTR_RgOffAddr(n)          ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x14 ))
#define DMA_TXCH_DESC_TAILPTR_RgOffAddr_Phy(n)      ((volatile ULONG *)(DMA_TX_CH_PHY_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x14 ))
#define DMA_TXCH_DESC_RING_LENGTH_RgOffAddr(n)      ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x18 ))
#define DMA_TXCH_CUR_DESC_RgOffAddr(n)              ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x1C ))
#define DMA_TXCH_CUR_BUFHA_RgOffAddr(n)             ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x20 ))
#define DMA_TXCH_CUR_BUFLA_RgOffAddr(n)             ((volatile ULONG *)(DMA_TX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x24 ))

#define DMA_RXCHSTS_RgOffAddr(n)                    ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x00 ))
#define DMA_RXCHINTMASK_RgOffAddr(n)                ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x04 ))
#define DMA_RXCHCTL_RgOffAddr(n)                    ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x08 ))
#define DMA_RXCH_DESC_LISTHADDR_RgOffAddr(n)        ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x0C ))
#define DMA_RXCH_DESC_LISTLADDR_RgOffAddr(n)        ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x10 ))
#define DMA_RXCH_DESC_TAILPTR_RgOffAddr(n)          ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x14 ))
#define DMA_RXCH_DESC_TAILPTR_RgOffAddr_Phy(n)      ((volatile ULONG *)(DMA_RX_CH_PHY_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x14 ))
#define DMA_RXCH_DESC_RING_LENGTH_RgOffAddr(n)      ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x18 ))
#define DMA_RXCH_CUR_DESC_RgOffAddr(n)              ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x1C ))
#define DMA_RXCH_CUR_BUFHA_RgOffAddr(n)             ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x20 ))
#define DMA_RXCH_CUR_BUFLA_RgOffAddr(n)             ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x24 ))
#define DMA_RXCH_CUR_WATCHDOG_TIMER_RgOffAddr(n)    ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x28 ))
#define DMA_RXCH_MISS_COUNTER_RgOffAddr(n)          ((volatile ULONG *)(DMA_RX_CH_BASE_ADDRESS + (n * DMA_INTER_CH_OFFSET) + 0x2C ))


#define DMA_TXCHSTS_TC_LPOS 0
#define DMA_TXCHSTS_TC_HPOS 0
#define DMA_TXCHSTS_TS_LPOS 1
#define DMA_TXCHSTS_TS_HPOS 1
#define DMA_TXCHSTS_UNF_LPOS 2
#define DMA_TXCHSTS_UNF_HPOS 2
#define DMA_TXCHSTS_ETC_LPOS 3
#define DMA_TXCHSTS_ETC_HPOS 3
#define DMA_TXCHSTS_FE_LPOS 4
#define DMA_TXCHSTS_FE_HPOS 4
#define DMA_TXCHSTS_FESTS_LPOS 5
#define DMA_TXCHSTS_FESTS_HPOS 7

#define DMA_TXCHSTS_CHSTS_LPOS 8
#define DMA_TXCHSTS_CHSTS_HPOS 10
#define DMA_TXCHSTS_CDE_LPOS 11
#define DMA_TXCHSTS_CDE_HPOS 11

#define DMA_TXCHSTS_CHSTS_STOP    	0
#define DMA_TXCHSTS_CHSTS_SUSPEND 	5

#define DMA_TXCHSTS_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCHSTS_RgOffAddr(n));   }
#define DMA_TXCHSTS_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCHSTS_RgOffAddr(n)); }

/*#define DMA_TXCHSTS_TC_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHSTS_TC_Mask (ULONG)(0x1)
/*#define DMA_TXCHSTS_TC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/
#define DMA_TXCHSTS_TC_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_TXCHSTS_TC_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_TC_Wr_Mask) | ((data & DMA_TXCHSTS_TC_Mask)<<0));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_TC_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 0) & DMA_TXCHSTS_TC_Mask);\
} while(0)

/*#define DMA_TXCHSTS_TS_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHSTS_TS_Mask (ULONG)(0x1)
/*#define DMA_TXCHSTS_TS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/
#define DMA_TXCHSTS_TS_Wr_Mask (ULONG)(0xfffffffd)

#define DMA_TXCHSTS_TS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_TS_Wr_Mask) | ((data & DMA_TXCHSTS_TS_Mask)<<1));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_TS_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 1) & DMA_TXCHSTS_TS_Mask);\
} while(0)

/*#define DMA_TXCHSTS_UNF_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHSTS_UNF_Mask (ULONG)(0x1)
/*#define DMA_TXCHSTS_UNF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/
#define DMA_TXCHSTS_UNF_Wr_Mask (ULONG)(0xfffffffb)

#define DMA_TXCHSTS_UNF_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_UNF_Wr_Mask) | ((data & DMA_TXCHSTS_UNF_Mask)<<2));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_UNF_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 2) & DMA_TXCHSTS_UNF_Mask);\
} while(0)

/*#define DMA_TXCHSTS_ETC_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHSTS_ETC_Mask (ULONG)(0x1)
/*#define DMA_TXCHSTS_ETC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (3)))*/
#define DMA_TXCHSTS_ETC_Wr_Mask (ULONG)(0xfffffff7)

#define DMA_TXCHSTS_ETC_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_ETC_Wr_Mask) | ((data & DMA_TXCHSTS_ETC_Mask)<<3));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_ETC_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 3) & DMA_TXCHSTS_ETC_Mask);\
} while(0)

/*#define DMA_TXCHSTS_FE_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHSTS_FE_Mask (ULONG)(0x1)
/*#define DMA_TXCHSTS_FE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/
#define DMA_TXCHSTS_FE_Wr_Mask (ULONG)(0xffffffef)

#define DMA_TXCHSTS_FE_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_FE_Wr_Mask) | ((data & DMA_TXCHSTS_FE_Mask)<<4));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_FE_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 4) & DMA_TXCHSTS_FE_Mask);\
} while(0)

/*#define DMA_TXCHSTS_FESTS_Mask (ULONG)(~(~0<<(3)))*/
#define DMA_TXCHSTS_FESTS_Mask (ULONG)(0x7)
/*#define DMA_TXCHSTS_FESTS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (5)))*/
#define DMA_TXCHSTS_FESTS_Wr_Mask (ULONG)(0xffffff1f)

#define DMA_TXCHSTS_FESTS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_FESTS_Wr_Mask) | ((data & DMA_TXCHSTS_FESTS_Mask)<<5));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_FESTS_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 5) & DMA_TXCHSTS_FESTS_Mask);\
} while(0)


/*#define DMA_TXCHSTS_CDE_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHSTS_CDE_Mask (ULONG)(0x1)
/*#define DMA_TXCHSTS_CDE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (9)))*/
#define DMA_TXCHSTS_CDE_Wr_Mask (ULONG)(0xfffff7ff)

#define DMA_TXCHSTS_CDE_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_CDE_Wr_Mask) | ((data & DMA_TXCHSTS_CDE_Mask)<<11));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_CDE_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 11) & DMA_TXCHSTS_CDE_Mask);\
} while(0)

/*#define DMA_TXCHSTS_CHSTS_Mask (ULONG)(~(~0<<(2)))*/
#define DMA_TXCHSTS_CHSTS_Mask (ULONG)(0x7)
/*#define DMA_TXCHSTS_CHSTS_Wr_Mask (ULONG)(~((~(~0 << (2))) << (10)))*/
#define DMA_TXCHSTS_CHSTS_Wr_Mask (ULONG)(0xfffff8ff)

#define DMA_TXCHSTS_CHSTS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHSTS_RgRd(n, v);\
        v = ((v & DMA_TXCHSTS_CHSTS_Wr_Mask) | ((data & DMA_TXCHSTS_CHSTS_Mask)<<8));\
        DMA_TXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_TXCHSTS_CHSTS_UdfRd(n, data) do {\
        DMA_TXCHSTS_RgRd(n, data);\
        data = ((data >> 8) & DMA_TXCHSTS_CHSTS_Mask);\
} while(0)



#define DMA_TXCHCTL_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCHCTL_RgOffAddr(n));   }
#define DMA_TXCHCTL_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCHCTL_RgOffAddr(n)); }

/*#define DMA_TXCHCTL_ST_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHCTL_ST_Mask (ULONG)(0x1)
/*#define DMA_TXCHCTL_ST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/
#define DMA_TXCHCTL_ST_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_TXCHCTL_ST_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHCTL_RgRd(n, v);\
        v = ((v & DMA_TXCHCTL_ST_Wr_Mask) | ((data & DMA_TXCHCTL_ST_Mask)<<0));\
        DMA_TXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_TXCHCTL_ST_UdfRd(n, data) do {\
        DMA_TXCHCTL_RgRd(n, data);\
        data = ((data >> 0) & DMA_TXCHCTL_ST_Mask);\
} while(0)

/*#define DMA_TXCHCTL_OSP_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHCTL_OSP_Mask (ULONG)(0x1)
/*#define DMA_TXCHCTL_OSP_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/
#define DMA_TXCHCTL_OSP_Wr_Mask (ULONG)(0xffffffef)

#define DMA_TXCHCTL_OSP_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHCTL_RgRd(n, v);\
        v = ((v & DMA_TXCHCTL_OSP_Wr_Mask) | ((data & DMA_TXCHCTL_OSP_Mask)<<4));\
        DMA_TXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_TXCHCTL_OSP_UdfRd(n, data) do {\
        DMA_TXCHCTL_RgRd(n, data);\
        data = ((data >> 4) & DMA_TXCHCTL_OSP_Mask);\
} while(0)

/*#define DMA_TXCHCTL_IPBL_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_TXCHCTL_IPBL_Mask (ULONG)(0x1)
/*#define DMA_TXCHCTL_IPBL_Wr_Mask (ULONG)(~((~(~0 << (1))) << (15)))*/
#define DMA_TXCHCTL_IPBL_Wr_Mask (ULONG)(0xffff7fff)

#define DMA_TXCHCTL_IPBL_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHCTL_RgRd(n, v);\
        v = ((v & DMA_TXCHCTL_IPBL_Wr_Mask) | ((data & DMA_TXCHCTL_IPBL_Mask)<<15));\
        DMA_TXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_TXCHCTL_IPBL_UdfRd(n, data) do {\
        DMA_TXCHCTL_RgRd(n, data);\
        data = ((data >> 15) & DMA_TXCHCTL_IPBL_Mask);\
} while(0)

/*#define DMA_TXCHCTL_TXPBL_Mask (ULONG)(~(~0<<(6)))*/
#define DMA_TXCHCTL_TXPBL_Mask (ULONG)(0x3f)
/*#define DMA_TXCHCTL_TXPBL_Wr_Mask (ULONG)(~((~(~0 << (6))) << (16)))*/
#define DMA_TXCHCTL_TXPBL_Wr_Mask (ULONG)(0xffc0ffff)

#define DMA_TXCHCTL_TXPBL_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHCTL_RgRd(n, v);\
        v = ((v & DMA_TXCHCTL_TXPBL_Wr_Mask) | ((data & DMA_TXCHCTL_TXPBL_Mask)<<16));\
        DMA_TXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_TXCHCTL_TXPBL_UdfRd(n, data) do {\
        DMA_TXCHCTL_RgRd(n, data);\
        data = ((data >> 16) & DMA_TXCHCTL_TXPBL_Mask);\
} while(0)


#define DMA_TXCH_DESC_LISTHADDR_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_DESC_LISTHADDR_RgOffAddr(n));   }
#define DMA_TXCH_DESC_LISTHADDR_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_DESC_LISTHADDR_RgOffAddr(n)); }

#define DMA_TXCH_DESC_LISTLADDR_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_DESC_LISTLADDR_RgOffAddr(n));   }
#define DMA_TXCH_DESC_LISTLADDR_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_DESC_LISTLADDR_RgOffAddr(n)); }

#define DMA_TXCH_DESC_TAILPTR_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_DESC_TAILPTR_RgOffAddr(n));   }
#define DMA_TXCH_DESC_TAILPTR_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_DESC_TAILPTR_RgOffAddr(n)); }

#define DMA_TXCH_DESC_RING_LENGTH_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_DESC_RING_LENGTH_RgOffAddr(n));   }
#define DMA_TXCH_DESC_RING_LENGTH_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_DESC_RING_LENGTH_RgOffAddr(n)); }

#define DMA_TXCHINTMASK_MASK    (0x813)

#define DMA_TXCHINTMASK_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCHINTMASK_RgOffAddr(n));   }
#define DMA_TXCHINTMASK_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCHINTMASK_RgOffAddr(n)); }


#define DMA_TXCHINTMASK_TCEN_Mask (ULONG)(0x1)
#define DMA_TXCHINTMASK_TCEN_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_TXCHINTMASK_TCEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_TXCHINTMASK_TCEN_Wr_Mask) | ((data & DMA_TXCHINTMASK_TCEN_Mask)<<0));\
        DMA_TXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_TXCHINTMASK_TCEN_UdfRd(n, data) do {\
        DMA_TXCHINTMASK_RgRd(n, data);\
        data = ((data >> 0) & DMA_TXCHINTMASK_TCEN_Mask);\
} while(0)


#define DMA_TXCHINTMASK_TSEN_Mask (ULONG)(0x1)
#define DMA_TXCHINTMASK_TSEN_Wr_Mask (ULONG)(0xfffffffd)

#define DMA_TXCHINTMASK_TSEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_TXCHINTMASK_TSEN_Wr_Mask) | ((data & DMA_TXCHINTMASK_TSEN_Mask)<<1));\
        DMA_TXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_TXCHINTMASK_TSEN_UdfRd(n, data) do {\
        DMA_TXCHINTMASK_RgRd(n, data);\
        data = ((data >> 1) & DMA_TXCHINTMASK_TSEN_Mask);\
} while(0)


#define DMA_TXCHINTMASK_UNFEN_Mask (ULONG)(0x1)
#define DMA_TXCHINTMASK_UNFEN_Wr_Mask (ULONG)(0xfffffffb)

#define DMA_TXCHINTMASK_UNFEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_TXCHINTMASK_UNFEN_Wr_Mask) | ((data & DMA_TXCHINTMASK_UNFEN_Mask)<<2));\
        DMA_TXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_TXCHINTMASK_UNFEN_UdfRd(n, data) do {\
        DMA_TXCHINTMASK_RgRd(n, data);\
        data = ((data >> 2) & DMA_TXCHINTMASK_UNFEN_Mask);\
} while(0)


#define DMA_TXCHINTMASK_ETCEN_Mask (ULONG)(0x1)
#define DMA_TXCHINTMASK_ETCEN_Wr_Mask (ULONG)(0xfffffff7)

#define DMA_TXCHINTMASK_ETCEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_TXCHINTMASK_ETCEN_Wr_Mask) | ((data & DMA_TXCHINTMASK_ETCEN_Mask)<<3));\
        DMA_TXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_TXCHINTMASK_ETCEN_UdfRd(n, data) do {\
        DMA_TXCHINTMASK_RgRd(n, data);\
        data = ((data >> 3) & DMA_TXCHINTMASK_ETCEN_Mask);\
} while(0)


#define DMA_TXCHINTMASK_FEEN_Mask (ULONG)(0x1)
#define DMA_TXCHINTMASK_FEEN_Wr_Mask (ULONG)(0xffffffef)

#define DMA_TXCHINTMASK_FEEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_TXCHINTMASK_FEEN_Wr_Mask) | ((data & DMA_TXCHINTMASK_FEEN_Mask)<<4));\
        DMA_TXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_TXCHINTMASK_FEEN_UdfRd(n, data) do {\
        DMA_TXCHINTMASK_RgRd(n, data);\
        data = ((data >> 4) & DMA_TXCHINTMASK_FEEN_Mask);\
} while(0)


#define DMA_TXCHINTMASK_CDEE_Mask (ULONG)(0x1)
#define DMA_TXCHINTMASK_CDEE_Wr_Mask (ULONG)(0xfffffeff)

#define DMA_TXCHINTMASK_CDEE_UdfWr(n, data) do {\
        ULONG v;\
        DMA_TXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_TXCHINTMASK_CDEE_Wr_Mask) | ((data & DMA_TXCHINTMASK_CDEE_Mask)<<8));\
        DMA_TXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_TXCHINTMASK_CDEE_UdfRd(n, data) do {\
        DMA_TXCHINTMASK_RgRd(n, data);\
        data = ((data >> 8) & DMA_TXCHINTMASK_CDEE_Mask);\
} while(0)


#define DMA_TXCH_CUR_DESC_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_CUR_DESC_RgOffAddr(n));   }
#define DMA_TXCH_CUR_DESC_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_CUR_DESC_RgOffAddr(n)); }


#define DMA_TXCH_CUR_BUFHA_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_CUR_BUFHA_RgOffAddr(n));   }
#define DMA_TXCH_CUR_BUFHA_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_CUR_BUFHA_RgOffAddr(n)); }


#define DMA_TXCH_CUR_BUFLA_RgWr(n, data) { iowrite32(data, (void *)DMA_TXCH_CUR_BUFLA_RgOffAddr(n));   }
#define DMA_TXCH_CUR_BUFLA_RgRd(n, data) { (data) = ioread32((void *)DMA_TXCH_CUR_BUFLA_RgOffAddr(n)); }


#define DMA_RXCHSTS_RC_LPOS 0
#define DMA_RXCHSTS_RC_HPOS 0
#define DMA_RXCHSTS_RS_LPOS 1
#define DMA_RXCHSTS_RS_HPOS 1
#define DMA_RXCHSTS_UNF_LPOS 2
#define DMA_RXCHSTS_UNF_HPOS 2
#define DMA_RXCHSTS_FE_LPOS 4
#define DMA_RXCHSTS_FE_HPOS 4
#define DMA_RXCHSTS_FESTS_LPOS 5
#define DMA_RXCHSTS_FESTS_HPOS 6
#define DMA_RXCHSTS_CHSTS_LPOS 7
#define DMA_RXCHSTS_CHSTS_HPOS 9

#define DMA_RXCHSTS_CHSTS_STOP 		0
#define DMA_RXCHSTS_CHSTS_SUSPEND 	5

#define DMA_RXCHSTS_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCHSTS_RgOffAddr(n));   }
#define DMA_RXCHSTS_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCHSTS_RgOffAddr(n)); }

/*#define DMA_RXCHSTS_RC_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_RXCHSTS_RC_Mask (ULONG)(0x1)
/*#define DMA_RXCHSTS_RC_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/
#define DMA_RXCHSTS_RC_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_RXCHSTS_RC_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHSTS_RgRd(n, v);\
        v = ((v & DMA_RXCHSTS_RC_Wr_Mask) | ((data & DMA_RXCHSTS_RC_Mask)<<0));\
        DMA_RXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_RXCHSTS_RC_UdfRd(n, data) do {\
        DMA_RXCHSTS_RgRd(n, data);\
        data = ((data >> 0) & DMA_RXCHSTS_RC_Mask);\
} while(0)

/*#define DMA_RXCHSTS_RS_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_RXCHSTS_RS_Mask (ULONG)(0x1)
/*#define DMA_RXCHSTS_RS_Wr_Mask (ULONG)(~((~(~0 << (1))) << (1)))*/
#define DMA_RXCHSTS_RS_Wr_Mask (ULONG)(0xfffffffd)

#define DMA_RXCHSTS_RS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHSTS_RgRd(n, v);\
        v = ((v & DMA_RXCHSTS_RS_Wr_Mask) | ((data & DMA_RXCHSTS_RS_Mask)<<1));\
        DMA_RXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_RXCHSTS_RS_UdfRd(n, data) do {\
        DMA_RXCHSTS_RgRd(n, data);\
        data = ((data >> 1) & DMA_RXCHSTS_RS_Mask);\
} while(0)

/*#define DMA_RXCHSTS_UNF_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_RXCHSTS_UNF_Mask (ULONG)(0x1)
/*#define DMA_RXCHSTS_UNF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (2)))*/
#define DMA_RXCHSTS_UNF_Wr_Mask (ULONG)(0xfffffffb)

#define DMA_RXCHSTS_UNF_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHSTS_RgRd(n, v);\
        v = ((v & DMA_RXCHSTS_UNF_Wr_Mask) | ((data & DMA_RXCHSTS_UNF_Mask)<<2));\
        DMA_RXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_RXCHSTS_UNF_UdfRd(n, data) do {\
        DMA_RXCHSTS_RgRd(n, data);\
        data = ((data >> 2) & DMA_RXCHSTS_UNF_Mask);\
} while(0)


/*#define DMA_RXCHSTS_FE_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_RXCHSTS_FE_Mask (ULONG)(0x1)
/*#define DMA_RXCHSTS_FE_Wr_Mask (ULONG)(~((~(~0 << (1))) << (4)))*/
#define DMA_RXCHSTS_FE_Wr_Mask (ULONG)(0xffffffef)

#define DMA_RXCHSTS_FE_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHSTS_RgRd(n, v);\
        v = ((v & DMA_RXCHSTS_FE_Wr_Mask) | ((data & DMA_RXCHSTS_FE_Mask)<<4));\
        DMA_RXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_RXCHSTS_FE_UdfRd(n, data) do {\
        DMA_RXCHSTS_RgRd(n, data);\
        data = ((data >> 4) & DMA_RXCHSTS_FE_Mask);\
} while(0)


/*#define DMA_RXCHSTS_FESTS_Mask (ULONG)(~(~0<<(3)))*/
#define DMA_RXCHSTS_FESTS_Mask (ULONG)(0x7)
/*#define DMA_RXCHSTS_FESTS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (5)))*/
#define DMA_RXCHSTS_FESTS_Wr_Mask (ULONG)(0xffffff1f)

#define DMA_RXCHSTS_FESTS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHSTS_RgRd(n, v);\
        v = ((v & DMA_RXCHSTS_FESTS_Wr_Mask) | ((data & DMA_RXCHSTS_FESTS_Mask)<<5));\
        DMA_RXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_RXCHSTS_FESTS_UdfRd(n, data) do {\
        DMA_RXCHSTS_RgRd(n, data);\
        data = ((data >> 5) & DMA_RXCHSTS_FESTS_Mask);\
} while(0)


/*#define DMA_RXCHSTS_CHSTS_Mask (ULONG)(~(~0<<(3)))*/
#define DMA_RXCHSTS_CHSTS_Mask (ULONG)(0x7)
/*#define DMA_RXCHSTS_CHSTS_Wr_Mask (ULONG)(~((~(~0 << (3))) << (8)))*/
#define DMA_RXCHSTS_CHSTS_Wr_Mask (ULONG)(0xfffff8ff)

#define DMA_RXCHSTS_CHSTS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHSTS_RgRd(n, v);\
        v = ((v & DMA_RXCHSTS_CHSTS_Wr_Mask) | ((data & DMA_RXCHSTS_CHSTS_Mask)<<8));\
        DMA_RXCHSTS_RgWr(n, v);\
} while(0)

#define DMA_RXCHSTS_CHSTS_UdfRd(n, data) do {\
        DMA_RXCHSTS_RgRd(n, data);\
        data = ((data >> 8) & DMA_RXCHSTS_CHSTS_Mask);\
} while(0)




#define DMA_RXCHCTL_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCHCTL_RgOffAddr(n));   }
#define DMA_RXCHCTL_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCHCTL_RgOffAddr(n)); }

/*#define DMA_RXCHCTL_ST_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_RXCHCTL_ST_Mask (ULONG)(0x1)
/*#define DMA_RXCHCTL_ST_Wr_Mask (ULONG)(~((~(~0 << (1))) << (0)))*/
#define DMA_RXCHCTL_ST_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_RXCHCTL_ST_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHCTL_RgRd(n, v);\
        v = ((v & DMA_RXCHCTL_ST_Wr_Mask) | ((data & DMA_RXCHCTL_ST_Mask)<<0));\
        DMA_RXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_RXCHCTL_ST_UdfRd(n, data) do {\
        DMA_RXCHCTL_RgRd(n, data);\
        data = ((data >> 0) & DMA_RXCHCTL_ST_Mask);\
} while(0)


/*#define DMA_RXCHCTL_RBSZ_Mask (ULONG)(~(~0<<(14)))*/
#define DMA_RXCHCTL_RBSZ_Mask (ULONG)(0x3fff)
/*#define DMA_RXCHCTL_RBSZ_Wr_Mask (ULONG)(~((~(~0 << (14))) << (1)))*/
#define DMA_RXCHCTL_RBSZ_Wr_Mask (ULONG)(0xffff8001)

#define DMA_RXCHCTL_RBSZ_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHCTL_RgRd(n, v);\
        v = ((v & DMA_RXCHCTL_RBSZ_Wr_Mask) | ((data & DMA_RXCHCTL_RBSZ_Mask)<<1));\
        DMA_RXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_RXCHCTL_RBSZ_UdfRd(n, data) do {\
        DMA_RXCHCTL_RgRd(n, data);\
        data = ((data >> 1) & DMA_RXCHCTL_RBSZ_Mask);\
} while(0)

/*#define DMA_RXCHCTL_RXPBL_Mask (ULONG)(~(~0<<(6)))*/
#define DMA_RXCHCTL_RXPBL_Mask (ULONG)(0x3f)
/*#define DMA_RXCHCTL_RXPBL_Wr_Mask (ULONG)(~((~(~0 << (6))) << (16)))*/
#define DMA_RXCHCTL_RXPBL_Wr_Mask (ULONG)(0xffc0ffff)

#define DMA_RXCHCTL_RXPBL_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHCTL_RgRd(n, v);\
        v = ((v & DMA_RXCHCTL_RXPBL_Wr_Mask) | ((data & DMA_RXCHCTL_RXPBL_Mask)<<16));\
        DMA_RXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_RXCHCTL_RXPBL_UdfRd(n, data) do {\
        DMA_RXCHCTL_RgRd(n, data);\
        data = ((data >> 16) & DMA_RXCHCTL_RXPBL_Mask);\
} while(0)

/*#define DMA_RXCHCTL_RDSL_Mask (ULONG)(~(~0<<(3)))*/
#define DMA_RXCHCTL_RDSL_Mask (ULONG)(0x7)
/*#define DMA_RXCHCTL_RDSL_Wr_Mask (ULONG)(~((~(~0 << (3))) << (25)))*/
#define DMA_RXCHCTL_RDSL_Wr_Mask (ULONG)(0xf1ffffff)

#define DMA_RXCHCTL_RDSL_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHCTL_RgRd(n, v);\
        v = ((v & DMA_RXCHCTL_RDSL_Wr_Mask) | ((data & DMA_RXCHCTL_RDSL_Mask)<<25));\
        DMA_RXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_RXCHCTL_RDSL_UdfRd(n, data) do {\
        DMA_RXCHCTL_RgRd(n, data);\
        data = ((data >> 25) & DMA_RXCHCTL_RDSL_Mask);\
} while(0)

/*#define DMA_RXCHCTL_RPF_Mask (ULONG)(~(~0<<(1)))*/
#define DMA_RXCHCTL_RPF_Mask (ULONG)(0x1)
/*#define DMA_RXCHCTL_RPF_Wr_Mask (ULONG)(~((~(~0 << (1))) << (31)))*/
#define DMA_RXCHCTL_RPF_Wr_Mask (ULONG)(0x7fffffff)

#define DMA_RXCHCTL_RPF_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHCTL_RgRd(n, v);\
        v = ((v & DMA_RXCHCTL_RPF_Wr_Mask) | ((data & DMA_RXCHCTL_RPF_Mask)<<31));\
        DMA_RXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_RXCHCTL_RPF_UdfRd(n, data) do {\
        DMA_RXCHCTL_RgRd(n, data);\
        data = ((data >> 31) & DMA_RXCHCTL_RPF_Mask);\
} while(0)


#define DMA_RXCHCTL_WCHSTS_Mask (ULONG)(0xFF)
#define DMA_RXCHCTL_WCHSTS_Wr_Mask (ULONG)(0xfffff00ff)

#define DMA_RXCHCTL_WCHSTS_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHCTL_RgRd(n, v);\
        v = ((v & DMA_RXCHCTL_WCHSTS_Wr_Mask) | ((data & DMA_RXCHCTL_WCHSTS_Mask)<<8));\
        DMA_RXCHCTL_RgWr(n, v);\
} while(0)

#define DMA_RXCHCTL_WCHSTS_UdfRd(n, data) do {\
        DMA_RXCHCTL_RgRd(n, data);\
        data = ((data >> 8) & DMA_RXCHCTL_WCHSTS_Mask);\
} while(0)


#define DMA_RXCH_DESC_LISTHADDR_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_DESC_LISTHADDR_RgOffAddr(n));   }
#define DMA_RXCH_DESC_LISTHADDR_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_DESC_LISTHADDR_RgOffAddr(n)); }


#define DMA_RXCH_DESC_LISTLADDR_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_DESC_LISTLADDR_RgOffAddr(n));   }
#define DMA_RXCH_DESC_LISTLADDR_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_DESC_LISTLADDR_RgOffAddr(n)); }


#define DMA_RXCH_DESC_TAILPTR_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_DESC_TAILPTR_RgOffAddr(n));   }
#define DMA_RXCH_DESC_TAILPTR_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_DESC_TAILPTR_RgOffAddr(n)); }


#define DMA_RXCH_DESC_RING_LENGTH_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_DESC_RING_LENGTH_RgOffAddr(n));   }
#define DMA_RXCH_DESC_RING_LENGTH_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_DESC_RING_LENGTH_RgOffAddr(n)); }

#define DMA_RXCHINTMASK_MASK    (0x11)

#define DMA_RXCHINTMASK_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCHINTMASK_RgOffAddr(n));   }
#define DMA_RXCHINTMASK_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCHINTMASK_RgOffAddr(n)); }

#define DMA_RXCHINTMASK_RCEN_Mask (ULONG)(0x1)
#define DMA_RXCHINTMASK_RCEN_Wr_Mask (ULONG)(0xfffffffe)

#define DMA_RXCHINTMASK_RCEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_RXCHINTMASK_RCEN_Wr_Mask) | ((data & DMA_RXCHINTMASK_RCEN_Mask)<<0));\
        DMA_RXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_RXCHINTMASK_RCEN_UdfRd(n, data) do {\
        DMA_RXCHINTMASK_RgRd(n, data);\
        data = ((data >> 0) & DMA_RXCHINTMASK_RCEN_Mask);\
} while(0)


#define DMA_RXCHINTMASK_RSEN_Mask (ULONG)(0x1)
#define DMA_RXCHINTMASK_RSEN_Wr_Mask (ULONG)(0xfffffffd)

#define DMA_RXCHINTMASK_RSEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_RXCHINTMASK_RSEN_Wr_Mask) | ((data & DMA_RXCHINTMASK_RSEN_Mask)<<1));\
        DMA_RXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_RXCHINTMASK_RSEN_UdfRd(n, data) do {\
        DMA_RXCHINTMASK_RgRd(n, data);\
        data = ((data >> 1) & DMA_RXCHINTMASK_RSEN_Mask);\
} while(0)


#define DMA_RXCHINTMASK_UNFEN_Mask (ULONG)(0x1)
#define DMA_RXCHINTMASK_UNFEN_Wr_Mask (ULONG)(0xfffffffb)

#define DMA_RXCHINTMASK_UNFEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_RXCHINTMASK_UNFEN_Wr_Mask) | ((data & DMA_RXCHINTMASK_UNFEN_Mask)<<2));\
        DMA_RXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_RXCHINTMASK_UNFEN_UdfRd(n, data) do {\
        DMA_RXCHINTMASK_RgRd(n, data);\
        data = ((data >> 2) & DMA_RXCHINTMASK_UNFEN_Mask);\
} while(0)


#define DMA_RXCHINTMASK_RWTE_Mask (ULONG)(0x1)
#define DMA_RXCHINTMASK_RWTE_Wr_Mask (ULONG)(0xfffffff7)

#define DMA_RXCHINTMASK_RWTE_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_RXCHINTMASK_RWTE_Wr_Mask) | ((data & DMA_RXCHINTMASK_RWTE_Mask)<<3));\
        DMA_RXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_RXCHINTMASK_RWTE_UdfRd(n, data) do {\
        DMA_RXCHINTMASK_RgRd(n, data);\
        data = ((data >> 3) & DMA_RXCHINTMASK_RWTE_Mask);\
} while(0)


#define DMA_RXCHINTMASK_FEEN_Mask (ULONG)(0x1)
#define DMA_RXCHINTMASK_FEEN_Wr_Mask (ULONG)(0xffffffef)

#define DMA_RXCHINTMASK_FEEN_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCHINTMASK_RgRd(n, v);\
        v = ((v & DMA_RXCHINTMASK_FEEN_Wr_Mask) | ((data & DMA_RXCHINTMASK_FEEN_Mask)<<4));\
        DMA_RXCHINTMASK_RgWr(n, v);\
} while(0)

#define DMA_RXCHINTMASK_FEEN_UdfRd(n, data) do {\
        DMA_RXCHINTMASK_RgRd(n, data);\
        data = ((data >> 4) & DMA_RXCHINTMASK_FEEN_Mask);\
} while(0)


#define DMA_RXCH_CUR_DESC_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_CUR_DESC_RgOffAddr(n));   }
#define DMA_RXCH_CUR_DESC_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_CUR_DESC_RgOffAddr(n)); }


#define DMA_RXCH_CUR_BUFHA_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_CUR_BUFHA_RgOffAddr(n));   }
#define DMA_RXCH_CUR_BUFHA_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_CUR_BUFHA_RgOffAddr(n)); }


#define DMA_RXCH_CUR_BUFLA_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_CUR_BUFLA_RgOffAddr(n));   }
#define DMA_RXCH_CUR_BUFLA_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_CUR_BUFLA_RgOffAddr(n)); }



#define DMA_RXCH_CUR_WATCHDOG_TIMER_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_CUR_WATCHDOG_TIMER_RgOffAddr(n));   }
#define DMA_RXCH_CUR_WATCHDOG_TIMER_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_CUR_WATCHDOG_TIMER_RgOffAddr(n)); }

#define DMA_RXCH_CUR_WATCHDOG_RWT_Mask (ULONG)(0xff)
#define DMA_RXCH_CUR_WATCHDOG_RWT_Wr_Mask (ULONG)(0xffffff00)

#define DMA_RXCH_CUR_WATCHDOG_TIMER_RWT_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCH_CUR_WATCHDOG_TIMER_RgRd(n, v);\
        v = ((v & DMA_RXCH_CUR_WATCHDOG_RWT_Wr_Mask) | ((data & DMA_RXCH_CUR_WATCHDOG_RWT_Mask)<<0));\
        DMA_RXCH_CUR_WATCHDOG_TIMER_RgWr(n, v);\
} while(0)

#define DMA_RXCH_CUR_WATCHDOG_TIMER_RWT_UdfRd(n, data) do {\
        DMA_RXCH_CUR_WATCHDOG_TIMER_RgRd(n, data);\
        data = ((data >> 0) & DMA_RXCH_CUR_WATCHDOG_RWT_Mask);\
} while(0)


#define DMA_RXCH_MISS_COUNTER_RgWr(n, data) { iowrite32(data, (void *)DMA_RXCH_MISS_COUNTER_RgOffAddr(n));   }
#define DMA_RXCH_MISS_COUNTER_RgRd(n, data) { (data) = ioread32((void *)DMA_RXCH_MISS_COUNTER_RgOffAddr(n)); }

#define DMA_RXCH_MISS_COUNTER_MFC_Mask (ULONG)(0x7ff)
#define DMA_RXCH_MISS_COUNTER_MFC_Wr_Mask (ULONG)(0xfffff800)

#define DMA_RXCH_MISS_COUNTER_MFC_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCH_MISS_COUNTER_RgRd(n, v);\
        v = ((v & DMA_RXCH_MISS_COUNTER_MFC_Wr_Mask) | ((data & DMA_RXCH_MISS_COUNTER_MFC_Mask)<<0));\
        DMA_RXCH_MISS_COUNTER_RgWr(n, v);\
} while(0)

#define DMA_RXCH_MISS_COUNTER_MFC_UdfRd(n, data) do {\
        DMA_RXCH_MISS_COUNTER_RgRd(n, data);\
        data = ((data >> 0) & DMA_RXCH_MISS_COUNTER_MFC_Mask);\
} while(0)

#define DMA_RXCH_MISS_COUNTER_MFCO_Mask (ULONG)(0x1)
#define DMA_RXCH_MISS_COUNTER_MFCO_Wr_Mask (ULONG)(0xffff7fff)

#define DMA_RXCH_MISS_COUNTER_MFCO_UdfWr(n, data) do {\
        ULONG v;\
        DMA_RXCH_MISS_COUNTER_RgRd(n, v);\
        v = ((v & DMA_RXCH_MISS_COUNTER_MFCO_Wr_Mask) | ((data & DMA_RXCH_MISS_COUNTER_MFCO_Mask)<<15));\
        DMA_RXCH_MISS_COUNTER_RgWr(n, v);\
} while(0)

#define DMA_RXCH_MISS_COUNTER_MFCO_UdfRd(n, data) do {\
        DMA_RXCH_MISS_COUNTER_RgRd(n, data);\
        data = ((data >> 15) & DMA_RXCH_MISS_COUNTER_MFCO_Mask);\
} while(0)


/******************************************************************************/
/**                  Neutrino Wrapper TimeStamp Control  register            **/
/******************************************************************************/

#define ETH_AVB_WRAPPER_TS_CTRL_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x308C))      /* Neutrino PTP Local update value */


/* Neutrino Ethernet AVB Wrapper Timestamp Control Register Value*/
#define ETH_AVB_WRAPPER_TS_CTRL_RgWr(data) do {\
        iowrite32(data, (void *)ETH_AVB_WRAPPER_TS_CTRL_RgOffAddr);\
} while(0)

#define ETH_AVB_WRAPPER_TS_CTRL_RgRd(data) do {\
        (data) = ioread32((void *)ETH_AVB_WRAPPER_TS_CTRL_RgOffAddr);\
} while(0)

#define ETH_AVB_WRAPPER_TS_CTRL_Mask (ULONG)(0x1)
#define ETH_AVB_WRAPPER_TS_CTRL_Wr_Mask (ULONG)(0xffffffef)

#define ETH_AVB_WRAPPER_TS_CTRL_UdfWr(data) do {\
        ULONG v;\
        ETH_AVB_WRAPPER_TS_CTRL_RgRd(v);\
        v = ((v & ETH_AVB_WRAPPER_TS_CTRL_Wr_Mask) | ((data & ETH_AVB_WRAPPER_TS_CTRL_Mask)<<4));\
        ETH_AVB_WRAPPER_TS_CTRL_RgWr(v);\
} while(0)

#define ETH_AVB_WRAPPER_TS_CTRL_UdfRd(data) do {\
        ETH_AVB_WRAPPER_TS_CTRL_RgRd(data);\
        data = ((data >> 4) & ETH_AVB_WRAPPER_TS_CTRL_Mask);\
} while(0)

#define ETH_AVB_WRAPPER_TS_CTRL_TSTMP_WINDOW_Wr(data) do {\
        ULONG v;\
        ETH_AVB_WRAPPER_TS_CTRL_RgRd(v);\
        v = ((v & 0xFF00FFFF) | ((data & 0xFF)<<16));\
        ETH_AVB_WRAPPER_TS_CTRL_RgWr(v);\
} while(0)

#define ETH_AVB_WRAPPER_TS_CTRL_TSTMP_WINDOW_Rd(data) do {\
        ETH_AVB_WRAPPER_TS_CTRL_RgRd(data);\
        data = ((data >> 16) & 0xFF);\
} while(0)

/******************************************************************************/
/**                         Neutrino Wrapper PTP  registers                  **/
/******************************************************************************/
#define NTN_PTPLCLINIT_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x30E8))      /* Neutrino PTP Local init value */
#define NTN_PTPLCLUPDT_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x30EC))      /* Neutrino PTP Local update value */

/* Neutrino PTP Local init value */
#define NTN_PTPLCLINIT_RgWr(data) do {\
        iowrite32(data, (void *)NTN_PTPLCLINIT_RgOffAddr);\
} while(0)

#define NTN_PTPLCLINIT_RgRd(data) do {\
        (data) = ioread32((void *)NTN_PTPLCLINIT_RgOffAddr);\
} while(0)

/* Neutrino PTP Local udpate value */
#define NTN_PTPLCLUPDT_RgWr(data) do {\
        iowrite32(data, (void *)NTN_PTPLCLUPDT_RgOffAddr);\
} while(0)

#define NTN_PTPLCLUPDT_RgRd(data) do {\
        (data) = ioread32((void *)NTN_PTPLCLUPDT_RgOffAddr);\
} while(0)

/******************************************************************************/
/**                         Neutrino Config registers                        **/
/******************************************************************************/
#define NTN_NCID_RgOffAddr       ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x0000))      /* Neutrino Chip and revision ID       */
#define NTN_NMODESTS_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x0004))      /* Neutrino current operation mode     */
#define NTN_NCTLSTS_RgOffAddr    ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1000))      /* Neutrino control and status         */
#define NTN_NEMACCTL_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x0010))      /* Neutrino eMAC div and control       */
#define NTN_NCLKCTRL_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1004))      /* Neutrino clock control              */
#define NTN_NRSTCTRL_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1008))      /* Neutrino reset control              */
#define NTN_NFUNCEN0_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x0008))      /* Neutrino pin mux control            */
#define NTN_NFUNCEN1_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x100C))      /* Neutrino pin mux control            */
#define NTN_NLCLMSK_RgOffAddr    ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1010))      /* Neutrino logical window mask        */
#define NTN_NLCLLBASE_RgOffAddr  ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1014))      /* Neutrino logical window lower base  */
#define NTN_NLCLUBASE_RgOffAddr  ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1018))      /* Neutrino logical window upper base  */
#define NTN_NSPLLPARAM_RgOffAddr ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1020))      /* Neutrino System PLL parameters      */
#define NTN_NSPLLUPDT_RgOffAddr  ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1024))      /* Neutrino System PLL update          */
#define NTN_NHPLLPARAM_RgOffAddr ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1030))      /* Neutrino HSIC PLL parameters        */
#define NTN_NHPLLUPDT_RgOffAddr  ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1034))      /* Neutrino HSIC PLL update            */
#define NTN_NQSPIDIV_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x000C))      /* Neutrino QSPI DIV                   */
#define NTN_NMEMOFST_RgOffAddr   ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1100))      /* Neutrino sideband SRAM load pointer */
#define NTN_NMEMVAL_RgOffAddr    ((volatile ULONG *)(NTN_REG_BASE_ADRS + 0x1104))      /* Neutrino sideband SRAM data         */

/* Neutrino clock control              */
#define NTN_NCLKCTRL_RgWr(data) do {\
        iowrite32(data, (void *)NTN_NCLKCTRL_RgOffAddr);\
} while(0)

#define NTN_NCLKCTRL_RgRd(data) do {\
        (data) = ioread32((void *)NTN_NCLKCTRL_RgOffAddr);\
} while(0)

/* eMAC TX clock enable */
#define NTN_NCLKCTRL_MACTXCEN_Mask (ULONG)(0x1)
#define NTN_NCLKCTRL_MACTXCEN_Wr_Mask (ULONG)(~(0x0080))

#define NTN_NCLKCTRL_MACTXCEN_UdfWr(data) do {\
        ULONG v;\
        NTN_NCLKCTRL_RgRd(v);\
        v = ((v & NTN_NCLKCTRL_MACTXCEN_Wr_Mask) | ((data & NTN_NCLKCTRL_MACTXCEN_Mask) << 7));\
        NTN_NCLKCTRL_RgWr(v);\
} while(0)

#define NTN_NCLKCTRL_MACTXCEN_UdfRd(data) do {\
        NTN_NCLKCTRL_RgRd(data);\
        data = ((data >> 7) & NTN_NCLKCTRL_MACTXCEN_Mask);\
} while(0)

/* eMAC RX clock enable */
#define NTN_NCLKCTRL_MACRXCEN_Mask (ULONG)(0x1)
#define NTN_NCLKCTRL_MACRXCEN_Wr_Mask (ULONG)(~(0x4000))

#define NTN_NCLKCTRL_MACRXCEN_UdfWr(data) do {\
        ULONG v;\
        NTN_NCLKCTRL_RgRd(v);\
        v = ((v & NTN_NCLKCTRL_MACRXCEN_Wr_Mask) | ((data & NTN_NCLKCTRL_MACRXCEN_Mask) << 14));\
        NTN_NCLKCTRL_RgWr(v);\
} while(0)

#define NTN_NCLKCTRL_MACRXCEN_UdfRd(data) do {\
        NTN_NCLKCTRL_RgRd(data);\
        data = ((data >> 7) & NTN_NCLKCTRL_MACTXCEN_Mask);\
} while(0)

/* Neutrino reset control */
#define NTN_NRSTCTRL_RgWr(data) do {\
        iowrite32(data, (void *)NTN_NRSTCTRL_RgOffAddr);\
} while(0)

#define NTN_NRSTCTRL_RgRd(data) do {\
        (data) = ioread32((void *)NTN_NRSTCTRL_RgOffAddr);\
} while(0)

/* eMAC force reset */
#define NTN_NRSTCTRL_MACRST_Mask (ULONG)(0x1)
#define NTN_NRSTCTRL_MACRST_Wr_Mask (ULONG)(~(0x0080))

#define NTN_NRSTCTRL_MACRST_UdfWr(data) do {\
        ULONG v;\
        NTN_NRSTCTRL_RgRd(v);\
        v = ((v & NTN_NRSTCTRL_MACRST_Wr_Mask) | ((data & NTN_NRSTCTRL_MACRST_Mask) << 7));\
        NTN_NRSTCTRL_RgWr(v);\
} while(0)

#define NTN_NRSTCTRL_MACRST_UdfRd(data) do {\
        NTN_NRSTCTRL_RgRd(data);\
        data = ((data >> 7) & NTN_NRSTCTRL_MACRST_Mask);\
} while(0)

/* Neutrino eMAC div and control register  */
#define NTN_NEMACCTL_RgRd(data) do {\
        (data) = ioread32((void *)NTN_NEMACCTL_RgOffAddr);\
} while(0)

#define NTN_NEMACCTL_RgWr(data) do {\
        iowrite32(data, (void *)NTN_NEMACCTL_RgOffAddr);\
} while(0)

#define NTN_NMODESTS_HOST_BOOT_MASK      (0x1<<6)
#define NTN_NMODESTS_SECURE_BOOT_MASK    (0x1)
#define NTN_NCTLSTS_HW_SEQ_COMPLETE_MASK (0x1)

/* Neutrino boot mode option */
#define NTN_NMODESTS_RgRd(data) do {\
        (data) = ioread32((void *)NTN_NMODESTS_RgOffAddr);\
} while(0)

/* Neutrino control and status */
#define NTN_NCTLSTS_RgRd(data) do {\
        (data) = ioread32((void *)NTN_NCTLSTS_RgOffAddr);\
} while(0)

#define NTN_NEMACCTL_TX_CLK_MASK     	   (~0x7)
#define NTN_NEMACCTL_TX_CLK_125MHz  	   (0x0)
#define NTN_NEMACCTL_TX_CLK_25MHz   	   (0x2)
#define NTN_NEMACCTL_TX_CLK_2_5MHz    	   (0x3)
#define NTN_NEMACCTL_TX_CLK_25MHz_RMII     (0x6)
#define NTN_NEMACCTL_TX_CLK_2_5MHz_RMII    (0x7)
#define NTN_NEMACCTL_PHY_INTF_MASK         (~0x38)
#define NTN_NEMACCTL_PHY_INTF_MII          (0x00)
#define NTN_NEMACCTL_PHY_INTF_RGMII        (0x08)
#define NTN_NEMACCTL_PHY_INTF_RMII         (0x20)

/******************************************************************************/
/**                          Neutrino M3 SRAM Debug Memory                   **/
/******************************************************************************/
#define M3STAT_MSI_CNT_TXCH_MemOffAddr(n)     ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_MSI_CNT_TXCH_OFFSET + (n * 4) ))
#define M3STAT_MSI_CNT_RXCH_MemOffAddr(n)     ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_MSI_CNT_RXCH_OFFSET + (n * 4) ))
#define M3STAT_MSI_MAC_LPI_MemOffAddr         ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_MSI_MAC_LPI_OFFSET ))
#define M3STAT_MSI_TDM_MemOffAddr             ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_MSI_TDM_OFFSET ))
#define M3STAT_MSI_CAN_MemOffAddr             ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_MSI_CAN_OFFSET ))
#define M3STAT_M3TICK_MemOffAddr              ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_M3TICK_OFFSET ))
#define M3STAT_WDT_MemOffAddr                 ((volatile ULONG *)(M3STAT_BASE_ADDRESS + M3STAT_WDT_OFFSET ))

/******************************************************************************/
/**                          Neutrino INTC registers                         **/
/******************************************************************************/
#define NTN_INTC_INTSTATUS_RgOffAddr         ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x00))    /* Neutrino Wrapper Interrupt status register */
#define NTN_INTC_HSICSTATUS_RgOffAddr        ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x04))    /* Neutrino Wrapper HSIC status register */
#define NTN_INTC_GDMASTATUS_RgOffAddr        ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x08))    /* Neutrino Wrapper GDMA status register */
#define NTN_INTC_MACSTATUS_RgOffAddr         ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x0C))    /* Neutrino Wrapper eMAC status register */
#define NTN_INTC_TDMSTATUS_RgOffAddr         ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x10))    /* Neutrino Wrapper TDM status register */

#define NTN_INTC_INTMCUMASK0_RgOffAddr       ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x20))    /* Neutrino Wrapper Mask register for MCU interrupts register */
#define NTN_INTC_INTMCUMASK1_RgOffAddr       ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x24))    /* Neutrino Wrapper Mask register for MCU interrupts register */
#define NTN_INTC_INTMCUMASK2_RgOffAddr       ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x28))    /* Neutrino Wrapper Mask register for MCU interrupts register */

#define NTN_INTC_INTEXTMASK0_RgOffAddr       ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x30))    /* Neutrino Wrapper Mask register for EXT interrupt register */
#define NTN_INTC_INTEXTMASK1_RgOffAddr       ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x34))    /* Neutrino Wrapper Mask register for EXT interrupt register */
#define NTN_INTC_INTEXTMASK2_RgOffAddr       ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x38))    /* Neutrino Wrapper Mask register for EXT interrupt register */

#define NTN_INTC_EXTINTCFG_RgOffAddr         ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x4C))    /* Neutrino Wrapper External interrupt configuration register */
#define NTN_INTC_PMEINTCFG_RgOffAddr         ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x50))    /* Neutrino Wrapper PME interrupt configuration register */
#define NTN_INTC_MCUFLG_RgOffAddr            ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x54))    /* Neutrino Wrapper MCU Flag interrupt register */
#define NTN_INTC_EXTFLG_RgOffAddr            ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x58))    /* Neutrino Wrapper External host flag interrupt register */
#define NTN_INTC_WDCTL_RgOffAddr             ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x60))    /* Neutrino Interrupt WatchDog Interrupt Control register */
#define NTN_INTC_WDEXP_RgOffAddr             ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x64))    /* Neutrino Interrupt WatchDog Expiry Control register */
#define NTN_INTC_WDMON_RgOffAddr             ((volatile ULONG *)(INTC_BASE_ADDRESS + 0x68))    /* Neutrino Interrupt WatchDog Monitor register */

/* Neutrino wrapper interrupt status */
#define NTN_INTC_INTSTATUS_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_INTSTATUS_RgOffAddr);\
} while(0)

/* Write PME interrupt configuration */
#define NTN_INTC_PMEINTCFG_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_PMEINTCFG_RgOffAddr);\
} while(0)

#define NTN_INTC_GMAC_INT_MASK  (0x3FC000FF)
#define NTN_INTC_GMAC_INT_TXCH2_BIT_POS 13
#define NTN_INTC_GMAC_INT_RXCH0_BIT_POS 16

#define NTN_INTC_INTSTATUS_GMAC_LPOS    (9)
#define NTN_INTC_INTSTATUS_GMAC_HPOS    (13)
#define NTN_INTC_INTSTATUS_MAC_LPI_LPOS (9)
#define NTN_INTC_INTSTATUS_MAC_LPI_HPOS (9)
#define NTN_INTC_INTSTATUS_MAC_PM_LPOS  (10)
#define NTN_INTC_INTSTATUS_MAC_PM_HPOS  (10)
#define NTN_INTC_INTSTATUS_MAC_LPOS     (9)
#define NTN_INTC_INTSTATUS_MAC_HPOS     (9)
#define NTN_INTC_INTSTATUS_TXRX_LPOS    (12)
#define NTN_INTC_INTSTATUS_TXRX_HPOS    (13)
#define NTN_INTC_INTSTATUS_TDM_LPOS    (14)
#define NTN_INTC_INTSTATUS_TDM_HPOS    (14)

/* Neutrino wrapper MAC LPI exit interrupt status */
#define NTN_INTC_INTSTATUS_MACLPI_Mask (ULONG)(0x1)

#define NTN_INTC_INTSTATUS_MACLPI_UdfRd(n, data) do {\
        NTN_INTC_INTSTATUS_RgRd(n, data);\
        data = ((data >> 9) & NTN_INTC_INTSTATUS_MACLPI_Mask);\
} while(0)

/* Neutrino wrapper MAC Power management interrupt status */
#define NTN_INTC_INTSTATUS_MACPM_Mask (ULONG)(0x1)

#define NTN_INTC_INTSTATUS_MACPM_UdfRd(n, data) do {\
        NTN_INTC_INTSTATUS_RgRd(n, data);\
        data = ((data >> 10) & NTN_INTC_INTSTATUS_MACPM_Mask);\
} while(0)

/* Neutrino wrapper MAC interrupt to indicate event from LPI, RGMII, Management counters, power management or MAC core */
#define NTN_INTC_INTSTATUS_MAC_Mask (ULONG)(0x1)

#define NTN_INTC_INTSTATUS_MAC_UdfRd(n, data) do {\
        NTN_INTC_INTSTATUS_RgRd(n, data);\
        data = ((data >> 11) & NTN_INTC_INTSTATUS_MAC_Mask);\
} while(0)

/* Neutrino wrapper MAC DMA TX channels interrupt status */
#define NTN_INTC_INTSTATUS_MACDMATX_Mask (ULONG)(0x1)

#define NTN_INTC_INTSTATUS_MACDMATX_UdfRd(n, data) do {\
        NTN_INTC_INTSTATUS_RgRd(n, data);\
        data = ((data >> 12) & NTN_INTC_INTSTATUS_MACDMATX_Mask);\
} while(0)

/* Neutrino wrapper MAC DMA RX channels interrupt status */
#define NTN_INTC_INTSTATUS_MACDMARX_Mask (ULONG)(0x1)

#define NTN_INTC_INTSTATUS_MACDMARX_UdfRd(n, data) do {\
        NTN_INTC_INTSTATUS_RgRd(n, data);\
        data = ((data >> 13) & NTN_INTC_INTSTATUS_MACDMARX_Mask);\
} while(0)



/* Neutrino MAC interrupt status */
#define NTN_INTC_MACSTATUS_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_MACSTATUS_RgOffAddr);\
} while(0)

/* Neutrino wrapper MAC DMA channels interrupt mask */
#define NTN_INTC_MACSTATUS_RXCH5INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_RXCH4INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_RXCH3INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_RXCH2INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_RXCH1INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_RXCH0INT_Mask (ULONG)(0x1)

#define NTN_INTC_MACSTATUS_TXCH4INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_TXCH3INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_TXCH2INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_TXCH1INT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_TXCH0INT_Mask (ULONG)(0x1)

#define NTN_INTC_MACSTATUS_RXCH5INT_StartPos (ULONG)(21)
#define NTN_INTC_MACSTATUS_RXCH4INT_StartPos (ULONG)(20)
#define NTN_INTC_MACSTATUS_RXCH3INT_StartPos (ULONG)(19)
#define NTN_INTC_MACSTATUS_RXCH2INT_StartPos (ULONG)(18)
#define NTN_INTC_MACSTATUS_RXCH1INT_StartPos (ULONG)(17)
#define NTN_INTC_MACSTATUS_RXCH0INT_StartPos (ULONG)(16)

#define NTN_INTC_MACSTATUS_TXCH4INT_StartPos (ULONG)(4)
#define NTN_INTC_MACSTATUS_TXCH3INT_StartPos (ULONG)(3)
#define NTN_INTC_MACSTATUS_TXCH2INT_StartPos (ULONG)(2)
#define NTN_INTC_MACSTATUS_TXCH1INT_StartPos (ULONG)(1)
#define NTN_INTC_MACSTATUS_TXCH0INT_StartPos (ULONG)(0)

#define NTN_INTC_MACSTATUS_TXCHINT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_TXCHINT_StartPos (ULONG)(0)
#define NTN_INTC_MACSTATUS_RXCHINT_Mask (ULONG)(0x1)
#define NTN_INTC_MACSTATUS_RXCHINT_StartPos (ULONG)(16)

/* Neutrino wrapper MAC TX DMA CH status */
#define NTN_INTC_MACSTATUS_TXCHINT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> (NTN_INTC_MACSTATUS_TXCHINT_StartPos + n)) & NTN_INTC_MACSTATUS_TXCHINT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH status */
#define NTN_INTC_MACSTATUS_RXCHINT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> (NTN_INTC_MACSTATUS_RXCHINT_StartPos + n)) & NTN_INTC_MACSTATUS_RXCHINT_Mask);\
} while(0)



/* Neutrino wrapper MAC TX DMA CH0 status */
#define NTN_INTC_MACSTATUS_TXCH0INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH0INT_StartPos) & NTN_INTC_MACSTATUS_TXCH0INT_Mask);\
} while(0)

/* Neutrino wrapper MAC TX DMA CH1 status */
#define NTN_INTC_MACSTATUS_TXCH1INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH1INT_StartPos) & NTN_INTC_MACSTATUS_TXCH1INT_Mask);\
} while(0)

/* Neutrino wrapper MAC TX DMA CH2 status */
#define NTN_INTC_MACSTATUS_TXCH2INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH2INT_StartPos) & NTN_INTC_MACSTATUS_TXCH2INT_Mask);\
} while(0)

/* Neutrino wrapper MAC TX DMA CH3 status */
#define NTN_INTC_MACSTATUS_TXCH3INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH3INT_StartPos) & NTN_INTC_MACSTATUS_TXCH3INT_Mask);\
} while(0)

/* Neutrino wrapper MAC TX DMA CH4 status */
#define NTN_INTC_MACSTATUS_TXCH4INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH4INT_StartPos) & NTN_INTC_MACSTATUS_TXCH4INT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH0 status */
#define NTN_INTC_MACSTATUS_RXCH0INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_RXCH0INT_StartPos) & NTN_INTC_MACSTATUS_RXCH0INT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH1 status */
#define NTN_INTC_MACSTATUS_RXCH1INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_RXCH1INT_StartPos) & NTN_INTC_MACSTATUS_RXCH1INT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH2 status */
#define NTN_INTC_MACSTATUS_RXCH2INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_RXCH2INT_StartPos) & NTN_INTC_MACSTATUS_RXCH2INT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH3 status */
#define NTN_INTC_MACSTATUS_RXCH3INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH3INT_StartPos) & NTN_INTC_MACSTATUS_RXCH3INT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH4 status */
#define NTN_INTC_MACSTATUS_RXCH4INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_TXCH4INT_StartPos) & NTN_INTC_MACSTATUS_RXCH4INT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH5 status */
#define NTN_INTC_MACSTATUS_RXCH5INT_UdfRd(n, data) do {\
        NTN_INTC_MACSTATUS_RgRd(data);\
        data = ((data >> NTN_INTC_MACSTATUS_RXCH5INT_StartPos) & NTN_INTC_MACSTATUS_RXCH5INT_Mask);\
} while(0)


/* Neutrino TDM interrupt status */
#define NTN_INTC_TDMSTATUS_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_TDMSTATUS_RgOffAddr);\
} while(0)

#define NTN_INTC_TDMSTATUS_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_TDMSTATUS_RgOffAddr);\
} while(0)

#define NTN_INTC_TDM_IP_0_OVERFLOW 	(0x40)
#define NTN_INTC_TDM_IP_1_OVERFLOW 	(0x20)
#define NTN_INTC_TDM_IP_2_OVERFLOW 	(0x10)
#define NTN_INTC_TDM_IP_3_OVERFLOW 	(0x08)
#define NTN_INTC_TDM_OP_OVERFLOW 	(0x04)
#define NTN_INTC_TDM_OP_UNDERFLOW 	(0x02)
#define NTN_INTC_TDM_GENERAL_ERROR	(0x01)

/* Neutrino Interrupt MCU mask */
#define NTN_INTC_INTMCUMASK0_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_INTMCUMASK0_RgOffAddr);\
} while(0)

#define NTN_INTC_INTMCUMASK0_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_INTMCUMASK0_RgOffAddr);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_INTMCUMASK1_RgOffAddr);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_INTMCUMASK1_RgOffAddr);\
} while(0)

#define NTN_INTC_INTMCUMASK2_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_INTMCUMASK2_RgOffAddr);\
} while(0)

#define NTN_INTC_INTMCUMASK2_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_INTMCUMASK2_RgOffAddr);\
} while(0)


/* Neutrino wrapper MAC DMA channels interrupt status */
#define NTN_INTC_INTMCUMASK1_RXCH5INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_RXCH4INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_RXCH3INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_RXCH2INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_RXCH1INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_RXCH0INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_TXCH4INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_TXCH3INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_TXCH2INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_TXCH1INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_TXCH0INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_MAC_Mask       (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_MACPM_Mask     (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_MACLPI_Mask    (ULONG)(0x1)

#define NTN_INTC_INTMCUMASK1_RXCH5INT_Wr_Mask  (ULONG)(0x00200000)
#define NTN_INTC_INTMCUMASK1_RXCH4INT_Wr_Mask  (ULONG)(0x00100000)
#define NTN_INTC_INTMCUMASK1_RXCH3INT_Wr_Mask  (ULONG)(0x00080000)
#define NTN_INTC_INTMCUMASK1_RXCH2INT_Wr_Mask  (ULONG)(0x00040000)
#define NTN_INTC_INTMCUMASK1_RXCH1INT_Wr_Mask  (ULONG)(0x00020000)
#define NTN_INTC_INTMCUMASK1_RXCH0INT_Wr_Mask  (ULONG)(0x00010000)
#define NTN_INTC_INTMCUMASK1_TXCH4INT_Wr_Mask  (ULONG)(0x00008000)
#define NTN_INTC_INTMCUMASK1_TXCH3INT_Wr_Mask  (ULONG)(0x00004000)
#define NTN_INTC_INTMCUMASK1_TXCH2INT_Wr_Mask  (ULONG)(0x00002000)
#define NTN_INTC_INTMCUMASK1_TXCH1INT_Wr_Mask  (ULONG)(0x00001000)
#define NTN_INTC_INTMCUMASK1_TXCH0INT_Wr_Mask  (ULONG)(0x00000800)
#define NTN_INTC_INTMCUMASK1_MAC_Wr_Mask       (ULONG)(0x00000400)
#define NTN_INTC_INTMCUMASK1_MACPM_Wr_Mask     (ULONG)(0x00000200)
#define NTN_INTC_INTMCUMASK1_MACLPI_Wr_Mask    (ULONG)(0x00000100)

#define NTN_INTC_INTMCUMASK1_RXCH5INT_StartPos  (ULONG)(21)
#define NTN_INTC_INTMCUMASK1_RXCH4INT_StartPos  (ULONG)(20)
#define NTN_INTC_INTMCUMASK1_RXCH3INT_StartPos  (ULONG)(19)
#define NTN_INTC_INTMCUMASK1_RXCH2INT_StartPos  (ULONG)(18)
#define NTN_INTC_INTMCUMASK1_RXCH1INT_StartPos  (ULONG)(17)
#define NTN_INTC_INTMCUMASK1_RXCH0INT_StartPos  (ULONG)(16)
#define NTN_INTC_INTMCUMASK1_TXCH4INT_StartPos  (ULONG)(15)
#define NTN_INTC_INTMCUMASK1_TXCH3INT_StartPos  (ULONG)(14)
#define NTN_INTC_INTMCUMASK1_TXCH2INT_StartPos  (ULONG)(13)
#define NTN_INTC_INTMCUMASK1_TXCH1INT_StartPos  (ULONG)(12)
#define NTN_INTC_INTMCUMASK1_TXCH0INT_StartPos  (ULONG)(11)
#define NTN_INTC_INTMCUMASK1_MAC_StartPos       (ULONG)(10)
#define NTN_INTC_INTMCUMASK1_MACPM_StartPos     (ULONG)(9)
#define NTN_INTC_INTMCUMASK1_MACLPI_StartPos    (ULONG)(8)

#define NTN_INTC_INTMCUMASK1_TXCHINT_Mask (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_TXCHINT_StartPos (ULONG)(11)
#define NTN_INTC_INTMCUMASK1_RXCHINT_Mask (ULONG)(0x1)
#define NTN_INTC_INTMCUMASK1_RXCHINT_StartPos (ULONG)(16)

/* Neutrino wrapper MAC TX DMA CH interrupt mask */
#define NTN_INTC_INTMCUMASK1_TXCHINT_UdfWr(n, data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & (NTN_INTC_INTMCUMASK1_TXCHINT_Mask << (NTN_INTC_INTMCUMASK1_TXCHINT_StartPos + n))) | ((data & NTN_INTC_INTMCUMASK1_TXCHINT_Mask)<<(NTN_INTC_INTMCUMASK1_TXCHINT_StartPos + n)));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_TXCHINT_UdfRd(n, data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> (NTN_INTC_INTMCUMASK1_TXCHINT_StartPos + n)) & NTN_INTC_INTMCUMASK1_TXCHINT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCHINT_UdfWr(n ,data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & (NTN_INTC_INTMCUMASK1_RXCHINT_Mask << (NTN_INTC_INTMCUMASK1_RXCHINT_StartPos + n))) | ((data & NTN_INTC_INTMCUMASK1_RXCHINT_Mask)<<(NTN_INTC_INTMCUMASK1_RXCHINT_StartPos + n)));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCHINT_UdfRd(n, data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> (NTN_INTC_INTMCUMASK1_RXCHINT_StartPos + n)) & NTN_INTC_INTMCUMASK1_RXCHINT_Mask);\
} while(0)




/* Neutrino wrapper DMA RX Channel 5 interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCH5INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_RXCH5INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_RXCH5INT_Mask)<<NTN_INTC_INTMCUMASK1_RXCH5INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCH5INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_RXCH5INT_StartPos) & NTN_INTC_INTMCUMASK1_RXCH5INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 4 interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCH4INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_RXCH4INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_RXCH4INT_Mask)<<NTN_INTC_INTMCUMASK1_RXCH4INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCH4INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_RXCH4INT_StartPos) & NTN_INTC_INTMCUMASK1_RXCH4INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 3 interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCH3INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_RXCH3INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_RXCH3INT_Mask)<<NTN_INTC_INTMCUMASK1_RXCH3INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCH3INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_RXCH3INT_StartPos) & NTN_INTC_INTMCUMASK1_RXCH3INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 2 interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCH2INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_RXCH2INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_RXCH2INT_Mask)<<NTN_INTC_INTMCUMASK1_RXCH2INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCH2INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_RXCH2INT_StartPos) & NTN_INTC_INTMCUMASK1_RXCH2INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 1 interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCH1INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_RXCH1INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_RXCH1INT_Mask)<<NTN_INTC_INTMCUMASK1_RXCH1INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCH1INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_RXCH1INT_StartPos) & NTN_INTC_INTMCUMASK1_RXCH1INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 0 interrupt mask */
#define NTN_INTC_INTMCUMASK1_RXCH0INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_RXCH0INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_RXCH0INT_Mask)<<NTN_INTC_INTMCUMASK1_RXCH0INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_RXCH0INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_RXCH0INT_StartPos) & NTN_INTC_INTMCUMASK1_RXCH0INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 4 interrupt mask */
#define NTN_INTC_INTMCUMASK1_TXCH4INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_TXCH4INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_TXCH4INT_Mask)<<NTN_INTC_INTMCUMASK1_TXCH4INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_TXCH4INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_TXCH4INT_StartPos) & NTN_INTC_INTMCUMASK1_TXCH4INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 3 interrupt mask */
#define NTN_INTC_INTMCUMASK1_TXCH3INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_TXCH3INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_TXCH3INT_Mask)<<NTN_INTC_INTMCUMASK1_TXCH3INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_TXCH3INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_TXCH3INT_StartPos) & NTN_INTC_INTMCUMASK1_TXCH3INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 2 interrupt mask */
#define NTN_INTC_INTMCUMASK1_TXCH2INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_TXCH2INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_TXCH2INT_Mask)<<NTN_INTC_INTMCUMASK1_TXCH2INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_TXCH2INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_TXCH2INT_StartPos) & NTN_INTC_INTMCUMASK1_TXCH2INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 1 interrupt mask */
#define NTN_INTC_INTMCUMASK1_TXCH1INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_TXCH1INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_TXCH1INT_Mask)<<NTN_INTC_INTMCUMASK1_TXCH1INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_TXCH1INT_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_TXCH1INT_StartPos) & NTN_INTC_INTMCUMASK1_TXCH1INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 0 interrupt mask */
#define NTN_INTC_INTMCUMASK1_TXCH0INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_TXCH0INT_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_TXCH0INT_Mask)<<NTN_INTC_INTMCUMASK1_TXCH0INT_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_TXCH0INT_UdfRd(n, data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_TXCH0INT_StartPos) & NTN_INTC_INTMCUMASK1_TXCH0INT_Mask);\
} while(0)

/* Neutrino wrapper MAC interrupt mask */
#define NTN_INTC_INTMCUMASK1_MAC_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_MAC_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_MAC_Mask)<<NTN_INTC_INTMCUMASK1_MAC_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_MAC_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_MAC_StartPos) & NTN_INTC_INTMCUMASK1_MAC_Mask);\
} while(0)

/* Neutrino wrapper MAC PM interrupt mask */
#define NTN_INTC_INTMCUMASK1_MACPM_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_MACPM_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_MACPM_Mask)<<NTN_INTC_INTMCUMASK1_MACPM_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_MACPM_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_MACPM_StartPos) & NTN_INTC_INTMCUMASK1_MACPM_Mask);\
} while(0)

/* Neutrino wrapper MAC LPI interrupt mask */
#define NTN_INTC_INTMCUMASK1_MACLPI_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTMCUMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTMCUMASK1_MACLPI_Wr_Mask) | ((data & NTN_INTC_INTMCUMASK1_MACLPI_Mask)<<NTN_INTC_INTMCUMASK1_MACLPI_StartPos));\
        NTN_INTC_INTMCUMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTMCUMASK1_MACLPI_UdfRd(data) do {\
        NTN_INTC_INTMCUMASK1_RgRd(data);\
        data = ((data >> NTN_INTC_INTMCUMASK1_MACLPI_StartPos) & NTN_INTC_INTMCUMASK1_MACLPI_Mask);\
} while(0)



/* Neutrino Interrupt EXT mask1 */
#define NTN_INTC_INTEXTMASK1_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_INTEXTMASK1_RgOffAddr);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_INTEXTMASK1_RgOffAddr);\
} while(0)

/* Neutrino wrapper MAC DMA channels interrupt status */
#define NTN_INTC_INTEXTMASK1_RXCH5INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_RXCH4INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_RXCH3INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_RXCH2INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_RXCH1INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_RXCH0INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_TXCH4INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_TXCH3INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_TXCH2INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_TXCH1INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_TXCH0INT_Mask  (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_MAC_Mask       (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_MACPM_Mask     (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_MACLPI_Mask    (ULONG)(0x1)

#define NTN_INTC_INTEXTMASK1_RXCH5INT_Wr_Mask  (ULONG)(0x00200000)
#define NTN_INTC_INTEXTMASK1_RXCH4INT_Wr_Mask  (ULONG)(0x00100000)
#define NTN_INTC_INTEXTMASK1_RXCH3INT_Wr_Mask  (ULONG)(0x00080000)
#define NTN_INTC_INTEXTMASK1_RXCH2INT_Wr_Mask  (ULONG)(0x00040000)
#define NTN_INTC_INTEXTMASK1_RXCH1INT_Wr_Mask  (ULONG)(0x00020000)
#define NTN_INTC_INTEXTMASK1_RXCH0INT_Wr_Mask  (ULONG)(0x00010000)
#define NTN_INTC_INTEXTMASK1_TXCH4INT_Wr_Mask  (ULONG)(0x00008000)
#define NTN_INTC_INTEXTMASK1_TXCH3INT_Wr_Mask  (ULONG)(0x00004000)
#define NTN_INTC_INTEXTMASK1_TXCH2INT_Wr_Mask  (ULONG)(0x00002000)
#define NTN_INTC_INTEXTMASK1_TXCH1INT_Wr_Mask  (ULONG)(0x00001000)
#define NTN_INTC_INTEXTMASK1_TXCH0INT_Wr_Mask  (ULONG)(0x00000800)
#define NTN_INTC_INTEXTMASK1_MAC_Wr_Mask       (ULONG)(0x00000400)
#define NTN_INTC_INTEXTMASK1_MACPM_Wr_Mask     (ULONG)(0x00000200)
#define NTN_INTC_INTEXTMASK1_MACLPI_Wr_Mask    (ULONG)(0x00000100)

#define NTN_INTC_INTEXTMASK1_RXCH5INT_StartPos  (ULONG)(21)
#define NTN_INTC_INTEXTMASK1_RXCH4INT_StartPos  (ULONG)(20)
#define NTN_INTC_INTEXTMASK1_RXCH3INT_StartPos  (ULONG)(19)
#define NTN_INTC_INTEXTMASK1_RXCH2INT_StartPos  (ULONG)(18)
#define NTN_INTC_INTEXTMASK1_RXCH1INT_StartPos  (ULONG)(17)
#define NTN_INTC_INTEXTMASK1_RXCH0INT_StartPos  (ULONG)(16)
#define NTN_INTC_INTEXTMASK1_TXCH4INT_StartPos  (ULONG)(15)
#define NTN_INTC_INTEXTMASK1_TXCH3INT_StartPos  (ULONG)(14)
#define NTN_INTC_INTEXTMASK1_TXCH2INT_StartPos  (ULONG)(13)
#define NTN_INTC_INTEXTMASK1_TXCH1INT_StartPos  (ULONG)(12)
#define NTN_INTC_INTEXTMASK1_TXCH0INT_StartPos  (ULONG)(11)
#define NTN_INTC_INTEXTMASK1_MAC_StartPos       (ULONG)(10)
#define NTN_INTC_INTEXTMASK1_MACPM_StartPos     (ULONG)(9)
#define NTN_INTC_INTEXTMASK1_MACLPI_StartPos    (ULONG)(8)



#define NTN_INTC_INTEXTMASK1_TXCHINT_Mask (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_TXCHINT_StartPos (ULONG)(11)
#define NTN_INTC_INTEXTMASK1_RXCHINT_Mask (ULONG)(0x1)
#define NTN_INTC_INTEXTMASK1_RXCHINT_StartPos (ULONG)(16)

/* Neutrino wrapper MAC TX DMA CH interrupt mask */
#define NTN_INTC_INTEXTMASK1_TXCHINT_UdfWr(n, data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & (NTN_INTC_INTEXTMASK1_TXCHINT_Mask << (NTN_INTC_INTEXTMASK1_TXCHINT_StartPos + n))) | ((data & NTN_INTC_INTEXTMASK1_TXCHINT_Mask)<<(NTN_INTC_INTEXTMASK1_TXCHINT_StartPos + n)));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_TXCHINT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> (NTN_INTC_INTEXTMASK1_TXCHINT_StartPos + n)) & NTN_INTC_INTEXTMASK1_TXCHINT_Mask);\
} while(0)

/* Neutrino wrapper MAC RX DMA CH interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCHINT_UdfWr(n, data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & (NTN_INTC_INTEXTMASK1_RXCHINT_Mask << (NTN_INTC_INTEXTMASK1_RXCHINT_StartPos + n))) | ((data & NTN_INTC_INTEXTMASK1_RXCHINT_Mask)<<(NTN_INTC_INTEXTMASK1_RXCHINT_StartPos + n)));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCHINT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> (NTN_INTC_INTEXTMASK1_RXCHINT_StartPos + n)) & NTN_INTC_INTEXTMASK1_RXCHINT_Mask);\
} while(0)




/* Neutrino wrapper DMA RX Channel 5 interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCH5INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_RXCH5INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_RXCH5INT_Mask)<<NTN_INTC_INTEXTMASK1_RXCH5INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCH5INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_RXCH5INT_StartPos) & NTN_INTC_INTEXTMASK1_RXCH5INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 4 interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCH4INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_RXCH4INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_RXCH4INT_Mask)<<NTN_INTC_INTEXTMASK1_RXCH4INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCH4INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_RXCH4INT_StartPos) & NTN_INTC_INTEXTMASK1_RXCH4INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 3 interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCH3INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_RXCH3INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_RXCH3INT_Mask)<<NTN_INTC_INTEXTMASK1_RXCH3INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCH3INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_RXCH3INT_StartPos) & NTN_INTC_INTEXTMASK1_RXCH3INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 2 interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCH2INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_RXCH2INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_RXCH2INT_Mask)<<NTN_INTC_INTEXTMASK1_RXCH2INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCH2INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_RXCH2INT_StartPos) & NTN_INTC_INTEXTMASK1_RXCH2INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 1 interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCH1INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_RXCH1INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_RXCH1INT_Mask)<<NTN_INTC_INTEXTMASK1_RXCH1INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCH1INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_RXCH1INT_StartPos) & NTN_INTC_INTEXTMASK1_RXCH1INT_Mask);\
} while(0)

/* Neutrino wrapper DMA RX Channel 0 interrupt mask */
#define NTN_INTC_INTEXTMASK1_RXCH0INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_RXCH0INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_RXCH0INT_Mask)<<NTN_INTC_INTEXTMASK1_RXCH0INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_RXCH0INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_RXCH0INT_StartPos) & NTN_INTC_INTEXTMASK1_RXCH0INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 4 interrupt mask */
#define NTN_INTC_INTEXTMASK1_TXCH4INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_TXCH4INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_TXCH4INT_Mask)<<NTN_INTC_INTEXTMASK1_TXCH4INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_TXCH4INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_TXCH4INT_StartPos) & NTN_INTC_INTEXTMASK1_TXCH4INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 3 interrupt mask */
#define NTN_INTC_INTEXTMASK1_TXCH3INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_TXCH3INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_TXCH3INT_Mask)<<NTN_INTC_INTEXTMASK1_TXCH3INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_TXCH3INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_TXCH3INT_StartPos) & NTN_INTC_INTEXTMASK1_TXCH3INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 2 interrupt mask */
#define NTN_INTC_INTEXTMASK1_TXCH2INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_TXCH2INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_TXCH2INT_Mask)<<NTN_INTC_INTEXTMASK1_TXCH2INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_TXCH2INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_TXCH2INT_StartPos) & NTN_INTC_INTEXTMASK1_TXCH2INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 1 interrupt mask */
#define NTN_INTC_INTEXTMASK1_TXCH1INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_TXCH1INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_TXCH1INT_Mask)<<NTN_INTC_INTEXTMASK1_TXCH1INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_TXCH1INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_TXCH1INT_StartPos) & NTN_INTC_INTEXTMASK1_TXCH1INT_Mask);\
} while(0)

/* Neutrino wrapper DMA TX Channel 0 interrupt mask */
#define NTN_INTC_INTEXTMASK1_TXCH0INT_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_TXCH0INT_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_TXCH0INT_Mask)<<NTN_INTC_INTEXTMASK1_TXCH0INT_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_TXCH0INT_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_TXCH0INT_StartPos) & NTN_INTC_INTEXTMASK1_TXCH0INT_Mask);\
} while(0)

/* Neutrino wrapper MAC interrupt mask */
#define NTN_INTC_INTEXTMASK1_MAC_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_MAC_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_MAC_Mask)<<NTN_INTC_INTEXTMASK1_MAC_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_MAC_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_MAC_StartPos) & NTN_INTC_INTEXTMASK1_MAC_Mask);\
} while(0)

/* Neutrino wrapper MAC PM interrupt mask */
#define NTN_INTC_INTEXTMASK1_MACPM_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_MACPM_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_MACPM_Mask)<<NTN_INTC_INTEXTMASK1_MACPM_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_MACPM_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_MACPM_StartPos) & NTN_INTC_INTEXTMASK1_MACPM_Mask);\
} while(0)

/* Neutrino wrapper MAC LPI interrupt mask */
#define NTN_INTC_INTEXTMASK1_MACLPI_UdfWr(data) do {\
        ULONG v;\
        NTN_INTC_INTEXTMASK1_RgRd(v);\
        v = ((v & NTN_INTC_INTEXTMASK1_MACLPI_Wr_Mask) | ((data & NTN_INTC_INTEXTMASK1_MACLPI_Mask)<<NTN_INTC_INTEXTMASK1_MACLPI_StartPos));\
        NTN_INTC_INTEXTMASK1_RgWr(v);\
} while(0)

#define NTN_INTC_INTEXTMASK1_MACLPI_UdfRd(n, data) do {\
        NTN_INTC_INTEXTMASK1_RgRd(n, data);\
        data = ((data >> NTN_INTC_INTEXTMASK1_MACLPI_StartPos) & NTN_INTC_INTEXTMASK1_MACLPI_Mask);\
} while(0)



/* Neutrino Interrupt MCU Flag */
#define NTN_INTC_MCUFLG_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_MCUFLG_RgOffAddr);\
} while(0)

#define NTN_INTC_MCUFLG_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_MCUFLG_RgOffAddr);\
} while(0)

/* Neutrino Interrupt External Flag */
#define NTN_INTC_EXTFLG_RgWr(data) do {\
        iowrite32(data, (void *)NTN_INTC_EXTFLG_RgOffAddr);\
} while(0)

#define NTN_INTC_EXTFLG_RgRd(data) do {\
        (data) = ioread32((void *)NTN_INTC_EXTFLG_RgOffAddr);\
} while(0)

/******************************************************************************/
/**                          Neutrino PCIe registers                         **/
/******************************************************************************/
#define NTN_PCIE_RANGE_UP_OFFSET_RgOffAddr(no)  ((volatile ULONG *)(PCIE_BASE_ADDRESS + 0x6200 + (no*0x10) ))
#define NTN_PCIE_RANGE_EN_RgOffAddr(no)         ((volatile ULONG *)(PCIE_BASE_ADDRESS + 0x6204 + (no*0x10) ))
#define NTN_PCIE_RANGE_UP_RPLC_RgOffAddr(no)    ((volatile ULONG *)(PCIE_BASE_ADDRESS + 0x6208 + (no*0x10) ))
#define NTN_PCIE_RANGE_WIDTH_RgOffAddr(no)      ((volatile ULONG *)(PCIE_BASE_ADDRESS + 0x620C + (no*0x10) ))

#define NTN_PCIE_RANGE_UP_OFFSET_RgWr(no, data) do {\
        iowrite32(data, (void *)NTN_PCIE_RANGE_UP_OFFSET_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_EN_RgWr(no, data) do {\
        iowrite32(data, (void *)NTN_PCIE_RANGE_EN_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_UP_RPLC_RgWr(no, data) do {\
        iowrite32(data, (void *)NTN_PCIE_RANGE_UP_RPLC_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_WIDTH_RgWr(no, data) do {\
        iowrite32(data, (void *)NTN_PCIE_RANGE_WIDTH_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_UP_OFFSET_RgRd(no, data) do {\
        (data) = ioread32((void *)NTN_PCIE_RANGE_UP_OFFSET_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_EN_RgRd(no, data) do {\
        (data) = ioread32((void *)NTN_PCIE_RANGE_EN_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_UP_RPLC_RgRd(no, data) do {\
        (data) = ioread32((void *)NTN_PCIE_RANGE_UP_RPLC_RgOffAddr(no));\
} while(0)

#define NTN_PCIE_RANGE_WIDTH_RgRd(no, data) do {\
        (data) = ioread32((void *)NTN_PCIE_RANGE_WIDTH_RgOffAddr(no));\
} while(0)


#define MII_AUX_CTRL		0x12	/* Auxillary control register */
