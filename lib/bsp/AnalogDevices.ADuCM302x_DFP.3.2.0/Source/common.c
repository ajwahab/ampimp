/*!
 *****************************************************************************
 * @file:    common.c
 * @brief:   Common utilities for testing
 *-----------------------------------------------------------------------------
 *
Copyright (c) 2010-2017 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#include "common.h"

char aDebugString[150u];

#ifdef REDIRECT_OUTPUT_TO_UART

#include <drivers/uart/adi_uart.h>
#include <drivers/general/adi_drivers_general.h>

ADI_UART_HANDLE hDevOutput = NULL;

ADI_ALIGNED_PRAGMA(4)
uint8_t OutDeviceMem[ADI_UART_UNIDIR_MEMORY_SIZE] ADI_ALIGNED_ATTRIBUTE(4);

#define UART0_TX_PORTP0_MUX (1u<<20)
#define UART0_RX_PORTP0_MUX (1u<<22)

#endif /* REDIRECT_OUTPUT_TO_UART */

/**
 * Test Initialization
 *
 * @brief  Test initialization
 *
 */
void common_Init(void)
{     
#ifdef REDIRECT_OUTPUT_TO_UART
    /* Set the pinmux for the UART */
    *pREG_GPIO0_CFG |= UART0_TX_PORTP0_MUX | UART0_RX_PORTP0_MUX;

    /* Open the UART device, data transfer is bidirectional with NORMAL mode by default */
    adi_uart_Open(0u, ADI_UART_DIR_TRANSMIT, OutDeviceMem, sizeof OutDeviceMem, &hDevOutput);
#endif
}


/**
 * Passing Result
 *
 * @brief  Report a passing test result
 *
 */
void common_Pass(void)
{
    char pass[] = "All done!\n\r";
    
#ifdef REDIRECT_OUTPUT_TO_UART
    uint32_t pHwError;
    /* Ignore return codes since there's nothing we can do if it fails */
    adi_uart_Write(hDevOutput, pass, strlen(pass), false, &pHwError);
#else
    printf(pass);
#endif
}


/**
 * Failing Result
 *
 * @brief  Report a failing test result
 *
 * @param  [in] FailureReason : String identifying the failure
 *
 */
void common_Fail(char *FailureReason)
{
    char fail[] = "Failed: ";
    char term[] = "\n\r";

#ifdef REDIRECT_OUTPUT_TO_UART
    uint32_t pHwError;
    /* Ignore return codes since there's nothing we can do if it fails */
    adi_uart_Write(hDevOutput, fail, strlen(fail), false, &pHwError);
    adi_uart_Write(hDevOutput, FailureReason, strlen(FailureReason), false, &pHwError);
    adi_uart_Write(hDevOutput, term, strlen(term), false, &pHwError);
#else
    printf(fail);
    printf(FailureReason);
    printf(term);
#endif
 }


/**
 * Info
 *
 * @brief  Report a failing test result
 *
 * @param  [in] InfoString : String with test information
 *
 */
void common_Perf(char *InfoString)
{
    char term[] = "\n\r";
    
#ifdef REDIRECT_OUTPUT_TO_UART
    uint32_t pHwError;
    /* Ignore return codes since there's nothing we can do if it fails */
    adi_uart_Write(hDevOutput, InfoString, strlen(InfoString), false, &pHwError);
    adi_uart_Write(hDevOutput, term, strlen(term), false, &pHwError);
#else
    printf(InfoString);
    printf(term);
#endif
}
