LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------

mm-audio-native-def := -g -O3
mm-audio-native-def += -DQC_MODIFIED
mm-audio-native-def += -D_ANDROID_

ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
mm-audio-native-def += -DTARGET_USES_QCOM_MM_AUDIO
endif
# ---------------------------------------------------------------------------------
#                       Make the apps-test (mm-audio-native-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
mm-audio-native-inc     += $(TARGET_OUT_HEADERS)/mm-audio/atu
endif

LOCAL_MODULE            := mm-audio-native-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(mm-audio-native-def)
LOCAL_PRELINK_MODULE    := false
ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
LOCAL_C_INCLUDES        := $(mm-audio-native-inc)
LOCAL_SHARED_LIBRARIES  := libatu
endif

LOCAL_SRC_FILES := audiotest.c
LOCAL_SRC_FILES += amrnbtest.c
LOCAL_SRC_FILES += mp3test.c
LOCAL_SRC_FILES += pcmtest.c
LOCAL_SRC_FILES += equalizer.c
LOCAL_SRC_FILES += aactest.c
LOCAL_SRC_FILES += audio_ctrl.c
LOCAL_SRC_FILES += atutest.c
LOCAL_SRC_FILES += qcptest.c

include $(BUILD_EXECUTABLE)

# ---------------------------------------------------------------------------------
#                                       END
# ---------------------------------------------------------------------------------

