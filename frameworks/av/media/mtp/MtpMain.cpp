/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utils/String16.h>

#include "mtp.h"
#include "MtpServer.h"
#include "MtpStorage.h"
#include "MtpDatabase.h"
#include "MtpStringBuffer.h"
#include "MtpObjectInfo.h"
#include "MtpProperty.h"
#include "MtpDebug.h"
#include "MtpTypes.h"

#include <map>
#include <stdexcept>

#define EXTERNAL_STORAGE_PATH "/mnt/sdcard"
#define EXTERNAL_STORAGE_DESC "sdcard"
#define INTERNAL_STORAGE_PATH "/media/internal"
#define INTERNAL_STORAGE_DESC "Internal storage"
#define DEVICE_FRIENDLY_NAME "MTP"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static void* inotifyWatchEntry(void *obj);

using namespace android;

namespace android
{
struct ObjectInfo
{
    MtpStorageID mStorageId;
    MtpObjectFormat mFormat;
    uint32_t mCompressedSize;
    MtpObjectHandle mParent;
    char* mName; // "/tmp/foo/bar.c" --> "bar.c"
    char* mPath; // "/tmp/foo/bar.c" --> "/tmp/foo/bar.c"
    int mWd; // Inotify watch fd
    time_t mDateModified;
};

class MyMtpDatabase : public MtpDatabase {
private:
    std::map<MtpObjectHandle, ObjectInfo> mObjectDb;
    std::map<MtpString, MtpObjectFormat> mSupportedFormatMap;
    MtpServer* mServer;
    pthread_t mInotifyWatchThread;
    uint32_t mHandleId;
    bool mRunning;
    int mMediaFlag;

    void buildSupportedformatMap();
    time_t getModifiedTime(const MtpString& path);
    MtpObjectFormat getObjectFormatByExtension(const MtpString& extension);
    bool isDirectory(const MtpString& path);
    int64_t getFileSize(const MtpString& path);
    uint32_t getCompressedFileSize(const MtpString& path);
    void addObjectInfo(const MtpString& path, MtpObjectHandle parent, MtpStorageID storage);
    void enumDirectory(const MtpString& path, MtpObjectHandle parent, MtpStorageID storage);

    int checkObjectHandle(MtpObjectHandle handle) {
        if (handle == 0 || handle == MTP_PARENT_ROOT)
            return -1;
        else
            return 0;
    }

public:
    int mInotifyFd; //Monitor file change operations in MTP storage

                                    MyMtpDatabase();
    virtual                         ~MyMtpDatabase();

    virtual MtpObjectHandle         beginSendObject(const char* path,
                                            MtpObjectFormat format,
                                            MtpObjectHandle parent,
                                            MtpStorageID storage,
                                            uint64_t size,
                                            time_t modified);

    virtual void                    endSendObject(const char* path,
                                            MtpObjectHandle handle,
                                            MtpObjectFormat format,
                                            bool succeeded);

    virtual MtpObjectHandleList*    getObjectList(MtpStorageID storageID,
                                    MtpObjectFormat format,
                                    MtpObjectHandle parent);

    virtual int                     getNumObjects(MtpStorageID storageID,
                                            MtpObjectFormat format,
                                            MtpObjectHandle parent);

    // callee should delete[] the results from these
    // results can be NULL
    virtual MtpObjectFormatList*    getSupportedPlaybackFormats();
    virtual MtpObjectFormatList*    getSupportedCaptureFormats();
    virtual MtpObjectPropertyList*  getSupportedObjectProperties(MtpObjectFormat format);
    virtual MtpDevicePropertyList*  getSupportedDeviceProperties();

    virtual MtpResponseCode         getObjectPropertyValue(MtpObjectHandle handle,
                                            MtpObjectProperty property,
                                            MtpDataPacket& packet);

    virtual MtpResponseCode         setObjectPropertyValue(MtpObjectHandle handle,
                                            MtpObjectProperty property,
                                            MtpDataPacket& packet);

    virtual MtpResponseCode         getDevicePropertyValue(MtpDeviceProperty property,
                                            MtpDataPacket& packet);

    virtual MtpResponseCode         setDevicePropertyValue(MtpDeviceProperty property,
                                            MtpDataPacket& packet);

    virtual MtpResponseCode         resetDeviceProperty(MtpDeviceProperty property);

    virtual MtpResponseCode         getObjectPropertyList(MtpObjectHandle handle,
                                            uint32_t format, uint32_t property,
                                            int groupCode, int depth,
                                            MtpDataPacket& packet);

    virtual MtpResponseCode         getObjectInfo(MtpObjectHandle handle,
                                            MtpObjectInfo& info);

    virtual void*                   getThumbnail(MtpObjectHandle handle, size_t& outThumbSize);

    virtual MtpResponseCode         getObjectFilePath(MtpObjectHandle handle,
                                            MtpString& outFilePath,
                                            int64_t& outFileLength,
                                            MtpObjectFormat& outFormat);
    virtual MtpResponseCode         deleteFile(MtpObjectHandle handle);

    bool                            getObjectPropertyInfo(MtpObjectProperty property, int& type);
    bool                            getDevicePropertyInfo(MtpDeviceProperty property, int& type);

    virtual MtpObjectHandleList*    getObjectReferences(MtpObjectHandle handle);

    virtual MtpResponseCode         setObjectReferences(MtpObjectHandle handle,
                                            MtpObjectHandleList* references);

    virtual MtpProperty*            getObjectPropertyDesc(MtpObjectProperty property,
                                            MtpObjectFormat format);

    virtual MtpProperty*            getDevicePropertyDesc(MtpDeviceProperty property);

    virtual void                    sessionStarted();

    virtual void                    sessionEnded();

