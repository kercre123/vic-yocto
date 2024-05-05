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
 

#include "ntn_ptp.h"


/*  Delay generator */
void plat_delay(u32 delay)
{
	while(delay--);
}

/* \brief This sequence is used configure MAC SSIR
* \param[in] ptp_clock
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

int config_sub_second_increment(unsigned long ptp_clock)
{
  unsigned long val;
  unsigned long varMAC_TCR;

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
  DBGPR("fine/coarse value %ld ptp clock %ld\n", val, ptp_clock);

  /* 0.465ns accurecy */
  if (GET_VALUE(varMAC_TCR, MAC_TCR_TSCTRLSSR_LPOS, MAC_TCR_TSCTRLSSR_HPOS) == 0) {
    val = (val * 1000) / 465;
  }

  MAC_SSIR_SSINC_UdfWr(val);
  DBGPR("increment value %lx\n", val);

  return Y_SUCCESS;
}



/*!
* \brief This sequence is used get 64-bit system time in nano sec
* \return (unsigned long long) on success
* \retval ns
*/

static unsigned long long get_systime(void)
{
  	unsigned long long ns;
	  unsigned long varmac_stnsr;
	unsigned long varmac_stsr1, varmac_stsr2;

	/* Read the seconds once */
	MAC_STSR_RgRd(varmac_stsr1);

	while (1) {
		/* Read the nanoseconds */
		MAC_STNSR_RgRd(varmac_stnsr);
		/* Extract 30-bit value for Nano second register */
		ns = GET_VALUE(varmac_stnsr, MAC_STNSR_TSSS_LPOS, MAC_STNSR_TSSS_HPOS);

		/* Read the seconds again */
		MAC_STSR_RgRd(varmac_stsr2);

		/* If the seconds didn't roll over,
		 * break and return the time
		 */
		if (varmac_stsr1 == varmac_stsr2) {
			break;
		}

		/* If the seconds did roll over, read the time again */
		varmac_stsr1 = varmac_stsr2;
		DBGPR("%s second rollover happend\n", __func__);
	}

	/* Convert Total value in Nano second */
	ns = ns + (varmac_stsr1 * 1000000000ull);
	return ns;
}


/*!
* \brief This sequence is used to adjust/update the system time
* \param[in] sec
* \param[in] nsec
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

static int adjust_systime(unsigned int sec,
						unsigned int nsec,
			  			int add_sub,
						bool one_nsec_accuracy)
{
  unsigned long retryCount = 100000;
  unsigned long vy_count;
  volatile unsigned long varMAC_TCR;

  u64 data = 0;
//	DBGPR("%s\n", __func__);
  /* wait for previous(if any) time adjust/update to complete. */

  DBGPR("%s sec = %ld nsec = %ld add_sub = %d ns_acc = %d\n", __func__,
		sec, nsec, add_sub, one_nsec_accuracy);

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
#ifdef NTN_WRAPPER_DMA
  data = (u64)(sec * 1000000000);
  data = (u64)(data + nsec);
  NTN_PTPLCLUPDT_RgWr((u32)data);
#endif

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
  DBGPR("value of TCR in adjust_systime %x\n", varMAC_TCR);

  return Y_SUCCESS;
}



/*!
* \brief This sequence is used to adjust the ptp operating frequency.
* \return Success or Failure
* \retval  0 Success
* \retval -1 Failure
*/

int config_addend(unsigned int data)
{
  unsigned long retryCount = 100000;
  unsigned long vy_count;
  volatile unsigned long varMAC_TCR;

  DBGPR("%s data = %d\n", __func__, data);
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
  DBGPR("value of TCR in config_addend %x\n", varMAC_TCR);

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

//static int init_systime(unsigned int sec,
int init_systime(unsigned int sec,
                        unsigned int nsec)
{
  unsigned long retryCount = 100;
  unsigned long vy_count;
  volatile unsigned long varMAC_TCR;
  u64 data = 0;

//	DBGPR("%s\n", __func__);
  /* wait for previous(if any) time initialize to complete. */

   DBGPR("%s sec = %ld nsec = %ld\n", __func__, sec, nsec);


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

#ifdef NTN_WRAPPER_DMA
  data = (u64)(sec * 1000000000);
  data = (u64)(data + nsec);
  NTN_PTPLCLINIT_RgWr((u32)data);
#endif

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
    DBGPR("value of TCR in init systime %lux\n", varMAC_TCR);
    if (GET_VALUE(varMAC_TCR, MAC_TCR_TSINIT_LPOS, MAC_TCR_TSINIT_HPOS) == 0) {
      break;
    }
    vy_count++;
    mdelay(1);
  }

  return Y_SUCCESS;
}

