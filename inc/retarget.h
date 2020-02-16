
#ifndef _RETARGET_H_
#define _RETARGET_H_  __FILE__

#include "main.h"

struct __FILE { int handle; /* add here as needed */ };

int fgetc(FILE *f);
int fputc(int c, FILE *f);
int _read(int fd, char *ptr, int len);
int _write(int fd, char *ptr, int len);

#endif //_RETARGET_H_
