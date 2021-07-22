#ifndef	_BMP_H
#define _BMP_H

#include "lcd.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

//显示一张BMP图片
int Bmp_Show(struct LcdInfo *lcd,const char *bmp_name);




#endif