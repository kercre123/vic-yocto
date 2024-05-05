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
/** \file
 * This file defines the synopsys GMAC device dependent functions.
 * Most of the operations on the GMAC device are available in this file.
 * Functions for initiliasing and accessing MAC/DMA/PHY registers and the DMA descriptors
 * are encapsulated in this file. The functions are platform/host/OS independent.
 * These functions in turn use the low level device dependent (HAL) functions to 
 * access the register space.
 * \internal
 * ------------------------REVISION HISTORY---------------------------------
 * Synopsys                 01/Aug/2007                              Created
 */
 
/*! History:   
 *      18-July-2016 : Initial 
 */
 
#include "ntn_common.h"
#include "ntn_gmac.h"

#include <linux/delay.h>


static u32 GMAC_Power_down; // This global variable is used to indicate the ISR whether the interrupts occured in the      process of powering down the mac or not

/*Sample Wake-up frame filter configurations*/

u32 synopGMAC_wakeup_filter_config0[] = {
	0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
	0x00000000,	// For Filter1 CRC is not computed may be it is 0x0000
	0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
	0x5F5F5F5F, // For Filter3 CRC is based on 0,1,2,3,4,6,8,9,10,11,12,14,16,17,18,19,20,22,24,25,26,27,28,30 bytes from offset
	0x09000000, // Filter 0,1,2 are disabled, Filter3 is enabled and filtering applies to only multicast packets
	0x1C000000, // Filter 0,1,2 (no significance), filter 3 offset is 28 bytes from start of Destination MAC address 
	0x00000000, // No significance of CRC for Filter0 and Filter1
	0xBDCC0000  // No significance of CRC for Filter2, Filter3 CRC is 0xBDCC
};

u32 synopGMAC_wakeup_filter_config1[] = {
	0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
	0x00000000,	// For Filter1 CRC is not computed may be it is 0x0000
	0x7A7A7A7A,	// For Filter2 CRC is based on 1,3,4,5,6,9,11,12,13,14,17,19,20,21,25,27,28,29,30 bytes from offset
	0x00000000, // For Filter3 CRC is not computed may be it is 0x0000
	0x00010000, // Filter 0,1,3 are disabled, Filter2 is enabled and filtering applies to only unicast packets
	0x00100000, // Filter 0,1,3 (no significance), filter 2 offset is 16 bytes from start of Destination MAC address 
	0x00000000, // No significance of CRC for Filter0 and Filter1
	0x0000A0FE  // No significance of CRC for Filter3, Filter2 CRC is 0xA0FE
};
u32 synopGMAC_wakeup_filter_config2[] = {
	0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
	0x000000FF,	// For Filter1 CRC is computed on 0,1,2,3,4,5,6,7 bytes from offset
	0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
	0x00000000, // For Filter3 CRC is not computed may be it is 0x0000
	0x00000100, // Filter 0,2,3 are disabled, Filter 1 is enabled and filtering applies to only unicast packets
	0x0000DF00, // Filter 0,2,3 (no significance), filter 1 offset is 223 bytes from start of Destination MAC address 
	0xDB9E0000, // No significance of CRC for Filter0, Filter1 CRC is 0xDB9E
	0x00000000  // No significance of CRC for Filter2 and Filter3 
};

/*
   The synopGMAC_wakeup_filter_config3[] is a sample configuration for wake up filter. 
   Filter1 is used here
   Filter1 offset is programmed to 50 (0x32)
   Filter1 mask is set to 0x000000FF, indicating First 8 bytes are used by the filter
   Filter1 CRC= 0x7EED this is the CRC computed on data 0x55 0x55 0x55 0x55 0x55 0x55 0x55 0x55

   Refer accompanied software DWC_gmac_crc_example.c for CRC16 generation and how to use the same.
   */

u32 synopGMAC_wakeup_filter_config3[] = {
	0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
	0x000000FF,	// For Filter1 CRC is computed on 0,1,2,3,4,5,6,7 bytes from offset
	0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
	0x00000000, // For Filter3 CRC is not computed may be it is 0x0000
	0x00000100, // Filter 0,2,3 are disabled, Filter 1 is enabled and filtering applies to only unicast packets
	0x00003200, // Filter 0,2,3 (no significance), filter 1 offset is 50 bytes from start of Destination MAC address 
	0x7eED0000, // No significance of CRC for Filter0, Filter1 CRC is 0x7EED, 
	0x00000000  // No significance of CRC for Filter2 and Filter3 
};

/**
 * Function to set the MDC clock for mdio transactiona
 *
 * @param[in] pointer to device structure.
 * @param[in] clk divider value.
 * \return Reuturns 0 on success else return the error value.
 */
s32 synopGMAC_set_mdc_clk_div(struct usbnet *dev, u32 clk_div_val)
{
	u32 orig_data;
	u32 ret;
	DBGPR("In synopGMAC_set_mdc_clk_div\n");
	ret = ntn_reg_read(dev, MAC_GMIIAR_RgOffAddr, &orig_data);
	DBGPR("MAC GMIIAR reg orig value  %x\n",orig_data);

	orig_data &= (~ 0xF00);
	orig_data |= clk_div_val;

	ret = ntn_reg_write(dev, MAC_GMIIAR_RgOffAddr, orig_data);
//	CHECK(ret, "ntn_reg_write: GmacGmiiAddr");

	return 0;
}

/**
 * Returns the current MDC divider value programmed in the ip.
 *
 * @param[in] pointer to device structure.
 * @param[in] clk divider value.
 * \return Returns the MDC divider value read.
 */
u32 synopGMAC_get_mdc_clk_div(struct usbnet *dev)
{
	u32 data;
	DBGPR("In synopGMAC_get_mdc_clk_div\n");
	ntn_reg_read(dev,MAC_GMIIAR_RgOffAddr, &data);
	data &= 0xF00;
	return data;
}

/**
 * Function to read the Phy register. The access to phy register
 * is a slow process as the data is moved accross MDI/MDO interface
 * @param[in] pointer to Register Base (It is the mac base in our case) .
 * @param[in] PhyBase register is the index of one of supported 32 PHY devices.
 * @param[in] Register offset is the index of one of the 32 phy register.
 * @param[out] u16 data read from the respective phy register (only valid iff return value is 0).
 * \return Returns 0 on success else return the error status.
 */
s32 synopGMAC_read_phy_reg(struct usbnet *dev, u32 PhyBase, u32 RegOffset, u16 *data)
{
	u32 addr;
	u32 loop_variable;
	u32 ret;
	u32 read_val = 0;

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask) | (GmiiRead) | (GmiiCsrClk1);
	addr = addr | GmiiBusy ; //Gmii busy bit

	ret = ntn_reg_write(dev, MAC_GMIIAR_RgOffAddr, addr);
