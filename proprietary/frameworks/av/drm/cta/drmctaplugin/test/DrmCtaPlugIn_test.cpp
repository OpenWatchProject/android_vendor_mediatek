//
// Copyright 2013 Google Inc. All Rights Reserved.
//

#include <stdio.h>
#include <string>
#include <utils/Vector.h>
#include <iostream>

#include <fcntl.h>
#include <dlfcn.h>
#include "gtest/gtest.h"
#include "media/stagefright/foundation/ABase.h"
#include "media/stagefright/foundation/AString.h"
#include <DrmCtaPlugIn.h>
#include <DrmRequestType.h>
#include <drm/DrmInfoRequest.h>
#include <Cta5File.h>
#include <Cta5FileFactory.h>
#include <drm/drm_framework_common.h>
#include <drm/DrmManagerClient.h>
#include <drm/DrmInfoEvent.h>
#include <DrmDef.h>
#include <drm/DrmInfo.h>

using namespace android;
using namespace std;
using namespace testing;

typedef void (*destroy_t)(IDrmEngine *);
typedef IDrmEngine *(*create_t)();

//static const int TYPE_SET_DRM_INFO = 2021;

const String8 ACTION_CTA5_ENCRYPT("CTA5Encrypt");
const String8 ACTION_CTA5_DECRYPT("CTA5Decrypt");
const String8 ACTION_CTA5_SETKEY("CTA5SetKey");
const String8 ACTION_CTA5_CHANGEPASSWORD("CTA5ChangePassword");
const String8 ACTION_CTA5_GETPROGESS("CTA5GetProgress");
const String8 ACTION_CTA5_CANCEL("CTA5Cancel");
const String8 ACTION_CTA5_ISCTAFILE("CTA5IsCtaFile");

String8 clearPath("/sdcard/clear.txt");
String8 cipherPath("/sdcard/cipher.txt");
String8 decryptPath("/sdcard/decrypt.txt");
/*String8 clearPath("/sdcard/aa.jpg");
String8 cipherPath("/sdcard/bb.jpg");
String8 decryptPath("/sdcard/cc.jpg");*/
String8 testDecryptPath("/sdcard/testdecrypt.txt");
String8 PWD("123456");
String8 NEWPWD("654321");


class DrmCtaPlugInTest: public Test
{
protected:

    static void SetUpTestCase();
    static void TearDownTestCase();
    static void setKey();
    static bool isFileEquals(String8 clearFile, String8 cipherFile);
    static int writeToFile();
    static bool checkDrmInfoResult(DrmInfo* drmInfo, String8 str);
    static void compareFile(int fd1, int fd2);
    virtual void SetUp();
    virtual void TearDown();
protected:
    static void *handle;
    static create_t sCreator;
    static destroy_t sDestroyer;
    static IDrmEngine *sPlugin;
    static int sUniqeId;
};

class CTAMultimediaTestListener : public IDrmEngine::OnInfoListener {
public:
    virtual void onInfo(const DrmInfoEvent& event);
    void reset();

    bool isReachedCancelCondition(const String8 message);

    bool done;
    bool hasError;
    bool needCancel;
    bool inCancelCase;
    bool cancelDone;
};

// get encrypt/decrypt progress
// check whether is reached cancel condition
// if the progress is greater than 30%, return true
bool CTAMultimediaTestListener::isReachedCancelCondition(const String8 message) {
    int pos = 0, found = 0, wanted_pos = 0;
    const char* spliter = "::";
    char temp[40] = {0};
    char data_s[32] = {0};
    char cnt_s[32] = {0};
    float data = 0.0f, cnt = 0.0f;
    while((found = message.find(spliter, pos)) != -1) {
        bzero(temp, sizeof(temp));
        bzero(data_s, sizeof(data_s));
        bzero(cnt_s, sizeof(cnt_s));
        strncpy(temp, message.string() + pos, found - pos);
        String8 item = String8(temp);
        pos = found + strlen(spliter);

        wanted_pos = item.find("data_s:", 0);
        if(-1 != wanted_pos) {
            strcpy(data_s, item.string() + wanted_pos + 7);
            data = atof(data_s);
            continue;
        }

        wanted_pos = item.find("cnt_s:", 0);
        if(-1 != wanted_pos) {
            strcpy(cnt_s, item.string() + wanted_pos + 6);
            cnt = atof(cnt_s);
            continue;
        }

    }
    // get last item
    strcpy(temp, message.string() + pos);

    if(cnt / data > 0.3) {
        printf("progress reached 30%\n");
        return true;
    } else {
        return false;
    }

}


void CTAMultimediaTestListener::onInfo(const DrmInfoEvent& event) {
    String8 message = event.getMessage();
    int position = message.find("result:", 0);
    position = position + 7;
    char result[32] = {0};
    strcpy(result, message.string() + position);
    if(0 == strcmp(result, DrmDef::CTA5_MULTI_MEDIA_ENCRYPT_DONE.string())) {
        printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        done = true;
        hasError = false;
    } else if(0 == strcmp(result, DrmDef::CTA5_MULTI_MEDIA_DECRYPT_DONE.string())) {
        printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        done = true;
        hasError = false;
    } else if(0 == strcmp(result, DrmDef::CTA5_CANCEL_DONE.string())) {
        printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        done = true;
        cancelDone = true;
        hasError = false;
    } else if(0 == strcmp(result, DrmDef::CTA5_ERROR.string())) {
        printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        done = true;
        hasError = true;
    } else if(0 == strcmp(result, DrmDef::CTA5_CANCEL_ERROR.string())) {
        printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        done = true;
        hasError = true;
        cancelDone = false;
    } else if(0 == strcmp(result, DrmDef::CTA5_UPDATING.string())) {
        //printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        // check progress
        cancelDone = false;
        hasError = false;
        if(inCancelCase && !needCancel) {
            needCancel = isReachedCancelCondition(message);
        }
    } else if(0 == strcmp(result, DrmDef::CTA5_DONE.string())) {
        printf("CTATestListener - onInfo -> message: %s, result: %s\n", message.string(), result);
        cancelDone = false;
        done = true;
        hasError = false;
    }
}

void CTAMultimediaTestListener::reset() {
    printf("CTATestListener - reset...");
    done = false;
    hasError =  false;
    needCancel = false;
    cancelDone = false;
}

