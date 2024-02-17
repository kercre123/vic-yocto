# Kernel module instructions go here.
#
obj-m := DWC_ETH_QOS.o
DWC_ETH_QOS-y := DWC_ETH_QOS_dev.o DWC_ETH_QOS_drv.o DWC_ETH_QOS_desc.o DWC_ETH_QOS_ethtool.o DWC_ETH_QOS_pci.o DWC_ETH_QOS_mdio.o DWC_ETH_QOS_eee.o DWC_ETH_QOS_plt_init.o DWC_ETH_QOS_ptp.o

ifeq ($(CONFIG_IPA_OFFLOAD), 1)
EXTRA_CFLAGS+=-DDWC_ETH_QOS_ENABLE_IPA
DWC_ETH_QOS-y += DWC_ETH_QOS_ipa.o
endif
