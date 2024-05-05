require libsystemdq.inc

PE = "1"

DEPENDS = "intltool-native gperf-native libcap util-linux"

SECTION = "base/shell"
PACKAGE = "libsystemdq"
inherit useradd pkgconfig meson perlnative update-alternatives qemu systemd gettext bash-completion manpages distro_features_check

# As this recipe builds udev, respect systemd being in DISTRO_FEATURES so
# that we don't build both udev and systemd in world builds.
REQUIRED_DISTRO_FEATURES = "systemd"

SRC_URI += "file://touchscreen.rules \
           file://00-create-volatile.conf \
           file://init \
           file://0001-binfmt-Don-t-install-dependency-links-at-install-tim.patch \
           file://0002-use-lnr-wrapper-instead-of-looking-for-relative-opti.patch \
           file://0003-implment-systemd-sysv-install-for-OE.patch \
           file://0004-rules-whitelist-hd-devices.patch \
           file://0005-Make-root-s-home-directory-configurable.patch \
           file://0006-remove-nobody-user-group-checking.patch \
           file://0007-rules-watch-metadata-changes-in-ide-devices.patch \
           file://0008-Do-not-enable-nss-tests-if-nss-systemd-is-not-enable.patch \
           file://0009-nss-mymachines-Build-conditionally-when-ENABLE_MYHOS.patch \
           file://0001-login-use-parse_uid-when-unmounting-user-runtime-dir.patch \
           file://0001-sd-bus-make-BUS_DEFAULT_TIMEOUT-configurable.patch \
           file://0022-build-sys-Detect-whether-struct-statx-is-defined-in-.patch \
           file://0023-resolvconf-fixes-for-the-compatibility-interface.patch \
           file://0001-core-when-deserializing-state-always-use-read_line-L.patch \
           file://0001-chown-recursive-let-s-rework-the-recursive-logic-to-.patch \
           file://0001-dhcp6-make-sure-we-have-enough-space-for-the-DHCP6-o.patch \
           "

SRC_URI += "file://remove-udev-references-from-meson-build.patch \
          file://remove-journal-from-systemctl.patch \
          file://src-basic-meson-build.patch \
          file://src-shared-meson-build.patch \
          file://src-libsystemd-meson-build.patch \
          file://add-dependencies-to-libsystemdq.patch \
          file://remove-libsystemd-network-from-meson-build.patch \
          file://remove-udev-and-libudev.patch \
          file://meson-build-remove-git-ls-files-check.patch \
          file://update-libsystemd-pc-in.patch \
          "

# Helper variables to clarify locations.  This mirrors the logic in systemd's
# build system.
rootprefix ?= "${root_prefix}"
rootlibdir ?= "${base_libdir}"
rootlibexecdir = "${rootprefix}/lib"

# This links udev statically with systemd helper library.
# Otherwise udev package would depend on systemd package (which has the needed shared library),
# and always pull it into images.
#EXTRA_OEMESON += "-Dlink-udev-shared=true"

EXTRA_OEMESON += "-Dnobody-user=nobody \
                  -Dnobody-group=nobody \
                  -Droothomedir=${ROOT_HOME} \
                  -Drootlibdir=${rootlibdir} \
                  -Drootprefix=${rootprefix} \
                  -Dsysvrcnd-path=${sysconfdir} \
                  -Dlink-systemctl-shared=true \
                  "

# Hardcode target binary paths to avoid using paths from sysroot
EXTRA_OEMESON += "-Dkexec-path=${sbindir}/kexec \
                  -Dkill-path=${base_bindir}/kill \
                  -Dkmod-path=${base_bindir}/kmod \
                  -Dmount-path=${base_bindir}/mount \
                  -Dquotacheck-path=${sbindir}/quotacheck \
                  -Dquotaon-path=${sbindir}/quotaon \
                  -Dsulogin-path=${base_sbindir}/sulogin \
                  -Dumount-path=${base_bindir}/umount \
                "

EXTRA_OEMESON += "-Denvironment-d=false \
                  -Dnss-systemd=false -Dmyhostname=false \
                  -Dhibernate=false \
                  -Dblkid=false \
                  -Dresolve=false \
                  -Dlogind=false \
                  -Dpam=false \
                  -Defi=false \
                  -Dgnu-efi=false \
                  -Dportabled=false \
                  -Dbacklight=false \
                  -Drfkill=false \
                  -Dlibcryptsetup=false \
                  -Dsysv-compat=false \
                  -Dhostnamed=false \
                  -Dlocaled=false \
                  -Dtimedated=false \
                  -Dtimesyncd=false \
                  -Dmachined=false \
                  -Dimportd=false \
                  -Dremote=false \
                  -Dlibcurl=false \
                  -Dmicrohttpd=false \
                  -Dcoredump=false \
                  -Dvconsole=false \
                  -Dbinfmt=false \
                  -Drandomseed=false \
                  -Dfirstboot=false \
                  -Dsysusers=false \
                  -Dtmpfiles=false \
                  -Dhwdb=false \
                  -Dquotacheck=false \
                  -Dkmod=false \
                  -Dnetworkd=false \
                  -Didn=false \
                  -Dtpm=false \
                  -Dadm-group=false \
                  -Dwheel-group=false \
                  -Dpolkit=false \
                  -Dman=false "
                  

do_install() {
   meson_do_install
   install -d ${D}/${base_sbindir}
   rm -rf ${D}/usr/share/zsh/
   rm -rf ${D}/usr/lib/systemd/
   rm -rf ${D}/usr/bin/
   rm -rf ${D}/bin/
   rm -rf ${D}/lib/systemd/
   rm -rf ${D}/var/
   rm -rf ${D}/lib/systemd/system-generators/
   rm -rf ${D}/sbin/
   rm -rf ${D}/usr/share/
   mv ${D}/usr/include/systemd ${D}/usr/include/systemdq
}


python populate_packages_prepend (){
    systemdlibdir = d.getVar("rootlibdir")
    do_split_packages(d, systemdlibdir, '^lib(.*)\.so\.*', 'lib%s', 'Systemd %s library', extra_depends='', allow_links=True)
}
PACKAGES_DYNAMIC += "^lib(systemd).*"

FILES_${PN} = " ${exec_prefix}/lib/systemd \
               "

FILES_${PN}-dev += "${base_libdir}/security/*.la ${datadir}/dbus-1/interfaces/ ${sysconfdir}/rpm/macros.systemd"

RDEPENDS_${PN} += "kmod dbus util-linux-mount  util-linux-agetty util-linux-fsck"
RDEPENDS_${PN} += "volatile-binds update-rc.d systemd-conf"


CFLAGS_append = " -fPIC"

INSANE_SKIP_${PN} += "dev-so libdir"
INSANE_SKIP_${PN}-dbg += "libdir"
INSANE_SKIP_${PN}-doc += " libdir"

python __anonymous() {
    if not bb.utils.contains('DISTRO_FEATURES', 'sysvinit', True, False, d):
        d.setVar("INHIBIT_UPDATERCD_BBCLASS", "1")
}

PACKAGE_WRITE_DEPS += "qemu-native"
