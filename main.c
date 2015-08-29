
#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include "lcd_driver.h"
#include "res.h"
#include "painter.h"

void Delay(__IO uint32_t nCount);

//#define DUMMY Delay(11345678)
#define DUMMY while(getTime_ms() % 1000)
#define LDUMMY {DUMMY;DUMMY;DUMMY;DUMMY;}

extern DrawLineContext _d_dl_ctx;

void long_shadow(){ // WE LOVE IT! NO ANTIALIAS!!
	s16 x, y;
	LCD_SetWindow(30, 30, 210, 210);
	for (y=30;y<240;y++)for (x=30;x<240;x++){
		if (((x-120)*(x-120)+(y-120)*(y-120)<119*119)&&
			(x>y-138)&&(x<y+138)&&(x+y>240))
				LCD_MixPixel_x32(0x0, 12);
		else
			LCD_GetPixel();
	}

}

__IO u32 _sys_tick;
u32 getTime_ms(){
	return (_sys_tick+1)/2;
}

u32 last_time = 0;
u32 elapse_ms(){
	u32 t0, t1;
	t0 = getTime_ms();
	t1 = t0 - last_time;
	last_time = t0;
	return t1;
}

u32 fps(){
	static u32 last;
	u32 t = elapse_ms();
	t = t?t:1;
	t = (t+last)/2;
	last = t;
	return (2000+t)/(t*2);
}

const u16 *number[] = { res_string_0/*_FSTR_0*/,
						res_string_1/*_FSTR_1*/,
						res_string_2/*_FSTR_2*/,
						res_string_3/*_FSTR_3*/,
						res_string_4/*_FSTR_4*/,
						res_string_5/*_FSTR_5*/,
						res_string_6/*_FSTR_6*/,
						res_string_7/*_FSTR_7*/,
						res_string_8/*_FSTR_8*/,
						res_string_9/*_FSTR_9*/};

u16 *itoa(u32 n, u16 *s){
	if (n>=10) s = itoa(n/10, s);
	*s = ((u16 *)number[n % 10])[0];
	*(++s) = 0;
	return s;
}

void put_fps(u32 multiples){
	u16 s[10];
	itoa(fps()*multiples, s);
	Painter_PutString(s, 14, 0xffff, 0x0f
					 , 0, 0, 8*4, 14, 0);
}

