#Fetch from the Codelinaro location
SRC_URI = "${CLO_LE_GIT}/ncurses.git;protocol=https;branch=caf_migration/debian/master"
SRC_URI += "file://0001-tic-hang.patch \
            file://0002-configure-reproducible.patch \
            file://config.cache \
"
