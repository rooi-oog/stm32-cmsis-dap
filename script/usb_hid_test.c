/*
 * gcc usb_hid_test.c `pkg-config --cflags --libs hidapi-hidraw` -o usb_hid_test
 */

#include <stdlib.h>
#include <hidapi.h>
#include <stdio.h>
#include <string.h>

#define USB_TIMEOUT       1000

int main (int argc, char *argv[])
{
	char buf [65]; sprintf (buf, "%s", "Hello from PC to stm32 USB and welcome back!!!\n");
	hid_device *dev = NULL;
	
	unsigned short vid = 0xc251;
	unsigned short pid = 0xf002;
	
	if (hid_init() != 0) {
		printf ("unable hid init\n");
		exit (1);
	}
	
	dev = hid_open(vid, pid, NULL);
	
	if (dev == NULL) {
		printf ("unable to open device\n");
		exit (1);
	}
	
	int retval = hid_write (dev, buf, 64);
	
	if (retval == -1) {
		printf ("error writitng data\n");
		exit (1);
	}
	
	memset (buf, 0, 65);
	
	while (1)
	{	
		retval = hid_read_timeout (dev, buf, 64, USB_TIMEOUT);
		
		if (retval == -1 || retval == 0) {
			printf ("error reading data\n");
			exit (1);
		}
		
		printf ("usb device return: = %s\n", buf);	
	}
	
	printf ("Exit\n");
	
	return 0;
}
