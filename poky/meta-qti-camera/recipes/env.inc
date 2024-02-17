DEPENDS += "system-core-headers"
DEPENDS += "llvm-arm-toolchain-native"
DEPENDS += "glib-2.0"
DEPENDS += "libutils"
DEPENDS += "libcutils"
DEPENDS += "libsync"

CAMX_FLAGS               = "-target ${TARGET_SYS} "
CAMX_FLAGS               += "-I ${STAGING_INCDIR}/c++ "
OECMAKE_C_FLAGS_append   = "${CAMX_FLAGS}"
OECMAKE_CXX_FLAGS_append = "${CAMX_FLAGS}"


def config_target_sys(d):
    if d.getVar('PRODUCT', True) == 'ubuntu':
        return "aarch64-linux-gnu"
    else:
        return "${TARGET_ARCH}${TARGET_VENDOR}${@['-' + d.getVar('TARGET_OS'), ''][d.getVar('TARGET_OS') == ('' or 'custom')]}"

TARGET_SYS = "${@config_target_sys(d)}"


def get_platform(d):
    if d.getVar('MACHINE', True) == 'qrb5165-rb5':
        return "kona"
    else:
        return ""

EXTRA_OECMAKE = "\
        -DCAMXDEBUG:STRING=True \
        -DPLATFORM:STRING=linux \
        -DCPU:STRING=64 \
        -DCMAKE_CROSSCOMPILING:BOOL=True \
        -DCMAKE_C_COMPILER:STRING=${OECMAKE_C_COMPILER} \
        -DTARGET_PLATFORM:STRING=${@get_platform(d)} \
"