void *DrmCtaPlugInTest::handle = NULL;
create_t DrmCtaPlugInTest::sCreator = NULL;
destroy_t DrmCtaPlugInTest::sDestroyer = NULL;
IDrmEngine *DrmCtaPlugInTest::sPlugin = NULL;
int DrmCtaPlugInTest::sUniqeId = 0;

void DrmCtaPlugInTest::SetUpTestCase()
{
    //cout<<"DrmCtaPlugInTest::SetUpTestCase"<<endl;
    const char *path = "drm/libdrmctaplugin.so";
    handle = dlopen(path, RTLD_NOW);
    if (handle == NULL)
    {
        fprintf(stderr, "Can't open plugin: %s %s\n", path, dlerror());
        exit(-1);
    }
    sCreator = (create_t) dlsym(handle, "create");
    if (!sCreator)
    {
        fprintf(stderr, "Can't find create method\n");
        exit(-1);
    }

    sDestroyer = (destroy_t) dlsym(handle, "destroy");
    if (!sDestroyer)
    {
        fprintf(stderr, "Can't find destroy method\n");
        exit(-1);
    }

}

void DrmCtaPlugInTest::TearDownTestCase()
{
    //cout<<"DrmCtaPlugInTest::TearDownTestCase"<<endl;
    if(handle)
    {
        dlclose(handle);
        handle = NULL;
    }
}

void DrmCtaPlugInTest::setKey()
{
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_SETKEY);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_KEY, PWD);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    cout <<"set key done" << endl;
    ASSERT_TRUE(info != NULL);
    delete info;
}

void DrmCtaPlugInTest::compareFile(int fd1, int fd2) {
    int size1 = lseek(fd1, 0, SEEK_END);
    ASSERT_NE(-1, size1);
    lseek(fd1, 0, SEEK_SET);
    int size2 = lseek(fd2, 0, SEEK_END);
    ASSERT_NE(-1, size2);
    lseek(fd2, 0, SEEK_SET);

    ASSERT_EQ(size1, size2) << "size1: " << size1 << ", size2: " << size2;

    const int BUFF_LEN = 4096;
    uint8_t read1[BUFF_LEN] = {0};
    uint8_t read2[BUFF_LEN] = {0};
    int bytesRead = 0;
    int totalRead = 0;
    while(totalRead < size1) {
        bytesRead = totalRead + BUFF_LEN <= size1 ? BUFF_LEN : size1 - totalRead;
        bytesRead = read(fd1, read1, bytesRead);
        bytesRead = read(fd2, read2, bytesRead);
        ASSERT_EQ(0, memcmp(read1, read2, bytesRead)) << "bytesRead: " << bytesRead;

        totalRead += bytesRead;
    }
    printf("compare file sucess.\n");
}

void DrmCtaPlugInTest::SetUp()
{
    //cout<<"DrmCtaPlugInTest::SetUp()"<<endl;
    if(sCreator == NULL)
    {
        cout<<"SetUp fail - sCreator is NULL"<<endl;
        return ;
    }
    sPlugin = (*sCreator)();
    if (sPlugin == NULL)
    {
        cout << "SetUp fail - sPlugin is NULL" << endl;
        return;
    }
    sUniqeId = 0;
    if (sPlugin->initialize(sUniqeId) != DRM_NO_ERROR)
    {
        cout<<"onInitialize failed!"<<endl;
        return;
    }
}

void DrmCtaPlugInTest::TearDown()
{
    //cout<<"DrmCtaPlugInTest::TearDown()"<<endl;
    if(sDestroyer == NULL || sPlugin == NULL)
    {
        cout << "TearDown failed!" << endl;
        return;
    }
    sPlugin->terminate(sUniqeId);
    sDestroyer(sPlugin);
    sPlugin = NULL;
}


//Create two files with the same content
//Encrypt a file and decrypt it, then compare the decrypt file and the created content
//Normal cta5 file case and multi media file case
TEST_F(DrmCtaPlugInTest, Encrypt)
{
    //step 0. write random content to clear file
    cout << "start write to file" << endl;
    int writeLen = writeToFile();
    cout << "end write to file" << endl;

    //step 1. encrypt the clear file to cipher file
    bool result = false;
    setKey();
    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);
    cout <<"start encrypt-------"<<endl;
    //step 1.1 get clear file fd
    int enclearFd = -1;
    enclearFd = open(clearPath, O_RDWR, "0666");
    ASSERT_TRUE(enclearFd != -1) << "open clear file failed" << endl;
    String8 enclearFd_str;
    enclearFd_str.appendFormat("%d", enclearFd);
    //step 1.2 get cipher file fd
    int encipherFd = -1;
    encipherFd = open(cipherPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(encipherFd != -1) << "open cipherFd file failed" << endl;
    String8 encipherFd_str;
    encipherFd_str.appendFormat("%d", encipherFd);

    //step 1.3 do encrypt
    DrmInfoRequest endrmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    endrmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    endrmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, enclearFd_str);
    endrmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, encipherFd_str);

    DrmInfo *eninfo = sPlugin->acquireDrmInfo(sUniqeId, &endrmInfoReq);
    ASSERT_TRUE(eninfo != NULL) << "encrypt return value is fail" << endl;
    delete eninfo;

    //step 1.4. wait the encrypt for cancel
    while (!listener->done)
    {
        sleep(1);
    }
    cout <<"encrypt done-------"<<endl;
    close(enclearFd);
    close(encipherFd);
    ASSERT_FALSE(listener->hasError) << "encrypt return value is fail" << endl;
    listener->reset();
    cout <<"start decrypt-------"<<endl;
    //step 2. using password to decrypt the file
    //step 2.1 get cipher file fd
    setKey();
    int cipherFd = -1;
    cipherFd = open(cipherPath, O_RDWR, "0666");
    ASSERT_TRUE(cipherFd != -1) << "open cipher failed" << endl;
    String8 cipherFd_str;
    cipherFd_str.appendFormat("%d",cipherFd);
    //step 2.2 get decrypt target file fd
    int decyptFd = -1;
    decyptFd = open(decryptPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(decyptFd != -1) << "open decyptFd failed" << endl;
    String8 decypt_str;
    decypt_str.appendFormat("%d",decyptFd);
    //step 2.3 decrypt
    DrmInfoRequest drmInfoReqDecrypt(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_DECRYPT);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_CTA5_CLEAR_FD, decypt_str);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_CTA5_CIPHER_FD, cipherFd_str);
    DrmInfo *infoDecrypt = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReqDecrypt);

    ASSERT_TRUE(infoDecrypt != NULL)<< "encrypt return value is fail"<<endl;
    delete infoDecrypt;

    //step 2.4. wait for decypt done
    while (!listener->done)
    {
        sleep(1);
    }
    close(cipherFd);
    close(decyptFd);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();
    cout << "decrypt done" << endl;

    //step 3. compare the oringal clear file and decypted file
    cout << "file compare start "<<endl;
    result = isFileEquals(clearPath, decryptPath);
    cout << "file compare result :"<< result <<endl;
    ASSERT_TRUE(result) << "file not the same fail----------" << endl;
}

