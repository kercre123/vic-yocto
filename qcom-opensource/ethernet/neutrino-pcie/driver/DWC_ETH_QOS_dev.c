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
 *     21-March-2016 : Added print message for "configure_mtl_queue"
 *                     Moved and modified "DWC_ETH_QOS_yinit"
 *                     Modified "ntn_mac_clock_config" for RX clock
 *      3-Jun-2016   : Some Clean up for unused legacy code.
 *                   : Phy reset is allowed now through driver
 *                   : Setting Receiveall mode when promiscuous is requested as per Neutrino requirement
 *                   : Split "configure_mtl_queue" function for TX & RX
 */

/*!@file: DWC_ETH_QOS_dev.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_yapphdr.h"
#include "DWC_ETH_QOS_yregacc.h"
#include <linux/ktime.h>

/*!
* \brief This sequence is used to enable/disable MAC loopback mode
* \param[in] enb_dis
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_mac_loopback_mode(UINT enb_dis, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_MCR_LM_UdfWr(enb_dis);

  return Y_SUCCESS;
}


/* enable/disable PFC(Priority Based Flow Control) */
static void config_pfc(int enb_dis, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_RFCR_PFCE_UdfWr(enb_dis);
}

/*!
* \brief This sequence is used to configure mac double vlan processing feature.
* \param[in] enb_dis
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_tx_outer_vlan(UINT op_type, UINT outer_vlt, struct DWC_ETH_QOS_prv_data *pdata)
{
	NMSGPR_ALERT( "--> config_tx_outer_vlan()\n");

	MAC_VLANTIRR_VLTI_UdfWr(0x0);
	MAC_VLANTIRR_VLT_UdfWr(outer_vlt);
	MAC_VLANTIRR_VLP_UdfWr(0x1);
	MAC_VLANTIRR_VLC_UdfWr(op_type);

	if (op_type == DWC_ETH_QOS_DVLAN_NONE) {
		MAC_VLANTIRR_VLP_UdfWr(0x0);
		MAC_VLANTIRR_VLT_UdfWr(0x0);
	}

	NMSGPR_ALERT( "<-- config_tx_outer_vlan()\n");

	return Y_SUCCESS;
}

static INT config_tx_inner_vlan(UINT op_type, UINT inner_vlt, struct DWC_ETH_QOS_prv_data *pdata)
{
	NMSGPR_ALERT( "--> config_tx_inner_vlan()\n");

	MAC_IVLANTIRR_VLTI_UdfWr(0x0);
	MAC_IVLANTIRR_VLT_UdfWr(inner_vlt);
	MAC_IVLANTIRR_VLP_UdfWr(0x1);
	MAC_IVLANTIRR_VLC_UdfWr(op_type);

	if (op_type == DWC_ETH_QOS_DVLAN_NONE) {
		MAC_IVLANTIRR_VLP_UdfWr(0x0);
		MAC_IVLANTIRR_VLT_UdfWr(0x0);
	}

	NMSGPR_ALERT( "<-- config_tx_inner_vlan()\n");

	return Y_SUCCESS;
}

static INT config_svlan(UINT flags, struct DWC_ETH_QOS_prv_data *pdata)
{
	INT ret = Y_SUCCESS;

	NMSGPR_ALERT( "--> config_svlan()\n");

	MAC_VLANTR_ESVL_UdfWr(1);
	if (flags == DWC_ETH_QOS_DVLAN_NONE) {
		MAC_VLANTR_ESVL_UdfWr(0);
		MAC_IVLANTIRR_CSVL_UdfWr(0);
		MAC_VLANTIRR_CSVL_UdfWr(0);
	} else if (flags == DWC_ETH_QOS_DVLAN_INNER) {
		MAC_IVLANTIRR_CSVL_UdfWr(1);
	} else if (flags == DWC_ETH_QOS_DVLAN_OUTER) {
		MAC_VLANTIRR_CSVL_UdfWr(1);
	} else if (flags == DWC_ETH_QOS_DVLAN_BOTH) {
		MAC_IVLANTIRR_CSVL_UdfWr(1);
		MAC_VLANTIRR_CSVL_UdfWr(1);
	} else {
		NMSGPR_ALERT( "ERROR : double VLAN enable SVLAN configuration - Invalid argument");
		ret = Y_FAILURE;
	}

	NMSGPR_ALERT( "<-- config_svlan()\n");

	return ret;
}

static VOID config_dvlan(bool enb_dis, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_VLANTR_EDVLP_UdfWr(enb_dis);
}


/*!
* \brief This sequence is used to enable/disable ARP offload
* \param[in] enb_dis
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int config_arp_offload(int enb_dis, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_MCR_ARPEN_UdfWr(enb_dis);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update the IP addr into MAC ARP Add reg,
* which is used by MAC for replying to ARP packets
* \param[in] addr
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int update_arp_offload_ip_addr(UCHAR addr[], struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_ARPA_RgWr((addr[3] | (addr[2] << 8) | (addr[1] << 16) | addr[0] << 24));

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to get the status of LPI/EEE mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static u32 get_lpi_status(struct DWC_ETH_QOS_prv_data *pdata)
{
  u32 varmac_lps;

  MAC_LPS_RgRd(varmac_lps);

  return varmac_lps;
}




/*!
* \brief This sequence is used to enable EEE mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int set_eee_mode(struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_LPS_LPIEN_UdfWr(0x1);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to disable EEE mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int reset_eee_mode(struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_LPS_LPITXA_UdfWr(0);
  MAC_LPS_LPIEN_UdfWr(0);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to set PLS bit
* \param[in] phy_link
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int set_eee_pls(int phy_link, struct DWC_ETH_QOS_prv_data *pdata)
{

  if (phy_link == 1) {
    MAC_LPS_PLS_UdfWr(0x1);
  }
  else {
    MAC_LPS_PLS_UdfWr(0);
  }

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to set EEE timer values
* \param[in] lpi_lst
* \param[in] lpi_twt
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int set_eee_timer(int lpi_lst,
                         int lpi_twt, struct DWC_ETH_QOS_prv_data *pdata)
{

  /* mim time(us) for which the MAC waits after it stops transmitting */
  /* the LPI pattern to the PHY and before it resumes the normal transmission. */
  MAC_LPC_TWT_UdfWr(lpi_twt);
  /* mim time(ms) for which the link status from the PHY should be Up before */
  /* the LPI pattern can be transmitted to the PHY. */
  MAC_LPC_TLPIEX_UdfWr(lpi_lst);

  return Y_SUCCESS;
}




static int set_lpi_tx_automate(struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_LPS_LPITXA_UdfWr(0x1);

	return Y_SUCCESS;
}




/*!
* \brief This sequence is used to enable/disable Auto-Negotiation
* and restart the autonegotiation
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int control_an(bool enable, bool restart, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_ANC_ANE_UdfWr(enable);
  MAC_ANC_RAN_UdfWr(restart);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to get Auto-Negotiation advertisment
* pause parameter
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int get_an_adv_pause_param(struct DWC_ETH_QOS_prv_data *pdata)
{
  unsigned long varmac_aad;

  MAC_AAD_RgRd(varmac_aad);

  return GET_VALUE(varmac_aad, MAC_AAD_PSE_LPOS, MAC_AAD_PSE_HPOS);
}




/*!
* \brief This sequence is used to get Auto-Negotiation advertisment
* duplex parameter. Returns one if Full duplex mode is selected
* else returns zero
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int get_an_adv_duplex_param(struct DWC_ETH_QOS_prv_data *pdata)
{
  unsigned long varmac_aad;

  MAC_AAD_RgRd(varmac_aad);
  if (GET_VALUE(varmac_aad, MAC_AAD_FD_LPOS, MAC_AAD_FD_HPOS) == 1) {
    return 1;
  }
  else {
    return 0;
  }
}




/*!
* \brief This sequence is used to get Link partner Auto-Negotiation
* advertisment pause parameter
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int get_lp_an_adv_pause_param(struct DWC_ETH_QOS_prv_data *pdata)
{
  unsigned long varmac_alpa;

  MAC_ALPA_RgRd(varmac_alpa);

  return GET_VALUE(varmac_alpa, MAC_ALPA_PSE_LPOS, MAC_ALPA_PSE_HPOS);
}




/*!
* \brief This sequence is used to get Link partner Auto-Negotiation
* advertisment duplex parameter. Returns one if Full duplex mode
* is selected else returns zero
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int get_lp_an_adv_duplex_param(struct DWC_ETH_QOS_prv_data *pdata)
{
  unsigned long varmac_alpa;

  MAC_ALPA_RgRd(varmac_alpa);
  if (GET_VALUE(varmac_alpa, MAC_ALPA_FD_LPOS, MAC_ALPA_FD_HPOS) == 1) {
    return 1;
  }
  else {
    return 0;
  }
}


static UINT get_vlan_tag_comparison(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT etv;

	MAC_VLANTR_ETV_UdfRd(etv);

	return etv;
}

/*!
* \brief This sequence is used to enable/disable VLAN filtering and
* also selects VLAN filtering mode- perfect/hash
* \param[in] filter_enb_dis
* \param[in] perfect_hash
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_vlan_filtering(INT filter_enb_dis,
                                 INT perfect_hash_filtering,
                                 INT perfect_inverse_match,
                                 struct DWC_ETH_QOS_prv_data *pdata)
{
  MAC_MPFR_VTFE_UdfWr(filter_enb_dis);
  DBGPR_VLAN("%s:VLAN: MPFR_VTFE=%d\n",__func__,filter_enb_dis);
  MAC_VLANTR_VTIM_UdfWr(perfect_inverse_match);
  DBGPR_VLAN("%s:VLAN: TR_VTIMi=%d\n",__func__,perfect_inverse_match);
  MAC_VLANTR_VTHM_UdfWr(perfect_hash_filtering);
  DBGPR_VLAN("%s:VLAN: TR_VTHM=%d\n",__func__,perfect_hash_filtering);
  /* To enable only HASH filtering then VL/VID
   * should be > zero. Hence we are writting 1 into VL.
   * It also means that MAC will always receive VLAN pkt with
   * VID = 1 if inverse march is not set.
   * */
  if (perfect_hash_filtering){
    MAC_VLANTR_VL_UdfWr(0x1);
    DBGPR_VLAN("%s:VLAN: TR_VL=1\n",__func__);
  }

  /* By default enable MAC to calculate vlan hash on
   * only 12-bits of received VLAN tag (ie only on
   * VLAN id and ignore priority and other fields)
   * */
  if (perfect_hash_filtering) {
    MAC_VLANTR_ETV_UdfWr(0x1);
    DBGPR_VLAN("%s:VLAN: TR_ETV=1\n",__func__);
  }

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update the VLAN ID for perfect filtering
* \param[in] vid
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_vlan_id(USHORT vid, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_VLANTR_VL_UdfWr(vid);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update the VLAN Hash Table reg with new VLAN ID
* \param[in] data
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_vlan_hash_table_reg(USHORT data, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_VLANHTR_VLHT_UdfWr(data);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to get the content of VLAN Hash Table reg
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT get_vlan_hash_table_reg(struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG varMAC_VLANHTR;

  MAC_VLANHTR_RgRd(varMAC_VLANHTR);

  return GET_VALUE(varMAC_VLANHTR, MAC_VLANHTR_VLHT_LPOS, MAC_VLANHTR_VLHT_HPOS);
}




/*!
* \brief This sequence is used to update Destination Port Number for
* L4(TCP/UDP) layer filtering
* \param[in] filter_no
* \param[in] port_no
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_l4_da_port_no(INT filter_no,
                                USHORT port_no,
								struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_L4AR_L4DP0_UdfWr(filter_no, port_no);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update Source Port Number for
* L4(TCP/UDP) layer filtering
* \param[in] filter_no
* \param[in] port_no
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_l4_sa_port_no(INT filter_no,
                                USHORT port_no,
								struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_L4AR_L4SP0_UdfWr(filter_no, port_no);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to configure L4(TCP/UDP) filters for
* SA and DA Port Number matching
* \param[in] filter_no
* \param[in] tcp_udp_match
* \param[in] src_dst_port_match
* \param[in] perfect_inverse_match
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_l4_filters(INT filter_no,
		                         INT enb_dis,
                             INT tcp_udp_match,
                             INT src_dst_port_match,
                             INT perfect_inverse_match,
							 struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_L3L4CR_L4PEN0_UdfWr(filter_no, tcp_udp_match);

  if (src_dst_port_match == 0) {
	if (enb_dis == 1) {
		/* Enable L4 filters for SOURCE Port No matching */
		MAC_L3L4CR_L4SPM0_UdfWr(filter_no, 0x1);
		MAC_L3L4CR_L4SPIM0_UdfWr(filter_no, perfect_inverse_match);
    }
    else {
		/* Disable L4 filters for SOURCE Port No matching */
		MAC_L3L4CR_L4SPM0_UdfWr(filter_no, 0x0);
		MAC_L3L4CR_L4SPIM0_UdfWr(filter_no, 0x0);
	}
  }
  else {
	if (enb_dis == 1) {
		/* Enable L4 filters for DESTINATION port No matching */
		MAC_L3L4CR_L4DPM0_UdfWr(filter_no, 0x1);
		MAC_L3L4CR_L4DPIM0_UdfWr(filter_no, perfect_inverse_match);
	}
	else {
		/* Disable L4 filters for DESTINATION port No matching */
		MAC_L3L4CR_L4DPM0_UdfWr(filter_no, 0x0);
		MAC_L3L4CR_L4DPIM0_UdfWr(filter_no, 0x0);
	}
  }

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update IPv6 source/destination Address for L3 layer filtering
* \param[in] filter_no
* \param[in] addr
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_ip6_addr(INT filter_no,
                           USHORT addr[],
						   struct DWC_ETH_QOS_prv_data *pdata)
{
  /* update Bits[31:0] of 128-bit IP addr */
  MAC_L3A0R_RgWr(filter_no, (addr[7] | (addr[6] << 16)));
  /* update Bits[63:32] of 128-bit IP addr */
  MAC_L3A1R_RgWr(filter_no, (addr[5] | (addr[4] << 16)));
  /* update Bits[95:64] of 128-bit IP addr */
  MAC_L3A2R_RgWr(filter_no, (addr[3] | (addr[2] << 16)));
  /* update Bits[127:96] of 128-bit IP addr */
  MAC_L3A3R_RgWr(filter_no, (addr[1] | (addr[0] << 16)));

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update IPv4 destination Address for L3 layer filtering
* \param[in] filter_no
* \param[in] addr
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_ip4_addr1(INT filter_no,
                            UCHAR addr[],
							struct DWC_ETH_QOS_prv_data *pdata)
{
  MAC_L3A1R_RgWr(filter_no, (addr[3] | (addr[2] << 8) | (addr[1] << 16) | (addr[0] << 24)));

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to update IPv4 source Address for L3 layer filtering
* \param[in] filter_no
* \param[in] addr
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT update_ip4_addr0(INT filter_no,
                            UCHAR addr[],
							struct DWC_ETH_QOS_prv_data *pdata)
{
  MAC_L3A0R_RgWr(filter_no, (addr[3] | (addr[2] << 8) | (addr[1] << 16) | (addr[0] << 24)));

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to configure L3(IPv4/IPv6) filters
* for SA/DA Address matching
* \param[in] filter_no
* \param[in] ipv4_ipv6_match
* \param[in] src_dst_addr_match
* \param[in] perfect_inverse_match
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_l3_filters(INT filter_no,
		                         INT enb_dis,
                             INT ipv4_ipv6_match,
                             INT src_dst_addr_match,
                             INT perfect_inverse_match,
							 struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_L3L4CR_L3PEN0_UdfWr(filter_no, ipv4_ipv6_match);

	/* For IPv6 either SA/DA can be checked, not both */
	if (ipv4_ipv6_match == 1) {
		if (enb_dis == 1) {
			if (src_dst_addr_match == 0) {
				/* Enable L3 filters for IPv6 SOURCE addr matching */
				MAC_L3L4CR_L3SAM0_UdfWr(filter_no, 0x1);
				MAC_L3L4CR_L3SAIM0_UdfWr(filter_no, perfect_inverse_match);
				MAC_L3L4CR_L3DAM0_UdfWr(filter_no, 0x0);
				MAC_L3L4CR_L3DAIM0_UdfWr(filter_no, 0x0);
			}
			else {
				/* Enable L3 filters for IPv6 DESTINATION addr matching */
				MAC_L3L4CR_L3SAM0_UdfWr(filter_no, 0x0);
				MAC_L3L4CR_L3SAIM0_UdfWr(filter_no, 0x0);
				MAC_L3L4CR_L3DAM0_UdfWr(filter_no, 0x1);
				MAC_L3L4CR_L3DAIM0_UdfWr(filter_no, perfect_inverse_match);
			}
		}
		else {
			/* Disable L3 filters for IPv6 SOURCE/DESTINATION addr matching */
			MAC_L3L4CR_L3PEN0_UdfWr(filter_no, 0x0);
			MAC_L3L4CR_L3SAM0_UdfWr(filter_no, 0x0);
			MAC_L3L4CR_L3SAIM0_UdfWr(filter_no, 0x0);
			MAC_L3L4CR_L3DAM0_UdfWr(filter_no, 0x0);
			MAC_L3L4CR_L3DAIM0_UdfWr(filter_no, 0x0);
		}
	}
	else {
		if (src_dst_addr_match == 0) {
			if (enb_dis == 1) {
				/* Enable L3 filters for IPv4 SOURCE addr matching */
				MAC_L3L4CR_L3SAM0_UdfWr(filter_no, 0x1);
				MAC_L3L4CR_L3SAIM0_UdfWr(filter_no, perfect_inverse_match);
			}
			else {
				/* Disable L3 filters for IPv4 SOURCE addr matching */
				MAC_L3L4CR_L3SAM0_UdfWr(filter_no, 0x0);
				MAC_L3L4CR_L3SAIM0_UdfWr(filter_no, 0x0);
			}
		}
		else {
			if (enb_dis == 1) {
				/* Enable L3 filters for IPv4 DESTINATION addr matching */
				MAC_L3L4CR_L3DAM0_UdfWr(filter_no, 0x1);
				MAC_L3L4CR_L3DAIM0_UdfWr(filter_no, perfect_inverse_match);
			}
			else {
				/* Disable L3 filters for IPv4 DESTINATION addr matching */
				MAC_L3L4CR_L3DAM0_UdfWr(filter_no, 0x0);
				MAC_L3L4CR_L3DAIM0_UdfWr(filter_no, 0x0);
			}
		}
	}

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

static INT config_mac_pkt_filter_reg(UCHAR pr_mode,
                                     UCHAR huc_mode,
                                     UCHAR hmc_mode,
                                     UCHAR pm_mode,
                                     UCHAR hpf_mode,
									 struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG varMAC_MPFR;

  /* configure device in differnet modes */
  /* promiscuous, hash unicast, hash multicast, */
  /* all multicast and perfect/hash filtering mode. */
  /* NOTE: need to use receive_all instead of promiscuous for neutrino.*/
  MAC_MPFR_RgRd(varMAC_MPFR);
  varMAC_MPFR = varMAC_MPFR & (ULONG)(0x003003e8);
  varMAC_MPFR = varMAC_MPFR | ((pr_mode) << 31) | ((huc_mode) << 1) | ((hmc_mode) << 2) |
                ((pm_mode) << 4) | ((hpf_mode) << 10);
  MAC_MPFR_RgWr(varMAC_MPFR);

  MAC_MPFR_RgRd(varMAC_MPFR);
  NDBGPR_L1("PKT Filter Value : %lx\n", varMAC_MPFR);
  return Y_SUCCESS;
}



/*!
* \brief This sequence is used to enable/disable L3 and L4 filtering
* \param[in] filter_enb_dis
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_l3_l4_filter_enable(INT filter_enb_dis, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_MPFR_IPFE_UdfWr(filter_enb_dis);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to select perfect/inverse matching for L2 DA
* \param[in] perfect_inverse_match
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_l2_da_perfect_inverse_match(INT perfect_inverse_match, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_MPFR_DAIF_UdfWr(perfect_inverse_match);

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

static INT update_mac_addr3_31_low_high_reg(INT idx,
                                            UCHAR addr[],
											struct DWC_ETH_QOS_prv_data *pdata)
{
  if(idx < 3)
	return Y_FAILURE;

  MAC_MA1_31LR_RgWr(idx+3, (addr[0] | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24)));
  MAC_MA1_31HR_ADDRHI_UdfWr(idx+3, (addr[4] | (addr[5] << 8)));
  MAC_MA1_31HR_AE_UdfWr(idx+3, 0x1);

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

static INT update_hash_table_reg(INT idx,
                                 UINT data,
								 struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_HTR_RgWr(idx, data);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used check whether Tx drop status in the
* MTL is enabled or not, returns 1 if it is enabled and 0 if
* it is disabled.
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT drop_tx_status_enabled(struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG varMTL_OMR;

  MTL_OMR_RgRd(varMTL_OMR);

  return GET_VALUE(varMTL_OMR, MTL_OMR_DTXSTS_LPOS, MTL_OMR_DTXSTS_HPOS);
}




/*!
* \brief This sequence is used configure MAC SSIR
* \param[in] ptp_clock
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_sub_second_increment(ULONG ptp_clock, struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG val;
  ULONG varMAC_TCR;

  MAC_TCR_RgRd(varMAC_TCR);

  /* convert the PTP_CLOCK to nano second */
  /*  formula is : ((1/ptp_clock) * 1000000000) */
	/*  where, ptp_clock = 50MHz if FINE correction */
	/*  and ptp_clock = DWC_ETH_QOS_SYSCLOCK if COARSE correction */
  if (GET_VALUE(varMAC_TCR, MAC_TCR_TSCFUPDT_LPOS, MAC_TCR_TSCFUPDT_HPOS) == 1) {
    val = ((1 * 1000000000ull) / 50000000);
  }
  else {
    val = ((1 * 1000000000ull) / ptp_clock);
  }

  /* 0.465ns accurecy */
  if (GET_VALUE(varMAC_TCR, MAC_TCR_TSCTRLSSR_LPOS, MAC_TCR_TSCTRLSSR_HPOS) == 0) {
    val = (val * 1000) / 465;
  }
  MAC_SSIR_SSINC_UdfWr(val);

  return Y_SUCCESS;
}



/*!
* \brief This sequence is used get 64-bit system time in nano sec
* \return (unsigned long long) on success
* \retval ns
*/

static ULONG_LONG get_systime(struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG ns;
  ULONG varmac_stnsr;
  ULONG varmac_stsr1, varmac_stsr2;

  /* Read the seconds once */
  MAC_STSR_RgRd(varmac_stsr1);

  while (1) {
    /* Read the nanoseconds */
    MAC_STNSR_RgRd(varmac_stnsr);
    ns = GET_VALUE(varmac_stnsr, MAC_STNSR_TSSS_LPOS, MAC_STNSR_TSSS_HPOS);

    /* Read the seconds again */
    MAC_STSR_RgRd(varmac_stsr2);

    /* If the seconds didn't roll over, break and return the time */
    if (varmac_stsr1 == varmac_stsr2) {
      break;
    }

    /* If the seconds did roll over, read the time again */
    varmac_stsr1 = varmac_stsr2;
  }

  /* convert sec/high time value to nanosecond */
  return ((ULONG_LONG) ns) + (varmac_stsr1 * 1000000000ull);
}






/*!
* \brief This sequence is used to adjust/update the system time
* \param[in] sec
* \param[in] nsec
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT adjust_systime(UINT sec,
						UINT nsec,
			  			INT add_sub,
						bool one_nsec_accuracy,
						struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG retryCount = 100000;
  ULONG vy_count;
  volatile ULONG varMAC_TCR;
  u64 data = 0;

  /* wait for previous(if any) time adjust/update to complete. */

