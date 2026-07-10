/**
 * @file lcd_dma.c
 * @brief LCD DMA single-framebuffer configuration.
 */

#include "lcd_dma.h"
#include "soc_C6748.h"
#include "raster.h"
#include "dspcache.h"
#include "delay.h"
#include <string.h>

#pragma DATA_SECTION(Lcd_Buffer, "offscreen_buffer")
#pragma DATA_ALIGN(Lcd_Buffer, 4)
unsigned char Lcd_Buffer[LCD_BUFFER_BYTES];

#pragma DATA_SECTION(Lcd_BackBuffer, "offscreen_buffer")
#pragma DATA_ALIGN(Lcd_BackBuffer, 4)
static unsigned char Lcd_BackBuffer[LCD_BUFFER_BYTES];

static unsigned char *s_lcd_front_buffer = Lcd_Buffer;
static unsigned char *s_lcd_draw_buffer = Lcd_BackBuffer;

static unsigned short palette_32b[PALETTE_SIZE / 2] = {
    0x4000u, 0x0000u, 0x0000u, 0x0000u,
    0x0000u, 0x0000u, 0x0000u, 0x0000u,
    0x0000u, 0x0000u, 0x0000u, 0x0000u,
    0x0000u, 0x0000u, 0x0000u, 0x0000u
};

static void Lcd_CopyPalette(unsigned char *buffer)
{
    unsigned int i;
    unsigned char *dest = buffer + PALETTE_OFFSET;
    unsigned char *src = (unsigned char *)palette_32b;

    for (i = 0; i < PALETTE_SIZE; i++) {
        *dest++ = *src++;
    }
}

static void Lcd_ClearFrameBuffer(void)
{
    memset(Lcd_Buffer, 0, LCD_BUFFER_BYTES);
    memset(Lcd_BackBuffer, 0, LCD_BUFFER_BYTES);
    Lcd_CopyPalette(Lcd_Buffer);
    Lcd_CopyPalette(Lcd_BackBuffer);
    CacheWB((unsigned int)Lcd_Buffer, LCD_BUFFER_BYTES);
    CacheWB((unsigned int)Lcd_BackBuffer, LCD_BUFFER_BYTES);
}

unsigned char Lcd_WaitForEndOfFrame(void)
{
    unsigned int status;
    const unsigned int eof_mask = RASTER_END_OF_FRAME0_INT_STAT |
                                  RASTER_END_OF_FRAME1_INT_STAT;

    status = RasterIntStatus(SOC_LCDC_0_REGS, eof_mask);
    if (status != 0u) {
        RasterClearGetIntStatus(SOC_LCDC_0_REGS, status & eof_mask);
    }

    SysStartTimer(25u);
    while (!SysIsTimerElapsed()) {
        status = RasterIntStatus(SOC_LCDC_0_REGS, eof_mask);
        if (status != 0u) {
            RasterClearGetIntStatus(SOC_LCDC_0_REGS, status & eof_mask);
            SysStopTimer();
            return 1u;
        }
    }

    SysStopTimer();
    return 0u;
}

unsigned int Lcd_GetRasterFaultStatus(void)
{
    unsigned int raw_status;
    unsigned int fault_status = 0u;
    const unsigned int fault_mask = RASTER_SYNC_LOST_INT_STAT |
                                    RASTER_FIFO_UNDERFLOW_INT_STAT;

    raw_status = RasterIntStatus(SOC_LCDC_0_REGS, fault_mask);
    if ((raw_status & RASTER_SYNC_LOST_INT_STAT) != 0u) {
        fault_status |= LCD_RASTER_FAULT_SYNC_LOST;
    }
    if ((raw_status & RASTER_FIFO_UNDERFLOW_INT_STAT) != 0u) {
        fault_status |= LCD_RASTER_FAULT_FIFO_UNDERFLOW;
    }

    if ((raw_status & fault_mask) != 0u) {
        RasterClearGetIntStatus(SOC_LCDC_0_REGS, raw_status & fault_mask);
    }

    return fault_status;
}

unsigned char *Lcd_GetDrawBuffer(void)
{
    return s_lcd_draw_buffer;
}

void Lcd_RecoverRasterDma(void)
{
    RasterDisable(SOC_LCDC_0_REGS);

    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_SINGLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8,
                    RASTER_BIG_ENDIAN_DISABLE);

    RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);
    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(s_lcd_front_buffer + PALETTE_OFFSET),
                      (unsigned int)(s_lcd_front_buffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_0);
    (void)Lcd_GetRasterFaultStatus();
    RasterEnable(SOC_LCDC_0_REGS);
}

unsigned char Lcd_SwapFrameBuffers(void)
{
    unsigned char *old_front;

    if (!Lcd_WaitForEndOfFrame()) {
        Lcd_RecoverRasterDma();
        return 0u;
    }

    old_front = s_lcd_front_buffer;
    s_lcd_front_buffer = s_lcd_draw_buffer;
    s_lcd_draw_buffer = old_front;

    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(s_lcd_front_buffer + PALETTE_OFFSET),
                      (unsigned int)(s_lcd_front_buffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_0);
    (void)Lcd_GetRasterFaultStatus();

    return 1u;
}

void Lcd_DMA_Init(void)
{
    Lcd_ClearFrameBuffer();
    s_lcd_front_buffer = Lcd_Buffer;
    s_lcd_draw_buffer = Lcd_BackBuffer;

    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_SINGLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8,
                    RASTER_BIG_ENDIAN_DISABLE);

    RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);
    (void)Lcd_GetRasterFaultStatus();
    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(s_lcd_front_buffer + PALETTE_OFFSET),
                      (unsigned int)(s_lcd_front_buffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_0);
}
