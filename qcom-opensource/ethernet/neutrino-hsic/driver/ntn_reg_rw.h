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
 

#ifndef	__NTN_REG_RW_H
#define	__NTN_REG_RW_H

//for ntn_reg_read_i2c and ntn_reg_write_i2c
//#include "../../test/common/drv_intrf.h"

void ntn_delay(u32 delay);
int ntn_reg_read_8bits(struct usbnet *dev, u32 addr, u32 *val);
int ntn_reg_write_8bits(struct usbnet *dev, u32 addr, u32 val);
int ntn_read(u32 addr, u32 *val);
int ntn_write(u32 addr, u32 val);
int ntn_write_async(u32 addr, u32 val);
int ntn_reg_read(struct usbnet *dev, u32 addr, u32 *val);
int ntn_reg_write(struct usbnet *dev, u32 addr, u32 val);
int ntn_reg_write_async(struct usbnet *dev, u32 addr, u32 val);
void ntn_reg_set_bits(struct usbnet *dev, u32 addr, u32 bit_pos);
void ntn_reg_clear_bits(struct usbnet *dev, u32 addr, u32 bit_pos);
bool ntn_reg_check_bits(struct usbnet *dev, u32 addr, u32 bit_pos);

#endif //__NTN_REG_RW_H
