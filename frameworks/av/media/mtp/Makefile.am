ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = mtpserver

mtpserver_CPPFLAGS = -Wno-all -Wno-error -Wno-unused-parameter -g -O2
mtpserver_CPPFLAGS += -I$(top_srcdir)/media/mtp
mtpserver_CPPFLAGS += -I$(WORKSPACE)/system/core/include
mtpserver_CPPFLAGS += -DMTP_DEVICE
mtpserver_CPPFLAGS += -DLINUX_ENABLED
mtpserver_CPPFLAGS += -DHAVE_PTHREADS
mtpserver_CPPFLAGS += -DANDROID_SMP=0
mtpserver_CPPFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1

mtpserver_LDADD = -lpthread -lutils -lcutils

# source to compile
mtpserver_SOURCES = MtpDebug.cpp \
              MtpDataPacket.cpp \
              MtpDeviceInfo.cpp \
              MtpEventPacket.cpp \
              MtpObjectInfo.cpp \
              MtpPacket.cpp \
              MtpProperty.cpp \
              MtpRequestPacket.cpp \
              MtpResponsePacket.cpp \
              MtpServer.cpp \
              MtpStorage.cpp \
              MtpStorageInfo.cpp \
              MtpStringBuffer.cpp \
              MtpUtils.cpp \
              MtpMain.cpp

