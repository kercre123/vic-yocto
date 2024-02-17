FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
# Adjust branch & URI to fetch from Codelinaro.
SRCBRANCH = "caf_migration/drains/release/${PV}/master"
GLIBC_GIT_URI = "${CLO_LE_GIT}/glibc.git;protocol=https"

SRC_URI += "file://CVE-2019-25013.patch \
            file://CVE-2020-1752.patch \
            file://CVE-2020-6096.patch \
            file://CVE-2020-6096-01.patch \
            file://CVE-2020-6096-02.patch \
            file://CVE-2020-6096-03.patch \
            file://CVE-2020-6096-04.patch \
            file://CVE-2020-6096-05.patch \
            file://CVE-2021-3326.patch \
            file://CVE-2021-35942.patch \
            file://CVE-2021-38604.patch \
            file://CVE-2021-38604-01.patch \
            file://CVE-2022-23218.patch \
            file://CVE-2022-23218-01.patch \
            file://CVE-2022-23219.patch \
            file://CVE-2021-43396.patch \
            file://0001-malloc-tcache-double-free-check.patch \
            file://0001-Fix-tcache-count-maximum-BZ-24531.patch \
"

# glibc expects -fstack-protector optimization passed as a configuration option
# instead of a top level build flag.
python __anonymous () {
    sel_opt = d.getVar("SELECTED_OPTIMIZATION", True).split()

    for opt in sel_opt:
        if opt in ("-fstack-protector", "-fstack-protector-all", "-fstack-protector-strong"):
            # bb.note("%s can't be built with %s" % (d.getVar('PN'), sel_opt))
            sel_opt.remove(opt)
    d.setVar('SELECTED_OPTIMIZATION', ' '.join(sel_opt))
}

EXTRA_OECONF += "${@bb.utils.contains('FULL_OPTIMIZATION', '-fstack-protector', '--enable-stack-protector=yes', '', d)}"
EXTRA_OECONF += "${@bb.utils.contains('FULL_OPTIMIZATION', '-fstack-protector-all', '--enable-stack-protector=all', '', d)}"
EXTRA_OECONF += "${@bb.utils.contains('FULL_OPTIMIZATION', '-fstack-protector-strong', '--enable-stack-protector=strong', '', d)}"

EXTRA_OECONF += "${@bb.utils.contains('DEBUG_OPTIMIZATION', '-fstack-protector', '--enable-stack-protector=yes', '', d)}"
EXTRA_OECONF += "${@bb.utils.contains('DEBUG_OPTIMIZATION', '-fstack-protector-all', '--enable-stack-protector=all', '', d)}"
EXTRA_OECONF += "${@bb.utils.contains('DEBUG_OPTIMIZATION', '-fstack-protector-strong', '--enable-stack-protector=strong', '', d)}"