    void addStorage(const MtpString& storagePath, const MtpString& discription, MtpStorageID storage);
    void removeStorage(MtpStorageID storage);
    void setMtpServer(MtpServer* server);
    void stopInotifyWatchThread();
    void setMediaFlag(bool flag);
    bool needStopInotifyWatchThread();
    void inotifyWatchHandler(const char eventBuf[], size_t transferred);
};

// ----------------------------------------------------------------------------
void MyMtpDatabase::MyMtpDatabase::buildSupportedformatMap() {
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".txt"), MTP_FORMAT_TEXT));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".htm"), MTP_FORMAT_HTML));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".html"), MTP_FORMAT_HTML));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".wav"), MTP_FORMAT_WAV));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".mp3"), MTP_FORMAT_MP3));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".mpeg"), MTP_FORMAT_MPEG));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".wmv"), MTP_FORMAT_WMV));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".avi"), MTP_FORMAT_AVI));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".jpg"), MTP_FORMAT_EXIF_JPEG));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".bmp"), MTP_FORMAT_BMP));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".gif"), MTP_FORMAT_GIF));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".jpeg"), MTP_FORMAT_JFIF));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".png"), MTP_FORMAT_PNG));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".tif"), MTP_FORMAT_TIFF));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".tiff"), MTP_FORMAT_TIFF));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".ogg"), MTP_FORMAT_OGG));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".aac"), MTP_FORMAT_AAC));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".mp4"), MTP_FORMAT_MP4_CONTAINER));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".3gp"), MTP_FORMAT_3GP_CONTAINER));
    mSupportedFormatMap.insert(std::pair<MtpString, MtpObjectFormat>(MtpString(".flac"), MTP_FORMAT_FLAC));
}

time_t MyMtpDatabase::getModifiedTime(const MtpString& path)
{
    struct stat64 attr;
    ::stat64(path.string(), &attr);
    return attr.st_mtime;
}

MtpObjectFormat MyMtpDatabase::getObjectFormatByExtension(const MtpString& extension)
{
    std::map<MtpString, MtpObjectFormat>::iterator iter;

    iter = mSupportedFormatMap.find(extension);
    if (iter == mSupportedFormatMap.end()) {
            return MTP_FORMAT_UNDEFINED;
    }

    return iter->second;
}

bool MyMtpDatabase::isDirectory(const MtpString& path)
{
    struct stat64 attr;
    ::stat64(path.string(), &attr);
    return S_ISDIR(attr.st_mode);
}

int64_t MyMtpDatabase::getFileSize(const MtpString& path)
{
    struct stat64 attr;
    ::stat64(path.string(), &attr);
    return attr.st_size;
}

uint32_t MyMtpDatabase::getCompressedFileSize(const MtpString& path)
{
    struct stat64 attr;
    ::stat64(path.string(), &attr);
    if (attr.st_size > 0xFFFFFFFFLL)
        return 0xFFFFFFFF; //Consistent with Android.
    else
        return attr.st_size;
}

void MyMtpDatabase::addObjectInfo(const MtpString& path, MtpObjectHandle parent, MtpStorageID storage)
{
    ObjectInfo objInfo;
    MtpObjectHandle handle = mHandleId++;

    if (isDirectory(path)) {
        objInfo.mStorageId = storage;
        objInfo.mParent = parent;
        objInfo.mName = ::strdup(path.getPathLeaf());
        objInfo.mPath = ::strdup(path);
        objInfo.mFormat = MTP_FORMAT_ASSOCIATION;
        objInfo.mCompressedSize = 0;
        objInfo.mWd = inotify_add_watch(mInotifyFd, path.string(),
                /*IN_MODIFY | */IN_CREATE | IN_DELETE);
        objInfo.mDateModified = getModifiedTime(path);

        mObjectDb.insert(std::pair<MtpObjectHandle, ObjectInfo>(handle, objInfo) );

        ALOGD("%s: add directory %s\n", __func__, path.string());

        if (mServer)
            mServer->sendObjectAdded(handle);

        enumDirectory(path, handle, storage);
    } else {
        objInfo.mStorageId = storage;
        objInfo.mParent = parent;
        objInfo.mName = ::strdup(path.getPathLeaf());
        objInfo.mPath = ::strdup(path);
        objInfo.mFormat = getObjectFormatByExtension(path.getPathExtension());
        objInfo.mCompressedSize = getCompressedFileSize(path);
        objInfo.mDateModified = getModifiedTime(path);
        mObjectDb.insert( std::pair<MtpObjectHandle, ObjectInfo>(handle, objInfo) );
        ALOGD("%s: add file %s\n", __func__, path.string());

        if (mServer)
            mServer->sendObjectAdded(handle);
    }
}

void MyMtpDatabase::enumDirectory(const MtpString& path, MtpObjectHandle parent, MtpStorageID storage)
{
    ObjectInfo objInfo;
    Vector<MtpString> v;

    DIR *dir;
    struct dirent64 *ent;

    ALOGD("parse_directory %s \n", path.string());
    if (path.string() != NULL && (dir = ::opendir(path.string())) != NULL) {
        while ((ent = ::readdir64(dir)) != NULL) {
            if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' &&
                ent->d_name[2] == '\0')))
                continue;
            MtpString subPath(path);
            subPath.append("/");
            subPath.append(ent->d_name);
            v.push(subPath);
            ALOGD("enumDirectory:readdir %s \n", ent->d_name);
        }
        ::closedir(dir);
    }

    for (Vector<MtpString>::const_iterator iter(v.begin()), iter_end(v.end()); iter != iter_end; ++iter)
    {
        addObjectInfo(*iter, parent, storage);
    }
}

