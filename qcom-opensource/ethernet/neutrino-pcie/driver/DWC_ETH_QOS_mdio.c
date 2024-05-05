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
 *        3-Jun-2016 : Added prints for debug
 */

/*!@file: DWC_ETH_QOS_mdio.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"
void DWC_ETH_QOS_reload_fqtss_cfg(struct DWC_ETH_QOS_prv_data *pdata);

/*!
* \brief read MII PHY register, function called by the driver alone
*
* \details Read MII registers through the API read_phy_reg where the
* related MAC registers can be configured.
*
* \param[in] pdata - pointer to driver private data structure.
* \param[in] phyaddr - the phy address to read
* \param[in] phyreg - the phy regiester id to read
* \param[out] phydata - pointer to the value that is read from the phy registers
*
* \return int
*
* \retval  0 - successfully read data from register
* \retval -1 - error occurred
* \retval  1 - if the feature is not defined.
*/

INT DWC_ETH_QOS_mdio_read_direct(struct DWC_ETH_QOS_prv_data *pdata,
				 int phyaddr, int phyreg, int *phydata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int phy_reg_read_status;

	DBGPR_MDIO("--> DWC_ETH_QOS_mdio_read_direct: phyaddr = %d, phyreg = %d\n",
	      phyaddr, phyreg);

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return -1;
        }

	if (hw_if->read_phy_regs) {
		phy_reg_read_status =
		    hw_if->read_phy_regs(phyaddr, phyreg, phydata, pdata);
	} else {
		phy_reg_read_status = 1;
		NMSGPR_ALERT( "%s: hw_if->read_phy_regs not defined",
		       DEV_NAME);
	}

	DBGPR_MDIO("<-- DWC_ETH_QOS_mdio_read_direct: phydata = %#x\n", *phydata);

	return phy_reg_read_status;
}

/*!
* \brief write MII PHY register, function called by the driver alone
*
* \details Writes MII registers through the API write_phy_reg where the
* related MAC registers can be configured.
*
* \param[in] pdata - pointer to driver private data structure.
* \param[in] phyaddr - the phy address to write
* \param[in] phyreg - the phy regiester id
*
* to write
* \param[out] phydata - actual data to be written into the phy registers
*
* \return void
*
* \retval  0 - successfully read data from register
* \retval -1 - error occurred
* \retval  1 - if the feature is not defined.
*/

INT DWC_ETH_QOS_mdio_write_direct(struct DWC_ETH_QOS_prv_data *pdata,
				  int phyaddr, int phyreg, int phydata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int phy_reg_write_status;

	DBGPR_MDIO("--> DWC_ETH_QOS_mdio_write_direct: phyaddr = %d, phyreg = %d, phydata = %#x\n",
	      phyaddr, phyreg, phydata);

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return -1;
        }

	if (hw_if->write_phy_regs) {
		phy_reg_write_status =
		    hw_if->write_phy_regs(phyaddr, phyreg, phydata, pdata);
	} else {
		phy_reg_write_status = 1;
		NMSGPR_ALERT( "%s: hw_if->write_phy_regs not defined",
		       DEV_NAME);
	}

	DBGPR_MDIO("<-- DWC_ETH_QOS_mdio_write_direct\n");

	return phy_reg_write_status;
}

/*!
* \brief read MII PHY register.
*
* \details Read MII registers through the API read_phy_reg where the
* related MAC registers can be configured.
*
* \param[in] bus - points to the mii_bus structure
* \param[in] phyaddr - the phy address to write
* \param[in] phyreg - the phy register offset to write
*
* \return int
*
* \retval  - value read from given phy register
*/

static INT DWC_ETH_QOS_mdio_read(struct mii_bus *bus, int phyaddr, int phyreg)
{
	struct net_device *dev = bus->priv;
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int phydata;

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return -1;
        }

	DBGPR_MDIO("--> DWC_ETH_QOS_mdio_read: phyaddr = %d, phyreg = %d\n",
	      phyaddr, phyreg);

	if (hw_if->read_phy_regs) {
		hw_if->read_phy_regs(phyaddr, phyreg, &phydata, pdata);
	} else {
		NMSGPR_ALERT( "%s: hw_if->read_phy_regs not defined",
		       DEV_NAME);
	}

	DBGPR_MDIO("<-- DWC_ETH_QOS_mdio_read: phydata = %#x\n", phydata);

	return phydata;
}

