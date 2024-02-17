inherit autotools
SUMMARY = "General purpose compressed audio format for mid to high quality at fixed variable bitrates."
HOMEPAGE = "http://xiph.org/vorbis/"
BUGTRACKER = "https://trac.xiph.org/"
LICENSE = "BSD"
PRIORITY = "optional"
DEPENDS = "libogg"
LIC_FILES_CHKSUM = "file://COPYING;md5=ca77c6c3ea4d29cb68dce8ef5ab0d897"

# Package Revision (update whenever recipe is changed)
PR = "r1"

SRC_URI = "\
    http://downloads.xiph.org/releases/vorbis/${PN}-1.3.3.tar.gz \
"

SRC_URI[md5sum] = "6b1a36f0d72332fae5130688e65efe1f"
SRC_URI[sha256sum] = "6d747efe7ac4ad249bf711527882cef79fb61d9194c45b5ca5498aa60f290762"

# vorbisfile.c reveals a problem in the gcc register spilling for the
# thumb instruction set...
FULL_OPTIMIZATION_thumb = "-O0"

EXTRA_OECONF = "--with-ogg-libraries=${STAGING_LIBDIR} \
                --with-ogg-includes=${STAGING_INCDIR}"