MyMtpDatabase::MyMtpDatabase()
{
    int ret;

    mHandleId = 1;
    mRunning = true;
    mServer = NULL;
    mMediaFlag = 0;

    mObjectDb = std::map<MtpObjectHandle, ObjectInfo>();
    mSupportedFormatMap = std::map<MtpString, MtpObjectFormat>();
    buildSupportedformatMap();

    mInotifyFd = inotify_init();
    if (mInotifyFd <= 0) {
        ALOGE("Fail to inotify_init in %s\n", __func__);
        exit(-1);
    }
    ALOGD("using inotify fd %d\n", mInotifyFd);


    ret = pthread_create(&mInotifyWatchThread, NULL, inotifyWatchEntry, this);
    if (ret < 0) {
        ALOGE("Can't create notifier_thread to monitor files change in target MTP device\n");
        exit(-1);
    }
}

MyMtpDatabase::~MyMtpDatabase() {
    stopInotifyWatchThread();
    close(mInotifyFd);
    pthread_detach(mInotifyWatchThread);

    for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
            iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
        if (mObjectDb.at(iter->first).mName != NULL)
            ::free(mObjectDb.at(iter->first).mName);
        if (mObjectDb.at(iter->first).mPath != NULL)
            ::free(mObjectDb.at(iter->first).mPath);

        mObjectDb.erase(iter->first);
    }
}

MtpObjectHandle MyMtpDatabase::beginSendObject(const char* path,
        MtpObjectFormat format,
        MtpObjectHandle parent,
        MtpStorageID storage,
        uint64_t size,
        time_t modified)
{
    ObjectInfo objInfo;
    MtpObjectHandle handle = mHandleId;
    MtpString pathString(path);

    if (pathString.find(INTERNAL_STORAGE_PATH) == -1 && pathString.find(EXTERNAL_STORAGE_PATH) == -1)
        return kInvalidObjectHandle;
    if (::access(path, F_OK) == 0) {
        ALOGD("%s:path %s exists, return\n", __func__, path);
        return kInvalidObjectHandle;
    }

    ALOGD("%s:path %s,format %d\n", __func__, path, format);

    objInfo.mStorageId = storage;
    objInfo.mParent = parent;
    objInfo.mName = ::strdup(pathString.getPathLeaf());
    objInfo.mPath = ::strdup(path);
    objInfo.mFormat = format;
    objInfo.mCompressedSize = (uint32_t)size;
    objInfo.mDateModified = modified;

    mObjectDb.insert(std::pair<MtpObjectHandle, ObjectInfo>(handle, objInfo));

    mHandleId++;

    return handle;
}

void MyMtpDatabase::endSendObject(const char* path,
        MtpObjectHandle handle,
        MtpObjectFormat format,
        bool succeeded)
{
    ObjectInfo objInfo;
    MtpString pathString(path);
    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return;
    }

    ALOGD("%s: path %s\n", __func__, path);

    if (!succeeded) {
        if (mObjectDb.at(handle).mName != NULL)
            ::free(mObjectDb.at(handle).mName);
        if (mObjectDb.at(handle).mPath != NULL)
            ::free(mObjectDb.at(handle).mPath);

        mObjectDb.erase(handle);
    } else {
        if (format != MTP_FORMAT_ASSOCIATION) {
            mObjectDb.at(handle).mCompressedSize = getCompressedFileSize(pathString);
        }
    }
}

MtpObjectHandleList* MyMtpDatabase::getObjectList(MtpStorageID storageID,
        MtpObjectFormat format,
        MtpObjectHandle parent)
{
    ObjectInfo objInfo;
    MtpObjectHandleList* list = new MtpObjectHandleList;

    if (parent == MTP_PARENT_ROOT)
        parent = 0;

    try {
        if (parent != 0)
            objInfo = mObjectDb.at(parent);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, parent);
        return list;
    }

    ALOGD("%s:storageID %d, format %d, parent %d, mObjectDb size %d\n",
            __func__, storageID, format, parent, mObjectDb.size());

    for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
            iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
        if (iter->second.mStorageId == storageID && iter->second.mParent == parent) {
            if (format == 0 || iter->second.mFormat == format) {
                list->push_back(iter->first);
                ALOGD("%s:handle %d,path %s\n", __func__, iter->first, iter->second.mPath);
            }
        }
    }

    return list;
}

int MyMtpDatabase::getNumObjects(MtpStorageID storageID,
        MtpObjectFormat format,
        MtpObjectHandle parent)
{
    int result = 0;

    ALOGD("%s: storageID %d, format %d, parent %d\n",
            __func__, storageID, format, parent);

    MtpObjectHandleList *list = getObjectList(storageID, format, parent);
    result = list->size();
    delete list;

    return result;
}

MtpObjectFormatList* MyMtpDatabase::getSupportedPlaybackFormats()
{
    static const uint16_t playbackFormatList[] = {
        MTP_FORMAT_UNDEFINED,
        MTP_FORMAT_ASSOCIATION,
        MTP_FORMAT_TEXT,
        MTP_FORMAT_HTML,
        MTP_FORMAT_WAV,
        MTP_FORMAT_MP3,
        MTP_FORMAT_MPEG,
        MTP_FORMAT_EXIF_JPEG,
        MTP_FORMAT_TIFF_EP,
        MTP_FORMAT_BMP,
        MTP_FORMAT_GIF,
        MTP_FORMAT_JFIF,
        MTP_FORMAT_PNG,
        MTP_FORMAT_TIFF,
        MTP_FORMAT_WMA,
        MTP_FORMAT_OGG,
        MTP_FORMAT_AAC,
        MTP_FORMAT_MP4_CONTAINER,
        MTP_FORMAT_3GP_CONTAINER,
        MTP_FORMAT_ABSTRACT_AV_PLAYLIST,
        MTP_FORMAT_FLAC
    };

    MtpObjectFormatList* list = new MtpObjectFormatList();
    list->appendArray(playbackFormatList, ARRAY_SIZE(playbackFormatList));

    ALOGD("%s size %d\n", __func__, list->size());

    return list;
}