//	CHECK(ret, "ntn_reg_write: GmacGmiiAddr");

	//Wait till the busy bit gets cleared with in a certain amount of time
	for(loop_variable = 0; loop_variable < 10/* DEFAULT_LOOP_VARIABLE*/; loop_variable++)
	{
		ntn_reg_read(dev, MAC_GMIIAR_RgOffAddr , &read_val);

		if (!(read_val & GmiiBusy)){
			break;
		}
		ntn_delay(DEFAULT_DELAY_VARIABLE);
	}
	if(loop_variable < 10/*DEFAULT_LOOP_VARIABLE*/) {
		ntn_reg_read(dev, MAC_GMIIDR_RgOffAddr , &read_val);
		*data = (u16)(read_val & 0xFFFF);
	}
	else{
		TR("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}


	//    return -ESYNOPGMACNOERR;
	return 0;
}

/**
 * Function to write to the Phy register. The access to phy register
 * is a slow process as the data is moved accross MDI/MDO interface
 * @param[in] pointer to Register Base (It is the mac base in our case) .
 * @param[in] PhyBase register is the index of one of supported 32 PHY devices.
 * @param[in] Register offset is the index of one of the 32 phy register.
 * @param[in] data to be written to the respective phy register.
 * \return Returns 0 on success else return the error status.
 */
s32 synopGMAC_write_phy_reg(struct usbnet *dev, u32 PhyBase, u32 RegOffset, u16 data)
{

#if 1
	u32 addr;
	u32 loop_variable;
	u32 ret, read_val;

	DBGPR("In synopGMAC_write_phy_reg\n");

	ret = ntn_reg_write(dev, MAC_GMIIDR_RgOffAddr , data);
	CHECK(ret, "ntn_reg_write");

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask) | (GmiiWrite) | (GmiiCsrClk1);
	addr = addr | GmiiBusy ; //set Gmii clk to 100-150 Mhz and Gmii busy bit

	ret = ntn_reg_write(dev, MAC_GMIIAR_RgOffAddr , data);
	CHECK(ret, "ntn_reg_write");

	//Wait till the busy bit gets cleared with in a certain amount of time
	for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++)
	{
		ntn_reg_read(dev, MAC_GMIIAR_RgOffAddr, &read_val);
		if (!(read_val & GmiiBusy)){
			break;
		}
		ntn_delay(DEFAULT_DELAY_VARIABLE);
	}
	if(loop_variable < DEFAULT_LOOP_VARIABLE)
		return -ESYNOPGMACNOERR;
	else{
		TR("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}
#endif
	return 0;
}

/**
 * Function to configure the phy in loopback mode. 
 *
 * @param[in] pointer to synopGMACdevice.
 * @param[in] enable or disable the loopback.
 * \return 0 on success else return the error status.
 * \note Don't get confused with mac loop-back synopGMAC_loopback_on(synopGMACdevice *) 
 * and synopGMAC_loopback_off(synopGMACdevice *) functions.
 */
s32 synopGMAC_phy_loopback(struct usbnet *dev, bool loopback)
{
	s32 status = -ESYNOPGMACNOERR;

	struct ntn_data *priv = (struct ntn_data *)dev->data[0];

	DBGPR("In synopGMAC_phy_loopback\n");
	if(loopback)
		status = synopGMAC_write_phy_reg(dev, priv->phy_id, PHY_CONTROL_REG, Mii_Loopback);
	else
		status = synopGMAC_write_phy_reg(dev, priv->phy_id, PHY_CONTROL_REG, Mii_NoLoopback);

	return status;
}



/**
 * Function to read the GMAC IP Version and populates the same in device data structure.
 * @param[in] pointer to synopGMACdevice.
 * \return Always return 0.
 */

s32 synopGMAC_read_version(struct usbnet *dev) 
{	
	u32 data = 0;
	u32 ret = 0;

	DBGPR("In synopGMAC_read_version\n");
	ret = ntn_reg_read(dev, MAC_VR_RgOffAddr, &data);
//	CHECK(ret, "Neutrino_read_cmd: GmacVersion");

	printk("GMAC Version is %08x\n", data);
	return 0;
}

/**
 * Function to reset the GMAC core. 
 * This reests the DMA and GMAC core. After reset all the registers holds their respective reset value
 * @param[in] pointer to synopGMACdevice.
 * \return 0 on success else return the error status.
 */
s32 synopGMAC_reset (struct usbnet *dev) 
{	
#ifdef DEBUG
	u32 data = 0;
#endif
	u32 ret = 0;

	DBGPR("In synopGMAC_read_version\n");
//	ret = ntn_reg_write(dev, DMA_SBUS_RgOffAddr , DmaResetOn);
	CHECK(ret, "ntn_reg_write: DmaBusMode");

	ntn_delay(DEFAULT_LOOP_VARIABLE);

//	ret = ntn_reg_read(dev, DMA_SBUS_RgOffAddr , &data);

	TR("After Reset: DmaBusMode: %08x\n",data);
	return 0;	
}

/**
 * Function to program DMA bus mode register. 
 * 
 * The Bus Mode register is programmed with the value given. The bits to be set are
 * bit wise or'ed and sent as the second argument to this function.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] the data to be programmed.
 * \return 0 on success else return the error status.
 */
s32 synopGMAC_dma_bus_mode_init(struct usbnet *dev, u32 init_value )
{
	u32 ret = 0;

	DBGPR("In synopGMAC_dma_bus_mode_init\n");
//	ret = ntn_reg_write(dev, DMA_SBUS_RgOffAddr , init_value);
	CHECK(ret, "ntn_reg_write: DmaBusMode");
	return 0;
}

/**
 * Function to program DMA Control register. 
 * 
 * The Dma Control register is programmed with the value given. The bits to be set are
 * bit wise or'ed and sent as the second argument to this function.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] the data to be programmed.
 * \return 0 on success else return the error status.
 */
s32 synopGMAC_dma_control_init(struct usbnet *dev, u32 init_value)
{
	u32 ret = 0;

	DBGPR("In synopGMAC_dma_control_init\n");
	ret = ntn_reg_write(dev, DmaControl, init_value);
	CHECK(ret, "ntn_reg_write: DmaControl");
	return 0;
}

/*Gmac configuration functions*/

/**
 * Enable the watchdog timer on the receiver. 
 * When enabled, Gmac enables Watchdog timer, and GMAC allows no more than
 * 2048 bytes of data (10,240 if Jumbo frame enabled).
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_wd_enable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_wd_enable\n");
#if 0
	ntn_reg_clear_bits(dev, MAC_MCR_RgOffAddr, GmacWatchdog);
#endif
	return;
}
/**
 * Disable the watchdog timer on the receiver. 
 * When disabled, Gmac disabled watchdog timer, and can receive frames up to
 * 16,384 bytes.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_wd_disable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_wd_disable\n");
#if 0
	ntn_reg_set_bits(dev, MAC_MCR_RgOffAddr, GmacWatchdog);
#endif
	return;
}

/**
 * Enables the Jabber frame support. 
 * When enabled, GMAC disabled the jabber timer, and can transfer 16,384 byte frames.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_jab_enable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_jab_enable\n");
	MAC_MCR_JD_UdfWr(1);
//	ntn_reg_set_bits(dev, MAC_MCR_RgOffAddr, GmacJabber);
	return;
}
/**
 * Disables the Jabber frame support. 
 * When disabled, GMAC enables jabber timer. It cuts of transmitter if application 
 * sends more than 2048 bytes of data (10240 if Jumbo frame enabled).
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_jab_disable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_jab_disable\n");
	MAC_MCR_JD_UdfWr(0);
//	ntn_reg_clear_bits(dev, GmacConfig, GmacJabber);
	return;
}

/**
 * Enables Frame bursting (Only in Half Duplex Mode). 
 * When enabled, GMAC allows frame bursting in GMII Half Duplex mode.
 * Reserved in 10/100 and Full-Duplex configurations.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_frame_burst_enable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_frame_burst_enable\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacFrameBurst);
	MAC_MCR_BE_UdfWr(1);
	return;
}
/**
 * Disables Frame bursting. 
 * When Disabled, frame bursting is not supported.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_frame_burst_disable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_frame_burst_disable\n");
//	ntn_reg_clear_bits(dev, GmacConfig, GmacFrameBurst);
	MAC_MCR_BE_UdfWr(0);
	return;
}

/**
 * Enable Jumbo frame support. 
 * When Enabled GMAC supports jumbo frames of 9018/9022(VLAN tagged).
 * Giant frame error is not reported in receive frame status.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_jumbo_frame_enable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_jumbo_frame_enable\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacJumboFrame);
	MAC_MCR_JE_UdfWr(1);
	return;
}
/**
 * Disable Jumbo frame support. 
 * When Disabled GMAC does not supports jumbo frames.
 * Giant frame error is reported in receive frame status.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_jumbo_frame_disable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_jumbo_frame_disable\n");
//	ntn_reg_clear_bits(dev, GmacConfig, GmacJumboFrame);
	MAC_MCR_JE_UdfWr(0);
	return;
}

/**
 * Disable Carrier sense. 
 * When Disabled GMAC ignores CRS signal during frame transmission
 * in half duplex mode.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */

void synopGMAC_disable_crs(struct usbnet *dev)
{
	DBGPR("In synopGMAC_disable_crs\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacDisableCrs);
	MAC_MCR_DCRS_UdfWr(1);
	return;
}



/**
 * Selects the GMII port. 
 * When called GMII (1000Mbps) port is selected (programmable only in 10/100/1000 Mbps configuration).
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_select_gmii(struct usbnet *dev)
{
	DBGPR("In synopGMAC_select_gmii\n");
//	ntn_reg_clear_bits(dev, GmacConfig, GmacMiiGmii);
	MAC_MCR_PS_UdfWr(0);
	return;
}
/**
 * Selects the MII port. 
 * When called MII (10/100Mbps) port is selected (programmable only in 10/100/1000 Mbps configuration).
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_select_mii(struct usbnet *dev)
{
	DBGPR("In synopGMAC_select_mii\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacMiiGmii);
	MAC_MCR_PS_UdfWr(1);
	return;
}

/**
 * Enables Receive Own bit (Only in Half Duplex Mode). 
 * When enaled GMAC receives all the packets given by phy while transmitting.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_rx_own_enable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_rx_own_enable\n");
//	ntn_reg_clear_bits(dev, GmacConfig, GmacRxOwn);
	MAC_MCR_DRO_UdfWr(0);
	return;
}
/**
 * Disables Receive Own bit (Only in Half Duplex Mode). 
 * When enaled GMAC disables the reception of frames when gmii_txen_o is asserted.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_rx_own_disable(struct usbnet *dev)
{
	DBGPR("In synopGMAC_rx_own_disable\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacRxOwn);
	MAC_MCR_DRO_UdfWr(1);
	return;
}

/**
 * Sets the GMAC in loopback mode. 
 * When on GMAC operates in loop-back mode at GMII/MII.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 * \note (G)MII Receive clock is required for loopback to work properly, as transmit clock is
 * not looped back internally.
 */
void synopGMAC_loopback_on(struct usbnet *dev)
{
	DBGPR("In synopGMAC_loopback_on\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacLoopback);
	MAC_MCR_LM_UdfWr(1);
	return;
}
/**
 * Sets the GMAC in Normal mode. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_loopback_off(struct usbnet *dev)
{
	DBGPR("In synopGMAC_loopback_off\n");
//	ntn_reg_clear_bits(dev, GmacConfig, GmacLoopback);
	MAC_MCR_LM_UdfWr(0);
	return;
}

/**
 * Sets the GMAC core in Full-Duplex mode. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_set_full_duplex(struct usbnet *dev)
{
	DBGPR("In synopGMAC_set_full_duplex\n");
//	ntn_reg_set_bits(dev, GmacConfig, GmacDuplex);
	MAC_MCR_DM_UdfWr(1);
	return;
}
/**
 * Sets the GMAC core in Half-Duplex mode. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_set_half_duplex(struct usbnet *dev)
{
	DBGPR("In synopGMAC_set_half_duplex\n");
//	ntn_reg_clear_bits(dev, GmacConfig, GmacDuplex);
	MAC_MCR_DM_UdfWr(0);
	return;
}

/**
 * GMAC tries retransmission (Only in Half Duplex mode).
 * If collision occurs on the GMII/MII, GMAC attempt retries based on the 
 * back off limit configured. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 * \note This function is tightly coupled with synopGMAC_back_off_limit(synopGMACdev *, u32).
 */
void synopGMAC_retry_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacConfig, GmacRetry);
	MAC_MCR_DR_UdfWr(0);
	return;
}
/**
 * GMAC tries only one transmission (Only in Half Duplex mode).
 * If collision occurs on the GMII/MII, GMAC will ignore the current frami
 * transmission and report a frame abort with excessive collision in tranmit frame status. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_retry_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacConfig, GmacRetry);
	MAC_MCR_DR_UdfWr(1);
	return;
}

/**
 * GMAC strips the Pad/FCS field of incoming frames.
 * This is true only if the length field value is less than or equal to
 * 1500 bytes. All received frames with length field greater than or equal to
 * 1501 bytes are passed to the application without stripping the Pad/FCS field. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_pad_crc_strip_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacConfig, GmacPadCrcStrip);
	MAC_MCR_ACS_UdfWr(1);
	return;
}
/**
 * GMAC doesnot strips the Pad/FCS field of incoming frames.
 * GMAC will pass all the incoming frames to Host unmodified. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_pad_crc_strip_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacConfig, GmacPadCrcStrip);
	MAC_MCR_ACS_UdfWr(0);
	return;
}
/**
 * GMAC programmed with the back off limit value.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 * \note This function is tightly coupled with synopGMAC_retry_enable(synopGMACdevice * gmacdev)
 */
void synopGMAC_back_off_limit(struct usbnet *dev, u32 value)
{
//	u32 data;
	DBGPR("func:%s\n",__func__);
	MAC_MCR_BL_UdfWr(value);
/*
	ntn_reg_read(dev, GmacConfig, &data);
	data &= (~GmacBackoffLimit);
	data |= value;
	ntn_reg_write(dev, GmacConfig,data);*/
	return;
}

/**
 * Enables the Deferral check in GMAC (Only in Half Duplex mode)
 * GMAC issues a Frame Abort Status, along with the excessive deferral error bit set in the 
 * transmit frame status when transmit state machine is deferred for more than
 * 	- 24,288 bit times in 10/100Mbps mode
 * 	- 155,680 bit times in 1000Mbps mode or Jumbo frame mode in 10/100Mbps operation. 
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 * \note Deferral begins when transmitter is ready to transmit, but is prevented because  of
 * an active CRS (carrier sense) 
 */
void synopGMAC_deferral_check_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacConfig, GmacDeferralCheck);
	MAC_MCR_DEFC_UdfWr(1);
	return;
}
/**
 * Disables the Deferral check in GMAC (Only in Half Duplex mode).
 * GMAC defers until the CRS signal goes inactive.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_deferral_check_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacConfig, GmacDeferralCheck);
	MAC_MCR_DEFC_UdfWr(0);
	return;
}
/**
 * Enable the reception of frames on GMII/MII.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_rx_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacConfig, GmacRx);
	MAC_MCR_RE_UdfWr(1);
	return;
}
/**
 * Disable the reception of frames on GMII/MII.
 * GMAC receive state machine is disabled after completion of reception of current frame.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_rx_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacConfig, GmacRx);
	MAC_MCR_RE_UdfWr(0);
	return;
}
/**
 * Enable the transmission of frames on GMII/MII.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_tx_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacConfig, GmacTx);
	MAC_MCR_TE_UdfWr(1);
	return;
}
/**
 * Disable the transmission of frames on GMII/MII.
 * GMAC transmit state machine is disabled after completion of transmission of current frame.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_tx_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacConfig, GmacTx);
	MAC_MCR_TE_UdfWr(0);
	return;
}


/*Receive frame filter configuration functions*/