//Check the return value in setKey
TEST_F(DrmCtaPlugInTest, SetKey)
{
    //setKey();
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_SETKEY);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_KEY, PWD);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    cout <<"set key done" << endl;
    ASSERT_TRUE(info != NULL);
    delete info;

    cout <<"test key start" << endl;
    int fd_in = open(cipherPath, O_RDONLY);
    ASSERT_TRUE(fd_in != -1) << "open clear file failed"<<endl;
    Cta5File *pCta5File = Cta5FileFactory::createCta5File(fd_in, PWD);
    bool result = pCta5File->isKeyValid(PWD);
    cout << "pass word:"<<PWD.string() <<endl;
    bool result2 = pCta5File->isKeyValid(String8("333"));
    delete pCta5File;
    pCta5File = NULL;
    close(fd_in);
    ASSERT_TRUE(result) << "password 1 is wrong"<<endl;
    ASSERT_FALSE(result2) << "password 2 is wrong"<<endl;
}

//After password change, using the new password to decrypt
//Normal cta5 file case and multi media file case
//need be executed after encrypt
TEST_F(DrmCtaPlugInTest, ChangePassword)
{
    //step 0. write random content to clear file
    cout << "start write to file" << endl;
    int writeLen = writeToFile();
    cout << "end write to file" << endl;

    //step 1. encrypt the clear file to cipher file
    bool result = false;
    setKey();
    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);

    //step 1.1 get clear file fd
    cout << "start encrypt" << endl;
    int enclearFd = -1;
    enclearFd = open(clearPath, O_RDWR, "0666");
    ASSERT_TRUE(enclearFd != -1) << "open clear file failed" << endl;
    String8 enclearFd_str;
    enclearFd_str.appendFormat("%d", enclearFd);
    //step 1.2 get cipher file fd
    int encipherFd = -1;
    encipherFd = open(cipherPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(encipherFd != -1) << "open cipherFd file failed" << endl;
    String8 encipherFd_str;
    encipherFd_str.appendFormat("%d", encipherFd);

    //step 1.3 do encrypt
    DrmInfoRequest endrmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    endrmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    endrmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, enclearFd_str);
    endrmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, encipherFd_str);

    DrmInfo *eninfo = sPlugin->acquireDrmInfo(sUniqeId, &endrmInfoReq);
    ASSERT_TRUE(eninfo != NULL) << "encrypt return value is fail" << endl;
    delete eninfo;

    //step 1.4. wait the encrypt for cancel
    while (!listener->done)
    {
        sleep(1);
    }
    cout <<"encrypt done"<<endl;
    close(enclearFd);
    close(encipherFd);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();

    //step 2. change cipher file password
    cout << "start change password" << endl;
    int cipher = -1;
    cipher = open(cipherPath, O_RDWR, "0666");
    ASSERT_TRUE(cipher != -1) << "open cipher failed" << endl;
    String8 cipher_str;
    cipher_str.appendFormat("%d",cipher);

    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CHANGEPASSWORD);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_OLDKEY, PWD);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_NEWKEY, NEWPWD);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_FD, cipher_str);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    ASSERT_TRUE(info != NULL);
    result = checkDrmInfoResult(info, String8("success"));
    ASSERT_TRUE(result) << "changepassword result is false" << endl;
    delete info;
    close(cipher);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();
    cout << "change password done" << endl;

    //step 3. using new password to decrypt the file
    //step 3.1 get cipher file fd
    cout << "start decrypt" << endl;
    int cipherFd = -1;
    cipherFd = open(cipherPath, O_RDWR, "0666");
    ASSERT_TRUE(cipherFd != -1) << "open cipher failed" << endl;
    String8 cipherFd_str;
    cipherFd_str.appendFormat("%d",cipherFd);
    //step 3.2 get decrypt target file fd
    int decyptFd = -1;
    decyptFd = open(decryptPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(decyptFd != -1) << "open decyptFd failed" << endl;
    String8 decypt_str;
    decypt_str.appendFormat("%d",decyptFd);
    //step 3.3 decrypt
    DrmInfoRequest drmInfoReqDecrypt(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_DECRYPT);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_CTA5_CLEAR_FD, decypt_str);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_CTA5_CIPHER_FD, cipherFd_str);
    DrmInfo *infoDecrypt = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReqDecrypt);

    ASSERT_TRUE(infoDecrypt != NULL)<< "encrypt return value is fail"<<endl;
    delete infoDecrypt;

    //step 3.4. wait for decypt done
    while (!listener->done)
    {
        sleep(1);
    }
    close(cipherFd);
    close(decyptFd);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();
    cout << "decrypt done" << endl;

    //step 4. compare the oringal clear file and decypted file
    cout << "file compare start "<<endl;
    result = isFileEquals(clearPath, decryptPath);
    cout << "file compare result :"<< result <<endl;
    ASSERT_TRUE(result) << "file not the same fail----------" << endl;
    delete listener;
}

