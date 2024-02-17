#
# Yocto upstream doesn't allow compiling kernel for multilib.
# This class helps to reset environment variables required for kernel
# compilation (including modules) for an arch different from default
# tune based on multilib settings. Kernel arch is identified based on
# TARGET_KERNEL_ARCH variable. This class assumes MULTILIB environment
# provides required toolchain packages.
#

inherit kernel-arch

DEFAULTTUNE = "${TARGET_KERNEL_ARCH}"
TUNE_ARCH = "${@bb.utils.contains('TARGET_KERNEL_ARCH', 'aarch64', '${TUNE_ARCH_64}', '${TUNE_ARCH_32}' ,d)}"
TARGET_PREFIX = "${MULTILIB_VARIANTS}-${TARGET_SYS}-"
HOST_PREFIX = "${TARGET_SYS}-"

DEPENDS += "${MULTILIB_VARIANTS}-binutils-cross-${TARGET_ARCH}"

export CROSS_COMPILE = "${TARGET_SYS}-"

export ARCH = "${@map_kernel_arch(d.getVar('TARGET_KERNEL_ARCH'), d)}"
