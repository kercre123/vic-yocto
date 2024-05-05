LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/Makefile.sources

LOCAL_SRC_FILES := $(filter-out %.h,$(MODETEST_FILES))

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
kernel_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES              := $(kernel_includes)
LOCAL_CFLAGS                  := $(common_flags) -DUSE_ION
endif

LOCAL_MODULE := modetest

LOCAL_SHARED_LIBRARIES := libdrm
LOCAL_STATIC_LIBRARIES := libdrm_util

include $(BUILD_EXECUTABLE)