MtpObjectFormatList* MyMtpDatabase::getSupportedCaptureFormats()
{
    ALOGD("%s\n", __func__);
    return NULL;
}

MtpObjectPropertyList* MyMtpDatabase::getSupportedObjectProperties(MtpObjectFormat format)
{
    static const uint16_t objectPropertyList[] = {
            MTP_PROPERTY_STORAGE_ID,
            MTP_PROPERTY_OBJECT_FORMAT,
            MTP_PROPERTY_OBJECT_SIZE,
            MTP_PROPERTY_OBJECT_FILE_NAME,
            MTP_PROPERTY_DATE_MODIFIED,
            MTP_PROPERTY_PARENT_OBJECT,
            MTP_PROPERTY_DISPLAY_NAME,
            MTP_PROPERTY_PERSISTENT_UID,
            MTP_PROPERTY_ASSOCIATION_TYPE
    };

    MtpObjectPropertyList* list = new MtpObjectPropertyList();
    list->appendArray(objectPropertyList, ARRAY_SIZE(objectPropertyList));

    ALOGD("%s size %d\n", __func__, list->size());

    return list;
}

MtpDevicePropertyList* MyMtpDatabase::getSupportedDeviceProperties()
{
    static const uint16_t devicePropertyList[] = {
        MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER,
        MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME
    };

    MtpDevicePropertyList* list = new MtpDevicePropertyList();
    list->appendArray(devicePropertyList, ARRAY_SIZE(devicePropertyList));

    ALOGD("%s\n", __func__);

    return list;
}

MtpResponseCode MyMtpDatabase::getObjectPropertyValue(MtpObjectHandle handle,
        MtpObjectProperty property,
        MtpDataPacket& packet)
{
    char date[20];
    ObjectInfo objInfo;

    if (checkObjectHandle(handle) != 0)
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }

    ALOGD("%s: handle %d, path %s, property %s\n", __func__, handle,
            mObjectDb.at(handle).mPath, MtpDebug::getObjectPropCodeName(property));

    switch(property)
    {
        case MTP_PROPERTY_STORAGE_ID:
            packet.putUInt32(mObjectDb.at(handle).mStorageId);
            break;
        case MTP_PROPERTY_OBJECT_FORMAT:
            packet.putUInt16(mObjectDb.at(handle).mFormat);
            break;
        case MTP_PROPERTY_OBJECT_SIZE:
            packet.putUInt64((uint64_t)getFileSize(MtpString(mObjectDb.at(handle).mPath)));
            break;
        case MTP_PROPERTY_OBJECT_FILE_NAME:
            packet.putString(mObjectDb.at(handle).mName);
            break;
        case MTP_PROPERTY_PARENT_OBJECT:
            packet.putUInt32(mObjectDb.at(handle).mParent);
            break;
        case MTP_PROPERTY_DISPLAY_NAME:
            packet.putString(mObjectDb.at(handle).mName);
            break;
        case MTP_PROPERTY_PERSISTENT_UID:
            packet.putUInt128(handle);
            break;
        case MTP_PROPERTY_ASSOCIATION_TYPE:
            if (mObjectDb.at(handle).mFormat == MTP_FORMAT_ASSOCIATION)
                packet.putUInt16(MTP_ASSOCIATION_TYPE_GENERIC_FOLDER);
            else
                packet.putUInt16(0);
            break;
        case MTP_PROPERTY_DATE_MODIFIED:
            formatDateTime(mObjectDb.at(handle).mDateModified, date, sizeof(date));
            packet.putString(date);
            break;
        default:
            return MTP_RESPONSE_GENERAL_ERROR;
    }

    return MTP_RESPONSE_OK;
}

MtpResponseCode MyMtpDatabase::setObjectPropertyValue(MtpObjectHandle handle,
        MtpObjectProperty property,
        MtpDataPacket& packet)
{
    ObjectInfo objInfo;
    MtpStringBuffer buffer;
    MtpString oldname;
    MtpString newname;
    MtpString newPath;

    if (checkObjectHandle(handle) != 0)
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }

    ALOGI("%s: handle: %d, property: %s\n", __func__, handle,
            MtpDebug::getObjectPropCodeName(property));

    switch(property)
    {
        case MTP_PROPERTY_OBJECT_FILE_NAME:
            objInfo = mObjectDb.at(handle);
            packet.getString(buffer);
            newPath = objInfo.mPath;
            newPath = newPath.getPathDir();
            newPath.append("/");
            newPath.append(buffer);
            ALOGD("%s: newname->%s\n", __func__, newPath.string());

            if (::rename(objInfo.mPath, newPath.string()) != 0) {
                ALOGD("%s: error; %s\n", __func__, strerror(errno));
                return MTP_RESPONSE_DEVICE_BUSY;
            }

            if (mObjectDb.at(handle).mName != NULL)
                ::free(mObjectDb.at(handle).mName);
            if (mObjectDb.at(handle).mPath != NULL)
                ::free(mObjectDb.at(handle).mPath);

            mObjectDb.at(handle).mName = ::strdup(buffer);
            mObjectDb.at(handle).mPath = ::strdup(newPath);
            break;
        default:
            return MTP_RESPONSE_OPERATION_NOT_SUPPORTED;
    }

    return MTP_RESPONSE_OK;
}

