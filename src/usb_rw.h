#ifndef USB_RW_HANDLER_H
#define USB_RW_HANDLER_H

typedef struct 
{
	uint8_t buf [64];
	uint8_t len;
} cdcacm_buf_t;

void usbhid_write (uint8_t *);
uint8_t usbhid_read_nonblock (void);
void usbhid_read (uint8_t **, int *);


#endif /* USB_RW_HANDLER_H */

