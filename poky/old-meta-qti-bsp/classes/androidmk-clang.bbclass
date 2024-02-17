FULL_OPTIMIZATION = "${@bb.utils.contains('DISTRO_FEATURES', 'le-clang',\
  '-O2 -frename-registers -fomit-frame-pointer -ftree-vectorize -Wno-error=maybe-uninitialized -finline-limit=64 -Wno-error=unused-result -Wno-error=unknown-warning-option -Wno-error=int-conversion', \
  '-O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer -ftree-vectorize   -Wno-error=maybe-uninitialized -finline-functions -finline-limit=64', d)}"

use_clang_android () {
  if ${@bb.utils.contains('DISTRO_FEATURES', 'le-clang', 'true', 'false', d)}; then
        export USE_CLANG=true
        export LE_LLVM_TOOLCHAIN_PATH=${STAGING_BINDIR_TOOLCHAIN}/../llvm-arm-toolchain/bin/
        export LE_TRIPLE="${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS}"
        export LOCAL_CLANG=true
fi
}
