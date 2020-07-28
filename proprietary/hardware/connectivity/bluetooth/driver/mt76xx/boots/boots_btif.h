//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_BTIF_H__
#define __BOOTS_BTIF_H__

//---------------------------------------------------------------------------
int boots_btif_open(boots_if_e interface);
int boots_btif_close(boots_if_e interface);
int boots_btif_read(int fd, uint8_t *buf, int len);
int boots_btif_write(int fd, uint8_t *buf, int len);

//---------------------------------------------------------------------------
#endif // __BOOTS_BTIF_H__
