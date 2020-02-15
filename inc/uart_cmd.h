#ifndef _UART_CMD_H_
#define _UART_CMD_H_ __FILE__

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"

void uart_cmd_remove_spaces(void);
void uart_cmd_match_command(void);
void uart_cmd_translate_params(void);
void uart_cmd_process(char c);
void uart_cmd_translate_params(void);

uint32_t cmd_help(uint32_t param1, uint32_t param2);
uint32_t cmd_start_measurement(uint32_t param1, uint32_t param2);
uint32_t cmd_stop_measurement(uint32_t param1, uint32_t param2);
uint32_t cmd_switch_app(uint32_t app_id, uint32_t param2);
uint32_t cmd_status(uint32_t param1, uint32_t param2);

#endif //_UART_CMD_H_
