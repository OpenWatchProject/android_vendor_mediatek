#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rc4.h>
#include <ByteBuffer.h>
#include <utils/String8.h>
#include <DrmUtil.h>
#include <RO.h>
#include <StrUtil.h>
#include <FileUtil.h>
#include <drm/DrmManagerClient.h>
#include <DrmMtkPlugIn.h>
#include <DrmRequestType.h>
#include <drm/DrmInfoRequest.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


using namespace android;

void dump(RO *pRO);
bool test_install_drm_msg(int dm_fd, int dcf_fd);

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Usage: %s path\n",argv[0]);
        printf("path is the dm file's full name path\n");
        return -1;
    }

    char *dcf_path = "/sdcard/rui_dcf.dcf";
    int dcf_fd = open(dcf_path,O_WRONLY | O_CREAT | O_TRUNC, 0744);
    int dm_fd = open(argv[1],O_RDWR);
    if(dm_fd <= 0)
    {
        printf("open dm file faild:%s\n",strerror(errno));
        return -1;
    }
    test_install_drm_msg(dm_fd,dcf_fd);
    close(dcf_fd);
    close(dm_fd);
    printf("test install drm msg success\n");
    return 0;
}

bool test_install_drm_msg(int dm_fd, int dcf_fd)
{
    printf("test_install_drm_msg():dm_fd[%d], dcf_fd[%d]",dm_fd, dcf_fd);
    DrmInfoRequest drmInfoReq(DrmRequestType::TYPE_SET_DRM_INFO , DrmDef::MIME_DRM_MESSAGE);
    char dm_str[32] = {0};
    char dcf_str[32] = {0};
    sprintf(dm_str,"%d",dm_fd);
    sprintf(dcf_str,"%d",dcf_fd);
    String8 dm_fd_str(dm_str);
    String8 dcf_fd_str(dcf_str);
    printf("dm_fd_str=%s\n",dm_fd_str.string());
    printf("dcf_fd_str=%s\n",dcf_fd_str.string());
    drmInfoReq.put(/*DrmRequestType::KEY_ACTION*/String8("action"),String8("installDrmMsgDmByFd"));
    drmInfoReq.put(/*DrmRequestType::KEY_DM_FD*/String8("dmFd"),dm_fd_str);
    drmInfoReq.put(/*DrmRequestType::KEY_DCF_FD*/String8("dcfFd"), dcf_fd_str);
    DrmManagerClient drmClient;
    DrmInfo *pDrmInfo = drmClient.acquireDrmInfo(&drmInfoReq);
    delete pDrmInfo;
    pDrmInfo = NULL;
    return true;
}