  /*Poll*/
  vy_count = 0;
  while(1){
    if(vy_count > retryCount) {
      return -Y_FAILURE;
    }

    MAC_TCR_RgRd(varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSUPDT_LPOS, MAC_TCR_TSUPDT_HPOS) == 0) {
      break;
    }
    vy_count++;
		mdelay(1);
  }

  if (add_sub) {
    /* If the new sec value needs to be subtracted with
     * the system time, then MAC_STSUR reg should be
     * programmed with (2^32 – <new_sec_value>)
     * */
    sec = (0x100000000ull - sec);

    /* If the new nsec value need to be subtracted with
     * the system time, then MAC_STNSUR.TSSS field should be
     * programmed with,
     * (10^9 - <new_nsec_value>) if MAC_TCR.TSCTRLSSR is set or
     * (2^31 - <new_nsec_value> if MAC_TCR.TSCTRLSSR is reset)
     * */
  	if (one_nsec_accuracy)
      nsec = (0x3B9ACA00 - nsec);
   	else
      nsec = (0x80000000 - nsec);
  }

  MAC_STSUR_RgWr(sec);
  MAC_STNSUR_TSSS_UdfWr(nsec);
  MAC_STNSUR_ADDSUB_UdfWr(add_sub);

  data = (u64)(sec * 1000000000);
  data = (u64)(data + nsec);
  NTN_PTPLCLUPDT_RgWr((u32)data);


  /* issue command to initialize system time with the value */
  /* specified in MAC_STSUR and MAC_STNSUR. */
  MAC_TCR_TSUPDT_UdfWr(0x1);
  /* wait for present time initialize to complete. */

  /*Poll*/
  vy_count = 0;
  while(1){
    if(vy_count > retryCount) {
      return -Y_FAILURE;
    }

    MAC_TCR_RgRd(varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSUPDT_LPOS, MAC_TCR_TSUPDT_HPOS) == 0) {
      break;
    }
    vy_count++;
		mdelay(1);
  }

  return Y_SUCCESS;
}



/*!
* \brief This sequence is used to adjust the ptp operating frequency.
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_addend(UINT data, struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG retryCount = 100000;
  ULONG vy_count;
  volatile ULONG varMAC_TCR;

  /* wait for previous(if any) added update to complete. */

  /*Poll*/
  vy_count = 0;
  while(1){
    if(vy_count > retryCount) {
      return -Y_FAILURE;
    }

    MAC_TCR_RgRd(varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSADDREG_LPOS, MAC_TCR_TSADDREG_HPOS) == 0) {
      break;
    }
    vy_count++;
    mdelay(1);
  }

  MAC_TAR_RgWr(data);
  /* issue command to update the added value */
  MAC_TCR_TSADDREG_UdfWr(0x1);
  /* wait for present added update to complete. */

  /*Poll*/
  vy_count = 0;
  while(1){
    if(vy_count > retryCount) {
      return -Y_FAILURE;
    }
    MAC_TCR_RgRd(varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSADDREG_LPOS, MAC_TCR_TSADDREG_HPOS) == 0) {
      break;
    }
    vy_count++;
    mdelay(1);
  }

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to initialize the system time
* \param[in] sec
* \param[in] nsec
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT init_systime(UINT sec,
                        UINT nsec,
						struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG retryCount = 100000;
  ULONG vy_count;
  volatile ULONG varMAC_TCR;
  u64 data = 0;

  /* wait for previous(if any) time initialize to complete. */

  /*Poll*/
  vy_count = 0;
  while(1){
    if(vy_count > retryCount) {
      return -Y_FAILURE;
    }

    MAC_TCR_RgRd(varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSINIT_LPOS, MAC_TCR_TSINIT_HPOS) == 0) {
      break;
    }
    vy_count++;
    mdelay(1);
  }
  MAC_STSUR_RgWr(sec);
  MAC_STNSUR_RgWr(nsec);

  data = (u64)(sec * 1000000000);
  data = (u64)(data + nsec);
  NTN_PTPLCLINIT_RgWr((u32)data);

  /* issue command to initialize system time with the value */
  /* specified in MAC_STSUR and MAC_STNSUR. */
  MAC_TCR_TSINIT_UdfWr(0x1);
  /* wait for present time initialize to complete. */

  /*Poll*/
  vy_count = 0;
  while(1){
    if(vy_count > retryCount) {
      return -Y_FAILURE;
    }

    MAC_TCR_RgRd(varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSINIT_LPOS, MAC_TCR_TSINIT_HPOS) == 0) {
      break;
    }
    vy_count++;
    mdelay(1);
  }

  return Y_SUCCESS;
}





/*!
* \brief This sequence is used to enable HW time stamping
* and receive frames
* \param[in] count
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_hw_time_stamping(UINT config_val, struct DWC_ETH_QOS_prv_data *pdata)
{

  MAC_TCR_RgWr(config_val);

  return Y_SUCCESS;
}





/*!
* \brief This sequence is used get the 64-bit of the timestamp
* captured by the device for the corresponding received packet
* in nanosecond.
* \param[in] rxdesc
* \return (unsigned long long) on success
* \retval ns
*/

static ULONG_LONG get_rx_tstamp(t_RX_CONTEXT_DESC *rxdesc)
{
  ULONG_LONG ns;
  ULONG varrdes1;

  RX_CONTEXT_DESC_RDES0_Ml_Rd(rxdesc->RDES0, ns);
  RX_CONTEXT_DESC_RDES1_Ml_Rd(rxdesc->RDES1, varrdes1);
  ns = ns + (varrdes1 * 1000000000ull);

  return ns;
}





/*!
* \brief This sequence is used to check whether the captured timestamp
* for the corresponding received packet is valid or not.
* Returns 0 if no context descriptor
* Returns 1 if timestamp is valid
* Returns 2 if time stamp is corrupted
* \param[in] rxdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static UINT get_rx_tstamp_status(t_RX_CONTEXT_DESC *rxdesc)
{
  UINT varOWN;
  UINT varCTXT;
  UINT varRDES0;
  UINT varRDES1;

  /* check for own bit and CTXT bit */
  RX_CONTEXT_DESC_RDES3_OWN_Mlf_Rd(rxdesc->RDES3, varOWN);
  RX_CONTEXT_DESC_RDES3_CTXT_Mlf_Rd(rxdesc->RDES3, varCTXT);
  if ((varOWN == 0) && (varCTXT == 0x1)) {
    RX_CONTEXT_DESC_RDES0_Ml_Rd(rxdesc->RDES0, varRDES0);
    RX_CONTEXT_DESC_RDES1_Ml_Rd(rxdesc->RDES1, varRDES1);
    if ((varRDES0 == 0xffffffff) && (varRDES1 == 0xffffffff)) {
      /* time stamp is corrupted */
      return 2;
    }
    else {
      /* time stamp is valid */
      return 1;
    }
  }
  else {
    /* no CONTEX desc to hold time stamp value */
    return 0;
  }
}




/*!
* \brief This sequence is used to check whether the timestamp value
* is available in a context descriptor or not. Returns 1 if timestamp
* is available else returns 0
* \param[in] rxdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static UINT rx_tstamp_available(t_RX_NORMAL_DESC *rxdesc)
{
  UINT varRS1V;
  UINT varTSA;

  RX_NORMAL_DESC_RDES3_RS1V_Mlf_Rd(rxdesc->RDES3, varRS1V);
  if (varRS1V == 1) {
    RX_NORMAL_DESC_RDES1_TSA_Mlf_Rd(rxdesc->RDES1, varTSA);
    return varTSA;
  }
  else {
    return 0;
  }
}




/*!
* \brief This sequence is used get the least 64-bit of the timestamp
* captured by the device for the corresponding transmit packet in nanosecond
* \return (unsigned long long) on success
* \retval ns
*/

static ULONG_LONG get_tx_tstamp_via_reg(struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG_LONG ns;
  ULONG varmac_ttn;

  MAC_TTSN_TXTSSTSLO_UdfRd(ns);
  MAC_TTN_TXTSSTSHI_UdfRd(varmac_ttn);
  ns = ns + (varmac_ttn * 1000000000ull);

  return ns;
}






/*!
* \brief This sequence is used to check whether a timestamp has been
* captured for the corresponding transmit packet. Returns 1 if
* timestamp is taken else returns 0
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static UINT get_tx_tstamp_status_via_reg(struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG varMAC_TCR;
  ULONG varMAC_TTSN;

  /* device is configured to overwrite the timesatmp of */
  /* eariler packet if driver has not yet read it. */
  MAC_TCR_RgRd(varMAC_TCR);
  if (GET_VALUE(varMAC_TCR, MAC_TCR_TXTSSTSM_LPOS, MAC_TCR_TXTSSTSM_HPOS) == 1) {
    /* nothing to do */
  }
  else {
    /* timesatmp of the current pkt is ignored or not captured */
    MAC_TTSN_RgRd(varMAC_TTSN);
    if (GET_VALUE(varMAC_TTSN, MAC_TTSN_TXTSSTSMIS_LPOS, MAC_TTSN_TXTSSTSMIS_HPOS) == 1) {
      return 0;
    }
    else {
      return 1;
    }
  }

  return 0;
}





/*!
* \brief This sequence is used get the 64-bit of the timestamp captured
* by the device for the corresponding transmit packet in nanosecond.
* \param[in] txdesc
* \return (unsigned long long) on success
* \retval ns
*/

static ULONG_LONG get_tx_tstamp(t_TX_NORMAL_DESC *txdesc)
{
  ULONG_LONG ns;
  ULONG vartdes1;

  TX_NORMAL_DESC_TDES0_Ml_Rd(txdesc->TDES0, ns);
  TX_NORMAL_DESC_TDES1_Ml_Rd(txdesc->TDES1, vartdes1);
  ns = ns + (vartdes1 * 1000000000ull);

  return ns;
}




