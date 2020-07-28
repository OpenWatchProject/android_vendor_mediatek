#ifndef __PCCIF_H__
#define __PCCIF_H__

void ccif_irq0_handler(void *cookie);
void ccif_irq1_handler(void *cookie);
void pseudo_ccif_handler(void __unused * cookie);
void pccif1_hw_init(void);
int ccif_irq0_user_write(unsigned int buf[], unsigned int size, unsigned int offset);
int ccif_irq1_user_write(unsigned int buf[], unsigned int size, unsigned int offset);
int ccif_irq0_user_notify_md(unsigned int ch);
int ccif_irq1_user_notify_md(unsigned int ch);
int ccif_irq0_user_read(unsigned int buf[], unsigned int size, unsigned int offset);
int ccif_irq1_user_read(unsigned int buf[], unsigned int size, unsigned int offset);

#endif
