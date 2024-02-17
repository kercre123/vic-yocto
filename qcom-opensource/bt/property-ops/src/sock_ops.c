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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/socket.h>
#include <syslog.h>

#include "../include/property_ops.h"

#define SOCK_NAMED_ADDR  "/data/misc/bluetooth/btprop"
#define TRIGGER_CONNECTION_INDEX    (1)
#define MAX_LISTENERS               (4)
//500ms, unless MAX_LISTENERS are connected
#define INITIAL_POLLING_INTERVAL    (500000)

void *sock_thread_handler (void *param);
bool sock_thread_listener = false;
static pthread_t conn_listener;
static pthread_t io_listener;


struct sockaddr_un un_sock_name;
int listening_fd  = -1; //GLOBAL

typedef struct socket_data {
    int max_fd ;
    int conn_fd[MAX_LISTENERS];
    int connection_counter ;
    fd_set socket_set;
}socket_data;

socket_data sdata;

pthread_mutex_t count_mutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;
pthread_mutex_t io_mutex     = PTHREAD_MUTEX_INITIALIZER;


int init_local_socket(void)
{
    int len ; //Len of the Unix Socket.
    //Create Socket, Bind and then Listen.
    memset(&sdata, 0, sizeof(sdata));

    listening_fd  = -1; //GLOBAL
    unlink(SOCK_NAMED_ADDR);
    listening_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(listening_fd < 0) {
        LOG_DEBUG("Sock creation failed with error (%s)\n", strerror(errno));
        return -1;
    }

    memset(&un_sock_name, 0, sizeof(struct sockaddr_un));

    un_sock_name.sun_family = AF_UNIX;
    strlcpy(un_sock_name.sun_path, SOCK_NAMED_ADDR, sizeof(un_sock_name.sun_path));
    len = sizeof(un_sock_name.sun_family) + strlen(un_sock_name.sun_path);

    /*Bind the socket to the address.*/

    if (bind(listening_fd, (struct sockaddr *) &un_sock_name, len) < 0) {
        LOG_DEBUG("Sock bind failed with error (%s)\n", strerror(errno));
        unlink(SOCK_NAMED_ADDR);
        return -1;
    }

    if (listen(listening_fd,MAX_LISTENERS) < 0) {
        LOG_DEBUG("Listening failed with error (%s)\n", strerror(errno));
        close(listening_fd);
        return -1;
    }
    //All OK here , return Valid Listening Fd
    return listening_fd;
}

void *sock_thread_handler (void *param)
{
    int last_fd = -1;

    LOG_DEBUG(" Thread Listener Started \n");
    if (init_local_socket() == -1) {
        LOG_DEBUG("Listening socket failed return \n");
        return NULL;
    }

    //By now we should have got the listening fd.
    for(;;)
    {
        /* Accept a connection.*/
        int len = sizeof(un_sock_name.sun_family) +
            strlen(un_sock_name.sun_path);

        if ((sdata.conn_fd[sdata.connection_counter] =
                    accept(listening_fd,(struct sockaddr *)&un_sock_name,&len))< 0)
        {
            LOG_DEBUG("Failed to accept with err (%s)\n", strerror(errno));
            pthread_mutex_lock(&count_mutex);
            sdata.connection_counter = 0;
            LOG_DEBUG("Signalling the Select thread to release \n");
            pthread_cond_signal(&condition_var);
            pthread_mutex_unlock(&count_mutex);
            return NULL;
        } else {
            pthread_mutex_lock(&count_mutex);

            int counter = sdata.connection_counter ;
            sdata.max_fd =(last_fd >
                    sdata.conn_fd[counter] ?  last_fd :
                    sdata.conn_fd[counter])+ 1;
            LOG_DEBUG("Connection done with fd (%d), sdata.max_fd =(%d) \
                    conn counter =(%d) \n",sdata.conn_fd, sdata.max_fd,\
                    sdata.connection_counter );

            last_fd = sdata.conn_fd[sdata.connection_counter];

            if (++sdata.connection_counter == TRIGGER_CONNECTION_INDEX)
                //atleast one is connected
            {
                //Signal the condition mutex to start the IO thread
                LOG_DEBUG("Signalling the Select thread \n");
                pthread_cond_signal(&condition_var);
            }
            pthread_mutex_unlock(&count_mutex);

            if (sdata.connection_counter < MAX_LISTENERS)
                continue; // Go back and listen to next connections
            else
            {
                LOG_DEBUG("Connection done with all clients, proceed select\n");
                break;
            }
        }
    }
    LOG_DEBUG(" Thread Listener Exited \n");
    return NULL;
}

