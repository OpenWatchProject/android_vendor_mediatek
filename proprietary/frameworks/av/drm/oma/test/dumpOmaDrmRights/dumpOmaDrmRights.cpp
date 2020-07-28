#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rc4.h>
#include <ByteBuffer.h>
#include <utils/String8.h>
#include <DrmUtil.h>
#include <RO.h>
#include <StrUtil.h>
#include <FileUtil.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace android;

static const int OLD_STYLE_FL_CID_LENGTH = 28;

static const int VER_01_0 = 10; // the value indicating Rights Object version
static const char* RO_VERSION_01_0 = "VER_01_0"; // the version string tag, placed at the beginning of RO file
static const int SIZE_VER_01_0  = 9; // the length of string tag

void dump(RO *pRO);
bool restore(String8 cid, RO* ro, String8 &deviceId);
bool restoreRights(FILE* fp, RO* ro, String8 &deviceId);
void rc4_decrypt_buffer(unsigned char* toBuffer, unsigned char* fromBuffer, int size, String8 &deviceId);
void rc4_encrypt_buffer(unsigned char* toBuffer, unsigned char* fromBuffer, int size, String8 &deviceId);
ByteBuffer getDrmKey(String8 &deviceId);
static void convertRO(Rights* rights, OldRights* oldRights);
bool restoreEntry(String8 ro_file, FILE* fp, RO* ro, String8 &deviceId);

int getRightsPosition()
{
    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Usage: %s ro_full_name deviceId\n",argv[0]);
        printf("cid is the rights object's id\n");
        return -1;
    }
    RO oldRO;
    String8 deviceId(argv[2]);
    printf("deviceId = %s\n",deviceId.string());
    if (!restore(String8(argv[1]), &oldRO, deviceId))
    {
        printf("restore fail\n");
        return 1;
    }

    dump(&oldRO);

    printf("restore success\n");
    return 0;
}


void dump_entry(Entry *pEntry)
{
    // get String8 for type  ---->
    String8 typeStr("");
    for (int i = DrmDef::CONSTRAINT_COUNT; i <= DrmDef::CONSTRAINT_NONE; i = i << 1)
    {
        if (0 != (pEntry->type & i))
        {
            typeStr += DrmDef::getConstraintStr(i);
        }
        else
        {
            typeStr += "_";
        }
        if (i != DrmDef::CONSTRAINT_NONE)
        {
            typeStr += "|";
        }
    }
    // get String8 for type <----

    // get String8 for constraints ---->
    String8 conStr("");
    conStr = conStr + "used_count[" + StrUtil::toString(pEntry->used_count) + "] ";
    conStr = conStr + "total_count[" + StrUtil::toString(pEntry->total_count) + "] ";

    String8 dt_start_str = StrUtil::toTimeStr((time_t*)(&pEntry->start_time));
    String8 dt_end_str = StrUtil::toTimeStr((time_t*)(&pEntry->end_time));
    String8 start_intv_str = StrUtil::toTimeStr((time_t*)(&pEntry->start_intv));
    String8 end_intv_str = StrUtil::toTimeStr((time_t*)(&pEntry->end_intv));

    conStr = conStr + "start_time[" + StrUtil::toString(pEntry->start_time) + "]" + dt_start_str + " ";
    conStr = conStr + "end_time[" + StrUtil::toString(pEntry->end_time) + "]" + dt_end_str + " ";
    conStr = conStr + "interval[" + StrUtil::toString(pEntry->interval) + "] ";
    conStr = conStr + "start_intv[" + StrUtil::toString(pEntry->start_intv) + "]" + start_intv_str + " ";
    conStr = conStr + "end_intv[" + StrUtil::toString(pEntry->end_intv) + "]" + end_intv_str + " ";
    // get String8 for constraints <----
    printf("        next[%s] type[%s] %s\n", StrUtil::toString(pEntry->next), typeStr.string(), conStr.string());
}

void dump_rights(Rights *pRights)
{
    printf("Rights ---->\n");
    printf("%s\n", pRights->getGeneralStr().string());

    // print constraint by permission ---->
    for (int i = DrmDef::PERMISSION_PLAY_INDEX; i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        String8 pStr = DrmDef::getPermissionStr(i);
        printf("[%s] ---->\n", pStr.string());
        // print constraint list ---->
        if (pRights->best[i] != 0xFF) // have constraint for the permission
        {
            int entryIndex = pRights->best[i];
            while (entryIndex != 0xFF)
            {
                Entry* entry = &pRights->entryPtr[entryIndex];
                // print entry ---->
                printf("    entry[%d] ---->\n", entryIndex);
                //entry->dump();
                dump_entry(entry);
                printf("    entry[%d] <----\n", entryIndex);
                // print entry <----
                entryIndex = entry->next;
            }
        }
        // print constraint list <----
        printf("[%s] <----\n", pStr.string());
    }
    // print constraint by permission <----
    printf("Rights <----\n");
}