MtpResponseCode MyMtpDatabase::getDevicePropertyValue(MtpDeviceProperty property,
        MtpDataPacket& packet)
{
    ALOGD("%s\n", __func__);
    switch(property)
    {
        case MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER:
        case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
            packet.putString(DEVICE_FRIENDLY_NAME); //FIXME: more suitable name?
            break;
        default:
            return MTP_RESPONSE_OPERATION_NOT_SUPPORTED;
    }

    return MTP_RESPONSE_OK;
}

MtpResponseCode MyMtpDatabase::setDevicePropertyValue(MtpDeviceProperty property,
        MtpDataPacket& packet)
{
    ALOGD("%s\n", __func__);
    return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
}

MtpResponseCode MyMtpDatabase::resetDeviceProperty(MtpDeviceProperty property)
{
    ALOGD("%s", __func__);
    return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
}

MtpResponseCode MyMtpDatabase::getObjectPropertyList(MtpObjectHandle handle,
        uint32_t format, uint32_t property,
        int groupCode, int depth,
        MtpDataPacket& packet)
{
    ALOGD("%s\n", __func__);
    return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
}

MtpResponseCode MyMtpDatabase::getObjectInfo(MtpObjectHandle handle, MtpObjectInfo& info)
{
    ObjectInfo objInfo;

    if (checkObjectHandle(handle) != 0)
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }

    ALOGD("%s, handle %d, path %s, name %s\n", __func__, handle,
            mObjectDb.at(handle).mPath, mObjectDb.at(handle).mName);

    info.mHandle = handle;
    info.mStorageID = mObjectDb.at(handle).mStorageId;
    info.mFormat = mObjectDb.at(handle).mFormat;
    info.mCompressedSize = mObjectDb.at(handle).mCompressedSize;
    info.mParent = mObjectDb.at(handle).mParent;
    info.mName = mObjectDb.at(handle).mName == NULL ? NULL : (::strdup(mObjectDb.at(handle).mName));
    info.mDateModified = mObjectDb.at(handle).mDateModified;
    //info.mAssociationType = (info.mFormat == MTP_FORMAT_ASSOCIATION ?
    //                            MTP_ASSOCIATION_TYPE_GENERIC_FOLDER :
    //                            MTP_ASSOCIATION_TYPE_UNDEFINED);
    info.mAssociationType = MTP_ASSOCIATION_TYPE_UNDEFINED;

    info.mProtectionStatus = 0x0;
    info.mImagePixWidth = 0;
    info.mImagePixHeight = 0;
    info.mImagePixDepth = 0;
    info.mAssociationDesc = 0;
    info.mSequenceNumber = 0;
    info.mDateCreated = 0;

    return MTP_RESPONSE_OK;
}

void* MyMtpDatabase::getThumbnail(MtpObjectHandle handle, size_t& outThumbSize)
{
    ALOGD("%s not support\n", __func__);
    return NULL;
}

MtpResponseCode MyMtpDatabase::getObjectFilePath(MtpObjectHandle handle,
        MtpString& outFilePath,
        int64_t& outFileLength,
        MtpObjectFormat& outFormat)
{
    ObjectInfo objInfo;

    if (checkObjectHandle(handle) != 0)
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }

    outFilePath = objInfo.mPath;
    outFileLength = getFileSize(outFilePath);
    outFormat = objInfo.mFormat;

    ALOGD("%s: handle:  %d, path: %s, length: %lld, format: %d", __func__,
            handle, objInfo.mPath, outFileLength, objInfo.mFormat);


    return MTP_RESPONSE_OK;
}

MtpResponseCode MyMtpDatabase::deleteFile(MtpObjectHandle handle)
{
    ObjectInfo objInfo;
    ALOGD("%s: handle: %d\n", __func__, handle);

    if (checkObjectHandle(handle) != 0)
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }

    if (mObjectDb.at(handle).mFormat == MTP_FORMAT_ASSOCIATION) {
        inotify_rm_watch(mInotifyFd, mObjectDb.at(handle).mWd);
        for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
                iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
            if (iter->second.mParent == handle) {
                deleteFile(iter->first);
            }
        }
    }

    if (mObjectDb.at(handle).mName != NULL)
        ::free(mObjectDb.at(handle).mName);
    if (mObjectDb.at(handle).mPath != NULL)
        ::free(mObjectDb.at(handle).mPath);
    mObjectDb.erase(handle);

    return MTP_RESPONSE_OK;
}

struct PropertyTableEntry {
    MtpObjectProperty   property;
    int                 type;
};

static const PropertyTableEntry   kObjectPropertyTable[] = {
    {   MTP_PROPERTY_STORAGE_ID,        MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_OBJECT_FORMAT,     MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_PROTECTION_STATUS, MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_OBJECT_SIZE,       MTP_TYPE_UINT64     },
    {   MTP_PROPERTY_OBJECT_FILE_NAME,  MTP_TYPE_STR        },
    {   MTP_PROPERTY_DATE_MODIFIED,     MTP_TYPE_STR        },
    {   MTP_PROPERTY_PARENT_OBJECT,     MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_PERSISTENT_UID,    MTP_TYPE_UINT128    },
    {   MTP_PROPERTY_NAME,              MTP_TYPE_STR        },
    {   MTP_PROPERTY_DISPLAY_NAME,      MTP_TYPE_STR        },
    {   MTP_PROPERTY_DATE_ADDED,        MTP_TYPE_STR        },
    {   MTP_PROPERTY_ARTIST,            MTP_TYPE_STR        },
    {   MTP_PROPERTY_ALBUM_NAME,        MTP_TYPE_STR        },
    {   MTP_PROPERTY_ALBUM_ARTIST,      MTP_TYPE_STR        },
    {   MTP_PROPERTY_TRACK,             MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_ORIGINAL_RELEASE_DATE, MTP_TYPE_STR    },
    {   MTP_PROPERTY_GENRE,             MTP_TYPE_STR        },
    {   MTP_PROPERTY_COMPOSER,          MTP_TYPE_STR        },
    {   MTP_PROPERTY_DURATION,          MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_DESCRIPTION,       MTP_TYPE_STR        },
    {   MTP_PROPERTY_AUDIO_WAVE_CODEC,  MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_BITRATE_TYPE,      MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_AUDIO_BITRATE,     MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_NUMBER_OF_CHANNELS,MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_SAMPLE_RATE,       MTP_TYPE_UINT32     },
};

