#ifndef _MAIN_H_
#define _MAIN_H_  __FILE__

#include <stdio.h>
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "retarget.h"
#include "ADuCM3029.h"
#include "ad5940.h"

enum app_ctrl{
  APPCTRL_START,
  APPCTRL_STOPNOW,
  APPCTRL_SHUTDOWN=4
  };

#endif //_MAIN_H_