/**
 * Enables reception of all the frames to application.
 * GMAC passes all the frames received to application irrespective of whether they
 * pass SA/DA address filtering or not.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_frame_filter_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacFilter);
	MAC_MPFR_RA_UdfWr(0);
	return;
}
/**
 * Disables reception of all the frames to application.
 * GMAC passes only those received frames to application which 
 * pass SA/DA address filtering.
 * @param[in] pointer to synopGMACdevice.
 * \return void. 
 */
void synopGMAC_frame_filter_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacFilter);
	MAC_MPFR_RA_UdfWr(1);
	return;
}

/**
 * Populates the Hash High register with the data supplied.
 * This function is called when the Hash filtering is to be enabled.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] data to be written to hash table high register.
 * \return void. 
 */
void synopGMAC_write_hash_table_high(struct usbnet *dev, u32 data)
{
	DBGPR("func:%s\n",__func__);
	//ntn_reg_write(dev,GmacHashHigh,data);
	//Spec 4.00 supports 256 bit hash table, if required, change this function
	return;
}

/**
 * Populates the Hash Low register with the data supplied.
 * This function is called when the Hash filtering is to be enabled.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] data to be written to hash table low register.
 * \return void. 
 */
void synopGMAC_write_hash_table_low(struct usbnet *dev, u32 data)
{
	DBGPR("func:%s\n",__func__);
	//Spec 4.00 supports 256 bit hash table, if required, change this function
	//ntn_reg_write(dev,GmacHashLow,data);
	return;
}

/**
 * Enables Hash or Perfect filter (only if Hash filter is enabled in H/W).
 * Only frames matching either perfect filtering or Hash Filtering as per HMC and HUC 
 * configuration are sent to application.
 * @param[in] pointer to synopGMACdevice.
 * \return void. 
 */
void synopGMAC_hash_perfect_filter_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacHashPerfectFilter);
	MAC_MPFR_HPF_UdfWr(1);
	return;
}

/**
 * Enables only Hash(only if Hash filter is enabled in H/W).
 * Only frames matching Hash Filtering as per HMC and HUC 
 * configuration are sent to application.
 * @param[in] pointer to synopGMACdevice.
 * \return void. 
 */
void synopGMAC_Hash_filter_only_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacHashPerfectFilter);
	MAC_MPFR_HPF_UdfWr(1);
	return;
}

/**
 * Enables Source address filtering.
 * When enabled source address filtering is performed. Only frames matching SA filtering are passed  to application with 
 * SAMatch bit of RxStatus is set. GMAC drops failed frames. 
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 * \note This function is overriden by synopGMAC_frame_filter_disable(synopGMACdevice *) 
 */
void synopGMAC_src_addr_filter_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacSrcAddrFilter);
	MAC_MPFR_SAF_UdfWr(1);
	return;
}
/**
 * Disables Source address filtering.
 * When disabled GMAC forwards the received frames with updated SAMatch bit in RxStatus. 
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_src_addr_filter_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacSrcAddrFilter);
	MAC_MPFR_SAF_UdfWr(0);
	return;
}
/**
 * Enables Inverse Destination address filtering.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_dst_addr_filter_inverse(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacDestAddrFilterNor);
	return;
}
/**
 * Enables the normal Destination address filtering.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_dst_addr_filter_normal(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacDestAddrFilterNor);
	return;
}

/**
 * Enables forwarding of control frames.
 * When set forwards all the control frames (incl. unicast and multicast PAUSE frames).
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 * \note Depends on RFE of FlowControlRegister[2]
 */
