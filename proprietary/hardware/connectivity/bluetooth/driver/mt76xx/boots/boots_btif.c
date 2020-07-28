//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "boots.h"

//---------------------------------------------------------------------------
#define LOG_TAG "boots_btif"

//---------------------------------------------------------------------------
extern int fd[BOOTS_BTIF_ALL];
extern int mtk_66xx_Init(int comPort, unsigned int chipId);
extern uint32_t mtk_66xx_detect(void);
extern int mtk_hci_enable(char *arg, void *func_cb);
extern int mtk_hci_disable(int bt_fd);

//---------------------------------------------------------------------------
static char *get_inf_path(boots_if_e interface)
{
    int i = 0;

    while (boots_btif[i].inf != interface) {
        i++;
    }
    BPRINT_D("path: %s", boots_btif[i].p);
    return boots_btif[i].p;
}

//---------------------------------------------------------------------------
static int bt_init(int fd, uint32_t chip)
{
    /* Patch download is moved to WMT driver on combo chip */
    /* Invoke HCI transport entrance */
    if (mtk_66xx_Init(fd, chip) != 0) {
        BPRINT_E("mtk_66xx_Init failed");
        return -1;
    }
    return 0;
}

//---------------------------------------------------------------------------
static int stpbt_enable(char *dev)
{
    int fd = -1;

    if (dev == NULL) return fd;

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        BPRINT_E("Can't open BT port, errno: %d", fd);
        return -1;
    } else {
        BPRINT_D("Open %s(%d) successfully", dev, fd);
    }

    return fd;
}

//---------------------------------------------------------------------------
static void stpbt_disable(int fd)
{
    if (fd > 0)
        close(fd);

    return;
}

//---------------------------------------------------------------------------
int boots_btif_open(boots_if_e interface)
{
    uint32_t chip = 0x0000;
    /**
     * Depends on different interface:
     *  HCI, STPBT, UART & ETHERNET
     *  to do.
     */
    switch (interface) {
    case BOOTS_BTIF_STPBT:
        fd[interface] = stpbt_enable(get_inf_path(interface));
        break;
/*
    case BOOTS_BTIF_HCI:
        fd[interface] = mtk_hci_enable(get_inf_path(interface), NULL);
        break;
*/
    default:
        BPRINT_E("%s: Incorrect interface: %d", __func__, interface);
        return -1;
        break;
    }

    if (fd[interface] > 0) {
        chip = mtk_66xx_detect();
        switch (chip) {
        case 0x6630:
//        case 0x8167:
            BPRINT_I("chip: %04X", chip);
            if (bt_init(fd[interface], chip) != 0) {
                return -1;
            }
            break;
        default:
            break;
        };
    }

    return fd[interface];
}

//---------------------------------------------------------------------------
int boots_btif_close(boots_if_e interface)
{
    /** bt deinit */
    if (fd[interface] < 0) {
        return -1;
    }

    switch (interface) {
    case BOOTS_BTIF_STPBT:
        stpbt_disable(fd[interface]);
        break;
/*
    case BOOTS_BTIF_HCI:
        mtk_hci_disable(fd[interface]);
        break;
*/
    default:
        BPRINT_E("%s: Incorrect interface: %d", __func__, interface);
        break;
    }
    return 0;
}

//---------------------------------------------------------------------------
ssize_t boots_btif_read(int fd, uint8_t *buf, size_t size)
{
    ssize_t nRead = 0;

    if (fd < 0) {
        BPRINT_E("File descriptor in bad state");
        return -EIO;
    } else if (!buf || !size) {
        BPRINT_E("%s: Invalid argument buf:%p, size:%d", __func__, buf, (int)size);
        return -EINVAL;
    }

    nRead = read(fd, buf, size);
    if (nRead <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            BPRINT_W("%s: %s(%d)", __func__, strerror(errno), errno);
            return 0;
        } else {
            BPRINT_E("%s: %s(%d)", __func__, strerror(errno), errno);
            return -errno;
        }
    }

    return nRead;
}

//---------------------------------------------------------------------------
ssize_t boots_btif_write(int fd, uint8_t *buf, size_t len)
{
    ssize_t ret = 0;
    size_t bytesToWrite = len;

    if (fd < 0) {
        BPRINT_E("%s: No available com port", __func__);
        return -EIO;
    }

    while (bytesToWrite > 0) {
        ret = write(fd, buf, bytesToWrite);
        if (ret <= 0) {
            if (errno == EINTR || errno == EAGAIN) {
                BPRINT_W("%s: %s(%d)", __func__, strerror(errno), errno);
                return 0;
            } else {
                BPRINT_E("%s: %s(%d)", __func__, strerror(errno), errno);
                return -errno;
            }
        }
        bytesToWrite -= ret;
        buf += ret;
    }

    return (len - bytesToWrite);
}

//---------------------------------------------------------------------------