void benchmark(u32 cnt){
	u32 i;
	while (1){
		put_fps(cnt);
		i = cnt;
		while (i--){
//			for (u32 j=0;j<2000;j++) LCD_SetEntryMode(j, i); // 1332000aps
//			for (u32 j=0;j<1000;j++) LCD_SetPoint(j, i); // 200000aps
//			for (u32 j=0;j<1000;j++) LCD_SetPoint_InCtx(j, i); // 200000aps
//			for (u32 j=0;j<1000;j++) LCD_SetWindow(i, i, j/5, j/4); // 666000aps
//			LCD_SetWindow(0,100,200,200);for (u32 j=0;j<10000;j++) LCD_GetPixel(); //2500000pps // 2860000pps(0.572x)
//			LCD_SetWindow(0,100,200,200);for (u32 j=0;j<10000;j++) LCD_PutPixel(C_RGB565(0xff, 0x22, 0x66)); // 5000000pps(1x)
//			LCD_SetWindow(0,100,200,200);for (u32 j=0;j<10000;j++) LCD_MixPixel_x32(0xf0206, 0); // 1540000pps(0.308x)
//			LCD_SetWindow(0,100,200,200);LCD_PutImage_RGB565(0, 10000); // 666fps/6660000pps
//			LCD_SetWindow(0,100,0x8,0xe);for (u32 j=0;j<100;j++)LCD_PutChar_RGB565(res_glyphs+3, 0xe*0x8, C_RGB565(0xff, 0x22, 0x66), 0xff); // 76fps/851200pps(0.17x)

//			LCD_FillRectangle_RGB565(0, 14, 240, 320-14, C_RGB565(0xff, 0x22, 0x66)); //154fps// 166fps/12191040pps(1x)
//			LCD_FillRectangle_RGB4444(0, 14, 240, 320-14, 0xf263); //28fps// 30fps/2203200pps(0.18x)
//			LCD_FillCircle_RGB565(120, 120, 119, C_RGB565(0xff, 0x22, 0x66)); //68fps// 80fps/4569680pps(0.375x)
//			LCD_FillCircle_RGB4444(120, 120, 119, 0xf263); // 30fps/1713630pps(0.14x)

			u16 lw;
			lw = 0;
//			Painter_DrawLine(30, 30, 180, 30, 0xffff, lw, 0);// 2000fps+
//			Painter_DrawLine(30, 30, 30, 180, 0xffff, lw, 0);// 2000fps+
//			Painter_DrawLine(50, 50, 200, 51, 0xffff, lw, 0);// 2000fps+
//			Painter_DrawLine(50, 50, 51, 200, 0xffff, lw, 0);// 2000fps+
//			Painter_DrawLine(50, 50, 200, 250, 0xffff, lw, 0); // 1000fps+

			lw = 5;
//			Painter_DrawLine(30, 30, 180, 30, 0xffff, lw, 0);// 1000fps+
//			Painter_DrawLine(30, 30, 30, 180, 0xffff, lw, 0);// 1000fps+
//			Painter_DrawLine(50, 50, 200, 51, 0xffff, lw, 0);// 1000fps+
//			Painter_DrawLine(50, 50, 51, 200, 0xffff, lw, 0);// 666fps
//			Painter_DrawLine(50, 50, 200, 250, 0xffff, lw, 0); // 400fps, 700000pps


			lw = 12;
//			Painter_DrawLine(30, 30, 180, 30, 0xffff, lw, 0);// 400fps,840000pps
//			Painter_DrawLine(30, 30, 30, 180, 0xffff, lw, 0);// 400fps,840000pps
//			Painter_DrawLine(50, 50, 200, 51, 0xffff, lw, 0);// 400fps,840000pps
//			Painter_DrawLine(50, 50, 51, 200, 0xffff, lw, 0);// 333fps,700000pps
////			Painter_DrawLine(50, 50, 200, 250, 0xffff, lw, 0); // 222fps, 777000pps
//			Painter_DrawLine(50, 50, 200, 230, 0xffff, lw, 0); // 222fps, 777000pps


//			Painter_DrawLine(50, 50, 200, 250, 0xf262, 0, 0); // 1000fps+
//			Painter_DrawLine(50, 50, 200, 250, 0xf262, 5, 0); // 400fps
//			Painter_DrawLine(50, 50, 200, 250, 0xf262, 30, 0); // 84fps,720000pps
//			Painter_LocateContextBitmask(50, 50);
//			Painter_DrawLine(50, 50, 200, 250, 0xf262, 5, PAINTER_DRAW_BM_HOLD); // 666fps, border redraw

// 			u16 poly_x[] = {57, 103, 167, 171,  39, 152, 106, 180,  23, 173,  45, 115,  87,
// 							27,  21,  38, 145, 141,  78,  62,  37,  79, 199, 130, 201,  52,
// 						   137, 100,  50,  56};
// 			u16 poly_y[] = {124,  56,  31, 292, 248, 294,  38,  51,  73,  70, 275,  37,  20,
// 							244, 176, 229, 146, 271,  64, 281, 272, 296,  42,  31, 127,  85,
// 							167, 222, 184, 265};
//			u8 cs[] = {0, 1, 1, 0, 1, 1, 0, 1, 1, 0,
//					   1, 1, 0, 1, 1, 0, 1, 1, 0, 1,
//					   1, 0, 1, 1, 0, 1, 1, 0, 1, 0};

//			Painter_DrawPoly(poly_x, poly_y, 30, 0x6fc1, 1, 0); // 12fps(x1)
//			Painter_DrawPoly(poly_x, poly_y, 30, 0x6fc1, 5, 0); // 10fps
//			Painter_DrawPoly(poly_x, poly_y, 30, 0x6fc1, 12, 0); // 8fps(x0.72)

//			Painter_DrawCircle(120, 120, 100, 0xf263, 5, 0); //100fps, 527787pps, some blank not skipped

//			Painter_LocateContextBitmask(20, 20);
//			Painter_DrawCubicCurve(20, 20, 20, 200, 200, 20, 200, 200, 0xfff4, 5, 0);//250fps
//			Painter_DrawCubicCurve(20, 20, 20, 200, 200, 20, 200, 200, 0xfff4, 5, PAINTER_DRAW_BM_HOLD);
//			Painter_DrawBezier(poly_x, poly_y, cs, 30, 0x6fc9, 5, 0);//28fps

			u16 acroread_x[] = {183, 154, 128, 113, 100,  95,  98, 100, 141, 134, 124,  98,  57,
							   4, 204, 222, 235, 222, 183};
			u16 acroread_y[] = {103, 113, 134, 159, 179, 200, 221, 236, 229, 196, 123,  37,  49,
								 61, 165, 137, 120,  91, 103};
			u8 cs[] = {0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0};

			Painter_LocateContextBitmask(acroread_x[0], acroread_y[0]);
			Painter_DrawBezier(acroread_x, acroread_y, cs, 19, 0xf22f, 9, PAINTER_DRAW_BM_HOLD);
			Painter_Fill_BitMaskShadow(10, 10, 230, 300, 0L, 10, 10, 0, 0x000d, -6, 6, 0);


//			Painter_PutImage(res_img_res_lp_jpg,0,10,0xff); //100fps, 7680000pps(x0.63)
//			Painter_PutImage(res_img_res_lp_jpg,0,10,0x2f); //24fps, 1843200pps(x0.139)
//			Painter_PutImage(res_img_res_fg_png,0,100,0xff); //46fps, 1472000pps(x0.121)
//			Painter_PutImage(res_img_res_fg_png,0,100,0x2f); //48fps, 1536000pps(x0.126)


//			u16 x, y, i;
//			u32 t;
//			for (x=0;x<200;x++) for (y=0;y<200;y++){
//				t = (x-100)*(x-100)+(y-100)*(y-100);
//				if ((t>=91*91)&&(t<100*100)) LCD_SetBitMask(_d_dl_ctx.bm, x, y, 240);
//			}
//			Painter_BitMaskImage(res_img_res_fg_png, 0, 100, _d_dl_ctx.bm);//58fps
//			Painter_BitMaskImage(res_img_res_lp_jpg, 0, 10, _d_dl_ctx.bm);//36fps

			const u16 *text = res_string_10/*_FSTR_对于在快速处理器上使用的较慢显示控制器,使用端口线可能是唯一的解决方案。这种访问显示器的方法有一个缺点,比直接总线接口稍微慢些,但是缓存能最小化对显示器的访问,因此显示更新不会显著减慢。所有需要做的事情就是定义例程或宏,设置或读取显示器。*/;
//			Painter_PutString(text, 24, 0xfff2, 0, 0, 14, 240, 320, 0); // 26fps
//			Painter_PutString(text, 24, 0x000a, 0xfff6, 0, 14, 240, 320, 0); // 14fps
//			Painter_PutString(text, 24, 0x000a, 0xfff6, 0, 14, 240, 320, PAINTER_STR_SFLUSH); // 14fps
//			Painter_PutString(text, 24, 0xfffe, 0x888e, 0, 14, 240, 320, PAINTER_STR_SHADOW*2); // 8fps
//			Painter_PutString(text, 24, 0xfffe, 0x888e, 0, 14, 240, 320, PAINTER_STR_SFLUSH | PAINTER_STR_SHADOW*2); // 8fps


//			u16 x, y, i;
//			u32 t0, t1;
//			for (x=0;x<240;x++) for (y=0;y<240;y++){
//				t0 = (x-120)*(y-120);
//				t1 = (x-120)*(x-120)+(y-120)*(y-120);
//				if ((t0*t0>=150*150)&&((t1<70*70)||(t1>80*80))) LCD_SetBitMask(_d_dl_ctx.bm, x, y, 240);
//				else LCD_ResetBitMask(_d_dl_ctx.bm, x, y, 240);
//			}//23.8ms
//			Painter_Fill_Floodfill(0,20,239,319, 120,120,120,120,0L,0,0xf264,0);//21.65ms
//			Painter_Fill_Floodfill(0,20,239,319, 120,120,120,120,0L,0,0xf264,PAINTER_FILL_CONNECT8);//26ms

//			Painter_Fill_BitMaskShadow(0, 20, 239, 259, 0L, 0, 20, 0, 0xf268, 1, 1, 0); // 38.7ms
//			Painter_Fill_BitMaskShadow(0, 20, 239, 259, 0L, 0, 20, 0, 0xf268, 1, 10, 0); // 76.2ms
//			Painter_Fill_BitMaskShadow(0, 20, 239, 259, 0L, 0, 20, 0, 0xf268, 10, 10, 0); // 101ms
//			Painter_Fill_BitMaskShadow(0, 20, 239, 259, 0L, 0, 20, 0, 0xf268, 4, 4, 0); // 60ms
//			Painter_Fill_BitMaskShadow(0, 20, 239, 259, 0L, 0, 20, 0, 0xf268, 4, 4, 30); // 140ms

//			Painter_PutString(text, 24, 0xfffe, 0x888e, 20, 20, 200, 200, PAINTER_STR_SFLUSH | PAINTER_STR_SHADOW*2); // 16fps
//			LCD_SetEntryMode(i, 0);

//			while(1);

		}
	}
}

