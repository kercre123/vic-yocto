require go_${PV}.inc

GOROOT_FINAL="${libdir}/go"
export GOROOT_FINAL

inherit cross

require go-common-tasks.inc
