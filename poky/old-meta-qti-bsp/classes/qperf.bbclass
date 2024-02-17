#
# This class configure recipes which need customizations in production builds
#

python __anonymous() {
    # Append PRODUCT, VARIANT info to PR
    prd = d.getVar('PRODUCT', True)
    var = d.getVar('VARIANT', True)
    revision = d.getVar('PR', True)
    if prd != "base":
        revision += "_"+prd
    if var != "":
        revision += "_"+var

    # Update PR value to ensure recipe rebuilds.
    if (d.getVar('PERF_BUILD', True) == '1'):
        if (d.getVar('USER_BUILD', True) == '1'):
            revision += "_user"
        else:
            revision += "_perf"
    d.setVar('PR', revision)

    # While building kernel or kernel module recipes add a task to
    # copy build artifacts into DEPLOY_DIR for ease of access
    provides = d.getVar('PROVIDES', True)
    if (("virtual/kernel" in provides)):
        bb.build.addtask('do_copy_vmlinux', 'do_strip', 'do_shared_workdir', d)
    elif (bb.data.inherits_class("module", d)):
        bb.build.addtask('do_copy_kernel_module', 'do_module_signing', 'do_install', d)
}

# Copy vmlinux into image specific deploy directory.
do_copy_vmlinux[dirs] = "${DEPLOY_DIR_IMAGE}"
do_copy_vmlinux() {
    install -m 644 ${B}/vmlinux ${DEPLOY_DIR_IMAGE}
}

# Copy kernel modules into image specific deploy directory.
do_copy_kernel_module[dirs] = "${DEPLOY_DIR_IMAGE}/kernel_modules/${PN}"
do_copy_kernel_module() {
    cd ${S}
    for mod in *.ko; do
        if [ -f $mod ]; then
            install -m 0644 $mod ${DEPLOY_DIR_IMAGE}/kernel_modules/${PN}
        fi
    done
}
