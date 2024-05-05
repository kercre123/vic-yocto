FILESEXTRAPATHS_prepend := "${WORKSPACE}/display/:"

SRC_DIR = "${WORKSPACE}/display/weston/"
SRC_URI = "file://${@d.getVar('SRC_DIR', True).replace('${WORKSPACE}/display/', '')}"
REPO_SRC_URI = "file://${@d.getVar('SRC_DIR',True).replace('${WORKSPACE}/display/', '')}"
S = "${WORKDIR}/weston"

FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

DEPENDS_apq8098 = "libxkbcommon gdk-pixbuf pixman cairo glib-2.0 jpeg"
DEPENDS_apq8098 += "wayland libinput virtual/egl pango"
DEPENDS_apq8098 += "display-noship-linux"

EXTRA_OECONF_append = "\
	--enable-drm-compositor \
	"
CFLAGS += "-idirafter ${STAGING_KERNEL_DIR}/include/"
CPPFLAGS += "-I${STAGING_INCDIR}/sdm"
CPPFLAGS += "-I${STAGING_INCDIR}/sdm/core"

#
# Compositor choices
#
# Weston on KMS
PACKAGECONFIG[kms] = "--enable-drm-compositor"
# Weston on Wayland (nested Weston)
PACKAGECONFIG[wayland] = "--enable-wayland-compositor,--disable-wayland-compositor,gbm"
FILES_${PN} += "${bindir}/weston-fullscreen ${bindir}/weston-flower ${bindir}/weston-simple-egl"
INSANE_SKIP_weston += "dev-deps"