//Check cancel a task which is done
//Check cancel a task which is doing
//Normal cta5 file case and multi media file case
TEST_F(DrmCtaPlugInTest, Cancel)
{
    //step 1. write random content to clear file
    cout << "start write to file" << endl;
    int writeLen = writeToFile();
    cout << "end write to file" << endl;
    //step 2. encrypt the clear file to cipher file
    setKey();
    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    listener->inCancelCase = true;
    sPlugin->setOnInfoListener(sUniqeId, listener);
    //step 2.1 get clear file fd
    int clearFd = -1;
    clearFd = open(clearPath, O_RDWR, "0666");
    ASSERT_TRUE(clearFd != -1) << "open clear file failed" << endl;
    String8 clearFd_str;
    clearFd_str.appendFormat("%d", clearFd);
    //step 2.2 get cipher file fd
    int cipherFd = -1;
    cipherFd = open(cipherPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(cipherFd != -1) << "open cipherFd file failed" << endl;
    String8 cipherFd_str;
    cipherFd_str.appendFormat("%d", cipherFd);

    //step 2.3 do encrypt
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, clearFd_str);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, cipherFd_str);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    ASSERT_TRUE(info != NULL) << "encrypt return value is fail" << endl;
    delete info;

    //step 2.4. wait the encrypt for cancel
    while (!listener->done)
    {
        if (listener->needCancel)
        {
            cout << "need cancel" << endl;
            DrmInfoRequest drmInfoReq2(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
            drmInfoReq2.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CANCEL);
            drmInfoReq2.put(DrmRequestType::KEY_CTA5_FD, clearFd_str);
            DrmInfo *info2 = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq2);
            ASSERT_TRUE(info2 != NULL) << "encrypt return value is fail" << endl;
            delete info2;
        }
        sleep(1);
    }
    sleep(1);
    ASSERT_TRUE(listener->cancelDone) << "cancel fail" << endl;
    cout <<"cancel encrypt done-------"<<endl;
    close(clearFd);
    close(cipherFd);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();

    //step 3. decypt the file
    setKey();
    //step 3.1 get cipher file fd
    listener->inCancelCase = true;
    sPlugin->setOnInfoListener(sUniqeId, listener);

    int cipher = -1;
    cipher = open(cipherPath, O_RDWR, "0666");
    ASSERT_TRUE(cipher != -1) << "open cipher failed" << endl;
    String8 cipher_str;
    cipher_str.appendFormat("%d", cipher);
    //step 3.2 get decrypt target file fd
    int decyptFd = -1;
    decyptFd = open(decryptPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(decyptFd != -1) << "open decyptFd failed" << endl;
    String8 decypt_str;
    decypt_str.appendFormat("%d", decyptFd);

    //step 3.3 do decrypt
    DrmInfoRequest drmInfoReqDecrypt(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_DECRYPT);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_CTA5_CLEAR_FD, decypt_str);
    drmInfoReqDecrypt.put(DrmRequestType::KEY_CTA5_CIPHER_FD, cipher_str);
    DrmInfo *infoDecrypt = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReqDecrypt);

    ASSERT_TRUE(infoDecrypt != NULL) << "encrypt return value is fail" << endl;
    delete infoDecrypt;

    //step 3.4. wait the decrypt for cancel
    while (!listener->done)
    {
        if (listener->needCancel)
        {
            cout << "need cancel" << endl;
            DrmInfoRequest drmInfoReqDecrypt2(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
            drmInfoReqDecrypt2.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CANCEL);
            drmInfoReqDecrypt2.put(DrmRequestType::KEY_CTA5_FD, cipher_str);
            DrmInfo *infoDecrypt2 = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReqDecrypt2);
            ASSERT_TRUE(infoDecrypt2 != NULL) << "encrypt return value is fail" << endl;
            delete infoDecrypt2;
        }
        sleep(1);
    }
    sleep(1);
    ASSERT_TRUE(listener->cancelDone) << "cancel fail" << endl;
    cout <<"cancel decrypt done-------"<<endl;
    close(cipher);
    close(decyptFd);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();
    delete listener;
}


//Check a normal file , a normal cta5 file and a multi media cta5 file
TEST_F(DrmCtaPlugInTest, IsCtaFile)
{
    //step 0. write random content to clear file
    bool result = false;
    cout << "start write to file" << endl;
    int writeLen = writeToFile();
    cout << "end write to file" << endl;
    //step 1. encrypt the clear file to cipher file
    setKey();
    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);

    //step 1.1 get clear file fd
    cout << "start encrypt" << endl;
    int enclearFd = -1;
    enclearFd = open(clearPath, O_RDWR, "0666");
    ASSERT_TRUE(enclearFd != -1) << "open clear file failed" << endl;
    String8 enclearFd_str;
    enclearFd_str.appendFormat("%d", enclearFd);
    //step 1.2 get cipher file fd
    int encipherFd = -1;
    encipherFd = open(cipherPath, O_RDWR | O_TRUNC, "0666");
    ASSERT_TRUE(encipherFd != -1) << "open cipherFd file failed" << endl;
    String8 encipherFd_str;
    encipherFd_str.appendFormat("%d", encipherFd);

    //step 1.3 do encrypt
    DrmInfoRequest endrmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    endrmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    endrmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, enclearFd_str);
    endrmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, encipherFd_str);

    DrmInfo *eninfo = sPlugin->acquireDrmInfo(sUniqeId, &endrmInfoReq);
    ASSERT_TRUE(eninfo != NULL) << "encrypt return value is fail" << endl;
    delete eninfo;

    //step 1.4. wait the encrypt done
    while (!listener->done)
    {
        sleep(1);
    }
    cout <<"encrypt done"<<endl;
    close(enclearFd);
    close(encipherFd);
    ASSERT_FALSE(listener->hasError) << "decrypt return value is fail" << endl;
    listener->reset();

    //step 2. check cta5 file is a cta5 file
    cout << "start check cta5 file is valid" << endl;
    setKey();
    int cipherFd = -1;
    cipherFd = open(cipherPath, O_RDWR, "0666");
    ASSERT_TRUE(cipherFd != -1) << "open cipherFd file failed" << endl;
    String8 cipherFd_str;
    cipherFd_str.appendFormat("%d", cipherFd);

    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ISCTAFILE);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_FD, cipherFd_str);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    ASSERT_TRUE(info != NULL)<<"cquireDrmInfo failed - info is NULL"<<endl;

    result = checkDrmInfoResult(info, String8("success"));
    ASSERT_TRUE(result) << "result is not success" << endl;
    close(cipherFd);
    delete info;
    cout << "end check cta5 file is valid" << endl;

    //step 3. check clear file is a cta5 file
    cout << "start check clear file is valid" << endl;
    int clearFd = -1;
    clearFd = open(clearPath, O_RDWR, "0666");
    ASSERT_TRUE(clearFd != -1) << "open clear file failed" << endl;
    String8 clearFd_str;
    clearFd_str.appendFormat("%d", clearFd);

    DrmInfoRequest cdrmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    cdrmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ISCTAFILE);
    cdrmInfoReq.put(DrmRequestType::KEY_CTA5_FD, cipherFd_str);

    DrmInfo *cinfo = sPlugin->acquireDrmInfo(sUniqeId, &cdrmInfoReq);
    ASSERT_TRUE(cinfo != NULL) << "cquireDrmInfo failed - info is NULL" << endl;

    result = checkDrmInfoResult(cinfo, String8("failure"));
    ASSERT_TRUE(result) << "result is not success" << endl;
    close(clearFd);
    delete cinfo;
    cout << "end check clear file is valid" << endl;
    delete listener;
}

