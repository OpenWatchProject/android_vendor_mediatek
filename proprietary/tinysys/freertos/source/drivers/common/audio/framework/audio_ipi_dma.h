#ifndef AUDUI_IPI_DMA_H
#define AUDUI_IPI_DMA_H

#include <stdint.h>


enum { /* audio_ipi_dma_path_t */
    AUDIO_IPI_DMA_AP_TO_SCP = 0,
    AUDIO_IPI_DMA_SCP_TO_AP = 1,
    NUM_AUDIO_IPI_DMA_PATH
};


struct ipi_msg_t;

struct aud_ptr_t {
    union {
        uint8_t *addr;
        uint64_t addr_val; /* the value of address */

        uint32_t dummy[2]; /* work between 32/64 bit environment */
    };
};


int init_audio_ipi_dma(struct ipi_msg_t *msg);

int audio_ipi_dma_update_region_info(struct ipi_msg_t *msg);


int audio_ipi_dma_write_region(const uint8_t task,
                               const void *data_buf,
                               uint32_t data_size,
                               uint32_t *write_idx);

int audio_ipi_dma_read_region(const uint8_t task,
                              void *data_buf,
                              uint32_t data_size,
                              uint32_t read_idx);

int audio_ipi_dma_drop_region(const uint8_t task,
                              uint32_t data_size,
                              uint32_t read_idx);


void audio_ipi_dma_read(void *scp_buf, void *dram_addr, uint32_t size);
void audio_ipi_dma_write(void *dram_addr, void *scp_buf, uint32_t size);



#endif /* end of AUDUI_IPI_DMA_H */

