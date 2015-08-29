/*
*
*  Under GPL License
*
*/

#include "lcd_driver.h"

#include "stm32f10x_fsmc.h"

static void delay(__IO u32 nCount){
	for(; nCount != 0; nCount--);
}

static void delay_ms(__IO u32 nCount){
	for(; nCount != 0; nCount--)
		delay(SystemCoreClock/1000/6); // how it works~
}


static GraphicContext _d_g_ctx = {
	.direction = 0,
	.invert = 0
};

/**
 * @brief  Setup direction and inversion of display
 * applies to LCD driver and context
 * @param  direction: 0:normal, 1:right, 2: up-side-down, 3:left
 * @param  inv: 0:none, 1:invert
 */
void LCD_SetEntryMode(u8 direction, u8 inv){
	u16 reg = 0x1200;
	u8 LUT[] = {0b110, 0b011, 0b000, 0b101, 0b100, 0b111, 0b010, 0b111};
	direction &=3;
	inv = (inv>0)&1;
	_d_g_ctx.direction = direction;
	_d_g_ctx.invert = inv;
	LCD_WR_CMD(0x0003, reg | (LUT[direction << inv] << 3));
}

/**
 * @brief  Start pixel action from this point.
 * be aware of the window opened and the context should be NORMAL
 * use LCD_SetPoint_InCtx if the screen is rotated.
 */
void LCD_SetPoint(u16 x, u16 y){
	LCD_WR_CMD(0x20, x);
	LCD_WR_CMD(0x21, y);
	LCD_WR_REG(0x22);
}

/**
 * @brief  Start pixel action from this point.
 * be aware of the window opened.
 */
void LCD_SetPoint_InCtx(u16 x, u16 y){
	switch (_d_g_ctx.direction){
	case 1:
		LCD_WR_CMD(0x20, y);
		LCD_WR_CMD(0x21, LCD_SCR_HEI1 - x);
		break;
	case 2:
		LCD_WR_CMD(0x20, LCD_SCR_WID1 - x);
		LCD_WR_CMD(0x21, LCD_SCR_HEI1 - y);
		break;
	case 3:
		LCD_WR_CMD(0x20, LCD_SCR_WID1 - y);
		LCD_WR_CMD(0x21, x);
		break;
	default:
		LCD_WR_CMD(0x20, x);
		LCD_WR_CMD(0x21, y);
	}
	LCD_WR_REG(0x22);
}

/**
 * @brief  OpenWindow action in 8080 interface
 * see 8.2.24. Horizontal and Vertical RAM Address Position (R50h, R51h, R52h, R53h)
 * in ILI9325 datasheet page 73.
 * Context direction is read to ensure wanted display.
 */
void LCD_SetWindow(u16 left, u16 top, u16 width, u16 height){
	u16 _left, _top, _width, _height, _x, _y;

	switch (_d_g_ctx.direction){
	case 1:
		_left = top;
		_top = LCD_SCR_HEI - left - width;
		_width = height;_height = width;
		_x = top; _y = LCD_SCR_HEI1 - left;
		break;
	case 2:
		_left = LCD_SCR_WID - left - width;
		_top = LCD_SCR_HEI - top - height;
		_width = width;	_height = height;
		_x = LCD_SCR_WID1 - left; _y = LCD_SCR_HEI1 - top;
		break;
	case 3:
		_left = LCD_SCR_WID - top - height;
		_top = left;
		_width = height;_height = width;
		_x = LCD_SCR_WID1 - top; _y = left;
		break;
	default:
		_left = left; _top = top; _width = width; _height = height;
		_x = left; _y = top;
	}

	LCD_WR_CMD(0x50, _left);
	LCD_WR_CMD(0x51, _left+_width-1);
	LCD_WR_CMD(0x52, _top);
	LCD_WR_CMD(0x53, _top+_height-1);
	LCD_SetPoint(_x, _y);
}


//static u32 mix_x155(u32 a, u32 b, u8 u){ // u= u+ 0.5 max=15.5/16
//	return (a*u + b*(16-u) + ((a>>1)&0x70707) - ((b>>1)&0x70707));
//}

static u32 mix_x16(u32 a, u32 b, u8 u){ // max=16
	return a*u + b*(16-u);
}

static u32 mix_x32(u32 a, u32 b, u8 u){ // max=32
	return a*u + b*(32-u);
}


inline u16 LCD_GetPixel(){ // FIXME:HAS READBACK ERROR PROBLEM!!!
//u16 LCD_GetPixel(){ // FIXME:HAS READBACK ERROR PROBLEM!!!
	u16 c565;
	c565 = LCD_RD_DAT2();
	LCD_WR_DAT(c565);
	return c565;
}

inline void LCD_PutPixel(u16 c565){
//void LCD_PutPixel(u16 c565){
	LCD_WR_DAT(c565);
}

/**
 * @brief  Mix a foreground color to screen with an alpha value
 * @param  fc888: foreground color in 32bit=0:8,R:8,G:8,B:8 format
 * @param  a4: 4-bit+ alpha range from 0 to 16
 */
void LCD_MixPixel_x16(const u32 fc888, const u8 a4){
	u16 c565;
	u32 c888;
	c565 = LCD_RD_DAT2();
	c888 = C_RGB565to888h4(c565);
	c888 = mix_x16(fc888, c888, a4);
	c565 = C_RGB888to565(c888);
	LCD_WR_DAT(c565);
}

