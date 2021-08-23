
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lcd.h"
#include "2048.h"
#include "bmp.h"
#include "ts.h"

int main()
{
	srandom ( time(NULL) );

	//1. 打开屏幕
	lcd_open();
	restart:lcd_clear_screen(0x555555);
	// 关闭屏幕
	game_2048();
	int mv = get_user_input();
	if (mv)
	{	
		goto restart;
	}
	
	lcd_close();
	return 0;

}

