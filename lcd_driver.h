/*
*
*  Under GPL License
*
*/

#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include "stm32f10x.h"

#define min(a,b)			(((a)>(b))?(b):(a))
#define max(a,b)			(((a)>(b))?(a):(b))
#define abs(a)				(((a)<0)?(-(a)):(a))
#define update_max(a, b)	{if((b)> (a))a=(b);}
#define update_min(a, b)	{if((b)< (a))a=(b);}
#define smod(a, b)			((a)>=(b)?(a)-(b):(a))
#define _mod(a, b)			smod(a,b)
//#define c_prod2(a,b)		(a[0]-b[0])
//#define _mod(a, b)			((a)%(b))
#define _UNUSED(x) {(void)x;}

#define Bank1_LCD_C	((u32)0x60000000) // addr
#define Bank1_LCD_D	((u32)0x60020000) // data

#define LCD_WR_REG(c)	{*(__IO uint16_t *)(Bank1_LCD_C)=c;}
#define LCD_WR_DAT(d)	{*(__IO uint16_t *)(Bank1_LCD_D)=d;}
#define LCD_RD_DAT()	(*(__IO uint16_t *)(Bank1_LCD_D))
#define LCD_RD_DAT2()	({LCD_RD_DAT(); LCD_RD_DAT();})
#define LCD_WR_CMD(c,d)	{LCD_WR_REG(c);LCD_WR_DAT(d);}
#define LCD_RD_REG(c)	({LCD_WR_REG(c); LCD_RD_DAT();})

#define LCD_WR_DAT2(d)	{LCD_WR_DAT(d);LCD_WR_DAT(d);}
#define LCD_WR_DAT4(d)	{LCD_WR_DAT2(d);LCD_WR_DAT2(d);}
#define LCD_WR_DAT8(d)	{LCD_WR_DAT4(d);LCD_WR_DAT4(d);}
#define LCD_WR_DAT16(d)	{LCD_WR_DAT8(d);LCD_WR_DAT8(d);}

#define C_ALPHA4(c)	((c) & 0xf)
#define C_ALPHA5(c)	((c) & 0x1f)
//#define C_ALPHA8(c)	(c & 0xff)
#define C_ALPHA8(c)	((u8)(c))

#define C_A15to32(x)  (((x)*15+3)/7)
#define C_A225to32(x) (((x)+3)/7)
#define C_A255to32(x) (((x)/2+2)/4)

#define C_RGB565(r, g, b)		((uint16_t)((((r)<<8)&0xf800)|(((g)<<3)&0x07e0)|((b)>>3)))
#define C_RGBA4444(r, g, b, a)	((uint16_t)(\
	(((r)<<8)&0xf000)|(((g)<<4)&0xf00)|((b)&0xf0)|(((a)>>4)&0xf)\
	))

#define C_RGB565to888h4(d)	({u32 _t=(d);\
	_t = ((_t << 4)&0xf0000)|((_t << 1)&0xf00)|((_t >> 1)&0xf); _t;\
	})
#define C_RGB565toABAh5(d)	({u32 _t=(d);\
	_t = ((_t << 10)&0x3e00000)|((_t << 5)&0xfc00)|(_t &0x1f); _t;\
	})
#define C_RGB4444to888h4(d)	({u32 _t=(d);\
	_t = ((_t << 4)&0xf0000)|(_t&0xf00)|((_t >> 4)&0xf); _t;\
	})
#define C_RGB4444toABAh5(d)	({u32 _t=(d);\
	_t = ((_t << 10)&0x3c00000)|((_t << 4)&0xf000)|((_t >> 3)&0x1e); _t;\
	})
#define C_RGB4444toABAh6(d)	({u32 _t=(d);\
	_t = ((_t << 9)&0x1e00000)|((_t << 3)&0x7800)|((_t >> 4)&0xf); _t;\
	})
#define C_RGB888to565(d)	((uint16_t)((((d)>>8)&0xf800)|(((d)>>5)&0x7e0)|(((d)>>3)&0x1f)))
#define C_RGBABAto565(d)	((uint16_t)((((d)>>15)&0xf800)|(((d)>>10)&0x7e0)|(((d)>>5)&0x1f)))

typedef u8* bitmask;

typedef struct {
	u16 sx, sy;					// x,y of startpoint
	s16 rx, ry;					// relative x,y from endpoint to startpoint
	s16 lx0, lx1, ly0, ly1;		// relative x,y range in loop
	u16 grad;					// gradient from llhw to llhw1
	u16 lw, lhw;				// linewidth, half linewidth
	u16 xhw, yhw;				// half x width, half y width
	u32 ll;						// ll=sqr(line length),
	u32 hard, soft;				// hard and soft criteria
	u32 fc;						// foreground color in RGBABAh
	u8	alpha32;				// foreground alpha in 5bit+
	u16 bmw, bmx, bmy;			// bitmark width, bitmark offset
	bitmask bm;
} DrawLineContext;

