AUDIO_NATIVE := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-board-platform-in-list,$(MSM7K_BOARD_PLATFORMS)),true)
    include $(AUDIO_NATIVE)/qdsp5/Android.mk
else ifeq ($(call is-board-platform-in-list,$(QSD8K_BOARD_PLATFORMS)),true)
    include $(AUDIO_NATIVE)/qdsp6/Android.mk
else ifeq ($(call is-board-platform,msm8660),true)
    include $(AUDIO_NATIVE)/qdsp5/Android.mk
endif
