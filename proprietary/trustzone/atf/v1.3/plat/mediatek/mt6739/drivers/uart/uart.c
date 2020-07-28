#include <stdint.h>
#include <mmio.h>

/* FCR */
#define UART_FCR_FIFOE              (1 << 0)
#define UART_FCR_CLRR               (1 << 1)
#define UART_FCR_CLRT               (1 << 2)
#define UART_FCR_DMA1               (1 << 3)
#define UART_FCR_RXFIFO_1B_TRI      (0 << 6)
#define UART_FCR_RXFIFO_6B_TRI      (1 << 6)
#define UART_FCR_RXFIFO_12B_TRI     (2 << 6)
#define UART_FCR_RXFIFO_RX_TRI      (3 << 6)
#define UART_FCR_TXFIFO_1B_TRI      (0 << 4)
#define UART_FCR_TXFIFO_4B_TRI      (1 << 4)
#define UART_FCR_TXFIFO_8B_TRI      (2 << 4)
#define UART_FCR_TXFIFO_14B_TRI     (3 << 4)

#define UART_FCR_FIFO_INIT          (UART_FCR_FIFOE|UART_FCR_CLRR|UART_FCR_CLRT)
#define UART_FCR_NORMAL             (UART_FCR_FIFO_INIT | \
                                     UART_FCR_TXFIFO_4B_TRI| \
                                     UART_FCR_RXFIFO_12B_TRI)


/* LCR */
#define UART_LCR_BREAK              (1 << 6)
#define UART_LCR_DLAB               (1 << 7)

#define UART_WLS_5                  (0 << 0)
#define UART_WLS_6                  (1 << 0)
#define UART_WLS_7                  (2 << 0)
#define UART_WLS_8                  (3 << 0)
#define UART_WLS_MASK               (3 << 0)

#define UART_1_STOP                 (0 << 2)
#define UART_2_STOP                 (1 << 2)
#define UART_1_5_STOP               (1 << 2)    /* Only when WLS=5 */
#define UART_STOP_MASK              (1 << 2)

#define UART_NONE_PARITY            (0 << 3)
#define UART_ODD_PARITY             (0x1 << 3)
#define UART_EVEN_PARITY            (0x3 << 3)
#define UART_MARK_PARITY            (0x5 << 3)
#define UART_SPACE_PARITY           (0x7 << 3)
#define UART_PARITY_MASK            (0x7 << 3)

/* MCR */
#define UART_MCR_DTR                (1 << 0)
#define UART_MCR_RTS                (1 << 1)
#define UART_MCR_OUT1               (1 << 2)
#define UART_MCR_OUT2               (1 << 3)
#define UART_MCR_LOOP               (1 << 4)
#define UART_MCR_XOFF               (1 << 7)    /* read only */
#define UART_MCR_NORMAL             (UART_MCR_DTR|UART_MCR_RTS)

/* LSR */
#define UART_LSR_DR                 (1 << 0)
#define UART_LSR_OE                 (1 << 1)
#define UART_LSR_PE                 (1 << 2)
#define UART_LSR_FE                 (1 << 3)
#define UART_LSR_BI                 (1 << 4)
#define UART_LSR_THRE               (1 << 5)
#define UART_LSR_TEMT               (1 << 6)
#define UART_LSR_FIFOERR            (1 << 7)

/* MSR */
#define UART_MSR_DCTS               (1 << 0)
#define UART_MSR_DDSR               (1 << 1)
#define UART_MSR_TERI               (1 << 2)
#define UART_MSR_DDCD               (1 << 3)
#define UART_MSR_CTS                (1 << 4)
#define UART_MSR_DSR                (1 << 5)
#define UART_MSR_RI                 (1 << 6)
#define UART_MSR_DCD                (1 << 7)

/* EFR */
#define UART_EFR_EN                 (1 << 4)
#define UART_EFR_AUTO_RTS           (1 << 6)
#define UART_EFR_AUTO_CTS           (1 << 7)
#define UART_EFR_SW_CTRL_MASK       (0xf << 0)

