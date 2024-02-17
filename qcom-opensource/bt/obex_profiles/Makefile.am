AM_CPPFLAGS = -O2 \
              -D_GNU_SOURCE

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -DLINUX \
        -DOI_CPU_TYPE=19 \
        -DOI_DEBUG \
        -DOI_USE_NATIVE_MALLOC \
        -DOI_USE_NATIVE_MEMCPY \
        -DOI_OBEX_OVER_L2CAP \
        -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/hal/include/ \
        -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/vhal/include \
        -I${WORKSPACE}/system/core/include/ \
        -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/include \
        -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/include/profiles \
        -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/sdk/include \
        -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/sdk/include/profiles \
        -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/sdk/oem \
        -I${WORKSPACE}/qcom-opensource/bt/obex_profiles/platform/linux/include

ACLOCAL_AMFLAGS = -I m4

c_sources = profiles/obex/obexauth.c \
            profiles/obex/obexcommon.c \
            profiles/obex/obexcli.c \
            profiles/obex/obexsrv.c \
            profiles/obex/obex_lower.c \
            profiles/obex/obex_lower_socket.c \
            profiles/obex/obextest.c \
            profiles/pbap/pbap_client.c \
            profiles/pbap/pbap_server.c \
            profiles/pbap/pbap_private.c \
            profiles/opp/opp_client.c \
            profiles/opp/opp_server.c \
            profiles/opp/opp_common.c \
            stack/support/md5_hash.c \
            stack/support/memprof.c \
            stack/support/oi_handle.c \
            stack/support/oi_mbuf.c \
            stack/support/oi_time.c \
            stack/support/oi_list.c \
            stack/support/oi_init_flags.c \
            stack/support/oi_config_table.c \
            stack/support/oi_obextext.c \
            stack/support/oi_statustext.c \
            stack/support/oi_unicode.c \
            stack/support/oi_memmgr.c \
            stack/support/oi_debug.c \
            stack/support/oi_dispatch.c \
            stack/support/oi_fcs.c \
            stack/support/oi_varstring.c \
            stack/support/oi_utils.c \
            stack/support/oi_simplemap.c \
            stack/support/bt_assigned_nos.c \
            stack/dataelem/dataelem.c \
            stack/dataelem/dataelem_text.c \
            platform/linux/src/oi_osinterface.c \
            platform/linux/src/oi_assert.c \
            platform/linux/src/oi_wrapper.c \
            platform/linux/src/oi_thread.c \
            platform/linux/src/sockets/oi_eventloop.c \
            sdk/oem/oi_bt_stack_init_default.c \
            sdk/oem/oi_bt_profile_config_default.c \
            sdk/oem/oi_bt_stack_config_default.c \
            unicode/ConvertUTF.c

AM_LDFLAGS := $(LDFLAFGS)

library_includedir = $(pkgincludedir)
lib_LTLIBRARIES = libbtobex.la
libbtobex_la_CC = @CC@
libbtobex_la_SOURCES = $(c_sources)
libbtobex_la_CPPFLAGS = $(AM_CPPFLAGS)
libbtobex_la_CFLAGS = $(AM_CFLAGS)
libbtobex_la_LDFLAGS = $(AM_LDFLAGS) -static -lpthread -lm -ldl -lrt
