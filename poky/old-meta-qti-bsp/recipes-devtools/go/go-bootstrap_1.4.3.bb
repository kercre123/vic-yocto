require go_${PV}.inc

inherit native

do_compile() {
  ## Setting `$GOBIN` doesn't do any good, looks like it ends up copying binaries there.
  export GOROOT_FINAL="${GOROOT_BOOTSTRAP}"

  setup_go_arch

  export CGO_ENABLED="0"
  ## TODO: consider setting GO_EXTLINK_ENABLED

  # Compile
  cd src && bash -x ./make.bash

  # Log the resulting environment
  env "GOROOT=${WORKDIR}/go-${PV}/go" "${WORKDIR}/go-${PV}/go/bin/go" env
}
