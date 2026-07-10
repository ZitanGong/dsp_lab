/**
 * @file lcd_grlib.c
 * @brief GrLib context binding for the LCD framebuffer.
 */

#include "lcd_grlib.h"
#include "lcd_dma.h"
#include "grlib.h"

tDisplay Lcd_Display;
tContext Lcd_Context;
tRectangle Lcd_Rectangle;

void Lcd_Grlib_Init(void)
{
    GrOffScreen16BPPInit(&Lcd_Display, Lcd_Buffer, LCD_WIDTH, LCD_HEIGHT);
    GrContextInit(&Lcd_Context, &Lcd_Display);
}
