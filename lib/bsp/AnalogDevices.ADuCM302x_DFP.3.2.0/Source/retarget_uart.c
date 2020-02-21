/*
** I/O redirection support over UART, via SSL/DD.
** Copyright (C) 2017 Analog Devices, Inc. All Rights Reserved.
**
** This file is intended for use with the ARM:Compiler:IO:*:User
** components, which set up redirection of stdout and stderr.
*/

#include <stdio.h>
#include <stdint.h>
#include <drivers/uart/adi_uart.h>
#include <retarget_uart_config.h>


/* Amount of memory required by the driver for Rx and TX only. */
#define ADI_UART_MEMORY_SIZE  (ADI_UART_BIDIR_MEMORY_SIZE)

#define UART_DEVICE_NUM 0

/* Handle for the UART device */
static ADI_UART_HANDLE hDevice;

/* Memory for the driver */
static uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE];

bool Init_Uart(void) {
#if ADI_UART_SETUP_PINMUX
  static bool pinmux_done = false;

  if (!pinmux_done) {
    /* Configure GPIO0 pins 10 and 11 for UART0 TX and RX */
    uint32_t gpio0_cfg = *pREG_GPIO0_CFG;
    gpio0_cfg &= ~(BITM_GPIO_CFG_PIN10 | BITM_GPIO_CFG_PIN11);
    gpio0_cfg |= (1u << BITP_GPIO_CFG_PIN10) | (1u << BITP_GPIO_CFG_PIN11);
    *pREG_GPIO0_CFG = gpio0_cfg;
    pinmux_done = true;
  }
#endif
  
  if (hDevice == NULL) {
    /* Open the UART device. Data transfer is bidirectional with NORMAL mode by default.  */
    if(adi_uart_Open(UART_DEVICE_NUM,
                     ADI_UART_DIR_BIDIRECTION,
                     UartDeviceMem,
                     ADI_UART_MEMORY_SIZE,
                     &hDevice) != ADI_UART_SUCCESS) {
      return false;
    }
  }
  
  return true;
}

void Uninit_Uart(void) {
  /* Close the UART device */
  adi_uart_Close(hDevice);
  hDevice = NULL;
}

static int write_to_uart(int ch) {
  uint32_t hwError;
  if ((Init_Uart() == false) || (adi_uart_Write(hDevice, &ch, /*nBufSize=*/1, /*bDMA=*/false, &hwError) != ADI_UART_SUCCESS)) {
    return EOF;
  } else {
    return ch;
  }
}

int stdout_putchar(int ch) {
  return write_to_uart(ch);
}

int stderr_putchar(int ch) {
  return write_to_uart(ch);
}

int stdin_getchar(void) {
  return EOF;
}

int ttywrch(int ch) {
  return write_to_uart(ch);
}

void _sys_exit(int exit_value) {
#if ADI_UART_EXIT_BREAKPOINT
  __BKPT(0);
#else
  while (1) {
    /* Do nothing */
  }
#endif
}
