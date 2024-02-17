inherit npm-base

DESCRIPTION = "WebUI"
SECTION = "webui"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

DEPENDS := "nodejs-native"

FILESPATH =+ "${WORKSPACE}/vendor/qcom/opensource/qmmf-webserver:"

SRC_URI = "file://webui"

S 	= "${WORKDIR}/webui/"

FILES_${PN} += "/data/*"

do_compile(){
}

do_install() {
	mkdir -p ${S}dist_temp/
	mkdir -p ${TOPDIR}/downloads/node_modules/
	cd ${S}/src/
	rm -rf ./node_modules
	ln -s ${TOPDIR}/downloads/node_modules node_modules
	oe_runnpm install
	oe_runnpm_native run build
	install -d ${D}/data/misc/qmmf/ipc_webserver/dist/
	cp -r ${S}/dist_temp/* ${D}/data/misc/qmmf/ipc_webserver/dist/
}

sysroot_preprocess() {
	install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/dist/
	cp -r ${S}/dist_temp/*  ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/dist/
}

SYSROOT_PREPROCESS_FUNCS += "sysroot_preprocess"
