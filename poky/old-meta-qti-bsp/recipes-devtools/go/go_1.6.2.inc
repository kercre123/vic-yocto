require go-common.inc

GO_TAG = "1_6"

SRC_URI = "${SRC_URI_PREFIX}/go${PV}.src.tar.gz;name=go_${GO_TAG};subdir=go-${PV}"
SRC_URI[go_1_6.md5sum] = "d1b50fa98d9a71eeee829051411e6207"
SRC_URI[go_1_6.sha256sum] = "787b0b750d037016a30c6ed05a8a70a91b2e9db4bd9b1a2453aa502a63f1bccc"

SRC_URI += "\
        file://Fix-ccache-compilation-issue.patch \
        file://fix-cross-compilation.patch \
        "
