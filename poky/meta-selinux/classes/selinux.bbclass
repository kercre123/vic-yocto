def target_selinux(d, truevar = 'selinux', falsevar = ''):
    if not bb.utils.contains("DISTRO_FEATURES", "selinux", True, False, d):
        return falsevar

    pn = d.getVar("PN", True) 
    type = pn.replace(d.getVar("BPN", True), "") 
    if type in ("-native", "nativesdk-", "-cross", "-crosssdk"):
        return falsevar

    return truevar 

LIBSELINUX = "${@target_selinux(d, 'libselinux')}"
