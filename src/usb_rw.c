/*
 * usb_rw.c
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
#include <stdio.h>
#include <errno.h>
#include <libopencm3/stm32/otg_fs.h>
#include <libopencm3/usb/usbd.h>

#include "DAP_config.h"
#include "usb_comp.h"
#include "usb_rw.h"

extern usbd_device *usbd_dev;
										
#define RINGBUF_INC(x, count)		x = (x + 1) & (count - 1)

/* ----------------------------------------------------------------------------------------------------------------
 * USB Human Interface Device 
 * ---------------------------------------------------------------------------------------------------------------- */

/* Incoming request buffer */
static uint8_t usbhid_rx_buf [DAP_PACKET_COUNT][DAP_PACKET_SIZE];
static volatile uint8_t usbhid_rx_produce;
static volatile uint8_t usbhid_rx_consume;

/* Outgoing response buffer */
static uint8_t usbhid_tx_buf [DAP_PACKET_COUNT][DAP_PACKET_SIZE];
static volatile uint8_t usbhid_tx_produce;
static volatile uint8_t usbhid_tx_consume;
static volatile bool usbhid_tx_cts = 1;


/* Data packet received from the Host */
void usbhid_data_rx_cb (usbd_device *usbd_dev, uint8_t ep)
{	
	(void) ep;
	
	usbd_ep_read_packet (usbd_dev, 0x01, usbhid_rx_buf [usbhid_rx_produce], 64);
	RINGBUF_INC (usbhid_rx_produce, DAP_PACKET_COUNT);
}

/* Data packet sent to the Host */
void usbhid_data_tx_cb (usbd_device *usbd_dev, uint8_t ep)
{
	(void) ep;
	
	if (usbhid_tx_produce != usbhid_tx_consume)
	{
		usbd_ep_write_packet (usbd_dev, 0x81, usbhid_tx_buf [usbhid_tx_consume], DAP_PACKET_SIZE);
		RINGBUF_INC (usbhid_tx_consume, DAP_PACKET_COUNT);
	}
	else
		usbhid_tx_cts = 1;
}

/* Send packet to the Host */
void usbhid_write (uint8_t *buf)
{
	if (usbhid_tx_cts)
	{
		usbhid_tx_cts = 0;
		usbd_ep_write_packet (usbd_dev, 0x81, buf, DAP_PACKET_SIZE);
	}
	else
	{
		memcpy (usbhid_tx_buf [usbhid_tx_produce], buf, DAP_PACKET_SIZE);
		RINGBUF_INC (usbhid_tx_produce, DAP_PACKET_COUNT);
		
		if (usbhid_tx_cts)
			usbhid_data_tx_cb (usbd_dev, 0x81);
	}
}

uint8_t usbhid_read_nonblock (void)
{
	return usbhid_rx_produce != usbhid_rx_consume;
}

void usbhid_read (uint8_t **buf, int *len)
{
	while (!usbhid_read_nonblock ());
	
	*buf = usbhid_rx_buf [usbhid_rx_consume];
	*len = DAP_PACKET_SIZE;
	RINGBUF_INC (usbhid_rx_consume, DAP_PACKET_COUNT);
}

/* ----------------------------------------------------------------------------------------------------------------
 * Communication Device Class (Abstract Control Model) 
 * ---------------------------------------------------------------------------------------------------------------- */
#if defined (USE_DEBUG) || defined (USE_INFO) || defined (USE_ERROR)
 
#define CDCACM_BUF_LEN				8	// for some reason (maybe it depends on wMaxPacketSize) this is the minimum 
										// value for correct operational
										
cdcacm_buf_t cdcacm_tx_buf [CDCACM_BUF_LEN];

static volatile uint8_t cdcacm_tx_produce;
static volatile uint8_t cdcacm_tx_consume;
static volatile bool cdcacm_tx_cts = 1;

void cdcacm_data_rx_cb (usbd_device *usbd_dev, uint8_t ep)
{
	(void) usbd_dev;
	(void) ep;
}

void cdcacm_data_tx_cb (usbd_device *usbd_dev, uint8_t ep)
{	
	(void) ep;
	
	if (cdcacm_tx_produce != cdcacm_tx_consume)
	{
		usbd_ep_write_packet (usbd_dev, 0x82, 
			cdcacm_tx_buf [cdcacm_tx_consume].buf, cdcacm_tx_buf [cdcacm_tx_consume].len);					
		RINGBUF_INC (cdcacm_tx_consume, CDCACM_BUF_LEN);
	}
	else
		cdcacm_tx_cts = 1;		
}

void cdcacm_write (uint8_t *buf, int len)
{	
	if (cdcacm_tx_cts)
	{
		cdcacm_tx_cts = 0;
		usbd_ep_write_packet (usbd_dev, 0x82, buf, len);
	}
	else
	{	
		memcpy (cdcacm_tx_buf [cdcacm_tx_produce].buf, buf, len);
		cdcacm_tx_buf [cdcacm_tx_produce].len = len;
		RINGBUF_INC (cdcacm_tx_produce, CDCACM_BUF_LEN);		// (***)
		
		// We should check a state of this flag because it may be changed in the interrupt
		if (cdcacm_tx_cts)
			// If so, we should call the interrupt handler by hand due to (***)
			// Actually I think that it is a some kind of hack
			// thus TODO: Think a lot in this way to remove the race condition
			// or find more beautiful (if any) decision (maybe full refactoring?)
			cdcacm_data_tx_cb (usbd_dev, 0x82);
	}	
}

uint8_t cdcacm_read_nonblock (void)
{
	 return 0;
}

void cdcacm_read (uint8_t **buf, int *len)
{
	(void) buf;
	(void) len;
}

int _write(int file, char *ptr, int len)
{
	if (file == 1) 
	{
		if (len <= 64)
		{
			cdcacm_write ((uint8_t *) ptr, len);
		}
		else
		{
			int i;
	
			for (i = 0; (i + 64) < len; i += 64)	
				cdcacm_write ((uint8_t *) &ptr [i], 64);
		
			cdcacm_write ((uint8_t *) &ptr [i], len - i - 1);
		}
	
		return len;
	}

	errno = EIO;
	return -1;	
}

#else

void cdcacm_data_rx_cb (usbd_device *usbd_dev, uint8_t ep)
{
	(void) usbd_dev;
	(void) ep;
}

void cdcacm_data_tx_cb (usbd_device *usbd_dev, uint8_t ep)
{
	(void) usbd_dev;
	(void) ep;
}

#endif
