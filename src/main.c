/*
 * main.c
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

#include "DAP_config.h"
#include "DAP.h"
#include "usb_comp.h"
#include "usb_rw.h"

uint8_t *request;
uint8_t response [DAP_PACKET_SIZE]; 

static void clock_setup (void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz ();	
}

static void usb_setup (void)
{
	rcc_periph_clock_enable (RCC_GPIOA);
	rcc_periph_clock_enable (RCC_OTGFS);
	usb_composite_init ();
}

static void gpio_setup (void)
{
	/* Single LED onboard :( */
	rcc_periph_clock_enable (RCC_GPIOC);
	gpio_set_mode (GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	
	/* LED off */	
	gpio_set (GPIOC, GPIO13);
	
	/* Configure JTAG pins according to CMSIS-DAP Hardware pins support table
	 * described in DAP_config.h */ 
	DAP_SETUP ();
}

void main (void)
{	
	clock_setup ();
	usb_setup ();
	gpio_setup ();	
	
	// delay ~100ms
	for (uint32_t i = 0; i < rcc_ahb_frequency / 10; i++)
		__asm__ ("nop");
		
	while (1)
	{
		/* When packet received from the Host */
		if (usbhid_read_nonblock ())
		{
			int len;
			
			/* Pick request from the usb Host */
			usbhid_read (&request, &len);
			
			/* Handle the request */
			DAP_ProcessCommand (request, response);
			
			/* Send response to the Host */
			usbhid_write (response);
		}
	}	
}