void cleanup_listening_socket()
{
    /* TODO : Check if we should clean this conn socket too ?
    for (x = 0; x < sdata.connection_counter ; x++){
        shutdown(sdata.conn_fd[x]);
        close(sdata.conn_fd[x]);
   }*/
    shutdown(listening_fd, SHUT_RDWR);
    close(listening_fd);
    LOG_DEBUG(" Cleanup Listening Socket\n");
}

int add_fd_to_set()
{
    int counter = 0;
    int max = 0;
    int zerocount = -1;
    int iter;

    counter = sdata.connection_counter;
    FD_ZERO(&sdata.socket_set);
    zerocount = 0;
    for (iter = 0; iter < counter ; iter++)
    {
        if (sdata.conn_fd[iter] > 0)
        {
            zerocount --;
            //LOG_DEBUG("++zerocount =(%d)iter (%d)\n", zerocount,iter);
            FD_SET(sdata.conn_fd[iter],&sdata.socket_set);
        }
        else if (sdata.conn_fd[iter] == 0) {
            zerocount ++;
            //LOG_DEBUG("*zerocount =(%d)iter (%d)\n", zerocount,iter);
        }
        if (sdata.conn_fd[iter] > max)
            max = sdata.conn_fd[iter] ;
    }
    if(zerocount == counter || zerocount == MAX_LISTENERS)
    {
        LOG_DEBUG("Listeners Released \n");
        cleanup_listening_socket();
        return -1;
    }
    return max ;
}

bool parse_and_exec(int cmd, unsigned char* command, unsigned char* prop_val)
{
    char delimiter[] = " ";
    char *ptr1;
    char *token ;
    int iter = 0;
    bool result = false;
    char prop_name[MAX_ALLOWED_LINE_LEN];
    char prop_value[MAX_ALLOWED_LINE_LEN];
    char parsed_data[MAX_PROPERTY_ITER][MAX_ALLOWED_LINE_LEN] = {};

    LOG_DEBUG("Command Received (%s)\n", command);
    memset(parsed_data, 0, sizeof(parsed_data));
    memset(prop_value, 0, sizeof(prop_value));
    memset(prop_name, 0, sizeof(prop_name));

    token = strtok_r(command, delimiter, &ptr1);
    while (token) {
        LOG_DEBUG("(%s) \n", token);
        strlcpy(parsed_data[iter ++], token, (strlen(token) + 1));
        token = strtok_r(NULL, delimiter, &ptr1);
    }

    if (cmd == GET_PROP_VALUE) {
        strlcpy(prop_name,parsed_data[1], (strlen(parsed_data[1]) + 1));
        result = get_property_value_bt(prop_name, prop_val);
    } else if (cmd == SET_PROP_VALUE) {
        strlcpy(prop_name,parsed_data[1], (strlen(parsed_data[1]) + 1));
        strlcpy(prop_value,parsed_data[2], (strlen(parsed_data[2]) + 1));
        LOG_DEBUG("Set Prop name (%s) Prop Val (%s)\n", prop_name, prop_value);
        if (!strncmp(prop_name, "wc_transport.start_hci", strlen(prop_name))) {
            if (!strncmp(prop_value, "true", strlen(prop_value))) {
                LOG_DEBUG("starting wcnssfilter\n");
                system("wcnssfilter &");
            }
        }
        if (!strncmp(prop_name, "bluetooth.startbtsnoop", strlen(prop_name))) {
            if (!strncmp(prop_value, "true", strlen(prop_value))) {
                LOG_DEBUG("starting btsnoop\n");
                system("btsnoop &");
            } else {
                LOG_DEBUG("stopping btsnoop\n");
                system("killall -KILL btsnoop");
            }
        }

        if (!strncmp(prop_name, "bluetooth.isEnabled", strlen(prop_name))) {
            if (!strncmp(prop_value, "true", strlen(prop_value))) {
                LOG_DEBUG("starting abtfilter\n");
                system("abtfilt -d -z -n -m -a -w wlan0 &");
            } else {
                LOG_DEBUG("stopping abtfilter\n");
                system("killall -KILL abtfilt");
            }
        }
        result = set_property_value_bt(prop_name, prop_value);
    }

    dump_current_ds();

    return result;

}

