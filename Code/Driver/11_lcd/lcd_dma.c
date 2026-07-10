/**
 * @file lcd_dma.c
 * @brief LCD DMA double-framebuffer configuration.
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
unsigned char Lcd_BackBuffer[LCD_BUFFER_BYTES];

volatile unsigned long lcd_eof0_count = 0u;
volatile unsigned long lcd_eof1_count = 0u;
volatile unsigned long lcd_fifo_underflow_count = 0u;
volatile unsigned long lcd_sync_lost_count = 0u;
volatile unsigned long lcd_recovery_count = 0u;

static unsigned int s_lcd_consecutive_faults = 0u;

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

static void Lcd_RecordEofStatus(unsigned int status)
{
    if ((status & RASTER_END_OF_FRAME0_INT_STAT) != 0u) {
        lcd_eof0_count++;
    }
    if ((status & RASTER_END_OF_FRAME1_INT_STAT) != 0u) {
        lcd_eof1_count++;
    }
}

static unsigned char Lcd_WaitForFrameDone(unsigned int frame)
{
    unsigned int status;
    unsigned int mask;

    mask = (frame == LCD_FRAME_0) ? RASTER_END_OF_FRAME0_INT_STAT :
                                    RASTER_END_OF_FRAME1_INT_STAT;

    status = RasterIntStatus(SOC_LCDC_0_REGS, mask);
    if (status != 0u) {
        RasterClearGetIntStatus(SOC_LCDC_0_REGS, status & mask);
    }

    SysStartTimer(25u);
    while (!SysIsTimerElapsed()) {
        status = RasterIntStatus(SOC_LCDC_0_REGS, mask);
        if (status != 0u) {
            RasterClearGetIntStatus(SOC_LCDC_0_REGS, status & mask);
            Lcd_RecordEofStatus(status & mask);
            SysStopTimer();
            return 1u;
        }
    }

    SysStopTimer();
    return 0u;
}

static void Lcd_CopyRegionToBuffer(unsigned char *buffer,
                                   const unsigned char *src,
                                   unsigned int x,
                                   unsigned int y,
                                   unsigned int width,
                                   unsigned int height,
                                   unsigned int src_row_bytes)
{
    unsigned int row;
    unsigned char *dst = buffer + PALETTE_OFFSET + PALETTE_SIZE +
                         ((y * LCD_WIDTH + x) * 2u);
    const unsigned int lcd_row_bytes = LCD_WIDTH * 2u;
    const unsigned int dst_row_bytes = width * 2u;
    const unsigned int writeback_bytes =
        ((height - 1u) * LCD_WIDTH + width) * 2u;

    for (row = 0u; row < height; row++) {
        memcpy(dst + row * lcd_row_bytes,
               src + row * src_row_bytes,
               dst_row_bytes);
    }

    CacheWB((unsigned int)dst, writeback_bytes);
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
            Lcd_RecordEofStatus(status & eof_mask);
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
        lcd_sync_lost_count++;
    }
    if ((raw_status & RASTER_FIFO_UNDERFLOW_INT_STAT) != 0u) {
        fault_status |= LCD_RASTER_FAULT_FIFO_UNDERFLOW;
        lcd_fifo_underflow_count++;
    }

    if ((raw_status & fault_mask) != 0u) {
        RasterClearGetIntStatus(SOC_LCDC_0_REGS, raw_status & fault_mask);
    }

    return fault_status;
}

unsigned char *Lcd_GetDrawBuffer(void)
{
    return Lcd_BackBuffer;
}

void Lcd_RecoverRasterDma(void)
{
    lcd_recovery_count++;

    RasterDisable(SOC_LCDC_0_REGS);

    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_256,
                    RASTER_BIG_ENDIAN_DISABLE);

    RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);
    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(Lcd_Buffer + PALETTE_OFFSET),
                      (unsigned int)(Lcd_Buffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_0);
    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(Lcd_BackBuffer + PALETTE_OFFSET),
                      (unsigned int)(Lcd_BackBuffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_1);
    (void)Lcd_GetRasterFaultStatus();
    RasterEnable(SOC_LCDC_0_REGS);
}

void Lcd_ServiceRasterHealth(void)
{
    unsigned int fault = Lcd_GetRasterFaultStatus();

    if (fault == 0u) {
        s_lcd_consecutive_faults = 0u;
        return;
    }

    s_lcd_consecutive_faults++;
    if (s_lcd_consecutive_faults >= 3u) {
        s_lcd_consecutive_faults = 0u;
        Lcd_RecoverRasterDma();
    }
}

unsigned char Lcd_SwapFrameBuffers(void)
{
    /*
     * In true double-buffer mode LCDC alternates FB0/FB1 by itself.  Runtime
     * framebuffer-address hot swapping is intentionally disabled.
     */
    return Lcd_WaitForEndOfFrame();
}

unsigned char Lcd_UpdateRegionToBothBuffers(const unsigned char *src,
                                            unsigned int x,
                                            unsigned int y,
                                            unsigned int width,
                                            unsigned int height,
                                            unsigned int src_row_bytes)
{
    if (src == 0 ||
        width == 0u || height == 0u ||
        x >= LCD_WIDTH || y >= LCD_HEIGHT ||
        width > (LCD_WIDTH - x) ||
        height > (LCD_HEIGHT - y) ||
        src_row_bytes < width * 2u) {
        return 0u;
    }

    /*
     * EOF0 means FB0 has just finished scanning and LCDC is moving to FB1.
     * Update FB0 there; then wait for EOF1 and update FB1.  After this returns,
     * both framebuffers contain the same complete text image.
     */
    if (!Lcd_WaitForFrameDone(LCD_FRAME_0)) {
        return 0u;
    }
    Lcd_CopyRegionToBuffer(Lcd_Buffer, src, x, y, width, height, src_row_bytes);

    if (!Lcd_WaitForFrameDone(LCD_FRAME_1)) {
        return 0u;
    }
    Lcd_CopyRegionToBuffer(Lcd_BackBuffer, src, x, y, width, height, src_row_bytes);

    return 1u;
}

void Lcd_DMA_Init(void)
{
    Lcd_ClearFrameBuffer();
    s_lcd_consecutive_faults = 0u;

    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_256,
                    RASTER_BIG_ENDIAN_DISABLE);

    RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);
    (void)Lcd_GetRasterFaultStatus();
    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(Lcd_Buffer + PALETTE_OFFSET),
                      (unsigned int)(Lcd_Buffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_0);
    RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)(Lcd_BackBuffer + PALETTE_OFFSET),
                      (unsigned int)(Lcd_BackBuffer + LCD_BUFFER_BYTES - 2),
                      LCD_FRAME_1);
}