/**
 * ntn_ptp_systim_to_hwtstamp - convert system time value to hw timestamp
 * @ntn_pdata: board private structure
 * @hwtstamps: timestamp structure to update
 * @systim: unsigned 64bit system time value.
 *
 * We need to convert the system time value stored in the registers
 * into a hwtstamp which can be used by the upper level timestamping functions.
 **/
static void ntn_ptp_systim_to_hwtstamp(struct usbnet *dev,
		struct skb_shared_hwtstamps *hwtstamps,
		u64 systim)
{
	memset(hwtstamps, 0, sizeof(*hwtstamps));
	/* Upper 32 bits contain sec, lower 32 bits contain nsec. */
	hwtstamps->hwtstamp = ktime_set(systim >> 32,
			systim & 0xFFFFFFFF);
}



/*!
 * \brief API to adjust the frequency of hardware clock.
 *
 * \details This function is used to adjust the frequency of the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] delta – desired period change in parts per billion.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
int ntn_ptp_adjfreq(struct ptp_clock_info *ptp, s32 ppb)
{
	struct ntn_data *pdata =
                container_of(ptp, struct ntn_data, ptp_clock_ops);
	u64 adj;
	u32 diff, addend;
	int neg_adj = 0;

//	DBGPR("ntn_ptp_adjfreq\n");
	if (ppb < 0) {
		neg_adj = 1;
		ppb = -ppb;
	}

	addend = pdata->default_addend;
	adj = addend;
	DBGPR("ntn_ptp_adjfreq: ppb %8x na %8x addend %8x\n",
		ppb, neg_adj, addend);

        adj *= ppb;
        /*
         * div_u64 will divided the "adj" by "1000000000ULL"
         * and return the quotient.
         */
        diff = div_u64(adj, 1000000000ULL);
        addend = neg_adj ? (addend - diff) : (addend + diff);

	down(&pdata->ptp_mutex);
	config_addend(addend);
	up(&pdata->ptp_mutex);

	DBGPR("ntn_ptp_adjfreq ppb = %d addend = %d\n", ppb, addend);

	return 0;
}