static const PropertyTableEntry   kDevicePropertyTable[] = {
    {   MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER,    MTP_TYPE_STR },
    {   MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,       MTP_TYPE_STR },
};

bool MyMtpDatabase::getObjectPropertyInfo(MtpObjectProperty property, int& type) {
    int count = ARRAY_SIZE(kObjectPropertyTable);
    const PropertyTableEntry* entry = kObjectPropertyTable;
    for (int i = 0; i < count; i++, entry++) {
        if (entry->property == property) {
            type = entry->type;
            return true;
        }
    }
    return false;
}

bool MyMtpDatabase::getDevicePropertyInfo(MtpDeviceProperty property, int& type) {
    int count = ARRAY_SIZE(kDevicePropertyTable);
    const PropertyTableEntry* entry = kDevicePropertyTable;
    for (int i = 0; i < count; i++, entry++) {
        if (entry->property == property) {
            type = entry->type;
            return true;
        }
    }
    return false;
}

MtpObjectHandleList* MyMtpDatabase::getObjectReferences(MtpObjectHandle handle)
{
    ObjectInfo objInfo;
    ALOGD("%s handle %d, db size %d\n", __func__, handle, mObjectDb.size());

    if (handle == 0 || handle == MTP_PARENT_ROOT)
        return NULL;

    try {
        objInfo = mObjectDb.at(handle);
    }
    catch (const std::out_of_range& ex) { //handle is valid?
        ALOGE("%s: invalid handle %d\n", __func__, handle);
        return NULL;
    }

    return getObjectList(objInfo.mStorageId, objInfo.mFormat, handle);
}

MtpResponseCode MyMtpDatabase::setObjectReferences(
    MtpObjectHandle handle,
    MtpObjectHandleList* references)
{
    ALOGI("%s\n", __func__);

    return MTP_RESPONSE_OK;
}

MtpProperty* MyMtpDatabase::getObjectPropertyDesc(MtpObjectProperty property,
        MtpObjectFormat format)
{
    ALOGD("%s: %s\n", __func__, MtpDebug::getObjectPropCodeName(property));

    MtpProperty* result = NULL;

    switch (property) {
    case MTP_PROPERTY_OBJECT_FORMAT:
        // use format as default value
        result = new MtpProperty(property, MTP_TYPE_UINT16, false, format);
        break;
    case MTP_PROPERTY_PROTECTION_STATUS:
        result = new MtpProperty(property, MTP_TYPE_UINT16);
        break;
    case MTP_PROPERTY_STORAGE_ID:
    case MTP_PROPERTY_PARENT_OBJECT:
        result = new MtpProperty(property, MTP_TYPE_UINT32);
        break;
    case MTP_PROPERTY_OBJECT_SIZE:
        result = new MtpProperty(property, MTP_TYPE_UINT64);
        break;
    case MTP_PROPERTY_PERSISTENT_UID:
        result = new MtpProperty(property, MTP_TYPE_UINT128);
        break;
    case MTP_PROPERTY_NAME:
    case MTP_PROPERTY_DISPLAY_NAME:
    case MTP_PROPERTY_DESCRIPTION:
        result = new MtpProperty(property, MTP_TYPE_STR);
        break;
    case MTP_PROPERTY_DATE_MODIFIED:
    case MTP_PROPERTY_DATE_ADDED:
        result = new MtpProperty(property, MTP_TYPE_STR);
        result->setFormDateTime();
        break;
    case MTP_PROPERTY_OBJECT_FILE_NAME:
        // We allow renaming files and folders
        result = new MtpProperty(property, MTP_TYPE_STR, true);
        break;
    case MTP_PROPERTY_ASSOCIATION_TYPE:
        result = new MtpProperty(property, MTP_TYPE_UINT16, false); break;
    case MTP_PROPERTY_ASSOCIATION_DESC:
        result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
    default:
        break;
    }

    return result;
}

MtpProperty* MyMtpDatabase::getDevicePropertyDesc(MtpDeviceProperty property)
{
    ALOGD("%s: %s\n", __func__, MtpDebug::getDevicePropCodeName(property));

    MtpProperty* result = NULL;
    String16 str(DEVICE_FRIENDLY_NAME); //Same with getDevicePropertyValue.
    switch(property)
    {
        case MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER:
        case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
            result = new MtpProperty(property, MTP_TYPE_STR, true);
            result->setCurrentValue(str.string());
            break;
        default:
            break;
    }

    return result;
}

void MyMtpDatabase::sessionStarted()
{
    ALOGD("%s\n", __func__);
}

void MyMtpDatabase::sessionEnded()
{
    ALOGD("%s\n", __func__);
    mServer = NULL;
}

