#ifndef _MAIN_H_
#define _MAIN_H_  __FILE__

#include "string.h"
#include "math.h"
#include "stdio.h"
#include "retarget.h"
#include "ADuCM3029.h"
#include "ad5940.h"
#include "uart_cmd.h"

enum {
  APP_CTRL_START,
  APP_CTRL_STOP_NOW,
  APP_CTRL_SHUTDOWN = 4
};

enum {
  APP_ID_AMP,
  APP_ID_IMP,
  APP_NUM
};

void ampimp_main(void);

#define APP_AMP_SEQ_ADDR    (0)
#define APP_IMP_SEQ_ADDR    (256)

#define APP_AMP_MAX_SEQLEN  (256)
#define APP_IMP_MAX_SEQLEN  (128)

#define APP_BUF_SIZE        (512)

#define AMPIMP_PKT_HEAD "F09F91BF" //imp emoji
#define AMPIMP_PKT_TAIL "0D0A" //CR+LF

#endif //_MAIN_H_