/*
 * getToken method is only allowed to be called by dataprotection app.
 * so if test this. need change drmctaplugin getToken code.
//Check token is valid
TEST_F(DrmCtaPlugInTest, CheckToken)
{
    setKey();
    // get token
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_GET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_GETTOKEN);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_FILEPATH, cipherPath);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    int length =info->getData().length;
    ASSERT_TRUE(info != NULL);
    char* backChar4 = new char[length+1];
    memset(backChar4, 0, length+1);
    //backChar4[length] = '\0';

    cout << "info->getData().data:"<<info->getData().data<<endl;
    cout << "info->getData().length:"<<length<<endl;
    memcpy(backChar4, info->getData().data, length);
    String8 getToken(backChar4);
    cout << "getToken backChar:"<<backChar4<<endl;
    cout << "getToken done:" << getToken << endl;
    cout << "getToken length:" << getToken.length() << endl;
    ASSERT_TRUE(getToken.length()>1)<<"get token is invalid" << endl;
    delete info;
    delete backChar4;

    // check token is valid
    DrmInfoRequest drmInfoReq2(DrmRequestType::TYPE_GET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq2.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CHECKTOKEN);
    drmInfoReq2.put(DrmRequestType::KEY_CTA5_FILEPATH, cipherPath);
    drmInfoReq2.put(DrmRequestType::KEY_CTA5_TOKEN, getToken);

    DrmInfo *info2 = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq2);
    ASSERT_TRUE(info2 != NULL);
    length =info2->getData().length;
    char* backChar2 = new char[length+1];
    memset(backChar2, 0, length+1);
    memcpy(backChar2, info2->getData().data,length);
    cout << "backChar2:" << backChar2<<endl;
    String8 success("success");
    int eq = memcmp(backChar2, success.string(), length);
    cout << "result:" << eq <<endl;
    cout << "check valid Token done:" << eq << endl;
    ASSERT_TRUE(eq==0)<<"token is invalid"<<endl;
    delete info2;
    delete backChar2;

    // check token is invalid
    DrmInfoRequest drmInfoReq3(DrmRequestType::TYPE_GET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    drmInfoReq3.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CHECKTOKEN);
    drmInfoReq3.put(DrmRequestType::KEY_CTA5_FILEPATH, cipherPath);
    drmInfoReq3.put(DrmRequestType::KEY_CTA5_TOKEN, String8("asdf"));

    DrmInfo *info3 = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq3);
    ASSERT_TRUE(info3 != NULL);
    length =info3->getData().length;
    char* backChar3 = new char[length+1];
    memset(backChar3, 0, length+1);
    memcpy(backChar3, info3->getData().data,length);
    cout << "backChar3:" << backChar3<<endl;
    eq = memcmp(backChar3, success.string(), length);
    cout << "result:" << eq <<endl;
    cout << "check invalid token done:" << eq << endl;
    ASSERT_FALSE(eq==0)<<"token is invalid"<<endl;
    delete info3;
    delete backChar3;
}*/


// Create two files with the same content
// Encrypt a file and decrypt it, then compare the decrypt file and the created content
// multi media file case
// steps:
// 1. backup original multimedia file
// 2. encrypt multimedia file
// 3. decrypt cta file
// 4. compare decrypted file with backup file
TEST_F(DrmCtaPlugInTest, EncryptAndDecrypt_MultiMedia)
{
    printf("EncryptAndDecrypt_MultiMedia start\n");
    const char* path = "/sdcard/test.mp4";
    const char* cta_path = "/sdcard/test.mp4.cta";
    const char* backup_path = "/sdcard/test.mp4.bak";
    int original_file_size;

    // 1. backup original multimedia file
    printf("1. backup original multimedia file\n");
    // 1.1 check file exists
    ASSERT_EQ(0, access(path, F_OK)) << "please make sure the file [" << path << "] already existed in your device";
    // 1.2 backup
    system("cp /sdcard/test.mp4 /sdcard/test.mp4.bak");
    // 1.3 check backup file exists
    ASSERT_EQ(0, access(backup_path, F_OK)) << "reason: " << strerror(errno);

    // 2. encrypt multimedia file
    printf("2. encrypt multimedia file\n");
    setKey();
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    // parameters: action, mime, cipher fd, clear fd
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    // 2.1 create two file descriptor
    int clear_fd = open(path, O_RDWR);
    ASSERT_NE(-1, clear_fd);
    int cipher_fd = open(cta_path, O_CREAT, S_IRWXU);
    ASSERT_NE(-1, cipher_fd);

    char str_clear_fd[8] = {0};
    sprintf(str_clear_fd, "%d", clear_fd);
    char str_cipher_fd[8] = {0};
    sprintf(str_cipher_fd, "%d", cipher_fd);

    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_cipher_fd));

    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);

    // 2.2 encrypt
    original_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, original_file_size);
    lseek(clear_fd, 0, SEEK_SET);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

    // wait encrypt finish
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = 50 * 1000;
    while(!listener->done) {
        select(0, NULL, NULL, NULL, &delay);
    }

    // encrypt finish
    printf("encrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);

    ASSERT_TRUE(info != NULL);


    // 2.3 check file size whether has changed
    int cipher_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, cipher_file_size);
    ASSERT_NE(cipher_file_size, original_file_size);

    // 3. decrypt multimedia file
    printf("3. decrypt multimedia file\n");
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_DECRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_cipher_fd));

    lseek(clear_fd, 0, SEEK_SET);
    listener->reset();
    info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    while(!listener->done) {
        select(0, NULL, NULL, NULL, &delay);
    }

    // decrypt finish
    printf("decrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);
    ASSERT_TRUE(info != NULL);

    // 4. compare decrypted file with backup file
    printf("4. compare decrypted file with backup file\n");
    int backup_fd = open(backup_path, O_RDONLY);
    compareFile(clear_fd, backup_fd);

    // 5. recover
    printf("5. recover\n");
    close(backup_fd);
    close(cipher_fd);
    close(clear_fd);
    remove(cta_path);
    remove(backup_path);

    delete info;
    delete listener;

    printf("EncryptAndDecrypt_MultiMedia finish\n");
}