/*!
* \brief This sequence is used to check whether a timestamp has been
* captured for the corresponding transmit packet. Returns 1 if
* timestamp is taken else returns 0
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static UINT get_tx_tstamp_status(t_TX_NORMAL_DESC *txdesc)
{
  UINT varTDES3;

  TX_NORMAL_DESC_TDES3_Ml_Rd(txdesc->TDES3, varTDES3);

  return (varTDES3 & 0x20000);
}



/*!
* \brief This sequence is used to set tx queue operating mode for Queue[0 - 7]
* \param[in] chInx
* \param[in] q_mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_tx_queue_operating_mode(UINT chInx,
                                       UINT q_mode,
									   struct DWC_ETH_QOS_prv_data *pdata)
{

  MTL_QTOMR_TXQEN_UdfWr(chInx, q_mode);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to select Tx Scheduling Algorithm for AVB feature for Queue[1 - 7]
* \param[in] avb_algo
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_avb_algorithm(UINT chInx, UCHAR avb_algo, struct DWC_ETH_QOS_prv_data *pdata)
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

static INT config_credit_control(UINT chInx, UINT cc, struct DWC_ETH_QOS_prv_data *pdata)
{

  MTL_QECR_CC_UdfWr(chInx, cc);

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

static INT config_send_slope(UINT chInx,
                          UINT sendSlope,
						  struct DWC_ETH_QOS_prv_data *pdata)
{
  NDBGPR_L1("send slop  %08x\n",sendSlope);
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

static INT config_idle_slope(UINT chInx,
                          UINT idleSlope,
						  struct DWC_ETH_QOS_prv_data *pdata)
{
  NDBGPR_L1("Idle slop  %08x\n",idleSlope);
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

static INT config_low_credit(UINT chInx,
							UINT lowCredit,
							struct DWC_ETH_QOS_prv_data *pdata)
{
	INT lowCredit_neg = lowCredit;
	NDBGPR_L1( "lowCreidt = %08x lowCredit_neg:%08x\n",
			lowCredit, lowCredit_neg);
	MTL_QLCR_LC_UdfWr(chInx, lowCredit_neg);

  MTL_QLCR_LC_UdfWr(chInx, lowCredit);

  return Y_SUCCESS;
}


/*!
* \brief This sequence is used to enable/disable slot number check When set,
* this bit enables the checking of the slot number programmed in the TX
* descriptor with the current reference given in the RSN field. The DMA fetches
* the data from the corresponding buffer only when the slot number is: equal to
* the reference slot number or  ahead of the reference slot number by one.
*
* \param[in] chInx
* \param[in] slot_check
*
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_slot_num_check(UINT chInx, UCHAR slot_check)
{

  NMSGPR_ALERT( "ALERT : Function not supported: %s: %d\n", __FUNCTION__, __LINE__);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to enable/disable advance slot check When set,
* this bit enables the DAM to fetch the data from the buffer when the slot
* number programmed in TX descriptor is equal to the reference slot number
* given in RSN field or ahead of the reference number by upto two slots
*
* \param[in] chInx
* \param[in] adv_slot_check
*
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_advance_slot_num_check(UINT chInx, UCHAR adv_slot_check)
{

  NMSGPR_ALERT( "ALERT : Function not supported: %s: %d\n", __FUNCTION__, __LINE__);

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

static INT config_high_credit(UINT chInx,
							 UINT hiCredit,
							 struct DWC_ETH_QOS_prv_data *pdata)
{
   NDBGPR_L1( "hiCreidt = %08x \n",hiCredit);

  MTL_QHCR_HC_UdfWr(chInx, hiCredit);

  return Y_SUCCESS;
}

/*!
* \brief This sequence is used to set weights for DCB feature for Queue[0 - 7]
* \param[in] chInx
* \param[in] q_weight
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_dcb_queue_weight(UINT chInx,
                                UINT q_weight,
								struct DWC_ETH_QOS_prv_data *pdata)
{

  MTL_QW_ISCQW_UdfWr(chInx, q_weight);

  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to select Tx Scheduling Algorithm for DCB feature
* \param[in] dcb_algo
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_dcb_algorithm(UCHAR dcb_algo, struct DWC_ETH_QOS_prv_data *pdata)
{

  MTL_OMR_SCHALG_UdfWr(dcb_algo);

  return Y_SUCCESS;
}

/*!
* \brief This sequence is used to get Tx queue count
* \param[in] count
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

UCHAR get_tx_queue_count(ULONG dwc_eth_ntn_reg_pci_base_addr)
{
	UCHAR count;
  ULONG varMAC_HFR2;

  MAC_HFR2_NOPRV_RgRd(varMAC_HFR2);
  count = GET_VALUE(varMAC_HFR2, MAC_HFR2_TXQCNT_LPOS, MAC_HFR2_TXQCNT_HPOS);

  return (count + 1);
}




/*!
* \brief This sequence is used to get Rx queue count
* \param[in] count
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

UCHAR get_rx_queue_count(ULONG dwc_eth_ntn_reg_pci_base_addr)
{
	UCHAR count;
  ULONG varMAC_HFR2;

  MAC_HFR2_NOPRV_RgRd(varMAC_HFR2);
  count = GET_VALUE(varMAC_HFR2, MAC_HFR2_RXQCNT_LPOS, MAC_HFR2_RXQCNT_HPOS);

  return (count + 1);
}




/*!
* \brief This sequence is used to disables all Tx/Rx MMC interrupts
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_mmc_interrupts(struct DWC_ETH_QOS_prv_data *pdata)
{

  /* disable all TX interrupts */
  MMC_INTR_MASK_TX_RgWr(0xffffffff);
  /* disable all RX interrupts */
  MMC_INTR_MASK_RX_RgWr(0xffffffff);
  MMC_IPC_INTR_MASK_RX_RgWr(0xffffffff); /* Disable MMC Rx Interrupts for IPC */
  return Y_SUCCESS;
}




/*!
* \brief This sequence is used to configure MMC counters
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_mmc_counters(struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG varMMC_CNTRL;

  /* set COUNTER RESET */
  /* set RESET ON READ */
  /* set COUNTER PRESET */
  /* set FULL_HALF PRESET */
  MMC_CNTRL_RgRd(varMMC_CNTRL);
  varMMC_CNTRL = varMMC_CNTRL & (ULONG)(0x10a);
  varMMC_CNTRL = varMMC_CNTRL | ((0x1) << 0) | ((0x1) << 2) | ((0x1) << 4) |
                ((0x1) << 5);
  MMC_CNTRL_RgWr(varMMC_CNTRL);


  return Y_SUCCESS;
}



/*!
* \brief This sequence is used to disable given DMA channel rx interrupts
* \param[in] chInx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_rx_interrupt(UINT chInx,struct DWC_ETH_QOS_prv_data *pdata)
{
    /* Disable receive complete interrupt*/
    DMA_RXCHINTMASK_RCEN_UdfWr(chInx, 0);

    return Y_SUCCESS;
}




/*!
* \brief This sequence is used to enable given DMA channel rx interrupts
* \param[in] chInx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_rx_interrupt(UINT chInx,struct DWC_ETH_QOS_prv_data *pdata)
{
    /* Enable receive complete interrupt */
    DMA_RXCHINTMASK_RCEN_UdfWr(chInx, 0x1);

  return Y_SUCCESS;
}


static VOID configure_sa_via_reg(u32 cmd, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_MCR_SARC_UdfWr(cmd);
}

static VOID configure_mac_addr1_reg(UCHAR *mac_addr, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_MA1HR_RgWr(((mac_addr[5] << 8) | (mac_addr[4])));
	MAC_MA1LR_RgWr(((mac_addr[3] << 24) | (mac_addr[2] << 16) |
			(mac_addr[1] << 8) | (mac_addr[0])));
}

static VOID configure_mac_addr0_reg(UCHAR *mac_addr, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_MA0HR_RgWr(((mac_addr[5] << 8) | (mac_addr[4])));
	MAC_MA0HR_AE_UdfWr(0x1);
	MAC_MA0LR_RgWr(((mac_addr[3] << 24) | (mac_addr[2] << 16) |
			(mac_addr[1] << 8) | (mac_addr[0])));
}

static VOID config_rx_outer_vlan_stripping(u32 cmd, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_VLANTR_EVLS_UdfWr(cmd);
}

static VOID config_rx_inner_vlan_stripping(u32 cmd, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_VLANTR_EIVLS_UdfWr(cmd);
}

static VOID config_ptpoffload_engine(UINT pto_cr, UINT mc_uc, struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_PTO_CR_RgWr(pto_cr);
    MAC_TCR_TSENMACADDR_UdfWr(mc_uc);
}


static VOID configure_reg_vlan_control(struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data, struct DWC_ETH_QOS_prv_data *pdata)
{
	USHORT vlan_id = desc_data->vlan_tag_id;
	UINT vlan_control = desc_data->tx_vlan_tag_ctrl;

	MAC_VLANTIRR_RgWr(((1 << 18) | (vlan_control << 16) | (vlan_id << 0)));
}