/*!
* \brief API to write MII PHY register
*
* \details This API is expected to write MII registers with the value being
* passed as the last argument which is done in write_phy_regs API
* called by this function.
*
* \param[in] bus - points to the mii_bus structure
* \param[in] phyaddr - the phy address to write
* \param[in] phyreg - the phy register offset to write
* \param[in] phydata - the register value to write with
*
* \return 0 on success and -ve number on failure.
*/

static INT DWC_ETH_QOS_mdio_write(struct mii_bus *bus, int phyaddr, int phyreg,
				  u16 phydata)
{
	struct net_device *dev = bus->priv;
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	INT ret = Y_SUCCESS;

	DBGPR_MDIO("--> DWC_ETH_QOS_mdio_write: phyaddr = %d, phyreg = %d, phydata = %#x\n",
	      phyaddr, phyreg, phydata);

	/* Prevent PHY powerdown during WOL */
	if (phyreg == MII_BMCR && pdata->wolopts == WAKE_MAGIC)
		phydata &= ~BMCR_PDOWN;

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return -1;
        }

	if (hw_if->write_phy_regs) {
		hw_if->write_phy_regs(phyaddr, phyreg, phydata, pdata);
	} else {
		ret = -1;
		NMSGPR_ALERT( "%s: hw_if->write_phy_regs not defined",
		       DEV_NAME);
	}

	DBGPR_MDIO("<-- DWC_ETH_QOS_mdio_write\n");

	return ret;
}


/*!
* \brief API to reset PHY
*
* \details This API is issue soft reset to PHY core and waits
* until soft reset completes.
*
* \param[in] bus - points to the mii_bus structure
*
* \return 0 on success and -ve number on failure.
*/

static INT DWC_ETH_QOS_mdio_reset(struct mii_bus *bus)
{
	struct net_device *dev = bus->priv;
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	INT phydata;

	DBGPR_MDIO("-->DWC_ETH_QOS_mdio_reset: phyaddr : %d\n", pdata->phyaddr);

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return -ENODEV;
        }

	hw_if->read_phy_regs(pdata->phyaddr, MII_BMCR, &phydata, pdata);

	if (phydata < 0)
		return 0;

	/* issue soft reset to PHY */
	phydata |= BMCR_RESET;
	hw_if->write_phy_regs(pdata->phyaddr, MII_BMCR, phydata, pdata);

	/* wait until software reset completes */
	do {
		hw_if->read_phy_regs(pdata->phyaddr, MII_BMCR, &phydata, pdata);
	} while ((phydata >= 0) && (phydata & BMCR_RESET));

	DBGPR_MDIO("<--DWC_ETH_QOS_mdio_reset\n");

	return 0;
}

/*!
 * \details This function is invoked by other functions to get the PHY register
 * dump. This function is used during development phase for debug purpose.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return 0
 */

void dump_phy_registers(struct DWC_ETH_QOS_prv_data *pdata)
{
	int phydata = 0;

	NMSGPR_ALERT(
	       "\n************* PHY Reg dump *************************\n");
	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_BMCR, &phydata);
	NMSGPR_ALERT(
	       "Phy Control Reg(Basic Mode Control Reg) (%#x) = %#x\n",
	       MII_BMCR, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_BMSR, &phydata);
	NMSGPR_ALERT( "Phy Status Reg(Basic Mode Status Reg) (%#x) = %#x\n",
	       MII_BMSR, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_PHYSID1,
	    &phydata);
	NMSGPR_ALERT( "Phy Id (PHYS ID 1) (%#x)= %#x\n", MII_PHYSID1,
	    phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_PHYSID2,
	    &phydata);
	NMSGPR_ALERT( "Phy Id (PHYS ID 2) (%#x)= %#x\n", MII_PHYSID2,
	    phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_ADVERTISE,
	    &phydata);
	NMSGPR_ALERT( "Auto-nego Adv (Advertisement Control Reg)"\
	    " (%#x) = %#x\n", MII_ADVERTISE, phydata);

	/* read Phy Control Reg */
	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_LPA,
	    &phydata);
	NMSGPR_ALERT( "Auto-nego Lap (Link Partner Ability Reg)"\
	    " (%#x)= %#x\n", MII_LPA, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_EXPANSION,
	    &phydata);
	NMSGPR_ALERT( "Auto-nego Exp (Extension Reg)"\
	    "(%#x) = %#x\n", MII_EXPANSION, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr,
	    DWC_ETH_QOS_AUTO_NEGO_NP, &phydata);
	NMSGPR_ALERT( "Auto-nego Np (%#x) = %#x\n",
	    DWC_ETH_QOS_AUTO_NEGO_NP, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_ESTATUS,
				     &phydata);
	NMSGPR_ALERT( "Extended Status Reg (%#x) = %#x\n", MII_ESTATUS,
	       phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_CTRL1000,
	    &phydata);
	NMSGPR_ALERT( "1000 Ctl Reg (1000BASE-T Control Reg)"\
	    "(%#x) = %#x\n", MII_CTRL1000, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, MII_STAT1000,
	    &phydata);
	NMSGPR_ALERT( "1000 Sts Reg (1000BASE-T Status)(%#x) = %#x\n",
	       MII_STAT1000, phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr, DWC_ETH_QOS_PHY_CTL,
			&phydata);
	NMSGPR_ALERT( "PHY Ctl Reg (%#x) = %#x\n", DWC_ETH_QOS_PHY_CTL,
	    phydata);

	DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr,
	    DWC_ETH_QOS_PHY_STS, &phydata);
	NMSGPR_ALERT( "PHY Sts Reg (%#x) = %#x\n", DWC_ETH_QOS_PHY_STS,
	    phydata);

	NMSGPR_ALERT(
	       "\n****************************************************\n");
}