void dump(RO *pRO)
{
    printf("RO ---->\n");
    dump_rights(&pRO->rights);
    printf("RO <----\n");
}

// ro should be an clear ro
bool restore(String8 ro_file, RO* ro, String8 &deviceId)
{
    printf("restore ----> ro_file [%s]\n", ro_file.string());

    //String8 ro_file = getROFileName(cid);
    FILE* fp = NULL;
    if (!FileUtil::fopenx(ro_file.string(), "rb", &fp))
    {
        printf("restore: failed to open ro file.");
        return false;
    }

    if (!restoreRights(fp, ro, deviceId))
    {
        printf("restore: failed to restore rights from ro file.\n");
        FileUtil::fclosex(fp);
        if (DrmUtil::checkExistence(ro_file))
        {
            printf("restore: delete existing ro file.\n");
            remove(ro_file.string());
        }
        return false;
    }

    if (!restoreEntry(ro_file, fp, ro, deviceId))
    {
        printf("restore: failed to restore entries from ro file.\n");
        FileUtil::fclosex(fp);
        if (DrmUtil::checkExistence(ro_file))
        {
            printf("restore: delete existing ro file.\n");
            remove(ro_file.string());
        }
        return false;
    }

    FileUtil::fclosex(fp);
    return true;
}


bool restoreRights(FILE* fp, RO* ro, String8 &deviceId)
{
    FileUtil::fseekx(fp, getRightsPosition(), SEEK_SET);
    // first we check the beginning of the RO file for RO_VERSION_01_0 string.
    char ver[SIZE_VER_01_0];
    bzero(ver, SIZE_VER_01_0);
    if (!FileUtil::freadx(ver, strlen(RO_VERSION_01_0), fp))
    {
        ALOGE("restoreRights: failed to read possible rights object verison from file.");
        return false;
    }

    ALOGV("restoreRights: compare [%s] with [%s]", ver, RO_VERSION_01_0);
    int version = 0;
    if (0 == strcmp(ver, RO_VERSION_01_0))
    {
        version = VER_01_0;
    }

    size_t ro_size = (0 == version) ? sizeof(struct OldRights) : sizeof(struct Rights);
    unsigned char* encryptedRightsBuffer = new unsigned char[ro_size];
    bzero(encryptedRightsBuffer, ro_size);

    // if old RO struct, then go back to the beginning of file
    if (0 == version)
    {
        ALOGD("restoreRights: old-style rights object.");
        FileUtil::fseekx(fp, 0, SEEK_SET);
    }
    if (!FileUtil::freadx(encryptedRightsBuffer, ro_size, fp))
    {
        ALOGE("restoreRights: failed to read rights data from file.");
        delete[] encryptedRightsBuffer;
        return false;
    }

    // old RO struct compatibility, and we decrypt according to the amount of data
    OldRights old_rights;
    unsigned char* output = (0 == version) ? (unsigned char*)&old_rights : (unsigned char*)&ro->rights;
    rc4_decrypt_buffer(output, encryptedRightsBuffer, ro_size, deviceId);

    // if old RO struct, copy the value to the new type of struct
    if (0 == version)
    {
        convertRO(&ro->rights, &old_rights);
    }

    ro->rights.entryPtr = NULL;

    delete[] encryptedRightsBuffer;
    return true;
}

void rc4_encrypt_buffer(unsigned char* toBuffer, unsigned char* fromBuffer, int size, String8 &deviceId)
{
    ByteBuffer drmkey(getDrmKey(deviceId));
    RC4_KEY k;
    RC4_set_key(&k, DrmDef::DRM_MAX_KEY_LENGTH, (unsigned char*)drmkey.buffer());
    RC4(&k, size, fromBuffer, toBuffer);
}

void rc4_decrypt_buffer(unsigned char* toBuffer, unsigned char* fromBuffer, int size, String8 &deviceId)
{
    rc4_encrypt_buffer(toBuffer, fromBuffer, size,deviceId);
}

