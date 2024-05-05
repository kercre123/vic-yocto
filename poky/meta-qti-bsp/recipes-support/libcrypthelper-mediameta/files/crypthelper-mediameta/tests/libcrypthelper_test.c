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
#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int main(int c, char* argv[] ){
    int handle;
    char *key_loc;
    char *real_blk_device;
    struct mntent* mountinfo = NULL;
    int i = 1;

    handle = cryptfs_init_crypt_info_handle();
    if(handle < 0){
        printf("Error opening the file %s", strerror((-handle)));
        exit(1);
    }

    do {
        printf("=================== Entry # %d =================\n", i++);
        mountinfo = crtypfs_get_crypt_info(handle,&key_loc,&real_blk_device);
        if(mountinfo) {
            printf("     ======== DBG ======== \n       %s  %s  %s  %s  %d  %d   %s   %s\n",
                    mountinfo->mnt_fsname,mountinfo->mnt_dir,mountinfo->mnt_type,mountinfo->mnt_opts,mountinfo->mnt_freq,mountinfo->mnt_passno,
                    key_loc, real_blk_device);
            free(mountinfo);
        }
     } while(mountinfo != NULL);

    cryptfs_release_crypt_info_handle(handle);
    exit(0);
}

