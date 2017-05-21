#ifndef USB_COMPOSITE_H
#define USB_COMPOSITE_H

void usb_composite_init (void);

void usb_hid_read (uint8_t *, int *);
uint8_t usb_hid_read_nonblock (void);
void usb_hid_write_nonblock (uint8_t *, int);
void usb_hid_write (uint8_t *, int); 

void usb_cdc_read (uint8_t *, int *);
uint8_t usb_cdc_read_nonblock (void);
void usb_cdc_write_nonblock (uint8_t *, int);
void usb_cdc_write (uint8_t *, int);

void _usb_write_packet (uint8_t, uint8_t *, int);


#endif /* USB_COMPOSITE_H */