static VOID configure_desc_vlan_control(struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_VLANTIRR_RgWr((1 << 20));
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT configure_mac_for_vlan_pkt(struct DWC_ETH_QOS_prv_data *pdata)
{
	/* Enable VLAN Tag stripping (except for loopback testing)*/
#ifdef NTN_DRV_TEST_LOOPBACK
	MAC_VLANTR_EVLS_UdfWr(0x0);
#else
	MAC_VLANTR_EVLS_UdfWr(0x0);
#endif
	/* Enable operation on the outer VLAN Tag, if present */
	MAC_VLANTR_ERIVLT_UdfWr(0);
	DBGPR_VLAN("%s:VLAN: TR_ERIVLT=0\n",__func__);
	/* Disable double VLAN Tag processing on TX and RX */
	MAC_VLANTR_EDVLP_UdfWr(0);
	DBGPR_VLAN("%s:VLAN: TR_EDVLP=0\n",__func__);
	/* Enable VLAN Tag in RX Status. */
	MAC_VLANTR_EVLRXS_UdfWr(0x1);
	DBGPR_VLAN("%s:VLAN: TR_EVLRXS=1\n",__func__);
	/* Disable VLAN Type Check */
	MAC_VLANTR_DOVLTC_UdfWr(0x1);
	DBGPR_VLAN("%s:VLAN: TR_DOVLTC=1\n",__func__);

	/* configure MAC to get VLAN Tag to be inserted/replaced from */
	/* TX descriptor(context desc) */
	MAC_VLANTIRR_VLTI_UdfWr(0x1);
	DBGPR_VLAN("%s:VLAN: TIRR_VLTI=1\n",__func__);
	/* insert/replace C_VLAN in 13th ans 14th bytes of transmitted frames */
	MAC_VLANTIRR_CSVL_UdfWr(0);
	DBGPR_VLAN("%s:VLAN: TIRR_CSVL=0\n",__func__);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_pblx8(UINT chInx, UINT val)
{
    NMSGPR_ALERT( "ALERT : Function not supported: %s: %d\n", __FUNCTION__, __LINE__);

	return Y_SUCCESS;
}

/*!
* \return INT
* \retval programmed Tx PBL value
*/

static INT get_tx_pbl_val(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT tx_pbl;

  	DMA_TXCHCTL_TXPBL_UdfRd(chInx, tx_pbl);

	return tx_pbl;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_tx_pbl_val(UINT chInx, UINT tx_pbl, struct DWC_ETH_QOS_prv_data *pdata)
{
  	DMA_TXCHCTL_TXPBL_UdfWr(chInx, tx_pbl);

	return Y_SUCCESS;
}

/*!
* \return INT
* \retval programmed Rx PBL value
*/

static INT get_rx_pbl_val(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT rx_pbl;

  	DMA_RXCHCTL_RXPBL_UdfRd(chInx, rx_pbl);

	return rx_pbl;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_rx_pbl_val(UINT chInx, UINT rx_pbl, struct DWC_ETH_QOS_prv_data *pdata)
{
  	DMA_RXCHCTL_RXPBL_UdfWr(chInx, rx_pbl);

	return Y_SUCCESS;
}


/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_axi_rorl_val(UINT axi_rorl, struct DWC_ETH_QOS_prv_data *pdata)
{
        DMA_BUSCFG_RD_OSR_LMT_UdfWr(axi_rorl);

	return Y_SUCCESS;
}


/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_axi_worl_val(UINT axi_worl, struct DWC_ETH_QOS_prv_data *pdata)
{
        DMA_BUSCFG_WR_OSR_LMT_UdfWr(axi_worl);

	return Y_SUCCESS;
}


/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_axi_pbl_val(UINT axi_pbl, struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT varDMA_SBUS;

	DMA_BUSCFG_RgRd(varDMA_SBUS);
	varDMA_SBUS &= ~DMA_BUSCFG_AXI_PBL_MASK;
	varDMA_SBUS |= axi_pbl;
	DMA_BUSCFG_RgWr(varDMA_SBUS);

	return Y_SUCCESS;
}


/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_incr_incrx_mode(UINT val, struct DWC_ETH_QOS_prv_data *pdata)
{
	DMA_BUSCFG_FB_UdfWr(val);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_osf_mode(UINT chInx, UINT val, struct DWC_ETH_QOS_prv_data *pdata)
{
  	DMA_TXCHCTL_OSP_UdfWr(chInx, val);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_rsf_mode(UINT chInx, UINT val, struct DWC_ETH_QOS_prv_data *pdata)
{
	//if (chInx == 0) {
		//MTL_Q0ROMR_RSF_UdfWr(val);
	//}
	//else {
		MTL_QROMR_RSF_UdfWr(chInx, val);
	//}

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_tsf_mode(UINT chInx, UINT val, struct DWC_ETH_QOS_prv_data *pdata)
{
	MTL_QTOMR_TSF_UdfWr(chInx, val);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_rx_threshold(UINT chInx, UINT val, struct DWC_ETH_QOS_prv_data *pdata)
{
	MTL_QROMR_RTC_UdfWr(chInx, val);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_tx_threshold(UINT chInx, UINT val, struct DWC_ETH_QOS_prv_data *pdata)
{
	MTL_QTOMR_TTC_UdfWr(chInx, val);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT config_rx_watchdog_timer(UINT chInx, u32 riwt, struct DWC_ETH_QOS_prv_data *pdata)
{
        DMA_RXCH_CUR_WATCHDOG_TIMER_RWT_UdfWr(chInx, riwt);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_magic_pmt_operation(struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_PMTCSR_MGKPKTEN_UdfWr(0x1);
	MAC_PMTCSR_PWRDWN_UdfWr(0x1);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_magic_pmt_operation(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT varPMTCSR_PWRDWN;

	MAC_PMTCSR_MGKPKTEN_UdfWr(0x0);
	MAC_PMTCSR_PWRDWN_UdfRd(varPMTCSR_PWRDWN);
	if (varPMTCSR_PWRDWN == 0x1) {
		MAC_PMTCSR_PWRDWN_UdfWr(0x0);
	}

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_remote_pmt_operation(struct DWC_ETH_QOS_prv_data *pdata)
{
	MAC_PMTCSR_RWKPKTEN_UdfWr(0x1);
	MAC_PMTCSR_PWRDWN_UdfWr(0x1);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_remote_pmt_operation(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT varPMTCSR_PWRDWN;

	MAC_PMTCSR_RWKPKTEN_UdfWr(0x0);
	MAC_PMTCSR_PWRDWN_UdfRd(varPMTCSR_PWRDWN);
	if (varPMTCSR_PWRDWN == 0x1) {
		MAC_PMTCSR_PWRDWN_UdfWr(0x0);
	}

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT configure_rwk_filter_registers(UINT *value, UINT count, struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT i;

	MAC_PMTCSR_RWKFILTRST_UdfWr(1);
	for (i = 0; i < count; i++)
		MAC_RWPFFR_RgWr(value[i]);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_tx_flow_ctrl(UINT qInx, struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_QTFCR_TFE_UdfWr(qInx, 0);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_tx_flow_ctrl(UINT qInx, struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_QTFCR_TFE_UdfWr(qInx, 1);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_rx_flow_ctrl(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_RFCR_RFE_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_rx_flow_ctrl(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_RFCR_RFE_UdfWr(0x1);

	return Y_SUCCESS;
}

/*!
* \param[in] chInx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT stop_dma_rx(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG retryCount = 10;
  ULONG vy_count;

  volatile ULONG varDMA_RXCHSTS;

  /* issue Rx dma stop command */
  DMA_RXCHCTL_ST_UdfWr(chInx, 0);

  /* wait for Rx DMA to stop, ie wait till Rx DMA
   * goes in either Running or Suspend state.
   * */

    /*Poll*/
    vy_count = 0;
    while(1){
        if(vy_count > retryCount) {
            NMSGPR_ALERT( "ERROR: Rx Channel 0 stop failed, DSR0 = %#lx\n",
                varDMA_RXCHSTS);
            return -Y_FAILURE;
        }

        DMA_RXCHSTS_RgRd(chInx, varDMA_RXCHSTS);
        if ((GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_CHSTS_LPOS, DMA_RXCHSTS_CHSTS_HPOS) == 0x2)
            || (GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_CHSTS_LPOS, DMA_RXCHSTS_CHSTS_HPOS) == 0x5)
            || (GET_VALUE(varDMA_RXCHSTS, DMA_RXCHSTS_CHSTS_LPOS, DMA_RXCHSTS_CHSTS_HPOS) == 0x0)) {
            break;
        }
        vy_count++;
        mdelay(1);
    }

  return Y_SUCCESS;
}


/*!
* \param[in] chInx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT start_dma_rx(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{

    DMA_RXCHCTL_ST_UdfWr(chInx, 0x1);

  return Y_SUCCESS;
}

/*!
* \param[in] chInx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT stop_dma_tx(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{
  ULONG retryCount = 10;
  ULONG vy_count;

  volatile ULONG varDMA_TXCHSTS;

  /* issue Tx dma stop command */
  DMA_TXCHCTL_ST_UdfWr(chInx, 0);

  /* wait for Tx DMA to stop, ie wait till Tx DMA
   * goes in Suspend state or stopped state.
   */

    /*Poll*/
    vy_count = 0;
    while(1){
        if(vy_count > retryCount) {
        NMSGPR_ALERT( "ERROR: Channel 0 stop failed, DSR0 = %lx\n",
            varDMA_TXCHSTS);
            return -Y_FAILURE;
        }

        DMA_TXCHSTS_RgRd(chInx, varDMA_TXCHSTS);
        if ((GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_CHSTS_LPOS, DMA_TXCHSTS_CHSTS_HPOS) == 0x5) ||
            (GET_VALUE(varDMA_TXCHSTS, DMA_TXCHSTS_CHSTS_LPOS, DMA_TXCHSTS_CHSTS_LPOS) == 0x0)) {
            break;
        }
        vy_count++;
        mdelay(1);
    }

  return Y_SUCCESS;
}

/*!
* \param[in] chInx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT start_dma_tx(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{

  DMA_TXCHCTL_ST_UdfWr(chInx, 0x1);

  return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT stop_mac_tx_rx(struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG varMAC_MCR;

	MAC_MCR_RgRd(varMAC_MCR);
	varMAC_MCR = varMAC_MCR & (ULONG) (0xffffff7c);
	varMAC_MCR = varMAC_MCR | ((0) << 1) | ((0) << 0);
	MAC_MCR_RgWr(varMAC_MCR);

	return Y_SUCCESS;
}

/*!
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT start_mac_tx_rx(struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG varMAC_MCR;

	MAC_MCR_RgRd(varMAC_MCR);
	varMAC_MCR = varMAC_MCR & (ULONG) (0xffffff7c);
	varMAC_MCR = varMAC_MCR | ((0x1) << 1) | ((0x1) << 0);
	MAC_MCR_RgWr(varMAC_MCR);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to enable DMA TX interrupts
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_dma_tx_interrupts(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG varDMA_TXCHSTS;
	ULONG varDMA_TXCHINTMASK;

	/* clear all the interrupts which are set */
    DMA_TXCHSTS_RgRd(chInx, varDMA_TXCHSTS);
    DMA_TXCHSTS_RgWr(chInx, varDMA_TXCHSTS);

	/* Enable all interrupts for Queue */
    varDMA_TXCHINTMASK = DMA_TXCHINTMASK_MASK;
#if 0
#ifndef DWC_ETH_QOS_TXPOLLING_MODE_ENABLE
	/* TIE - Disable Transmit Interrupt Enable */
	varDMA_TXCHINTMASK &= ~((0x1) << 0);
#endif
#endif

    /* INTC register */
    //NTN_INTC_INTMCUMASK1_TXCHINT_UdfWr(chInx, 0x1);

    /* MAC Wrapper register */

	//NDBGPR_L1("------- CH INX = %d : %#x\n",chInx, varDMA_TXCHINTMASK);
    DMA_TXCHINTMASK_RgWr(chInx, varDMA_TXCHINTMASK);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to enable DMA RX interrupts
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_dma_rx_interrupts(UINT chInx, struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG varDMA_RXCHSTS;
	ULONG varDMA_RXCHINTMASK;

	/* clear all the interrupts which are set */
    DMA_RXCHSTS_RgRd(chInx, varDMA_RXCHSTS);
    DMA_RXCHSTS_RgWr(chInx, varDMA_RXCHSTS);

	/* Enable all interrupts for Queue */
    varDMA_RXCHINTMASK = DMA_RXCHINTMASK_MASK;

    /* INTC register */
    //NTN_INTC_INTMCUMASK1_RXCHINT_UdfWr(chInx, 0x1);

    /* MAC Wrapper register */
    DMA_RXCHINTMASK_RgWr(chInx, varDMA_RXCHINTMASK);

	return Y_SUCCESS;
}


/*!
* \brief This sequence is used to configure the MAC registers for
* GMII-1000Mbps speed
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_gmii_speed(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MCR_PS_UdfWr(0);
	MAC_MCR_FES_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the MAC registers for
* MII-10Mpbs speed
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_mii_speed_10(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MCR_PS_UdfWr(0x1);
	MAC_MCR_FES_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the MAC registers for
* MII-100Mpbs speed
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_mii_speed_100(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MCR_PS_UdfWr(0x1);
	MAC_MCR_FES_UdfWr(0x1);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the MAC registers for
* half duplex mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_half_duplex(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MCR_DM_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the MAC registers for
* full duplex mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_full_duplex(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MCR_DM_UdfWr(0x1);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the device in list of
* multicast mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_multicast_list_mode(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MPFR_HMC_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the device in unicast mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_unicast_mode(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MPFR_HUC_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the device in all multicast mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_all_multicast_mode(struct DWC_ETH_QOS_prv_data *pdata)
{

	MAC_MPFR_PM_UdfWr(0x1);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to configure the device in promiscuous mode
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT set_promiscuous_mode(struct DWC_ETH_QOS_prv_data *pdata)
{

	//MAC_MPFR_PR_UdfWr(0x1);
        MAC_MPFR_RA_UdfWr(0x1);

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

static INT write_phy_regs(INT phy_id, INT phy_reg, INT phy_reg_data, struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG retryCount = 1000;
	ULONG vy_count;
	volatile ULONG varMAC_GMIIAR;

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
	varMAC_GMIIAR = varMAC_GMIIAR & (ULONG) (0x12);
	varMAC_GMIIAR =
	    varMAC_GMIIAR | ((phy_id) << 21) | ((phy_reg) << 16) | ((0x3) << 8)
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

static INT read_phy_regs(INT phy_id, INT phy_reg, INT *phy_reg_data, struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG retryCount = 1000;
	ULONG vy_count;
	volatile ULONG varMAC_GMIIAR;
	ULONG varMAC_GMIIDR;

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
	varMAC_GMIIAR = varMAC_GMIIAR & (ULONG) (0x12);
	varMAC_GMIIAR =
	    varMAC_GMIIAR | ((phy_id) << 21) | ((phy_reg) << 16) | ((0x3) << 8)
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
* \brief This sequence is used to check whether transmitted pkts have
* fifo under run loss error or not, returns 1 if fifo under run error
* else returns 0
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT tx_fifo_underrun(t_TX_NORMAL_DESC *txdesc)
{
	UINT varTDES3;

	/* check TDES3.UF bit */
	TX_NORMAL_DESC_TDES3_Ml_Rd(txdesc->TDES3, varTDES3);
	if ((varTDES3 & 0x4) == 0x4) {
		return 1;
	} else {
		return 0;
	}
}

/*!
* \brief This sequence is used to check whether transmitted pkts have
* carrier loss error or not, returns 1 if carrier loss error else returns 0
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT tx_carrier_lost_error(t_TX_NORMAL_DESC *txdesc)
{
	UINT varTDES3;

	/* check TDES3.LoC and TDES3.NC bits */
	TX_NORMAL_DESC_TDES3_Ml_Rd(txdesc->TDES3, varTDES3);
	if (((varTDES3 & 0x800) == 0x800) || ((varTDES3 & 0x400) == 0x400)) {
		return 1;
	} else {
		return 0;
	}
}

/*!
* \brief This sequence is used to check whether transmission is aborted
* or not returns 1 if transmission is aborted else returns 0
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT tx_aborted_error(t_TX_NORMAL_DESC *txdesc)
{
	UINT varTDES3;

	/* check for TDES3.LC and TDES3.EC */
	TX_NORMAL_DESC_TDES3_Ml_Rd(txdesc->TDES3, varTDES3);
	if (((varTDES3 & 0x200) == 0x200) || ((varTDES3 & 0x100) == 0x100)) {
		return 1;
	} else {
		return 0;
	}
}

/*!
* \brief This sequence is used to check whether the pkt transmitted is
* successfull or not, returns 1 if transmission is success else returns 0
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT tx_complete(t_TX_NORMAL_DESC *txdesc)
{
	UINT varOWN;

	TX_NORMAL_DESC_TDES3_OWN_Mlf_Rd(txdesc->TDES3, varOWN);
	if (varOWN == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*!
* \brief This sequence is used to check whethet rx csum is enabled/disabled
* returns 1 if rx csum is enabled else returns 0
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT get_rx_csum_status(struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG varMAC_MCR;

	MAC_MCR_RgRd(varMAC_MCR);
	if (GET_VALUE(varMAC_MCR, MAC_MCR_IPC_LPOS, MAC_MCR_IPC_HPOS) == 0x1) {
		return 1;
	} else {
		return 0;
	}
}

/*!
* \brief This sequence is used to disable the rx csum
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT disable_rx_csum(struct DWC_ETH_QOS_prv_data *pdata)
{

	/* enable rx checksum */
	MAC_MCR_IPC_UdfWr(0);

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to enable the rx csum
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT enable_rx_csum(struct DWC_ETH_QOS_prv_data *pdata)
{

	/* enable rx checksum */
	MAC_MCR_IPC_UdfWr(0x1);

	return Y_SUCCESS;
}


/*!
* \brief This sequence is used to reinitialize the TX descriptor fields,
* so that device can reuse the descriptors
* \param[in] idx
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT tx_descriptor_reset(UINT idx, struct DWC_ETH_QOS_prv_data *pdata,
				UINT chInx)
{
	struct s_TX_NORMAL_DESC *TX_NORMAL_DESC =
		GET_TX_DESC_PTR(chInx, idx);

	DBGPR("-->tx_descriptor_reset\n");

	/* update buffer 1 address pointer to zero */
	TX_NORMAL_DESC_TDES0_Ml_Wr(TX_NORMAL_DESC->TDES0, 0);
	/* update buffer 2 address pointer to zero */
	TX_NORMAL_DESC_TDES1_Ml_Wr(TX_NORMAL_DESC->TDES1, 0);
	/* set all other control bits (IC, TTSE, B2L & B1L) to zero */
	TX_NORMAL_DESC_TDES2_Ml_Wr(TX_NORMAL_DESC->TDES2, 0);
	/* set all other control bits (OWN, CTXT, FD, LD, CPC, CIC etc) to zero */
	TX_NORMAL_DESC_TDES3_Ml_Wr(TX_NORMAL_DESC->TDES3, 0);

	DBGPR("<--tx_descriptor_reset\n");

	return Y_SUCCESS;
}

/*!
* \brief This sequence is used to reinitialize the RX descriptor fields,
* so that device can reuse the descriptors
* \param[in] idx
* \param[in] pdata
*/

static void rx_descriptor_reset(UINT idx,
				struct DWC_ETH_QOS_prv_data *pdata,
				unsigned int inte,
				UINT chInx)
{
	struct DWC_ETH_QOS_rx_buffer *buffer = GET_RX_BUF_PTR(chInx, idx);
	struct s_RX_NORMAL_DESC *RX_NORMAL_DESC = GET_RX_DESC_PTR(chInx, idx);
	u64 dma_adrs;

	DBGPR("-->rx_descriptor_reset\n");

	memset(RX_NORMAL_DESC, 0, sizeof(struct s_RX_NORMAL_DESC));
	/* update buffer 1 address pointer */
	dma_adrs = buffer->dma;
	RX_NORMAL_DESC_RDES0_Ml_Wr(RX_NORMAL_DESC->RDES0, (dma_adrs&0xFFFFFFFF));
#ifdef NTN_DATA_BUF_IN_HOST_MEM
        /* set the mask for physical address access */
        RX_NORMAL_DESC_RDES1_Ml_Wr(RX_NORMAL_DESC->RDES1, NTN_HOST_PHY_ADRS_MASK | ( (dma_adrs >> 32) & 0xF ));
#else
        /* set to zero */
        RX_NORMAL_DESC_RDES1_Ml_Wr(RX_NORMAL_DESC->RDES1, 0);
#endif

	/* set buffer 2 address pointer to zero */
	RX_NORMAL_DESC_RDES2_Ml_Wr(RX_NORMAL_DESC->RDES2, 0);
	/* set control bits - OWN, INTE and BUF1V */
	RX_NORMAL_DESC_RDES3_Ml_Wr(RX_NORMAL_DESC->RDES3,
				   (0x81000000 | inte));

	DBGPR("<--rx_descriptor_reset\n");
}

/*!
* \brief This sequence is used to initialize the rx descriptors.
* \param[in] pdata
*/

static void rx_descriptor_init(struct DWC_ETH_QOS_prv_data *pdata, UINT chInx)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data =
	    GET_RX_WRAPPER_DESC(chInx);
	struct DWC_ETH_QOS_rx_buffer *buffer =
	    GET_RX_BUF_PTR(chInx, rx_desc_data->cur_rx);
	struct s_RX_NORMAL_DESC *RX_NORMAL_DESC =
	    GET_RX_DESC_PTR(chInx, rx_desc_data->cur_rx);
	INT i;
	INT start_index = rx_desc_data->cur_rx;
	INT last_index;
	u64 dma_adrs;
	u64 desc_dma_adrs;

	DBGPR("-->rx_descriptor_init\n");

	/* initialize all desc */

	for (i = 0; i < pdata->rx_dma_ch[chInx].desc_cnt; i++) {
		memset(RX_NORMAL_DESC, 0, sizeof(struct s_RX_NORMAL_DESC));
		/* update buffer 1 address pointer */
		dma_adrs = buffer->dma;
		RX_NORMAL_DESC_RDES0_Ml_Wr(RX_NORMAL_DESC->RDES0, (dma_adrs & 0xFFFFFFFF) );
#ifdef NTN_DATA_BUF_IN_HOST_MEM
                /* set the mask for physical address access */
                RX_NORMAL_DESC_RDES1_Ml_Wr(RX_NORMAL_DESC->RDES1, NTN_HOST_PHY_ADRS_MASK | ( (dma_adrs >> 32) & 0xF ));
#else
                /* set to zero */
                RX_NORMAL_DESC_RDES1_Ml_Wr(RX_NORMAL_DESC->RDES1, 0);
#endif

		/* set buffer 2 address pointer to zero */
		RX_NORMAL_DESC_RDES2_Ml_Wr(RX_NORMAL_DESC->RDES2, 0);
		/* set control bits - OWN, INTE and BUF1V */
		RX_NORMAL_DESC_RDES3_Ml_Wr(RX_NORMAL_DESC->RDES3,
					   (0xc1000000));

		/* Don't Set the IOC bit for IPA controlled Desc */
		if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0) {
			UINT varRDES3 = 0;
			RX_NORMAL_DESC_RDES3_Ml_Rd(RX_NORMAL_DESC->RDES3,
									   varRDES3);
			/* reset IOC for all buffers */
			RX_NORMAL_DESC_RDES3_Ml_Wr(RX_NORMAL_DESC->RDES3,
									   (varRDES3 & ~(1 << 30)));
		}
		else {
			buffer->inte = (1 << 30);

			/* reconfigure INTE bit if RX watchdog timer is enabled */
			if (rx_desc_data->use_riwt) {
				if ((i % rx_desc_data->rx_coal_frames) != 0) {
					UINT varRDES3 = 0;
					RX_NORMAL_DESC_RDES3_Ml_Rd(RX_NORMAL_DESC->RDES3,
						varRDES3);
					/* reset INTE */
					RX_NORMAL_DESC_RDES3_Ml_Wr(RX_NORMAL_DESC->RDES3,
							(varRDES3 & ~(1 << 30)));
					buffer->inte = 0;
				}
			}
		}

		INCR_RX_DESC_INDEX(rx_desc_data->cur_rx, 1, pdata->rx_dma_ch[chInx].desc_cnt);
		RX_NORMAL_DESC =
			GET_RX_DESC_PTR(chInx, rx_desc_data->cur_rx);
		buffer = GET_RX_BUF_PTR(chInx, rx_desc_data->cur_rx);
	}

	/* Reset the OWN bit of last descriptor */
	if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0) {
		RX_NORMAL_DESC = GET_RX_DESC_PTR(chInx, pdata->rx_dma_ch[chInx].desc_cnt - 1);
		RX_CONTEXT_DESC_RDES3_OWN_Mlf_Wr(RX_NORMAL_DESC->RDES3, 0);
	}

	/* update the total no of Rx descriptors count */
	DMA_RXCH_DESC_RING_LENGTH_RgWr(chInx, (pdata->rx_dma_ch[chInx].desc_cnt - 1));
	/* update the Rx Descriptor Tail Pointer */
	last_index = GET_RX_CURRENT_RCVD_LAST_DESC_INDEX(start_index, 0, pdata->rx_dma_ch[chInx].desc_cnt);
	DMA_RXCH_DESC_TAILPTR_RgWr(chInx, GET_RX_DESC_DMA_ADDR(chInx, last_index));
	/* update the starting address of desc chain/ring */
	desc_dma_adrs = GET_RX_DESC_DMA_ADDR(chInx, start_index);
#ifdef NTN_DESC_BUF_IN_HOST_MEM
	DMA_RXCH_DESC_LISTHADDR_RgWr(chInx, NTN_HOST_PHY_ADRS_MASK | ((desc_dma_adrs >> 32) & 0xF));
#endif
	DMA_RXCH_DESC_LISTLADDR_RgWr(chInx, (u32)desc_dma_adrs);

	DBGPR("<--rx_descriptor_init\n");
}

/*!
* \brief This sequence is used to initialize the tx descriptors.
* \param[in] pdata
*/

static void tx_descriptor_init(struct DWC_ETH_QOS_prv_data *pdata,
				UINT chInx)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data =
		GET_TX_WRAPPER_DESC(chInx);
	struct s_TX_NORMAL_DESC *TX_NORMAL_DESC =
		GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
	INT i;
	INT start_index = tx_desc_data->cur_tx;
	u64 desc_dma_adrs;

	DBGPR("-->tx_descriptor_init\n");

	/* initialze all descriptors. */

	for (i = 0; i < pdata->tx_dma_ch[chInx].desc_cnt; i++) {
		/* update buffer 1 address pointer to zero */
		TX_NORMAL_DESC_TDES0_Ml_Wr(TX_NORMAL_DESC->TDES0, 0);
		/* update buffer 2 address pointer to zero */
		TX_NORMAL_DESC_TDES1_Ml_Wr(TX_NORMAL_DESC->TDES1, 0);
		/* set all other control bits (IC, TTSE, B2L & B1L) to zero */
		TX_NORMAL_DESC_TDES2_Ml_Wr(TX_NORMAL_DESC->TDES2, 0);
		/* set all other control bits (OWN, CTXT, FD, LD, CPC, CIC etc) to zero */
		TX_NORMAL_DESC_TDES3_Ml_Wr(TX_NORMAL_DESC->TDES3, 0);

		INCR_TX_DESC_INDEX(tx_desc_data->cur_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
	}

	/* update the total no of Tx descriptors count */
	DMA_TXCH_DESC_RING_LENGTH_RgWr(chInx, (pdata->tx_dma_ch[chInx].desc_cnt - 1));
	/* update the starting address of desc chain/ring */
	desc_dma_adrs = GET_TX_DESC_DMA_ADDR(chInx, start_index);
#ifdef NTN_DESC_BUF_IN_HOST_MEM
	DMA_TXCH_DESC_LISTHADDR_RgWr(chInx, NTN_HOST_PHY_ADRS_MASK | ((desc_dma_adrs >> 32) & 0xF));
#endif
	DMA_TXCH_DESC_LISTLADDR_RgWr(chInx, (u32)desc_dma_adrs);
	/* Program the tail pointer with the same address as init descriptor */
	DMA_TXCH_DESC_TAILPTR_RgWr(chInx, GET_TX_DESC_DMA_ADDR(chInx, start_index));

	DBGPR("<--tx_descriptor_init\n");
}


/*!
* \brief This sequence is used to prepare tx descriptor for
* packet transmission and issue the poll demand command to TxDMA
*
* \param[in] pdata
*/

static void pre_transmit(struct DWC_ETH_QOS_prv_data *pdata,
				UINT chInx, UINT ethertype)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data =
	    GET_TX_WRAPPER_DESC(chInx);
	struct DWC_ETH_QOS_tx_buffer *buffer =
	    GET_TX_BUF_PTR(chInx, tx_desc_data->cur_tx);
	struct s_TX_NORMAL_DESC *TX_NORMAL_DESC =
	    GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
	struct s_TX_CONTEXT_DESC *TX_CONTEXT_DESC =
	    (struct s_TX_CONTEXT_DESC *)GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
	UINT varcsum_enable;
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	UINT varvlan_pkt;
	UINT varvt = 0;
#endif
	INT i;
	INT start_index = tx_desc_data->cur_tx;
	INT last_index, original_start_index = tx_desc_data->cur_tx;
	struct s_tx_pkt_features *tx_pkt_features = GET_TX_PKT_FEATURES_PTR;
#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
	INT update_tail = 0;
	UINT varQTDR;
#endif
	UINT vartso_enable = 0;
	UINT varmss = 0;
	UINT varpay_len = 0;
	UINT vartcp_hdr_len = 0;
	UINT varptp_enable = 0;
	INT total_len = 0;
	u64 dma_adrs;

	DBGPR("-->pre_transmit: chInx = %u\n", chInx);

#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
	if (chInx == 0)
		MTL_Q0TDR_TXQSTS_UdfRd(varQTDR);
	else
		MTL_QTDR_TXQSTS_UdfRd(chInx, varQTDR);

	/* No activity on MAC Tx-Fifo and fifo is empty */
	if (0 == varQTDR) {
		/* disable MAC Transmit */
		MAC_MCR_TE_UdfWr(0);
		update_tail = 1;
	}
#endif

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	TX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Rd(
		tx_pkt_features->pkt_attributes, varvlan_pkt);
	if (varvlan_pkt == 0x1) {
		/* put vlan tag in contex descriptor and set other control
		 * bits accordingly */
		TX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Rd(tx_pkt_features->vlan_tag,
						   varvt);
		TX_CONTEXT_DESC_TDES3_VT_Mlf_Wr(TX_CONTEXT_DESC->TDES3, varvt);
		TX_CONTEXT_DESC_TDES3_VLTV_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
		TX_CONTEXT_DESC_TDES3_CTXT_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
		TX_CONTEXT_DESC_TDES3_OWN_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
		DBGPR_VLAN("%s:VLAN: TXDESC3_VT=%x VLTV,CTXT,OWN=1\n",__func__,varvt);

		original_start_index = tx_desc_data->cur_tx;
		INCR_TX_DESC_INDEX(tx_desc_data->cur_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		start_index = tx_desc_data->cur_tx;
		TX_NORMAL_DESC =
			GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
		buffer = GET_TX_BUF_PTR(chInx, tx_desc_data->cur_tx);
	}
#endif	/* DWC_ETH_QOS_ENABLE_VLAN_TAG */
#ifdef DWC_ETH_QOS_ENABLE_DVLAN
	if (pdata->via_reg_or_desc == DWC_ETH_QOS_VIA_DESC) {
		/* put vlan tag in contex descriptor and set other control
		 * bits accordingly */

		if (pdata->in_out & DWC_ETH_QOS_DVLAN_OUTER) {
			TX_CONTEXT_DESC_TDES3_VT_Mlf_Wr(TX_CONTEXT_DESC->TDES3,
					pdata->outer_vlan_tag);
			TX_CONTEXT_DESC_TDES3_VLTV_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
			/* operation (insertion/replacement/deletion/none) will be
 			 * specified in normal descriptor TDES2
 			 * */
		}
		if (pdata->in_out & DWC_ETH_QOS_DVLAN_INNER) {
			TX_CONTEXT_DESC_TDES2_IVT_Mlf_Wr(TX_CONTEXT_DESC->TDES2,
								pdata->inner_vlan_tag);
			TX_CONTEXT_DESC_TDES3_IVLTV_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
			TX_CONTEXT_DESC_TDES3_IVTIR_Mlf_Wr(TX_CONTEXT_DESC->TDES3,
								pdata->op_type);
		}
		TX_CONTEXT_DESC_TDES3_CTXT_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
		TX_CONTEXT_DESC_TDES3_OWN_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);

		original_start_index = tx_desc_data->cur_tx;
		INCR_TX_DESC_INDEX(tx_desc_data->cur_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		start_index = tx_desc_data->cur_tx;
		TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
		buffer = GET_TX_BUF_PTR(chInx, tx_desc_data->cur_tx);
	}
#endif /* End of DWC_ETH_QOS_ENABLE_DVLAN */

	/* prepare CONTEXT descriptor for TSO */
	TX_PKT_FEATURES_PKT_ATTRIBUTES_TSO_ENABLE_Mlf_Rd(
		tx_pkt_features->pkt_attributes, vartso_enable);
	if (vartso_enable && (tx_pkt_features->mss != tx_desc_data->default_mss)) {
		/* get MSS and update */
		TX_PKT_FEATURES_MSS_MSS_Mlf_Rd(tx_pkt_features->mss, varmss);
		TX_CONTEXT_DESC_TDES2_MSS_Mlf_Wr(TX_CONTEXT_DESC->TDES2, varmss);
		/* set MSS valid, CTXT and OWN bits */
		TX_CONTEXT_DESC_TDES3_TCMSSV_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
		TX_CONTEXT_DESC_TDES3_CTXT_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);
		TX_CONTEXT_DESC_TDES3_OWN_Mlf_Wr(TX_CONTEXT_DESC->TDES3, 0x1);

		tx_desc_data->default_mss = tx_pkt_features->mss;

		original_start_index = tx_desc_data->cur_tx;
		INCR_TX_DESC_INDEX(tx_desc_data->cur_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		start_index = tx_desc_data->cur_tx;
		TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
		buffer = GET_TX_BUF_PTR(chInx, tx_desc_data->cur_tx);
	}

	/* update the first buffer pointer and length */
	dma_adrs = buffer->dma;
	TX_NORMAL_DESC_TDES0_Ml_Wr(TX_NORMAL_DESC->TDES0, (dma_adrs&0xFFFFFFFF));
	TX_NORMAL_DESC_TDES2_HL_B1L_Mlf_Wr(TX_NORMAL_DESC->TDES2, buffer->len);

#ifdef NTN_DATA_BUF_IN_HOST_MEM
        /* set the mask for physical address access */
	TX_NORMAL_DESC_TDES2_BUFHA_Mlf_Wr(TX_NORMAL_DESC->TDES2, NTN_HOST_PHY_ADRS_MASK | ( (dma_adrs >> 32) & 0xF ));
#endif

	if (vartso_enable) {
		/* update TCP payload length (only for the descriptor with FD set) */
		TX_PKT_FEATURES_PAY_LEN_Ml_Rd(tx_pkt_features->pay_len, varpay_len);
		/* TDES3[17:0] will be TCP payload length */
		TX_NORMAL_DESC->TDES3 |= varpay_len;
	} else {
		/* update total length of packet */
		GET_TX_TOT_LEN(GET_TX_BUF_PTR(chInx, 0), tx_desc_data->cur_tx,
				GET_CURRENT_XFER_DESC_CNT(chInx), total_len, pdata->tx_dma_ch[chInx].desc_cnt);
		TX_NORMAL_DESC_TDES3_FL_Mlf_Wr(TX_NORMAL_DESC->TDES3, total_len);
	}

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	/* Insert a VLAN tag with a tag value programmed in MAC Reg 24 or
	 * CONTEXT descriptor
	 * */
	if (tx_desc_data->vlan_tag_present && Y_FALSE == tx_desc_data->tx_vlan_tag_via_reg) {
		TX_NORMAL_DESC_TDES2_VTIR_Mlf_Wr(TX_NORMAL_DESC->TDES2,
						 tx_desc_data->tx_vlan_tag_ctrl);
		DBGPR_VLAN("%s:VLAN control info update via descriptor\n\n",__func__);
	}
#endif	/* DWC_ETH_QOS_ENABLE_VLAN_TAG */

#ifdef DWC_ETH_QOS_ENABLE_DVLAN
	if (pdata->via_reg_or_desc == DWC_ETH_QOS_VIA_DESC) {
		if (pdata->in_out & DWC_ETH_QOS_DVLAN_OUTER) {
			TX_NORMAL_DESC_TDES2_VTIR_Mlf_Wr(TX_NORMAL_DESC->TDES2,
								pdata->op_type);
		}
	}
#endif /* End of DWC_ETH_QOS_ENABLE_DVLAN */


	/* Mark it as First Descriptor */
	TX_NORMAL_DESC_TDES3_FD_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0x1);
	/* Enable CRC and Pad Insertion (NOTE: set this only
	 * for FIRST descriptor) */
	TX_NORMAL_DESC_TDES3_CPC_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0);
	/* Mark it as NORMAL descriptor */
	TX_NORMAL_DESC_TDES3_CTXT_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0);
	/* Enable HW CSUM */
	TX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_ENABLE_Mlf_Rd(tx_pkt_features->pkt_attributes,
		varcsum_enable);
	if (varcsum_enable == 0x1) {
		TX_NORMAL_DESC_TDES3_CIC_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0x3);
	}
	/* configure SA Insertion Control */
	TX_NORMAL_DESC_TDES3_SAIC_Mlf_Wr(TX_NORMAL_DESC->TDES3,
					 pdata->tx_sa_ctrl_via_desc);
	if (vartso_enable) {
		/* set TSE bit */
		TX_NORMAL_DESC_TDES3_TSE_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0x1);

		/* update tcp data offset or tcp hdr len */
		TX_PKT_FEATURES_TCP_HDR_LEN_Ml_Rd(tx_pkt_features->tcp_hdr_len, vartcp_hdr_len);
		/* convert to bit value */
		vartcp_hdr_len = vartcp_hdr_len/4;
		TX_NORMAL_DESC_TDES3_SLOTNUM_TCPHDRLEN_Mlf_Wr(TX_NORMAL_DESC->TDES3, vartcp_hdr_len);
	}

	/* enable timestamping */
	TX_PKT_FEATURES_PKT_ATTRIBUTES_PTP_ENABLE_Mlf_Rd(tx_pkt_features->pkt_attributes, varptp_enable);
	if (varptp_enable) {
		TX_NORMAL_DESC_TDES2_TTSE_Mlf_Wr(TX_NORMAL_DESC->TDES2, 0x1);
	}

	INCR_TX_DESC_INDEX(tx_desc_data->cur_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
	TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
	buffer = GET_TX_BUF_PTR(chInx, tx_desc_data->cur_tx);

	for (i = 1; i < GET_CURRENT_XFER_DESC_CNT(chInx); i++) {
		/* update the first buffer pointer and length */
		dma_adrs = buffer->dma;
		TX_NORMAL_DESC_TDES0_Ml_Wr(TX_NORMAL_DESC->TDES0, (dma_adrs&0xFFFFFFFF));
		TX_NORMAL_DESC_TDES2_HL_B1L_Mlf_Wr(TX_NORMAL_DESC->TDES2, buffer->len);
#ifdef NTN_DATA_BUF_IN_HOST_MEM
                /* set the mask for physical address access */
	        TX_NORMAL_DESC_TDES2_BUFHA_Mlf_Wr(TX_NORMAL_DESC->TDES2, NTN_HOST_PHY_ADRS_MASK | ( (dma_adrs >> 32) & 0xF ));
#endif

		/* set own bit */
		TX_NORMAL_DESC_TDES3_OWN_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0x1);
		/* Mark it as NORMAL descriptor */
		TX_NORMAL_DESC_TDES3_CTXT_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0);

		INCR_TX_DESC_INDEX(tx_desc_data->cur_tx, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, tx_desc_data->cur_tx);
		buffer = GET_TX_BUF_PTR(chInx, tx_desc_data->cur_tx);
	}
	/* Mark it as LAST descriptor */
	last_index =
		GET_TX_CURRENT_XFER_LAST_DESC_INDEX(chInx, start_index, 0, pdata->tx_dma_ch[chInx].desc_cnt);
	TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, last_index);
	TX_NORMAL_DESC_TDES3_LD_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0x1);
	/* set Interrupt on Completion for last descriptor */
