require go_${PV}.inc

GOROOT_FINAL="${libdir}/go"
export GOROOT_FINAL

DEPENDS = "virtual/${TARGET_PREFIX}gcc"

inherit cross

do_compile() {
  setup_go_arch

  export CGO_ENABLED="${GO_CROSS_CGO_ENABLED}"
  ## TODO: consider setting GO_EXTLINK_ENABLED

  export CC="${BUILD_CC}"
  export CC_FOR_TARGET="${TARGET_PREFIX}gcc ${TARGET_CC_ARCH} --sysroot=${STAGING_DIR_TARGET}"
  export CXX_FOR_TARGET="${TARGET_PREFIX}g++ ${TARGET_CC_ARCH} --sysroot=${STAGING_DIR_TARGET}"
  export GO_CCFLAGS="${HOST_CFLAGS}"
  export GO_LDFLAGS="${HOST_LDFLAGS}"

  cd src && bash -x ./make.bash

  # log the resulting environment
  env "GOROOT=${WORKDIR}/go-${PV}/go" "${WORKDIR}/go-${PV}/go/bin/go" env
}

do_install() {
  install -d "${D}${libdir}/go"
  tar -C "${WORKDIR}/go-${PV}/go/" -cf - bin include lib pkg src test |
  tar -C "${D}${libdir}/go" -xf -
}
