/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "bt_app"

#ifdef USE_ANDROID_LOGGING
#include <utils/Log.h>
#define LOG_TAG "bt_app"
#define LOG_DEBUG ALOGD
#define LOG_ERROR ALOGE
#else
#include <syslog.h>
#define ALOGV(fmt, arg...) syslog (LOG_WARNING, fmt, ##arg)
#define ALOGD(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#define ALOGI(fmt, arg...) syslog (LOG_INFO, fmt, ##arg)
#define ALOGW(fmt, arg...) syslog (LOG_WARNING, fmt, ##arg)
#define ALOGE(fmt, arg...) syslog (LOG_ERR, fmt, ##arg)

#define LOG_DEBUG(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#define LOG_ERROR(fmt, arg...) syslog (LOG_ERR, fmt, ##arg)
#endif

#ifdef __cplusplus
}
#endif
