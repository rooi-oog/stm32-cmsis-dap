#ifndef USB_COMPOSITE_DESCRIPTORS_H
#define USB_COMPOSITE_DESCRIPTORS_H

#include "usbhid_desc.h"
#include "cdcacm_desc.h"

/* Product description */
static const char *usb_strings[] = {
	"Unitcomp Ltd.",
	"CMSIS-DAP",
	"N/A",
};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}, {
	.num_altsetting = 1,
	.altsetting = &hid_iface,
}};

/* Device descriptor */
static const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = 64,
	.idVendor = 0xc251,
	.idProduct = 0xf002,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/* Configuration descriptor */
static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	
	.bNumInterfaces = 3,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,
	.interface = ifaces,
};


#endif /* USB_COMPOSITE_DESCRIPTORS_H */

