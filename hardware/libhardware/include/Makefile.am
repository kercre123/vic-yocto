pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ../hardware.pc
EXTRA_DIST = $(pkgconfig_DATA)

h_sources = hardware/audio.h \
            hardware/hardware.h \
            hardware/audio_effect.h \
            hardware/sound_trigger.h

library_includedir = $(includedir)/hardware/
library_include_HEADERS = $(h_sources)
