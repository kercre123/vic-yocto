inherit cmake pythonnative

SUMMARY = "Nanopb - Protocol Buffers for Embedded Systems"
DESCRIPTION = "Nanopb is a small code-size Protocol Buffers implementation \
in ansi C. It is especially suitable for use in microcontrollers, but fits \
any memory restricted system."
HOMEPAGE = "https://github.com/nanopb/nanopb"
SECTION = "console/tools"

LICENSE = "Zlib"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=9db4b73a55a3994384112efcdb37c01f"

DEPENDS = "protobuf-native nanopb-native"
DEPENDS_append_class-native = " python-protobuf-native"

SRCREV = "cc74b9f200176edc5524aa00ba45fa90a5e87d27"
PV = "0.3.8_git_${SRCREV}"

SRC_URI = "git://source.codeaurora.org/quic/le/nanopb.git;protocol=git;branch=nanopb/master"
SRC_URI += "file://0001-bitbake-using-cmake.patch"

S = "${WORKDIR}/git"

# need to export these variables for python-config to work
FILES_${PN} += "/usr/include/*"
FILES_${PN} += "/usr/lib/*"
FILES_${PN} += "/usr/lib64/*"

export BUILD_SYS
export HOST_SYS
export STAGING_INCDIR
export STAGING_LIBDIR

EXTRA_OECMAKE_append_class-native = "-Dnanopb_BUILD_GENERATOR=ON -Dnanopb_BUILD_RUNTIME=OFF -Dnanopb_MSVC_STATIC_RUNTIME=OFF"
INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN}-dev += "dev-elf"

BBCLASSEXTEND = "native nativesdk"
