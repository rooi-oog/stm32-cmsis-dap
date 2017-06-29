/*
 * usb_composite.c
 * This file is part of stm32-cmsis-dap implementation
 *
 * Copyright (C) 2017 - Alex Petrov (rooi_oog@yahoo.com)
 *
 * stm32-cmsis-dap is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * stm32-cmsis-dap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with stm32-cmsis-dap. If not, see <http://www.gnu.org/licenses/>.
 */



#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/otg_fs.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/cdc.h>

#include "usb_comp_desc.h"
#include "usb_comp.h"

extern void usbhid_data_rx_cb (usbd_device *, uint8_t);
extern void usbhid_data_tx_cb (usbd_device *, uint8_t);
extern void cdcacm_data_rx_cb (usbd_device *, uint8_t);
extern void cdcacm_data_tx_cb (usbd_device *, uint8_t);

usbd_device *usbd_dev;

/* Buffer used for control requests. */
uint8_t usbd_control_buffer [128];

static int comp_control_request (usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *dev, struct usb_setup_data *req))
{
	(void) complete;
	(void) buf;
	(void) dev;
		
	if ((req->bmRequestType & USB_REQ_TYPE_DIRECTION) == USB_REQ_TYPE_IN)
	{
		if ((req->bmRequestType & USB_REQ_TYPE_TYPE) == USB_REQ_TYPE_STANDARD)
		{
			if (req->bRequest == USB_REQ_GET_DESCRIPTOR)
			{
					if (req->wValue == 0x2200)
					{
						*buf = (uint8_t *) hid_report_descriptor;
						*len = sizeof (hid_report_descriptor);
						return USBD_REQ_HANDLED;
					}					
					return USBD_REQ_NOTSUPP;		
			}
		}
	}
	
	if ((req->bmRequestType & USB_REQ_TYPE_TYPE) == USB_REQ_TYPE_CLASS)
	{	
		switch (req->bRequest) {
		case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
			/*
			 * This Linux cdc_acm driver requires this to be implemented
			 * even though it's optional in the CDC spec, and we don't
			 * advertise it in the ACM functional descriptor.
			 */
			char local_buf[10];
			struct usb_cdc_notification *notif = (void*)local_buf;

			/* We echo signals back to host as notification. */
			notif->bmRequestType = 0xA1;
			notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
			notif->wValue = 0;
			notif->wIndex = 0;
			notif->wLength = 2;
			local_buf[8] = req->wValue & 3;
			local_buf[9] = 0;		
			return 1;
		}
	
		case USB_CDC_REQ_SET_LINE_CODING:
			if (*len < sizeof(struct usb_cdc_line_coding))
				return 0;

			return 1;
			
		}
	}

	return USBD_REQ_NEXT_CALLBACK;
}

static void comp_set_config(usbd_device *dev, uint16_t wValue)
{
	(void) wValue;

	usbd_ep_setup (dev, 0x01, USB_ENDPOINT_ATTR_INTERRUPT, 64, usbhid_data_rx_cb);
	usbd_ep_setup (dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 64, usbhid_data_tx_cb);
	usbd_ep_setup (dev, 0x02, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
	usbd_ep_setup (dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_tx_cb);
	usbd_ep_setup (dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback (
				dev,
				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				comp_control_request);
}

void usb_composite_init (void)
{
	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, 
							usb_strings, 3, usbd_control_buffer, sizeof (usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, comp_set_config);
		
	
	// TODO: (2 << 4) why?! and why this do a deal?!
	// FIXME: (2 << 4) should be either named constant or define 
	nvic_set_priority (NVIC_USB_LP_CAN_RX0_IRQ, (2 << 4));
	nvic_enable_irq (NVIC_USB_LP_CAN_RX0_IRQ);
	nvic_enable_irq (NVIC_USB_WAKEUP_IRQ);
}

void usb_lp_can_rx0_isr (void) 
{
    usbd_poll (usbd_dev);
}

void usb_wakeup_isr (void) 
{
	usbd_poll (usbd_dev);
}
