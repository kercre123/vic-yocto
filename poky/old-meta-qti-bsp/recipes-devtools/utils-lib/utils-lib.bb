inherit autotools

DESCRIPTION = "util library"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PV = "1.0.0"
PR = "r2"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://base/libs/utils \
           file://string_fix_patch.txt;patch=1 \
 "

S = "${WORKDIR}/base/libs/utils"

DEPENDS += "system-core"

ARM_INSTRUCTION_SET = "arm"

EXTRA_OECONF_append = " --with-additional-include-directives="${WORKSPACE}/base/include""

CPPFLAGS += "-I${STAGING_INCDIR}/c++"
CPPFLAGS += "-I${STAGING_INCDIR}/c++/${TARGET_SYS}"

FILES_${PN} += "/usr/lib/*"

# package contains symlinks that trip up insane
INSANE_SKIP_${PN} = "dev-so"

#do_configure_prepend() {
# Apply patch to string class
#pushd ${S}/..
#patch -p0 < string_fix_patch.txt
#popd
#}