/**
 * @brief  Mix a foreground color to screen with an alpha value
 * @param  fcaba: foreground color in 32bit=0:1,R:10,G:11,B:10 format
 * @param  a5: 5-bit+ alpha range from 0 to 32
 */
void LCD_MixPixel_x32(const u32 fcaba, const u8 a5){
	u16 c565;
	u32 caba;
	c565 = LCD_RD_DAT2();
	caba = C_RGB565toABAh5(c565);
	caba = mix_x32(fcaba, caba, a5);
	c565 = C_RGBABAto565(caba);
	LCD_WR_DAT(c565);
}

// Better and faster alpha conversion lookup table
//const u8 LUT15to32[16]={
//	0,  2,  4,  6,  9, 11, 13, 15, 17, 19, 21, 23, 26, 28, 30, 32
//};
//#define C_A15to32(x) (LUT15to32[x])
//const u8 LUT225to32[226]={
//	0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,
//	2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  5,  5,
//	5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,
//	7,  7,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9, 10,
//   10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12,
//   12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14,
//   15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17,
//   17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19,
//   19, 19, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 22,
//   22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24,
//   24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26,
//   27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29,
//   29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31,
//   31, 32, 32, 32, 32
//};
//#define C_A225to32(x) (LUT225to32[x])
//const u8 LUT255to32[256]={
//	0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,
//	2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,
//	4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,
//	6,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,
//	9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 11,
//	11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13,
//	13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15,
//	15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17,
//	17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19,
//	19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21,
//	21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23,
//	23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25,
//	26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 28,
//	28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30,
//	30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32,
//	32
//};
//#define C_A255to32(x) (LUT255to32[x])

/**
 * Export of alpha conversion
 */
u8 LCD_ScaleAlpha_32(u8 v, u8 m){
	switch (m){
	case 15:return C_A15to32(v);
	case 225:return C_A225to32(v);
	case 255:return C_A255to32(v);
	default:return (u16)(v)*32/m; // OH SUCK~
	}
}

// You should SetWindow before this group of functions, the size is of pixels

void LCD_GetImage_RGB565(u16 *buf, u32 size){
	u16 *p = buf;
	while (size--){
		*p = LCD_RD_DAT2();
		LCD_WR_DAT(*p++);
	}
}

void LCD_PutImage_RGB565(const u16 *buf, u32 size){
	u16 *p = (u16 *)buf;
	while (size--){
		LCD_WR_DAT(*p++);
	}
}

void LCD_PutImage_RGB4444(const u16 *buf, u32 size){
	u16 *p = (u16 *)buf;
	u16 t0;
	u32 fc0;
	while (size--){
		t0 = *p++;
		fc0 = C_RGB4444toABAh5(t0);
		LCD_MixPixel_x32(fc0, C_A15to32(C_ALPHA4(t0)));
	}
}

void LCD_PutChar_RGB565(const u8* glyph, u16 size, u16 fc, u8 a8){
	u8 *p = (u8 *)glyph;
	u8 t0;
	u32 fc0;
	fc0 = C_RGB565toABAh5(fc);
	while (size--){
		t0 = *p++;
		LCD_MixPixel_x32(fc0, (u16)(a8)*(t0&0xf)/118);
		LCD_MixPixel_x32(fc0, (u16)(a8)*(t0>>4) /118);
	}
}

void LCD_PutChar_RGB4444(const u8* glyph, u16 size, u16 fc){
	u8 *p = (u8 *)glyph;
	u8 t0, fa0;
	u32 fc0;
	fa0 = C_ALPHA4(fc);
	fc0 = C_RGB4444toABAh5(fc);
	while (size--){
		t0 = *p++;
		LCD_MixPixel_x32(fc0, C_A225to32(fa0*(t0&0xf)));
		LCD_MixPixel_x32(fc0, C_A225to32(fa0*(t0>>4) ));
	}
}

/**
 * Mix stands for an overlay action on the screen with transparency
 */
void LCD_MixImage_RGB565(const u16 *buf, u32 size, u8 a8){
	u16 *p = (u16 *)buf;
	u16 t0;
	u32 fc0;
	a8 = C_A225to32(a8);
	while (size--){
		t0 = *p++;
		fc0 = C_RGB565toABAh5(t0);
		LCD_MixPixel_x32(fc0, a8);
	}
}

void LCD_MixImage_RGB4444(const u16 *buf, u32 size, u8 a8){
	u16 *p = (u16 *)buf;
	u16 t0;
	u32 fc0;
	while (size--){
		t0 = *p++;
		fc0 = C_RGB4444toABAh5(t0);
		LCD_MixPixel_x32(fc0, (u32)(a8)*C_ALPHA4(t0)/118);
	}
}

/**
 * Masking stands for an overlay action on the screen with alpha values in each pixel
 */
void LCD_MaskImage_RGB565(const u16 *buf, u32 size, const u8* a8){
	u16 *p = (u16 *)buf;
	u8 *q = (u8 *)a8;
	u16 t0;
	u32 fc0;
	while (size--){
		t0 = *p++;
		fc0 = C_RGB565toABAh5(t0);
		LCD_MixPixel_x32(fc0, C_A225to32(*q++));
	}
}

void LCD_MaskImage_RGB4444(const u16 *buf, u32 size, const u8* a8){
	u16 *p = (u16 *)buf;
	u8 *q = (u8 *)a8;
	u16 t0;
	u32 fc0;
	while (size--){
		t0 = *p++;
		fc0 = C_RGB4444toABAh5(t0);
		LCD_MixPixel_x32(fc0, C_A225to32(*q++));
	}
}