#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
	pdata->mac_enable_count += 1;
	if ((pdata->mac_enable_count % pdata->drop_tx_pktburstcnt) == 0)
		TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(TX_NORMAL_DESC->TDES2, 0x1);
		//TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(TX_NORMAL_DESC->TDES2,(chInx==3?(!(tx_desc_data->cur_tx%4)):1));
#else
	/* Set the interrupt on complete bit. For normal IP traffic
	 * (channel 0) and AVTP traffic (channels 3 and 4), trigger an interrupt on
	 * every 8th transmit to improve performance. For GPTP traffic, trigger
	 * an interrupt on complete for every packet (in order to get the TX time). */
	if (chInx == NTN_TX_DMA_CH_0) {
		if (ethertype == NTN_GPTP_ETH_TYPE) {
			TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(TX_NORMAL_DESC->TDES2, INTR_MODERATE_CNT_GPTP_TRAFFIC);
		} else {
			TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(TX_NORMAL_DESC->TDES2, (!(tx_desc_data->cur_tx % INTR_MODERATE_CNT_REGULAR_IP_TRAFFIC)));
		}
	} else if (chInx == NTN_TX_DMA_CH_3 || chInx == NTN_TX_DMA_CH_4) {
		TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(TX_NORMAL_DESC->TDES2, (!(tx_desc_data->cur_tx % INTR_MODERATE_CNT_AVB_TRAFFIC)));
	} else {
		TX_NORMAL_DESC_TDES2_IC_Mlf_Wr(TX_NORMAL_DESC->TDES2, (!(tx_desc_data->cur_tx % INTR_MODERATE_CNT_REGULAR_IP_TRAFFIC)));
	}
#endif

	/* Set the launch time for the TX packet, before setting the OWN bit */
	if( (chInx == NTN_TX_PKT_AVB_CLASS_A) || (chInx == NTN_TX_PKT_AVB_CLASS_B) )
	{
#ifdef NTN_AVB_LAUNCHTIME_TS_CAPTURE_DUMP
		TX_NORMAL_DESC_TDES2_TTSE_Mlf_Wr(TX_NORMAL_DESC->TDES2, 0x1);
#endif
		//TX_NORMAL_DESC_TDES2_TMODE_Mlf_Wr(TX_NORMAL_DESC->TDES2, TX_NORMAL_DESC_TDES2_TMODE_EMBEDDED_IN_PKT);
		TX_NORMAL_DESC_TDES2_TMODE_Mlf_Wr(TX_NORMAL_DESC->TDES2, TX_NORMAL_DESC_TDES2_TMODE_TDES1_VAL1);
		//TX_NORMAL_DESC_TDES2_TMODE_Mlf_Wr(TX_NORMAL_DESC->TDES2, TX_NORMAL_DESC_TDES2_TMODE_IGNORE);
		TX_NORMAL_DESC_TDES1_LAUNCHTIME_Mlf_Wr(TX_NORMAL_DESC->TDES1, tx_pkt_features->launch_time);
#ifdef NTN_AVB_LAUNCHTIME_DUMP
		{
			static unsigned int pre_launch_time = 0;
			if( (TX_NORMAL_DESC->TDES1 - pre_launch_time) != 125000)
				NDBGPR_L1("Launch = %#lx,                IPG = %d \n", (ULONG)TX_NORMAL_DESC->TDES1, TX_NORMAL_DESC->TDES1 - pre_launch_time);
			pre_launch_time = TX_NORMAL_DESC->TDES1;
		}
#endif
	}

	/* set OWN bit of FIRST descriptor at end to avoid race condition */
	TX_NORMAL_DESC = GET_TX_DESC_PTR(chInx, start_index);
	TX_NORMAL_DESC_TDES3_OWN_Mlf_Wr(TX_NORMAL_DESC->TDES3, 0x1);

#ifdef DWC_ETH_QOS_ENABLE_TX_DESC_DUMP
	dump_tx_desc(pdata, original_start_index, (tx_desc_data->cur_tx - 1),
			1, chInx);
#endif

#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
	/* updating descriptor tail pointer for DMA Transmit under two conditions,
	 * 1. if burst number of packets are present in descriptor list
	 * 2. MAC has no activity on Tx fifo
	 * */
	if ((pdata->mac_enable_count >= pdata->drop_tx_pktburstcnt)
		&& (1 == update_tail)) {
		pdata->mac_enable_count -= pdata->drop_tx_pktburstcnt;
		/* issue a poll command to Tx DMA by writing address
		 * of next immediate free descriptor */
		last_index = GET_TX_CURRENT_XFER_LAST_DESC_INDEX(chInx, start_index, 1, pdata->tx_dma_ch[chInx].desc_cnt);
		DMA_TXCH_DESC_TAILPTR_RgWr(chInx, GET_TX_DESC_DMA_ADDR(chInx, last_index));
	}
#else
	/* issue a poll command to Tx DMA by writing address
	 * of next immediate free descriptor */
	last_index = GET_TX_CURRENT_XFER_LAST_DESC_INDEX(chInx, start_index, 1, pdata->tx_dma_ch[chInx].desc_cnt);
    DMA_TXCH_DESC_TAILPTR_RgWr(chInx, GET_TX_DESC_DMA_ADDR(chInx, last_index));

