Here we describe the two parts of leveraging android native code (written for LA - Linux Android)
to build and link with this LE (Linux Embedded) build.

android_compat
==============

The recipe `androidcompat` contains the fetch and build instructions. This build process
pulls in a custom build-framework for both 32-bit and 64-bit building LA components.
In addition this project contains the the library and headers to harmonize the LA component.

Library and headers
-------------------

Recipe `androidcompat` builds a static library `android_compat.a`. This static library
provides the link time adaptation for symbols needed to run in LE build. In addition
the imported headers are organized to preserve the paths used in the code. All the headers
are installed to target system in the folder `/usr/include/android_compat`.

Build framework
---------------

The build framework is to enable build projects written with `Android.mk` files. This imports
along with the Android's makefile framework, sufficient product and board specific
definitions. The build framework lives in a work_shared folder as defined by the `LA_COMPAT_DIR`.

Class : inherit androidmk
==========================

`androidmk` class offers compile step for the code written to build with `Android.mk` file.
In this the `do_compile()` creates a suitable environment and invokes `make` tool for building
the code project files. This internally pulls in (via dependency) `androidcompat` recipe.

Following variables are pre-defined and are for use in the recipe files.

LA_COMPAT_DIR
--------------

This value is the path to location of `android_compat` build framework.

LA_OUT_DIR
-----------

This value is the path to output files. Typically located under thd folder `android_compat_build_artifacts`.

Writing a recipe that uses `Android.mk`
=======================================

Follow a normal recipe writing guide to compose the SRC_URI and revisions, etc.
Leverage the `androidmk.bbclass` by adding to your recipe. Here is a full example
for camera.bb

    inherit androidmk
    SUMMARY = "Camera libraries and SDK"
    SECTION = "camera"
    LICENSE = "MIT"
    LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

    # fetch sources
    FILESPATH =+ "${WORKSPACE}:"
    SRC_URI = "file://camera/lib"
    SRCREV = "${AUTOREV}"
    S = "${WORKDIR}/lib"
  
