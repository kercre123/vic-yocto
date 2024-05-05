
# on targets with MULTILIBS (e.g: lib32- and lib64-) prefix gdb binaries
# with arch to avoid over-writes as they are installed in /usr/bin
GDBPROPREFIX = "${@ '--program-prefix="${TARGET_PREFIX}"' if d.getVar('MULTILIBS') else ''}"