void synopGMAC_set_pass_control(struct usbnet *dev,u32 passcontrol)
{	
	MAC_MPFR_PCF_UdfWr(passcontrol);

/*	u32 data;
	ntn_reg_read(dev, GmacFrameFilter, &data);
	data &= (~GmacPassControl);
	data |= passcontrol;
	ntn_reg_write(dev,GmacFrameFilter,data);*/
	return;
}

/**
 * Enables Broadcast frames.
 * When enabled Address filtering module passes all incoming broadcast frames.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_broadcast_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_DBF_UdfWr(0);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacBroadcast);
	return;
}
/**
 * Disable Broadcast frames.
 * When disabled Address filtering module filters all incoming broadcast frames.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_broadcast_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_DBF_UdfWr(1);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacBroadcast);
	return;
}

/**
 * Enables Multicast frames.
 * When enabled all multicast frames are passed.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_multicast_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_PM_UdfWr(1);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacMulticastFilter);
	return;
}
/**
 * Disable Multicast frames.
 * When disabled multicast frame filtering depends on HMC bit.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_multicast_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_PM_UdfWr(0);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacMulticastFilter);
	return;
}

/**
 * Enables multicast hash filtering.
 * When enabled GMAC performs teh destination address filtering according to the hash table.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_multicast_hash_filter_enable(struct usbnet *dev)
{
	unsigned long data = 1;
	MAC_MPFR_HMC_UdfRd(data);
/*
	do {\
                MAC_MPFR_RgRd(data);\
                data = ((data >> 2) & MAC_MPFR_HMC_Mask);\
	} while(0)
*/	
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacMcastHashFilter);
	return;
}
/**
 * Disables multicast hash filtering.
 * When disabled GMAC performs perfect destination address filtering for multicast frames, it compares 
 * DA field with the value programmed in DA register.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_multicast_hash_filter_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	//MAC_MPFR_HMC_UdfRd(0);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacMcastHashFilter);
	return;
}

/**
 * Enables promiscous mode.
 * When enabled Address filter modules pass all incoming frames regardless of their Destination
 * and source addresses.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_promisc_enable(struct usbnet *dev)
{
	DBGPR("In %s %d\n", __func__, __LINE__);
	MAC_MPFR_PR_UdfWr(1);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacPromiscuousMode);
	return;
}
/**
 * Clears promiscous mode.
 * When called the GMAC falls back to normal operation from promiscous mode.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_promisc_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_PR_UdfWr(0);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacPromiscuousMode);
	return;
}


/**
 * Enables unicast hash filtering.
 * When enabled GMAC performs the destination address filtering of unicast frames according to the hash table.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_unicast_hash_filter_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_HUC_UdfWr(1);
//	ntn_reg_set_bits(dev, GmacFrameFilter, GmacUcastHashFilter);
	return;
}
/**
 * Disables multicast hash filtering.
 * When disabled GMAC performs perfect destination address filtering for unicast frames, it compares 
 * DA field with the value programmed in DA register.
 * @param[in] pointer to synopGMACdevice.
 * \return void.
 */
void synopGMAC_unicast_hash_filter_disable(struct usbnet *dev)
{
	u32 data = 0;
	DBGPR("func:%s\n",__func__);
	MAC_MPFR_HUC_UdfWr(data);
//	ntn_reg_clear_bits(dev, GmacFrameFilter, GmacUcastHashFilter);
	return;
}

/**
 * Example mac initialization sequence.
 * This function calls the initialization routines to initialize the GMAC register.
 * One can change the functions invoked here to have different configuration as per the requirement
 * @param[in] pointer to synopGMACdevice.
 * \return Returns 0 on success.
 */
s32 synopGMAC_mac_init(struct usbnet *dev)
{
	int ret;


	/* Neutrino Wrapper Register Configuration */
//	ret = ntn_reg_write(dev, InterruptMask, 0x00000000); 
//	ret = ntn_reg_write(dev, IFifoMask, 0x00000000); 
//	ret = ntn_reg_write(dev, EVBtoUSBWCtrl, 0x00000000); 
//	ret = ntn_reg_write(dev, 0x0308, 0x000003A1);
//	ret = ntn_reg_write(dev, InterruptMask, 0x00000000); 
//	ret = ntn_reg_write(dev, IFifoMask, 0x00000000); 
//	ret = ntn_reg_write(dev, T0evb_Intm, 0x00000000); 
//	ret = ntn_reg_write(dev, T1evb_Intm, 0x00000000); 

	/* Neutrino GMAC register configuration */

	ret = ntn_reg_write(dev, MTL_OMR_RgOffAddr, 0x00000060); 
	ret = ntn_reg_write(dev, MTL_QTOMR_RgOffAddr, 0x00060068);    
	ret = ntn_reg_write(dev, MTL_Q0ROMR_RgOffAddr, 0x00f00038); 
	ret = ntn_reg_write(dev, MTL_QTOMR1_RgOffAddr, 0x00060004); 
#ifdef DISABLE_CBS
	ret = ntn_reg_write(dev, MTL_QECR1_RgOffAddr, 0x00000000); 
#else
	ret = ntn_reg_write(dev, MTL_QECR1_RgOffAddr, 0x0000000C); 
#endif


#if 0  //1Gbps speed
	/* reserved bandwidth is 10 Mbps Q1 */
	ret = ntn_reg_write(dev, MTLTxQ1Quantum_Weight, 0x00000052); 
	ret = ntn_reg_write(dev, MTLTxQ1SendSlopeCredit, 0x00001FAE); 
	ret = ntn_reg_write(dev, MTLTxQ1HiCredit, 0x00016F00);
	ret = ntn_reg_write(dev, MTLTxQ1LoCredit, 0xFFF20000);



	ret = ntn_reg_write(dev, MTLTxQ2Operation_Mode, 0x00060004);
	ret = ntn_reg_write(dev, MTLTxQ2ETS_Control, 0x0000000C);

	/* reserved bandwidth is 10 Mbps for Q2 */
	ret = ntn_reg_write(dev, MTLTxQ2Quantum_Weight, 0x00000052);
	ret = ntn_reg_write(dev, MTLTxQ2SendSlopeCredit, 0x00001FAE);
	ret = ntn_reg_write(dev, MTLTxQ2HiCredit, 0x00016F00);
	ret = ntn_reg_write(dev, MTLTxQ2LoCredit, 0xFFF20000);

#else
 //100 Mbps link rate
        ret = ntn_reg_write(dev, MTL_QW1_RgOffAddr, 0x00000290);
        ret = ntn_reg_write(dev, MTL_QSSCR1_RgOffAddr, 0x00001d71);
        ret = ntn_reg_write(dev, MTL_QHCR1_RgOffAddr, 0x00100400);
        ret = ntn_reg_write(dev, MTL_QLCR1_RgOffAddr, 0xFFF31E90);

        ret = ntn_reg_write(dev, MTL_QTOMR2_RgOffAddr, 0x00060004);
        ret = ntn_reg_write(dev, MTL_QECR2_RgOffAddr, 0x0000000C);

        /* reserved bandwidth is 10 Mbps for Q2 */
        ret = ntn_reg_write(dev, MTL_QW2_RgOffAddr, 0x00000290);
        ret = ntn_reg_write(dev, MTL_QSSCR2_RgOffAddr, 0x00001d71);
        ret = ntn_reg_write(dev, MTL_QHCR2_RgOffAddr, 0x00100400);
        ret = ntn_reg_write(dev, MTL_QLCR2_RgOffAddr, 0xFFF31E90);
//         ret = ntn_reg_write(dev, MTLTxQ2LoCredit, 0xFFF31F00);

#if 0
// 15 mb reservation, LR=100 Mbps

        ret = ntn_reg_write(dev, MTLTxQ1Quantum_Weight, 0x0000069D); 
        ret = ntn_reg_write(dev, MTLTxQ1SendSlopeCredit, 0x00001999); 
        ret = ntn_reg_write(dev, MTLTxQ1HiCredit, 0x002803C0);
        ret = ntn_reg_write(dev, MTLTxQ1LoCredit, 0xFFF4CD10);

        ret = ntn_reg_write(dev, MTLTxQ2Operation_Mode, 0x00060004);
        ret = ntn_reg_write(dev, MTLTxQ2ETS_Control, 0x0000000C);

        /* reserved bandwidth is 10 Mbps for Q2 */
        ret = ntn_reg_write(dev, MTLTxQ2Quantum_Weight, 0x0000069D);
        ret = ntn_reg_write(dev, MTLTxQ2SendSlopeCredit, 0x00001999);
        ret = ntn_reg_write(dev, MTLTxQ2HiCredit, 0x002803C0);
        ret = ntn_reg_write(dev, MTLTxQ2LoCredit, 0xFFF4CD10);
#endif


#endif
        



	ret = ntn_reg_write(dev, MAC_SSIR_RgOffAddr, 0x00080000); 
	ret = ntn_reg_write(dev, MAC_MECR_RgOffAddr, 0x000007D0);
	//ret = ntn_reg_write(dev, MAC_MPFR_RgOffAddr, 0x80000000);
	ret = ntn_reg_write(dev, MAC_MCR_RgOffAddr, 0x00300003);    
	ret = ntn_reg_write(dev, MAC_VLANTR_RgOffAddr, 0x0101001a);
	ret = ntn_reg_write(dev, MAC_VLANTIRR_RgOffAddr, 0x00100000);
	ret = ntn_reg_write(dev, MAC_PMTCSR_RgOffAddr, 0x00000006);
	ret = ntn_reg_write(dev, MAC_MCR_RgOffAddr, 0x00332003);

	/* Neutrino Wrapper Register configuration */
	ret = ntn_reg_write(dev, EVBTimeOff0, 0x00000140);
	ret = ntn_reg_write(dev, EVBSIDtoT00L, 0x67A9A1DC);
	ret = ntn_reg_write(dev, EVBSIDtoTO0H, 0xA2596B15);
	ret = ntn_reg_write(dev, EVBSrcAddrL, 0xDEADBEEF);
	ret = ntn_reg_write(dev, EVBSrcAddrH, 0x0000A55A);
//	ret = ntn_reg_write(dev, T0EVB_DDACtl, 0x00000008);
//	ret = ntn_reg_write(dev, T0EVB_DDACtl, 0x003D0908); 
//	ret = ntn_reg_write(dev, T0EVB_DDANRatio, 0x00000C00);
//	ret = ntn_reg_write(dev, T0EVB_DDACtl, 0x003D0900); 
//	ret = ntn_reg_write(dev, ClockCtrl,0xFFFFFFFF); 

	return 0;


}

