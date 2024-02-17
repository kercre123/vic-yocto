# Target in which  DISTRO_FEATURES = "le-clang" is enabled will use sdllvm/clang as compiler for recipes inheriting this class.
# Recipe inheriting this class and using  ENABLE_SDLLVM = "true"  will use sdllvm/clang as compiler. This check can also be used on target basis.


RECIPE_CHECK = "${@bb.utils.contains('ENABLE_SDLLVM', 'true','true', 'false ',d)}"

USE_CLANG = "${@bb.utils.contains('DISTRO_FEATURES', 'le-clang','true','${RECIPE_CHECK}',d)}"

DEPENDS +="${@bb.utils.contains('USE_CLANG', 'true','llvm-arm-toolchain-native','',d)}"

FULL_OPTIMIZATION = "${@bb.utils.contains('USE_CLANG', 'true',\
  '-O2  -fomit-frame-pointer -Wno-error=maybe-uninitialized  -Wno-error=unused-result -Wno-error=unknown-warning-option -Wno-error=unused-comparison \
    -Wno-error=unused-private-field -Wno-error=undefined-optimized -Wno-error=format -Wno-error=inconsistent-missing-override', \
  '-O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer -ftree-vectorize   -Wno-error=maybe-uninitialized -finline-functions -finline-limit=64', d)}"

NEON_FLAGS = "${@bb.utils.contains('TARGET_ARCH', 'arm', '-march=armv7-a -mfloat-abi=softfp -mfpu=neon -ftree-vectorize ', ' ',d)}"

CLANG_CPP   = "${@bb.utils.contains_any('TARGET_ARCH', 'arm aarch64', 'CPP="${STAGING_BINDIR_TOOLCHAIN}/../llvm-arm-toolchain/bin/clang -E  \
              -target ${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS} --sysroot=${STAGING_DIR_TARGET} ${NEON_FLAGS}" ', ' ',d)}"
CLANG_CC   = "${@bb.utils.contains_any('TARGET_ARCH', 'arm aarch64', 'CC="${STAGING_BINDIR_TOOLCHAIN}/../llvm-arm-toolchain/bin/clang  \
              -target ${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS} --sysroot=${STAGING_DIR_TARGET} ${NEON_FLAGS}" ', ' ',d)}"

CLANG_CXX  =  "${@bb.utils.contains_any('TARGET_ARCH', 'arm aarch64', 'CXX="${STAGING_BINDIR_TOOLCHAIN}/../llvm-arm-toolchain/bin/clang++  \
              -target ${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS} --sysroot=${STAGING_DIR_TARGET} ${NEON_FLAGS}" ', ' ',d)}"

EXTRA_OECONF +="${@bb.utils.contains('USE_CLANG', 'true','${CLANG_CPP}', ' ',d)}"

EXTRA_OECONF +="${@bb.utils.contains('USE_CLANG', 'true','${CLANG_CC}', ' ',d)}"

EXTRA_OECONF +="${@bb.utils.contains('USE_CLANG', 'true','${CLANG_CXX}', ' ',d)}"

COMPILER_C_CMAKE = "${@bb.utils.contains('USE_CLANG', 'true','"${STAGING_BINDIR_TOOLCHAIN}/../llvm-arm-toolchain/bin/clang"', '',d)}"

COMPILER_CPP_CMAKE = "${@bb.utils.contains('USE_CLANG', 'true','"${STAGING_BINDIR_TOOLCHAIN}/../llvm-arm-toolchain/bin/clang++"', '',d)}"

COMPILER_C_CMAKE_FLAGS =  "${@bb.utils.contains('USE_CLANG', 'true','-target ${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS} ${NEON_FLAGS} ', '',d)}"

OECMAKE_C_COMPILER = "${COMPILER_C_CMAKE}"

OECMAKE_CXX_COMPILER = "${COMPILER_CPP_CMAKE}"

OECMAKE_C_FLAGS += "${COMPILER_C_CMAKE_FLAGS}"

OECMAKE_CXX_FLAGS += "${COMPILER_C_CMAKE_FLAGS}"