// Create two files with the same content
// Encrypt a file, change password, and decrypt it, then compare the decrypt file and the created content
// multi media file case
// steps:
// 1. backup original multimedia file
// 2. encrypt multimedia file
// 3. change password
// 4. decrypt cta file
// 5. compare decrypted file with backup file
TEST_F(DrmCtaPlugInTest, ChangePassword_MultiMedia)
{
    printf("ChangePassword_MultiMedia start\n");
    const char* path = "/sdcard/test.mp4";
    const char* cta_path = "/sdcard/test.mp4.cta";
    const char* backup_path = "/sdcard/test.mp4.bak";
    int original_file_size;

    // 1. backup original multimedia file
    printf("1. backup original multimedia file\n");
    // 1.1 check file exists
    ASSERT_EQ(0, access(path, F_OK)) << "please make sure the file [" << path << "] already existed in your device";
    // 1.2 backup
    system("cp /sdcard/test.mp4 /sdcard/test.mp4.bak");
    // 1.3 check backup file exists
    ASSERT_EQ(0, access(backup_path, F_OK)) << "reason: " << strerror(errno);

    // 2. encrypt multimedia file
    printf("2. encrypt multimedia file\n");
    setKey();
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    // parameters: action, mime, cipher fd, clear fd
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    // 2.1 create two file descriptor
    int clear_fd = open(path, O_RDWR);
    ASSERT_NE(-1, clear_fd);
    int cipher_fd = open(cta_path, O_CREAT, S_IRWXU);
    ASSERT_NE(-1, cipher_fd);

    char str_clear_fd[8] = {0};
    sprintf(str_clear_fd, "%d", clear_fd);
    char str_cipher_fd[8] = {0};
    sprintf(str_cipher_fd, "%d", cipher_fd);

    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_cipher_fd));

    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);

    // 2.2 encrypt
    original_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, original_file_size);
    lseek(clear_fd, 0, SEEK_SET);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

    // wait encrypt finish
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = 50 * 1000;
    while(!listener->done) {
        select(0, NULL, NULL, NULL, &delay);
    }

    // encrypt finish
    printf("encrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);

    ASSERT_TRUE(info != NULL);


    // 2.3 check file size whether has changed
    int cipher_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, cipher_file_size);
    ASSERT_NE(cipher_file_size, original_file_size);

    // 3. change password
    printf("3. change password\n");
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CHANGEPASSWORD);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_OLDKEY, PWD);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_NEWKEY, String8("654321"));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_FD, String8(str_clear_fd));

    listener->reset();
    info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    ASSERT_TRUE(info != NULL);
    ASSERT_FALSE(listener->hasError);

    // 4. decrypt multimedia file
    printf("4. decrypt multimedia file\n");
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_DECRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_cipher_fd));

    lseek(clear_fd, 0, SEEK_SET);
    listener->reset();
    info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    while(!listener->done) {
        select(0, NULL, NULL, NULL, &delay);
    }

    // decrypt finish
    printf("decrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);
    ASSERT_TRUE(info != NULL);

    // 5. compare decrypted file with backup file
    printf("5. compare decrypted file with backup file\n");
    int backup_fd = open(backup_path, O_RDONLY);
    compareFile(clear_fd, backup_fd);

    // 6. recover
    printf("6. recover\n");
    close(backup_fd);
    close(cipher_fd);
    close(clear_fd);
    remove(cta_path);
    remove(backup_path);

    delete info;
    delete listener;

    printf("ChangePassword_MultiMedia finish\n");
}

// Create two files with the same content
// Encrypt a file, check whether is cta file
// multi media file case
// steps:
// 1. backup original multimedia file
// 2. encrypt multimedia file
// 3. check is cta file
// recover
TEST_F(DrmCtaPlugInTest, IsCtaFile_MultiMedia)
{
    printf("IsCtaFile_MultiMedia start\n");
    const char* path = "/sdcard/test.mp4";
    const char* cta_path = "/sdcard/test.mp4.cta";
    const char* backup_path = "/sdcard/test.mp4.bak";
    int original_file_size;

    // 1. backup original multimedia file
    printf("1. backup original multimedia file\n");
    // 1.1 check file exists
    ASSERT_EQ(0, access(path, F_OK)) << "please make sure the file [" << path << "] already existed in your device";
    // 1.2 backup
    system("cp /sdcard/test.mp4 /sdcard/test.mp4.bak");
    // 1.3 check backup file exists
    ASSERT_EQ(0, access(backup_path, F_OK)) << "reason: " << strerror(errno);

    // 2. encrypt multimedia file
    printf("2. encrypt multimedia file\n");
    setKey();
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    // parameters: action, mime, cipher fd, clear fd
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    // 2.1 create two file descriptor
    int clear_fd = open(path, O_RDWR);
    ASSERT_NE(-1, clear_fd);
    int cipher_fd = open(cta_path, O_CREAT, S_IRWXU);
    ASSERT_NE(-1, cipher_fd);

    char str_clear_fd[8] = {0};
    sprintf(str_clear_fd, "%d", clear_fd);
    char str_cipher_fd[8] = {0};
    sprintf(str_cipher_fd, "%d", cipher_fd);

    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_cipher_fd));

    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);

    // 2.2 encrypt
    original_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, original_file_size);
    lseek(clear_fd, 0, SEEK_SET);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

    // wait encrypt finish
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = 50 * 1000;
    while(!listener->done) {
        select(0, NULL, NULL, NULL, &delay);
    }

    // encrypt finish
    printf("encrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);

    ASSERT_TRUE(info != NULL);


    // 2.3 check file size whether has changed
    int cipher_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, cipher_file_size);
    ASSERT_NE(cipher_file_size, original_file_size);

    // 3. check whether is cta file
    printf("3. check whether is cta file\n");
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ISCTAFILE);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_FD, String8(str_clear_fd));

    listener->reset();
    info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    ASSERT_TRUE(info != NULL);
    ASSERT_FALSE(listener->hasError);

    char result[32] = {0};
    strncpy(result, info->getData().data, info->getData().length);
    printf("check result: %s\n", result);
    ASSERT_STREQ(DrmRequestType::RESULT_SUCCESS, result);



    // 4. recover
    printf("4. recover\n");
    close(cipher_fd);
    close(clear_fd);
    remove(path);
    remove(cta_path);
    system("mv /sdcard/test.mp4.bak /sdcard/test.mp4");

    delete info;
    delete listener;

    printf("IsCtaFile_MultiMedia finish\n");
}

