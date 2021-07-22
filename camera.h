#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <getopt.h>          
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>  //V4L2的头文件
#include "jpeglib.h"





//摄像头采集的YUYV格式转换为JPEG格式
int compress_yuyv_to_jpeg(unsigned char *buf, unsigned char *buffer, int size, int quality) ;

//读取一帧的内容
int read_frame (void);

//获取摄像头采集的图像
int get_camera_picture(void);

//获取文件大小
unsigned long file_size_get(const char *pfile_path);

//初始化LCD
int lcd_open(const char *str);

//LCD关闭
void close_lcd(void);

//显示jpg图片
int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path,char *pjpg_buf,unsigned int jpg_buf_size,unsigned int jpg_half);

//LCD画点
void lcd_draw_point(unsigned int x,unsigned int y, unsigned int color);

#endif