#endif

	if (pdata->eee_enabled) {
		/* restart EEE timer */
		mod_timer(&pdata->eee_ctrl_timer,
			DWC_ETH_QOS_LPI_TIMER(DWC_ETH_QOS_DEFAULT_LPI_TIMER));
	}

	DBGPR("<--pre_transmit\n");
}

/*!
* \brief This sequence is used to read data from device,
* it checks whether data is good or bad and updates the errors appropriately
* \param[in] pdata
*/

static void device_read(struct DWC_ETH_QOS_prv_data *pdata, UINT chInx)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data =
	    GET_RX_WRAPPER_DESC(chInx);
	struct s_RX_NORMAL_DESC *RX_NORMAL_DESC =
	    GET_RX_DESC_PTR(chInx, rx_desc_data->cur_rx);
	UINT varOWN;
	UINT varES;
	struct DWC_ETH_QOS_rx_buffer *buffer =
	    GET_RX_BUF_PTR(chInx, rx_desc_data->cur_rx);
	UINT varRS1V;
	UINT varIPPE;
	UINT varIPCB;
	UINT varIPHE;
	struct s_rx_pkt_features *rx_pkt_features = GET_RX_PKT_FEATURES_PTR;
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	UINT varRS0V;
	UINT varLT;
	UINT varRDES0;
#endif
	UINT varOE;
	struct s_rx_error_counters *rx_error_counters =
	    GET_RX_ERROR_COUNTERS_PTR;
	UINT varCE;
	UINT varRE;
	UINT varLD;

	DBGPR("-->device_read: cur_rx = %d\n", rx_desc_data->cur_rx);

	/* check for data availability */
	RX_NORMAL_DESC_RDES3_OWN_Mlf_Rd(RX_NORMAL_DESC->RDES3, varOWN);
	if (varOWN == 0) {
		/* check whether it is good packet or bad packet */
		RX_NORMAL_DESC_RDES3_ES_Mlf_Rd(RX_NORMAL_DESC->RDES3, varES);
		RX_NORMAL_DESC_RDES3_LD_Mlf_Rd(RX_NORMAL_DESC->RDES3, varLD);
#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT_HALFDUPLEX
		/* Synopsys testing and debugging purposes only */
		if (varES == 1 && varLD == 1) {
			varES = 0;
			DBGPR("Forwarding error packets as good packets to stack\n");
		}
#endif
		if ((varES == 0) && (varLD == 1)) {
			/* get the packet length */
			RX_NORMAL_DESC_RDES3_FL_Mlf_Rd(RX_NORMAL_DESC->RDES3, buffer->len);
			RX_NORMAL_DESC_RDES3_RS1V_Mlf_Rd(RX_NORMAL_DESC->RDES3, varRS1V);
			if (varRS1V == 0x1) {
				/* check whether device has done csum correctly or not */
				RX_NORMAL_DESC_RDES1_IPPE_Mlf_Rd(RX_NORMAL_DESC->RDES1, varIPPE);
				RX_NORMAL_DESC_RDES1_IPCB_Mlf_Rd(RX_NORMAL_DESC->RDES1, varIPCB);
				RX_NORMAL_DESC_RDES1_IPHE_Mlf_Rd(RX_NORMAL_DESC->RDES1, varIPHE);
				if ((varIPPE == 0) && (varIPCB == 0) && (varIPHE == 0)) {
					/* IPC Checksum done */
					RX_PKT_FEATURES_PKT_ATTRIBUTES_CSUM_DONE_Mlf_Wr(
						rx_pkt_features->pkt_attributes, 0x1);
				}
			}
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
			RX_NORMAL_DESC_RDES3_RS0V_Mlf_Rd(RX_NORMAL_DESC->RDES3,
							 varRS0V);
			if (varRS0V == 0x1) {
				/*  device received frame with VLAN Tag or double VLAN Tag ? */
				RX_NORMAL_DESC_RDES3_LT_Mlf_Rd(RX_NORMAL_DESC->RDES3, varLT);
				if ((varLT == 0x4) || (varLT == 0x5)) {
					RX_PKT_FEATURES_PKT_ATTRIBUTES_VLAN_PKT_Mlf_Wr(
						rx_pkt_features->pkt_attributes, 0x1);
					/* get the VLAN Tag */
					RX_NORMAL_DESC_RDES0_Ml_Rd(RX_NORMAL_DESC->RDES0, varRDES0);
					RX_PKT_FEATURES_VLAN_TAG_VT_Mlf_Wr(rx_pkt_features->vlan_tag,
						(varRDES0 & 0xffff));
				}
			}
#endif
		} else {
#ifdef DWC_ETH_QOS_ENABLE_RX_DESC_DUMP
			dump_rx_desc(chInx, RX_NORMAL_DESC, rx_desc_data->cur_rx);
#endif
			/* not a good packet, hence check for appropriate errors. */
			RX_NORMAL_DESC_RDES3_OE_Mlf_Rd(RX_NORMAL_DESC->RDES3, varOE);
			if (varOE == 1) {
				RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_Mlf_Wr(rx_error_counters->rx_errors, 1);
			}
			RX_NORMAL_DESC_RDES3_CE_Mlf_Rd(RX_NORMAL_DESC->RDES3, varCE);
			if (varCE == 1) {
				RX_ERROR_COUNTERS_RX_ERRORS_CRC_ERROR_Mlf_Wr(rx_error_counters->rx_errors, 1);
			}
			RX_NORMAL_DESC_RDES3_RE_Mlf_Rd(RX_NORMAL_DESC->RDES3, varRE);
			if (varRE == 1) {
				RX_ERROR_COUNTERS_RX_ERRORS_FRAME_ERROR_Mlf_Wr(rx_error_counters->rx_errors, 1);
			}
			RX_NORMAL_DESC_RDES3_LD_Mlf_Rd(RX_NORMAL_DESC->RDES3, varLD);
			if (varRE == 0) {
				RX_ERROR_COUNTERS_RX_ERRORS_OVERRUN_ERROR_Mlf_Wr(rx_error_counters->rx_errors, 1);
			}
		}
	}

	DBGPR("<--device_read: cur_rx = %d\n", rx_desc_data->cur_rx);
}

static void update_rx_tail_ptr(unsigned int chInx, unsigned int dma_addr, struct DWC_ETH_QOS_prv_data *pdata)
{
	NDBGPR_TS2("Updating RX Tail PTR\n");
	DMA_RXCH_DESC_TAILPTR_RgWr(chInx, dma_addr);
}

/*!
* \brief This sequence is used to check whether CTXT bit is
* set or not returns 1 if CTXT is set else returns zero
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT get_tx_descriptor_ctxt(t_TX_NORMAL_DESC *txdesc)
{
	ULONG varCTXT;

	/* check TDES3.CTXT bit */
	TX_NORMAL_DESC_TDES3_CTXT_Mlf_Rd(txdesc->TDES3, varCTXT);
	if (varCTXT == 1) {
		return 1;
	} else {
		return 0;
	}
}

/*!
* \brief This sequence is used to check whether LD bit is set or not
* returns 1 if LD is set else returns zero
* \param[in] txdesc
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT get_tx_descriptor_last(t_TX_NORMAL_DESC *txdesc)
{
	ULONG varLD;

	/* check TDES3.LD bit */
	TX_NORMAL_DESC_TDES3_LD_Mlf_Rd(txdesc->TDES3, varLD);
	if (varLD == 1) {
		return 1;
	} else {
		return 0;
	}
}


/*!
* \brief Exit routine
* \details Exit function that unregisters the device, deallocates buffers,
* unbinds the driver from controlling the device etc.
*
* \return Returns successful execution of the routine
* \retval Y_SUCCESS Function executed successfully
*/
static INT ntn_mac_reset_config(UINT assert_deassert, struct DWC_ETH_QOS_prv_data *pdata);
static INT DWC_ETH_QOS_yexit(struct DWC_ETH_QOS_prv_data *pdata)
{

#if 0 //Commented Synopsys Implementation
	ULONG retryCount = 1000;
	ULONG vy_count;
	volatile ULONG varDMA_BMR;

	DBGPR("-->DWC_ETH_QOS_yexit\n");

	/*issue a software reset */
	DMA_BMR_SWR_UdfWr(0x1);
	/*DELAY IMPLEMENTATION USING udelay() */
	udelay(10);

	/*Poll Until Poll Condition */
	vy_count = 0;
	while (1) {
		if (vy_count > retryCount) {
			return -Y_FAILURE;
		} else {
			vy_count++;
			mdelay(1);
		}
		DMA_BMR_RgRd(varDMA_BMR);
		if (GET_VALUE(varDMA_BMR, DMA_BMR_SWR_LPOS, DMA_BMR_SWR_HPOS) == 0) {
			break;
		}
	}

	DBGPR("<--DWC_ETH_QOS_yexit\n");

	return Y_SUCCESS;
#else
    INT ret = 0;

	DBGPR("-->DWC_ETH_QOS_yexit\n");

	/* assert software reset */
    ret |= ntn_mac_reset_config(0x1, pdata);
    /* DELAY IMPLEMENTATION USING udelay() */
	udelay(10);

	/* deassert software reset */
    ret |= ntn_mac_reset_config(0x0, pdata);
    /* DELAY IMPLEMENTATION USING udelay() */
	udelay(10);

	DBGPR("<--DWC_ETH_QOS_yexit\n");

	return ret;
#endif
}

static INT configure_rx_mtl_queue(UINT qInx, struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT p_rx_fifo = eDWC_ETH_QOS_256;

	DBGPR("-->configure_rx_mtl_queue\n");

	MTL_QROMR_RSF_UdfWr(qInx, 0x1);
	MTL_QROMR_FUP_UdfWr(qInx, 0x1);
	MTL_QROMR_FEP_UdfWr(qInx, 0x1);

	/* RX Queue sizes are hardcoded and assigned as per the Neutrino's configuration. */
	switch(qInx)
	{
		case 0:	p_rx_fifo = 29;	break;
		case 1:	p_rx_fifo = 1;	break;
		case 2:	p_rx_fifo = 1;	break;
		case 3:	p_rx_fifo = 29;	break;
	}

	/* Receive queue fifo size programmed */
	MTL_QROMR_RQS_UdfWr(qInx, p_rx_fifo);
	NDBGPR_L1( "Queue%d Rx fifo size %d\n",	qInx, ((p_rx_fifo + 1) * 256));

	DBGPR("<--configure_rx_mtl_queue\n");

	return Y_SUCCESS;
}

static INT configure_tx_mtl_queue(UINT qInx, struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_tx_dma_ch *queue_data = GET_TX_DMA_CH_PTR(qInx);
	ULONG retryCount = 1000;
	ULONG vy_count;
	volatile ULONG varMTL_QTOMR;
	UINT p_tx_fifo = eDWC_ETH_QOS_256;

	DBGPR("-->configure_tx_mtl_queue\n");

	/*Flush Tx Queue */
	MTL_QTOMR_FTQ_UdfWr(qInx, 0x1);

	/*Poll Until Poll Condition */
	vy_count = 0;
	while (1) {
		if (vy_count > retryCount) {
			NMSGPR_ALERT("MTL Queue %d init failure\n", qInx);
			return -Y_FAILURE;
		} else {
			vy_count++;
			mdelay(1);
		}
		MTL_QTOMR_RgRd(qInx, varMTL_QTOMR);
		if (GET_VALUE(varMTL_QTOMR, MTL_QTOMR_FTQ_LPOS, MTL_QTOMR_FTQ_HPOS)
				== 0) {
			break;
		}
	}

	/*Enable Store and Forward mode for TX */
	MTL_QTOMR_TSF_UdfWr(qInx, 0x1);
	/* Program Tx operating mode */
	MTL_QTOMR_TXQEN_UdfWr(qInx, queue_data->q_op_mode);
	/* Transmit Queue weight */
	MTL_QW_ISCQW_UdfWr(qInx, (0x10 + qInx));

	/* TX Queue sizes are hardcoded and assigned as per the Neutrino's configuration. */
	switch(qInx)
	{
		case 0:	p_tx_fifo = 9;		break;
		case 1:	p_tx_fifo = 0xa;	break;
		case 2:	p_tx_fifo = 0xa;	break;
	}

	/* Transmit queue fifo size programmed */
	MTL_QTOMR_TQS_UdfWr(qInx, p_tx_fifo);
	NDBGPR_L1( "Queue%d Tx fifo size %d\n",	qInx, ((p_tx_fifo + 1) * 256));
	DBGPR("<--configure_tx_mtl_queue\n");

	return Y_SUCCESS;
}

static INT configure_dma_tx_channel(UINT chInx,
			struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->configure_dma_tx_channel\n");

	DMA_TXCHCTL_OSP_UdfWr(chInx, 0x1);

	enable_dma_tx_interrupts(chInx, pdata);

	DMA_TXCHCTL_TXPBL_UdfWr(chInx, 48);

    /* To get Best Performance */
    DMA_BUSCFG_BLEN16_UdfWr(1);
    DMA_BUSCFG_BLEN8_UdfWr(1);
    DMA_BUSCFG_BLEN4_UdfWr(1);

    DMA_BUSCFG_RD_OSR_LMT_UdfWr(15);
    DMA_BUSCFG_WR_OSR_LMT_UdfWr(15);

    DMA_TXCHCTL_ST_UdfWr(chInx, 0x1);

	DBGPR("<--configure_dma_tx_channel\n");

	return Y_SUCCESS;
}

static INT configure_dma_rx_channel(UINT chInx,
			struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data =
		GET_RX_WRAPPER_DESC(chInx);
    UINT reg_val;

	DBGPR("-->configure_dma_rx_channel\n");

	/*Select Rx Buffer size = 2048bytes */
	switch (pdata->rx_buffer_len) {
	case 16384:
		reg_val = 16384;
		break;
	case 8192:
		reg_val = 8192;
		break;
	case 4096:
		reg_val = 4096;
		break;
	default:		/* default is 2K */
		reg_val = 2048;
		break;
	}

    DMA_RXCHCTL_RBSZ_UdfWr(chInx, reg_val);

	/* program RX watchdog timer */
	if (rx_desc_data->use_riwt) {
			/* Program RX Watchdog time to maximum for AVB RX Channel: that is = 0xFF*256*5.3ns (187MHz)*/
			if(NTN_RX_PKT_AVB == chInx)
        		DMA_RXCH_CUR_WATCHDOG_TIMER_RWT_UdfWr(chInx, 0xFF);
        	else
				DMA_RXCH_CUR_WATCHDOG_TIMER_RWT_UdfWr(chInx, rx_desc_data->rx_riwt);
	}
	else {
        	DMA_RXCH_CUR_WATCHDOG_TIMER_RWT_UdfWr(chInx, 0);
	}

	NDBGPR_L1( "%s Rx watchdog timer\n",
		(rx_desc_data->use_riwt ? "Enabled" : "Disabled"));

	enable_dma_rx_interrupts(chInx, pdata);

	/* set RX PBL = 128 bytes */
  	DMA_RXCHCTL_RXPBL_UdfWr(chInx, 16);

    DMA_RXCHCTL_ST_UdfWr(chInx, 0x1);

	DBGPR("<--configure_dma_rx_channel\n");

	return Y_SUCCESS;
}



#ifndef NTN_POLLING_METHOD

/*!
* \brief This sequence is used to enable MAC interrupts
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int enable_mac_interrupts(struct DWC_ETH_QOS_prv_data *pdata)
{
  unsigned long varmac_imr;
  unsigned long reg_val;

  /* Enable following interrupts */
  /* RGSMIIIM - RGMII/SMII interrupt Enable */
  /* PCSLCHGIM -  PCS Link Status Interrupt Enable */
  /* PCSANCIM - PCS AN Completion Interrupt Enable */
  /* PMTIM - PMT Interrupt Enable */
  /* LPIIM - LPI Interrupt Enable */
  MAC_IMR_RgRd(varmac_imr);
  varmac_imr = varmac_imr & (unsigned long)(0x1008);
  varmac_imr = varmac_imr | ((0x1) << 0) | ((0x1) << 1) | ((0x1) << 2) |
                ((0x1) << 4) | ((0x1) << 5);
  MAC_IMR_RgWr(varmac_imr);

  NTN_INTC_INTMCUMASK1_RgRd(reg_val);
  reg_val = reg_val & 0xC0000000;	//Preserve int mask for qSPI and GDMA
  reg_val = reg_val | NTN_INTC_GMAC_INT_MASK;
  NTN_INTC_INTMCUMASK1_RgWr(reg_val);

  /* Mask PCIe controller interrupt */
  reg_val = (0xF << 9);
  NTN_INTC_INTMCUMASK2_RgWr(reg_val);

  return Y_SUCCESS;
}
#endif