void MyMtpDatabase::addStorage(const MtpString& storagePath, const MtpString& discription, MtpStorageID storage)
{
    ObjectInfo objInfo;
    MtpString leafName = storagePath.getPathLeaf();

    if (!discription.empty())
        leafName = discription;

    if (::access(storagePath.string(), F_OK) == 0 && isDirectory(storagePath)) {
            MtpObjectHandle handle = mHandleId++;
            objInfo.mStorageId = storage;
            objInfo.mParent = MTP_PARENT_ROOT;
            objInfo.mName = ::strdup(leafName);
            objInfo.mPath = ::strdup(storagePath);
            objInfo.mFormat = MTP_FORMAT_ASSOCIATION;
            objInfo.mCompressedSize = 0;
            objInfo.mWd = inotify_add_watch(mInotifyFd, storagePath.string(),
                    /*IN_MODIFY | */IN_CREATE | IN_DELETE);
            objInfo.mDateModified = getModifiedTime(storagePath);

            mObjectDb.insert( std::pair<MtpObjectHandle, ObjectInfo>(handle, objInfo) );
            ALOGI("Add storage path %s.\n", storagePath.string());
            enumDirectory (storagePath, 0, storage); // Only support one storage.
    } else {
            ALOGI("%s invalid storage path.\n", storagePath.string());
    }
}

void MyMtpDatabase::removeStorage(MtpStorageID storage)
{
    for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
            iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
        if (iter->second.mStorageId == storage) {
            if (mObjectDb.at(iter->first).mName != NULL)
                ::free(mObjectDb.at(iter->first).mName);
            if (mObjectDb.at(iter->first).mPath != NULL)
                ::free(mObjectDb.at(iter->first).mPath);

            mObjectDb.erase(iter->first);
        }
    }
}

void MyMtpDatabase::setMtpServer(MtpServer* server)
{
    mServer = server;
}

void MyMtpDatabase::stopInotifyWatchThread()
{
    mRunning = false;
}

bool MyMtpDatabase::needStopInotifyWatchThread()
{
    return (mRunning == false);
}

void MyMtpDatabase::setMediaFlag(bool flag)
{
    if (flag == true)
        mMediaFlag++;
    else
        mMediaFlag--;
}

void MyMtpDatabase::inotifyWatchHandler(const char eventBuf[], size_t transferred)
{
    size_t processed = 0;

    //if no storage exist,there shouldn't recieved any inotification.
    if(mMediaFlag <= 0)
        return ;

    while(transferred - processed >= sizeof(inotify_event))
    {
        const char* data = processed + eventBuf;
        const inotify_event* event = reinterpret_cast<const inotify_event*>(data);
        MtpObjectHandle parent = 0;
        processed += sizeof(inotify_event) + event->len;

        for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
                iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
            if (iter->second.mWd == event->wd) {
                parent = iter->first;
                break;
            }
        }
        if (parent == 0 || !(event->mask & (IN_DELETE | IN_CREATE)))
            continue;
        MtpString path(mObjectDb.at(parent).mPath);
        path.append("/");
        path.append(event->name);

        // Comment IN_MODIFY since this will get two many IN_MODIFY events if copy large files.
        /*if(event->len > 0 && (event->mask & IN_MODIFY))
        {
            ALOGD("%s: file %s modified\n", __func__, path.string());
            for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
                    iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
                if (path == iter->second.mPath) {
                    mObjectDb.at(iter->first).mCompressedSize = getCompressedFileSize(path);
                    break;
                }
            }

        }
        else */if(event->len > 0 && (event->mask & IN_CREATE))
        {
            int pHandle = parent;
            bool isExist = false;

            ALOGD("%s: file %s created\n", __func__, path.string());
            // when new a folder or file by MTP host, the CREATE event will be received
            // after MTP_OPERATION_SEND_OBJECT_INFO. Avoid to add this MTP object twice.
            for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
                    iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
                if (path == iter->second.mPath) {
                    isExist = true;
                    break;
                }
            }
            if (!isExist)
                addObjectInfo(path, pHandle, mObjectDb.at(parent).mStorageId);
        }
        else if(event->len > 0 && (event->mask & IN_DELETE))
        {
            ALOGD("%s: file %s deleted\n", __func__, path.string());

            for (std::map<MtpObjectHandle, ObjectInfo>::const_iterator iter = mObjectDb.begin(),
                    iter_end = mObjectDb.end(); iter != iter_end; ++iter) {
                if (path == iter->second.mPath) {
                    ALOGD("deleting file %s at handle %d\n", iter->second.mPath, iter->first);
                    deleteFile(iter->first);
                    if (mServer)
                        mServer->sendObjectRemoved(iter->first);
                    break;
                }
            }
        }
    }
}

}

static MtpServer* mServer;
static MyMtpDatabase* myDatabase;
static MtpStorage *mExternalStorage; //SDcard
static MtpStorage *mInternalStorage;
static int mInotifyMediaFd; //check inotify of the sdcard inserting

/**
 * Watch any file/folder IN_CREATE,IN_MODIFY and IN_DELETE operations
 * under /mnt/sdcard.
 */
static void* inotifyWatchEntry(void *obj) {
    int ret;
    struct inotify_event *event;
    char eventBuf[512];
    MyMtpDatabase *me = reinterpret_cast<MyMtpDatabase *>(obj);

    while(!me->needStopInotifyWatchThread()) {
        ret = read(me->mInotifyFd, eventBuf, sizeof(eventBuf));
        if (ret >= (int)sizeof(struct inotify_event)) {
            me->inotifyWatchHandler(eventBuf, ret);
        }
    }
    ALOGD("InotifyWatchThread exit\n");
    return NULL;
}

static void inotifyAddWatch() {
    ALOGD("%s\n", __func__);
    inotify_add_watch(mInotifyMediaFd, "/media",
                    IN_CREATE | IN_DELETE);
}

static void addExternalStorage()
{
    ALOGD("%s\n", __func__);
    mExternalStorage = new MtpStorage(MTP_STORAGE_REMOVABLE_RAM, EXTERNAL_STORAGE_PATH, EXTERNAL_STORAGE_DESC, 0, true, 0);
    myDatabase->addStorage(MtpString(EXTERNAL_STORAGE_PATH), MtpString(EXTERNAL_STORAGE_DESC), mExternalStorage->getStorageID());

    mServer->addStorage(mExternalStorage);
    myDatabase->setMediaFlag(true);
}

