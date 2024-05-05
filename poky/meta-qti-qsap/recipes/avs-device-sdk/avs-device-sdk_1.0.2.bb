DESCRIPTION = "Amazon Alexa Voice Service"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=d92e60ee98664c54f68aa515a6169708 \
                    file://ThirdParty/rapidjson/rapidjson-1.1.0/license.txt;md5=ba04aa8f65de1396a7e59d1d746c2125 \
                    file://ThirdParty/MultipartParser/MultipartParser/LICENSE;md5=828ee02e6a6bbacd65f01e23be9459b7 \
                    file://ThirdParty/googletest-release-1.8.0/googlemock/LICENSE;md5=cbbd27594afd089daa160d3a16dd515a \
                    file://ThirdParty/googletest-release-1.8.0/googlemock/scripts/generator/LICENSE;md5=2c0b90db7465231447cf2dd2e8163333 \
                    file://ThirdParty/googletest-release-1.8.0/googletest/LICENSE;md5=cbbd27594afd089daa160d3a16dd515a"

SRC_URI = "https://github.com/alexa/avs-device-sdk/archive/v1.0.2.tar.gz \
           file://0001-Adding-setVolume-and-setMute-in-MediaPlayerInterface.patch \
           file://0001-Adding-Speaker-Capability-Agent.patch \
           file://0001-Add-EXPECTING_SPEECH-state.patch"

SRC_URI[md5sum] = "5c80ec6c7676ef1839ded4ce403d479e"
SRC_URI[sha256sum] = "473f6f4b6252f99530168ca3ed52bfed4ea8c2fb1ac4cf3d781deaed26f4dbc8"

inherit cmake

FILESEXTRAPATHS_prepend := "${THISDIR}/qti-patches:"

do_install_append () {
    install -d ${D}/usr/include
    cp -r ${S}/ThirdParty/rapidjson/rapidjson-1.1.0/include/* ${D}/usr/include/.
}

DEPENDS += "gcc (>= 4.8) cmake (>= 3.0) openssl (>= 1.0.2) curl (>= 7.44.0)"
DEPENDS += "gstreamer1.0 gstreamer1.0-plugins-base"

# Specify any options you want to pass to cmake using EXTRA_OECMAKE:
EXTRA_OECMAKE = "-DGSTREAMER_MEDIA_PLAYER=ON \
                 -DSYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR}"

FILES_${PN} += "${libdir}/lib*.so \
              ${bindir}/ \
              /usr/include/*"
FILES_${PN}-dbg  += "${libdir}/.debug/lib*.so \
                   ${bindir}/.debug/*"
FILES_SOLIBSDEV = ""
