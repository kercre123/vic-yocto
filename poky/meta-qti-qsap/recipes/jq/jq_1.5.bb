SUMMARY = "Lightweight and flexible command-line JSON processor"
HOMEPAGE = "https://stedolan.github.io/jq"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=29dd0c35d7e391bb8d515eacf7592e00"

SRC_URI = "\
	http://github.com/stedolan/jq/releases/download/jq-1.5/jq-1.5.tar.gz \
	"

SRC_URI[md5sum] = "0933532b086bd8b6a41c1b162b1731f9"
SRC_URI[sha256sum] = "c4d2bfec6436341113419debf479d833692cc5cdab7eb0326b5a4d4fbe9f493c"

inherit autotools