static INT configure_mac(struct DWC_ETH_QOS_prv_data *pdata)
{
	ULONG varMAC_MCR;
	UINT qInx;
	UINT regval;

	DBGPR("-->configure_mac\n");

	for (qInx = 0; qInx < NTN_RX_QUEUE_CNT; qInx++) {
		if(!pdata->rx_q_for_host[qInx])
			continue;
		MAC_RQC0R_RXQEN_UdfWr(qInx, 0x2);
	}

	/* Set Tx flow control parameters */
	for (qInx = 0; qInx < NTN_TX_QUEUE_CNT; qInx++) {
		if(!pdata->tx_q_for_host[qInx])
			continue;
		/* set Pause Time */
		MAC_QTFCR_PT_UdfWr(qInx, 0xffff);
		/* Assign priority for TX flow control */
		MAC_TQPM0R_PSTQ0_UdfWr(qInx);

		if ((pdata->flow_ctrl & DWC_ETH_QOS_FLOW_CTRL_TX) == DWC_ETH_QOS_FLOW_CTRL_TX) {
			enable_tx_flow_ctrl(qInx, pdata);
			NDBGPR_L1("TX flow control for Queue(%d): ENABLED \n",qInx);
		} else {
			disable_tx_flow_ctrl(qInx, pdata);
			NDBGPR_L1("TX flow control for Queue(%d): DISABLED \n",qInx);
		}
	}

	/* Set Rx flow control parameters */
	for (qInx = 0; qInx < NTN_RX_QUEUE_CNT; qInx++) {
		if(!pdata->rx_q_for_host[qInx])
			continue;
		/* Assign priority for RX flow control */
		switch(qInx) {
		case 0:
			MAC_RQC2R_PSRQ0_UdfWr(0x1 << qInx);
			break;
		case 1:
			MAC_RQC2R_PSRQ1_UdfWr(0x1 << qInx);
			break;
		case 2:
			MAC_RQC2R_PSRQ2_UdfWr(0x1 << qInx);
			break;
		case 3:
			MAC_RQC2R_PSRQ3_UdfWr(0x1 << qInx);
			break;
		}
	}

	MAC_RQC0R_RgRd(regval);
	NDBGPR_L1("RX_CTRL0 = %#x\n", regval);
	MAC_RQC1R_RgRd(regval);
	NDBGPR_L1("RX_CTRL1 = %#x\n", regval);
	MAC_RQC2R_RgRd(regval);
	NDBGPR_L1("RX_CTRL2 = %#x\n", regval);
	MAC_RQC3R_RgRd(regval);
	NDBGPR_L1("RX_CTRL3 = %#x\n", regval);

	MAC_RQC0R_RgWr(0x56);
	//MAC_RQC0R_RgWr(0x96);	//VLAN DEBUG
	MAC_RQC1R_RgWr(0x12);
	MAC_RQC2R_RgWr(0xC000000);

	MAC_RQC0R_RgRd(regval);
	NDBGPR_L1("RX_CTRL0 = %#x\n", regval);
	MAC_RQC1R_RgRd(regval);
	NDBGPR_L1("RX_CTRL1 = %#x\n", regval);
	MAC_RQC2R_RgRd(regval);
	NDBGPR_L1("RX_CTRL2 = %#x\n", regval);
	MAC_RQC3R_RgRd(regval);
	NDBGPR_L1("RX_CTRL3 = %#x\n", regval);


	/* Set Rx flow control parameters */
	if ((pdata->flow_ctrl & DWC_ETH_QOS_FLOW_CTRL_RX) == DWC_ETH_QOS_FLOW_CTRL_RX) {
		enable_rx_flow_ctrl(pdata);
		NDBGPR_L1("RX flow control ENABLED \n");
	} else {
		disable_rx_flow_ctrl(pdata);
		NDBGPR_L1("RX flow control DISABLED \n");
	}

	MAC_MCR_JE_UdfWr(0x0);
	MAC_MCR_WD_UdfWr(0x0);
	MAC_MCR_GPSLCE_UdfWr(0x0);
	MAC_MCR_JD_UdfWr(0x0);

	MAC_MA2HR_RgWr((((pdata->dev->dev_addr[5]) << 8) |
			(pdata->dev->dev_addr[4])));
	MAC_MA2HR_AE_UdfWr(0x1);
	MAC_MA2LR_RgWr(((pdata->dev->dev_addr[3] << 24) |
			(pdata->dev->dev_addr[2] << 16) |
			(pdata->dev->dev_addr[1] << 8) |
			(pdata->dev->dev_addr[0])));
	{
		UINT regval;
		/* Config DMA RX Channel 2 to use MAC ID offset 2 */
		/* Configure DMA RX Channel 1 to use MAC ID offset 65 as host doesn't control channel and
                 * as 65 is not a valid channel it want route any multicast packet to channel. This will
                 * avoid RX DMA stall issue that was observed in past. */
		//regval = ioread32((void*)(dwc_eth_ntn_reg_pci_base_addr + 0x3004));
		/* Configure the register value to 0xFFFF0000, so  all VLAN
		   unbridged IP packets are still sent to IPA */
		regval = 0xFFFF0000;
		//regval = (regval & 0x00FF0000) | 0x02000000;
		iowrite32(regval, (void*)(pdata->dev->base_addr + 0x3004));
	}

	/*Enable MAC Transmit process */
	/*Enable MAC Receive process */
	/*Enable padding - disabled */
	/*Enable CRC stripping - disabled */
	MAC_MCR_RgRd(varMAC_MCR);
	varMAC_MCR = varMAC_MCR & (ULONG) (0xffcfff7c);
	varMAC_MCR = varMAC_MCR | ((0x1) << 0) | ((0x1) << 20) | ((0x1) << 21);
#ifndef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
	varMAC_MCR |= ((0x1) << 1);
#endif
	MAC_MCR_RgWr(varMAC_MCR);

	if (pdata->hw_feat.rx_coe_sel &&
	     ((pdata->dev_state & NETIF_F_RXCSUM) == NETIF_F_RXCSUM))
		MAC_MCR_IPC_UdfWr(0x1);

#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	configure_mac_for_vlan_pkt(pdata);
	if (pdata->hw_feat.vlan_hash_en)
			config_vlan_filtering(1, 1, 0, pdata);
#endif

	if (pdata->hw_feat.mmc_sel) {
		/* disable all MMC intterrupt as MMC are managed in SW and
		 * registers are cleared on each READ eventually
		 * */
		disable_mmc_interrupts(pdata);
		config_mmc_counters(pdata);
	}

#ifndef NTN_POLLING_METHOD
	enable_mac_interrupts(pdata);
#endif

	//MAC_VLANTR_RgWr(0x1010025);
	MAC_VLANTR_ETV_UdfWr(0x1);
	MAC_VLANTR_EVLRXS_UdfWr(0x1);
	MAC_VLANTIRR_RgWr(0x1);

	DBGPR("<--configure_mac\n");

	return Y_SUCCESS;
}


/******************************************************************************/
/**                         Neutrino wrapper registers                       **/
/******************************************************************************/
/*!
* \brief This function is used to enable or disable the Neutrino GMAC TX & RX
*        clock
* \param[in] ena_dis, 1=enable, 0=disable
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT ntn_mac_clock_config(UINT ena_dis, struct DWC_ETH_QOS_prv_data *pdata)
{
    UINT rd_val;

    NTN_NCLKCTRL_MACTXCEN_UdfWr(ena_dis);
    NTN_NCLKCTRL_MACTXCEN_UdfRd(rd_val);

    if(rd_val != ena_dis)
    {
        NMSGPR_ALERT( "ERROR: NTN MACTXCEN Bit config error wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return -Y_FAILURE;
    }else{
        DBGPR("NTN MACTXCEN Bit config : wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
    }

    NTN_NCLKCTRL_MACRXCEN_UdfWr(ena_dis);
    NTN_NCLKCTRL_MACRXCEN_UdfRd(rd_val);

    if(rd_val != ena_dis)
    {
        NMSGPR_ALERT( "ERROR: NTN MACRXCEN Bit config error wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return -Y_FAILURE;
    }else{
        DBGPR("NTN MACRXCEN Bit config : wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return Y_SUCCESS;
    }
}



/*!
* \brief This function is used to enable or disable the Neutrino Wrapper Timestamp Packet Blocking feature
* \param[in] ena_dis, 1=enable, 0=disable
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT ntn_wrap_ts_ignore_config(UINT ena_dis, struct DWC_ETH_QOS_prv_data *pdata)
{
    UINT rd_val;

    ETH_AVB_WRAPPER_TS_CTRL_UdfWr(ena_dis);
    ETH_AVB_WRAPPER_TS_CTRL_UdfRd(rd_val);

    if(rd_val != ena_dis)
    {
        NMSGPR_ALERT( "ERROR: NTN Wrapper Timestamp Enable Feature wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return -Y_FAILURE;
    }else{
        DBGPR("NTN Wrapper Timestamp Enable Feature : wr_val:0x%x, rd_val:0x%x \n", ena_dis, rd_val);
        return Y_SUCCESS;
    }
}

/*!
* \brief This function is used to set the Neutrino Wrapper Timestamp Valid Window
* \param[in] ts_window, the timestamp valid window in units of 2^24 ns.
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT ntn_wrap_ts_valid_window_config(UINT ts_window, struct DWC_ETH_QOS_prv_data *pdata)
{
    UINT rd_val;

    ETH_AVB_WRAPPER_TS_CTRL_TSTMP_WINDOW_Wr(ts_window);
    ETH_AVB_WRAPPER_TS_CTRL_TSTMP_WINDOW_Rd(rd_val);

    if(rd_val != ts_window)
    {
        NMSGPR_ALERT( "ERROR: NTN Wrapper Timestamp Valid Window wr_val:0x%x, rd_val:0x%x \n", ts_window, rd_val);
        return -Y_FAILURE;
    }else{
        DBGPR("NTN Wrapper Timestamp Valid Window : wr_val:0x%x, rd_val:0x%x \n", ts_window, rd_val);
        return Y_SUCCESS;
    }
}

/*!
 * \brief This function is used to enable and configure PCIe TAMAP to access host memory from Neutrino
 *		It can be improved to extract no of bits for replacement from the size.
 *
 * \param[in] UINT
 *       [in] ULONG_LONG
 *       [in] ULONG_LONG
 *       [in] UINT
 * \return
 * \retval  0 Success
 */
void ntn_config_tamap(UINT tmap_no, ULONG_LONG adrs, ULONG_LONG replacement_adrs, UINT no_of_bits, struct DWC_ETH_QOS_prv_data *pdata)
{
	unsigned int varOFFSET_ADR_UP = adrs>>32;
	unsigned int varOFFSET_ADR_DW = adrs&0xFFFFF000;
	unsigned int varOFFSET_EN = 1;
	unsigned int varOFFSET_REPLACE_UP = replacement_adrs>>32;
	unsigned int varOFFSET_REPLACE_DW = replacement_adrs&0xFFFFF000;
	unsigned int varOFFSET_OW = no_of_bits;
	unsigned int reg_val;

	NDBGPR_L1("-->%s\n", __FUNCTION__);

	if(varOFFSET_OW >= 53) //53 is the max limit defined in spec
		NMSGPR_ALERT("No of mask bit error, HW cannot support %d mask bits\n", varOFFSET_OW);

	NTN_PCIE_RANGE_UP_OFFSET_RgWr(tmap_no, varOFFSET_ADR_UP);
	NTN_PCIE_RANGE_EN_RgWr(tmap_no, varOFFSET_ADR_DW | varOFFSET_EN);
	NTN_PCIE_RANGE_UP_RPLC_RgWr(tmap_no, varOFFSET_REPLACE_UP);
	NTN_PCIE_RANGE_WIDTH_RgWr(tmap_no,  varOFFSET_REPLACE_DW | varOFFSET_OW);

	NTN_PCIE_RANGE_UP_OFFSET_RgRd(tmap_no, reg_val);
	NDBGPR_L1("RANGE_UP_OFFSET[%d] = 0x%08x\n", tmap_no, reg_val);
	NTN_PCIE_RANGE_EN_RgRd(tmap_no, reg_val);
	NDBGPR_L1("RANGE_EN[%d] = 0x%08x\n", tmap_no, reg_val);
	NTN_PCIE_RANGE_UP_RPLC_RgRd(tmap_no, reg_val);
	NDBGPR_L1("RANGE_UP_RPLC[%d] = 0x%08x\n", tmap_no, reg_val);
	NTN_PCIE_RANGE_WIDTH_RgRd(tmap_no, reg_val);
	NDBGPR_L1("RANGE_WIDTH[%d] = 0x%08x\n", tmap_no, reg_val);

 	NDBGPR_L1("<--%s\n", __FUNCTION__);
}

/*!
* \brief This sequence is used to assert the Neutrino reset
* \param[in] assert_deassert, 1=assert, 0=deassert
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static INT ntn_mac_reset_config(UINT assert_deassert, struct DWC_ETH_QOS_prv_data *pdata)
{
    UINT rd_val;

	NTN_NRSTCTRL_MACRST_UdfWr(assert_deassert);
	NTN_NRSTCTRL_MACRST_UdfRd(rd_val);
    if(rd_val != assert_deassert)
    {
        NMSGPR_ALERT( "ERROR: NTN MAC_RST Bit config error wr_val:0x%x, rd_val:0x%x \n", assert_deassert, rd_val);
        return -Y_FAILURE;
    }else{
        DBGPR("NTN MAC_RST Bit config : wr_val:0x%x, rd_val:0x%x \n", assert_deassert, rd_val);
        return Y_SUCCESS;
    }
}

/*!
 * \brief API to read register
 * \param[in] address offset as per Neutrino data-sheet
 * \param[in] bar number
 * \return register value
 */
static UINT ntn_reg_rd(UINT reg_offset, INT bar_num, struct DWC_ETH_QOS_prv_data *pdata)
{
    if(2 == bar_num)
	    return ioread32( (void*)(pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt + reg_offset));
    else if(4 == bar_num)
	    return ioread32( (void*)(pdata->dwc_eth_ntn_FLASH_pci_base_addr + reg_offset));
    else //(0 == bar_num)
        return ioread32( (void*)(pdata->dev->base_addr + reg_offset));
}

/*!
* \brief API to read FW version and features
* \param[in]
* \return register value
*/
static UINT read_fw_ver_features(struct DWC_ETH_QOS_prv_data *pdata)
{
       return ntn_reg_rd(
                       (NTN_M3_DBG_CNT_START + NTN_M3_DBG_RSVD_OFST),
                       PCIE_SRAM_BAR_NUM, pdata);
}

/*!
 * \brief API to write register
 * \param[in] address offset as per Neutrino data-sheet
 * \return register
 */
static void ntn_reg_wr(UINT reg_offset, UINT reg_val, INT bar_num, struct DWC_ETH_QOS_prv_data *pdata)
{
    if(0 == bar_num)
        iowrite32(reg_val, (void*)(pdata->dev->base_addr + reg_offset));
    if(2 == bar_num)
            iowrite32(reg_val, (void*)(pdata->dwc_eth_ntn_SRAM_pci_base_addr_virt + reg_offset));
    if(4 == bar_num)
            iowrite32(reg_val, (void*)(pdata->dwc_eth_ntn_FLASH_pci_base_addr + reg_offset));
    return;
}

/*!
 * \brief Following Three API are used to set TX clk for EMAC
 * \param[in]
 * \return register
 */
static INT ntn_set_tx_clk_125MHz(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT reg_val;

    NTN_NEMACCTL_RgRd(reg_val);
	reg_val &= NTN_NEMACCTL_TX_CLK_MASK;
	reg_val |= NTN_NEMACCTL_TX_CLK_125MHz;
	NDBGPR_L2("NEMACCTL Reg Value : %x\n", reg_val);
    NTN_NEMACCTL_RgWr(reg_val);

	return Y_SUCCESS;
}

static INT ntn_set_tx_clk_25MHz(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT reg_val;

    NTN_NEMACCTL_RgRd(reg_val);
	reg_val &= NTN_NEMACCTL_TX_CLK_MASK;
  if (pdata->rmii_mode)
    reg_val |= NTN_NEMACCTL_TX_CLK_25MHz_RMII;
  else
    reg_val |= NTN_NEMACCTL_TX_CLK_25MHz;

	NDBGPR_L2("NEMACCTL Reg Value : %x\n", reg_val);
    NTN_NEMACCTL_RgWr(reg_val);

	return Y_SUCCESS;
}

static INT ntn_set_tx_clk_2_5MHz(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT reg_val;

    NTN_NEMACCTL_RgRd(reg_val);
	reg_val &= NTN_NEMACCTL_TX_CLK_MASK;
	reg_val |= NTN_NEMACCTL_TX_CLK_2_5MHz;
	NDBGPR_L2("NEMACCTL Reg Value : %x\n", reg_val);
    NTN_NEMACCTL_RgWr(reg_val);

	return Y_SUCCESS;
}

/*!
 * \brief Sets the interface for the PHY
 * \param[in]
 * \return register
 */
static INT ntn_set_phy_intf(struct DWC_ETH_QOS_prv_data *pdata)
{
  UINT reg_val;

  NTN_NEMACCTL_RgRd(reg_val);
  reg_val &= NTN_NEMACCTL_PHY_INTF_MASK;
  if (pdata->rmii_mode)
    reg_val |= NTN_NEMACCTL_PHY_INTF_RMII;
  else
    reg_val |= NTN_NEMACCTL_PHY_INTF_RGMII;
  NDBGPR_L2("NEMACCTL Reg Value : %x\n", reg_val);
  NTN_NEMACCTL_RgWr(reg_val);

  return Y_SUCCESS;
}

static UINT ntn_boot_host_initiated(struct DWC_ETH_QOS_prv_data *pdata)
{
  UINT rd_val;
  NTN_NMODESTS_RgRd(rd_val);
  /* Per datasheet bit0=1 indicates secure boot and overrides boot option
   * from MODE02 bit. */
  return !(rd_val & NTN_NMODESTS_SECURE_BOOT_MASK) &&
          (rd_val & NTN_NMODESTS_HOST_BOOT_MASK);
}

static UINT ntn_boot_from_flash_done(struct DWC_ETH_QOS_prv_data *pdata)
{
  UINT rd_val;
  NTN_NCTLSTS_RgRd(rd_val);
  /* Per datasheet bit0=1 indicates that HW sequencer is idle/complete. */
  return rd_val & NTN_NCTLSTS_HW_SEQ_COMPLETE_MASK;
}