/*!
* \brief API to adjust link parameters.
*
* \details This function will be called by PAL to inform the driver
* about various link parameters like duplex and speed. This function
* will configure the MAC based on link parameters.
*
* \param[in] dev - pointer to net_device structure
*
* \return void
*/
void DWC_ETH_QOS_adjust_link(struct work_struct *work)
{
	struct DWC_ETH_QOS_prv_data *pdata = container_of(work,
					struct DWC_ETH_QOS_prv_data, phy_dwork.work);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct phy_device *phydev = pdata->phydev;
	//unsigned long flags;
	int new_state = 0;

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return;
        }

	if (phydev == NULL)
		return;

	DBGPR_MDIO("-->DWC_ETH_QOS_adjust_link. address %d link %d\n", phydev->addr,
	      phydev->link);

	/* Ignore adjust_link interrupts if PCI link is down. PCI
	 * link down event will eventually stop the phy state machine */
	if (pdata->pdev->error_state != pci_channel_io_normal) {
		DBGPR_MDIO("%s: PCI link is down returning.\n");
		return;
	}

	//spin_lock_irqsave(&pdata->lock, flags);
	if (phydev->state != PHY_UP) {
		schedule_delayed_work(&pdata->phy_dwork, usecs_to_jiffies(1000000));
		return;
	}
	genphy_read_status(phydev);

	if (phydev->link) {
		/* Now we make sure that we can be in full duplex mode.
		 * If not, we operate in half-duplex mode */
		if (phydev->duplex != pdata->oldduplex) {
			new_state = 1;
			if (phydev->duplex)
				hw_if->set_full_duplex(pdata);
			else {
				hw_if->set_half_duplex(pdata);
#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT_HALFDUPLEX
				/* For Synopsys testing and debugging only */
				{
					UINT phydata;

					/* setting 'Assert CRS on transmit' */
					phydata = 0;
					DWC_ETH_QOS_mdio_read_direct(pdata, pdata->phyaddr,
						DWC_ETH_QOS_PHY_CTL, &phydata);
					phydata |= (1 << 11);
					DWC_ETH_QOS_mdio_write_direct(pdata, pdata->phyaddr,
						DWC_ETH_QOS_PHY_CTL, phydata);
				}
#endif
			}
			pdata->oldduplex = phydev->duplex;
		}

		/* FLOW ctrl operation */
		if (phydev->pause || phydev->asym_pause) {
			if (pdata->flow_ctrl != pdata->oldflow_ctrl)
				DWC_ETH_QOS_configure_flow_ctrl(pdata);
		}

		if (phydev->speed != pdata->speed) {
			new_state = 1;
			switch (phydev->speed) {
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
			pdata->speed = phydev->speed;
		}

		if (!pdata->oldlink) {
			new_state = 1;
			pdata->oldlink = 1;
			netif_carrier_on(pdata->dev);
		}
	} else if (pdata->oldlink) {
		new_state = 1;
		pdata->oldlink = 0;
		pdata->speed = 0;
		pdata->oldduplex = -1;
		netif_carrier_off(pdata->dev);
	}

	if (new_state)
		phy_print_status(phydev);

	if (new_state)
		DWC_ETH_QOS_reload_fqtss_cfg(pdata);

	/* At this stage, it could be need to setup the EEE or adjust some
	 * MAC related HW registers.
	 * */
	if (!pdata->eee_active)
		pdata->eee_enabled = DWC_ETH_QOS_eee_init(pdata);

	schedule_delayed_work(&pdata->phy_dwork, usecs_to_jiffies(1000000));

	//spin_unlock_irqrestore(&pdata->lock, flags);

	DBGPR_MDIO("<--DWC_ETH_QOS_adjust_link\n");
}

