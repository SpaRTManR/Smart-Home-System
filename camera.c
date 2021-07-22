#include "camera.h"
#include "lcd.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


#define  CAMERA_PATH	"/dev/video7"  //自己开发板上的设备号  video7 

#define OUTPUT_BUF_SIZE  4096
#define CLEAR(x) memset (&(x), 0, sizeof (x))

//摄像头采集图像的大小 (640,480)  (320,240)
#define WIDTH  320		
#define HEIGHT 240

//显示图像的起始坐标   如果想要显示(640,480)的图像，起始坐标一般为（0,0）
#define START_X   240
#define START_Y  60



#define LCD_WIDTH  			800
#define LCD_HEIGHT 			480
#define FB_SIZE				(LCD_WIDTH * LCD_HEIGHT * 4)

volatile int state = 0;
volatile int video_flag = 0;
volatile int photo_flag = 0;
volatile  int  capture_num = 0;
volatile int camera_flag=0; //volatile 是一个修饰符  表示易变的  摄像头采集画面的标志
static char g_color_buf[FB_SIZE]={0};
volatile char foldername[50];

static int  g_fb_fd;//LCD屏的文件描述符
static int *g_pfb_memory;//LCD屏的内存映射首地址


struct buffer {
	void   *start;
	size_t length;
};


typedef struct {

	struct jpeg_destination_mgr pub;
	JOCTET * buffer; 
	unsigned char *outbuffer;
	int outbuffer_size;
	unsigned char *outbuffer_cursor;
	int *written;

}mjpg_destination_mgr;

 
typedef mjpg_destination_mgr *mjpg_dest_ptr;

static char *           dev_name        = CAMERA_PATH;
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;
FILE *file_fd;
static unsigned long file_length;
static unsigned char *file_name;





 
METHODDEF(void) init_destination(j_compress_ptr cinfo)
{
	mjpg_dest_ptr dest = (mjpg_dest_ptr) cinfo->dest;
	dest->buffer = (JOCTET *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, OUTPUT_BUF_SIZE * sizeof(JOCTET));
	*(dest->written) = 0;
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

}

METHODDEF(boolean) empty_output_buffer(j_compress_ptr cinfo)
{

	mjpg_dest_ptr dest = (mjpg_dest_ptr) cinfo->dest;
	memcpy(dest->outbuffer_cursor, dest->buffer, OUTPUT_BUF_SIZE);
	dest->outbuffer_cursor += OUTPUT_BUF_SIZE;
	*(dest->written) += OUTPUT_BUF_SIZE;
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	return TRUE;

} 

METHODDEF(void) term_destination(j_compress_ptr cinfo)
{
	mjpg_dest_ptr dest = (mjpg_dest_ptr) cinfo->dest;
	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
	/* Write any data remaining in the buffer */
	memcpy(dest->outbuffer_cursor, dest->buffer, datacount);
	dest->outbuffer_cursor += datacount;
	*(dest->written) += datacount;

}

