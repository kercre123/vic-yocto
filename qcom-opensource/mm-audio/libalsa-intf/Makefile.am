AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I.

c_sources = alsa_mixer.c \
            alsa_pcm.c \
            alsa_ucm.c

h_sources = alsa_ucm.h \
            msm8960_use_cases.h \
            alsa_audio.h

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libalsa_intf.la
libalsa_intf_la_CC = @CC@
libalsa_intf_la_SOURCES = $(c_sources) $(h_sources)
libalsa_intfdir = $(prefix)/snd_soc_msm
dist_libalsa_intf_DATA    = snd_soc_msm/snd_soc_msm \
                            snd_soc_msm/snd_soc_msm_2x \
                            snd_soc_msm/snd_soc_msm_2x_Fusion3 \
                            snd_soc_msm/snd_soc_msm_Sitar \
                            snd_soc_msm/snd_soc_msm_I2S \
                            snd_soc_msm/snd_soc_msm_Taiko_I2S \
                            snd_soc_msm/snd_soc_msm_8x10_wcd \
                            snd_soc_msm/snd_soc_msm_8x10_wcd_skuaa \
                            snd_soc_msm/snd_soc_msm_8x10_wcd_skuab \
                            snd_soc_msm/snd_soc_msm_Tapan \
                            snd_soc_msm/snd_soc_msm_TapanLite \
                            snd_soc_msm/snd_soc_msm_Tapan_SKUF \
                            snd_soc_msm/snd_soc_msm_9x07_Tomtom_I2S \
                            snd_soc_msm/snd_soc_msm_9x40_Tomtom_I2S \
                            snd_soc_msm/snd_soc_msm_Tasha_I2S

if USE_QCOM_ACDB
libalsa_intf_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@ -DCONFIG_DIR=\"/etc/snd_soc_msm/\"
libalsa_intf_la_CPPFLAGS = $(AM_CPPFLAGS) $(ACDBLOADER_CFLAGS) -DQC_PROP -DUSE_GLIB @GLIB_CFLAGS@
libalsa_intf_la_LDFLAGS = $(ACDBLOADER_LIBS) -lm -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
else
libalsa_intf_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@ -DCONFIG_DIR=\"/etc/snd_soc_msm/\"
libalsa_intf_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libalsa_intf_la_LDFLAGS = -lm -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
endif

requiredlibs = libalsa_intf.la

bin_PROGRAMS = aplay amix arec alsaucm_test

aplay_SOURCES = aplay.c
aplay_LDADD = -lpthread $(requiredlibs)

amix_SOURCES = amix.c
amix_LDADD = -lpthread $(requiredlibs)

arec_SOURCES = arec.c
arec_LDADD = -lpthread $(requiredlibs)

alsaucm_test_SOURCES = alsaucm_test.c
if USE_QCOM_ACDB
alsaucm_test_LDADD = -lpthread $(requiredlibs) $(ACDBLOADER_LIBS)
alsaucm_test_CPPFLAGS = -DQC_PROP $(ACDBLOADER_CFLAGS)
else
alsaucm_test_LDADD = -lpthread $(requiredlibs)
endif