char prop_string[MAX_ALLOWED_LINE_LEN][MAX_ALLOWED_LINE_LEN];

int separate_recvd_commands(unsigned char * command)

{
    char delimiter[] = ",";
    char *ptr1;
    /* read if buffer has more than one properties */
    int iter = 0;
    int i = 0;
    char *token ;

    LOG_DEBUG("Command Received (%s)\n", command);
    memset(prop_string, 0, sizeof(prop_string));

    token = strtok_r(command, delimiter, &ptr1);
    while (token)
    {
        LOG_DEBUG("(%s) \n", token);
        strlcpy(prop_string[iter ++], token, (strlen(token) + 1));
        token = strtok_r(NULL, delimiter, &ptr1);
    }

    LOG_DEBUG("Found multiple commands (%d) \n", iter);
    return iter;
}

bool issue_property_command(unsigned char *command, unsigned char *prop_val)
{
    bool retval = false;

    LOG_DEBUG("command string (%s) \n", command);

    if(strstr(command, "get_property")!= NULL) {
        LOG_DEBUG("Command is get \n");
        retval = parse_and_exec(GET_PROP_VALUE, command, prop_val);
        prop_val[strlen(prop_val)] = ',';
    } else if(strstr(command, "set_property")!= NULL) {
        LOG_DEBUG("Command is set \n");
        retval = parse_and_exec(SET_PROP_VALUE, command, NULL);
    } else{
        LOG_DEBUG("Wrong command \n");
        return false;
    }
    return retval;
}

