# Pull in the default security flags
require ${COREBASE}/meta/conf/distro/include/security_flags.inc

# Modify the primary security flag definitions
SECURITY_CFLAGS = "-fstack-protector-strong ${lcl_maybe_fortify}"
SECURITY_LDFLAGS = "-Wl,-z,relro,-z,now,-z,noexecstack"

SECURITY_PIE_CFLAGS = "-fstack-protector-strong -pie -fPIE -Wa,--noexecstack ${lcl_maybe_fortify}"
SECURITY_PIC_CFLAGS = "-fstack-protector-strong -fstack-protector-strong -fPIC -Wa,--noexecstack ${lcl_maybe_fortify}"

SECURITY_PIE_NO_FORTIFY_CFLAGS = "-fstack-protector-strong -pie -fPIE -Wa,--noexecstack"

# GPS OSS packages to use -fPIC in security flags instead of -fpie
SECURITY_CFLAGS_pn-loc-hal = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-pla = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-stub = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-gps-utils = "${SECURITY_PIC_CFLAGS}"
SECURITY_CFLAGS_pn-loc-net-iface = "${SECURITY_PIC_CFLAGS}"

SECURITY_CFLAGS_pn-system-media = "${SECURITY_PIE_NO_FORTIFY_CFLAGS}"
SECURITY_CFLAGS_pn-audiohal = "${SECURITY_PIE_NO_FORTIFY_CFLAGS}"

SECURITY_CFLAGS_pn-connman = "${SECURITY_PIE_CFLAGS}"
SECURITY_CFLAGS_pn-chrony = "${SECURITY_PIE_CFLAGS}"
SECURITY_CFLAGS_pn-system-core = "${SECURITY_PIE_CFLAGS}"
