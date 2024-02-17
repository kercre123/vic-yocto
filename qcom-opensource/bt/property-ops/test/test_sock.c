/******************************************************************************
 *
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 *****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/property_ops.h"
#define SOCKETNAME  "/data/misc/bluetooth/btprop"
#define STDIN       0
#define STDOUT       1
#define MAX_LEN     (1024)


 int
 main(void)
{
    int sock;
    int len;
    int bytes_read = 0  ; //initialized to EOF
    int descriptor_ready = -1;
    struct sockaddr_un sock_un;
    char buf[MAX_LEN];
    fd_set sock_set;
	


    if( (sock = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0){
        LOG_DEBUG("%sock creation error string  =(%s)\n",__func__,
                strerror(errno));
        exit(1);
    }

    //Init the unix socket
    memset(&sock_un, 0, sizeof(struct sockaddr_un));
    sock_un.sun_family = AF_UNIX; //UNIX Socket
    strncpy(sock_un.sun_path, SOCKETNAME, strlen(SOCKETNAME));
    len = sizeof(sock_un.sun_family) + strlen(sock_un.sun_path);


    /*Connect to server socket .*/
    if (connect(sock, (struct sockaddr *) &sock_un, len) < 0){
        LOG_DEBUG("%sock connect error string  =(%s)\n",__func__,
                strerror(errno));
        exit(1);
    }


    while(1){
        FD_ZERO(&sock_set);
        FD_SET(sock,&sock_set);
        FD_SET(STDIN,&sock_set);

        descriptor_ready = select(sock+1 , &sock_set, (fd_set *) 0, (fd_set *) 0,
                (struct timeval *) 0);

        if (descriptor_ready == -1)
        {
            LOG_DEBUG("%s Select Failed error =(%s)\n",__func__,strerror(errno));
            exit(0);
        }
        else if(descriptor_ready) //Data available
        {

            if( FD_ISSET(sock, &sock_set))
            {
                bytes_read = recv(sock, buf, sizeof(buf), 0);
                if(bytes_read < 1){ // 0 means EOF
                    close(sock);
                    exit(0);
                }
                write(STDOUT, buf, bytes_read);
            }

            if( FD_ISSET(STDIN, &sock_set))
            {
                bytes_read = read(STDIN, buf, sizeof(buf));
                if(bytes_read < 1){ // 0 means EOF
                    close(sock);
                    exit(0);
                }
                send( sock, buf, bytes_read, 0);//Send user data back to Socket
            }
        }
    }

}

