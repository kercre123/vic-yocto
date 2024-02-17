SRC_URI = "${CAF_GIT}/platform/external/update-rc.d;protocol=https;branch=caf_migration/github/master \
           file://add-verbose.patch \
           file://check-if-symlinks-are-valid.patch \
          "

PR = "r6"