static void ntn_config_pme(UINT reg_val, struct DWC_ETH_QOS_prv_data *pdata)
{
	NTN_INTC_PMEINTCFG_RgWr(reg_val);
}

static int DWC_ETH_QOS_yinit_offload(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->DWC_ETH_QOS_yinit_offload\n");

	/*Init offload TX/RX pipes */
	configure_dma_tx_channel(NTN_TX_DMA_CH_2, pdata);
	configure_dma_rx_channel(NTN_RX_DMA_CH_0, pdata);

	DBGPR("-->DWC_ETH_QOS_yinit_offload\n");
	return Y_SUCCESS;
}

static int DWC_ETH_QOS_yexit_offload(void)
{
	DBGPR("-->DWC_ETH_QOS_yexit_offload\n");
	DBGPR("-->DWC_ETH_QOS_yexit_offload\n");
	return Y_SUCCESS;
}

static bool ntn_fw_ipa_supported(struct DWC_ETH_QOS_prv_data *pdata)
{
	UINT reg_val = ntn_reg_rd((NTN_M3_DBG_CNT_START + NTN_M3_DBG_RSVD_OFST),
				  PCIE_SRAM_BAR_NUM, pdata);
	return reg_val & NTN_M3_DBG_IPA_CAPABLE_MASK;
}

static void enable_offload(struct DWC_ETH_QOS_prv_data *pdata)
{
  UINT reg_val;

  reg_val = ntn_reg_rd((NTN_M3_DBG_CNT_START + NTN_M3_DBG_RSVD_OFST),
                       PCIE_SRAM_BAR_NUM, pdata);
  reg_val = (reg_val | IPA_ENABLE_OFFLOAD_MASK);
  ntn_reg_wr((NTN_M3_DBG_CNT_START + NTN_M3_DBG_RSVD_OFST),
             reg_val, PCIE_SRAM_BAR_NUM, pdata);
  NDBGPR_L1("Enabled FW ipa offload, 0x%x\n", reg_val);
}

static void disable_offload(struct DWC_ETH_QOS_prv_data *pdata)
{
  UINT reg_val;

  reg_val = ntn_reg_rd((NTN_M3_DBG_CNT_START + NTN_M3_DBG_RSVD_OFST),
                       PCIE_SRAM_BAR_NUM, pdata);
  reg_val = reg_val & IPA_DISABLE_OFFLOAD_MASK;
  ntn_reg_wr((NTN_M3_DBG_CNT_START + NTN_M3_DBG_RSVD_OFST),
             reg_val, PCIE_SRAM_BAR_NUM, pdata);
  NDBGPR_L1("Disabled FW ipa offload, 0x%x\n", reg_val);
}

/*!
 * * \brief Initialises device registers.
 * * \details This function initialises device registers.
 * *
 * * \return none
 * */

static INT DWC_ETH_QOS_yinit(struct DWC_ETH_QOS_prv_data *pdata)
{
        UINT qInx, chInx;

        DBGPR("-->DWC_ETH_QOS_yinit\n");

        /* reset mmc counters */
        MMC_CNTRL_RgWr(0x1);

        for (qInx = 0; qInx < NTN_TX_QUEUE_CNT; qInx++) {
                if(!pdata->tx_q_for_host[qInx])
                        continue;
                configure_tx_mtl_queue(qInx, pdata);
        }

        for (qInx = 0; qInx < NTN_RX_QUEUE_CNT; qInx++) {
                if(!pdata->rx_q_for_host[qInx])
                        continue;
                configure_rx_mtl_queue(qInx, pdata);
        }

#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
        /* enable tx drop status */
        MTL_OMR_DTXSTS_UdfWr(0x1);
#endif

        configure_mac(pdata);

        DMA_BUSCFG_RgWr(0x0);

        for (chInx = 0; chInx < NTN_TX_DMA_CH_CNT; chInx++) {

                if(!pdata->tx_dma_ch_for_host[chInx])
                        continue;

                /* DMA CH to be enabled when IPA uC is ready */
                if (pdata->ipa_enabled && chInx == NTN_TX_DMA_CH_2)
                        continue;

                configure_dma_tx_channel(chInx, pdata);
        }
        for (chInx = 0; chInx < NTN_RX_DMA_CH_CNT; chInx++) {

                if(!pdata->rx_dma_ch_for_host[chInx])
                        continue;

                /* DMA CH to be enabled when IPA uC is ready */
                if (pdata->ipa_enabled && chInx == NTN_RX_DMA_CH_0)
                        continue;

                configure_dma_rx_channel(chInx, pdata);
        }

#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT_HALFDUPLEX
        MTL_Q0ROMR_FEP_UdfWr(0x1);
        MAC_MPFR_RA_UdfWr(0x1);
        MAC_MCR_BE_UdfWr(0x1);
#endif

        DBGPR("<--DWC_ETH_QOS_yinit\n");

        return Y_SUCCESS;
}

/*!
* \brief API to initialize the function pointers.
*
* \details This function is called in probe to initialize all the
* function pointers which are used in other functions to capture
* the different device features.
*
* \param[in] hw_if - pointer to hw_if_struct structure.
*
* \return void.
*/

void DWC_ETH_QOS_init_function_ptrs_dev(struct hw_if_struct *hw_if)
{

	DBGPR("-->DWC_ETH_QOS_init_function_ptrs_dev\n");

	hw_if->tx_complete = tx_complete;
	hw_if->tx_window_error = NULL;
	hw_if->tx_aborted_error = tx_aborted_error;
	hw_if->tx_carrier_lost_error = tx_carrier_lost_error;
	hw_if->tx_fifo_underrun = tx_fifo_underrun;
	hw_if->tx_get_collision_count = NULL;
	hw_if->tx_handle_aborted_error = NULL;
	hw_if->tx_update_fifo_threshold = NULL;
	hw_if->tx_config_threshold = NULL;

	hw_if->set_promiscuous_mode = set_promiscuous_mode;
	hw_if->set_all_multicast_mode = set_all_multicast_mode;
	hw_if->set_multicast_list_mode = set_multicast_list_mode;
	hw_if->set_unicast_mode = set_unicast_mode;

	hw_if->enable_rx_csum = enable_rx_csum;
	hw_if->disable_rx_csum = disable_rx_csum;
	hw_if->get_rx_csum_status = get_rx_csum_status;

	hw_if->write_phy_regs = write_phy_regs;
	hw_if->read_phy_regs = read_phy_regs;
	hw_if->set_full_duplex = set_full_duplex;
	hw_if->set_half_duplex = set_half_duplex;
	hw_if->set_mii_speed_100 = set_mii_speed_100;
	hw_if->set_mii_speed_10 = set_mii_speed_10;
	hw_if->set_gmii_speed = set_gmii_speed;
	/* for PMT */
	hw_if->start_dma_rx = start_dma_rx;
	hw_if->stop_dma_rx = stop_dma_rx;
	hw_if->start_dma_tx = start_dma_tx;
	hw_if->stop_dma_tx = stop_dma_tx;
	hw_if->start_mac_tx_rx = start_mac_tx_rx;
	hw_if->stop_mac_tx_rx = stop_mac_tx_rx;

	hw_if->pre_xmit = pre_transmit;
	hw_if->dev_read = device_read;
	hw_if->init = DWC_ETH_QOS_yinit;
	hw_if->exit = DWC_ETH_QOS_yexit;
	hw_if->read_fw_ver_features = read_fw_ver_features;
	hw_if->init_offload = DWC_ETH_QOS_yinit_offload;
	hw_if->exit_offload = DWC_ETH_QOS_yexit_offload;
	hw_if->enable_offload = enable_offload;
	hw_if->disable_offload = disable_offload;
	hw_if->ntn_fw_ipa_supported = ntn_fw_ipa_supported;
	/* Descriptor related Sequences have to be initialized here */
	hw_if->tx_desc_init = tx_descriptor_init;
	hw_if->rx_desc_init = rx_descriptor_init;
	hw_if->rx_desc_reset = rx_descriptor_reset;
	hw_if->tx_desc_reset = tx_descriptor_reset;
	hw_if->get_tx_desc_ls = get_tx_descriptor_last;
	hw_if->get_tx_desc_ctxt = get_tx_descriptor_ctxt;
	hw_if->update_rx_tail_ptr = update_rx_tail_ptr;

	/* for FLOW ctrl */
	hw_if->enable_rx_flow_ctrl = enable_rx_flow_ctrl;
	hw_if->disable_rx_flow_ctrl = disable_rx_flow_ctrl;
	hw_if->enable_tx_flow_ctrl = enable_tx_flow_ctrl;
	hw_if->disable_tx_flow_ctrl = disable_tx_flow_ctrl;

	/* for PMT operation */
	hw_if->enable_magic_pmt = enable_magic_pmt_operation;
	hw_if->disable_magic_pmt = disable_magic_pmt_operation;
	hw_if->enable_remote_pmt = enable_remote_pmt_operation;
	hw_if->disable_remote_pmt = disable_remote_pmt_operation;
	hw_if->configure_rwk_filter = configure_rwk_filter_registers;

	/* for TX vlan control */
	hw_if->enable_vlan_reg_control = configure_reg_vlan_control;
	hw_if->enable_vlan_desc_control = configure_desc_vlan_control;

	/* for rx vlan stripping */
	hw_if->config_rx_outer_vlan_stripping =
	    config_rx_outer_vlan_stripping;
	hw_if->config_rx_inner_vlan_stripping =
	    config_rx_inner_vlan_stripping;

	/* for sa(source address) insert/replace */
	hw_if->configure_mac_addr0_reg = configure_mac_addr0_reg;
	hw_if->configure_mac_addr1_reg = configure_mac_addr1_reg;
	hw_if->configure_sa_via_reg = configure_sa_via_reg;

	/* for RX watchdog timer */
	hw_if->config_rx_watchdog = config_rx_watchdog_timer;

	/* for RX and TX threshold config */
	hw_if->config_rx_threshold = config_rx_threshold;
	hw_if->config_tx_threshold = config_tx_threshold;

	/* for RX and TX Store and Forward Mode config */
	hw_if->config_rsf_mode = config_rsf_mode;
	hw_if->config_tsf_mode = config_tsf_mode;

	/* for TX DMA Operating on Second Frame config */
	hw_if->config_osf_mode = config_osf_mode;

	/* for INCR/INCRX config */
	hw_if->config_incr_incrx_mode = config_incr_incrx_mode;
	/* for AXI PBL config */
	hw_if->config_axi_pbl_val = config_axi_pbl_val;
	/* for AXI WORL config */
	hw_if->config_axi_worl_val = config_axi_worl_val;
	/* for AXI RORL config */
	hw_if->config_axi_rorl_val = config_axi_rorl_val;

	/* for RX and TX PBL config */
	hw_if->config_rx_pbl_val = config_rx_pbl_val;
	hw_if->get_rx_pbl_val = get_rx_pbl_val;
	hw_if->config_tx_pbl_val = config_tx_pbl_val;
	hw_if->get_tx_pbl_val = get_tx_pbl_val;
	hw_if->config_pblx8 = config_pblx8;

	hw_if->disable_rx_interrupt = disable_rx_interrupt;
	hw_if->enable_rx_interrupt = enable_rx_interrupt;

	/* for handling MMC */
	hw_if->disable_mmc_interrupts = disable_mmc_interrupts;
	hw_if->config_mmc_counters = config_mmc_counters;

	hw_if->set_dcb_algorithm = set_dcb_algorithm;
	hw_if->set_dcb_queue_weight = set_dcb_queue_weight;

	hw_if->set_tx_queue_operating_mode = set_tx_queue_operating_mode;
	hw_if->set_avb_algorithm = set_avb_algorithm;
	hw_if->config_credit_control = config_credit_control;
	hw_if->config_send_slope = config_send_slope;
	hw_if->config_idle_slope = config_idle_slope;
	hw_if->config_high_credit = config_high_credit;
	hw_if->config_low_credit = config_low_credit;
	hw_if->config_slot_num_check = config_slot_num_check;
	hw_if->config_advance_slot_num_check = config_advance_slot_num_check;

	/* for hw time stamping */
	hw_if->config_hw_time_stamping = config_hw_time_stamping;
	hw_if->config_sub_second_increment = config_sub_second_increment;
	hw_if->init_systime = init_systime;
	hw_if->config_addend = config_addend;
	hw_if->adjust_systime = adjust_systime;
	hw_if->get_systime = get_systime;
	hw_if->get_tx_tstamp_status = get_tx_tstamp_status;
	hw_if->get_tx_tstamp = get_tx_tstamp;
	hw_if->get_tx_tstamp_status_via_reg = get_tx_tstamp_status_via_reg;
	hw_if->get_tx_tstamp_via_reg = get_tx_tstamp_via_reg;
	hw_if->rx_tstamp_available = rx_tstamp_available;
	hw_if->get_rx_tstamp_status = get_rx_tstamp_status;
	hw_if->get_rx_tstamp = get_rx_tstamp;
	hw_if->drop_tx_status_enabled = drop_tx_status_enabled;

	/* for l3 and l4 layer filtering */
	hw_if->config_l2_da_perfect_inverse_match = config_l2_da_perfect_inverse_match;
	hw_if->update_mac_addr3_31_low_high_reg = update_mac_addr3_31_low_high_reg;
	hw_if->update_hash_table_reg = update_hash_table_reg;
	hw_if->config_mac_pkt_filter_reg = config_mac_pkt_filter_reg;
	hw_if->config_l3_l4_filter_enable = config_l3_l4_filter_enable;
	hw_if->config_l3_filters = config_l3_filters;
	hw_if->update_ip4_addr0 = update_ip4_addr0;
	hw_if->update_ip4_addr1 = update_ip4_addr1;
	hw_if->update_ip6_addr = update_ip6_addr;
	hw_if->config_l4_filters = config_l4_filters;
	hw_if->update_l4_sa_port_no = update_l4_sa_port_no;
	hw_if->update_l4_da_port_no = update_l4_da_port_no;

	/* for VLAN filtering */
	hw_if->get_vlan_hash_table_reg = get_vlan_hash_table_reg;
	hw_if->update_vlan_hash_table_reg = update_vlan_hash_table_reg;
	hw_if->update_vlan_id = update_vlan_id;
	hw_if->config_vlan_filtering = config_vlan_filtering;
   	hw_if->config_mac_for_vlan_pkt = configure_mac_for_vlan_pkt;
	hw_if->get_vlan_tag_comparison = get_vlan_tag_comparison;

	/* for differnet PHY interconnect */
	hw_if->control_an = control_an;
	hw_if->get_an_adv_pause_param = get_an_adv_pause_param;
	hw_if->get_an_adv_duplex_param = get_an_adv_duplex_param;
	hw_if->get_lp_an_adv_pause_param = get_lp_an_adv_pause_param;
	hw_if->get_lp_an_adv_duplex_param = get_lp_an_adv_duplex_param;

	/* for EEE */
	hw_if->set_eee_mode = set_eee_mode;
	hw_if->reset_eee_mode = reset_eee_mode;
	hw_if->set_eee_pls = set_eee_pls;
	hw_if->set_eee_timer = set_eee_timer;
	hw_if->get_lpi_status = get_lpi_status;
	hw_if->set_lpi_tx_automate = set_lpi_tx_automate;

	/* for ARP */
	hw_if->config_arp_offload = config_arp_offload;
	hw_if->update_arp_offload_ip_addr = update_arp_offload_ip_addr;

	/* for MAC loopback */
	hw_if->config_mac_loopback_mode = config_mac_loopback_mode;

	/* for PFC */
	hw_if->config_pfc = config_pfc;


    /* for MAC Double VLAN Processing config */
	hw_if->config_tx_outer_vlan = config_tx_outer_vlan;
	hw_if->config_tx_inner_vlan = config_tx_inner_vlan;
	hw_if->config_svlan = config_svlan;
	hw_if->config_dvlan = config_dvlan;
	hw_if->config_rx_outer_vlan_stripping = config_rx_outer_vlan_stripping;
	hw_if->config_rx_inner_vlan_stripping = config_rx_inner_vlan_stripping;

	/* for PTP Offloading */
	hw_if->config_ptpoffload_engine = config_ptpoffload_engine;


        /* for Neutrino wraper */
        hw_if->ntn_mac_clock_config = ntn_mac_clock_config;
        hw_if->ntn_mac_reset_config = ntn_mac_reset_config;

	/* for PCIe tamap */
	hw_if->ntn_config_tamap = ntn_config_tamap;

	/* for register read-write */
	hw_if->ntn_reg_rd = ntn_reg_rd;
	hw_if->ntn_reg_wr = ntn_reg_wr;

	/* For Neutrino Wrapper Timestamp Feature disable Enbale */
	hw_if->ntn_wrap_ts_ignore_config = ntn_wrap_ts_ignore_config;
	hw_if->ntn_wrap_ts_valid_window_config = ntn_wrap_ts_valid_window_config;

	/* Config EMAC Div and Control register */
	hw_if->ntn_set_tx_clk_125MHz = ntn_set_tx_clk_125MHz;
	hw_if->ntn_set_tx_clk_25MHz = ntn_set_tx_clk_25MHz;
	hw_if->ntn_set_tx_clk_2_5MHz = ntn_set_tx_clk_2_5MHz;

	/* Determine if boot mode is host initiated */
	hw_if->ntn_boot_host_initiated = ntn_boot_host_initiated;
	hw_if->ntn_boot_from_flash_done = ntn_boot_from_flash_done;

	/* Configure PME pin for WOL */
	hw_if->ntn_config_pme = ntn_config_pme;

	/* Config Control register for PHY Interface Mode (MII/RMII or default RGMII) */
	hw_if->ntn_set_phy_intf = ntn_set_phy_intf;

	DBGPR("<--DWC_ETH_QOS_init_function_ptrs_dev\n");
}
