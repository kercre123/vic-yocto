SUMMARY = "Example recipe for using inherit useradd"
DESCRIPTION = "This recipe serves as an example for using features from useradd.bbclass"
SECTION = "examples"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://file1"

S = "${WORKDIR}"


inherit useradd

# You must set USERADD_PACKAGES when you inherit useradd. This
# lists which output packages will include the user/group
# creation code.
USERADD_PACKAGES = "${PN} "

# You must also set USERADD_PARAM and/or GROUPADD_PARAM when
# you inherit useradd.

# VIC-826 related work
# We modify the sample recipe useradd_example to 
# complete the proof of concept
# a more ANKI specific recipe will be created at a 
# later stage.
# We will copy the meta-skeleton layer to a 
# meta-user-management layer as a starting point
#
# USERADD_PARAM specifies command line options to pass to the
# useradd command. Multiple users can be created by separating
# the commands with a semicolon. Here we'll create three users,
# robot, bluetooth, net:
USERADD_PARAM_${PN} = "-u 1500 -r robot; -u 1501  -r bluetooth; -u 1502  -r net"


# GROUPADD_PARAM works the same way, which you set to the options
# you'd normally pass to the groupadd command. This will create
# groups robotics, wireless, networking:
GROUPADD_PARAM_${PN} = "-g 891 robotics; -g 892 wireless; -g 893 networking"


do_install () {
	install -d -m 755 ${D}${datadir}/tmp
	install -p -m 644 file1 ${D}${datadir}/tmp/
}

FILES_${PN} = "${datadir}/tmp/* "

# Prevents do_package failures with:
# debugsources.list: No such file or directory:
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