// Create two files with the same content
// Encrypt a file, check whether is cta file
// multi media file case
// steps:
// 1. backup original multimedia file
// 2. encrypt multimedia file
// 3. cancel
// 4. compare file
// 5. recover
TEST_F(DrmCtaPlugInTest, CancelEncrypt_MultiMedia)
{
    printf("CancelEncrypt_MultiMedia start\n");
    const char* path = "/sdcard/test.mp4";
    const char* cta_path = "/sdcard/test.mp4.cta";
    const char* backup_path = "/sdcard/test.mp4.bak";
    int original_file_size;

    // 1. backup original multimedia file
    printf("1. backup original multimedia file\n");
    // 1.1 check file exists
    ASSERT_EQ(0, access(path, F_OK)) << "please make sure the file [" << path << "] already existed in your device";
    // 1.2 backup
    system("cp /sdcard/test.mp4 /sdcard/test.mp4.bak");
    // 1.3 check backup file exists
    ASSERT_EQ(0, access(backup_path, F_OK)) << "reason: " << strerror(errno);

    // 2. encrypt multimedia file
    printf("2. encrypt multimedia file\n");
    setKey();
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    // parameters: action, mime, cipher fd, clear fd
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    // 2.1 create two file descriptor
    int clear_fd = open(path, O_RDWR);
    ASSERT_NE(-1, clear_fd);
    int cipher_fd = open(cta_path, O_CREAT, S_IRWXU);
    ASSERT_NE(-1, cipher_fd);

    char str_clear_fd[8] = {0};
    sprintf(str_clear_fd, "%d", clear_fd);
    char str_cipher_fd[8] = {0};
    sprintf(str_cipher_fd, "%d", cipher_fd);

    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_cipher_fd));

    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    listener->inCancelCase = true;
    sPlugin->setOnInfoListener(sUniqeId, listener);

    // 2.2 encrypt
    original_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, original_file_size);
    lseek(clear_fd, 0, SEEK_SET);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

    // wait encrypt finish
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = 50 * 1000;
    while(!listener->done) {
        if(listener->needCancel) {
            // cancel encrypt
            printf("cancel encrypt\n");
            drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CANCEL);
            drmInfoReq.put(DrmRequestType::KEY_CTA5_FD, String8(str_clear_fd));
            info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

            listener->needCancel = false;
            listener->inCancelCase = false;
        }
        select(0, NULL, NULL, NULL, &delay);
    }

    // encrypt finish
    printf("encrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);

    ASSERT_TRUE(info != NULL);

    // 3. compare cancelled file with backup file
    printf("4. compare cancelled file with backup file\n");
    int backup_fd = open(backup_path, O_RDONLY);
    compareFile(clear_fd, backup_fd);

    // 5. recover
    printf("5. recover\n");
    close(backup_fd);
    close(cipher_fd);
    close(clear_fd);
    remove(cta_path);
    remove(backup_path);

    delete info;
    delete listener;

    printf("CancelEncrypt_MultiMedia finish\n");
}

