#ifndef __SPI_H__
#define __SPI_H__

/**
 * struct spi_device - No need Now
 */
struct spi_device {
	//struct device		dev;
	//struct spi_master	*master;
	uint32_t		max_speed_hz;
	uint8_t			chip_select;
	uint8_t			mode;
#define	SPI_CPHA	0x01			/* clock phase */
#define	SPI_CPOL	0x02			/* clock polarity */
#define	SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define	SPI_MODE_1	(0|SPI_CPHA)
#define	SPI_MODE_2	(SPI_CPOL|0)
#define	SPI_MODE_3	(SPI_CPOL|SPI_CPHA)
#define	SPI_CS_HIGH	0x04			/* chipselect active high? */
#define	SPI_LSB_FIRST	0x08			/* per-word bits-on-wire */
#define	SPI_3WIRE	0x10			/* SI/SO signals shared */
#define	SPI_LOOP	0x20			/* loopback mode */
#define	SPI_NO_CS	0x40			/* 1 dev/bus, no chipselect */
#define	SPI_READY	0x80			/* slave pulls low to pause */
	uint8_t			bits_per_word;
	uint32_t			irq;
	void			*controller_state;
	void			*controller_data;
	uint32_t			cs_gpio;	/* chip select gpio */

};

struct spi_transfer {
	uint32_t    commandId;
	const void	*tx_buf;
	void		*rx_buf;
	uint32_t	len;
	uint32_t    is_dma_used;
	uint32_t    is_transfer_end; /*for clear pause bit*/

	uint32_t	tx_dma; //dma_addr_t
	uint32_t	rx_dma; //dma_addr_t

	struct mt_chip_conf *chip_config;

	//uint32_t	cs_change;
	//uint8_t		bits_per_word;
	//uint16_t	delay_usecs;
	//uint32_t	speed_hz;
	int exc_flag;

	//struct list_head transfer_list;
};

typedef struct spi_transfer *spi_transfer_ptr;
typedef struct spi_transfer spi_transfer_t;

struct spi_message {
	//struct list_head	transfers;

	struct spi_device	*spi;

	uint32_t		is_dma_mapped;

	/* completion is reported through a callback */
	void			(*complete)(void *context);
	void			*context;
	uint32_t		actual_length;
	int			status;
	void			*state;
};

enum spi_cpol {
	SPI_CPOL_0,
	SPI_CPOL_1
};

enum spi_cpha {
	SPI_CPHA_0,
	SPI_CPHA_1
};

enum spi_mlsb {
	SPI_LSB,
	SPI_MSB
};

enum spi_endian {
	SPI_LENDIAN,
	SPI_BENDIAN
};

enum spi_transfer_mode {
	FIFO_TRANSFER,
	DMA_TRANSFER,
	OTHER1,
	OTHER2,
};

enum spi_pause_mode {
	PAUSE_MODE_DISABLE,
	PAUSE_MODE_ENABLE
};
enum spi_finish_intr {
	FINISH_INTR_DIS,
	FINISH_INTR_EN,
};

enum spi_deassert_mode {
	DEASSERT_DISABLE,
	DEASSERT_ENABLE
};

enum spi_ulthigh {
	ULTRA_HIGH_DISABLE,
	ULTRA_HIGH_ENABLE
};

enum spi_tckdly {
	TICK_DLY0,
	TICK_DLY1,
	TICK_DLY2,
	TICK_DLY3
};

enum spi_irq_flag {
	IRQ_IDLE,
	IRQ_BUSY
};

struct mt_chip_conf {
	uint32_t setuptime;
	uint32_t holdtime;
	uint32_t high_time;
	uint32_t low_time;
	uint32_t cs_idletime;
	uint32_t ulthgh_thrsh;
	enum spi_cpol cpol;
	enum spi_cpha cpha;
	enum spi_mlsb tx_mlsb;
	enum spi_mlsb rx_mlsb;
	enum spi_endian tx_endian;
	enum spi_endian rx_endian;
	enum spi_transfer_mode com_mod;
	enum spi_pause_mode pause;
	enum spi_finish_intr finish_intr;
	enum spi_deassert_mode deassert;
	enum spi_ulthigh ulthigh;
	enum spi_tckdly tckdly;
};

struct mt_spi_t {
	//void __iomem *regs;
	uint32_t irq;
	uint32_t running;
	struct mt_chip_conf *config;	
	//struct spi_master *master;	
	struct spi_transfer *cur_transfer;
	struct spi_transfer *next_transfer;
	//struct list_head	queue;	
};

#define IDLE 0
#define INPROGRESS 1
#define PAUSED 2

#define PACKET_SIZE 0x400

#define SPI_FIFO_SIZE 32


#define INVALID_DMA_ADDRESS 0xffffffff

extern int timeout_flag ;
extern void Set_timeout_flag(void);
extern int Get_timeout_flag(void);
extern uint8_t is_last_xfer;
extern struct mt_chip_conf* chip_config;
struct mt_chip_conf* GetChipConfig(void);
struct spi_transfer* GetSpiTransfer(void);
void SetPauseStatus(int32_t status);
int32_t GetPauseStatus(void);
int32_t GetIrqFlag(void);
void SetIrqFlag(enum spi_irq_flag flag);
int32_t SpiTransfer(struct spi_transfer *xfer);
int32_t SpiSetup(struct mt_chip_conf *chip_config);
void SpiChipConfigSet(void); 
void dump_chip_config_default(void);
void dump_reg_config(void);
void SpiChipConfig(struct mt_chip_conf *ptr);
void SpiSetupTransfer(struct spi_transfer* ptr);
void SpiFifoTest(void);
int32_t SpiInit(void);
void ResetSpi(void);
void SpiDMAUnMapping(void);
void spi_enable_clk(void);
void spi_disable_clk(void);

#endif //__SPI_H__
