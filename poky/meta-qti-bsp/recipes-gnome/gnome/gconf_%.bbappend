#Appending the gobject-introspection package to the DEPEND list so the recipe could successfully find introspection.m4 during configure task.

DEPENDS += "gobject-introspection gobject-introspection-native"

# Fix the native recipe sysroot so that the qemu-arm is provided
# for build system to use when building pango library for target.
DEPENDS += "qemu-native"
