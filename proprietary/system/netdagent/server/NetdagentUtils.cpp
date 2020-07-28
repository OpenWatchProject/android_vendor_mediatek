#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <pthread.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils/Compat.h>

#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <NetdagentUtils.h>

#define LOG_TAG "NetdagentUtils"
#include "log/log.h"

namespace android {
namespace netdagent {

const char * const IPTABLES_PATH = "/system/bin/iptables-wrapper-1.0";
const char * const IP6TABLES_PATH = "/system/bin/ip6tables-wrapper-1.0";
const char * const IPTABLES_RESTORE_PATH = "/system/bin/iptables-restore";
const char * const IP6TABLES_RESTORE_PATH = "/system/bin/ip6tables-restore";
const char * const TC_PATH = "/system/bin/tc-wrapper-1.0";
const char * const IP_PATH = "/system/bin/ip-wrapper-1.0";
const char * const NDC_PATH = "/system/bin/ndc-wrapper-1.0";

static int ifc_ctl_sock = -1;
static pthread_mutex_t ifc_sock_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
/*
 * Check an interface name for plausibility. This should e.g. help against
 * directory traversal.
 */
bool isIfaceName(const char *name) {
    size_t i;
    size_t name_len = strlen(name);
    if ((name_len == 0) || (name_len > IFNAMSIZ)) {
        return false;
    }

    /* First character must be alphanumeric */
    if (!isalnum(name[0])) {
        return false;
    }

    for (i = 1; i < name_len; i++) {
        if (!isalnum(name[i]) && (name[i] != '_') && (name[i] != '-') && (name[i] != ':')) {
            return false;
        }
    }

    return true;
}

int ifc_init(void)
{
    int ret;

    pthread_mutex_lock(&ifc_sock_mutex);
    if (ifc_ctl_sock == -1) {
        ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
        if (ifc_ctl_sock < 0) {
            ALOGE("socket() failed: %s\n", strerror(errno));
        }
    }

    ret = ifc_ctl_sock < 0 ? -1 : 0;
    if (0) ALOGD("ifc_init_returning %d", ret);
    return ret;
}

void ifc_close(void)
{
    if (0) ALOGD("ifc_close");
    if (ifc_ctl_sock != -1) {
        (void)close(ifc_ctl_sock);
        ifc_ctl_sock = -1;
    }
    pthread_mutex_unlock(&ifc_sock_mutex);
}

static void ifc_init_ifr(const char *name, struct ifreq *ifr)
{
    memset(ifr, 0, sizeof(struct ifreq));
    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    ifr->ifr_name[IFNAMSIZ - 1] = 0;
}

int ifc_is_up(const char *name, unsigned *isup)
{
    struct ifreq ifr;
    ifc_init_ifr(name, &ifr);

    if(ioctl(ifc_ctl_sock, SIOCGIFFLAGS, &ifr) < 0) {
        ALOGE("ifc_is_up get flags error:%d(%s)", errno, strerror(errno));
        return -1;
    }
    if(ifr.ifr_flags & IFF_UP)
        *isup = 1;
    else
        *isup = 0;

    return 0;
}

static int ifc_set_flags(const char *name, unsigned set, unsigned clr)
{
    struct ifreq ifr;
    ifc_init_ifr(name, &ifr);

    if(ioctl(ifc_ctl_sock, SIOCGIFFLAGS, &ifr) < 0) return -1;
    ifr.ifr_flags = (ifr.ifr_flags & (~clr)) | set;
    return ioctl(ifc_ctl_sock, SIOCSIFFLAGS, &ifr);
}

int ifc_up(const char *name)
{
    int ret = ifc_set_flags(name, IFF_UP, 0);
    if (0) ALOGD("ifc_up(%s) = %d", name, ret);
    return ret;
}

int ifc_down(const char *name)
{
    int ret = ifc_set_flags(name, 0, IFF_UP);
    if (0) ALOGD("ifc_down(%s) = %d", name, ret);
    return ret;
}

bool ReadFdToString(int fd, std::string* content) {
  content->clear();

  char buf[BUFSIZ];
  ssize_t n;
  while ((n = TEMP_FAILURE_RETRY(read(fd, &buf[0], sizeof(buf)))) > 0) {
    content->append(buf, n);
  }
  return (n == 0) ? true : false;
}

bool ReadFileToString(const std::string& path, std::string* content) {
  content->clear();

  int fd = TEMP_FAILURE_RETRY(open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_BINARY));
  if (fd == -1) {
    return false;
  }
  bool result = ReadFdToString(fd, content);
  close(fd);
  return result;
}

bool WriteStringToFd(const std::string& content, int fd) {
  const char* p = content.data();
  size_t left = content.size();
  while (left > 0) {
    ssize_t n = TEMP_FAILURE_RETRY(write(fd, p, left));
    if (n == -1) {
      return false;
    }
    p += n;
    left -= n;
  }
  return true;
}

static bool CleanUpAfterFailedWrite(const std::string& path) {
  // Something went wrong. Let's not leave a corrupt file lying around.
  int saved_errno = errno;
  unlink(path.c_str());
  errno = saved_errno;
  return false;
}

#if !defined(_WIN32)
bool WriteStringToFile(const std::string& content, const std::string& path,
                       mode_t mode, uid_t owner, gid_t group) {
  int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW | O_BINARY;
  int fd = TEMP_FAILURE_RETRY(open(path.c_str(), flags, mode));
  if (fd == -1) {
    ALOGE("android::WriteStringToFile open failed: %s", strerror(errno));
    return false;
  }

  // We do an explicit fchmod here because we assume that the caller really
  // meant what they said and doesn't want the umask-influenced mode.
  if (fchmod(fd, mode) == -1) {
    ALOGE("android::WriteStringToFile fchmod failed: %s", strerror(errno));
    close(fd);
    return CleanUpAfterFailedWrite(path);
  }
  if (fchown(fd, owner, group) == -1) {
    ALOGE("android::WriteStringToFile fchown failed: %s", strerror(errno));
    close(fd);
    return CleanUpAfterFailedWrite(path);
  }
  if (!WriteStringToFd(content, fd)) {
    ALOGE("android::WriteStringToFile write failed: %s", strerror(errno));
    close(fd);
    return CleanUpAfterFailedWrite(path);
  }
  close(fd);
  return true;
}
#endif

bool WriteStringToFile(const std::string& content, const std::string& path) {
  int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW | O_BINARY;
  int fd = TEMP_FAILURE_RETRY(open(path.c_str(), flags, DEFFILEMODE));
  if (fd == -1) {
    return false;
  }

  bool result = WriteStringToFd(content, fd);
  close(fd);
  return result || CleanUpAfterFailedWrite(path);
}


}  // namespace netdagent
}  // namespace android

