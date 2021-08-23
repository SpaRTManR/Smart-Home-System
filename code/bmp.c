#include "bmp.h"


/*********************************************************
*		函数作用	：	显示一张BMP图片
*		函数参数	：
*					lcd  表示LCD的设备信息  struct LcdInfo *
*		返回值	：	bmp_name  指的是BMP图片的名字
*		注意事项	：  只能显示800*480大小的图片
*		函数作者	：  xxx
*		函数时间	：  15-07-2021
*		函数版本	：  V1.0
*
* *******************************************************/
int Bmp_Show(struct LcdInfo *lcd,const char *bmp_name)
{

	//1、打开BMP图片  open       只读  
	int bmp_fd = open(bmp_name,O_RDWR);
	if(bmp_fd < 0) //出错判断
	{
		perror("Open Bmp Error\n");
		return -1;
	}

	//2、跳过BMP图片的文件头信息   文件光标往后偏移54字节   lseek  54  SEEK_SET
	lseek(bmp_fd,54,SEEK_SET);

	//3、读取BMP图片的颜色数据并保存在数据缓冲区  read   char buf[800*480*3]  
	char bmp_buf[800*480*3] = {0};

	int cnt = read(bmp_fd,bmp_buf,800*480*3);
	if(cnt != 800*480*3) //出错判断
	{
		perror("Read Bmp Data Error");
		return -1;
	}

	//4、关闭BMP图片  close
	close(bmp_fd);


	//5、算法处理，把3字节的数据放入4字节的空间中  800*480    小学数学
	int lcd_buf[800*480] = {0}; //相当于是一块新的LCD

	int i,j;
	for(i=0,j=0;i<800*480;i++,j+=3)
	{
		lcd_buf[i] = bmp_buf[j] | bmp_buf[j+1]<<8 | bmp_buf[j+2]<<16;
	}
	
	int x,y;
	
	for(y=0;y<480;y++)
	{
		for(x=0;x<800;x++)
		{
			*(lcd->mp+800*y+x) = lcd_buf[800*(479-y)+x]; //* 表示对指针进行解引用  int * p;  *p = 10; 
		}
	}

}