/**
 * BitMasking is the same action with W3C Canvas Clip()
 * It is compressed into bits so a full screen clipping data is possible
 * to be storaged in 10k RAM.
 * Use LCD_GetBitMask, LCD_ResetBitMask, LCD_SetBitMask to access one bit mask.
 */
void LCD_BitMaskImage_RGB565(const u16 *buf, u32 size, const bitmask mask){
	u16 *p = (u16 *)buf;
	u8 *q = (bitmask)mask, t1;
	while (size>=8){
		t1 = *q++;
		LCD_WR_DAT((t1&0x1)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x2)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x4)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x8)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x10)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x20)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x40)?(*p):LCD_RD_DAT2());p++;
		LCD_WR_DAT((t1&0x80)?(*p):LCD_RD_DAT2());p++;
		size-=8;
	}
	t1 = *q;
	while (size--){
		LCD_WR_DAT((t1&1)?(*p):LCD_RD_DAT2());
		p++;
		t1 >>= 1;
	}
}

void LCD_BitMaskImage_RGB4444(const u16 *buf, u32 size, const bitmask mask){
	u16 *p = (u16 *)buf;
	u8 *q = (bitmask)mask, t1;
	u16 t0;
	while (size>=8){
		t1 = *q++;
		if (t1 & 1){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 2){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 4){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 8){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 0x10){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 0x20){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 0x40){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		if (t1 & 0x80){t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}else LCD_GetPixel();p++;
		p++;
		size-=8;
	}
	t1 = *q;
	while (size--){
		if (t1 & 1){
			t0 = *p;
			LCD_MixPixel_x32(C_RGB4444toABAh5(t0), C_A15to32(C_ALPHA4(t0)));
		}
		else LCD_GetPixel();
		p++;
		t1 >>= 1;
	}
}


u8 LCD_GetBitMask(bitmask mask, u16 x, u16 y, u16 w){
	u32 t;
	t = y*w+x;
	y = t & 0x7;
	return (mask[t/8]>>y) & 1;
//	return (mask[t/8]&(1 << y))>0;
}

void LCD_ResetBitMask(bitmask mask, u16 x, u16 y, u16 w){
	u32 t;
	t = y*w+x;
	y = t & 0x7;
	mask[t/8] &= ~(1<<y);
}

void LCD_SetBitMask(bitmask mask, u16 x, u16 y, u16 w){
	u32 t;
	t = y*w+x;
	y = t & 0x7;
	mask[t/8] |= 1<<y;
}


/**
 * @brief  Draw the main body of a line within linewith |<-lw->|
 * range: x:sx to ex, y:sy to ey
 * two end part is required to draw the entire line.
 * PLEASE USE Painter_DrawLine INSTEAD IN TOP LEVEL.
 * @param  ctx: a draw line context
 */
void LCD_DrawLineBody(DrawLineContext ctx){
	u16 c565;
	u32 caba;
	s16 x, y, cx, sx, ex;
	s32 dl;
	for (y=ctx.ly0;y<=ctx.ly1;y++){
		cx = ctx.rx*y/ctx.ry;
		sx = max(cx-ctx.xhw,ctx.lx0);
		ex = min(cx+ctx.xhw,ctx.lx1);
		LCD_SetPoint_InCtx(ctx.sx+sx, ctx.sy+y);
		for (x=sx;x<=ex;x++){
			c565 = LCD_RD_DAT2();
			if (!LCD_GetBitMask(ctx.bm, ctx.bmx+x, ctx.bmy+y, ctx.bmw)){
				dl = (ctx.rx*y-ctx.ry*x);
				dl = dl * dl;
				dl = (dl + (ctx.ll/2))/ ctx.ll;

				if (ctx.soft>dl){
					caba = C_RGB565toABAh5(c565);
					if (dl<=ctx.hard) {
						caba = mix_x32(ctx.fc, caba, ctx.alpha32);
						LCD_SetBitMask(ctx.bm, ctx.bmx+x, ctx.bmy+y, ctx.bmw);
					}
					else caba = mix_x32(ctx.fc, caba, ctx.alpha32*(ctx.soft-dl)/ctx.grad);
					c565 = C_RGBABAto565(caba);
				}
			}
			LCD_WR_DAT(c565);
		}
	}
}

void LCD_DrawLineBody_Vertical(DrawLineContext ctx){
	u16 c565;
	u32 caba;
	s16 x;
	for (x=ctx.lx0;x<=ctx.lx1;x++){
		c565 = LCD_RD_DAT2();
		if (!LCD_GetBitMask(ctx.bm, ctx.bmx+x, ctx.bmy, ctx.bmw)){
			caba = C_RGB565toABAh5(c565);
			caba = mix_x32(ctx.fc, caba, ctx.alpha32);
			c565 = C_RGBABAto565(caba);
			LCD_SetBitMask(ctx.bm, ctx.bmx+x, ctx.bmy, ctx.bmw);
		}
		LCD_WR_DAT(c565);
	}
}

void LCD_DrawLineBody_Horizontal(DrawLineContext ctx){
	u16 c565;
	u32 caba;
	s16 y;
	for (y=ctx.ly0;y<=ctx.ly1;y++){
		c565 = LCD_RD_DAT2();
		if (!LCD_GetBitMask(ctx.bm, ctx.bmx, ctx.bmy+y, ctx.bmw)){
			caba = C_RGB565toABAh5(c565);
			caba = mix_x32(ctx.fc, caba, ctx.alpha32);
			c565 = C_RGBABAto565(caba);
			LCD_SetBitMask(ctx.bm, ctx.bmx, ctx.bmy+y, ctx.bmw);
		}
		LCD_WR_DAT(c565);
	}
}

/**
 * @brief  Draw one end of a line on the start point,
 * range: x:sx-half line width~sx-1, ex+1~ex+half line width
 * y:sy-half line width~sy-1, ey+1~ey+half line width
 * this function described a round end, to change the line end style,
 * modify the criteria after (dd0,dd1,ctx.ll):
 * <1>:(ctx.llhw1>dl) in the range of +- lw of the line pivot
 * <2>:(dd0+ctx.ll<<dd1) in the range of end round with radius lw
 * <3>:(dl<=ctx.llhw) full grayscale region
 * <3>:(dl>ctx.llhw) antialias region
 * PLEASE USE Painter_DrawLine INSTEAD IN TOP LEVEL.
 * @param  ctx: a draw line context
 */
void LCD_DrawLineEndPart(DrawLineContext ctx){
	u16 c565;
	u32 caba;
	s16 x, y;
	s32 dl;
	u32 yy0, yy1, dd0, dd1;
	for (y=ctx.ly0;y<=ctx.ly1;y++){
		yy0 = y*y;
		yy1 = (y-ctx.ry)*(y-ctx.ry);
		for (x=ctx.lx0;x<=ctx.lx1;x++){
			c565 = LCD_RD_DAT2();
			if (!LCD_GetBitMask(ctx.bm, ctx.bmx+x, ctx.bmy+y, ctx.bmw)){
//			if (1){
				dl = (ctx.rx*y-ctx.ry*x);
				dl = dl * dl;
				dl = (dl + (ctx.ll/2))/ ctx.ll;
				if (ctx.soft>dl){ // <1>
					dd0 = yy0 + x*x;
					dd1 = yy1 + (x-ctx.rx)*(x-ctx.rx);
					dl = (dd0+ctx.ll<dd1)?dd0:(dd1+ctx.ll<dd0)?dd1:dl;// <2>
					if (ctx.soft>dl){
						caba = C_RGB565toABAh5(c565);
						if (dl<=ctx.hard) {// <3>
							caba = mix_x32(ctx.fc, caba, ctx.alpha32);
							LCD_SetBitMask(ctx.bm, ctx.bmx+x, ctx.bmy+y, ctx.bmw);
						}
						else caba = mix_x32(ctx.fc, caba, ctx.alpha32*(ctx.soft-dl)/ctx.grad);
						c565 = C_RGBABAto565(caba);
					}
				}
			}
			LCD_WR_DAT(c565);
		}
	}
}


/**
 * @brief  Draw two ends of a line, open proper window for line end parts
 * PLEASE USE Painter_DrawLine INSTEAD IN TOP LEVEL.
 * @param  ctx: a draw line context
 * @param  sx:  x of start point
 * @param  sy:  y of start point
 * @param  ex:  x of end point
 * @param  ey:  y of end point
 * @param  lh:  half line width |<-lh-|-lh->|
 */
void LCD_DrawLineEnd(DrawLineContext ctx, u16 sx, u16 sy, u16 ex, u16 ey){
	u16 xMin, xMax, yMin, yMax, t, b, l, r, lhw = ctx.lhw;

	/* U: (sy<ey)||(sy==ey)&&(sx<ex)
	 * R: (sx>ex)||(sx==ex)&&(sy<ey)
	 * D: (sy>ey)||(sy==ey)&&(sx>ex)
	 * L: (sx<ex)||(sx==ex)&&(sy>ey)
	 */

	xMin = min(sx, ex);xMax = max(sx, ex);
	yMin = min(sy, ey);yMax = max(sy, ey);

	if ((sy<ey)||((sy==ey)&&(sx<ex))){
		l = (sx<ex)?xMin-lhw:max(xMin-lhw,xMax-ctx.xhw);
		r = (sx<ex)?min(xMax,xMin+ctx.xhw):xMax;
		t = yMin-lhw;
		b = yMin-1;
		LCD_SetWindow(l, t, r-l+1, lhw);
		ctx.lx0 = l-sx;ctx.lx1 = r-sx;
		ctx.ly0 = t-sy;ctx.ly1 = b-sy;
		LCD_DrawLineEndPart(ctx);
	}

	if ((sx>ex)||((sx==ex)&&(sy<ey))){
		l = xMax+1;
		r = xMax+lhw;
		t = (sy<ey)?yMin-lhw:max(yMin-lhw,yMax-ctx.yhw);
		b = (sy<ey)?min(yMax,yMin+ctx.yhw):yMax;
		LCD_SetWindow(l, t, lhw, b-t+1);
		ctx.lx0 = l-sx;ctx.lx1 = r-sx;
		ctx.ly0 = t-sy;ctx.ly1 = b-sy;
		LCD_DrawLineEndPart(ctx);
	}

	if ((sy>ey)||((sy==ey)&&(sx>ex))){
		l = (sx<ex)?xMin:max(xMin,xMax-ctx.xhw);
		r = (sx<ex)?min(xMax+lhw,xMin+ctx.xhw):xMax+lhw;
		t = yMax+1;
		b = yMax+lhw;
		LCD_SetWindow(l, t, r-l+1, lhw);
		ctx.lx0 = l-sx;ctx.lx1 = r-sx;
		ctx.ly0 = t-sy;ctx.ly1 = b-sy;
		LCD_DrawLineEndPart(ctx);
	}

	if ((sx<ex)||((sx==ex)&&(sy>ey))){
		l = xMin-lhw;
		r = xMin-1;
		t = (sy<ey)?yMin:max(yMin,yMax-ctx.yhw);
		b = (sy<ey)?min(yMax+lhw,yMin+ctx.yhw):yMax+lhw;
		LCD_SetWindow(l, t, lhw, b-t+1);
		ctx.lx0 = l-sx;ctx.lx1 = r-sx;
		ctx.ly0 = t-sy;ctx.ly1 = b-sy;
		LCD_DrawLineEndPart(ctx);
	}
}

/**
 * @brief  Draw a circle with linewith lw
 * PLEASE USE Painter_DrawCircle INSTEAD IN TOP LEVEL.
 * @param  cx:  x of center point
 * @param  cy:  y of center point
 * @param  r:   radius of circle
 * @param  fc:  foreground color
 * @param  lw:  line width |<-lw->|
 * @param  bitmask: applied bitmask
 * @param  bmx:     x of bitmask start point(center)
 * @param  bmy:     y of bitmask start point(center)
 * @param  bmw:     bitmask line width
 * @return circle-drawed bitmask
 */
bitmask LCD_DrawCircle(u16 cx, u16 cy, u16 r, u16 fc, u16 lw
					   , bitmask bm, u16 bmx, u16 bmy, u16 bmw){
	s16 x, y;
	u16 ge, gi, lhw, c565;
	u32 hard_e, hard_i, soft_e, soft_i, dd, yy;
	u32 caba, fc0 = C_RGB4444toABAh5(fc);
	u8 a = C_ALPHA4(fc), a0;
	s16 ST;

	lw=(lw>0)?lw:1;
	hard_e = (r+(lw-1)/2)*(r+(lw-1)/2); hard_i = (r>(lw-1)/2)?(r-(lw-1)/2)*(r-(lw-1)/2):0;
	soft_e = (r+(lw+1)/2)*(r+(lw+1)/2); soft_i = (r>(lw+1)/2)?(r-(lw+1)/2)*(r-(lw+1)/2):0;
	ge = soft_e - hard_e; gi = soft_i - hard_i;
	lhw = (lw+1)/2;

	LCD_SetWindow(cx - r - lhw, cy - r - lhw, (r+lhw)*2+1, (r+lhw)*2+1);
	for (y=-r-lhw;y<=r+lhw;y++){
		yy = y*y;
		ST = 0;
		for (x=-r-lhw;x<=r+lhw;x++){
			dd = x*x+yy;
			if (!LCD_GetBitMask(bm, bmx+x, bmy+y, bmw)){
				if ((soft_i<dd) && (soft_e>dd)){
					c565 = LCD_RD_DAT2();
					if (dd>=hard_i){
						if (dd<=hard_e){
							a0 = C_A15to32(a);
							LCD_SetBitMask(bm, bmx+x, bmy+y, bmw);
						}
						else a0 = a*(soft_e-dd)*2/ge;
					}
					else a0 = a*(dd-soft_i)*2/gi;
					caba = C_RGB565toABAh5(c565);
					caba = mix_x32(fc0, caba, a0);
					c565 = C_RGBABAto565(caba);
					if (ST==0) ST=1;
					LCD_WR_DAT(c565);
				}
				else if ((x<0)&&(ST==1)) {
					x = -x;
					LCD_SetPoint_InCtx(cx+x+1, cy+y);
					ST = 2;
				}
				else LCD_GetPixel();
			}
			else LCD_GetPixel();
		}
	}
	return bm;
}


/**
 * @brief  Fill a rectangle
 * @param  left: left pos
 * @param  top:  top pos
 * @param  w:    rectangle width
 * @param  h:    rectangle height
 * @param  fc:   foreground color
 */
void LCD_FillRectangle_RGB565(u16 left, u16 top, u16 w, u16 h, u16 fc){
	u32 x;
	LCD_SetWindow(left, top, w, h);
	for (x=0;x<w*h;x++){
		LCD_WR_DAT(fc);
	}
}

void LCD_FillRectangle_RGB4444(u16 left, u16 top, u16 w, u16 h, u16 fc){
	u32 x;
	u32 fc0 = C_RGB4444toABAh5(fc);
	u8 a = C_ALPHA4(fc);
	LCD_SetWindow(left, top, w, h);
	for (x=0;x<w*h;x++){
			LCD_MixPixel_x32(fc0, C_A15to32(a));
	}
}


/**
 * @brief  Fill a circle
 * @param  cx:  x of ceneter
 * @param  cy:  y of center
 * @param  r:   radius
 * @param  fc:  foreground color
 */
void LCD_FillCircle_RGB565(u16 cx, u16 cy, u16 r, u16 fc){
	s16 x, y;
	u16 g, c565;
	u32 rr, rr1, dd, yy;
	u32 caba, fc0 = C_RGB565toABAh5(fc);
	rr = r*r; rr1 = (r+1)*(r+1); g = r+r+1;
	LCD_SetWindow(cx - r, cy - r, r*2+1, r*2+1);
	for (y=-r;y<=r;y++){
		yy = y*y;
		for (x=-r;x<=r;x++){
			dd = x*x+yy;
			if (dd<=rr) {
				c565 = fc;
			}
			else {
				c565 = LCD_RD_DAT2();
				if (rr1>dd){
					caba = C_RGB565toABAh5(c565);
					caba = mix_x32(fc0, caba, (rr1-dd)*32/g);
					c565 = C_RGBABAto565(caba);
				}
			}
			LCD_WR_DAT(c565);
		}
	}
}

void LCD_FillCircle_RGB4444(u16 cx, u16 cy, u16 r, u16 fc){
	s16 x, y;
	u16 g, c565;
	u32 rr, rr1, dd, yy;
	u32 caba, fc0 = C_RGB4444toABAh5(fc);
	u8 a = C_ALPHA4(fc), a0;
	rr = r*r; rr1 = (r+1)*(r+1); g = r+r+1;
	LCD_SetWindow(cx - r, cy - r, r*2+1, r*2+1);
	for (y=-r;y<=r;y++){
		yy = y*y;
		for (x=-r;x<=r;x++){
			dd = x*x+yy;
			c565 = LCD_RD_DAT2();
			if (rr1>dd){
				a0 = (dd<=rr)?C_A15to32(a):(a*(rr1-dd)*2/g);
				caba = C_RGB565toABAh5(c565);
				caba = mix_x32(fc0, caba, a0);
				c565 = C_RGBABAto565(caba);
			}
			LCD_WR_DAT(c565);
		}
	}
}

/**
 * @brief  Floodfill on screen according to bitmask from a start point
 * @param  left: fill region left pos
 * @param  top:  fill region top pos
 * @param  w:    fill region  width
 * @param  h:    fill region  height
 * @param  sx:   x of start point
 * @param  sy:   y of start point
 * @param  mx:   x of start point in bitmask
 * @param  my:   y of start point in bitmask
 * @param  mask: the bitmask
 * @param  bmw:  bitmask line width
 * @param  fc:   foreground color
 * @param  qlen: FIFO search queue size
 * @param  qx:   FIFO search queue stores x
 * @param  qy:   FIFO search queue stores y
 * @return bitmask atfer floodfill
 */
bitmask LCD_Fill_Floodfill4_Core(u16 left, u16 top, u16 right, u16 bottom
							   , u16 sx, u16 sy, u16 mx, u16 my
							   , bitmask mask, u16 bmw, u16 fc, u16 qlen
							   , s16 *qx, s16* qy){

	u32 fc0 = C_RGB4444toABAh5(fc);
	u8 a0 = C_A15to32(C_ALPHA4(fc));

	u16 head, tail;
	s16 cx = 0, cy = 0;
	head = tail = 0;

	#define PUSH(x, y) ({if(_mod(tail+1,qlen)!=head){\
						LCD_SetBitMask(mask, mx + x, my + y, bmw);\
						qx[tail]=x;qy[tail]=y;tail++;if(tail==qlen)tail=0;\
						}})
	#define POP ({cx=qx[head];cy=qy[head];head++;if(head==qlen)head=0;})

	LCD_SetWindow(left, top, right - left + 1, bottom - top + 1);
	PUSH(cx, cy);
	while (head != tail){
		POP;
		LCD_SetPoint_InCtx(sx + cx, sy + cy);
		LCD_MixPixel_x32(fc0, a0);
		if ((sx+cx+1<=right) && (!LCD_GetBitMask(mask, mx + cx + 1, my + cy, bmw))) PUSH(cx+1, cy);
		if ((sx+cx>=left+1)  && (!LCD_GetBitMask(mask, mx + cx - 1, my + cy, bmw))) PUSH(cx-1, cy);
		if ((sx+cy+1<=bottom)&& (!LCD_GetBitMask(mask, mx + cx, my + cy + 1, bmw))) PUSH(cx, cy+1);
		if ((sx+cy>=top+1)   && (!LCD_GetBitMask(mask, mx + cx, my + cy - 1, bmw))) PUSH(cx, cy-1);
	}

	return mask;
}
bitmask LCD_Fill_Floodfill8_Core(u16 left, u16 top, u16 right, u16 bottom
								 , u16 sx, u16 sy, u16 mx, u16 my
								 , bitmask mask, u16 bmw, u16 fc, u16 qlen
								 , s16 *qx, s16* qy){

	u32 fc0 = C_RGB4444toABAh5(fc);
	u8 a0 = C_A15to32(C_ALPHA4(fc));

	u16 head, tail;
	s16 cx = 0, cy = 0;
	head = tail = 0;

	#define PUSH(x, y) ({if(_mod(tail+1,qlen)!=head){\
		LCD_SetBitMask(mask, mx + x, my + y, bmw);\
		qx[tail]=x;qy[tail]=y;tail++;if(tail==qlen)tail=0;\
	}})
	#define POP ({cx=qx[head];cy=qy[head];head++;if(head==qlen)head=0;})

	LCD_SetWindow(left, top, right - left + 1, bottom - top + 1);
	PUSH(cx, cy);
	while (head != tail){
		POP;
		LCD_SetPoint_InCtx(sx + cx, sy + cy);
		LCD_MixPixel_x32(fc0, a0);
		if ((sx+cx+1<=right) && (!LCD_GetBitMask(mask, mx + cx + 1, my + cy, bmw))) PUSH(cx+1, cy);
		if ((sx+cx>=left+1)  && (!LCD_GetBitMask(mask, mx + cx - 1, my + cy, bmw))) PUSH(cx-1, cy);
		if ((sx+cy+1<=bottom)&& (!LCD_GetBitMask(mask, mx + cx, my + cy + 1, bmw))) PUSH(cx, cy+1);
		if ((sx+cy>=top+1)   && (!LCD_GetBitMask(mask, mx + cx, my + cy - 1, bmw))) PUSH(cx, cy-1);
		if ((sx+cx+1<=right)&&(sx+cy+1<=bottom) && (!LCD_GetBitMask(mask, mx + cx + 1, my + cy + 1, bmw))) PUSH(cx+1, cy+1);
		if ((sx+cx+1<=right)&&(sx+cy>=top+1)    && (!LCD_GetBitMask(mask, mx + cx + 1, my + cy - 1, bmw))) PUSH(cx+1, cy-1);
		if ((sx+cx>=left+1)&&(sx+cy+1<=bottom)  && (!LCD_GetBitMask(mask, mx + cx - 1, my + cy + 1, bmw))) PUSH(cx-1, cy+1);
		if ((sx+cx>=left+1)&&(sx+cy>=top+1)     && (!LCD_GetBitMask(mask, mx + cx - 1, my + cy - 1, bmw))) PUSH(cx-1, cy-1);
	}

	return mask;
}


