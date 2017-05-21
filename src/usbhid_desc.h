#ifndef USBHID_DESCRIOPTORS_H
#define USBHID_DESCRIOPTORS_H

/* CMSIS-DAP usage HID descriptor */
static const uint8_t hid_report_descriptor [] = {
	0x06, 0x00, 0xFF,	// USAGE_PAGE (Vendor Defined Page 1)	
	0x09, 0x01,			// USAGE (Vendor Usage 1)	
	0xA1, 0x01,			// COLLECTION (Application)	 
	0x15, 0x00,			//   LOGICAL_MINIMUM (0)	 
	0x26, 0xFF, 0x00,	//   LOGICAL_MAXIMUM (255)
		 
  	0x75, 0x08,			//   REPORT_SIZE (8)	 
	0x95, 0x40,			//   REPORT_COUNT (64)	 
	0x09, 0x01,			//   USAGE (Vendor Usage 1)	
	0x81, 0x02,			//   INPUT (Data,Var,Abs)
		 
	0x95, 0x40,			//   REPORT_COUNT (64)	 
	0x09, 0x01,			//   USAGE (Vendor Usage 1)	
	0x91, 0x02,			//   OUTPUT (Data,Var,Abs)
	 
	0x95, 0x40,			//   REPORT_COUNT (64)	 
	0x09, 0x01,			//   USAGE (Vendor Usage 1)	
	0xB1, 0x02,			//   FEATURE (Data,Var,Abs)	 
	0xC0				// END_COLLECTION		
};


static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	}
};


static const struct usb_endpoint_descriptor hid_endpoints [] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 64,
	.bInterval = 0x02,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 64,
	.bInterval = 0x02,
}};

static const struct usb_interface_descriptor hid_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	
	.bInterfaceNumber = 2,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
	.endpoint = hid_endpoints,
	.extra = &hid_function,
	.extralen = sizeof (hid_function),
};

#endif /* USBHID_DESCRIOPTORS_H */

