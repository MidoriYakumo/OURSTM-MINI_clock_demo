/*
*
*  Under GPL License
*
*/

#ifndef __PAINTER_H
#define __PAINTER_H

#include <stdint.h>

#include "lcd_driver.h"

// Screen resolution
#define PAINTER_SCR_HEI		LCD_SCR_HEI
#define PAINTER_SCR_WID		LCD_SCR_WID
#define PAINTER_SCR_HEI1	(PAINTER_SCR_HEI-1)
#define PAINTER_SCR_WID1	(PAINTER_SCR_WID-1)

/* Draw line options:
 * 	PAINTER_DRAW_BM_HOLD:		Hold bitmask in context
 * 	PAINTER_DRAW_POLY_CLOSE:	Close polygon path in polygon drawing
 */
#define PAINTER_DRAW_BM_HOLD	0x1
#define PAINTER_DRAW_POLY_CLOSE	0x2


// /* Fill options:
//  * 	PAINTER_FILL_FFLEN:			search queue length start bits
//  * 								range:0~15
//  * 	PAINTER_FILL_CONNECT8:		use 8-connect instead of 4-connect in
//  * 								floodfill
//  */
// #define PAINTER_FILL_FFLEN		0x1
// #define PAINTER_FILL_FFLEN_M		0xf
#define PAINTER_FILL_CONNECT8		0x10

/* Text render options:
 * 	PAINTER_STR_SHADOW:			text shadow length start bits
 * 								range 0~7
 * 	PAINTER_STR_SFLUSH:			draw background in each char printing
 *								otherwise draw background rectangle first
 */
#define PAINTER_STR_SHADOW		0x1
#define PAINTER_STR_SHADOW_M	0x3
#define PAINTER_STR_SFLUSH		0x4

extern const u32 res_glyph_index[];
extern const u8 res_glyphs[];

extern bitmask Painter_SetupContextBitmask(u16 w, u16 h, u8 pattern);
extern void Painter_LocateContextBitmask(u16 x, u16 y);
extern void Painter_TranslateContextBitmask(s16 x, s16 y);

extern void Painter_SetTransform(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

extern void Painter_MoveTo();
extern void Painter_LineTo();
extern void Painter_DrawPixel();
extern void Painter_BlendPixel();

extern bitmask Painter_DrawLine(u16 sx, u16 sy, u16 ex, u16 ey, u16 fc, u16 lw, u8 flag);
extern bitmask Painter_DrawPoly(u16* xs, u16* ys, u16 size, u16 fc, u16 lw, u8 flag);
extern bitmask Painter_DrawCircle(u16 cx, u16 cy, u16 r, u16 fc, u16 lw, u8 flag);
extern bitmask Painter_DrawCubicCurve(u16 p0x, u16 p0y, u16 p1x, u16 p1y, u16 p2x, u16 p2y, u16 p3x, u16 p3y,
								  u16 fc, u16 lw, u8 flag);
extern bitmask Painter_DrawBezier(u16 *xs, u16 *ys, u8 *cs, u16 size, u16 fc, u16 lw, u8 flag);

extern void Painter_PutImage(const u16* data, u16 left, u16 top, u8 a8);
extern void Painter_MaskImage(const u16* data, u16 left, u16 top, const u8 *a8);
extern void Painter_BitMaskImage(const u16* data, u16 left, u16 top, const bitmask mask);
extern void Painter_PutChar(const u8* glyph, u16 left, u16 top, u16 fc);
extern void Painter_PutString(const u16* str, u8 font_size, u16 fc, u16 bc,
							 u16 left, u16 top, u16 w, u16 h, u8 flag);

extern void Painter_FillRectangle();
extern void Painter_FillCircle();
extern void Painter_Fill_Floodfill(u16 left, u16 top, u16 right, u16 bottom, u16 sx, u16 sy
								   , s16 mx, s16 my, bitmask mask, u16 bmw, u16 fc,	u8 flag);
extern void Painter_Fill_BitMaskShadow(u16 left, u16 top, u16 right, u16 bottom
									   , bitmask mask, s16 mx, s16 my, u16 bmw
									   , u16 sc, s16 sx, s16 sy, s16 step5);

#endif // __PAINTER_H
