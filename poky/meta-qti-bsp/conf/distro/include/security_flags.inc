# Pull in the default security flags
require ${COREBASE}/meta/conf/distro/include/security_flags.inc

# Modify the primary security flag definitions of yocto
# _FORTIFY_SOURCE requires -O1 or higher, otherwise results in a compiler warning.
SECURITY_PIE_CFLAGS = "-fstack-protector-strong -pie -fPIE -D_FORTIFY_SOURCE=2 -Wa,--noexecstack ${SECURITY_STRINGFORMAT}"

# For usage by packages which needs position independent code that can be linked into shared libraires.
SECURITY_PIC_CFLAGS = "-fstack-protector-strong -fPIC -D_FORTIFY_SOURCE=2 -Wa,--noexecstack ${SECURITY_STRINGFORMAT}"

SECURITY_CFLAGS = "${SECURITY_PIE_CFLAGS}"
SECURITY_LDFLAGS = "-Wl,-z,relro,-z,now,-z,noexecstack"

# edk2
SECURITY_CFLAGS_pn-edk2 = "-pie -fpie -Wa,--noexecstack"

#
### PACKAGES to use -fPIC in security flags instead of -fPIE
#
# GPS OSS packages
SECURITY_CFLAGS_pn-gps-utils = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-hal = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-core = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-api-v02 = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-ds = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-net-iface = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-location-api = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-base-util = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-mq-client = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-izat-core = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-lowi-client = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-lbs-core = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-drplugin-client = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-drplugin = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-ulp2 = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-location-flp = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-location-service = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-glue = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-location-batching = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-location-geofence = "${SECURITY_PIC_CFLAGS}"

# General purpose OSS packages
SECURITY_CFLAGS_pn-gtest = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-go-runtime = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-binder  = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-libcutils  = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-liblog  = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-av-frameworks  = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-libbase = "${SECURITY_PIC_CFLAGS}"
# libutils needs -fPIC
SECURITY_CFLAGS_pn_libutils = "${SECURITY_PIC_CFLAGS}"

# BT OSS packages
SECURITY_CFLAGS_pn-event-observer = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-data-items = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-bt-property = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-bt-app = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-libbt-vendor = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-fluoride = "${SECURITY_PIC_CFLAGS}"

# MM packages
SECURITY_CFLAGS_pn-display-hal = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-media = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-lib32-media = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-qmmf-sdk = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-lib32-qmmf-sdk = "${SECURITY_PIC_CFLAGS}"

#
### PACKAGES which can't compile with _FORTIFY_SOURCE
#
SECURITY_CFLAGS_remove_pn-ebtables = "-D_FORTIFY_SOURCE=2"

#
### PACKAGES which can't compile with security format strings check
#
SECURITY_CFLAGS_remove_pn-audiohal = "${SECURITY_STRINGFORMAT}"
SECURITY_CFLAGS_remove_pn-setools = "${SECURITY_STRINGFORMAT}"
