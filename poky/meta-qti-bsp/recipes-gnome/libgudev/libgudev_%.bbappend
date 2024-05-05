#Appending the gobject-introspection package to the DEPEND list so the recipe could successfully find introspection.m4 during configure task.

DEPENDS += "gobject-introspection gobject-introspection-native qemu-native"

