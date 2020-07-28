/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "vendor.trustonic.teeregistry@1.0-service"
//#define LOG_NDEBUG 0

#include <string.h>

#include <mutex>
#include <vector>

#include <utils/Log.h>
#include <hidlmemory/mapping.h>

#include <MobiCoreDriverApi.h>  // This file uses the MC error codes
#include "mcLoadFormat.h"
#include "mcVersionHelper.h"
#include "uuid_attestation.h"
#include "mcContainer.h"
#include <sys/stat.h>
#include <unistd.h>
#include "TeeRegistry.h"

namespace vendor {
namespace trustonic {
namespace teeregistry {
namespace V1_0 {
namespace implementation {

/** Maximum size of a shared object container in bytes. */
#define MAX_SO_CONT_SIZE  (512)
#define AUTH_TOKEN_FILE_NAME "00000000.authtokcont"
#define AUTH_TOKEN_FILE_NAME_BACKUP_SUFFIX ".backup"
#define ENV_MC_AUTH_TOKEN_PATH "MC_AUTH_TOKEN_PATH"
#define ROOT_FILE_NAME "00000000.rootcont"
#define SP_CONT_FILE_EXT ".spcont"
#define TL_CONT_FILE_EXT ".tlcont"
#define TL_BIN_FILE_EXT ".tlbin"
#define GP_TA_BIN_FILE_EXT ".tabin"
#define GP_TA_SPID_FILE_EXT ".spid"

#undef ALOGH
#ifdef TEESERVICE_DEBUG
#define ALOGH ALOGI
#else
#define ALOGH(...) do {} while(0)
#endif

#undef ALOGM
#ifdef TEESERVICE_MEMLEAK_CHECK
#include <malloc.h>
#define ALOGM(fmt, ...) \
    ALOGI(fmt " (MEMLEAK_CHECK %zu) [%s:%d:%s]", ##__VA_ARGS__, \
          static_cast<size_t>(mallinfo().uordblks), \
          __FILE__, __LINE__, __func__)
#else
#define ALOGM(...) do {} while(0)
#endif

struct TeeRegistry::Impl {
    enum class RegistryType {
        MC_REGISTRY_ALL,
        MC_REGISTRY_WRITABLE,
    };

    std::string registry_path;
    std::string tb_storage_path;

    Impl(const std::string& rp): registry_path(rp) {
        tb_storage_path = registry_path + "/TbStorage";
    }

    std::string getRootContFilePath() {
        return registry_path + "/" + ROOT_FILE_NAME;
    }

    std::string uint32ToString(uint32_t value) {
        char hx[8 + 1];
        snprintf(hx, sizeof(hx), "%08X", value);
        std::string str(hx);
        return std::string(str.rbegin(), str.rend());
    }

    bool dirExists(const char* path) {
        struct stat ss;
        return (path) && (stat(path, &ss) == 0) && S_ISDIR(ss.st_mode);
    }

    inline bool isAllZeros(const unsigned char* so, uint32_t size) {
        for (uint32_t i = 0; i < size; i++) {
            if (so[i]) {
                return false;
            }
        }
        return true;
    }

    std::string byteArrayToString(const void* bytes, size_t elems) {
        auto cbytes = static_cast<const char*>(bytes);
        char hx[elems * 2 + 1];
        for (size_t i = 0; i < elems; i++) {
            sprintf(&hx[i * 2], "%02x", cbytes[i]);
        }
        return std::string(hx);
    }

    uint32_t getAsUint32BE(
            const void* p_value_unaligned
            ) {
        auto p = static_cast<const uint8_t*>(p_value_unaligned);
        uint32_t val = p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24);
        return val;
    }

    std::string getAuthTokenFilePath() {
        const char* path;
        std::string auth_token_path;

        // First, attempt to use regular auth token path environment variable.
        path = getenv(ENV_MC_AUTH_TOKEN_PATH);
        if (path) {
            ALOGH("%s (path != NULL)", __func__);
            if (dirExists(path)) {
                ALOGH("getAuthTokenFilePath(): Using MC_AUTH_TOKEN_PATH %s", path);
                auth_token_path = path;
            }
        } else {
            auth_token_path = registry_path;
            ALOGH("getAuthTokenFilePath(): Using path %s", auth_token_path.c_str());
        }

        return auth_token_path + "/" + AUTH_TOKEN_FILE_NAME;
    }

    std::string getAuthTokenFilePathBackup() {
        return getAuthTokenFilePath() + AUTH_TOKEN_FILE_NAME_BACKUP_SUFFIX;
    }

    std::string getSpContFilePath(const mcSpid_t spid) {
        return registry_path + "/" + uint32ToString(spid) + SP_CONT_FILE_EXT;
    }

    std::string getTlContFilePath(const mcUuid_t& uuid,
                                  const mcSpid_t spid) {
        return registry_path + "/" + byteArrayToString(&uuid, sizeof(uuid))
                + "." + uint32ToString(spid) + TL_CONT_FILE_EXT;
    }

    std::string getSpDataPath(const mcSpid_t spid) {
        return registry_path + "/" + uint32ToString(spid);
    }

    std::string getTlDataPath(const mcUuid_t& uuid) {
        return registry_path + "/" + byteArrayToString(&uuid, sizeof(uuid));
    }

    std::string getGenericFilePath(const mcUuid_t& uuid,
                                   RegistryType r_type, const std::string& ext) {

        std::string name = byteArrayToString(&uuid, sizeof(uuid)) + ext;
        if (r_type == RegistryType::MC_REGISTRY_ALL) {
            std::string path = registry_path + "/" + name;
            if (access(path.c_str(), R_OK|W_OK) == 0)
                return path;
        }
        return registry_path + "/" + name;
    }

    std::string getTABinFilePath(const mcUuid_t& uuid, RegistryType r_type) {
        return getGenericFilePath(uuid, r_type, GP_TA_BIN_FILE_EXT);
    }

    std::string getTlBinFilePath(const mcUuid_t& uuid, RegistryType r_type) {
        return getGenericFilePath(uuid, r_type, TL_BIN_FILE_EXT);
    }