/*!
 * \brief API to adjust the hardware time.
 *
 * \details This function is used to shift/adjust the time of the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] delta – desired change in nanoseconds.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
int ntn_ptp_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
        struct ntn_data *pdata =
                container_of(ptp, struct ntn_data, ptp_clock_ops);
        u32 sec, nsec;
        u32 quotient, reminder;
        int neg_adj = 0;

 DBGPR("%s %lld\n", __func__, delta);
        if (delta < 0) {
                neg_adj = 1;
                delta =-delta;
        }

        quotient = div_u64_rem(delta, 1000000000ULL, &reminder);
        sec = quotient;
        nsec = reminder;

	down(&pdata->ptp_mutex);
	adjust_systime(sec, nsec, neg_adj, pdata->one_nsec_accuracy);
	up(&pdata->ptp_mutex);

	return 0;
}


/*!
 * \brief API to get the current time.
 *
 * \details This function is used to read the current time from the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] ts – pointer to hold the time/result.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
int ntn_ptp_get_time(struct ptp_clock_info *ptp, struct timespec *ts)
{
        struct ntn_data *pdata =
                container_of(ptp, struct ntn_data, ptp_clock_ops);
        u64 ns;
        u32 reminder;
	u64 temp = 0;

//	DBGPR("%s\n", __func__);

	down(&pdata->ptp_mutex);
	ns = get_systime();
	up(&pdata->ptp_mutex);

	temp = div_u64_rem(ns, 1000000000ULL, &reminder);
        ts->tv_sec = temp;
        ts->tv_nsec = reminder;

#if 0
	DBGPR("%s: tv_sec = %ld, tv_nsec = %ld\n",
			 __func__, ts->tv_sec, ts->tv_nsec);
	{
		struct timespec now;
		getnstimeofday(&now);
		DBGPR("%s: tv_sec = %ld, tv_nsec = %ld\n",
			 __func__, now.tv_sec, now.tv_nsec);
	}
#endif

	return 0;
}



/*!
 * \brief API to set the current time.
 *
 * \details This function is used to set the current time on the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] ts – time value to set.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
static int ntn_ptp_set_time(struct ptp_clock_info *ptp,
        const struct timespec *ts)
{
        struct ntn_data *pdata =
                container_of(ptp, struct ntn_data, ptp_clock_ops);
#if 0
	DBGPR("%s: ts->tv_sec = %ld, ts->tv_nsec = %ld\n",
			__func__, ts->tv_sec, ts->tv_nsec);
	{
		struct timespec now;
		getnstimeofday(&now);
		DBGPR("%s: tv_sec = %ld, tv_nsec = %ld\n",
			 __func__, now.tv_sec, now.tv_nsec);
	}
#endif
	down(&pdata->ptp_mutex);

	init_systime(ts->tv_sec, ts->tv_nsec);

	up(&pdata->ptp_mutex);

        DBGPR("<--DWC_ETH_QOS_set_time\n");

        return 0;
}


static struct sk_buff *prev_skb=NULL, *prev_skb_rx=NULL;
static u32 len;
unsigned int tx_oui_track = 0;	
unsigned int rx_oui_track = 0;	


/*!
 * \brief API to Extract GPTP Tx Packet OUI time stamp and send to application layer
 *
 * \details This function is used to set the current time on the
 * hardware clock.
 *
 * \param[in] dev – pointer to USBNET device.
 * \param[in] skb – pointer to received OUI skb from HW.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
int ntn_ptp_tx_hwtstamp(struct usbnet *dev,
		struct sk_buff *skb)
{
	u8 msg_type;
	u16 eth_type;

	u32 *stamp = (u32 *)skb->data;

	eth_type = *((u16 *)stamp + 6);
	eth_type = cpu_to_be16(eth_type);
	msg_type = (*((u8 *)stamp + 14));
	msg_type &= 0xF; 

#ifndef NTN_OUI_DEBUG

        if (eth_type == ETH_TYPE_AVB_PTP && (msg_type == SYNC_MSG_TYPE || msg_type == PDELAY_RESP_MSG_TYPE || msg_type == PDELAY_REQ_MSG_TYPE))
        {
                prev_skb = skb_get(skb);
        }
        return 1;

#else
	static u32 tx_len;
	if (eth_type == ETH_TYPE_AVB_PTP && (msg_type == SYNC_MSG_TYPE || msg_type == PDELAY_RESP_MSG_TYPE || msg_type == PDELAY_REQ_MSG_TYPE))
	{
		
		prev_skb = skb_get(skb);
		tx_len = skb->len;
	
		if(tx_oui_track!=0)	printk("TX sequence issue = %d\n", tx_oui_track);
		tx_oui_track++;

	}

        else if ((eth_type == ETH_TYPE_AVB_PTP) && ((msg_type == 0x8 /* sync follow up*/) || (msg_type == 0xa /* pdly resp follow up */) || (msg_type == 0xb /* announce msg */)))
	{
		/* no need to do anything just return */
		return 1;
	}
	else{
		printk("Unexpected combination to save tx skb: Eth type = %x, msg_type = %x\n", eth_type, msg_type);
		return 0;
	}	
	return 1;
#endif
}


