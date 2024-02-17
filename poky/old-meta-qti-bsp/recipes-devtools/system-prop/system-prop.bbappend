SUMMARY = "Anki added persistent properties"

do_compile_append() {
  # turn off verbose qcom debug logs
  echo "persist.camera.hal.debug.mask=7" >> ${S}/build.prop
  echo "persist.camera.mct.debug.mask=1" >> ${S}/build.prop

  # VIC-2127: Increase max tombstones
  echo "service.debuggerd.tombstones=32" >> ${S}/build.prop

}