// return the device unique 16 bytes drm key
ByteBuffer getDrmKey(String8 &deviceId)
{
    ALOGD("getDrmKey() deviceId = %s",deviceId.string());
    char drmkey[DrmDef::DRM_MAX_ID_LENGTH + 1];
    bzero(drmkey, sizeof(drmkey));

    String8 imei = deviceId;

    char imei_encrypt_key[DrmDef::DRM_MAX_KEY_LENGTH];
    bzero(imei_encrypt_key, sizeof(imei_encrypt_key));

    // get an fixed key to encrypt imei
    long s = 0x3D4FAD6A;
    long v = ~(0xA9832DC6 ^ s);
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i] = (char) (0xFF & (v >> (i * 8)));
    }
    v = 0x16F0D768 ^ s;
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 4] = (char) (0xFF & (v >> (i * 8)));
    }
    v = ~(0x278FB1EA ^ s);
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 8] = (char) (0xFF & (v >> (i * 8)));
    }
    v = 0x5F3C54EC ^ s;
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 12] = (char) (0xFF & (v >> (i * 8)));
    }

    // encrypt imei to get an unique key
    RC4_KEY k;
    RC4_set_key(&k, DrmDef::DRM_MAX_KEY_LENGTH, (unsigned char*)imei_encrypt_key);
    RC4(&k, (unsigned long)imei.length() + 1, (unsigned char*)imei.string(), (unsigned char*)drmkey);

    return ByteBuffer(drmkey, DrmDef::DRM_MAX_KEY_LENGTH);
}


static void convertRO(Rights* rights, OldRights* oldRights)
{
    bzero(rights->cid, sizeof(rights->cid));
    memcpy(rights->cid, oldRights->cid, sizeof(oldRights->cid));

    memcpy(rights->key, oldRights->key, sizeof(oldRights->key));
    rights->permission = oldRights->permission;
    memcpy(rights->num, oldRights->num, sizeof(oldRights->num));
    memcpy(rights->best, oldRights->best, sizeof(oldRights->best));
}

bool restoreEntry(String8 ro_file, FILE* fp, RO* ro, String8 &deviceId)
{
    unsigned char encryptedEntryBuffer[sizeof(struct Entry)];
    bzero(encryptedEntryBuffer, sizeof(encryptedEntryBuffer));

    // get total entry number
    // the current fp shall points to the beginning of entries. (after call restoreRights)
    long old_pos = 0;
    FileUtil::ftellx(fp, old_pos);
    ALOGD("restoreEntry: entries starts at [%ld]", old_pos);
    struct stat stFileInfo;
    if (-1 == fstat(fileno(fp), &stFileInfo))
    {
        ALOGE("restoreEntry: failed to stat file info, reason [%s]", strerror(errno));
        return false;
    }
    ALOGD("restoreEntry: size of rights object [%ld]", (long)(stFileInfo.st_size));

    long cnt = (long)(stFileInfo.st_size) - old_pos;
    ALOGD("restoreEntry: entries data amount [%ld]", cnt);

    int totalEntryNumber = cnt / sizeof(struct Entry);
    ALOGD("restoreEntry: total entry number [%d]", totalEntryNumber);
    if ((cnt % sizeof(struct Entry)) != 0)
    {
        ALOGE("restoreEntry: invalid amount of entries data.");
        totalEntryNumber = -1;
    }

    if (totalEntryNumber < 0)
    {
        ALOGE("restoreEntry: invalid total entry number [%d]", totalEntryNumber);
        return false;
    }

    ro->rights.entryPtr = new Entry[totalEntryNumber];

    long entryBeginPosition = 0;
    FileUtil::ftellx(fp, entryBeginPosition);

    int valid_entry_num = 0;
    for (int i = DrmDef::PERMISSION_PLAY_INDEX; i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        valid_entry_num += ro->rights.num[i];
        if (ro->rights.num[i] != 0)
        {
            ALOGV("restoreEntry: permission [%s] entry num [%d]",
                    DrmDef::getPermissionStr(i).string(), ro->rights.num[i]);
            int entryIndex = ro->rights.best[i];
            while (entryIndex != 0xFF)
            {
                long pos = old_pos + entryIndex * sizeof(struct Entry);
                ALOGD("restoreEntry: restore entry #[%d] from position [%ld]",
                        entryIndex, pos);
                if (!FileUtil::fseekx(fp, pos, SEEK_SET))
                {
                    ALOGE("restoreEntry: failed to seek to [%ld]", pos);
                    return false;
                }

                bzero(encryptedEntryBuffer, sizeof(encryptedEntryBuffer));
                if (!FileUtil::freadx(encryptedEntryBuffer, sizeof(encryptedEntryBuffer), fp))
                {
                    ALOGE("restoreEntry: failed to read entry data from file.");
                    return false;
                }

                Entry* entry = &ro->rights.entryPtr[entryIndex];
                rc4_decrypt_buffer((unsigned char*)entry, encryptedEntryBuffer, sizeof(encryptedEntryBuffer), deviceId);

                entryIndex = entry->next;
            }
        }
    }

    ALOGD("restoreEntry: valid entry num [%d]", valid_entry_num);
    return true;
}