//获取文件大小
unsigned long file_size_get(const char *pfile_path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	
	if(stat(pfile_path, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	
	return filesize;
}





//初始化LCD
int lcd_open(const char *str)
{
	g_fb_fd = open(str, O_RDWR);
	
	if(g_fb_fd<0)
	{
			printf("open lcd error\n");
			return -1;
	}

	g_pfb_memory  = (int *)mmap(	NULL, 					//映射区的开始地址，设置为NULL时表示由系统决定映射区的起始地址
									FB_SIZE, 				//映射区的长度
									PROT_READ|PROT_WRITE, 	//内容可以被读取和写入
									MAP_SHARED,				//共享内存
									g_fb_fd, 				//有效的文件描述词
									0						//被映射对象内容的起点
								);
	if (MAP_FAILED == g_pfb_memory )
	{
		printf ("mmap failed %d\n",__LINE__);
	}

	return g_fb_fd;

}

//LCD关闭
void close_lcd(void)
{
	
	/* 取消内存映射 */
	munmap(g_pfb_memory, FB_SIZE);
	
	/* 关闭LCD设备 */
	close(g_fb_fd);
}



//LCD画点
void lcd_draw_point(unsigned int x,unsigned int y, unsigned int color)
{
	*(g_pfb_memory+y*800+x)=color;
}

//在LCD上显示jpg图片    x坐标      y坐标            图片路径          	数据缓冲区	     图片大小		  	   是否缩小一半
int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path,char *pjpg_buf,unsigned int jpg_buf_size,unsigned int jpg_half)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	char 	*pcolor_buf = g_color_buf;
	char 	*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	
			 int	jpg_fd;
	unsigned int 	jpg_size;
	
	unsigned int 	jpg_width;
	unsigned int 	jpg_height;
	
	lcd_open("/dev/fb0");

	if(pjpg_path!=NULL)
	{
		/* 申请jpg资源，权限可读可写 */	
		jpg_fd=open(pjpg_path,O_RDWR);
		
		if(jpg_fd == -1)
		{
		   printf("open %s error\n",pjpg_path);
		   
		   return -1;	
		}	
		
		/* 获取jpg文件的大小 */
		jpg_size=file_size_get(pjpg_path);	

		/* 为jpg文件申请内存空间 */	
		pjpg = malloc(jpg_size);

		/* 读取jpg文件所有内容到内存 */		
		read(jpg_fd,pjpg,jpg_size);
	}
	else
	{
		jpg_size = jpg_buf_size;
		
		pjpg = pjpg_buf;
	}

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*直接解码内存数据*/		
	jpeg_mem_src(&cinfo,pjpg,jpg_size);
	
	/*读文件头*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	
	
	
	if(jpg_half)
	{
		x_e	= x_s+(cinfo.output_width/2);
		y_e	= y  +(cinfo.output_height/2);		
		
		/*读解码数据*/
		while(cinfo.output_scanline < cinfo.output_height)
		{		
			pcolor_buf = g_color_buf;
			
			/* 读取jpg一行的rgb值 */
			jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&pcolor_buf,1);			
			
			/* 再读取jpg一行的rgb值 */
			jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&pcolor_buf,1);

			for(i=0; i<(cinfo.output_width/2); i++)
			{
				/* 获取rgb值 */
				color = 		*(pcolor_buf+2);
				color = color | *(pcolor_buf+1)<<8;
				color = color | *(pcolor_buf)<<16;
				
				/* 显示像素点 */
				lcd_draw_point(x,y,color);
				
				pcolor_buf +=6;
				
				x++;
			}
			
			/* 换行 */
			y++;					
			
			
			x = x_s;	

			
		}
	}
	else
	{
		x_e	= x_s+cinfo.output_width;
		y_e	= y  +cinfo.output_height;	

		/*读解码数据*/
		while(cinfo.output_scanline < cinfo.output_height )
		{		
			pcolor_buf = g_color_buf;
			
			/* 读取jpg一行的rgb值 */
			jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&pcolor_buf,1);
			
			for(i=0; i<cinfo.output_width; i++)
			{
				/* 获取rgb值 */
				color = 		*(pcolor_buf+2);
				color = color | *(pcolor_buf+1)<<8;
				color = color | *(pcolor_buf)<<16;
				
				/* 显示像素点 */
				lcd_draw_point(x,y,color);
				
				pcolor_buf +=3;
				
				x++;
			}
			
			/* 换行 */
			y++;			
			
			x = x_s;
			
		}		
	}
	

				
	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	if(pjpg_path!=NULL)
	{
		/* 关闭jpg文件 */
		close(jpg_fd);	
		
		/* 释放jpg文件内存空间 */
		free(pjpg);		
	}

	close_lcd();
	
	return 0;
}


void dest_buffer(j_compress_ptr cinfo, unsigned char *buffer, int size, int *written)
{
	mjpg_dest_ptr dest;
	if (cinfo->dest == NULL) {
	cinfo->dest = (struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(mjpg_destination_mgr));
	}

	dest = (mjpg_dest_ptr)cinfo->dest;
	dest->pub.init_destination = init_destination;
	dest->pub.empty_output_buffer = empty_output_buffer;
	dest->pub.term_destination = term_destination;
	dest->outbuffer = buffer;
	dest->outbuffer_size = size;
	dest->outbuffer_cursor = buffer;
	dest->written = written;

}