static void checkAndAddInternalStorage() {
    ALOGD("%s\n", __func__);

    if(access(INTERNAL_STORAGE_PATH, F_OK) < 0) {
        ALOGD("%s: path %s not exist\n", __func__, INTERNAL_STORAGE_PATH);
        return;
    }

    mInternalStorage = new MtpStorage(MTP_STORAGE_FIXED_RAM, INTERNAL_STORAGE_PATH,
            INTERNAL_STORAGE_DESC, 0, false, 0);
    myDatabase->addStorage(MtpString(INTERNAL_STORAGE_PATH),
            MtpString(INTERNAL_STORAGE_DESC), mInternalStorage->getStorageID());

    mServer->addStorage(mInternalStorage);
    myDatabase->setMediaFlag(true);

}

static void removeStorage(MtpStorage *storage)
{
    ALOGD("%s\n", __func__);
    mServer->removeStorage(storage);
    myDatabase->removeStorage(storage->getStorageID());
    delete storage;
    myDatabase->setMediaFlag(false);
}

/**
 * Detect SDcard mount/unmount events.
 */
static void inotifyWatchMediaHandler(const char eventBuf[], size_t transferred)
{
    size_t processed = 0;

    ALOGD("%s\n", __func__);

    while(transferred - processed >= sizeof(inotify_event))
    {
        const char* cdata = processed + eventBuf;
        const inotify_event* ievent = reinterpret_cast<const inotify_event*>(cdata);
        processed += sizeof(inotify_event) + ievent->len;

        if (ievent->len > 0 && ievent->mask & IN_CREATE  &&
                !strcmp(ievent->name, "sdcard")) {
            addExternalStorage();
        } else if (ievent->len > 0 && ievent->mask & IN_DELETE &&
                !strcmp(ievent->name, "sdcard")) {
            if (mExternalStorage != NULL) {
                removeStorage(mExternalStorage);
                mExternalStorage = NULL;
            }
        }
    }
}

/**
 * Watch /media/sdcard IN_CREATE or IN_DELETE under /media
 * to get SDcard mount/unmount events.
 */
static void *inotifyWatchMedia(void *obj) {
    int ret;
    struct inotify_event *event;
    char eventBuf[512];

    while(!myDatabase->needStopInotifyWatchThread()) {
        ret = read(mInotifyMediaFd, eventBuf, sizeof(eventBuf));
        if (ret >= (int)sizeof(struct inotify_event)) {
            inotifyWatchMediaHandler(eventBuf, ret);
        }
    }

    return NULL;
}

/*
 * Catch signals to free resources before exit.
 */
static void signalHandler(int signo) {
    ALOGI("Got signal %d, stop MTP\n", signo);
    mServer->stop();
}

static void installSignalHandler() {
  struct sigaction action;

  sigemptyset(&action.sa_mask);

  action.sa_flags = SA_RESETHAND;
  action.sa_handler = signalHandler;

  if(sigaction(SIGINT,&action,NULL) < 0)  {
      ALOGE("install sigal error\n");
  } else {
      ALOGI("install for SIGINT \n");
  }

  if(sigaction(SIGTERM,&action,NULL) < 0)  {
      ALOGE("install sigal error\n");
  } else {
      ALOGI("install for SIGTERM \n");
  }
}

static void checkExternalStorage() {
    int pid;
    FILE *fp;
    char buf[128];

    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        ALOGE("%s: could not open /proc/mounts, errno %d\n", __func__, errno);
        return;
    }

    while (!feof(fp)) {
        fgets(buf, 128, fp);
        if(strstr(buf, "/mnt/sdcard")) {
            // ln /media/sdcard to /mnt/sdcard if not.
            if(access("/media/sdcard", F_OK) < 0) {
                pid = fork();
                if (!pid) {
                    execl("/bin/ln", "ln", "-sf", "/mnt/sdcard", "/media/sdcard", NULL);
                    exit(-1);
                } else if (pid > 0) {
                    waitpid(pid, NULL, 0);
                    ALOGD("%s: finish to ln /media/sdcard\n", __func__);
                }
            } else {
                addExternalStorage();
            }
            break;
        }
    }

    fclose(fp);
}

int main(int argc, char** argv)
{
    int ret;
    pthread_t inotifyWatchMediaThread;


    //Initialize
    myDatabase = new MyMtpDatabase();

    int fd = open("/dev/mtp_usb", O_RDWR); //fd is closed in MtpServer::run.
    if (fd < 0) {
        ALOGI("could not open MTP driver, exit with errno %d\n", errno);
        return -1;
    }
    gid_t gid = getgid();
    ALOGD("MTP daemon gid %d fd %d\n", gid, fd);

    mServer = new MtpServer(fd, myDatabase, false, gid, 0644, 0775);
    myDatabase->setMtpServer(mServer);

    mInotifyMediaFd = inotify_init();
    ret = pthread_create(&inotifyWatchMediaThread, NULL, inotifyWatchMedia, NULL);
    if (ret < 0) {
        ALOGE("Can't create notifier_thread to monitor /media change in target MTP device\n");
        goto fail;
    }
    inotifyAddWatch();
    checkExternalStorage();
    checkAndAddInternalStorage();

    installSignalHandler();

    //MTP protocol handler
    mServer->run();

fail:
    //Release
    if (mExternalStorage != NULL)
        removeStorage(mExternalStorage);
    if (mInternalStorage != NULL)
        removeStorage(mInternalStorage);
    delete mServer;
    delete myDatabase;
    close(mInotifyMediaFd);
}
