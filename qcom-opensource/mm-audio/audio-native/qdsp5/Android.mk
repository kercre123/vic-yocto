ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------

mm-audio-native-def := -g -O3
mm-audio-native-def += -DQC_MODIFIED
mm-audio-native-def += -D_ANDROID_
mm-audio-native-def += -DFEATURE_EXPORT_SND
mm-audio-native-def += -DFEATURE_AUDIO_AGC
mm-audio-native-def += -DFEATURE_VOC_PCM_INTERFACE
mm-audio-native-def += -DFEATURE_VOICE_PLAYBACK
mm-audio-native-def += -DFEATURE_VOICE_RECORD
mm-audio-native-def += -DVERBOSE
mm-audio-native-def += -D_DEBUG

ifeq ($(BOARD_USES_QCOM_AUDIO_V2), true)
mm-audio-native-def += -DAUDIOV2
endif

ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
mm-audio-native-def += -DTARGET_USES_QCOM_MM_AUDIO
endif

ifeq ($(call is-board-platform,msm8660),true)
mm-audio-native-def += -DQDSP6V2
endif

ifeq ($(call is-board-platform,msm7627a),true)
mm-audio-native-def += -DAUDIO7X27A
endif

# ---------------------------------------------------------------------------------
#                       Make the apps-test (mm-audio-native-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-audio-native-inc     := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
mm-audio-native-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-alsa
ifeq ($(call is-board-platform,msm8660),true)
mm-audio-native-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-acdb-util
mm-audio-native-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-iec-60958-61937
mm-audio-native-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-parsers
endif
endif

LOCAL_MODULE            := mm-audio-native-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(mm-audio-native-def)
LOCAL_PRELINK_MODULE    := false

LOCAL_C_INCLUDES        := $(mm-audio-native-inc)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
LOCAL_SHARED_LIBRARIES  := libaudioalsa
ifeq ($(call is-board-platform,msm8660),true)
LOCAL_SHARED_LIBRARIES	+= libacdbloader
LOCAL_SHARED_LIBRARIES  += libacdbmapper
LOCAL_SHARED_LIBRARIES	+= lib_iec_60958_61937
LOCAL_SHARED_LIBRARIES	+= libaudioparsers
endif
endif

LOCAL_SRC_FILES := audiotest.c
LOCAL_SRC_FILES += mp3test.c
LOCAL_SRC_FILES += pcmtest.c
LOCAL_SRC_FILES += qcptest.c
LOCAL_SRC_FILES += aactest.c
LOCAL_SRC_FILES += amrnbtest.c
LOCAL_SRC_FILES += amrwbtest.c
LOCAL_SRC_FILES += wmatest.c
LOCAL_SRC_FILES += wmaprotest.c
LOCAL_SRC_FILES += voicememotest.c
LOCAL_SRC_FILES += audioprofile.c
LOCAL_SRC_FILES += snddevtest.c
LOCAL_SRC_FILES     += equalizer.c
LOCAL_SRC_FILES     += ion_alloc.c

ifeq ($(BOARD_USES_QCOM_AUDIO_V2), true)
LOCAL_SRC_FILES += sbctest.c
LOCAL_SRC_FILES += adpcmtest.c
LOCAL_SRC_FILES += voiceenctest.c
LOCAL_SRC_FILES += fm_test.c
LOCAL_SRC_FILES += lpatest.c
ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
LOCAL_SRC_FILES += devctltest.c
ifeq ($(call is-board-platform,msm8660),true)
LOCAL_SRC_FILES += mvstest.c
LOCAL_SRC_FILES += hdmi_ac3.c
LOCAL_SRC_FILES += hdmi_dts.c
endif
endif
else
ifeq ($(call is-board-platform,msm7627a),true)
LOCAL_SRC_FILES += voiceenctest.c
LOCAL_SRC_FILES += fm_test.c
endif
endif

include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID

# ---------------------------------------------------------------------------------
#                                       END
# ---------------------------------------------------------------------------------

