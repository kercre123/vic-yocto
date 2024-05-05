export PTPSUPPORT CONFIG_PTPSUPPORT_OBJ NTN_HSIC_CONFIG_PTP

#default values
PTPSUPPORT=y #ptp is enabled

ifeq ($(RELEASE_PACKAGE),1)
EXTRA_CFLAGS+=-DRELEASE_PACKAGE
endif
LBITS ?= $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
CCFLAGS += -m64
EXTRA_CFLAGS+=-DSYSTEM_IS_64
else
CCFLAGS += -m32
endif

ifeq "$(PTPSUPPORT)" "y"
CONFIG_PTPSUPPORT_OBJ=y
NTN_HSIC_CONFIG_PTP=-DPTPSUPPORT
EXTRA_CFLAGS+=-DCONFIG_PTPSUPPORT_OBJ
else
CONFIG_PTPSUPPORT_OBJ=y
endif


obj-m := NTN_HSIC.o

NTN_HSIC-y +=	ntn_gmac.o \
		ntn_reg_rw.o \
		ntn_usb_class_main.o

NTN_HSIC-$(CONFIG_PTPSUPPORT_OBJ) += ntn_ptp.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