/**
 * @brief  Fill shadow for a shape according to bitmask
 * @param  left:   fill region left pos
 * @param  top:    fill region top pos
 * @param  right:  fill region right pos
 * @param  bottom: fill region bottom pos
 * @param  mask:   the bitmask
 * @param  mx:     x of start point in bitmask
 * @param  my:     y of start point in bitmask
 * @param  bmw:    bitmask line width
 * @param  sc:     shadow color
 * @param  sx:     shadow offset in x
 * @param  sy:     shadow offset in y
 * @param  step5:  shadow interpolation steps: signed 5 bits+:-32~32bit+
 *                 negative step5 for shadow elimination action(revesed opacity)
 *
 */
void LCD_Fill_BitMaskShadow(u16 left, u16 top, u16 right, u16 bottom
							, bitmask mask, u16 mx, u16 my, u16 bmw
							, u16 sc, s16 sx, s16 sy, s16 step5){

	u32 sc0 = C_RGB4444toABAh5(sc), caba;
	u8 a0 = C_ALPHA4(sc), inv;
	u16 x, y, w = right - left + 1, h = bottom - top + 1;
	u16 i, sum, c565, step5h;

	if(step5 == 0) step5 = abs(sx)+abs(sy);
	inv = step5<0;
	step5 = abs(step5);
	update_min(step5, 32);
	step5h = step5/2;

	LCD_SetWindow(left, top, w, h);
	for (y=0;y<h;y++) for (x=0;x<w;x++){
		while ((x<w)&&(LCD_GetBitMask(mask, mx + x, my + y, bmw))) x++;
		LCD_SetPoint_InCtx(left+x, top+y);
		c565 = LCD_RD_DAT2();
		sum = 0;
		for (i=1;i<=step5;i++){
			sum += LCD_GetBitMask(mask, mx + x - (sx*i+step5h)/step5, my + y - (sy*i+step5h)/step5, bmw);
		}
		if (sum>0){
			sum = a0*sum*2/step5;
//			update_min(sum, 32);
			if (inv) sum = 32 - sum;
			caba = C_RGB565toABAh5(c565);
			caba = mix_x32(sc0, caba, sum);
			c565 = C_RGBABAto565(caba);
		}
		LCD_WR_DAT(c565);
	}

}