/*!
 * \brief API to Extract GPTP Rx Packet OUI time stamp and send to application layer
 *
 * \details This function is used to set the current time on the
 * hardware clock.
 *
 * \param[in] dev – pointer to USBNET device.
 * \param[in] skb – pointer to received OUI skb from HW.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
int ntn_ptp_rx_hwtstamp(struct usbnet *dev,
		struct sk_buff *skb)
{
	struct skb_shared_hwtstamps shhwtstamps;
	u64 hw_time;    
	u16 eth_type;
	u8 msg_type, vend_sub_type;

	u32 *stamp = (u32 *)skb->data;
        eth_type = *((u16 *)stamp + 6);
        eth_type = cpu_to_be16(eth_type);
        msg_type = (*((u8 *)stamp + 14));
        msg_type &= 0xF;

#ifndef NTN_OUI_DEBUG
        if (eth_type == ETH_TYPE_AVB_PTP && (msg_type == SYNC_MSG_TYPE || msg_type == PDELAY_RESP_MSG_TYPE || msg_type == PDELAY_REQ_MSG_TYPE))
        {
                prev_skb_rx = skb_get(skb);
                len = skb->len;
                skb->len = 0;
                return 1;
        }

        if (eth_type == ETH_TYPE_VEND) {

                vend_sub_type = (*((u8 *)stamp + 19));
                hw_time = le32_to_cpu(*(stamp + 6));
                hw_time |= (u64)le32_to_cpu(*(stamp + 7)) << 32;
                hw_time = (*(stamp + 6));
                hw_time |= (u64)(*(stamp + 7)) << 32;
                hw_time = cpu_to_be64(hw_time);

                if (vend_sub_type ==  0x01)  /*ETH_VEND_SUB_TYPE_gPTP_TX_TS*/
                {
				if (NULL == prev_skb) {
					DBGPR("%s Null prev tx skb\n", __func__);
					return 0;
				}

                        ntn_ptp_systim_to_hwtstamp(dev, &shhwtstamps, hw_time);
                        skb_tstamp_tx(prev_skb, &shhwtstamps);
                        dev_kfree_skb_any(prev_skb);
			prev_skb = NULL;
                }

                else if(vend_sub_type ==  0x00)
                {
			/* In case that the first packet is an OUI... */
			if (prev_skb_rx == NULL)
			{
				DBGPR("%s OUI packet\n", __func__);
				dev_kfree_skb_any(skb);
				return 1;
			}

                        prev_skb_rx->len = len;
                        len = prev_skb_rx->len - skb->len;

                        ntn_ptp_systim_to_hwtstamp(dev, skb_hwtstamps(skb), hw_time);
                        skb_put(skb, len);
                        memcpy(skb->data, prev_skb_rx->data, prev_skb_rx->len);
                        dev_kfree_skb_any(prev_skb_rx);
                }
        }

        return 0;

#else
	if(skb == NULL)
	{
		printk("NULL skb in ntn_ptp_rx_hwtstamp\n");
		return 0;
	}
		
	if(stamp == NULL)
	{
		printk("NULL stamp in ntn_ptp_rx_hwtstamp\n");
		return 0;
	}
		
	eth_type = *((u16 *)stamp + 6);
	eth_type = cpu_to_be16(eth_type);
	msg_type = (*((u8 *)stamp + 14));
	msg_type &= 0xF;

	if ((eth_type == ETH_TYPE_AVB_PTP) && ((msg_type == SYNC_MSG_TYPE) || (msg_type == PDELAY_RESP_MSG_TYPE) || (msg_type == PDELAY_REQ_MSG_TYPE)))
	{
		prev_skb_rx = skb_get(skb);
		len = skb->len;
		skb->len = 0;
	
		if(rx_oui_track!=0)	printk("RX sequence issue = %d\n", rx_oui_track);
		rx_oui_track++ ;

		return 1 ;
	}

        else if ((eth_type == ETH_TYPE_AVB_PTP) && ((msg_type == 0x8 /* sync follow up*/) || (msg_type == 0xa /* pdly resp follow up */) || (msg_type == 0xb /* announce msg */)))
	{
		/* no need to do anything just return */
		return 1;
	}

	else if (eth_type == ETH_TYPE_VEND) {

		vend_sub_type = (*((u8 *)stamp + 19));
		hw_time = le32_to_cpu(*(stamp + 6));
		hw_time |= (u64)le32_to_cpu(*(stamp + 7)) << 32;
		hw_time = (*(stamp + 6));
		hw_time |= (u64)(*(stamp + 7)) << 32;
		hw_time = cpu_to_be64(hw_time);
		if ((vend_sub_type ==  0x01)) //ETH_VEND_SUB_TYPE_gPTP_TX_TS
		{	
			
				if(tx_oui_track == 0){
					printk("Ignoring TX OUI as it is orphan = %d\n", tx_oui_track);
					return 1;
				}
				if(tx_oui_track!=1)	printk("TX sequence issue when OUI received = %d\n", tx_oui_track);
				tx_oui_track = 0;	

				if(NULL == prev_skb){
					printk("Null prev tx skb\n");
					return 0;
				}



				ntn_ptp_systim_to_hwtstamp(dev, &shhwtstamps, hw_time);
				skb_tstamp_tx(prev_skb, &shhwtstamps);
				dev_kfree_skb_any(prev_skb); 

	
		}
		else if(vend_sub_type ==  0x00) //ETH_VEND_SUB_TYPE_gPTP_RX_TS
		{
#if 1
			if(rx_oui_track == 0){
				printk("Ignoring RX OUI as it is orphan = %d\n", rx_oui_track);
				return 1;
			}
			if(rx_oui_track!=1)	printk("RX sequence issue when OUI received = %d\n", rx_oui_track);
			rx_oui_track = 0;
#endif
			if(NULL == prev_skb_rx){
				printk("Null prev rx skb\n");
				return 0;
			}

			prev_skb_rx->len = len;
			len = prev_skb_rx->len - skb->len;

			ntn_ptp_systim_to_hwtstamp(dev, skb_hwtstamps(skb), hw_time);
			
			skb_put(skb, len);
			memcpy(skb->data, prev_skb_rx->data, prev_skb_rx->len);

			dev_kfree_skb_any(prev_skb_rx);	
 		}
		else{
			printk("Found unidentified OUI\n");
			dev_kfree_skb_any(skb);
		}
	}
	else{
		printk("Unexpected combination: Eth type = %x, msg_type = %x\n", eth_type, msg_type);
		return 0;
	}
	return 0;