int main(){
	SysTick_Config(SystemCoreClock / 2000);  // 0.5ms

	// NEED OF KEY AND LED
	RCC_ClockSecuritySystemCmd(ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Init LCD
	LCD_Cmd_InitFSMC();
	LCD_Cmd_Init();
	LCD_Cmd_InitBacklight();

	// Put some background
//	LCD_FillRectangle_RGB565(0, 0, 240, 320, C_RGB565(0x80, 0x80, 0x80));
	Painter_PutImage(res_img_res_lp_jpg,0,0,0xff);

//	Painter_SetupContextBitmask(240, 320, 0);
//	benchmark(2);

	// A circle to overlay long shadow
	LCD_FillCircle_RGB4444(120, 120, 100+18, 0x88fa);

	// now you can see it
	long_shadow();
	// an alternative shadow
//	LCD_FillCircle(120+1, 120+1, 100+2, 0x8886, 0);
//	LCD_FillCircle(120, 120, 100+2, 0x8886, 0);
//	LCD_FillCircle(120, 120, 100+1, 0x8886, 0);

	// Put my clock panel on screen
	LCD_FillCircle_RGB565(120, 120, 100, C_RGB565(0, 0, 0));
	LCD_FillCircle_RGB565(120, 120, 90, C_RGB565(0xff, 0xff, 0xff));

	// Clock frame need some shadow!
	Painter_SetupContextBitmask(210, 210, 0);
	u16 x, y, i;
	u32 t;
	for (x=0;x<200;x++) for (y=0;y<200;y++){
		t = (x-100)*(x-100)+(y-100)*(y-100);
		if ((t>=91*91)&&(t<100*100)) LCD_SetBitMask(_d_dl_ctx.bm, x, y, 210);
	}

	// Draw ticks and numbers
	const s8 TICK_XI[] = { 0,  42,  73,  85,  73,  42,   0, -42, -73, -85, -73, -42};
	const s8 TICK_YI[] = {-85, -73, -42,   0,  42,  73,  85,  73,  42,   0, -42, -73};
	const s8 TICK_XO[] = { 0,  44,  77,  90,  77,  45,   0, -44, -77, -90, -77, -45};
	const s8 TICK_YO[] = {-90, -77, -45,   0,  44,  77,  90,  77,  45,   0, -44, -77};
	const u16 *text[] = {res_string_0/*_FSTR_0*/,
			res_string_1/*_FSTR_1*/,
			res_string_2/*_FSTR_2*/,
			res_string_3/*_FSTR_3*/,
			res_string_4/*_FSTR_4*/,
			res_string_5/*_FSTR_5*/,
			res_string_6/*_FSTR_6*/,
			res_string_7/*_FSTR_7*/,
			res_string_8/*_FSTR_8*/,
			res_string_9/*_FSTR_9*/,
			res_string_11/*_FSTR_10*/,
			res_string_12/*_FSTR_11*/,
			res_string_13/*_FSTR_12*/
			};
	const u16 *col = res_string_14/*_FSTR_:*/;
	const u16 *space = res_string_15/*_FSTR_ */;
	u16 time[20];

	for (i=0;i<12;i++){
		Painter_LocateContextBitmask(100+TICK_XO[i], 100+TICK_YO[i]);
		Painter_DrawLine(120+TICK_XO[i], 120+TICK_YO[i], 120+TICK_XI[i], 120+TICK_YI[i], 0x000f, 5, PAINTER_DRAW_BM_HOLD);
		Painter_PutString(text[i?i:12], 14, 0x000f, 0
						 , 120+TICK_XI[i]-(TICK_XO[i]-TICK_XI[i])*2-5, 120+TICK_YI[i]-(TICK_YO[i]-TICK_YI[i])*2-8
						 , 20, 20, PAINTER_STR_SHADOW*2);
	}

	// Bravo!
	Painter_Fill_BitMaskShadow(30, 30, 230, 230, 0L, 10, 10, 210, 0x0008, 4, 4, 15);

	// LET IT RUN!
	u8 m, s; // so-called second and minute
	bitmask bm;
	_UNUSED(bm);
	s = 0; m = 0;
	Painter_SetupContextBitmask(200, 200, 0); // new context for new shadow
	Painter_LocateContextBitmask(100, 100); // start from center
	u8 shadow_on;
	u16 bg[200*60];
	// grab screen region, saving for later repaint
	LCD_SetWindow(20, 240, 200, 60);
	LCD_GetImage_RGB565(bg, 200*60);
	while (1){
		// PUSH BUTTON TO RUN REALLY FAST, WITHOUT SHADOW!
		shadow_on = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15)?PAINTER_STR_SHADOW*2:0;
		LCD_FillCircle_RGB565(120, 120, 65, 0xffff); // Clear clock hands, sucks aha~
		// Dont forget bitmasks
		for (x=30;x<170;x++) for (y=30;y<170;y++){
			LCD_ResetBitMask(_d_dl_ctx.bm, x, y, 200);
		}
		// second hand
		u16 sh_x[] = {120, 120+(TICK_XI[_mod(s+11,12)]+3)/5,
					  120+TICK_XI[s]-(TICK_XO[s]-TICK_XI[s])*6,
					  120+(TICK_XI[_mod(s+1,12)]+3)/5
					 };
		u16 sh_y[] = {120, 120+(TICK_YI[_mod(s+11,12)]+3)/5,
					  120+TICK_YI[s]-(TICK_YO[s]-TICK_YI[s])*6,
					  120+(TICK_YI[_mod(s+1,12)]+3)/5
					 };
		// Center circle makes it cute~
		Painter_DrawCircle(120, 120, 4, 0x6a2f, 12, PAINTER_DRAW_BM_HOLD);
		Painter_DrawPoly(sh_x, sh_y, 4, 0x6a2f, 3, PAINTER_DRAW_BM_HOLD | PAINTER_DRAW_POLY_CLOSE);
//		Painter_DrawLine(120, 120
//					, 120+TICK_XI[s]-(TICK_XO[s]-TICK_XI[s])*6
//					, 120+TICK_YI[s]-(TICK_YO[s]-TICK_YI[s])*6,
//					0x6a2f, 5, PAINTER_DRAW_BM_HOLD);
//		Painter_DrawLine(120, 120
//					 , 120-(TICK_XI[s]+4)/8
//					 , 120-(TICK_YI[s]+4)/8,
//				0x6a2f, 5, PAINTER_DRAW_BM_HOLD);
		// GET MY SHADOW!
		if (shadow_on) Painter_Fill_BitMaskShadow(50, 50, 190, 190, 0L, 30, 30, 200, 0x0008, 4, 4, 12);
		for (x=30;x<170;x++) for (y=30;y<170;y++){
			LCD_ResetBitMask(_d_dl_ctx.bm, x, y, 200);
		}
		u32 mm = m % 12;
		// minute hand
		Painter_DrawLine(120, 120
						 , 120+TICK_XI[mm]-(TICK_XO[mm]-TICK_XI[mm])*10
						 , 120+TICK_YI[mm]-(TICK_YO[mm]-TICK_YI[mm])*10,
						 0xa22f, 5, PAINTER_DRAW_BM_HOLD);
		Painter_DrawLine(120, 120
						 , 120-(TICK_XI[mm]+4)/8
						 , 120-(TICK_YI[mm]+4)/8,
						 0xa22f, 5, PAINTER_DRAW_BM_HOLD);
		// Center circle makes it cute~
		Painter_DrawCircle(120, 120, 3, 0xa22f, 5, PAINTER_DRAW_BM_HOLD);
		// GET MY SHADOW!
		if (shadow_on) Painter_Fill_BitMaskShadow(50, 50, 190, 190, 0L, 30, 30, 200, 0x0008, 4, 4, 12);

		// show time in text
		u16 *p = time, *q;
		// need some space
		*p++ = space[0];
		mm = m?m:1;
		while (mm<10000) {
			*p++ = space[0];
			mm *= 10;
		}
		//put minute digits
		p = itoa(m, p);
		*p++ = col[0];
		//put second text directly
		q = (u16 *)(text)[s];
		while (*q) *p++ = *q++;
		*p++ = 0;

		// repaint the background
		LCD_SetWindow(20, 240, 200, 60);
		LCD_PutImage_RGB565(bg, 200*60);
		// GET MY TIME!
		Painter_PutString((const u16*)time, 48, 0xffff, 0xf5b9
						 , 20, 240, 200, 60, shadow_on  | PAINTER_STR_SFLUSH);

		put_fps(1);
		// PUSH A TO DASH!
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15)) DUMMY;
		// TICK TOCK~
		s++;
		if (s == 12) {
			m++;
			s = 0;
		}
	}
	return 0;
}










/**
 * @brief  Inserts a delay time.
 * @param  nCount: specifies the delay time length.
 * @retval None
 */
void Delay(__IO uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	// Flash Hall LEDs and send the message
	/* Infinite loop, flashing LEDs */
	while (1)
	{
	}
}

#endif
