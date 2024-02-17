inherit autotools pkgconfig

DESCRIPTION = "Bluetooth application layer"
LICENSE = "BSD-3-Clause"
HOMEPAGE = "https://www.codeaurora.org/"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=550794465ba0ec5312d6919e203a55f9"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://qcom-opensource/bt/bt-app/"

S = "${WORKDIR}/qcom-opensource/bt/bt-app/"

DEPENDS += "btvendorhal gen-gatt glib-2.0 btobex audiohal"
DEPENDS += "gstreamer1.0 gstreamer1.0-plugins-base orc qsthw-api gst-plugins"
DEPENDS_remove_mdm9607 = "audiohal"

CPPFLAGS_append = " -DUSE_ANDROID_LOGGING -DUSE_BT_OBEX -DUSE_LIBHW_AOSP"
CFLAGS_append = " -DUSE_ANDROID_LOGGING "
LDFLAGS_append = " -llog "

EXTRA_OECONF = " \
            --with-common-includes="${WORKSPACE}/vendor/qcom/opensource/bluetooth/hal/include/" \
            --with-glib \
            --with-lib-path=${STAGING_LIBDIR} \
            --with-btobex \
            --with-gstreamer \
               "
EXTRA_OECONF += "--enable-target=${BASEMACHINE}"

FILES_${PN} += "${userfsdatadir}/misc/bluetooth/*"

do_install_append() {
        install -d ${D}${userfsdatadir}/misc/bluetooth/

        if [ -f ${S}conf/bt_app.conf ]; then
           install -m 0660 ${S}conf/bt_app.conf ${D}${userfsdatadir}/misc/bluetooth/
        fi

        if [ -f ${S}conf/ext_to_mimetype.conf ]; then
           install -m 0660 ${S}conf/ext_to_mimetype.conf ${D}${userfsdatadir}/misc/bluetooth/
        fi
}
