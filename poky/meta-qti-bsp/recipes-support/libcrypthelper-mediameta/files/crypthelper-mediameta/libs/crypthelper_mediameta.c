/*

Copyright (c) 2018, The Linux Foundation. All rights reserved.

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

#include <crypthelper_mediameta.h>
#include <mntent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#define MAPPER_CONF_FILE ("/etc/conf/media-encryption.conf")

/*
*  Checks if the file descriptor has reached EOF.
*/
static int check_eof(int handle){
    char c;
    if(read(handle, &c, 1) == 0) {
        return 1;
    }
    lseek(handle, -1, SEEK_CUR);
    return 0;
}


/*
*  Reads a line at which file descriptor handle starts into buffer and stores the line
*  length in count.
*  Returns count of bytes read successfully.
*  Returns 0 when EOF is read.
*  Returns -1 on event of an error after printing error.
*/
static int read_line(int handle, char **buff, size_t* count){
    char c;
    int read_bytes;
    *count = 0;
    off_t start_offset = lseek(handle, 0, SEEK_CUR);
    int is_eof = 0;

    while(!(is_eof = check_eof(handle))) {
        read(handle, &c, 1);
        if(c == '\n' || c == '\r') {
            break;
        }
        *count+=1;
    }

    if (is_eof) {
        *buff = NULL;
        *count = 0;
        read_bytes = 0;
    } else {
        lseek(handle, start_offset, SEEK_SET);
        int buffsize = (int) *count + 1;
        *buff = (char *)malloc(sizeof(char) * buffsize);
        memset(*buff, 0, buffsize);
        read_bytes = read(handle, *buff, *count);
        if ( read_bytes == -1) {
            printf("%s: %s\n", __func__, strerror(errno));
            return -1;
        }
    }

    return read_bytes;
}


/*
*  Points the file descriptor handle to  nextline.
*/
static void next_line(int handle){
    char c;
    while(read(handle, &c, 1)!= 0) {
        if(c !='\n' && c!= '\r') {
            lseek(handle, -1, SEEK_CUR);
            break;
        }
    }
}


/*
*  Tokenizes the input str with any character in delim and stores the result at pointer in d.
*/
static void tokenize_assign(char *str, const char* delim, char **d) {
    *d = strtok(str, delim);
}


int cryptfs_init_crypt_info_handle(){
   int fd = open(MAPPER_CONF_FILE, O_RDONLY);
   if(fd == -1) {
        return -errno;
   }
   return fd;
}



struct mntent* crtypfs_get_crypt_info (int handle, char** key_loc, char** real_blk_device) {
    char* buff;
    char* delim = " \t";
    size_t line_len = 0;
    struct mntent* mountinfo = NULL;
    int ret = read_line(handle, &buff, &line_len);

    if(0 == ret) {
        *key_loc = 0;
        *real_blk_device = 0;
        mountinfo = NULL;
    } else {
        next_line(handle);

        mountinfo = (struct mntent*) malloc(sizeof(struct mntent));
        if(mountinfo) {
            mountinfo->mnt_freq = 0;
            mountinfo->mnt_passno = 2;
            tokenize_assign(buff, delim, &mountinfo->mnt_fsname);
            tokenize_assign(NULL, delim, &mountinfo->mnt_dir);
            tokenize_assign(NULL, delim, &mountinfo->mnt_type);
            tokenize_assign(NULL, delim, &mountinfo->mnt_opts);
            tokenize_assign(NULL, delim, key_loc);
            tokenize_assign(NULL, delim, real_blk_device);
        } else {
           *key_loc = 0;
           *real_blk_device = 0;
           mountinfo = NULL;
        }
    }

    return mountinfo;
}


int cryptfs_release_crypt_info_handle(int handle){
    if( close(handle) == -1){
        return -errno;
    }
    return 0;
}
