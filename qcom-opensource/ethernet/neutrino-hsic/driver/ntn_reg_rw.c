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
//#include "ntn_reg_rw.h"

//#define NTN_ACCESS_MAC 1

extern struct ntn_ptp_data ntn_pdata;
extern struct ntn_data *pdata_phc;

/**
  * This is a wrapper function for platform dependent delay 
  * Take care while passing the argument to this function 
  * @param[in] buffer pointer to be freed
  */
void ntn_delay(u32 delay)
{
	while (delay--);
	return;
}

static int ntn_reg_read_usb(struct usbnet *dev, u8 cmd, u16 value, u16 index, u16 size, void *data)
{
	int ret;
//	BUG_ON(!dev);

	ret = usbnet_read_cmd_nopm(dev, cmd, USB_DIR_IN | USB_TYPE_VENDOR |
		 USB_RECIP_DEVICE, value, index, data, size);

	if (unlikely(ret < 0))
		netdev_warn(dev->net, "Failed to read reg index 0x%04x: %d\n",
			    index, ret);
	return ret;
}

static int ntn_reg_write_usb(struct usbnet *dev, u8 cmd, u16 value, u16 index, u16 size, void *data)
{
	int ret;
//	BUG_ON(!dev);

	ret = usbnet_write_cmd_nopm(dev, cmd, USB_DIR_OUT | USB_TYPE_VENDOR |
		 USB_RECIP_DEVICE, value, index, data, size);

	if (unlikely(ret < 0))
		netdev_warn(dev->net, "Failed to write reg index 0x%04x: %d\n",
			    index, ret);
	return ret;
}

static int ntn_reg_write_usb_async(struct usbnet *dev, u8 cmd, u16 value, u16 index, u16 size, void *data)
{
	int ret;
//	BUG_ON(!dev);

	ret = usbnet_write_cmd_async(dev, cmd, USB_DIR_OUT | USB_TYPE_VENDOR |
		 USB_RECIP_DEVICE, value, index, data, size);

	if (unlikely(ret < 0))
		netdev_warn(dev->net, "Failed to write reg index 0x%04x: %d\n",
				index, ret);
	return ret;
}

int ntn_reg_read_8bits(struct usbnet *dev, u32 addr, u32 *val)
{
        int ret = 0;
    	ret = ntn_reg_read_usb(dev, NTN_ACCESS_MAC, addr, 4, 4, val); // FIXME: have to Replace appropriate API
        if(ret < 0)
                printk("ERROR: ntn_reg_read: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, *val);
        return ret;
}

int ntn_reg_write_8bits(struct usbnet *dev, u32 addr, u32 val)
{
        int ret;
    	ret = ntn_reg_write_usb(dev, NTN_ACCESS_MAC, addr, 4, 4, &val); // FIXME: have to Replace appropriate API
        if(ret < 0)
                printk("ERROR: ntn_reg_write: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, val);
        return ret;
}


int ntn_reg_read(struct usbnet *dev, u32 addr, u32 *val)
{
	int ret = 0;
    	ret = ntn_reg_read_usb(dev, NTN_ACCESS_MAC, addr, 4, 4, val);
	if(ret < 0)
		printk("ERROR: ntn_reg_read: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, *val);
	return ret;
}

int ntn_reg_write(struct usbnet *dev, u32 addr, u32 val)
{
	int ret;
    ret = ntn_reg_write_usb(dev, NTN_ACCESS_MAC, addr, 4, 4, &val);
//    printk("Reg access interface is not defined!!!\n");
//	if(ret < 0)
//		printk("ERROR: ntn_reg_write: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, val);
    return ret;
}

int ntn_reg_write_async(struct usbnet *dev, u32 addr, u32 val)
{
	int ret;
    ret = ntn_reg_write_usb_async(dev, NTN_ACCESS_MAC, addr, 4, 4, &val);
//    printk("Reg access interface is not defined!!!\n");
//	if(ret < 0)
//		printk("ERROR: ntn_reg_write: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, val);
    return ret;
}

int ntn_read(u32 addr, u32 *val)
{
//	struct usbnet *dev;
	int ret;
	ret = ntn_reg_read(ntn_pdata.dev, addr, val);
	if(ret < 0)
		printk("ERROR: ntn_reg_write: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, *val);
	return ret;
}


int ntn_write(u32 addr, u32 val)
{
//	struct usbnet *dev;
	int ret;
	ret = ntn_reg_write(ntn_pdata.dev, addr, val);
	if(ret < 0)
		printk("ERROR: ntn_reg_write: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, val);
	return ret;
}

int ntn_write_async(u32 addr, u32 val)
{
//	struct usbnet *dev;
	int ret;
	ret = ntn_reg_write_async(ntn_pdata.dev, addr, val);
	if(ret < 0)
		printk("ERROR: ntn_reg_write: failed. Addr: 0x%08x, Val: 0x%08x.\n", addr, val);
	return ret;
}


/**
 * The Low level function to set bits of a register in Hardware.
 * 
 * @param[in] pointer to the base of register map  
 * @param[in] Offset from the base
 * @param[in] Bit mask to set bits to logical 1 
 * \return  void 
 */
void ntn_reg_set_bits(struct usbnet *dev, u32 addr, u32 bit_pos)
{
    u32 data;

    ntn_reg_read(dev, addr, &data);
    data |= bit_pos; 
    //TR("%s !!!!!!!!!!!!!! addr = 0x%08x RegData = 0x%08x\n", __FUNCTION__, addr, data);
    ntn_reg_write(dev, addr, data);
    //TR("%s !!!!!!!!!!!!! addr = 0x%08x RegData = 0x%08x\n", __FUNCTION__, addr, data);
    return;
}

/**
 * The Low level function to clear bits of a register in Hardware.
 * 
 * @param[in] pointer to the base of register map  
 * @param[in] Offset from the base
 * @param[in] Bit mask to clear bits to logical 0 
 * \return  void 
 */
void ntn_reg_clear_bits(struct usbnet *dev, u32 addr, u32 bit_pos)
{
    u32 data;

    ntn_reg_read(dev, addr, &data);
    data &= (~bit_pos); 
    //TR("%s !!!!!!!!!!!!!! addr = 0x%08x RegData = 0x%08x\n", __FUNCTION__, addr, data);
    ntn_reg_write(dev, addr, data);
    //TR("%s !!!!!!!!!!!!! addr = 0x%08x RegData = 0x%08x\n", __FUNCTION__, addr, data);
    return;
}

/**
 * The Low level function to Check the setting of the bits.
 * 
 * @param[in] pointer to the base of register map  
 * @param[in] Offset from the base
 * @param[in] Bit mask to set bits to logical 1 
 * \return  returns TRUE if set to '1' returns FALSE if set to '0'. Result undefined there are no bit set in the bit_pos argument.
 * 
 */
bool ntn_reg_check_bits(struct usbnet *dev, u32 addr, u32 bit_pos)
{
    u32 data;

    ntn_reg_read(dev, addr, &data);
    data &= bit_pos; 
    if(data)  return true;
    else	  return false;
}