/**
 * @brief  Init FSMC interface on STM32 for ourstm MINI
 */
void LCD_Cmd_InitFSMC(){
	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
		RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE , ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//LCD Rest
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	// FSMC-D0--D15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
		GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
		GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//FSMC NE1  LCD CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//FSMC RS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//FSMC Spec
	p.FSMC_AccessMode = FSMC_AccessMode_B;
	p.FSMC_AddressSetupTime = 0x02;
	p.FSMC_AddressHoldTime = 0x00;
	p.FSMC_DataSetupTime = 0x05;
	p.FSMC_BusTurnAroundDuration = 0x00;
	p.FSMC_CLKDivision = 0x00;
	p.FSMC_DataLatency = 0x00;

	//FSMC Spec - faster(not stable)
	p.FSMC_AddressSetupTime = 0x01;
	p.FSMC_AddressHoldTime = 0x00;
	p.FSMC_DataSetupTime = 0x02;
	p.FSMC_BusTurnAroundDuration = 0x00;
	p.FSMC_DataLatency = 0x00;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Enable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

/**
 * @brief  Init backlight control on STM32 for ourstm MINI
 * TODO: add PWM support (TIM4 remap?)
 */
void LCD_Cmd_InitBacklight(){
	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef  TIM_OCInitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_13);
}

