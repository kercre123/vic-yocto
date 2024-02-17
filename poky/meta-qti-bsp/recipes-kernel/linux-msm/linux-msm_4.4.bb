inherit kernel

DESCRIPTION = "CAF Linux Kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

COMPATIBLE_MACHINE = "(apq8098)"

# Default image type is zImage, change it in machine conf if needed.

python __anonymous () {
  # Override KERNEL_IMAGETYPE_FOR_MAKE variable, which is internal
  # to kernel.bbclass. We override the variable as msm kernel can't
  # support alternate image builds
  if d.getVar("KERNEL_IMAGETYPE", True):
      d.setVar("KERNEL_IMAGETYPE_FOR_MAKE", "")
}

KERNEL_IMAGEDEST = "boot"

DEPENDS_append_aarch64 = " libgcc"
KERNEL_CC_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"
KERNEL_LD_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"

KERNEL_PRIORITY           = "9001"
# Add V=1 to KERNEL_EXTRA_ARGS for verbose
KERNEL_EXTRA_ARGS        += "O=${B}"

PACKAGE_ARCH = "${MACHINE_ARCH}"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI   =  "file://kernel"

SRC_DIR   =  "${WORKSPACE}/kernel/msm-4.4"
S         =  "${WORKDIR}/kernel/msm-4.4"
PR = "r6"

DEPENDS += "mkbootimg-native dtc-native"
DEPENDS += "bouncycastle"
DEPENDS += "openssl-native"

PACKAGES = "kernel kernel-base kernel-vmlinux kernel-dev kernel-modules"
RDEPENDS_kernel-base = ""

# Put the zImage in the kernel-dev pkg
FILES_kernel-dev += "/${KERNEL_IMAGEDEST}/${KERNEL_IMAGETYPE}-${KERNEL_VERSION}"

do_configure () {
    oe_runmake_call -C ${S} ARCH=${ARCH} ${KERNEL_EXTRA_ARGS} ${KERNEL_CONFIG}
}
do_compile () {
    unset LDFLAGS
    oe_runmake CC="${KERNEL_CC}" LD="${KERNEL_LD}" ${KERNEL_EXTRA_ARGS} $use_alternate_initrd
}

do_shared_workdir_append () {
        cp Makefile $kerneldir/
        cp -fR usr $kerneldir/

        cp include/config/auto.conf $kerneldir/include/config/auto.conf

        if [ -d arch/${ARCH}/include ]; then
                mkdir -p $kerneldir/arch/${ARCH}/include/
                cp -fR arch/${ARCH}/include/* $kerneldir/arch/${ARCH}/include/
        fi

        if [ -d arch/${ARCH}/boot ]; then
                mkdir -p $kerneldir/arch/${ARCH}/boot/
                cp -fR arch/${ARCH}/boot/* $kerneldir/arch/${ARCH}/boot/
        fi

        if [ -d scripts ]; then
            for i in \
                scripts/basic/bin2c \
                scripts/basic/fixdep \
                scripts/conmakehash \
                scripts/dtc/dtc \
                scripts/genksyms/genksyms \
                scripts/kallsyms \
                scripts/kconfig/conf \
                scripts/mod/mk_elfconfig \
                scripts/mod/modpost \
                scripts/recordmcount \
                scripts/sign-file \
                scripts/sortextable;
            do
                if [ -e $i ]; then
                    mkdir -p $kerneldir/`dirname $i`
                    cp $i $kerneldir/$i
                fi
            done
        fi

        cp ${STAGING_KERNEL_DIR}/scripts/gen_initramfs_list.sh $kerneldir/scripts/

        # Copy vmlinux and zImage into deplydir for boot.img creation
        install -m 0644 ${KERNEL_OUTPUT_DIR}/${KERNEL_IMAGETYPE} ${DEPLOY_DIR_IMAGE}/${KERNEL_IMAGETYPE}
        install -m 0644 vmlinux ${DEPLOY_DIR_IMAGE}

        # Generate kernel headers
        oe_runmake_call -C ${STAGING_KERNEL_DIR} ARCH=${ARCH} CC="${KERNEL_CC}" LD="${KERNEL_LD}" headers_install O=${STAGING_KERNEL_BUILDDIR}
}

do_shared_workdir[dirs] = "${DEPLOY_DIR_IMAGE}"

#-----------------------------------------
#  create siged bootimage
#-----------------------------------------
boot_signing () {
    B_BASE=`echo ${WORKDIR} | grep -oP "/.+/tmp-glibc/"`
    BC_BUILD="${B_BASE}/work/aarch64-oe-linux/bouncycastle/git-r0"

    /usr/bin/java -Xmx512M -jar ${BC_BUILD}/build/libs/BootSignature.jar \
        /boot                                                     \
        ${DEPLOY_DIR_IMAGE}/boot.img                   \
	${BC_BUILD}/security/target/product/security/verity.pk8 \
	${BC_BUILD}/security/target/product/security/verity.x509.pem \
        ${DEPLOY_DIR_IMAGE}/${BOOTIMAGE_TARGET}

    rm -f ${DEPLOY_DIR_IMAGE}/boot.img
}

do_module_signing() {
    for i in $(find ${PKGDEST} -name "*.ko"); do
    ${STAGING_KERNEL_BUILDDIR}/scripts/sign-file sha512 ${STAGING_KERNEL_BUILDDIR}/certs/signing_key.pem ${STAGING_KERNEL_BUILDDIR}/certs/signing_key.x509 ${i}
    done
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/drm/* ${STAGING_INCDIR}/drm/
    mkdir -p ${STAGING_INCDIR}/media
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/media/* ${STAGING_INCDIR}/media/
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/linux/ion.h ${STAGING_INCDIR}/linux/
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/linux/msm_ion.h ${STAGING_INCDIR}/linux/
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/linux/msm_mdp.h ${STAGING_INCDIR}/linux/
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/linux/msm_mdp_ext.h ${STAGING_INCDIR}/linux/
    cp -fR ${STAGING_KERNEL_BUILDDIR}/usr/include/linux/mdss_rotator.h ${STAGING_INCDIR}/linux/
}

addtask do_module_signing after do_package before do_package_write_ipk
