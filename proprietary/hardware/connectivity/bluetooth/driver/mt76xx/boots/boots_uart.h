//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_UART_H__
#define __BOOTS_UART_H__

//---------------------------------------------------------------------------
int boots_uart_init(char *dev, int speed);
ssize_t boots_uart_write(int fd, uint8_t const *buf, size_t len);
ssize_t boots_uart_read(int fd, uint8_t *buf, size_t len);

//---------------------------------------------------------------------------
#endif // __BOOTS_UART_H__

