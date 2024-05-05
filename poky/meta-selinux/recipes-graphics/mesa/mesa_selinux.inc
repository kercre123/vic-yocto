inherit enable-selinux

# But wait!  There's more!  mesa builds a host program named builtin_compiler
# and it needs selinux, too.  We replace the PACKAGECONFIG[] in the bbclass.
#
PACKAGECONFIG[selinux] = "--enable-selinux,--disable-selinux,libselinux libselinux-native,"