#define UART_EFR_NO_SW_CTRL         (0)
#define UART_EFR_NO_FLOW_CTRL       (0)
#define UART_EFR_AUTO_RTSCTS        (UART_EFR_AUTO_RTS|UART_EFR_AUTO_CTS)
#define UART_EFR_XON1_XOFF1         (0xa)       /* TX/RX XON1/XOFF1 flow control */
#define UART_EFR_XON2_XOFF2         (0x5)       /* TX/RX XON2/XOFF2 flow control */
#define UART_EFR_XON12_XOFF12       (0xf)       /* TX/RX XON1,2/XOFF1,2 flow
                                                   control */

#define UART_EFR_XON1_XOFF1_MASK    (0xa)
#define UART_EFR_XON2_XOFF2_MASK    (0x5)

/* IIR (Read Only) */
#define UART_IIR_NO_INT_PENDING     (0x01)
#define UART_IIR_RLS                (0x06)      /* Receiver Line Status */
#define UART_IIR_RDA                (0x04)      /* Receive Data Available */
#define UART_IIR_CTI                (0x0C)      /* Character Timeout Indicator */
#define UART_IIR_THRE               (0x02)      /* Transmit Holding Register Empty
                                                 */
#define UART_IIR_MS                 (0x00)      /* Check Modem Status Register */
#define UART_IIR_SW_FLOW_CTRL       (0x10)      /* Receive XOFF characters */
#define UART_IIR_HW_FLOW_CTRL       (0x20)      /* CTS or RTS Rising Edge */
#define UART_IIR_FIFO_EN            (0xc0)
#define UART_IIR_INT_MASK           (0x1f)

/* RateFix */
#define UART_RATE_FIX               (1 << 0)
//#define UART_AUTORATE_FIX           (1 << 1)
//#define UART_FREQ_SEL               (1 << 2)
#define UART_FREQ_SEL               (1 << 1)

#define UART_RATE_FIX_13M           (1 << 0)    /* means UARTclk = APBclk / 4 */
#define UART_AUTORATE_FIX_13M       (1 << 1)
#define UART_FREQ_SEL_13M           (1 << 2)
#define UART_RATE_FIX_ALL_13M       (UART_RATE_FIX_13M|UART_AUTORATE_FIX_13M| \
                                     UART_FREQ_SEL_13M)

#define UART_RATE_FIX_26M           (0 << 0)    /* means UARTclk = APBclk / 2 */
#define UART_AUTORATE_FIX_26M       (0 << 1)
#define UART_FREQ_SEL_26M           (0 << 2)
#define UART_RATE_FIX_ALL_26M       (UART_RATE_FIX_26M|UART_AUTORATE_FIX_26M| \
                                     UART_FREQ_SEL_26M)

#define UART_RATE_FIX_32M5          (0 << 0)    /* means UARTclk = APBclk / 2 */
#define UART_FREQ_SEL_32M5          (0 << 1)
#define UART_RATE_FIX_ALL_32M5      (UART_RATE_FIX_32M5|UART_FREQ_SEL_32M5)

#define UART_RATE_FIX_16M25         (0 << 0)    /* means UARTclk = APBclk / 4 */
#define UART_FREQ_SEL_16M25         (0 << 1)
#define UART_RATE_FIX_ALL_16M25     (UART_RATE_FIX_16M25|UART_FREQ_SEL_16M25)


/* Autobaud sample */
#define UART_AUTOBADUSAM_13M         7
#define UART_AUTOBADUSAM_26M        15
#define UART_AUTOBADUSAM_52M        31
//#define UART_AUTOBADUSAM_52M        29  /* 28 or 29 ? */
#define UART_AUTOBAUDSAM_58_5M      31  /* 31 or 32 ? */

/* VFIFO enable */
#define UART_VFIFO_ON               (1 << 0)
#define UART_FCR_FIFOE              (1 << 0)
#define UART_FCR_FIFO_INIT          (UART_FCR_FIFOE|UART_FCR_CLRR|UART_FCR_CLRT)


#define UART_BASE(uart)             (uart)