void synopGMAC_linux_powerdown_mac(struct usbnet *dev)
{
	TR0("Put the GMAC to power down mode..\n");
	DBGPR("func:%s\n",__func__);
	// Disable the Dma engines in tx path
	GMAC_Power_down = 1;	// Let ISR know that Mac is going to be in the power down mode
	ntn_delay(10000);		//allow any pending transmission to complete
	// Disable the Mac for both tx and rx
	synopGMAC_tx_disable(dev);
	synopGMAC_rx_disable(dev);
	ntn_delay(10000); 		//Allow any pending buffer to be read by host

	//prepare the gmac for magic packet reception and wake up frame reception
	synopGMAC_magic_packet_enable(dev);
	synopGMAC_write_wakeup_frame_register(dev, synopGMAC_wakeup_filter_config3);

	synopGMAC_wakeup_frame_enable(dev);

	//enable the Mac for reception
	synopGMAC_rx_enable(dev);

	//Enable the assertion of PMT interrupt
	synopGMAC_pmt_int_enable(dev);
	//enter the power down mode
	synopGMAC_power_down_enable(dev);
	return;
}

void synopGMAC_linux_powerup_mac(struct usbnet *dev)
{
	GMAC_Power_down = 0;	// Let ISR know that MAC is out of power down now
	DBGPR("func:%s\n",__func__);
	if( synopGMAC_is_magic_packet_received(dev))
		TR("GMAC wokeup due to Magic Pkt Received\n");
	if(synopGMAC_is_wakeup_frame_received(dev))
		TR("GMAC wokeup due to Wakeup Frame Received\n");
	//Disable the assertion of PMT interrupt
	synopGMAC_pmt_int_disable(dev);
	//Enable the mac and Dma rx and tx paths
	synopGMAC_rx_enable(dev);

	synopGMAC_tx_enable(dev);
}

// TAEC Change Start
/**
 * Sets thespeed of GMAC.
 * This function sets the GMAC config and EVB control reg to set the specific speed defiend. 
 * @param[in] usbnet dev.
 * @param[in] Speed.
 * \return none.
 */
void synopGMAC_set_speed(struct usbnet *dev, u32 speed)
{
	u32 data = 0;
	u32 gmac_config = 0;
	u32 nemacctl_speed_val = 0;

	/* Based on speed auto negotiated by PHY set GMAC registers */
	if (speed == SPEED_1000) {
		gmac_config = 0x0000;
		nemacctl_speed_val = 0;	//TX_CLK = 125MHz
	} else if (speed == SPEED_100) {
		gmac_config = 0xC000;
		nemacctl_speed_val = 2;	//TX_CLK = 25MHz
	} else if (speed == SPEED_10) {
		gmac_config = 0x8000;
		nemacctl_speed_val = 3;	//TX_CLK = 2.5MHz
	} else {
		printk("Auto-negotiation Speed selection is not correct\n"); 
		return;
	}
	/* GMAC Configuration settings */
	ntn_reg_read(dev, MAC_MCR_RgOffAddr, &data);
	DBGPR("GMAC config reg read = %x\n", data);
	data &= 0xFFFF3FFF;
	data |= gmac_config;
    	ntn_reg_write(dev, MAC_MCR_RgOffAddr, data);
	data = 0;
	ntn_reg_read(dev, MAC_MCR_RgOffAddr, &data);
	DBGPR("GMAC config reg read after write = %x\n", data);

	/* NEMACCTL settings*/
	data = 0;
	ntn_reg_read(dev, NEMACCTL, &data);
	DBGPR("NEMACCTL reg read = %x\n", data);
	data &= ~NEMACCTL_SPEED_MASK;
	data |= nemacctl_speed_val;
	ntn_reg_write(dev, NEMACCTL, data);
        data = 0;
	ntn_reg_read(dev, NEMACCTL, &data);
	DBGPR("NEMACCTL reg read after write = %x\n", data);

	return;
}
// TAEC Change End

/**
 * Checks and initialze phy.
 * This function checks whether the phy initialization is complete. 
 * @param[in] pointer to synopGMACdevice.
 * \return 0 if success else returns the error number.
 */
s32 synopGMAC_check_phy_init (struct usbnet *dev, int loopback) 
{	
	u16 data;
	s32 status = -ESYNOPGMACNOERR;		
	s32 loop_count;
	struct ntn_data *priv = (struct ntn_data *)dev->data[0];

	DBGPR("func:%s\n",__func__);

//	ret = ntn_reg_write(dev, MAC_GMIIDR_RgOffAddr, 0x0000B000); /* rgmii: MAC_MDIO_Data */			    if(ret<0) DBGPR("Mac Reg Write Failed");
//	ret = ntn_reg_write(dev, MAC_GMIIAR_RgOffAddr, 0x00320105); /* AuxCtl: MAC_MDIO_Address */            if(ret<0) DBGPR("Mac Reg Write Failed");
//	ret = ntn_reg_write(dev, MAC_GMIIDR_RgOffAddr, 0x00001140); /* csr: MAC_MDIO_Data */
//	ret = ntn_reg_write(dev, MAC_GMIIDR_RgOffAddr, 0x00002100); /* csr: MAC_MDIO_Data */
//	if(ret<0) DBGPR("Mac Reg Write Failed");

//	ret = ntn_reg_write(dev, MAC_GMIIAR_RgOffAddr, 0x00200105); /* MAC_MDIO_Address */                    if(ret<0) DBGPR("Mac Reg Write Failed");
//	ret = ntn_reg_write(dev, MAC_GMIIAR_RgOffAddr, 0x0021010D); /* MAC_MDIO_Address */                    if(ret<0) DBGPR("Mac Reg Write Failed");
//	DBGPR("ntn_dbg: %s priv->phy_id:%d\n", __func__,priv->phy_id);


	loop_count = DEFAULT_LOOP_VARIABLE;
//	loop_count = 1000;
	while(loop_count-- > 0)
	{
		status = synopGMAC_read_phy_reg(dev, priv->phy_id, PHY_STATUS_REG, &data);
		if(status)	
			return status;

		DBGPR("PHY_STATUS_REG val = 0x%08x\n", data);

		if((data & Mii_AutoNegCmplt) != 0){
			TR("Autonegotiation Complete\n");
			break;
		}
	}

	status = synopGMAC_read_phy_reg(dev, priv->phy_id, PHY_SPECIFIC_STATUS_REG, &data);
	if(status)
		return status;

	DBGPR("PHY_SPECIFIC_STATUS_REG val = 0x%08x\n", data);

	if((data & Mii_phy_status_link_up) == 0){
		printk("NTN HSIC Link is Down\n");
		/* Link is down */
		priv->LinkState = LINKDOWN; 
		priv->DuplexMode = -1;
		priv->Speed = 0;
		priv->LoopBackMode = 0; 
		netif_carrier_off(dev->net);
		return -ESYNOPGMACPHYERR;
	}
	else
		priv->LinkState = LINKUP; 

	priv->DuplexMode = (data & Mii_phy_status_full_duplex)  ? FULLDUPLEX: HALFDUPLEX ;

	if(data & Mii_phy_status_speed_1000)
		priv->Speed      =   SPEED_1000;
	else if(data & Mii_phy_status_speed_100)
		priv->Speed      =   SPEED_100;
	else
		priv->Speed      =   SPEED_10;
	

	status = synopGMAC_read_phy_reg(dev, priv->phy_id, PHY_1000BT_CTRL_REG, &data);
	if(status)
		return status;

	DBGPR("PHY_1000BT_CTRL_REG val = 0x%08x\n", data);

	if (data & Mii_Master_Mode)
		DBGPR("PHY is currently in MASTER MODE\n");	
	else
		DBGPR("PHY is currently in SLAVE MODE\n");

	printk("NTN HSIC Link is Up - %s/%s\n",
		((priv->Speed == SPEED_10) ? "10Mbps": ((priv->Speed == SPEED_100) ? "100Mbps": "1Gbps")),
		(priv->DuplexMode == FULLDUPLEX) ? "Full Duplex": "Half Duplex");

	return -ESYNOPGMACNOERR;
}