#endif
}


/**
 * ntn_ptp_hwtstamp_ioctl - control hardware time stamping
 * @netdev:
 * @ifre:
 * @cmd:
 *
 * Outgoing time stamping can be enabled and disabled. Play nice and
 * disable it when reuested, although it shouldn't case any overhead
 * when no packet needs it. At most one packet in the ueue may be
 * marked for time stamping, otherwise it would be impossible to tell
 * for sure to which packet the hardware time stamp belongs.
 *
 * Incoming time stamping has to be configured via the hardware
 * filters. Not all combinations are supported, in particular event
 * type has to be specified. Matching the kind of event packet is
 * not supported 
 *
 **/
int ntn_ptp_hwtstamp_ioctl(struct net_device *net,
		struct ifreq *ifr, int cmd)
{
	struct hwtstamp_config config;
	u32 tsync_tx_ctl = 0;
	u32 tsync_rx_ctl = 0;
	bool is_l4 = false;
	bool is_l2 = false;
	u64 ptp_temp;
	unsigned int default_addend_local;
	struct timespec now;

	struct ntn_data *priv = NULL;

	DBGPR("%s\n", __func__);

	if (ntn_pdata.dev) {
		if (ntn_pdata.dev->data[0]) {
			priv = (struct ntn_data *) ntn_pdata.dev->data[0];
			DBGPR("%s prv present\n", __func__);
		}
	}


	if (copy_from_user(&config, ifr->ifr_data, sizeof(config)))
		return -EFAULT;

	DBGPR("config.flags = %#x, tx_type = %#x, rx_filter = %#x\n",
		config.flags, config.tx_type, config.rx_filter);


	if (config.flags)
		return -EINVAL;

	switch (config.tx_type) {
		case HWTSTAMP_TX_OFF:
			tsync_tx_ctl = 0;
		case HWTSTAMP_TX_ON:
			break;
		default:
			return -ERANGE;
	}

	switch (config.rx_filter) {
		case HWTSTAMP_FILTER_NONE:
			tsync_rx_ctl = 0;
			break;
		case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
			tsync_rx_ctl |= GmacTSCLKTYPE;
			is_l4 = true;
			break;
		case HWTSTAMP_FILTER_PTP_V2_EVENT:
		case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
		case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
		case HWTSTAMP_FILTER_PTP_V2_SYNC:
		case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
		case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
		case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
		case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
		case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
			/* Enable event message time stamping */
			config.rx_filter = config.rx_filter;


			config_sub_second_increment(DWC_ETH_QOS_SYSCLOCK);

			ptp_temp = (u64)(50000000ULL << 32);
			default_addend_local = div_u64(ptp_temp, DWC_ETH_QOS_SYSCLOCK);
			if (priv)
				priv->default_addend = default_addend_local;
			config_addend(default_addend_local);

			getnstimeofday(&now);
			init_systime(now.tv_sec, now.tv_nsec);
#if 0
			{
				struct timespec now_local;
				getnstimeofday(&now_local);
				DBGPR("%s: current time tv_sec = %ld tv_nsec = %ld\n",
					 __func__, now_local.tv_sec, now_local.tv_nsec);
			}
#endif
			break;
		case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
		case HWTSTAMP_FILTER_ALL:

		default:
			config.rx_filter = HWTSTAMP_FILTER_NONE;
			return -ERANGE;
	}

