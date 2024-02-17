inherit autotools-brokensep pkgconfig update-rc.d
PR = "r7"

DESCRIPTION = "Recovery bootloader"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
HOMEPAGE = "https://www.codeaurora.org/gitweb/quic/la?p=platform/bootable/recovery.git"
DEPENDS = "glib-2.0 libmincrypt-native system-core oem-recovery libsparse bison-native bzip2"
RDEPENDS_${PN} = "zlib"
FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://OTA/recovery/"
SRC_URI += "file://poky/meta-qti-bsp/recipes-bsp/recovery/files/recovery.service"

S = "${WORKDIR}/OTA/recovery/"

EXTRA_OECONF = "--with-glib --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
                --with-core-headers=${STAGING_INCDIR}"
CFLAGS += "-lsparse -llog"

SYSTEMD_SUPPORT = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'systemd', '', d)}"

SELINUX_SUPPORT = "${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'selinux', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('DISTRO_FEATURES', 'nand-boot', '--with-nand_boot=true', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('DISTRO_FEATURES', 'nad-prod', '--with-nad_prod=true', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('DISTRO_FEATURES', 'nad-fde', '--with-nad_fde=true', '', d)}"
PARALLEL_MAKE = ""
INITSCRIPT_NAME = "recovery"
INITSCRIPT_PARAMS = "start 99 5 . stop 80 0 1 6 ."

FILES_${PN}  = "/recovery/etc/"
FILES_${PN} += "/cache"
FILES_${PN} += "/data"
FILES_${PN} += "/recovery/res"
FILES_${PN} += "/system"
#FILES_${PN} += "/tmp"
FILES_${PN} += "/recovery/res"
FILES_${PN} += "/data"
FILES_${PN} += "/recovery/lib"

PACKAGES += "${PN}-bin ${PN}-lib"
FILES_${PN}-bin = "/recovery/usr/bin"
FILES_${PN}-lib = "${libdir}"
RDEPENDS_${PN}-bin = "recovery-lib"
do_install_append() {
        install -d ${D}/cache/
#       install -d ${D}/tmp/
        install -d ${D}/res/
        install -d ${D}/data/
        install -d ${D}/system/
        install -m 0755 ${S}/start_recovery -D ${D}${sysconfdir}/init.d/recovery

        if [ "${SYSTEMD_SUPPORT}" == "systemd" ]; then
         install -d ${D}${systemd_unitdir}/system/
         install -m 0644 ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/recovery/files/recovery.service -D ${D}${systemd_unitdir}/system/recovery.service
         install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
         # enable the service for multi-user.target
         ln -sf ${systemd_unitdir}/system/recovery.service \
                                  ${D}${systemd_unitdir}/system/multi-user.target.wants/recovery.service
        fi
        if [ "${SELINUX_SUPPORT}" == "selinux" ]; then
         install -m  0755 ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/fstab -D ${D}${sysconfdir}/fstab
         install -m 0755 ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/fstab -D ${D}/res/recovery_volume_config
        else
         install -m 0755 ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/fstab_noselinux -D ${D}${sysconfdir}/fstab
         install -m 0755 ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/fstab_noselinux -D ${D}/res/recovery_volume_config
        fi

      #Recovery-ab and recovery brings same set of libs and bins so these changes.
      # Repackaging to recovery rootfs will be taken care in machine-recovery-image.bb
        install -d ${D}/recovery
        install -d ${D}/recovery/lib
        cp -rp  ${D}/lib/* ${D}/recovery/lib
        install -d ${D}/recovery/res
        cp -rp  ${D}/res/* ${D}/recovery/res
        install -d ${D}/recovery/usr
        cp -rp ${D}/usr/* ${D}/recovery/usr
        install -d ${D}/recovery/etc
        cp -rp ${D}/etc/* ${D}/recovery/etc/
        rm -rf ${D}/etc ${D}/res ${D}/lib ${D}/usr/bin ${D}/usr/etc
}
