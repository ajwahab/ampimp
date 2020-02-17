/*****************************************************************************
 * HelloWorld.c
 *****************************************************************************/

#include <stdio.h>
#include <drivers/pwr/adi_pwr.h>
#include "HelloWorld.h"

int main(int argc, char *argv[])
{
    /* Initialize the power service */
    if (ADI_PWR_SUCCESS != adi_pwr_Init())
    {
        return 1;
    }
    if (ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1))
    {
        return 1;
    }
    if (ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1))
    {
        return 1;
    }

	/* Begin adding your custom code here */
	printf("Hello, world!\n");

	return 0;
}