/*!
* \brief API to initialize PHY.
*
* \details This function will initializes the driver's PHY state and attaches
* the PHY to the MAC driver.
*
* \param[in] dev - pointer to net_device structure
*
* \return integer
*
* \retval 0 on success & negative number on failure.
*/

static int DWC_ETH_QOS_init_phy(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct phy_device *phydev = NULL;

	DBGPR_MDIO("-->DWC_ETH_QOS_init_phy\n");
	if (!pdata->enable_phy) {
		NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
		return -ENODEV;
	}

	phydev = kzalloc(sizeof(*phydev), GFP_KERNEL);
	if (NULL == phydev) {
		NMSGPR_ALERT("%s: PHY device create failed.\n", __func__);
		return PTR_ERR(phydev);
	}

	phydev->speed = SPEED_10;
	phydev->duplex = 0;
	phydev->pause = 0;
	phydev->asym_pause = 0;
	phydev->link = 0;
	phydev->interface = pdata->interface;
	phydev->autoneg = AUTONEG_ENABLE;
	phydev->addr = pdata->phyaddr;
	phydev->phy_id = pdata->phyaddr;
	phydev->state = PHY_READY;
	phydev->bus = pdata->mii;
	mutex_init(&phydev->lock);

	if ((pdata->interface == PHY_INTERFACE_MODE_GMII) ||
		(pdata->interface == PHY_INTERFACE_MODE_RGMII)) {
		phydev->supported |= PHY_GBIT_FEATURES;
#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT_HALFDUPLEX
		phydev->supported &= ~SUPPORTED_1000baseT_Full;
#endif
	} else if ((pdata->interface == PHY_INTERFACE_MODE_MII) ||
			   (pdata->interface == PHY_INTERFACE_MODE_RMII)) {
		phydev->supported |= PHY_BASIC_FEATURES;
	} else {
		NMSGPR_ALERT("%s : %d: Check me\n", __FUNCTION__,__LINE__);
	}

	phydev->advertising = phydev->supported;
	pdata->phydev = phydev;
	DBGPR_MDIO("<--DWC_ETH_QOS_init_phy\n");

	return 0;
}

/*!
* \brief API to register mdio.
*
* \details This function will allocate mdio bus and register it
* phy layer.
*
* \param[in] dev - pointer to net_device structure
*
* \return 0 on success and -ve on failure.
*/

int DWC_ETH_QOS_mdio_register(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct mii_bus *new_bus = NULL;
	int phyaddr = 0;
	unsigned short phy_detected = 0;
	int ret = Y_SUCCESS;
	int phyCheckLoop = 0;

	DBGPR_MDIO("-->DWC_ETH_QOS_mdio_register\n");
    if (!pdata->enable_phy) {
		NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
		return -ENODEV;
	}

	if (pdata->phyaddr == NTN_INVALID_PHY_ADDR) {
		/* find the phy ID or phy address which is connected to our MAC */
		do{
			for (phyaddr = 0; phyaddr < 32; phyaddr++) {
				int phy_reg_read_status, mii_status;

				phy_reg_read_status =
				    DWC_ETH_QOS_mdio_read_direct(pdata, phyaddr, MII_BMSR,
					&mii_status);
				if (phy_reg_read_status == 0) {
					if (mii_status != 0x0000 && mii_status != 0xffff) {
						NMSGPR_ALERT( "%s: Phy detected at"\
						    " ID/ADDR %d\n", DEV_NAME, phyaddr);
						phy_detected = 1;
						break;
					}
				} else if (phy_reg_read_status < 0) {
					NMSGPR_ALERT( "%s: Error reading the phy register"\
					    " MII_BMSR for phy ID/ADDR %d\n",
					    DEV_NAME, phyaddr);
				}
			}
			if(phy_detected) {
				break;
			}
			phyCheckLoop++;
		}while(phyCheckLoop < 100);

		if (!phy_detected) {
			NMSGPR_ALERT( "%s: No phy could be detected\n", DEV_NAME);
			return -ENOLINK;
		}
		pdata->phyaddr = phyaddr;
	}

	pdata->bus_id = pdata->mdio_bus_id;
	pdata->oldlink = 0;
	pdata->speed = 0;
	pdata->oldduplex = -1;

	DBGPHY_REGS(pdata);

	new_bus = mdiobus_alloc();
	if (new_bus == NULL) {
		NMSGPR_ALERT( "Unable to allocate mdio bus\n");
		return -ENOMEM;
	}

	new_bus->name = "dwc_phy";
	new_bus->read = DWC_ETH_QOS_mdio_read;
	new_bus->write = DWC_ETH_QOS_mdio_write;
	new_bus->reset = DWC_ETH_QOS_mdio_reset;
	snprintf(new_bus->id, MII_BUS_ID_SIZE, "%s-%x", new_bus->name,
		 pdata->bus_id);
	new_bus->priv = dev;
	new_bus->phy_mask = 0;
	new_bus->parent = &pdata->pdev->dev;
	mutex_init(&new_bus->mdio_lock);

	pdata->mii = new_bus;

	ret = DWC_ETH_QOS_init_phy(dev);
	if (unlikely(ret)) {
		NMSGPR_ALERT( "Cannot attach to PHY (error: %d)\n", ret);
		goto err_out_phy_connect;
	}

	DBGPR_MDIO("<--DWC_ETH_QOS_mdio_register\n");

	return ret;

 err_out_phy_connect:
	DWC_ETH_QOS_mdio_unregister(dev);
	return ret;
}

