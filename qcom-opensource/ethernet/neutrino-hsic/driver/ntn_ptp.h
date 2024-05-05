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
 

#ifndef	__NTN_PTP_H
#define	__NTN_PTP_H

#include "ntn_common.h"
#include "ntn_gmac.h"
#include "ntn_reg_rw.h"

#define  SYNC_MSG_TYPE 0x00
#define  SYNC_FOLLOWUP_MSG_TYPE 0x08
#define  PDELAY_REQ_MSG_TYPE 0x02
#define  PDELAY_RESP_MSG_TYPE 0x03
#define  PDELAY_RESP_FOLLOWUP_MSG_TYPE 0x0a

#define  ONE_NS  0x3B9ACA00
#define Y_SUCCESS 1
#define Y_FAILURE 0

struct ntn_ptp_data
{
    /*PTP Clock Parameters*/
    u32 flags;
    struct ptp_clock *ptp_clock;
    struct ptp_clock_info ptp_caps;
    struct delayed_work ptp_overflow_work;
    struct work_struct ptp_tx_work;
    struct sk_buff *ptp_tx_skb;
    spinlock_t tmreg_lock;
    struct timecounter tc;
    struct usbnet *dev;
}__attribute__ ((packed));

extern struct ntn_ptp_data ntn_pdata;

void ntn_ptp_tx_work(struct work_struct *work);
int ntn_ptp_tx_hwtstamp(struct usbnet *dev,struct sk_buff *skb);
int ntn_ptp_rx_hwtstamp(struct usbnet *dev,struct sk_buff *skb);
int ntn_ptp_hwtstamp_ioctl(struct net_device *net,struct ifreq *ifr, int cmd);
int ntn_ptp_init(struct ntn_data *pdata);
int ntn_phc_index(struct ntn_data *pdata);
void plat_delay(u32 delay);
void ntn_ptp_remove(struct ntn_data *pdata);
int config_sub_second_increment(unsigned long ptp_clock);
int config_addend(unsigned int data);
int init_systime(unsigned int sec,unsigned int nsec);

#endif //__NTN_PTP_H
