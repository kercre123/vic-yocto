#--------------------------------------------
#  The bouncycastle API is used by
#  kernel for secure boot image signing
#--------------------------------------------

DESCRIPTION = "Build Bouncycastle API JAR and Signed BootSignature JAR"
HOMEPAGE = "https//www.bouncycastle.org/java.html"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../bouncycastle/MODULE_LICENSE_BSD_LIKE;md5=d41d8cd98f00b204e9800998ecf8427e"
SRCREV = "${AUTOREV}"


#######################################
#  bouncycastle git repo
#######################################

SRC_URI = "git://source.codeaurora.org/quic/la/platform/external/bouncycastle;protocol=https;branch=LA.UM.5.6.c1;destsuffix=bouncycastle;name=bouncycastle"

SRC_URI[bouncycastle.md5sum] = "a786faa25a56a3e46331f34b4364575c"
SRC_URI[bouncycastle.sha256sum] = "909fb1e8ea06a9d6e918b3c5b0800a1f2408724a74e05cb310c019653d758e76"

S_BC = "${WORKDIR}/bouncycastle"


#######################################
#  extras git repo
#######################################
SRC_URI +=  "git://source.codeaurora.org/quic/la/platform/system/extras;protocol=https;branch=LA.UM.5.6.c1;destsuffix=extras;name=verity"

SRC_URI[verity.md5sum] = "ea0bac9d21dcae611ea6bdde97ed4125"
SRC_URI[verity.sha256sum] = "baa2967bbf0977150265ff878439c55bbd3b2095d7918146f6f9635268fc951e"

S_VERITY="${WORKDIR}/extras"

#######################################
#  keys/certificates git repo
#######################################
SRC_URI +=  "git://source.codeaurora.org/quic/la/platform/build;protocol=https;branch=LA.UM.5.6.c1;destsuffix=security;name=security"

SRC_URI[security.md5sum] = "1ba8f1041c6f81834450aefc80e78ba8"
SRC_URI[security.sha256sum] = "2174c4c82d24ec91e94949a3d1eb4fac29cc657b3e35c662c3e80042576fb6b6"

S_SECURITY="${WORKDIR}/security"

#######################################
#  build
#    - source
#    - classes
#    - libs
#######################################
BC_BUILD="${WORKDIR}/build"

check_java_version() {

    valid_version="1.7"
    ver=`java -version 2>&1 | grep -oP "([1-1]{1,}\.)+([2-8]{1,})"`
    version=`echo $ver | grep -oP "^1\.[1-8]{1,}"`

    if [ "$valid_version" = "$version" ]; then
        echo "Found Java $version"
    else
        echo "Checking if Java 1.7 is installed"
        if [ -d /usr/lib/jvm/java-1.7.0-openjdk-amd64/bin ]; then
            echo "Java 1.7 is present using it temporarily"
            export PATH=/usr/lib/jvm/java-1.7.0-openjdk-amd64/bin:$PATH
        fi

        ver=`java -version 2>&1 | grep -oP "([1-1]{1,}\.)+([2-8]{1,})"`
        version=`echo $ver | grep -oP "^1\.[1-8]{1,}"`
        if [ "$valid_version" = "$version" ]; then
             echo "Found valid Java $version"
        else
             echo "Invalid version $version, Please install Java 1.7"
             exit 1
        fi
    fi
}

do_precompile() {
    rm -fr ${BC_BUILD}/classes
    rm -fr ${BC_BUILD}/libs
    rm -fr ${BC_BUILD}/sources

    mkdir -p ${BC_BUILD}/classes
    mkdir -p ${BC_BUILD}/libs
    mkdir -p ${BC_BUILD}/sources

    echo "Main-Class: com.android.verity.BootSignature" > ${BC_BUILD}/manifest.mf
}


#---------------------------------------
#  bouncycastle API
#---------------------------------------
do_bc_compile() {

    find ${S_BC}/bcprov/ -name "*.java" > ${BC_BUILD}/sources/bcprov_java_source_list
    find ${S_BC}/bcpkix/ -name "*.java" > ${BC_BUILD}/sources/bcpkix_java_source_list

    /usr/bin/javac -J-Xmx1024M            \
          -Xmaxerrs 9999999      \
          -encoding UTF-8        \
          -g                     \
          -source 1.7            \
          -target 1.7            \
          -d ${BC_BUILD}/classes \
          @${BC_BUILD}/sources/bcprov_java_source_list


    #--------------------------------
    # create bcprov JAR
    #--------------------------------
    /usr/bin/jar -cf ${BC_BUILD}/libs/bcprov_jar.jar -C ${BC_BUILD}/classes .

    #--------------------------------
    # compile bcpkix
    #--------------------------------
    /usr/bin/javac -J-Xmx1024M       \
          -Xmaxerrs 9999999 \
          -encoding UTF-8   \
          -g                \
          -source 1.7       \
          -target 1.7     \
          -classpath ${BC_BUILD}/libs/bcprov_jar.jar  \
          -d ${BC_BUILD}/classes \
          @${BC_BUILD}/sources/bcpkix_java_source_list

    #--------------------------------
    # create bcprov + bcpkix JAR
    #--------------------------------
    /usr/bin/jar -cf ${BC_BUILD}/libs/bcprov_bcpkix_jar.jar -C ${BC_BUILD}/classes .
}

do_verity_compile () {

    find ${S_VERITY}/verity/ -name "*.java" >> ${BC_BUILD}/sources/verity_java_source_list

    #--------------------------------
    # compile BootSignature
    #--------------------------------
    /usr/bin/javac -J-Xmx1024M        \
          -Xmaxerrs 9999999  \
          -encoding UTF-8    \
          -g                 \
          -source 1.7      \
          -target 1.7        \
          -classpath ${BC_BUILD}/libs/bcprov_bcpkix_jar.jar  \
          -d ${BC_BUILD}/classes \
          @${BC_BUILD}/sources/verity_java_source_list

    #--------------------------------
    # create BootSignture JAR
    #--------------------------------
    /usr/bin/jar -cfm ${BC_BUILD}/libs/BootSignature.jar ${BC_BUILD}/manifest.mf -C ${BC_BUILD}/classes .

}

do_compile () {

    check_java_version
    do_precompile
    do_bc_compile
    do_verity_compile
}


