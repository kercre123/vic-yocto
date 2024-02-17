require go-common.inc

GO_TAG = "1_5"

SRC_URI = "${SRC_URI_PREFIX}/go${PV}.src.tar.gz;name=go_${GO_TAG};subdir=go-${PV}"
SRC_URI[go_1_5.md5sum] = "a04d570515c46e4935c63605cbd3a04e"
SRC_URI[go_1_5.sha256sum] = "002acabce7ddc140d0d55891f9d4fcfbdd806b9332fb8b110c91bc91afb0bc93"

SRC_URI += "\
        file://Fix-ccache-compilation-issue.patch \
        file://fix-cross-compilation.patch \
        "