#define UART_RBR(uart)              (UART_BASE(uart)+0x0)       /* Read only */
#define UART_THR(uart)              (UART_BASE(uart)+0x0)       /* Write only */
#define UART_IER(uart)              (UART_BASE(uart)+0x4)
#define UART_IIR(uart)              (UART_BASE(uart)+0x8)       /* Read only */
#define UART_FCR(uart)              (UART_BASE(uart)+0x8)       /* Write only */
#define UART_LCR(uart)              (UART_BASE(uart)+0xc)
#define UART_MCR(uart)              (UART_BASE(uart)+0x10)
#define UART_LSR(uart)              (UART_BASE(uart)+0x14)
#define UART_MSR(uart)              (UART_BASE(uart)+0x18)
#define UART_SCR(uart)              (UART_BASE(uart)+0x1c)
#define UART_DLL(uart)              (UART_BASE(uart)+0x0)
#define UART_DLH(uart)              (UART_BASE(uart)+0x4)
#define UART_EFR(uart)              (UART_BASE(uart)+0x8)
#define UART_XON1(uart)             (UART_BASE(uart)+0x10)
#define UART_XON2(uart)             (UART_BASE(uart)+0x14)
#define UART_XOFF1(uart)            (UART_BASE(uart)+0x18)
#define UART_XOFF2(uart)            (UART_BASE(uart)+0x1c)
#define UART_AUTOBAUD_EN(uart)      (UART_BASE(uart)+0x20)
#define UART_HIGHSPEED(uart)        (UART_BASE(uart)+0x24)
#define UART_SAMPLE_COUNT(uart)     (UART_BASE(uart)+0x28)
#define UART_SAMPLE_POINT(uart)     (UART_BASE(uart)+0x2c)
#define UART_AUTOBAUD_REG(uart)     (UART_BASE(uart)+0x30)
#define UART_RATE_FIX_AD(uart)      (UART_BASE(uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)  (UART_BASE(uart)+0x38)
#define UART_GUARD(uart)            (UART_BASE(uart)+0x3c)
#define UART_ESCAPE_DAT(uart)       (UART_BASE(uart)+0x40)
#define UART_ESCAPE_EN(uart)        (UART_BASE(uart)+0x44)
#define UART_SLEEP_EN(uart)         (UART_BASE(uart)+0x48)
#define UART_VFIFO_EN(uart)         (UART_BASE(uart)+0x4c)
#define UART_RXTRI_AD(uart)         (UART_BASE(uart)+0x50)

void mtk_uart_init (uintptr_t uart_port,
		unsigned int uart_clk, unsigned int baud_rate)
{

    volatile unsigned short divisor;
    unsigned short high_speed_div, data;
    unsigned short sample_count, sample_point;
    unsigned int uart_clock = uart_clk;
    unsigned int baudrate = baud_rate;
    unsigned int tmp_div;
    unsigned short byte;
    unsigned int remainder;
    unsigned long uart_base;

    uart_base = uart_port;

    //Setup N81,(UART_WLS_8 | UART_NONE_PARITY | UART_1_STOP) = 0x03

    mmio_write_16((UART_LCR(uart_base)), 0x0003);

    data=(unsigned short)(uart_clock/baudrate);
    high_speed_div = (data>>8) + 1; // divided by 256

    tmp_div=uart_clock/(baudrate*high_speed_div);
    divisor =  (unsigned short)tmp_div;

    remainder = (uart_clock)%(high_speed_div*baudrate);
    /*get (sample_count+1)*/
    if (remainder >= ((baudrate)*(high_speed_div))>>1)
        divisor =  (unsigned short)(tmp_div+1);
    else
        divisor =  (unsigned short)tmp_div;

    sample_count=divisor-1;

    /*get the sample point*/
    sample_point=(sample_count-1)>>1;

    /*configure register*/
    mmio_write_32(UART_HIGHSPEED(uart_base), 0x3);
    byte = mmio_read_32(UART_LCR(uart_base));     /* DLAB start */
    mmio_write_32(UART_LCR(uart_base), (byte | UART_LCR_DLAB));
    mmio_write_32(UART_DLL(uart_base), (high_speed_div & 0x00ff));
    mmio_write_32(UART_DLH(uart_base), ((high_speed_div >> 8)&0x00ff));
    mmio_write_32(UART_SAMPLE_COUNT(uart_base), sample_count);
    mmio_write_32(UART_SAMPLE_POINT(uart_base), sample_point);
    mmio_write_32(UART_LCR(uart_base), byte);      /* DLAB End */
    return;
}
