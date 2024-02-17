do_compile() {
  export GOROOT_BOOTSTRAP="${GOROOT_BOOTSTRAP}"

  setup_go_arch

  ## TODO: consider setting GO_EXTLINK_ENABLED
  export CGO_ENABLED="${GO_CROSS_CGO_ENABLED}"
  export CC=${BUILD_CC}
  export CC_FOR_TARGET="${WORKDIR}/${TARGET_PREFIX}gcc"
  export CXX_FOR_TARGET="${WORKDIR}/${TARGET_PREFIX}g++"
  export GO_GCFLAGS="${HOST_CFLAGS}"
  export GO_LDFLAGS="${HOST_LDFLAGS}"

  set > ${WORKDIR}/go-${PV}.env
  cd ${WORKDIR}/go-${PV}/go/src && bash -x ./make.bash

  # log the resulting environment
  env "GOROOT=${WORKDIR}/go-${PV}/go" "${WORKDIR}/go-${PV}/go/bin/go" env
}

do_install() {
  install -d "${D}${bindir}" "${GOROOT_FINAL}"
  tar -C "${WORKDIR}/go-${PV}/go" -cf - bin lib src pkg test |
  tar -C "${GOROOT_FINAL}" -xf -
  mv "${GOROOT_FINAL}/bin/"* "${D}${bindir}/"

  for t in gcc g++ ; do
    m="${GOROOT_FINAL}/bin/${TARGET_PREFIX}${t}"
    if ! [ -e "${m}" ]; then
      cat > ${m} <<EOT
#!/bin/sh
exec ${STAGING_BINDIR_NATIVE}/${TARGET_SYS}.${MLPREFIX}gcc-cross-initial-${TARGET_ARCH}/${TARGET_PREFIX}${t} ${TARGET_CC_ARCH} --sysroot=${STAGING_DIR_TARGET} "\$@"
EOT
    chmod +x ${GOROOT_FINAL}/bin/${TARGET_PREFIX}${t}
    fi
  done
}

do_cleanscripts() {
  for t in gcc g++ ; do
    m="${GOROOT_FINAL}/bin/${TARGET_PREFIX}${t}"
    if [ -e "${m}" ]; then
      rm ${m}
    fi
  done
}
addtask cleanscripts before do_install before do_clean