typedef DrawLineContext* DrawLineContextHandle;


typedef struct {
	u8 direction;				// Screen direction: 0~3
	u8 invert;					// Screen inversion: 0,1
	s32 a, b, c, d, e, f;		// Transform matrix, s16+16 fix point
} GraphicContext;

typedef GraphicContext* GraphicContextHandle;

#define LCD_SCR_HEI		320
#define LCD_SCR_WID		240
#define LCD_SCR_HEI1	(LCD_SCR_HEI-1)
#define LCD_SCR_WID1	(LCD_SCR_WID-1)

extern void LCD_Cmd_InitFSMC();
extern void LCD_Cmd_InitBacklight();
extern void LCD_Cmd_NReset();
extern void LCD_Cmd_Init();
extern void LCD_Cmd_EnterSleep();
extern void LCD_Cmd_ExitSleep();

extern void LCD_SetEntryMode(u8 direction, u8 inv);
extern void LCD_SetPoint(u16 x, u16 y);
extern void LCD_SetPoint_InCtx(u16 x, u16 y);
extern void LCD_SetWindow(u16 left, u16 top, u16 width, u16 height);

extern u8 LCD_ScaleAlpha_32(u8 v, u8 m);

extern u16 LCD_GetPixel();
extern void LCD_PutPixel(u16 c565);
extern void LCD_MixPixel_x16(u32 fc888, u8 a4);
extern void LCD_MixPixel_x32(u32 fcaba, u8 a5);

extern void LCD_GetImage_RGB565(u16 *buf, u32 size);
extern void LCD_PutImage_RGB565(const u16 *buf, u32 size);
extern void LCD_PutImage_RGB4444(const u16 *buf, u32 size);
extern void LCD_PutChar_RGB565(const u8* glyph, u16 size, u16 fc, u8 a8);
extern void LCD_PutChar_RGB4444(const u8* glyph, u16 size, u16 fc);
extern void LCD_MixImage_RGB565(const u16 *buf, u32 size, u8 a8);
extern void LCD_MixImage_RGB4444(const u16 *buf, u32 size, u8 a8);

extern u8 LCD_GetBitMask(bitmask mask, u16 x, u16 y, u16 w);
extern void LCD_ResetBitMask(bitmask mask, u16 x, u16 y, u16 w);
extern void LCD_SetBitMask(bitmask mask, u16 x, u16 y, u16 w);

extern void LCD_MaskImage_RGB565(const u16 *buf, u32 size, const u8* a4);
extern void LCD_MaskImage_RGB4444(const u16 *buf, u32 size, const u8* a4);
extern void LCD_BitMaskImage_RGB565(const u16 *buf, u32 size, const bitmask mask);
extern void LCD_BitMaskImage_RGB4444(const u16 *buf, u32 size, const bitmask mask);

extern void LCD_DrawLineBody(DrawLineContext ctx);
extern void LCD_DrawLineBody_Vertical(DrawLineContext ctx);
extern void LCD_DrawLineBody_Horizontal(DrawLineContext ctx);
//extern void LCD_DrawLineEndPart(DrawLineContext ctx);
extern void LCD_DrawLineEnd(DrawLineContext ctx, u16 sx, u16 sy, u16 ex, u16 ey);
extern bitmask LCD_DrawCircle(u16 cx, u16 cy, u16 r, u16 fc, u16 lw
					   , bitmask bm, u16 bmx, u16 bmy, u16 bmw);

extern void LCD_FillRectangle_RGB565(u16 left, u16 top, u16 w, u16 h, u16 fc);
extern void LCD_FillRectangle_RGB4444(u16 left, u16 top, u16 w, u16 h, u16 fc);
extern void LCD_FillCircle_RGB565(u16 cx, u16 cy, u16 r, u16 fc);
extern void LCD_FillCircle_RGB4444(u16 cx, u16 cy, u16 r, u16 fc);

extern bitmask LCD_Fill_Floodfill4_Core(u16 left, u16 top, u16 right, u16 bottom
										, u16 sx, u16 sy, u16 mx, u16 my
										, bitmask mask, u16 bmw, u16 fc, u16 qlen
										, s16 *qx, s16* qy);
extern bitmask LCD_Fill_Floodfill8_Core(u16 left, u16 top, u16 right, u16 bottom
										, u16 sx, u16 sy, u16 mx, u16 my
										, bitmask mask, u16 bmw, u16 fc, u16 qlen
										, s16 *qx, s16* qy);
extern void LCD_Fill_BitMaskShadow(u16 left, u16 top, u16 right, u16 bottom
								   , bitmask mask, u16 mx, u16 my
								   , u16 bmw, u16 sc, s16 sx, s16 sy, s16 step5);

#endif // LCD_DRIVER_H

