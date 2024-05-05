Neutrino HSIC Driver Release Version: 2.00.00
===============================================================================

Note: This release is not fully tested yet.

Introduction:
=============
The folder includes NTN drivers and related documents.

Top level directory structure:

  ntn_drv 
	|-ntn_usb_class_main.c 
	|-ntn_usbnet.c
	|-ntn_gmac.c 
	|-ntn_ptp.c 
	|-ntn_reg_rw.c 
	|-ntn_common.h
	|-ntn_gmac.h
	|-ntn_ptp.c
	|-ntn_reg_rw.h
	|-DWC_ETH_QOS_yapphdr.h
    |-Makefile
    |-readme 
  
Drivers:
========
Note:
1. Root access is required to build & load the driver.

Compilation:
============
1. cd Source_code/driver/hsic/
   # make clean
   # make
 
Insert steps:
=============
1. cd Source_Code/driver/hsic/
2. modprobe usbnet
2. insmod ntn_usb_class_drv.ko

Remove steps:
==============
1. rmmod ntn_usb_class_drv.ko
2. rmmod usbnet


Note: ntn_usbnet.c is based on usbnet.c in the kernel. For this release, the kernel version is 3.18.44.
If your kernel version is different with 3.18.44, then you need modify ntn_usbnet.c based on the usbnet.c
in your kernel (driver/net/usb/usbnet.c).
