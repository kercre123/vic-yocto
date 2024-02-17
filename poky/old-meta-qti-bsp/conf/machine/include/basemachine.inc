BASEMACHINE ??= ""

# Add BASEMACHINE to machine overrides
MACHINEOVERRIDES =. "${@['', '${BASEMACHINE}:']['${BASEMACHINE}' != '']}"

# Define BASEMACHINE_ARCH
BASEMACHINE_ARCH = "${@['${MACHINE_ARCH}', '${BASEMACHINE}']['${BASEMACHINE}' != '']}"

# Add BASEMACHINE_ARCH to PACKAGE_ARCHS. 
# This variable is originally set in bitbake.conf
PACKAGE_ARCHS .= "${@['', ' ${BASEMACHINE_ARCH}']['${BASEMACHINE_ARCH}' != '${MACHINE_ARCH}']}"
PACKAGE_ARCHS[vardepsexclude] += "BASEMACHINE_ARCH"