// Create two files with the same content
// Encrypt a file, check whether is cta file
// multi media file case
// steps:
// 1. backup original cta multimedia file
// 2. decrypt multimedia file
// 3. cancel
// 4. compare file
// 5. recover
TEST_F(DrmCtaPlugInTest, CancelDecrypt_MultiMedia)
{
    printf("CancelDecrypt_MultiMedia start\n");
    const char* path = "/sdcard/test.mp4";
    const char* cta_path = "/sdcard/test.mp4.cta";
    const char* backup_path = "/sdcard/test.mp4.bak";
    const char* cipher_backup_path = "/sdcard/test.mp4.cipher.bak";
    int original_file_size;

    // 1. backup original multimedia file
    printf("1. backup original multimedia file\n");
    // 1.1 check file exists
    ASSERT_EQ(0, access(path, F_OK)) << "please make sure the file [" << path << "] already existed in your device";
    // 1.2 backup
    system("cp /sdcard/test.mp4 /sdcard/test.mp4.bak");
    // 1.3 check backup file exists
    ASSERT_EQ(0, access(backup_path, F_OK)) << "reason: " << strerror(errno);

    // 2. encrypt multimedia file
    printf("2. encrypt multimedia file\n");
    setKey();
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO, DrmDef::MIME_CTA5_MESSAGE);
    // parameters: action, mime, cipher fd, clear fd
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_ENCRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    // 2.1 create two file descriptor
    int clear_fd = open(path, O_RDWR);
    ASSERT_NE(-1, clear_fd);
    int cipher_fd = open(cta_path, O_CREAT, S_IRWXU);
    ASSERT_NE(-1, cipher_fd);

    char str_clear_fd[8] = {0};
    sprintf(str_clear_fd, "%d", clear_fd);
    char str_cipher_fd[8] = {0};
    sprintf(str_cipher_fd, "%d", cipher_fd);

    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_cipher_fd));

    // set listener
    CTAMultimediaTestListener* listener = new CTAMultimediaTestListener();
    sPlugin->setOnInfoListener(sUniqeId, listener);

    // 2.2 encrypt
    original_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, original_file_size);
    lseek(clear_fd, 0, SEEK_SET);

    DrmInfo *info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

    // wait encrypt finish
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = 50 * 1000;
    while(!listener->done) {
        select(0, NULL, NULL, NULL, &delay);
    }

    // encrypt finish
    printf("encrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);

    ASSERT_TRUE(info != NULL);


    // 2.3 check file size whether has changed
    int cipher_file_size = lseek(clear_fd, 0, SEEK_END);
    ASSERT_NE(-1, cipher_file_size);
    ASSERT_NE(cipher_file_size, original_file_size);

    // 3. backup cipher file
    system("cp /sdcard/test.mp4 /sdcard/test.mp4.cipher.bak");

    // 4. decrypt multimedia file
    printf("3. decrypt multimedia file\n");
    drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_DECRYPT);
    drmInfoReq.put(DrmRequestType::KEY_CTA5_RAW_MIME, String8("video/mp4"));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CIPHER_FD, String8(str_clear_fd));
    drmInfoReq.put(DrmRequestType::KEY_CTA5_CLEAR_FD, String8(str_cipher_fd));

    lseek(clear_fd, 0, SEEK_SET);
    listener->reset();
    listener->inCancelCase = true;
    info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);
    while(!listener->done) {
        if(listener->needCancel) {
            // cancel decrypt
            printf("cancel decrypt\n");
            drmInfoReq.put(DrmRequestType::KEY_ACTION, DrmRequestType::ACTION_CTA5_CANCEL);
            drmInfoReq.put(DrmRequestType::KEY_CTA5_FD, String8(str_clear_fd));
            info = sPlugin->acquireDrmInfo(sUniqeId, &drmInfoReq);

            listener->needCancel = false;
            listener->inCancelCase = false;
        }
        select(0, NULL, NULL, NULL, &delay);
    }

    // decrypt finish
    printf("decrypt multimedia file finish\n");
    // check whether has error
    ASSERT_FALSE(listener->hasError);
    ASSERT_TRUE(info != NULL);

    // 4. compare cancelled file with backup cipher file
    printf("4. compare cancelled file with backup file\n");
    int backup_cipher_fd = open(cipher_backup_path, O_RDONLY);
    compareFile(clear_fd, backup_cipher_fd);

    // 5. recover
    printf("5. recover\n");
    close(backup_cipher_fd);
    close(cipher_fd);
    close(clear_fd);
    remove(cta_path);
    remove(cipher_backup_path);
    remove(path);
    system("mv /sdcard/test.mp4.bak /sdcard/test.mp4");

    delete info;
    delete listener;

    printf("CancelDecrypt_MultiMedia finish\n");
}

int main(int argc, char** argv)
{
    InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    cout << "status = " << status << endl;
    return status;
}


bool DrmCtaPlugInTest::isFileEquals(String8 clearFile, String8 cipherFile)
{
    bool result = true;
    int clearFd = -1;
    clearFd = open(clearFile, O_RDWR, "0666");
    if (clearFd == -1)
    {
        cout << "open clear file failed" << endl;
    }

    int decryptFd = -1;
    decryptFd = open(cipherFile, O_RDWR, "0666");
    if (decryptFd == -1)
    {
        cout << "open decryptFd file failed" << endl;
    }


    int clearLength = lseek(clearFd, 0, SEEK_END);
    if (clearLength == -1)
    {
        cout << "lseek clearLength failed" << endl;
        //ALOGE("[ERROR]decrypt:lseek clearLength failed. reason=[%s]", strerror(errno));
        return false;
    }
    int decryptLength = lseek(decryptFd, 0, SEEK_END);
    if (decryptLength == -1)
    {
        cout << "lseek decryptLength failed" << endl;
        //ALOGE("[ERROR]decrypt:lseek decryptLength failed. reason=[%s]", strerror(errno));
        return false;
    }

    if (clearLength != decryptLength)
    {
        cout << "file length is not the same" << endl;
        return false;
    }
    int data_size = clearLength;
    lseek(clearFd, 0, SEEK_SET);
    lseek(decryptFd, 0, SEEK_SET);

    int cnt_total = 0; //  read cipher size
    int cnt = 0;

    int BLK_LEN = 4096;
    unsigned char buf_in[BLK_LEN];
    bzero(buf_in, sizeof(buf_in));
    unsigned char buf_de[BLK_LEN];
    bzero(buf_de, sizeof(buf_de));

    while (cnt_total < clearLength)
    {
        bzero(buf_in, sizeof(buf_in));
        bzero(buf_de, sizeof(buf_de));
        int needReadSize = ((data_size - cnt_total) > BLK_LEN) ? BLK_LEN : (data_size - cnt_total);
        cnt = read(clearFd, buf_in, needReadSize);
        cnt_total += cnt;

        cnt = read(decryptFd, buf_de, needReadSize);

        if (memcmp(buf_in, buf_de, needReadSize))
        {
            result = false;
            cout <<"file content not equal---"<< endl;
        }
    }
    close(clearFd);
    close(decryptFd);
    cout << "result:"<<result<<endl;
    return result;
}

int DrmCtaPlugInTest::writeToFile()
{
    int clearFd = -1;
    clearFd = open(clearPath, O_RDWR | O_TRUNC, "0666");
    if (clearFd == -1)
    {
        cout << "open clear file failed" << endl;
    }

    int result = 0;
    int target = 1;
    lseek(clearFd, 0, SEEK_SET);
    int BLK_LEN = 4096;
    unsigned char buf_out[BLK_LEN];
    bzero(buf_out, sizeof(buf_out));

    for (int i=0; i < BLK_LEN; i++)
    {
        buf_out[i] = 'a';
    }
    int min = 1000;
    int max = 10000;

    srand(time(NULL));
    target = rand()%(max-min+1) + min;
    cout << "target is:"<<target<<endl;

    //***************
    // write the content to target file
    int count = 0;
    while (count < target)
    {
        result += write(clearFd, buf_out, sizeof(buf_out));
        count++;
    }
    close(clearFd);
    return result;
}

bool DrmCtaPlugInTest::checkDrmInfoResult(DrmInfo* drmInfo,  String8 str)
{
    int length = str.length();
    char* backChar = new char[length+1];
    memset(backChar, 0, length+1);
    memcpy(backChar, drmInfo->getData().data, length);
    cout << "backChar:" << backChar << endl;
    int eq = memcmp(backChar, str.string(), length);
    cout << "result:" << eq << endl;
    delete[] backChar;
    return (eq==0);
}