    std::string getTASpidFilePath(const mcUuid_t& uuid, RegistryType r_type) {
        return getGenericFilePath(uuid, r_type, GP_TA_SPID_FILE_EXT);
    }

    uint32_t mcRegistryReadSp(const mcSpid_t spid, void* so, uint32_t* size) {
        const std::string& sp_cont_file_path = getSpContFilePath(spid);
        size_t read_bytes;
        if ((spid == 0) || (!so)) {
            ALOGE("mcRegistry read So.Sp(SpId=0x%x) failed", spid);
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
        ALOGH(" Reading %s", sp_cont_file_path.c_str());

        FILE* fs = fopen(sp_cont_file_path.c_str(), "rb");
        if (!fs) {
            ALOGE("mcRegistry read %s failed: %d: %s", sp_cont_file_path.c_str(),
                  MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
            return MC_DRV_ERR_INVALID_DEVICE_FILE;
        }
        read_bytes = fread(so, 1, *size, fs);
        if (ferror(fs)) {
            fclose(fs);
            ALOGE("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
        fclose(fs);

        if (!(read_bytes > 0)) {
            ALOGE("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
            *size = -1;
            return MC_DRV_ERR_INVALID_DEVICE_FILE;

        }

        *size = static_cast<uint32_t>(read_bytes);
        ALOGH("mcRegistry Local read So.Sp(SpId) success");
        return MC_DRV_OK;
    }

    uint32_t getContentSize(const char* filename) {
        uint32_t content_size;
        FILE*   p_stream;

        /* Open the file */
        p_stream = fopen(filename, "rb");
        if (!p_stream) {
            ALOGE("Error: Cannot open file: %s.", filename);
            return 0;
        }

        if (fseek(p_stream, 0L, SEEK_END) != 0) {
            ALOGE("Error: Cannot read file: %s.", filename);
            fclose(p_stream);
            return 0;
        }

        content_size = ftell(p_stream);
        if (content_size < 0) {
            ALOGE("Error: Cannot get the file size: %s.", filename);
            fclose(p_stream);
            return 0;
        }

        if (content_size == 0) {
            ALOGE("Error: Empty file: %s.", filename);
            fclose(p_stream);
            return 0;
        }
        fclose(p_stream);
        return content_size;

    }

    std::string getFileContent(
            const char* p_path,
            uint32_t content_size
            ) {
        std::string content;
        FILE* p_stream = NULL;
        size_t res = 0;

        /* Open the file */
        p_stream = fopen(p_path, "rb");
        if (!p_stream) {
            ALOGE("Error: Cannot open file: %s.", p_path);
            goto error;
        }

        /* Read data from the file into the buffer */
        content.resize(content_size);
        res = fread(&content[0], content_size, 1, p_stream);
        if (ferror(p_stream)) {
            ALOGE("Error: Cannot read file: %s.", p_path);
            goto error;
        }

        if (res < 1) {
            //File is shorter than expected
            if (feof(p_stream)) {
                ALOGE("Error: EOF reached: %s.", p_path);
            }
            goto error;
        }

        /* Close the file */
        fclose(p_stream);
        p_stream = NULL;

        /* Return content buffer */
        return content;

error:
        if (p_stream) {
            fclose(p_stream);
        }
        return "";
    }

    bool isUuidOk(const mcUuid_t& uuid, const char* filename) {
        uint32_t    n_ta_size;

        n_ta_size = getContentSize(filename);

        if (n_ta_size == 0) {
            ALOGE("err: Trusted Application not found.");
            return false;
        }

        // Check blob size
        if (n_ta_size < sizeof(mclfHeaderV24_t)) {
            ALOGE("getFileContent failed - TA length is less than header size");
            return false;
        }
        auto p_ta_data = getFileContent(filename, n_ta_size);
        if (p_ta_data.size() == 0) {
            ALOGE("getFileContent failed - null content");
            return false;
        }

        auto header20 = reinterpret_cast<const mclfHeaderV2_t*>(p_ta_data.c_str());

        // Check header version
        if (header20->intro.version < MC_MAKE_VERSION(2, 4)) {
            ALOGE("isUuidOk() - TA blob header version is less than 2.4");
            return false;
        }

        // Check uuid
        return (memcmp(&uuid, &header20->uuid, sizeof(uuid)) == 0);
    }

    const std::string& getTbStoragePath() {
        return tb_storage_path;
    }

    //this function deletes all the files owned by a GP TA and stored in the tbase secure storage dir.
    //then it deletes GP TA folder.
    int CleanupGPTAStorage(const char* uuid) {
        ALOGH("%s was called", __func__);
        DIR*            p_dir;
        struct dirent*  p_dirent;
        int             err;
        std::string ta_path = getTbStoragePath() + "/" + uuid;
        if (nullptr != (p_dir = opendir(ta_path.c_str()))) {
            while (nullptr != (p_dirent = readdir(p_dir))) {
                if (p_dirent->d_name[0] != '.') {
                    std::string dname = ta_path + "/" + std::string(p_dirent->d_name);
                    ALOGH("delete DT: %s", dname.c_str());
                    if (0 != (err = remove(dname.c_str()))) {
                        ALOGE("remove UUID-files %s failed! error: %d", dname.c_str(), err);
                    }
                }
            }
            if (p_dir) {
                closedir(p_dir);
            }
            ALOGH("delete dir: %s", ta_path.c_str());
            if (0 != (err = rmdir(ta_path.c_str()))) {
                ALOGE("remove UUID-dir failed! errno: %d", err);
                return err;
            }
        }
        return MC_DRV_OK;
    }

    void deleteSPTA(const mcUuid_t& uuid, const mcSpid_t spid) {
        DIR*            p_dir;
        struct dirent*  p_dirent;
        int             err;

        // Delete TABIN and SPID files - we loop searching required spid file
        if (nullptr != (p_dir = opendir(registry_path.c_str()))) {
            while (nullptr != (p_dirent = readdir(p_dir))) {
                std::string spid_file;
                std::string tabin_file;
                std::string tabin_uuid;
                size_t pch_dot, pch_slash;
                spid_file = registry_path + "/" + std::string(p_dirent->d_name);
                pch_dot = spid_file.find_last_of('.');
                if (pch_dot == std::string::npos) {
                    continue;
                }
                pch_slash = spid_file.find_last_of('/');
                if ((pch_slash != std::string::npos) && (pch_slash > pch_dot)) {
                    continue;
                }
                if (spid_file.substr(pch_dot).compare(GP_TA_SPID_FILE_EXT) != 0) {
                    continue;
                }

                mcSpid_t curSpid = 0;

                int fd = open(spid_file.c_str(), O_RDONLY);
                if (fd != -1) {
                    if (read(fd, &curSpid, sizeof(mcSpid_t))!=sizeof(mcSpid_t)) {
                        curSpid = 0;
                    }
                    close(fd);
                }
                if (spid == curSpid) {
                    tabin_file =  spid_file.substr(0, pch_dot) + GP_TA_BIN_FILE_EXT;
                    if (isUuidOk(uuid, tabin_file.c_str())) {
                        tabin_uuid = spid_file.substr(0, pch_dot);
                        tabin_uuid = tabin_uuid.substr(tabin_uuid.find_last_of('/')+1);
                        ALOGH("Remove TA storage %s", tabin_uuid.c_str());
                        if (0 != (err = CleanupGPTAStorage(tabin_uuid.c_str()))) {
                            ALOGE("Remove TA storage failed! errno: %d", err);
                            /* Discard error */
                        }
                        ALOGH("Remove TA file %s", tabin_file.c_str());
                        if (0 != (err = remove(tabin_file.c_str()))) {
                            ALOGE("Remove TA file failed! errno: %d", err);
                            /* Discard error */
                        }
                        ALOGH("Remove spid file %s", spid_file.c_str());
                        if (0 != (err = remove(spid_file.c_str()))) {
                            ALOGE("Remove spid file failed! errno: %d", err);
                            /* Discard error */
                        }
                        break;
                    }
                }
            }
            if (p_dir) {
                closedir(p_dir);
            }
        }
    }

    uint32_t mcRegistryCleanupTrustlet(const mcUuid_t& uuid,
                                             const mcSpid_t spid) {
        DIR*            p_dir;
        struct dirent*  p_dirent;
        int             err;

        // Delete all TA related data
        std::string pathname = getTlDataPath(uuid);
        if (nullptr != (p_dir = opendir(pathname.c_str()))) {
            while (nullptr != (p_dirent = readdir(p_dir))) {
                if (p_dirent->d_name[0] != '.') {
                    std::string dname = pathname + "/" + std::string(p_dirent->d_name);
                    ALOGH("delete DT: %s", dname.c_str());
                    if (0 != (err = remove(dname.c_str()))) {
                        ALOGE("remove UUID-data %s failed! error: %d", dname.c_str(), err);
                    }
                }
            }
            if (p_dir) {
                closedir(p_dir);
            }
            ALOGH("delete dir: %s", pathname.c_str());
            if (0 != (err = rmdir(pathname.c_str()))) {
                ALOGE("remove UUID-dir failed! errno: %d", err);
                return MC_DRV_ERR_UNKNOWN;
            }
        }

        std::string tl_bin_file_path = getTlBinFilePath(uuid, RegistryType::MC_REGISTRY_WRITABLE);
        std::string tl_cont_file_path = getTlContFilePath(uuid, spid);;
        if (access(tl_bin_file_path.c_str(), R_OK|W_OK) == 0) {
            /* Legacy TA */
            ALOGH("Remove TA file %s", tl_bin_file_path.c_str());
            if (0 != (err = remove(tl_bin_file_path.c_str()))) {
                ALOGE("Remove TA file failed! errno: %d", err);
            }
        } else {
            /* GP TA */
            deleteSPTA(uuid, spid);
        }

        ALOGH("Remove TA container %s", tl_cont_file_path.c_str());
        if (0 != (err = remove(tl_cont_file_path.c_str()))) {
            ALOGE("Remove TA container failed! errno: %d", err);
            return MC_DRV_ERR_UNKNOWN;
        }

        return MC_DRV_OK;
    }

    uint32_t mcRegistryReadRoot(void* so, uint32_t* size) {
        const std::string& root_cont_file_path = getRootContFilePath();
        size_t read_bytes;

        if (!so) {
            ALOGE("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
        ALOGH(" Opening %s", root_cont_file_path.c_str());

        FILE* fs = fopen(root_cont_file_path.c_str(), "rb");
        if (!fs) {
            ALOGW("mcRegistry read %s failed: %d: %s", root_cont_file_path.c_str(),
                  MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
            return MC_DRV_ERR_INVALID_DEVICE_FILE;
        }
        read_bytes = fread(so, 1, *size, fs);
        if (ferror(fs)) {
            fclose(fs);
            ALOGE("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
        fclose(fs);

        if (read_bytes > 0) {
            *size = static_cast<uint32_t>(read_bytes);
            return MC_DRV_OK;
        } else {
            ALOGE("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
            return MC_DRV_ERR_INVALID_DEVICE_FILE;
        }
    }

    uint32_t mcRegistryCleanupSp(const mcSpid_t spid) {
        DIR* p_dir;
        struct dirent*  p_dirent;
        mcResult_t ret;
        mcSoSpCont_t data;
        uint32_t i, len;
        int err;

        if (0 == spid) {
            ALOGE("mcRegistry cleanupSP(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
        len = sizeof(mcSoSpCont_t);
        ret = mcRegistryReadSp(spid, &data, &len);
        if (MC_DRV_OK != ret || len != sizeof(mcSoSpCont_t)) {
            ALOGE("read SP->UUID aborted! Return code: %d", ret);
            return ret;
        }

        for (i = 0; (i < MC_CONT_CHILDREN_COUNT) && (ret == MC_DRV_OK); i++) {
            if (0 != strncmp(reinterpret_cast<const char*>(&data.cont.children[i]),
                             reinterpret_cast<const char*>(&MC_UUID_FREE),
                             sizeof(mcUuid_t))) {
                ret = mcRegistryCleanupTrustlet(data.cont.children[i], spid);
            }
        }
        if (MC_DRV_OK != ret) {
            ALOGE("delete SP->UUID failed! Return code: %d", ret);
            return ret;
        }

        std::string pathname = getSpDataPath(spid);

        if (nullptr != (p_dir = opendir(pathname.c_str()))) {
            while (nullptr != (p_dirent = readdir(p_dir))) {
                if (p_dirent->d_name[0] != '.') {
                    std::string dname = pathname + "/" + std::string(p_dirent->d_name);
                    ALOGH("delete DT: %s", dname.c_str());
                    if (0 != (err = remove(dname.c_str()))) {
                        ALOGE("remove SPID-data %s failed! error: %d", dname.c_str(), err);
                    }
                }
            }
            if (p_dir) {
                closedir(p_dir);
            }
            ALOGH("delete dir: %s", pathname.c_str());
            if (0 != (err = rmdir(pathname.c_str()))) {
                ALOGE("remove SPID-dir failed! error: %d", err);
                return MC_DRV_ERR_UNKNOWN;
            }
        }
        std::string sp_cont_file_path = getSpContFilePath(spid);
        ALOGH("delete Sp: %s", sp_cont_file_path.c_str());
        if (0 != (err = remove(sp_cont_file_path.c_str()))) {
            ALOGE("remove SP failed! error: %d", err);
            return MC_DRV_ERR_UNKNOWN;
        }
        return MC_DRV_OK;
    }
};

TeeRegistry::TeeRegistry(const std::string& registry_path):
    pimpl_(new Impl(registry_path)) {
    ALOGI("Registry path: '%s'", registry_path.c_str());
}

TeeRegistry::~TeeRegistry() {
}

Return<uint32_t> TeeRegistry::mcRegistryStoreAuthToken(
        const ::android::hardware::hidl_memory& so) {
    ALOGH("%s was called", __func__);
    int res = 0;

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return false;
    }
    void* data = memory_->getPointer();
    memory_->update();
    //

    size_t size = memory_->getSize();

    if (!data || size > 3 * MAX_SO_CONT_SIZE) {
        ALOGE("mcRegistry store So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    const std::string& auth_token_file_path = pimpl_->getAuthTokenFilePath();
    ALOGH("store AuthToken: %s", auth_token_file_path.c_str());

    /*
     * This special handling is needed because some of our OEM partners do not
     * supply the AuthToken to Kinibi Sphere. Instead, they retain the
     * AuthToken in a dedicated (reliable) storage area on the device. And in
     * that case, Kinibi Sphere is populated with a all-zero-padded AuthToken.
     * (Obviously the zero-padded AuthToken won't work, so we use that zero-
     * padding as an indicator to trigger the special behaviour.)
     *
     * When Kinibi Sphere supplies the device with the zero-padded AuthToken,
     * the device must look retrieve the AuthToken from its dedicated storage,
     * and use it in place of the zero-padded AuthToken supplied by Kinibi
     * Sphere. Since the AuthToken Backup will already have been retrieved from
     * the dedicated storage area by the time this method is called, using the
     * AuthToken Backup as our source is an acceptable proxy for retrieving it
     * from the dedicated storage area directly.
     *
     * This behaviour is triggered following Root.PA detecting a Factory Reset.
     */
    std::string backup;
    if (pimpl_->isAllZeros((const unsigned char*)data, static_cast<uint32_t>(size))) {
        std::string auth_token_file_path_backup = pimpl_->getAuthTokenFilePathBackup();

        ALOGH("AuthToken is all zeros");
        FILE* backupfs = fopen(auth_token_file_path_backup.c_str(), "rb");
        if (backupfs) {
            backup.resize(size);
            size_t readsize = fread(&backup[0], 1, size, backupfs);
            if (readsize == size) {
                ALOGH("AuthToken reset backup");
                data = &backup;
                memory_->commit();
            } else {
                ALOGE("AuthToken read size = %zu (%zu)", readsize, size);
            }
            fclose(backupfs);
        } else {
            ALOGW("can't open AuthToken %s", auth_token_file_path_backup.c_str());
        }
    }

    mcResult_t ret = MC_DRV_OK;
    FILE* fs = fopen(auth_token_file_path.c_str(), "wb");
    if (!fs) {
        ALOGW("mcRegistry store %s failed: %d: %s", auth_token_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        ret = MC_DRV_ERR_INVALID_DEVICE_FILE;
    } else {
        res = fseek(fs, 0, SEEK_SET);
        if (res != 0) {
            ret = MC_DRV_ERR_INVALID_PARAMETER;
        } else {
            fwrite(data, 1, size, fs);
            if (ferror(fs)) {
                ret = MC_DRV_ERR_OUT_OF_RESOURCES;
            }
        }
        fflush(fs);
        fclose(fs);
    }
    if (ret != MC_DRV_OK) {
        ALOGE("mcRegistry store So.Soc failed");
    }

    return ret;
}

Return<uint32_t> TeeRegistry::mcRegistryReadAuthToken(
        const ::android::hardware::hidl_memory& so) {
    ALOGH("%s was called", __func__);
    int res = 0;

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    void* data = memory_->getPointer();
    memory_->update();
    //

    if (!data) {
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    const std::string& auth_token_file_path = pimpl_->getAuthTokenFilePath();
    ALOGH("read AuthToken: %s", auth_token_file_path.c_str());

    FILE* fs = fopen(auth_token_file_path.c_str(), "rb");
    if (!fs) {
        ALOGW("mcRegistry read %s failed: %d: %s", auth_token_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    res = fseek(fs, 0, SEEK_END);
    if (res!=0) {
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    long filesize = ftell(fs);
    // We ensure that mcSoAuthTokenCont_t matches with filesize, as ferror (during fread operation) can't
    // handle the case where mcSoAuthTokenCont_t < filesize
    if (static_cast<long>(memory_->getSize()) != filesize) {
        fclose(fs);
        ALOGW("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    size_t read_res = fread(data, 1, static_cast<uint32_t>(memory_->getSize()), fs);
    memory_->commit();
    if (ferror(fs)) {
        fclose(fs);
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    if (read_res<static_cast<uint32_t>(memory_->getSize())) {
        //File is shorter than expected
        if (feof(fs)) {
            ALOGE("%s(): EOF reached: res is %zu, size of mcSoAuthTokenCont_t is %u",
                  __func__, read_res, static_cast<uint32_t>(memory_->getSize()));
        }
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    fclose(fs);

    return MC_DRV_OK;
}

Return<uint32_t> TeeRegistry::mcRegistryReadAuthTokenBackup(
        const ::android::hardware::hidl_memory& so) {
    ALOGH("%s was called", __func__);
    int res = 0;

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    void* data = memory_->getPointer();
    memory_->update();
    //

    if (!data) {
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    const std::string& auth_token_file_path = pimpl_->getAuthTokenFilePathBackup();
    ALOGH("read AuthTokenBackup: %s", auth_token_file_path.c_str());

    FILE* fs = fopen(auth_token_file_path.c_str(), "rb");
    if (!fs) {
        ALOGW("mcRegistry read %s failed: %d: %s", auth_token_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    res = fseek(fs, 0, SEEK_END);
    if (res!=0) {
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    long filesize = ftell(fs);
    // We ensure that mcSoAuthTokenCont_t matches with filesize, as ferror (during fread operation) can't
    // handle the case where mcSoAuthTokenCont_t < filesize
    if (static_cast<long>(memory_->getSize()) != filesize) {
        fclose(fs);
        ALOGW("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    size_t read_res = fread(data, 1, static_cast<uint32_t>(memory_->getSize()), fs);
    memory_->commit();
    if (ferror(fs)) {
        fclose(fs);
        ALOGE("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    if (read_res<static_cast<uint32_t>(memory_->getSize())) {
        //File is shorter than expected
        if (feof(fs)) {
            ALOGE("%s(): EOF reached: res is %zu, size of data is %zu",
                  __func__,
                  read_res, static_cast<size_t>(memory_->getSize()));
        }
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    fclose(fs);

    return MC_DRV_OK;
}

Return<uint32_t> TeeRegistry::mcRegistryDeleteAuthToken() {
    ALOGH("%s was called", __func__);
    if (::rename(pimpl_->getAuthTokenFilePath().c_str(),
                 pimpl_->getAuthTokenFilePathBackup().c_str())) {
        ALOGE("%s renaming Auth token file", ::strerror(errno));
        return MC_DRV_ERR_UNKNOWN;
    } else {
        return MC_DRV_OK;
    }
}

Return<uint32_t> TeeRegistry::mcRegistryStoreRoot(
        const ::android::hardware::hidl_memory& so) {
    ALOGH("%s was called", __func__);
    int res = 0;

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    void* data = memory_->getPointer();
    memory_->update();
    //

    auto size = static_cast<uint32_t>(memory_->getSize());
    if (!data || size > 3 * MAX_SO_CONT_SIZE) {
        ALOGE("mcRegistry store So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    const std::string& root_cont_file_path = pimpl_->getRootContFilePath();
    ALOGH("store Root: %s", root_cont_file_path.c_str());

    FILE* fs = fopen(root_cont_file_path.c_str(), "wb");
    if (!fs) {
        ALOGE("mcRegistry store %s failed: %d  : %s", root_cont_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("mcRegistry store So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    fwrite(data, 1, size, fs);
    if (ferror(fs)) {
        ALOGE("mcRegistry store So.Root failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        fclose(fs);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}

Return<void> TeeRegistry::mcRegistryReadRoot(
        const ::android::hardware::hidl_memory& so,
        mcRegistryReadRoot_cb _hidl_cb) {
    ALOGH("%s was called", __func__);
    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    void* data = memory_->getPointer();
    memory_->update();
    //

    auto size = static_cast<uint32_t>(memory_->getSize());

    const std::string& root_cont_file_path = pimpl_->getRootContFilePath();
    size_t readBytes;

    if (!data) {
        ALOGE("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    ALOGH(" Opening %s", root_cont_file_path.c_str());

    FILE* fs = fopen(root_cont_file_path.c_str(), "rb");
    if (!fs) {
        ALOGW("mcRegistry read %s failed: %d can't open file : %s", root_cont_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        _hidl_cb(MC_DRV_ERR_INVALID_DEVICE_FILE, 0);
        return Void();
    }
    readBytes = fread(data, 1, size, fs);
    memory_->commit();
    if (ferror(fs)) {
        fclose(fs);
        ALOGE("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    fclose(fs);

    if (!(readBytes > 0)) {
        ALOGE("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        _hidl_cb(MC_DRV_ERR_INVALID_DEVICE_FILE, 0);
        return Void();
    }

    size = static_cast<uint32_t>(readBytes);
    ALOGH("mcRegistry read So.Root success");
    _hidl_cb(MC_DRV_OK, size);
    return Void();
}

Return<uint32_t> TeeRegistry::mcRegistryStoreSp(uint32_t spid,
                                                const ::android::hardware::hidl_memory& so) {
    ALOGH("%s was called", __func__);
    int res = 0;

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    void* data = memory_->getPointer();
    memory_->update();
    auto size = static_cast<uint32_t>(memory_->getSize());

    if ((spid == 0) || !data || size > 3 * MAX_SO_CONT_SIZE) {
        ALOGE("mcRegistry store So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    const std::string& sp_cont_file_path = pimpl_->getSpContFilePath(spid);
    ALOGH("store SP: %s", sp_cont_file_path.c_str());

    FILE* fs = fopen(sp_cont_file_path.c_str(), "wb");
    if (!fs) {
        ALOGE("mcRegistry store %s failed: %d : %s", sp_cont_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("mcRegistry store So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    fwrite(data, 1, size, fs);
    if (ferror(fs)) {
        ALOGE("mcRegistry store So.Sp(SpId) failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        fclose(fs);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}

Return<void> TeeRegistry::mcRegistryReadSp(
        uint32_t spid,
        const ::android::hardware::hidl_memory& so,
        mcRegistryReadSp_cb _hidl_cb) {
    ALOGH("%s was called", __func__);

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    void* data = memory_->getPointer();
    memory_->update();
    auto size = static_cast<uint32_t>(memory_->getSize());

    const std::string& sp_cont_file_path = pimpl_->getSpContFilePath(spid);
    size_t readBytes;
    if ((spid == 0) || !data) {
        ALOGE("mcRegistry read So.Sp(SpId=0x%x) failed", spid);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    ALOGH(" Reading %s", sp_cont_file_path.c_str());

    FILE* fs = fopen(sp_cont_file_path.c_str(), "rb");
    if (!fs) {
        ALOGE("mcRegistry read %s failed: can't open file :%d :%s", sp_cont_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        _hidl_cb(MC_DRV_ERR_INVALID_DEVICE_FILE, 0);
        return Void();
    }
    readBytes = fread(data, 1, size, fs);
    memory_->commit();
    if (ferror(fs)) {
        fclose(fs);
        ALOGE("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    fclose(fs);

    if (!(readBytes > 0)) {
        ALOGE("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        _hidl_cb(MC_DRV_ERR_INVALID_DEVICE_FILE, 0);
        return Void();
    }

    _hidl_cb(MC_DRV_OK, static_cast<uint32_t>(readBytes));
    return Void();
}

Return<uint32_t> TeeRegistry::mcRegistryCleanupSp(uint32_t spid) {
    ALOGH("%s was called", __func__);
    DIR* p_dir;
    struct dirent*  p_dirent;
    mcResult_t ret;
    mcSoSpCont_t data;
    uint32_t i, len;
    int err;

    if (0 == spid) {
        ALOGE("mcRegistry cleanupSP(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    len = sizeof(mcSoSpCont_t);

    ret = pimpl_->mcRegistryReadSp(spid, &data, &len);

    if (MC_DRV_OK != ret || len != sizeof(mcSoSpCont_t)) {
        ALOGE("read SP->UUID aborted! Return code: %d", ret);
        return ret;
    }
    for (i = 0; (i < MC_CONT_CHILDREN_COUNT) && (ret == MC_DRV_OK); i++) {
        if (0 != strncmp(reinterpret_cast<const char*>(&data.cont.children[i]),
                         reinterpret_cast<const char*>(&MC_UUID_FREE),
                         sizeof(mcUuid_t))) {
            ret = pimpl_->mcRegistryCleanupTrustlet(data.cont.children[i], spid);
        }
    }
    if (MC_DRV_OK != ret) {
        ALOGE("delete SP->UUID failed! Return code: %d", ret);
        return ret;
    }

    std::string pathname = pimpl_->getSpDataPath(spid);

    if (nullptr != (p_dir = opendir(pathname.c_str()))) {
        while (nullptr != (p_dirent = readdir(p_dir))) {
            if (p_dirent->d_name[0] != '.') {
                std::string dname = pathname + "/" + std::string(p_dirent->d_name);
                ALOGH("delete DT: %s", dname.c_str());
                if (0 != (err = remove(dname.c_str()))) {
                    ALOGE("remove SPID-data %s failed! error: %d", dname.c_str(), err);
                }
            }
        }
        if (p_dir) {
            closedir(p_dir);
        }
        ALOGH("delete dir: %s", pathname.c_str());
        if (0 != (err = rmdir(pathname.c_str()))) {
            ALOGE("remove SPID-dir failed! error: %d", err);
            return MC_DRV_ERR_UNKNOWN;
        }
    }
    std::string sp_cont_file_path = pimpl_->getSpContFilePath(spid);
    ALOGH("delete Sp: %s", sp_cont_file_path.c_str());
    if (0 != (err = remove(sp_cont_file_path.c_str()))) {
        ALOGE("remove SP failed! error: %d", err);
        return MC_DRV_ERR_UNKNOWN;
    }
    return MC_DRV_OK;
}

Return<uint32_t> TeeRegistry::mcRegistryStoreTrustletCon(
        const ::android::hardware::hidl_vec<uint8_t>& uuid,
        uint32_t spid,
        const ::android::hardware::hidl_memory& so) {
    ALOGH("%s was called", __func__);

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    void* data = memory_->getPointer();
    memory_->update();
    //
    auto size = static_cast<uint32_t>(memory_->getSize());

    int res = 0;
    if ((uuid == NULL) || !data || size > 3 * MAX_SO_CONT_SIZE) {
        ALOGE("mcRegistry store So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    mcUuid_t mcregistry_uuid;
    ::memcpy(&mcregistry_uuid, &uuid[0], uuid.size());
    const std::string& tl_cont_file_path =
            pimpl_->getTlContFilePath(mcregistry_uuid, spid);
    ALOGH("store TLc: %s", tl_cont_file_path.c_str());

    FILE* fs = fopen(tl_cont_file_path.c_str(), "wb");
    if (!fs) {
        ALOGE("mcRegistry store %s failed: %d: %s", tl_cont_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("mcRegistry store So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    fwrite(data, 1, size, fs);
    if (ferror(fs)) {
        ALOGE("mcRegistry store So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_OUT_OF_RESOURCES);
        fclose(fs);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}

Return<void> TeeRegistry::mcRegistryReadTrustletCon(
        const ::android::hardware::hidl_vec<uint8_t>& uuid,
        uint32_t spid,
        const ::android::hardware::hidl_memory& so,
        mcRegistryReadTrustletCon_cb _hidl_cb) {
    ALOGH("%s was called", __func__);

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(so);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    void* data = memory_->getPointer();
    memory_->update();
    //
    auto size = static_cast<uint32_t>(memory_->getSize());

    int res = 0;
    if ((uuid == NULL) || !data) {
        ALOGE("mcRegistry read So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_INVALID_PARAMETER);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    size_t readBytes;

    mcUuid_t mcRegistry_uuid;
    ::memcpy(&mcRegistry_uuid, &uuid[0], uuid.size());
    const std::string& tl_cont_file_path =
            pimpl_->getTlContFilePath(mcRegistry_uuid, spid);
    ALOGH("read TLc: %s", tl_cont_file_path.c_str());

    FILE* fs = fopen(tl_cont_file_path.c_str(), "rb");
    if (!fs) {
        ALOGE("mcRegistry read %s failed: %d: %s", tl_cont_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        _hidl_cb(MC_DRV_ERR_INVALID_DEVICE_FILE, 0);
        return Void();
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("mcRegistry read So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    readBytes = fread(data, 1, size, fs);
    memory_->commit();
    if (ferror(fs)) {
        fclose(fs);
        ALOGE("mcRegistry read So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_INVALID_PARAMETER);
        _hidl_cb(MC_DRV_ERR_INVALID_PARAMETER, 0);
        return Void();
    }
    fclose(fs);

    if (!(readBytes > 0)) {
        ALOGE("mcRegistry read So.TrustletCont(uuid) failed: %d",
              MC_DRV_ERR_INVALID_DEVICE_FILE);
        _hidl_cb(MC_DRV_ERR_INVALID_DEVICE_FILE, 0);
        return Void();
    }

    _hidl_cb(MC_DRV_OK, static_cast<uint32_t>(readBytes));
    return Void();
}

Return<uint32_t> TeeRegistry::mcRegistryCleanupTrustlet(
        const ::android::hardware::hidl_vec<uint8_t>& uuid,
        uint32_t spid) {
    ALOGH("%s was called", __func__);

    DIR*            p_dir;
    struct dirent*  p_dirent;
    int             err;

    if (uuid == NULL) {
        ALOGE("mcRegistry cleanupTrustlet(uuid) failed: %d",
              MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    mcUuid_t mcregistry_uuid;
    ::memcpy(&mcregistry_uuid, &uuid[0], uuid.size());
    // Delete all TA related data
    std::string pathname = pimpl_->getTlDataPath(mcregistry_uuid);
    if (nullptr != (p_dir = ::opendir(pathname.c_str()))) {
        while (nullptr != (p_dirent = ::readdir(p_dir))) {
            if (p_dirent->d_name[0] != '.') {
                std::string dname = pathname + "/" + std::string(p_dirent->d_name);
                ALOGH("delete DT: %s", dname.c_str());
                if (0 != (err = remove(dname.c_str()))) {
                    ALOGE("remove UUID-data %s failed! error: %d", dname.c_str(), err);
                }
            }
        }
        if (p_dir) {
            ::closedir(p_dir);
        }
        ALOGH("delete dir: %s", pathname.c_str());
        if (0 != (err = ::rmdir(pathname.c_str()))) {
            ALOGE("remove UUID-dir failed! errno: %d", err);
            return MC_DRV_ERR_UNKNOWN;
        }
    }

    std::string tl_bin_file_path =
            pimpl_->getTlBinFilePath(mcregistry_uuid, Impl::RegistryType::MC_REGISTRY_WRITABLE);
    std::string tl_cont_file_path =
            pimpl_->getTlContFilePath(mcregistry_uuid, spid);

    if (access(tl_bin_file_path.c_str(), R_OK|W_OK) == 0) {
        /* Legacy TA */
        ALOGH("Remove TA file %s", tl_bin_file_path.c_str());
        if (0 != (err = remove(tl_bin_file_path.c_str()))) {
            ALOGE("Remove TA file failed! errno: %d", err);
        }
    } else {
        /* GP TA */
        pimpl_->deleteSPTA(mcregistry_uuid, spid);
    }

    ALOGH("Remove TA container %s", tl_cont_file_path.c_str());
    if (0 != (err = remove(tl_cont_file_path.c_str()))) {
        ALOGE("Remove TA container failed! errno: %d", err);
        return MC_DRV_ERR_UNKNOWN;
    }

    return MC_DRV_OK;
}

Return<uint32_t> TeeRegistry::mcRegistryCleanupTA(
        const ::android::hardware::hidl_vec<uint8_t>& uuid) {
    ALOGH("%s was called", __func__);
    mcUuid_t mcregistry_uuid;
    ::memcpy(&mcregistry_uuid, &uuid[0], uuid.size());
    return pimpl_->CleanupGPTAStorage(pimpl_->byteArrayToString(&mcregistry_uuid, sizeof(mcregistry_uuid)).c_str());
}

Return<uint32_t> TeeRegistry::mcRegistryCleanupRoot() {
    ALOGH("%s was called", __func__);
    mcResult_t ret;
    mcSoRootCont_t data;
    uint32_t i, len;
    int e;
    len = sizeof(mcSoRootCont_t);
    ret = pimpl_->mcRegistryReadRoot(&data, &len);
    if (MC_DRV_OK != ret || len != sizeof(mcSoRootCont_t)) {
        ALOGE("read Root aborted! Return code: %d", ret);
        return ret;
    }
    for (i = 0; (i < MC_CONT_CHILDREN_COUNT) && (ret == MC_DRV_OK); i++) {
        mcSpid_t spid = data.cont.children[i];
        if (spid != MC_SPID_FREE) {
            ret = pimpl_->mcRegistryCleanupSp(spid);
            if (MC_DRV_OK != ret) {
                ALOGE("Cleanup SP failed! Return code: %d", ret);
                return ret;
            }
        }
    }

    std::string root_cont_file_path = pimpl_->getRootContFilePath();
    ALOGH("Delete root: %s", root_cont_file_path.c_str());
    if (0 != (e = remove(root_cont_file_path.c_str()))) {
        ALOGE("Delete root failed! error: %d", e);
        return MC_DRV_ERR_UNKNOWN;
    }
    return MC_DRV_OK;
}

Return<uint32_t> TeeRegistry::mcRegistryStoreTABlob(
        uint32_t spid,
        const ::android::hardware::hidl_memory& blob) {
    int res = 0;
    ALOGH("mcRegistryStoreTABlob started");

    //memory stuff
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    memory_ = ::android::hardware::mapMemory(blob);
    if (!memory_) {
        ALOGE("Failed to map source buffer.");
        return false;
    }
    void* data = memory_->getPointer();
    memory_->update();
    //

    auto size = static_cast<uint32_t>(memory_->getSize());

    // Check blob size
    if (size < sizeof(mclfHeaderV24_t)) {
        ALOGE("RegistryStoreTABlob failed - TA blob length is less then header size");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    auto header24 = reinterpret_cast<const mclfHeaderV24_t*>(data);
    auto header20 = reinterpret_cast<const mclfHeaderV2_t*>(data);

    // Check header version
    if (header20->intro.version < MC_MAKE_VERSION(2, 4)) {
        ALOGE("RegistryStoreTABlob failed - TA blob header version is less than 2.4");
        return MC_DRV_ERR_TA_HEADER_ERROR;
    }

    //Check GP version
    if (header24->gp_level != 1) {
        ALOGE("RegistryStoreTABlob failed - TA blob header gp_level is not equal to 1");
        return MC_DRV_ERR_TA_HEADER_ERROR;
    }

    mcUuid_t uuid;
    switch (header20->serviceType) {
        case SERVICE_TYPE_SYSTEM_TRUSTLET: {
            // Check spid
            if (spid != MC_SPID_SYSTEM) {
                ALOGE("RegistryStoreTABlob failed - SPID is not equal to %d for System TA",
                      spid);
                return MC_DRV_ERR_INVALID_PARAMETER;
            }
            memcpy(&uuid, &header20->uuid, sizeof(mcUuid_t));
            break;
        }
        case SERVICE_TYPE_SP_TRUSTLET: {
            // Check spid
            if (spid >= MC_SPID_SYSTEM) {
                ALOGE("RegistryStoreTABlob failed - SPID is equal to %u ", spid);
                return MC_DRV_ERR_INVALID_PARAMETER;
            }

            auto pUa = reinterpret_cast<const uuid_attestation_params_t*>
                       (&static_cast<const uint8_t*>(data)[header24->attestationOffset]);
            // Check attestation size
            if ((header24->attestationOffset > size) &&
                (header24->attestationOffset + pimpl_->getAsUint32BE(&pUa->size) > size)) {
                ALOGE("RegistryStoreTABlob failed - Attestation size is not correct");
                return MC_DRV_ERR_TA_HEADER_ERROR;
            }

            // Check attestation size
            if (pimpl_->getAsUint32BE(&pUa->size) < sizeof(uuid_attestation_params_t)) {
                ALOGE("RegistryStoreTABlob failed - Attestation size is equal to %d and is less then %zu",
                      pimpl_->getAsUint32BE(&pUa->size), sizeof(uuid_attestation_params_t));
                return MC_DRV_ERR_TA_ATTESTATION_ERROR;
            }

            // Check magic word
            if (::memcmp(pUa->magic, MAGIC, AT_MAGIC_SIZE)) {
                ALOGE("RegistryStoreTABlob failed - Attestation magic word is not correct");
                return MC_DRV_ERR_TA_ATTESTATION_ERROR;
            }

            // Check version
            if (pimpl_->getAsUint32BE(&pUa->version) != AT_VERSION) {
                ALOGE("RegistryStoreTABlob failed - Attestation version is equal to %08X. It has to be equal to %08X",
                      pimpl_->getAsUint32BE(&pUa->version), AT_VERSION);
                return MC_DRV_ERR_TA_ATTESTATION_ERROR;
            }

            ::memcpy(&uuid, &pUa->uuid, sizeof(mcUuid_t));
            break;
        }
        default: {
            ALOGE("RegistryStoreTABlob failed");
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
    }
    const std::string ta_bin_file_path =
            pimpl_->getTABinFilePath(uuid, Impl::RegistryType::MC_REGISTRY_WRITABLE);

    ALOGH("Store TA blob at: %s", ta_bin_file_path.c_str());

    FILE* fs = fopen(ta_bin_file_path.c_str(), "wb");
    if (!fs) {
        ALOGE("RegistryStoreTABlob failed - %s open error: %d: %s", ta_bin_file_path.c_str(),
              MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    res = fseek(fs, 0, SEEK_SET);
    if (res!=0) {
        ALOGE("RegistryStoreTABlob failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        fclose(fs);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    fwrite(data, 1, size, fs);
    if (ferror(fs)) {
        ALOGE("RegistryStoreTABlob failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        fclose(fs);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    fflush(fs);
    fclose(fs);

    if (header20->serviceType == SERVICE_TYPE_SP_TRUSTLET) {
        const std::string ta_spid_file_path =
                pimpl_->getTASpidFilePath(uuid, Impl::RegistryType::MC_REGISTRY_WRITABLE);

        ALOGH("Store spid file at: %s", ta_spid_file_path.c_str());

        FILE* fs = fopen(ta_spid_file_path.c_str(), "wb");
        if (!fs) {
            ALOGE("RegistryStoreTABlob failed - %s open error: %d: %s", ta_bin_file_path.c_str(),
                  MC_DRV_ERR_INVALID_DEVICE_FILE, std::strerror(errno));
            return MC_DRV_ERR_INVALID_DEVICE_FILE;
        }
        res = fseek(fs, 0, SEEK_SET);
        if (res!=0) {
            ALOGE("RegistryStoreTABlob failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
            fclose(fs);
            return MC_DRV_ERR_INVALID_PARAMETER;
        }
        fwrite(&spid, 1, sizeof(mcSpid_t), fs);
        if (ferror(fs)) {
            ALOGE("RegistryStoreTABlob failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
            fclose(fs);
            return MC_DRV_ERR_OUT_OF_RESOURCES;
        }
        fflush(fs);
        fclose(fs);
    }
    return MC_DRV_OK;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace teeregistry
}  // namespace trustonic
}  // namespace vendor