void *io_thread_handler (void *param)
{
    int descriptor_set = -1;
    int bytes_available = 0;
    int selectloop =0;
    unsigned char buf[1024];
    int counter = 0;
    int max = 0;
    int i = 0;
    int no_of_separate_strings = 0;
    struct timeval to;
    bool result = false;
    char prop_val[MAX_ALLOWED_LINE_LEN];

    //Atleast let 1 Listener start and then we can keep adding
    //and polling on the others

    pthread_mutex_lock(&count_mutex);
    pthread_cond_wait(&condition_var, &count_mutex);
    if (sdata.connection_counter == 0)
    {
        LOG_DEBUG("Exiting from iothread on zero count\n");
        //in this case it should a abnormal signaling on 1st accept failure.
        pthread_mutex_unlock(&count_mutex);
        return NULL;
    }
    pthread_mutex_unlock(&count_mutex);

    for(;;)
    {
        pthread_mutex_lock(&io_mutex);
        counter = sdata.connection_counter;
        for (selectloop=0; selectloop< MAX_LISTENERS; selectloop++)
        {
            max = add_fd_to_set();
            if (max == -1) { //Exit since all listeners are closed
                pthread_mutex_unlock(&io_mutex);
                return NULL;
            }

            to.tv_sec = 0;
            to.tv_usec = INITIAL_POLLING_INTERVAL;

            /* Wait for some data from remote end. */
            //LOG_DEBUG(" \nWaiting on Select Max(%d),counter(%d)\n",max,counter);
            descriptor_set = select(max+1, &sdata.socket_set,NULL, NULL,
                             (counter == MAX_LISTENERS)? NULL: &to);

            if (descriptor_set == -1)
            {
                LOG_DEBUG("%s Select Failed error =(%s)\n",__func__,
                        strerror(errno));
                cleanup_listening_socket();
                pthread_mutex_unlock(&io_mutex);
                return NULL;
            }
            else if(descriptor_set) //Data available
            {
                if(FD_ISSET(sdata.conn_fd[selectloop], &sdata.socket_set))
                {
                    //LOG_DEBUG("Data set available on (%d)\n",selectloop);
                    memset(&buf[0], 0, sizeof(buf));

                    bytes_available = recv(sdata.conn_fd[selectloop], buf,
                            sizeof(buf), 0);

                    if(bytes_available == 0) {
                       /* LOG_DEBUG("I/O Done EOF counter=(%d)selectloop =(%d)\n",
                                counter, selectloop);*/
                        shutdown(sdata.conn_fd[selectloop], SHUT_RDWR);
                        close(sdata.conn_fd[selectloop]);
                        sdata.conn_fd[selectloop] = 0;
                    } else {
                        //LOG_DEBUG("I/O data on fd, at index(%d)=%s\n",
                        //        selectloop,buf);
                        /* Read out the property Value */
                        memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);

                        no_of_separate_strings = separate_recvd_commands(buf);
                        if (no_of_separate_strings == 1)
                        {
                            result = issue_property_command(buf, prop_val);
                            if (strlen(prop_val) ) {
                                LOG_DEBUG("I/O thread - sending single response (%s), len = %d\n",
                                    prop_val, strlen(prop_val));
                                send(sdata.conn_fd[selectloop], prop_val, strlen(prop_val), 0);
                            }
                        } else {
                            for (i = 0; i < no_of_separate_strings; i ++)
                            {
                                result = issue_property_command(prop_string[i], prop_val);
                                if (strlen(prop_val) ) {
                                    LOG_DEBUG("I/O thread - sending (%d) of (%d) response (%s),"
                                        "len = %d\n", i + 1, no_of_separate_strings, prop_val,
                                        strlen(prop_val));
                                    send(sdata.conn_fd[selectloop], prop_val, strlen(prop_val), 0);
                                }
                            }
                        }
                    }
                }
            } else {
                //LOG_DEBUG("Data Not Set on (%d)\n", selectloop);
            }
        }
        pthread_mutex_unlock(&io_mutex);
    }
    LOG_DEBUG(" I/O Thread Listener Exited \n");
    return NULL;
}

bool start_listeners() {
    /* Start the Socket Accept Threads */
    int err = pthread_create(&conn_listener, NULL, sock_thread_handler, NULL);
    if (err < 0) {
        LOG_DEBUG("%s Sock thread creation %s\n", __func__, strerror(errno));
        return false;
    } else {
        LOG_DEBUG("conn_listener Thread Initialized for Listening\n");
    }

    err = pthread_create(&io_listener, NULL, io_thread_handler, NULL);
    if (err <0 ) {
        LOG_DEBUG("%s Sock thread creation %s\n", __func__, strerror(errno));
        return false;
    } else {
        LOG_DEBUG("io_listener Thread Initialized for Listening\n");
    }
    return true;
}

int main()
{
    void **retval;
    bool result = false;

    /* Database Initializer */
    result = create_node_from_persist(path);
    openlog ("bt_property", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    if (result ==true)
    {
        LOG_DEBUG("List Should be created by now head = %0x\n",
                __get_list_head());
        dump_current_ds();
    } else {
        LOG_DEBUG("List Pull Failure\n");
    }

    LOG_DEBUG("\n SOCKET INITIALIZER \n");
    //Init Socket Listener inside a thread
    for (;;) //This is a main Daemon - to be started from Init before usage.
    {
        if( !start_listeners()) {
            LOG_DEBUG("%s Listener thread not created something wrong: return\n",
                    __func__);
        } else {
            LOG_DEBUG("%s Listeners Started\n ", __func__);
        }
        pthread_join(conn_listener, retval);
        LOG_DEBUG("%s conn_listener closed\n ", __func__);
        pthread_join(io_listener, retval);
        LOG_DEBUG("%s io_listener closed\n ", __func__);
    }
    closelog ();
}


