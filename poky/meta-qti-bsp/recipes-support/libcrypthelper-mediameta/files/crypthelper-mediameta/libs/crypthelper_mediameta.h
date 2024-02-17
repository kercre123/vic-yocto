/* #########################################################################
   ##  Header File provides APIs allowing the one-to-one mapping  between ##
   ##  the encryptable partitons and their metadata.                      ##
   #########################################################################
*/
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


#include "mntent.h"
#include "stdlib.h"
#include "string.h"


/************************************************************************************
 * Return a handle for the configuration file with the encrytion-meta and
 * mount configuration for a partition on disk.
 * The provided handle after it is successfully created is expected to be
 * maintained by the caller.
 * On error, the function returns an error code less than zero.
*************************************************************************************/
int cryptfs_init_crypt_info_handle();




/*************************************************************************************
 * Call to this API with the handle returned by cryptfs_init_crypt_info_handle
 * will return mount and encryptionmeta information about the encryptable
 * partitions.
 * Call has to be made repeatedly to read the entries sequentially.
 * On error   : returns NULL, does not populate the “key_loc”
 *              or “real_blk_device”
 * On success : returns mntent structure as defined in mntent.h containing the mount
 * info of the block device.
 * handle  : is the handle returned by cryptfs_init_crypt_info_handle
 * key_loc : on successful return, it is populated with a string identifying
 *           the location of metadata about the encryptable partition.
 *           It can take two values:  "footer" and  "header"
 *           footer : Corresponds to the last 128KB of the block device not
 *                    overlapping with filesystem and reserved for the metadata.
 *           header : Corresponds to a raw block device soley dedicated for metadata.
 * real_blk_device : on successful return is populated with string that indicates on
 *           which block device meta data lives.
 * The “key_loc” and “real_blk_device” strings must be freed by the client using the API.
****************************************************************************************/
struct mntent* crtypfs_get_crypt_info(int handle, char **key_loc, char **real_blk_device);




/***************************************************************************************
 * Release the handle obtained by the client. Call to this API destroys the handle.
 * Returns zero on success.
 * Returns an error code less than zero on failure.
***************************************************************************************/
int cryptfs_release_crypt_info_handle(int handle);

