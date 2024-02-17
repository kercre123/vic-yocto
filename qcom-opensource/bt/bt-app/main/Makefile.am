AM_CPPFLAGS = -O2 \
            -D_GNU_SOURCE

AM_CFLAGS = -Wall \
            -Wundef \
            -Wstrict-prototypes \
            -Wno-trigraphs \
            -std=c99

if USE_BT_OBEX
AM_CFLAGS += -DOI_CPU_TYPE=19
endif
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS += -I${WORKSPACE}/qcom-opensource/bt/bt-app \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/utils/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/audio_manager/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/a2dp_sink/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/avrcp/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/a2dp_src/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/osi/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/gap/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/pan/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/rsp/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/main/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/hfp_client/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/hfp_ag/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/include \
               -I${WORKSPACE}/qcom-opensource/bt/gatt/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/sdp_client/include \
               -I${WORKSPACE}/system/media/audio/include \
               -I${WORKSPACE}/hardware/qcom/audio/qahw_api/inc \
               -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/hal/include \
               -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/vhal/include \
               -I${WORKSPACE}/mcm-api/api

if USE_BT_OBEX
AM_CPPFLAGS += -DOI_CPU_TYPE=19 \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/pbap_client/include \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/opp/include \
               -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/include \
               -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/include/profiles \
               -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/profiles/obex \
               -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/sdk/include \
               -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/sdk/include/profiles \
               -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/sdk/oem
endif

if AUDIO_HAL_SUPPORTED
AM_CPPFLAGS += -DBT_AUDIO_HAL_INTEGRATION
endif

c_sources = ../utils/src/utils.c \
            src/Main.cpp \
            src/ipc.c \
            ../gap/src/Gap.cpp \
            ../gap/src/AdapterProperties.cpp \
            ../gap/src/RemoteDevices.cpp \
            ../audio_manager/src/Audio_Manager.cpp \
            ../a2dp_sink/src/A2dp_Sink.cpp \
            ../a2dp_sink/src/A2dp_Sink_Streaming.cpp \
            ../avrcp/src/Avrcp.cpp \
            ../a2dp_src/src/A2dp_Src.cpp \
            ../hfp_client/src/HfpClient.cpp \
            ../hfp_ag/src/HfpAG.cpp \
            ../pan/src/Pan.cpp \
            ../sdp_client/src/SdpClient.cpp\
            ../rsp/src/Rsp.cpp

if USE_BT_OBEX
c_sources +=  ../pbap_client/src/PbapClient.cpp \
              ../opp/src/Opp.cpp
endif

if USE_GST
requiredlibs = ../osi/libgstbtapposi.la -lpthread -lm -ldl -lrt -lhardware
else
requiredlibs = ../osi/libbtapposi.la -lpthread -lm -ldl -lrt -lhardware
endif

requiredlibs += $(SYS_LIB)/libgengatt.la
if AUDIO_HAL_SUPPORTED
requiredlibs += $(SYS_LIB)/libqahw.la
endif

if USE_BT_OBEX
requiredlibs += $(SYS_LIB)/libbtobex.la
endif

if USE_GLIB

if USE_GST
 gstbtapp_CFLAGS  = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
 gstbtapp_CPPFLAGS  = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
 gstbtapp_LDFLAGS = $(requiredlibs) @GLIB_LIBS@
 gstbtapp_CPPFLAGS += $(GSTREAMER_CFLAGS) -DUSE_GST
else
 btapp_CFLAGS  = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
 btapp_CPPFLAGS  = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
 btapp_LDFLAGS = $(requiredlibs) @GLIB_LIBS@
endif

else

if USE_GST
 gstbtapp_CFLAGS = $(AM_CFLAGS)
 gstbtapp_LDFLAGS = $(requiredlibs)
else
 btapp_CFLAGS = $(AM_CFLAGS)
 btapp_LDFLAGS = $(requiredlibs)
endif

endif

if USE_GST
gstbtapp_CFLAGS += $(GSTREAMER_CFLAGS)
gstbtapp_LDFLAGS += $(GSTREAMER_LIBS) -lgstaudio-1.0 -lgstapp-1.0 -lgstbthelper
endif

AM_LDFLAGS = $(LDFLAFGS)

if USE_GST
bin_PROGRAMS = gstbtapp
gstbtapp_CC = @CC@
gstbtapp_SOURCES = $(c_sources)
else
bin_PROGRAMS = btapp
btapp_CC = @CC@
btapp_SOURCES = $(c_sources)
endif
pkgconfigdir = $(libdir)/pkgconfig
