/**
* \file:    uart_cmd.c
* \author:  Adam Wahab
* \brief:   UART Command process Based on UARTCmd.C from Analog Devices, Inc.
* \date:    2020
* -----------------------------------------------------------------------------
*/
#include "uart_cmd.h"

#define LINE_BUF_SIZE (128)
#define CMD_TABLE_SIZE (5)

struct __uart_cmd_table
{
  void *p_obj;
  const char *p_cmd_name;
  const char *p_desc;
}uart_cmd_table[CMD_TABLE_SIZE] =
{
  {(void *)cmd_help, "help", "print supported commands"},
  {(void *)cmd_start_measurement, "start", "start selected application"},
  {(void *)cmd_stop_measurement, "stop", "stop selected application"},
  {(void *)cmd_switch_app, "switch", "stop current APP and switch to new APP set by parameter1"},
  {(void *)cmd_status, "status", "indicates whether system is ready to receive commands"}
};

uint32_t cmd_help(uint32_t param1, uint32_t param2)
{
  for(int i = 0;i<CMD_TABLE_SIZE;i++)
  {
    if(uart_cmd_table[i].p_obj)
      printf("%-8s\t%s\r\n", uart_cmd_table[i].p_cmd_name, uart_cmd_table[i].p_desc);
  }
  return 0x87654321;
}

char g_line_buf[LINE_BUF_SIZE];
uint32_t g_line_buf_idx = 0;
uint32_t g_token_count = 0;
void *g_p_obj_found = 0;
uint32_t g_param[2] = {0};

void uart_cmd_remove_spaces(void)
{
  int i = 0;
  g_token_count = 0;
  char flag_found_token = 0;
  while(i < g_line_buf_idx)
  {
    if(g_line_buf[i] == ' ') g_line_buf[i] = '\0';
    else break;
    i++;
  }
  if(i == g_line_buf_idx) return;  /* All spaces... */
  while(i < g_line_buf_idx)
  {
    if(g_line_buf[i] == ' ')
    {
      g_line_buf[i] = '\0';
      flag_found_token = 0;
    }
    else
    {
      if(flag_found_token == 0)
        g_token_count ++;
      flag_found_token = 1;
    }
    i++;
  }
}

void uart_cmd_match_command(void)
{
  char *pcmd = &g_line_buf[0];
  int i = 0;
  g_p_obj_found = 0;
  while(i < g_line_buf_idx)
  {
    if(g_line_buf[i] != '\0')
    {
      pcmd = &g_line_buf[i];
      break;
    }
    i++;
  }
  for(i=0;i<CMD_TABLE_SIZE;i++)
  {
    if(strcmp(uart_cmd_table[i].p_cmd_name, pcmd) == 0)
    {
      /* Found you! */
      g_p_obj_found = uart_cmd_table[i].p_obj;
      break;
    }
  }
}

/* Translate string 'p' to number, store results in 'Res', return error code */
static uint32_t str2num(char *s, uint32_t *res)
{
  char *p;
  unsigned int base = 10;

  *res = strtoul(s, &p, base);

  return 0;
}

void uart_cmd_translate_params(void)
{
  char *p = g_line_buf;
  g_param[0] = 0;
  g_param[1] = 0;
  while(*p == '\0') p++;    /* goto command */
  while(*p != '\0') p++;    /* skip command. */
  while(*p == '\0') p++;    /* goto first parameter */
  if(str2num(p, &g_param[0]) != 0) return;
  if(g_token_count == 2) return;           /* Only one parameter */
  while(*p != '\0') p++;    /* skip first command. */
  while(*p == '\0') p++;    /* goto second parameter */
  str2num(p, &g_param[1]);
}

void uart_cmd_process(char c)
{
  if(g_line_buf_idx >= LINE_BUF_SIZE-1)
    g_line_buf_idx = 0;  /* Error: buffer overflow */
  if( (c == '\r') || (c == '\n'))
  {
    uint32_t res;
    g_line_buf[g_line_buf_idx] = '\0';
    /* Start to process command */
    if(g_line_buf_idx == 0)
    {
      g_line_buf_idx = 0; /* Reset buffer */
      return;  /* No command inputs, return */
    }
    /* Step1, remove space */
    uart_cmd_remove_spaces();
    if(g_token_count == 0)
    {
      g_line_buf_idx = 0; /* Reset buffer */
      return; /* No valid input */
    }
    /* Step2, match commands */
    uart_cmd_match_command();
    if(g_p_obj_found == 0)
    {
      g_line_buf_idx = 0; /* Reset buffer */
      return;   /* Command not support */
    }
    if(g_token_count > 1)           /* There is parameters */
    {
      uart_cmd_translate_params();
    }
    /* Step3, call function */
    res = ((uint32_t (*)(uint32_t, uint32_t))(g_p_obj_found))(g_param[0], g_param[1]);
    // printf("res:0x%08x\r\n", res);
    g_line_buf_idx = 0;  /* Reset buffer */
  }
  else
  {
    g_line_buf[g_line_buf_idx++] = c;
  }
}