/**
 * Sets the Mac address in to GMAC register.
 * This function sets the MAC address to the MAC register in question.
 * @param[in] pointer to synopGMACdevice to populate mac dma and phy addresses.
 * @param[in] Register offset for Mac address high
 * @param[in] Register offset for Mac address low
 * @param[in] buffer containing mac address to be programmed.
 * \return 0 upon success. Error code upon failure.
 */
s32 synopGMAC_set_mac_addr(struct usbnet *dev, u32 MacHigh, u32 MacLow, u8 *MacAddr)
{
	u32 data;

	DBGPR("func:%s\n",__func__);
	data = (MacAddr[5] << 8) | MacAddr[4];
	ntn_reg_write(dev,MacHigh,data);
	ntn_reg_write(dev,MacHigh,((1<<31)|data));
	data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0] ;
	ntn_reg_write(dev,MacLow,data);
	return 0;
}


/**
 * Get the Mac address in to the address specified.
 * The mac register contents are read and written to buffer passed.
 * @param[in] pointer to synopGMACdevice to populate mac dma and phy addresses.
 * @param[in] Register offset for Mac address high
 * @param[in] Register offset for Mac address low
 * @param[out] buffer containing the device mac address.
 * \return 0 upon success. Error code upon failure.
 */
s32 synopGMAC_get_mac_addr(struct usbnet *dev, u32 MacHigh, u32 MacLow, u8 *MacAddr)
{
	u32 data;
	u32 ret;

	ret = ntn_reg_read(dev, MacHigh, &data);
//	CHECK(ret, "Neutrino_read_cmd: MacHigh");

	MacAddr[5] = (data >> 8) & 0xff;
	MacAddr[4] = (data)        & 0xff;

	ret = ntn_reg_read(dev, MacLow, &data);
//	CHECK(ret, "Neutrino_read_cmd: MacLow");
//	DBGPR("Mac low address %x\n",data);

	MacAddr[3] = (data >> 24) & 0xff;
	MacAddr[2] = (data >> 16) & 0xff;
	MacAddr[1] = (data >> 8 ) & 0xff;
	MacAddr[0] = (data )      & 0xff;

	return 0;
}

/**
 * Checks whether the tx is aborted due to collisions.
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if collisions, else returns false.
 */
bool synopGMAC_is_tx_aborted(u32 status)
{
//	return (((status & DescTxLateCollision) == DescTxLateCollision) | ((status & DescTxExcCollisions) == DescTxExcCollisions));
	return 1;
}

/**
 * Checks whether the tx carrier error.
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if carrier error occured, else returns falser.
 */
bool synopGMAC_is_tx_carrier_error(u32 status)
{
//	return (((status & DescTxLostCarrier) == DescTxLostCarrier)  | ((status & DescTxNoCarrier) == DescTxNoCarrier));
	return 1;
}


/**
 * Gives the transmission collision count.
 * returns the transmission collision count indicating number of collisions occured before the frame was transmitted.
 * Make sure to check excessive collision didnot happen to ensure the count is valid.
 * @param[in] pointer to DmaDesc structure.
 * \return returns the count value of collision.
 */
u32 synopGMAC_get_tx_collision_count(u32 status)
{
//	return ((status & DescTxCollMask) >> DescTxCollShift);
	return 1;
}
u32 synopGMAC_is_exc_tx_collisions(u32 status)
{
//	return ((status & DescTxExcCollisions) == DescTxExcCollisions);
	return 1;
}


/**
 * Check for damaged frame due to overflow or collision.
 * Retruns true if rx frame was damaged due to buffer overflow in MTL or late collision in half duplex mode.
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if error else returns false.
 */
bool synopGMAC_is_rx_frame_damaged(u32 status)
{
//	return (((status & DescRxDamaged) == DescRxDamaged) | ((status & DescRxCollision) == DescRxCollision));
	return 1;
}

/**
 * Check for damaged frame due to collision.
 * Retruns true if rx frame was damaged due to late collision in half duplex mode.
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if error else returns false.
 */
bool synopGMAC_is_rx_frame_collision(u32 status)
{
//	return ((status & DescRxCollision) == DescRxCollision);
	return 1;
}

/**
 * Check for receive CRC error.
 * Retruns true if rx frame CRC error occured.
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if error else returns false.
 */
bool synopGMAC_is_rx_crc(u32 status)
{
//	return ((status & DescRxCrc) == DescRxCrc);
	return 1;
}

/**
 * Indicates rx frame has non integer multiple of bytes. (odd nibbles).
 * Retruns true if dribbling error in rx frame.
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if error else returns false.
 */
bool synopGMAC_is_frame_dribbling_errors(u32 status)
{
//	return ((status & DescRxDribbling) == DescRxDribbling);
	return 1;
}

/**
 * Indicates error in rx frame length.
 * Retruns true if received frame length doesnot match with the length field
 * @param[in] pointer to DmaDesc structure.
 * \return returns true if error else returns false.
 */
bool synopGMAC_is_rx_frame_length_errors(u32 status)
{
//	return((status & DescRxLengthError) == DescRxLengthError);
	return 1;
}

/*******************PMT APIs***************************************/

/**
 * Enables the assertion of PMT interrupt.
 * This enables the assertion of PMT interrupt due to Magic Pkt or Wakeup frame
 * reception.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_pmt_int_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev,GmacInterruptMask,GmacPmtIntMask); 
	return;
}
/**
 * Disables the assertion of PMT interrupt.
 * This disables the assertion of PMT interrupt due to Magic Pkt or Wakeup frame
 * reception.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_pmt_int_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev,GmacInterruptMask,GmacPmtIntMask); 
	return;
}
/**
 * Enables the power down mode of GMAC.
 * This function puts the Gmac in power down mode.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_power_down_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev,GmacPmtCtrlStatus,GmacPmtPowerDown);	
	return;
}
/**
 * Disables the powerd down setting of GMAC.
 * If the driver wants to bring up the GMAC from powerdown mode, even though the magic packet or the
 * wake up frames received from the network, this function should be called.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_power_down_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev,GmacPmtCtrlStatus,GmacPmtPowerDown);	
	return;
}
/**
 * Enables the pmt interrupt generation in powerdown mode.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_enable_pmt_interrupt(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_clear_bits(dev,GmacInterruptMask,GmacPmtIntMask);	
}
/**
 * Disables the pmt interrupt generation in powerdown mode.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_disable_pmt_interrupt(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev,GmacInterruptMask,GmacPmtIntMask);	
}
/**
 * Enables GMAC to look for Magic packet.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_magic_packet_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_PMTCSR_MGKPKTEN_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacPmtCtrlStatus,GmacPmtMagicPktEnable);	
	return;
}

/**
 * Enables GMAC to look for wake up frame. 
 * Wake up frame is defined by the user.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_wakeup_frame_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_PMTCSR_RWKPKTEN_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacPmtCtrlStatus,GmacPmtWakeupFrameEnable);	
	return;
}

/**
 * Enables wake-up frame filter to handle unicast packets.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_pmt_unicast_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MAC_PMTCSR_GLBLUCAST_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacPmtCtrlStatus,GmacPmtGlobalUnicast);	
	return;
}
/**
 * Checks whether the packet received is a magic packet?.
 * @param[in] pointer to synopGMACdevice.
 * \return returns True if magic packet received else returns false.
 */
