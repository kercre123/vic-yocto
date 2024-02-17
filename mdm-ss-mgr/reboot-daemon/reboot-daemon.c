/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> //Fix clang build errors.

/* magic string to reboot upon reading */
#define REBOOT_STR "REBOOT"
#define EDL_REBOOT_STR "EDL-REBOOT"

/* size to make read buffer */
#define MAX_BUF 64
/* name of named pipe to read from */
#define FIFO_NAME "/dev/rebooterdev"
/* owning user of the named pipe */
#define UID 0
/* owning group of the named pipe */
#define GID 1301

#ifdef DEBUG
#define debug_printf(args...) fprintf(stderr,"DEBUG: " args)
#else
#define debug_printf(args...) ;
#endif

int main() {
    int pipe_fd; // file descriptor for fifo
    char buf[MAX_BUF]; // buffer to store read data in
    int read_count; //numbef of characters read

    /* delete the fifo if it already exists */
    debug_printf("Unlinking\n");
    if(unlink(FIFO_NAME) == -1) {
        if (errno != ENOENT) {
            debug_printf("unlink failed: %s\n", strerror(errno));
        }
        /* if the file does not exist to be deleted, that is ok */
    }

    /* create fifo with rw for owner and group */
    debug_printf("mknodding\n");
    if(mknod(FIFO_NAME, S_IFIFO | 0660,0) == -1) {
        debug_printf("mknod failed: %s\n", strerror(errno));
        return 1;
    }

    /* set fifo to user rw, group w */
    debug_printf("chmodding\n");
    if(chmod(FIFO_NAME, 0620) == -1) {
        debug_printf("chmod failed: %s\n", strerror(errno));
        return 1;
    }

    /* change fifo owner to rebooters group */
    debug_printf("chowning\n");
    if(chown(FIFO_NAME,UID,GID) == -1) {
        debug_printf("chown failed: %s\n", strerror(errno));
        return 1;
    }

    /* read and open forever */
    while(1) {

        /* open fifo for reading */
        debug_printf("opening\n");
        pipe_fd = open(FIFO_NAME, O_RDONLY);
        if(pipe_fd == -1) {
            debug_printf("open failed: %s\n", strerror(errno));
            return 1;
        }

        /* read until an error or reboot */
        while(1) {

            /* reset the buffer to prevent matching multiple times */
            memset(buf,0,MAX_BUF);

            /* blocking read from fifo */
            debug_printf("about to read\n");
            read_count = read(pipe_fd,buf,MAX_BUF-1);
            if(read_count <= 0) {
                debug_printf("read failed: %s\n", strerror(errno));
                break;
            }

            debug_printf("read: read_count %d, read: %s\n",read_count,buf);

            /* if read REBOOT_STR, then call reboot 
               if read EDL_REBOOT_STR, then call edl-reboot */
            if(strncmp(buf,REBOOT_STR,strlen(REBOOT_STR)) == 0) {
                debug_printf("going for reboot\n");
                printf("reboot-daemon: initiating reboot\n");
                system("reboot");
                break;
            } else if(strncmp(buf,EDL_REBOOT_STR,strlen(EDL_REBOOT_STR)) == 0) {
                debug_printf("going for edl-reboot\n");
                printf("reboot-daemon: initiating edl-reboot\n");
                system("sys_reboot edl");
                break;
            }

        }

        /* close FIFO */
        debug_printf("closing fifo\n");
        close(pipe_fd);
    }
}