/**
 * @brief  Generate nReset signal for driver
 * See: ILI9325AN
 */
void LCD_Cmd_NReset(){
	GPIO_SetBits(GPIOE, GPIO_Pin_1);
	delay_ms(1);
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
	delay_ms(10);
	GPIO_SetBits(GPIOE, GPIO_Pin_1);
	delay_ms(50);
}

/**
 * @brief  Init LCD
 * See: ILI9325AN
 */
void LCD_Cmd_Init(void)
{
	LCD_Cmd_NReset();
	assert_param(LCD_RD_REG(0) == 0x9325); // ILI9325 only

	//************* Start Initial Sequence **********//
	LCD_WR_CMD(0x0001, 0x0100); // set SS and SM bit
	LCD_WR_CMD(0x0002, 0x0700); // set 1 line inversion
	LCD_SetEntryMode(0, 0);
	LCD_WR_CMD(0x0004, 0x0000); // Resize register
//	LCD_WR_CMD(0x0007, 0x3033); // Screen ON, partial only
	LCD_WR_CMD(0x0007, 0x0133); // Screen ON, no partial
//	LCD_WR_CMD(0x0008, 0x0207); // set the back porch and front porch
//	LCD_WR_CMD(0x0009, 0x032f); // set non-display area refresh cycle ISC[3:0]
//	LCD_WR_CMD(0x000A, 0x0000); // FMARK function
//	LCD_WR_CMD(0x000C, 0x3001); // System interface - 16bit
//	LCD_WR_CMD(0x000D, 0x0000); // Frame marker Position
//	LCD_WR_CMD(0x000F, 0x0000); // RGB interface polarity
	//*************Power On sequence ****************//
	LCD_WR_CMD(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_WR_CMD(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
	LCD_WR_CMD(0x0012, 0x0000); // VREG1OUT voltage
	LCD_WR_CMD(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	LCD_WR_CMD(0x0007, 0x0001); // Display OFF
	delay_ms(200); // Dis-charge capacitor power voltage
	LCD_WR_CMD(0x0010, 0x1490); // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_WR_CMD(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0]
	delay_ms(50); // Delay 50ms
	LCD_WR_CMD(0x0012, 0x001C); // Internal reference voltage= Vci;
	delay_ms(50); // Delay 50ms
	LCD_WR_CMD(0x0013, 0x1A00); // Set VDV[4:0] for VCOM amplitude
	LCD_WR_CMD(0x0029, 0x0025); // Set VCM[5:0] for VCOMH
//	LCD_WR_CMD(0x002B, 0x0000); // Set Frame Rate = 30
//	LCD_WR_CMD(0x002B, 0x0009); // Set Frame Rate = 56
	LCD_WR_CMD(0x002B, 0x000e); // Set Frame Rate = 112
	delay_ms(50);// Delay 50ms
	// ----------- Adjust the Gamma Curve ----------//
	LCD_WR_CMD(0x0030, 0x0000);
	LCD_WR_CMD(0x0031, 0x0506);
	LCD_WR_CMD(0x0032, 0x0104);
	LCD_WR_CMD(0x0035, 0x0207);
	LCD_WR_CMD(0x0036, 0x000F);
	LCD_WR_CMD(0x0037, 0x0306);
	LCD_WR_CMD(0x0038, 0x0102);
	LCD_WR_CMD(0x0039, 0x0707);
	LCD_WR_CMD(0x003C, 0x0702);
	LCD_WR_CMD(0x003D, 0x1604);
	//------------------ Set GRAM area ---------------//
	LCD_WR_CMD(0x0060, 0xA700); // Gate Scan Line
	LCD_WR_CMD(0x0061, 0x0001); // NDL,VLE, REV
	//-------------- Partial Display Control ---------//
//	LCD_WR_CMD(0x006A, 0x0000); // set scrolling line
//	LCD_WR_CMD(0x0080, 0);
//	LCD_WR_CMD(0x0081, 0);
//	LCD_WR_CMD(0x0082, 100);
//	LCD_WR_CMD(0x0083, 100);
//	LCD_WR_CMD(0x0084, 0);
//	LCD_WR_CMD(0x0085, 100);
	//-------------- Panel Control -------------------//
//	LCD_WR_CMD(0x0090, 0x0010);
	LCD_WR_CMD(0x0092, 0x0600);
//	LCD_WR_CMD(0x0007, 0x3033); // Screen ON, partial only
	LCD_WR_CMD(0x0007, 0x0133); // Screen ON, no partial

}

/**
 * @brief  Enter sleep mode for LCD
 * See: ILI9325AN
 */
void LCD_Cmd_EnterSleep(void)
{
	LCD_WR_CMD(0x0007, 0x0131); // Set D1=0, D0=1
	delay_ms(10);
	LCD_WR_CMD(0x0007, 0x0130); // Set D1=0, D0=0
	delay_ms(10);
	LCD_WR_CMD(0x0007, 0x0000); // display OFF
	//************* Power OFF sequence **************//
	LCD_WR_CMD(0x0010, 0x0080); // SAP, BT[3:0], APE, AP, DSTB, SLP
	LCD_WR_CMD(0x0011, 0x0000); // DC1[2:0], DC0[2:0], VC[2:0]
	LCD_WR_CMD(0x0012, 0x0000); // VREG1OUT voltage
	LCD_WR_CMD(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	delay_ms(200); // Dis-charge capacitor power voltage
	LCD_WR_CMD(0x0010, 0x0082); // SAP, BT[3:0], APE, AP, DSTB, SLP
}

/**
 * @brief  Exit sleep mode for LCD
 * See: ILI9325AN
 */
void LCD_Cmd_ExitSleep(void)
{
	//*************Power On sequence ******************//
	LCD_WR_CMD(0x0010, 0x0080);
	LCD_WR_CMD(0x0011, 0x0000);
	LCD_WR_CMD(0x0012, 0x0000);
	LCD_WR_CMD(0x0013, 0x0000);
	LCD_WR_CMD(0x0007, 0x0001);
	delay_ms(200);
	LCD_WR_CMD(0x0010, 0x1490);
	LCD_WR_CMD(0x0011, 0x0227);
	delay_ms(50);
	LCD_WR_CMD(0x0012, 0x001C);
	delay_ms(50);
	LCD_WR_CMD(0x0013, 0x1A00);
	LCD_WR_CMD(0x0029, 0x0025);
	delay_ms(50);
	LCD_WR_CMD(0x0007, 0x0133);
}
