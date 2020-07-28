#include <driver_api.h>
#include <interrupt.h>
#include <ccci.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#define CCIF_CON    (0x00)
#define CCIF_BUSY   (0x04)
#define CCIF_START  (0x08)
#define CCIF_TCHNUM (0x0C)
#define CCIF_RCHNUM (0x10)
#define CCIF_ACK    (0x14)
#define CCIF_CHDATA (0x100)
#define CCIF_SRAM_SIZE 512

static const unsigned int ccif_base_addr[MD_NUM] = {MD1_CCIF_BASE, MD3_CCIF_BASE};
static const unsigned int ccif_md_base_addr[MD_NUM] = {MD1_CCIF_MD_BASE, MD3_CCIF_MD_BASE};

void ccci_reset_hw(int md_id)
{
    int i;
    unsigned int tx_channel = 0;

    /* clear occupied channel */
    while (tx_channel < 16) {
        if (INREG32(ccif_base_addr[md_id] + CCIF_BUSY) & (1 << tx_channel))
            OUTREG32(ccif_base_addr[md_id] + CCIF_TCHNUM, tx_channel);

        if (INREG32(ccif_md_base_addr[md_id] + CCIF_BUSY) & (1 << tx_channel))
            OUTREG32(ccif_md_base_addr[md_id] + CCIF_TCHNUM, tx_channel);

        tx_channel++;
    }
    /* clear un-ached channel */
    OUTREG32(ccif_base_addr[md_id] + CCIF_ACK, INREG32(ccif_md_base_addr[md_id] + CCIF_BUSY));
    OUTREG32(ccif_md_base_addr[md_id] + CCIF_ACK, INREG32(ccif_base_addr[md_id] + CCIF_BUSY));

    /* clear SRAM */
    for (i = 0; i < CCIF_SRAM_SIZE / sizeof(unsigned int); i++) {
        OUTREG32(ccif_base_addr[md_id] + CCIF_CHDATA + i * sizeof(unsigned int), 0);
        OUTREG32(ccif_md_base_addr[md_id] + CCIF_CHDATA + i * sizeof(unsigned int), 0);
    }
}

static int ccif_send_to_md(int md_id, int channel_id)
{
    unsigned int busy = INREG32(ccif_base_addr[md_id] + CCIF_BUSY);

    if (busy & (1 << channel_id))
        return -1;
    OUTREG32(ccif_base_addr[md_id] + CCIF_BUSY, 1 << channel_id);
    OUTREG32(ccif_base_addr[md_id] + CCIF_TCHNUM, channel_id);
    return 0;
}

void ccci_wakeup_md(int md_id, int qno)
{
    ccif_send_to_md(md_id, qno);
}

static void md1_ccif_irq_handler(void)
{
    unsigned int channel_id = INREG32(MD1_CCIF_BASE + CCIF_RCHNUM);

    OUTREG32(MD1_CCIF_BASE + CCIF_ACK, channel_id);

    ccci_trigger_rx_task(MD_SYS1);

    mask_irq(MD1_CCIF_IRQ);
    CCCI_DBG_MSG(MD_SYS1, "HIF", "CCIF handler %x\n", channel_id);
}


static void md3_ccif_irq_handler(void)
{
    unsigned int channel_id = INREG32(MD3_CCIF_BASE + CCIF_RCHNUM);

    OUTREG32(MD3_CCIF_BASE + CCIF_ACK, channel_id);

    ccci_trigger_rx_task(MD_SYS3);

    mask_irq(MD3_CCIF_IRQ);
    CCCI_DBG_MSG(MD_SYS3, "HIF", "CCIF handler %x\n", channel_id);
}

void ccci_enable_irq(int md_id)
{
    switch (md_id) {
        case MD_SYS1:
            unmask_irq(MD1_CCIF_IRQ);
            break;
        case MD_SYS3:
            unmask_irq(MD3_CCIF_IRQ);
            break;
    }
}

int ccci_interface_init(void)
{
    request_irq(MD1_CCIF_IRQ, md1_ccif_irq_handler, "CCIF0");
    scp_wakeup_src_setup(MD1_CCIF_WAKEUP_SOURCE, 0);
    CCCI_INF_MSG(-1, "INIT", "register MD1 CCIF IRQ %d\n", MD1_CCIF_IRQ);
    request_irq(MD3_CCIF_IRQ, md3_ccif_irq_handler, "CCIF1");
    scp_wakeup_src_setup(MD3_CCIF_WAKEUP_SOURCE, 0);
    CCCI_INF_MSG(-1, "INIT", "register MD3 CCIF IRQ %d\n", MD3_CCIF_IRQ);
    return 0;
}