bool synopGMAC_is_magic_packet_received(struct usbnet *dev)
{
	u32 data;
	DBGPR("func:%s\n",__func__);
	MAC_PMTCSR_MGKPRCVD_UdfRd(data);
//	ntn_reg_read(dev,GmacPmtCtrlStatus, &data);	
	return data;
}
/**
 * Checks whether the packet received is a wakeup frame?.
 * @param[in] pointer to synopGMACdevice.
 * \return returns true if wakeup frame received else returns false.
 */
bool synopGMAC_is_wakeup_frame_received(struct usbnet *dev)
{
	u32 data;
	DBGPR("func:%s\n",__func__);
	MAC_PMTCSR_RWKPRCVD_UdfRd(data);
//	ntn_reg_read(dev,GmacPmtCtrlStatus, &data);	
	return data;
}

/**
 * Populates the remote wakeup frame registers.
 * Consecutive 8 writes to GmacWakeupAddr writes the wakeup frame filter registers.
 * Before commensing a new write, frame filter pointer is reset to 0x0000.
 * A small delay is introduced to allow frame filter pointer reset operation.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] pointer to frame filter contents array.
 * \return returns void.
 */
void synopGMAC_write_wakeup_frame_register(struct usbnet *dev, u32 * filter_contents)
{
	s32 i;
	MAC_PMTCSR_RWKFILTRST_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacPmtCtrlStatus,GmacPmtFrmFilterPtrReset);
	ntn_delay(10);	
	for(i =0; i<WAKEUP_REG_LENGTH; i++)
		ntn_reg_write(dev, MAC_RWPFFR_RgOffAddr ,  *(filter_contents + i));
	return;

}
/*******************PMT APIs***************************************/
/*******************MMC APIs***************************************/

/**
 * Freezes the MMC counters.
 * This function call freezes the MMC counters. None of the MMC counters are updated
 * due to any tx or rx frames until synopGMAC_mmc_counters_resume is called.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_mmc_counters_stop(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MMC_CNTRL_CNTFREEZ_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacMmcCntrl,GmacMmcCounterFreeze);
	return;
}
/**
 * Resumes the MMC counter updation.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_mmc_counters_resume(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MMC_CNTRL_CNTFREEZ_UdfWr(0);
//	ntn_reg_clear_bits(dev,GmacMmcCntrl,GmacMmcCounterFreeze);
	return;
}
/**
 * Configures the MMC in Self clearing mode.
 * Programs MMC interface so that counters are cleared when the counters are read.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_mmc_counters_set_selfclear(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MMC_CNTRL_RSTONRD_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacMmcCntrl,GmacMmcCounterResetOnRead);
	return;
}
/**
 * Configures the MMC in non-Self clearing mode.
 * Programs MMC interface so that counters are cleared when the counters are read.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_mmc_counters_reset_selfclear(struct usbnet *dev)
{
//	ntn_reg_clear_bits(dev,GmacMmcCntrl,GmacMmcCounterResetOnRead);
	MMC_CNTRL_RSTONRD_UdfWr(0);
	DBGPR("func:%s\n",__func__);
	return;
}
/**
 * Configures the MMC to stop rollover.
 * Programs MMC interface so that counters will not rollover after reaching maximum value.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_mmc_counters_disable_rollover(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MMC_CNTRL_CNTSTOPRO_UdfWr(1);
//	ntn_reg_set_bits(dev,GmacMmcCntrl,GmacMmcCounterStopRollover);
	return;
}
/**
 * Configures the MMC to rollover.
 * Programs MMC interface so that counters will rollover after reaching maximum value.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_mmc_counters_enable_rollover(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	MMC_CNTRL_CNTSTOPRO_UdfWr(0);
//	ntn_reg_clear_bits(dev,GmacMmcCntrl,GmacMmcCounterStopRollover);
	return;
}

/**
 * Read the MMC Counter.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] the counter to be read.
 * \return returns the read count value.
 */
u32 synopGMAC_read_mmc_counter(struct usbnet *dev, u32 counter)
{
	u32 data;
	DBGPR("func:%s\n",__func__);
	ntn_reg_read(dev,counter, &data);
	return data;
}
/**
 * Read the MMC Rx interrupt status.
 * @param[in] pointer to synopGMACdevice.
 * \return returns the Rx interrupt status.
 */
u32 synopGMAC_read_mmc_rx_int_status(struct usbnet *dev)
{
	u32 data;
	DBGPR("func:%s\n",__func__);
	ntn_reg_read(dev,MMC_INTR_RX_RgOffAddr, &data);
	return data;
}
/**
 * Read the MMC Tx interrupt status.
 * @param[in] pointer to synopGMACdevice.
 * \return returns the Tx interrupt status.
 */
u32 synopGMAC_read_mmc_tx_int_status(struct usbnet *dev)
{
	u32 data;
	DBGPR("func:%s\n",__func__);
	ntn_reg_read(dev,MMC_INTR_TX_RgOffAddr, &data);
	return data;
}
/**
 * Disable the MMC Tx interrupt.
 * The MMC tx interrupts are masked out as per the mask specified.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] tx interrupt bit mask for which interrupts needs to be disabled.
 * \return returns void.
 */
void synopGMAC_disable_mmc_tx_interrupt(struct usbnet *dev, u32 mask)
{

	DBGPR("func:%s\n",__func__);
	MMC_INTR_MASK_TX_RgWr(mask);
//	ntn_reg_set_bits(dev,GmacMmcIntrMaskTx,mask);
	return;
}
/**
 * Enable the MMC Tx interrupt.
 * The MMC tx interrupts are enabled as per the mask specified.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] tx interrupt bit mask for which interrupts needs to be enabled.
 * \return returns void.
 */
void synopGMAC_enable_mmc_tx_interrupt(struct usbnet *dev, u32 mask)
{
	DBGPR("func:%s\n",__func__);
	MMC_INTR_MASK_TX_RgWr(mask);
//	ntn_reg_clear_bits(dev,GmacMmcIntrMaskTx,mask);
}
/**
 * Disable the MMC Rx interrupt.
 * The MMC rx interrupts are masked out as per the mask specified.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] rx interrupt bit mask for which interrupts needs to be disabled.
 * \return returns void.
 */
void synopGMAC_disable_mmc_rx_interrupt(struct usbnet *dev, u32 mask)
{
	DBGPR("func:%s\n",__func__);
	MMC_INTR_MASK_RX_RgWr(mask);
//	ntn_reg_set_bits(dev,GmacMmcIntrMaskRx,mask);
	return;
}
/**
 * Enable the MMC Rx interrupt.
 * The MMC rx interrupts are enabled as per the mask specified.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] rx interrupt bit mask for which interrupts needs to be enabled.
 * \return returns void.
 */
void synopGMAC_enable_mmc_rx_interrupt(struct usbnet *dev, u32 mask)
{
	DBGPR("func:%s\n",__func__);
	MMC_INTR_MASK_RX_RgWr(mask);
//	ntn_reg_clear_bits(dev,GmacMmcIntrMaskRx,mask);
	return;
}
/**
 * Disable the MMC ipc rx checksum offload interrupt.
 * The MMC ipc rx checksum offload interrupts are masked out as per the mask specified.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] rx interrupt bit mask for which interrupts needs to be disabled.
 * \return returns void.
 */
void synopGMAC_disable_mmc_ipc_rx_interrupt(struct usbnet *dev, u32 mask)
{
	DBGPR("func:%s\n",__func__);
	MMC_IPC_INTR_MASK_RX_RgWr(mask);
//	ntn_reg_set_bits(dev,GmacMmcRxIpcIntrMask,mask);
	return;
}
/**
 * Enable the MMC ipc rx checksum offload interrupt.
 * The MMC ipc rx checksum offload interrupts are enabled as per the mask specified.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] rx interrupt bit mask for which interrupts needs to be enabled.
 * \return returns void.
 */
void synopGMAC_enable_mmc_ipc_rx_interrupt(struct usbnet *dev, u32 mask)
{
	DBGPR("func:%s\n",__func__);
	MMC_IPC_INTR_MASK_RX_RgWr(mask);
//	ntn_reg_clear_bits(dev,GmacMmcRxIpcIntrMask,mask);
	return;
}
/*******************MMC APIs***************************************/
/*******************Ip checksum offloading APIs***************************************/