//摄像头采集的YUYV格式转换为JPEG格式
int compress_yuyv_to_jpeg(unsigned char *buf, unsigned char *buffer, int size, int quality) 
{

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	unsigned char *line_buffer, *yuyv;
	int z;
	static int written;
	//int count = 0;
	//printf("%s\n", buf);

	line_buffer = calloc (WIDTH * 3, 1);
	yuyv = buf;//将YUYV格式的图片数据赋给YUYV指针
	// printf("compress start...\n");

	cinfo.err = jpeg_std_error (&jerr);
	jpeg_create_compress (&cinfo);

	/* jpeg_stdio_dest (&cinfo, file); */

	dest_buffer(&cinfo, buffer, size, &written);

	cinfo.image_width = WIDTH;
	cinfo.image_height = HEIGHT;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;


	jpeg_set_defaults (&cinfo);
	jpeg_set_quality (&cinfo, quality, TRUE);
	jpeg_start_compress (&cinfo, TRUE);

	z = 0;

	while (cinfo.next_scanline < HEIGHT) 
	{
		int x;
		unsigned char *ptr = line_buffer;
		for (x = 0; x < WIDTH; x++) 
		{
			int r, g, b;
			int y, u, v;
			if (!z)
				y = yuyv[0] << 8;
			else
				y = yuyv[2] << 8;

			u = yuyv[1] - 128;
			v = yuyv[3] - 128;

			r = (y + (359 * v)) >> 8;
			g = (y - (88 * u) - (183 * v)) >> 8;
			b = (y + (454 * u)) >> 8;

			*(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
			*(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
			*(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

			if (z++) 
			{
				z = 0;
				yuyv += 4;
			}

		}
		row_pointer[0] = line_buffer;
		jpeg_write_scanlines (&cinfo, row_pointer, 1);

	}

	jpeg_finish_compress (&cinfo);
	jpeg_destroy_compress (&cinfo);
	free (line_buffer);
	
	return (written);

}

 
//读取一帧的内容
int read_frame (void)
{
	
	struct v4l2_buffer buf;
	int ret;
	unsigned int i;
	CLEAR (buf);
	char video_name[50] = {0};
	char photo_name[50] = {0} ;
	static int p = 0;
	static int v = 0; //区分每个视频和每张图片的参数
	static int vp = 0;
	
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	int ff = ioctl (fd, VIDIOC_DQBUF, &buf); //出列采集的帧缓冲
	if(ff<0)
		printf("failture\n");
	
	unsigned char src[buf.length+1];
	unsigned char dest[buf.length+1];  //保存转换好的颜色数据

	assert (buf.index < n_buffers);
	// printf ("buf.index dq is %d,\n",buf.index);

	memcpy(src, buffers[buf.index].start, buf.length);
	ret = compress_yuyv_to_jpeg(src, dest,(WIDTH * HEIGHT), 80);//数据转换
	
	//判断摄像头采集画面的标志  如果标志为1则显示画面
	if(camera_flag == 1)
	{	
		//           起始坐标（可以修改）   画面显示在LCD的中间
		lcd_draw_jpg(START_X,START_Y,NULL,dest,ret,0); //在LCD显示图片 ,dest==jpeg图片数据，ret==图片字节数	
	}

	

	if(photo_flag){
		printf("capture = %d",capture_num);
		sprintf(photo_name,"./img/capture_%d.jpg",capture_num ++);
		printf("begin to capture\n");
		photo_flag = 0;
		int photo_fd = open(photo_name,O_RDWR|O_CREAT);
		write(photo_fd,dest,sizeof(dest));
		
		close(photo_fd);
		sleep(1);

	}


	if(video_flag){
		while(state >= 2){
			v ++;
			vp = 0;
			state = 0;
			video_flag = 0;
			if(v > 2) v= 0;


		}
		printf("vp = %d",vp);
		sprintf(video_name,"./video/v%dphoto%d.jpg",v,vp++);
		printf("begin to vedio!!!\n");
		int video_fd = open(video_name,O_RDWR|O_CREAT);
		write(video_fd,dest,sizeof(dest));
		close(video_fd);
		lcd_draw_jpg(START_X,START_Y,NULL,dest,ret,0);
		usleep(10000);
	

	}
	
	
	ff=ioctl (fd, VIDIOC_QBUF, &buf); //重新入列
	if(ff<0)
		printf("failture VIDIOC_QBUF\n");

	return 1;

}

//获取摄像头采集的图像
int get_camera_picture(void)
{
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	unsigned int i;
	enum v4l2_buf_type type;
	

	
	file_fd = fopen("test-mmap.jpg", "w");//打开一张图片，没有则创建
	fd = open (dev_name, O_RDWR | O_NONBLOCK, 0);
	int ff=ioctl(fd, VIDIOC_QUERYCAP, &cap);//获取摄像头参数

	if(ff<0)
	printf("failture VIDIOC_QUERYCAP\n");

	struct v4l2_fmtdesc fmt1;
	int ret;

	memset(&fmt1, 0, sizeof(fmt1));
	fmt1.index = 0;

	fmt1.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt1)) == 0) //查看摄像头所支持的格式
	{
		fmt1.index++;
		printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
		fmt1.pixelformat & 0xFF, (fmt1.pixelformat >> 8) & 0xFF,
		(fmt1.pixelformat >> 16) & 0xFF, (fmt1.pixelformat >> 24) & 0xFF,fmt1.description);
	}

	CLEAR (fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = WIDTH; //320;
	fmt.fmt.pix.height      = HEIGHT;//240;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	ff = ioctl (fd, VIDIOC_S_FMT, &fmt); //设置图像格式
	if(ff<0)
	printf("failture VIDIOC_S_FMT\n");


	file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height; //计算图片大小
	struct v4l2_requestbuffers req;
	CLEAR (req);
	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	ioctl (fd, VIDIOC_REQBUFS, &req);  //申请缓冲，count是申请的数量
	if(ff<0)
	printf("failture VIDIOC_REQBUFS\n");

	if (req.count < 1)
	printf("Insufficient buffer memory\n");

	buffers = calloc (req.count, sizeof (*buffers));//内存中建立对应空间


	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf; 
		CLEAR (buf);
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)) //映射用户空间
			printf ("VIDIOC_QUERYBUF error\n");

		buffers[n_buffers].length = buf.length;
		//通过mmap建立映射关系
		buffers[n_buffers].start=mmap(
										NULL,
										buf.length,
										PROT_READ|PROT_WRITE,
										MAP_SHARED,
										fd,
										buf.m.offset); 
		
		if (MAP_FAILED == buffers[n_buffers].start)
		printf ("mmap failed\n");
	}


	for (i = 0; i < n_buffers; ++i)
	{
		struct v4l2_buffer buf;
		CLEAR (buf);
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))//申请到的缓冲进入列队
		printf ("VIDIOC_QBUF failed\n");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) //开始捕捉图像数据
	printf ("VIDIOC_STREAMON failed\n");

	sleep(1);
	for (;;) //这一段涉及到异步IO ==>while(1)
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO (&fds);//将指定的文件描述符集清空
		FD_SET (fd, &fds);//在文件描述符集合中增加一个新的文件描述符
		/* Timeout. */
		
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select (fd + 1, &fds, NULL, NULL, &tv);//判断是否可读（即摄像头是否准备好），tv是定时
		if (-1 == r)
		{
			if (EINTR == errno)
			continue;
			printf ("select err\n");
		}

		if (0 == r)
		{
			fprintf (stderr, "select timeout\n");
			exit (EXIT_FAILURE);
		}

		if (read_frame ())//如果可读，执行read_frame函数，把采集到的每一帧图像显示到开发板上
		{}//break;
	}
	unmap:
	for (i = 0; i < n_buffers; ++i)
	if (-1 == munmap (buffers[i].start, buffers[i].length))
		 printf ("munmap error");
		 type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

	if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))   
		printf("VIDIOC_STREAMOFF"); 
	close (fd);
	fclose (file_fd);
	exit (EXIT_SUCCESS);
	return 0;
}



