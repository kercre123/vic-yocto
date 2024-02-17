#
# This empty Android.mk file exists to prevent the build system from
# automatically including any other Android.mk files under this directory.
#

#include $(CLEAR_VARS)
#LOCAL_MODULE       := wpa_supplicant.conf
#LOCAL_MODULE_TAGS  := optional
#LOCAL_MODULE_CLASS := ETC
#LOCAL_SRC_FILES    := $(LOCAL_MODULE)
#LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/wifi
#include $(BUILD_PREBUILT)

#include $(call all-makefiles-under,$(LOCAL_PATH))