	DBGPR("config.flags = %#x, tx_type = %#x, rx_filter = %#x\n",
		config.flags, config.tx_type, config.rx_filter);

	return copy_to_user(ifr->ifr_data, &config, sizeof(config)) ?
		-EFAULT : 0;
}

/*!
 * \brief API to enable/disable an ancillary feature.
 *
 * \details This function is used to enable or disable an ancillary
 * device feature like PPS, PEROUT and EXTTS.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] rq – desired resource to enable or disable.
 * \param[in] on – caller passes one to enable or zero to disable.
 *
 * \return int
 *
 * \retval 0 on success and -ve(EINVAL or EOPNOTSUPP) number on failure.
 */

static int ntn_ptp_enable(struct ptp_clock_info *ptp,
	struct ptp_clock_request *rq, int on)
{
	return -EOPNOTSUPP;
}


/*
 * structure describing a PTP hardware clock.
 */
static struct ptp_clock_info ntn_ptp_clock_ops = {
        .owner = THIS_MODULE,
        .name = "DWC_ETH_QOS_clk",
        .max_adj = DWC_ETH_QOS_SYSCLOCK, /* the max possible frequency adjustment,
                                in parts per billion */
        .n_alarm = 0,   /* the number of programmable alarms */
        .n_ext_ts = 0,  /* the number of externel time stamp channels */
        .n_per_out = 0, /* the number of programmable periodic signals */
        .pps = 0,       /* indicates whether the clk supports a PPS callback */
        .adjfreq = ntn_ptp_adjfreq,
        .adjtime = ntn_ptp_adjtime,
        .gettime = ntn_ptp_get_time,
        .settime = ntn_ptp_set_time,
        .enable = ntn_ptp_enable,
};



/*!
 * \brief API to register ptp clock driver.
 *
 * \details This function is used to register the ptp clock
 * driver to kernel. It also does some housekeeping work.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */
int ntn_ptp_init(struct ntn_data *pdata)
{
        int ret = 0;
        DBGPR("-->ptp_init\n");

/*        if (!pdata->hw_feat.tsstssel) {
                ret = -1;
                pdata->ptp_clock = NULL;
                printk(KERN_ALERT "No PTP supports in HW\n"
                        "Aborting PTP clock driver registration\n");
                goto no_hw_ptp;
        }*/
	sema_init(&pdata->ptp_mutex, 1);

        pdata->ptp_clock_ops = ntn_ptp_clock_ops;

        pdata->ptp_clock = ptp_clock_register(&pdata->ptp_clock_ops, &pdata->udev->intf->dev);

        if (IS_ERR(pdata->ptp_clock)) {
		pdata->ptp_clock = NULL;
		printk(KERN_ALERT "ptp_clock_register() failed\n");
	} else {
		printk(KERN_ALERT "Added PTP HW clock successfully\n");
	}

        return ret;

//no_hw_ptp:
//        return ret;


}


/*!
 * \brief API to unregister ptp clock driver.
 *
 * \details This function is used to remove/unregister the ptp
 * clock driver from the kernel.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */
void ntn_ptp_remove(struct ntn_data *pdata)
{
        DBGPR("-->ntn_ptp_remove\n");

        if (pdata->ptp_clock) {
                ptp_clock_unregister(pdata->ptp_clock);
                printk(KERN_ALERT "Removed PTP HW clock successfully\n");
        }

        DBGPR("<--ntn_ptp_remove\n");
}

int ntn_phc_index(struct ntn_data *pdata)
{
    DBGPR("-->ntn_phc_index\n");
    if (pdata->ptp_clock)
         return ptp_clock_index(pdata->ptp_clock);
    else
	return 0;
}

