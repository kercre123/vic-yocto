ifneq ($(SDM660_DISABLE_MODULE),true)
LOCAL_PATH := $(call my-dir)

MTD_UTILS_VERSION=1.5.1

TARGET_MTD_UTILS_VERSION_H := $(TARGET_OUT_HEADERS)/mtd-utils/include/version.h
HOST_MTD_UTILS_VERSION_H := $(HOST_OUT_HEADERS)/mtd-utils/include/version.h
$(TARGET_MTD_UTILS_VERSION_H) $(HOST_MTD_UTILS_VERSION_H):
	@mkdir -p $(dir $@)
	echo '#define VERSION "$(MTD_UTILS_VERSION)"' > $@

TARGET_INCLUDE_PATHS := \
                external/mtd-utils/ubi-utils/include \
                external/mtd-utils/include \
                $(TARGET_OUT_HEADERS)/mtd-utils/include

HOST_INCLUDE_PATHS := \
                external/mtd-utils/ubi-utils/include \
                external/mtd-utils/include \
                $(HOST_OUT_HEADERS)/mtd-utils/include

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(HOST_INCLUDE_PATHS)
LOCAL_SRC_FILES :=  ubi-utils/libubi.c ubi-utils/libubigen.c \
                    ubi-utils/libscan.c ubi-utils/dictionary.c ubi-utils/libiniparser.c \
                    ubi-utils/ubiutils-common.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(HOST_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := libubi
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libmtd
include $(BUILD_HOST_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES :=  ubi-utils/libubi.c ubi-utils/libubigen.c \
                    ubi-utils/libscan.c ubi-utils/dictionary.c ubi-utils/libiniparser.c \
                    ubi-utils/ubiutils-common.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := libubi
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libmtd
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES :=  ubi-utils/libubi.c ubi-utils/libubigen.c \
                    ubi-utils/libscan.c ubi-utils/dictionary.c ubi-utils/libiniparser.c \
                    ubi-utils/ubiutils-common.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := libubi
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(HOST_INCLUDE_PATHS)
LOCAL_SRC_FILES :=  lib/libmtd.c lib/libcrc32.c lib/libmtd_legacy.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(HOST_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := libmtd
LOCAL_MODULE_TAGS := optional
include $(BUILD_HOST_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES :=  lib/libmtd.c lib/libcrc32.c lib/libmtd_legacy.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := libmtd
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubinfo.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubinfo
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libc libubi
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubiattach.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubiattach
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubidetach.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubidetach
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubiformat.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubiformat
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi libmtd
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubimkvol.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubimkvol
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubirmvol.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubirmvol
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubiupdatevol.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubiupdatevol
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libc libubi
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubicrc32.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubicrc32
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi libmtd
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubirename.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubirename
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubirsvol.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubirsvol
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubiblock.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubiblock
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(HOST_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/ubinize.c ubi-utils/dictionary.c ubi-utils/ubi-fastmap.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(HOST_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := ubinize
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libubi libmtd
include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := mkfs.ubifs/mkfs.ubifs.c mkfs.ubifs/crc16.c mkfs.ubifs/devtable.c \
                   mkfs.ubifs/lpt.c mkfs.ubifs/compr.c mkfs.ubifs/hashtable/hashtable.c \
                   mkfs.ubifs/hashtable/hashtable_itr.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(HOST_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID -D_GNU_SOURCE
LOCAL_C_INCLUDES += $(HOST_INCLUDE_PATHS) \
                    external/lzo/include external/mtd-utils/mkfs.ubifs/hashtable \
                    external/zlib external/e2fsprogs/lib/uuid
LOCAL_SHARED_LIBRARIES := libubi liblzo libext2_uuid_host libmtd libz-host
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := mkfsubifs
include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(TARGET_INCLUDE_PATHS)
LOCAL_SRC_FILES := ubi-utils/mtdinfo.c
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_MTD_UTILS_VERSION_H)
LOCAL_CFLAGS += -DANDROID
LOCAL_MODULE := mtdinfo
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc libubi libmtd
include $(BUILD_EXECUTABLE)

endif#SDM845_DISABLE_MODULE
