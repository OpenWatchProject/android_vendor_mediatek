#ifndef __SPI_H__
#define __SPI_H__

#define SPI_TRANSFER_POLLING

//#define SPI_PERF

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

/**
 * struct spi_transfer - a read/write buffer pair
 * @tx_buf: data to be written (dma-safe memory), or NULL
 * @rx_buf: data to be read (dma-safe memory), or NULL
 * @tx_dma: DMA address of tx_buf, if @spi_message.is_dma_mapped
 * @rx_dma: DMA address of rx_buf, if @spi_message.is_dma_mapped
 * @len: size of rx and tx buffers (in bytes)
 * @speed_hz: Select a speed other than the device default for this
 *      transfer. If 0 the default (from @spi_device) is used.
 * @bits_per_word: select a bits_per_word other than the device default
 *      for this transfer. If 0 the default (from @spi_device) is used.
 * @cs_change: affects chipselect after this transfer completes
 * @delay_usecs: microseconds to delay after this transfer before
 *	(optionally) changing the chipselect status, then starting
 *	the next transfer or completing this @spi_message.
 * @transfer_list: transfers are sequenced through @spi_message.transfers
 *
 * SPI transfers always write the same number of bytes as they read.
 * Protocol drivers should always provide @rx_buf and/or @tx_buf.
 * In some cases, they may also want to provide DMA addresses for
 * the data being transferred; that may reduce overhead, when the
 * underlying driver uses dma.
 *
 * If the transmit buffer is null, zeroes will be shifted out
 * while filling @rx_buf.  If the receive buffer is null, the data
 * shifted in will be discarded.  Only "len" bytes shift out (or in).
 * It's an error to try to shift out a partial word.  (For example, by
 * shifting out three bytes with word size of sixteen or twenty bits;
 * the former uses two bytes per word, the latter uses four bytes.)
 *
 * In-memory data values are always in native CPU byte order, translated
 * from the wire byte order (big-endian except with SPI_LSB_FIRST).  So
 * for example when bits_per_word is sixteen, buffers are 2N bytes long
 * (@len = 2N) and hold N sixteen bit words in CPU byte order.
 *
 * When the word size of the SPI transfer is not a power-of-two multiple
 * of eight bits, those in-memory words include extra bits.  In-memory
 * words are always seen by protocol drivers as right-justified, so the
 * undefined (rx) or unused (tx) bits are always the most significant bits.
 *
 * All SPI transfers start with the relevant chipselect active.  Normally
 * it stays selected until after the last transfer in a message.  Drivers
 * can affect the chipselect signal using cs_change.
 *
 * (i) If the transfer isn't the last one in the message, this flag is
 * used to make the chipselect briefly go inactive in the middle of the
 * message.  Toggling chipselect in this way may be needed to terminate
 * a chip command, letting a single spi_message perform all of group of
 * chip transactions together.
 *
 * (ii) When the transfer is the last one in the message, the chip may
 * stay selected until the next transfer.  On multi-device SPI busses
 * with nothing blocking messages going to other devices, this is just
 * a performance hint; starting a message to another device deselects
 * this one.  But in other cases, this can be used to ensure correctness.
 * Some devices need protocol transactions to be built from a series of
 * spi_message submissions, where the content of one message is determined
 * by the results of previous messages and where the whole transaction
 * ends when the chipselect goes intactive.
 *
 * The code that submits an spi_message (and its spi_transfers)
 * to the lower layers is responsible for managing its memory.
 * Zero-initialize every field you don't set up explicitly, to
 * insulate against future API updates.  After you submit a message
 * and its transfers, ignore them until its completion callback.
 */
struct spi_transfer {
	/* it's ok if tx_buf == rx_buf (right?)
	 * for MicroWire, one buffer must be null
	 * buffers must work with dma_*map_single() calls, unless
	 *   spi_message.is_dma_mapped reports a pre-existing mapping
	 */
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
	uint32_t enable_custom;
};

typedef struct spi_transfer *spi_transfer_ptr;
typedef struct spi_transfer spi_transfer_t;

/**
 * struct spi_message - one multi-segment SPI transaction
 * @transfers: list of transfer segments in this transaction
 * @spi: SPI device to which the transaction is queued
 * @is_dma_mapped: if true, the caller provided both dma and cpu virtual
 *	addresses for each transfer buffer
 * @complete: called to report transaction completions
 * @context: the argument to complete() when it's called
 * @actual_length: the total number of bytes that were transferred in all
 *	successful segments
 * @status: zero for success, else negative errno
 * @queue: for use by whichever driver currently owns the message
 * @state: for use by whichever driver currently owns the message
 *
 * A @spi_message is used to execute an atomic sequence of data transfers,
 * each represented by a struct spi_transfer.  The sequence is "atomic"
 * in the sense that no other spi_message may use that SPI bus until that
 * sequence completes.  On some systems, many such sequences can execute as
 * as single programmed DMA transfer.  On all systems, these messages are
 * queued, and might complete after transactions to other devices.  Messages
 * sent to a given spi_device are alway executed in FIFO order.
 *
 * The code that submits an spi_message (and its spi_transfers)
 * to the lower layers is responsible for managing its memory.
 * Zero-initialize every field you don't set up explicitly, to
 * insulate against future API updates.  After you submit a message
 * and its transfers, ignore them until its completion callback.
 */
struct spi_message {
	//struct list_head	transfers;

	struct spi_device	*spi;

	uint32_t		is_dma_mapped;

	/* REVISIT:  we might want a flag affecting the behavior of the
	 * last transfer ... allowing things like "read 16 bit length L"
	 * immediately followed by "read L bytes".  Basically imposing
	 * a specific message scheduling algorithm.
	 *
	 * Some controller drivers (message-at-a-time queue processing)
	 * could provide that as their default scheduling algorithm.  But
	 * others (with multi-message pipelines) could need a flag to
	 * tell them about such special cases.
	 */

	/* completion is reported through a callback */
	void			(*complete)(void *context);
	void			*context;
	uint32_t		actual_length;
	int			status;

	/* for optional use by whatever driver currently owns the
	 * spi_message ...  between calls to spi_async and then later
	 * complete(), that's the spi_master controller driver.
	 */
	//struct list_head	queue;
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

int dr_spi_rw(void *rx_buf, void *tx_buf, uint32_t len);
int spi_tz_irq_handler(void);
int spi_read_image(uint8_t *origindata, uint32_t origindata_size, uint32_t enable_custom);



#endif //__SPI_H__
