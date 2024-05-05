/*
Copyright (c) 2017, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <libinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev/libevdev.h>
#include <input_helper.h>

static int open_restricted(const char *path, int flags, void *user_data);
static void close_restricted(int fd, void *user_data);

struct libinput_interface interface = {
   .open_restricted = open_restricted,
   .close_restricted = close_restricted,
};

static int open_restricted(const char *path, int flags, void *user_data)
{
   int fd = open(path, flags);

   if (fd < 0) {
      fprintf(stderr, "Failed: open() %s (%s)\n", path, strerror(errno));
      return errno;
   }
   return fd;
}

static void close_restricted(int fd, void *user_data)
{
   close(fd);
}

static struct libinput * udev_open(const char *seat, void *userdata)
{
   struct libinput *linput = NULL;
   struct udev *udev = udev_new();

   linput = libinput_udev_create_context(&interface, userdata, udev);

   if (!linput) {
      fprintf(stderr, "Failed: instance creation.\n");
      return NULL;
   }

   if (libinput_udev_assign_seat(linput, seat)) {
       fprintf(stderr, "Failed: libinput seat assignment\n");
       libinput_unref(linput);
       udev_unref(udev);
       linput = NULL;
   }

   udev_unref(udev);
   return linput;
}

static void print_key_event(struct libinput_event *ev)
{
   struct libinput_event_keyboard *kb = libinput_event_get_keyboard_event(ev);
   struct libinput_device *dev = libinput_event_get_device(ev);
   enum libinput_key_state state;
   const char *key_name;

   state = libinput_event_keyboard_get_key_state(kb);
   key_name = libevdev_event_code_get_name(EV_KEY,
                        libinput_event_keyboard_get_key(kb));

   fprintf(stderr, "%s\t%s\n", key_name,
           state == LIBINPUT_KEY_STATE_PRESSED ? "pressed " : "released");
}

static int handle_events(struct libinput *linput,
                         enum libinput_event_type handle_event,
                         input_event_callback_f cb_func)
{
   struct libinput_event *ev;

   libinput_dispatch(linput);
   while ((ev = libinput_get_event(linput))) {
      enum libinput_event_type ev_type = libinput_event_get_type(ev);

      if (handle_event == ev_type) {
            if (cb_func)
               cb_func(ev);
            else
               print_key_event(ev);
      } else {
         fprintf(stderr, "Event (%d) unhandled.\n", ev_type);
      }
      libinput_event_destroy(ev);
      libinput_dispatch(linput);
   }
   return 0;
}

int input_helper_create_instance(input_helper_instance *instance)
{
   struct libinput *linput = NULL;
   char *seat = NULL;
   int ret = -1;

   if (!instance)
      return -1;

   seat = (instance->seat == NULL) ? "seat0" : instance->seat;
   linput = udev_open(seat, instance->userdata);
   instance->linput = linput;

   ret = linput ? 0 : -1;

   return ret;
}

int input_helper_release_instance (input_helper_instance *instance)
{
   if (!instance || !instance->linput)
      return -1;

   libinput_unref(instance->linput);

   return 0;
}

int input_helper_handle_key_events(input_helper_instance *instance)
{
   if (!instance)
      return -1;

   handle_events(instance->linput,
                 LIBINPUT_EVENT_KEYBOARD_KEY,
                 instance->cb_func);
   return 0;
}
