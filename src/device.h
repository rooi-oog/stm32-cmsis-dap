#ifndef DEVICE_H
#define DEVICE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>

#if defined (USE_DEBUG)
	#define DEBUG(...)		printf (__VA_ARGS__)
	#define INFO(...)		printf (__VA_ARGS__)
	#define ERROR(...)		printf (__VA_ARGS__)
#elif defined (USE_INFO)
	#define DEBUG(...)
	#define INFO(...)		printf (__VA_ARGS__)
	#define ERROR(...)		printf (__VA_ARGS__)
#elif defined (USE_ERROR)
	#define DEBUG(...)
	#define INFO(...)
	#define ERROR(...)		printf (__VA_ARGS__)
#else
	#define DEBUG(...)
	#define INFO(...)
	#define ERROR(...)
#endif

#define JTAG_PORT			GPIOA
#define JTAG_TCK			GPIO0		// aka SWCLK
#define JTAG_TMS			GPIO1		// aka SWDIO
#define JTAG_TDI			GPIO2
#define JTAG_TDO			GPIO3
#define JTAG_nTRST			GPIO4
#define JTAG_nRESET			GPIO5

#define SW_PORT				GPIOA
#define SW_CLK				GPIO0
#define SW_DIO				GPIO1
#define SW_nRESET			GPIO5

#define LED_CONNECTED_PORT	GPIOC
#define LED_CONNECTED		GPIO13

#endif /* DEVICE_H */

