inherit autotools pkgconfig

DESCRIPTION = "Bluetooth application layer"
LICENSE = "Apache-2.0"
HOMEPAGE = "https://www.codeaurora.org/"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://qcom-opensource/bt/bt-app/"

S = "${WORKDIR}/qcom-opensource/bt/bt-app/"

def get_depends():
    if "$(BASEMACHINE)" == "mdm9607":
        return  "btvendorhal gen-gatt glib-2.0 btobex"
    else:
        return   "btvendorhal gen-gatt glib-2.0 btobex audiohal"

DEPENDS  += "${@get_depends()}"

CPPFLAGS_append = " -DUSE_ANDROID_LOGGING -DUSE_BT_OBEX -DUSE_LIBHW_AOSP"
CFLAGS_append = " -DUSE_ANDROID_LOGGING "
LDFLAGS_append = " -llog "

EXTRA_OECONF = " \
                --with-common-includes="${WORKSPACE}/vendor/qcom/opensource/bluetooth/hal/include/" \
                --with-glib \
                --with-lib-path=${STAGING_LIBDIR} \
                --with-btobex \
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