/*!
* \brief API to unregister mdio.
*
* \details This function will unregister mdio bus and free's the memory
* allocated to it.
*
* \param[in] dev - pointer to net_device structure
*
* \return void
*/

void DWC_ETH_QOS_mdio_unregister(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	DBGPR_MDIO("-->DWC_ETH_QOS_mdio_unregister\n");

        if (!pdata->enable_phy) {
                NMSGPR_ALERT("%s: PHY is not supported.\n", __func__);
                return;
        }

	if (pdata->phydev) {
		kfree(pdata->phydev);
		pdata->phydev = NULL;
	}

	if (pdata->mii) {
		pdata->mii->priv = NULL;
		mdiobus_free(pdata->mii);
		pdata->mii = NULL;
	}

	DBGPR_MDIO("<--DWC_ETH_QOS_mdio_unregister\n");
}

void DWC_ETH_QOS_enable_phy_wol(struct DWC_ETH_QOS_prv_data *pdata)
{
	uint16_t bmcr_val, ctrl1000_val, adv_val;
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	/* Disable 1000M mode */
	ctrl1000_val = phy_read(pdata->phydev, MII_CTRL1000);
	ctrl1000_val &= ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL);
	phy_write(pdata->phydev, MII_CTRL1000, ctrl1000_val);
	/* Disable 100M mode */
	adv_val = phy_read(pdata->phydev, MII_ADVERTISE);
	adv_val &= ~(ADVERTISE_100HALF | ADVERTISE_100FULL);
	phy_write(pdata->phydev, MII_ADVERTISE, adv_val);
	/* Restart aneg for 10M link */
	bmcr_val = phy_read(pdata->phydev, MII_BMCR);
	bmcr_val |= BMCR_ANRESTART;
	phy_write(pdata->phydev, MII_BMCR, bmcr_val);
	/* Setup MAC for 10M */
	hw_if->set_mii_speed_10(pdata);
	hw_if->ntn_set_tx_clk_2_5MHz(pdata);
}

void DWC_ETH_QOS_disable_phy_wol(struct DWC_ETH_QOS_prv_data *pdata)
{
	uint16_t bmcr_val, ctrl1000_val, adv_val;

	/* Enable 1000M mode */
	ctrl1000_val = phy_read(pdata->phydev, MII_CTRL1000);
	if (pdata->phydev->advertising & ADVERTISED_1000baseT_Half)
		ctrl1000_val |= ADVERTISE_1000HALF;
	if (pdata->phydev->advertising & ADVERTISED_1000baseT_Full)
		ctrl1000_val |= ADVERTISE_1000FULL;
	phy_write(pdata->phydev, MII_CTRL1000, ctrl1000_val);
	/* Enable 100M mode */
	adv_val = phy_read(pdata->phydev, MII_ADVERTISE);
	if (pdata->phydev->advertising & ADVERTISED_100baseT_Half)
		adv_val |= ADVERTISE_100HALF;
	if (pdata->phydev->advertising & ADVERTISED_100baseT_Full)
		adv_val |= ADVERTISE_100FULL;
	phy_write(pdata->phydev, MII_ADVERTISE, adv_val);
	/* Restart aneg */
	bmcr_val = phy_read(pdata->phydev, MII_BMCR);
	bmcr_val |= BMCR_ANRESTART;
	phy_write(pdata->phydev, MII_BMCR, bmcr_val);
}
