SUMMARY = "SELinux binary policy manipulation library"
DESCRIPTION = "libsepol provides an API for the manipulation of SELinux binary policies. \
It is used by checkpolicy (the policy compiler) and similar tools, as well \
as by programs like load_policy that need to perform specific transformations \
on binary policies such as customizing policy boolean settings."
SECTION = "base"
LICENSE = "LGPLv2+"

inherit lib_package

# Change RANLIB for cross compiling, use host-tools $(AR) rather than
# local ranlib.
EXTRA_OEMAKE += "RANLIB='$(AR) s'"

DEPENDS += "flex-native"

BBCLASSEXTEND = "native"