/**
 * Enables the ip checksum offloading in receive path.
 * When set GMAC calculates 16 bit 1's complement of all received ethernet frame payload.
 * It also checks IPv4 Header checksum is correct. GMAC core appends the 16 bit checksum calculated
 * for payload of IP datagram and appends it to Ethernet frame transferred to the application.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_enable_rx_chksum_offload(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
//	ntn_reg_set_bits(dev,GmacConfig,GmacRxIpcOffload);
	MAC_MCR_IPC_UdfWr(1);
	return;
}
/**
 * Disable the ip checksum offloading in receive path.
 * Ip checksum offloading is disabled in the receive path.
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_disable_rx_Ipchecksum_offload(struct usbnet *dev)
{
//	ntn_reg_clear_bits(dev,GmacConfig,GmacRxIpcOffload);
	MAC_MCR_IPC_UdfWr(0);
}
/**
 * Instruct the DMA to drop the packets fails tcp ip checksum.
 * This is to instruct the receive DMA engine to drop the recevied packet if they 
 * fails the tcp/ip checksum in hardware. Valid only when full checksum offloading is enabled(type-2).
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_rx_tcpip_chksum_drop_enable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	ntn_reg_clear_bits(dev, DmaControl, DmaDisableDropTcpCs);
	return;
}
/**
 * Instruct the DMA not to drop the packets even if it fails tcp ip checksum.
 * This is to instruct the receive DMA engine to allow the packets even if recevied packet
 * fails the tcp/ip checksum in hardware. Valid only when full checksum offloading is enabled(type-2).
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_rx_tcpip_chksum_drop_disable(struct usbnet *dev)
{
	DBGPR("func:%s\n",__func__);
	ntn_reg_set_bits(dev, DmaControl, DmaDisableDropTcpCs);
	return;
}

/** 
 * When the Enhanced Descriptor is enabled then the bit 0 of RDES0 indicates whether the
 * Extended Status is available (RDES4). Time Stamp feature and the Checksum Offload Engine2
 * makes use of this extended status to provide the status of the received packet.
 * @param[in] pointer to synopGMACdevice
 * \return returns TRUE or FALSE
 */
#ifdef ENH_DESC_8W

/**
 * This function indicates whether extended status is available in the RDES0.
 * Any function which accesses the fields of extended status register must ensure a check on this has been made
 * This is valid only for Enhanced Descriptor.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns TRUE or FALSE.
 */
bool synopGMAC_is_ext_status(struct usbnet *dev,u32 status) 		      // extended status present indicates that the RDES4 need to be probed
{
	return((status & DescRxEXTsts ) != 0 ); // if extstatus set then it returns 1
}
/**
 * This function returns true if the IP header checksum bit is set in the extended status.
 * Valid only when enhaced status available is set in RDES0 bit 0.
 * This is valid only for Enhanced Descriptor.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns TRUE or FALSE.
 */
bool synopGMAC_ES_is_IP_header_error(struct usbnet *dev,u32 ext_status)          // IP header (IPV4) checksum error
{
	return((ext_status & DescRxIpHeaderError) != 0 ); // if IPV4 header error return 1
}
/**
 * This function returns true if the Checksum is bypassed in the hardware.
 * Valid only when enhaced status available is set in RDES0 bit 0.
 * This is valid only for Enhanced Descriptor.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns TRUE or FALSE.
 */
bool synopGMAC_ES_is_rx_checksum_bypassed(struct usbnet *dev,u32 ext_status)     // Hardware engine bypassed the checksum computation/checking
{
	return((ext_status & DescRxChkSumBypass ) != 0 ); // if checksum offloading bypassed return 1
}
/**
 * This function returns true if payload checksum error is set in the extended status.
 * Valid only when enhaced status available is set in RDES0 bit 0.
 * This is valid only for Enhanced Descriptor.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns TRUE or FALSE.
 */
bool synopGMAC_ES_is_IP_payload_error(struct usbnet *dev,u32 ext_status)         // IP payload checksum is in error (UDP/TCP/ICMP checksum error)
{
	return((ext_status & DescRxIpPayloadError) != 0 ); // if IP payload error return 1
}
#endif



/**
 * Decodes the Rx Descriptor status to various checksum error conditions.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns decoded enum (u32) indicating the status.
 */
u32 synopGMAC_is_rx_checksum_error(struct usbnet *dev, u32 status)
{
#if 0
	if     (((status & DescRxChkBit5) == 0) && ((status & DescRxChkBit7) == 0) && ((status & DescRxChkBit0) == 0))
		return RxLenLT600;
	else if(((status & DescRxChkBit5) == 0) && ((status & DescRxChkBit7) == 0) && ((status & DescRxChkBit0) != 0))
		return RxIpHdrPayLoadChkBypass;
	else if(((status & DescRxChkBit5) == 0) && ((status & DescRxChkBit7) != 0) && ((status & DescRxChkBit0) != 0))
		return RxChkBypass;
	else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) == 0) && ((status & DescRxChkBit0) == 0))
		return RxNoChkError;
	else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) == 0) && ((status & DescRxChkBit0) != 0))
		return RxPayLoadChkError;
	else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) != 0) && ((status & DescRxChkBit0) == 0))
		return RxIpHdrChkError;
	else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) != 0) && ((status & DescRxChkBit0) != 0))
		return RxIpHdrPayLoadChkError;
	else
		return RxIpHdrPayLoadRes;
#endif
	return 1;
}
/**
 * Checks if any Ipv4 header checksum error in the frame just transmitted.
 * This serves as indication that error occureed in the IPv4 header checksum insertion.
 * The sent out frame doesnot carry any ipv4 header checksum inserted by the hardware.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns true if error in ipv4 header checksum, else returns false.
 */
bool synopGMAC_is_tx_ipv4header_checksum_error(struct usbnet *dev, u32 status)
{
//	return((status & DescTxIpv4ChkError) == DescTxIpv4ChkError);
	return 1;
}


/**
 * Checks if any payload checksum error in the frame just transmitted.
 * This serves as indication that error occureed in the payload checksum insertion.
 * The sent out frame doesnot carry any payload checksum inserted by the hardware.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] u32 status field of the corresponding descriptor.
 * \return returns true if error in ipv4 header checksum, else returns false.
 */
bool synopGMAC_is_tx_payload_checksum_error(struct usbnet *dev, u32 status)
{
//	return((status & DescTxPayChkError) == DescTxPayChkError);
	return 1;	
}


/**
 * The check summ offload engine is bypassed in the tx path.
 * Checksum is not computed in the Hardware.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] Pointer to tx descriptor for which  ointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_tx_checksum_offload_bypass(struct usbnet *dev, DmaDesc *desc)
{
#ifdef ENH_DESC
//	desc->status = (desc->length & (~DescTxCisMask));//ENH_DESC
#else
//	desc->length = (desc->length & (~DescTxCisMask));
#endif

}
/**
 * The check summ offload engine is enabled to do only IPV4 header checksum.
 * IPV4 header Checksum is computed in the Hardware.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] Pointer to tx descriptor for which  ointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_tx_checksum_offload_ipv4hdr(struct usbnet *dev, DmaDesc *desc)
{
#ifdef ENH_DESC
//	desc->status = ((desc->status & (~DescTxCisMask)) | DescTxCisIpv4HdrCs);//ENH_DESC
#else
//	desc->length = ((desc->length & (~DescTxCisMask)) | DescTxCisIpv4HdrCs);
#endif

}

/**
 * The check summ offload engine is enabled to do TCPIP checsum assuming Pseudo header is available.
 * Hardware computes the tcp ip checksum assuming pseudo header checksum is computed in software.
 * Ipv4 header checksum is also inserted.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] Pointer to tx descriptor for which  ointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_tx_checksum_offload_tcponly(struct usbnet *dev, DmaDesc *desc)
{
#ifdef ENH_DESC
//	desc->status = ((desc->status & (~DescTxCisMask)) | DescTxCisTcpOnlyCs);//ENH_DESC
#else
//	desc->length = ((desc->length & (~DescTxCisMask)) | DescTxCisTcpOnlyCs);
#endif

}
/**
 * The check summ offload engine is enabled to do complete checksum computation.
 * Hardware computes the tcp ip checksum including the pseudo header checksum.
 * Here the tcp payload checksum field should be set to 0000.
 * Ipv4 header checksum is also inserted.
 * @param[in] pointer to synopGMACdevice.
 * @param[in] Pointer to tx descriptor for which  ointer to synopGMACdevice.
 * \return returns void.
 */
void synopGMAC_tx_checksum_offload_tcp_pseudo(struct usbnet *dev, DmaDesc *desc)
{
#ifdef ENH_DESC
//	desc->status = ((desc->length & (~DescTxCisMask)) | DescTxCisTcpPseudoCs);
#else
//	desc->length = ((desc->length & (~DescTxCisMask)) | DescTxCisTcpPseudoCs);
#endif

}



