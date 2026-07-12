#ifndef _PROJECT3_H_
#define _PROJECT3_H_

#include "driver_include.h"
#include "user_include.h"
#include "weights.h"
#include "raster.h"
#include "soc_C6748.h"
#include "lcd_dma.h"
#include "lcd_grlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// ============================================================
// 固定布局宏定义 - 防偏移专用
// ============================================================
#define P3_LCD_W                800
#define P3_LCD_H                480
#define P3_TEXT_AREA_X          80
#define P3_TEXT_AREA_Y          135
#define P3_TEXT_AREA_W          640
#define P3_TEXT_AREA_H          210

// 坐标裁剪宏
#define P3_CLAMP_X(x)           (((x) < 0) ? 0 : (((x) >= P3_LCD_W) ? (P3_LCD_W - 1) : (x)))
#define P3_CLAMP_Y(y)           (((y) < 0) ? 0 : (((y) >= P3_LCD_H) ? (P3_LCD_H - 1) : (y)))

// LCD刷新控制
#define P3_LCD_BUFFER_BYTES     (4 + 32 + P3_LCD_W * P3_LCD_H * 2)
#define P3_LCD_PIXEL_OFFSET     (4 + 32)
#define P3_CACHE_LINE_BYTES     128u
#define P3_TEXT_BAND_X          P3_TEXT_AREA_X
#define P3_TEXT_BAND_Y          P3_TEXT_AREA_Y
#define P3_TEXT_BAND_W          P3_TEXT_AREA_W
#define P3_TEXT_BAND_H          P3_TEXT_AREA_H
#define P3_DYNAMIC_X            24
#define P3_DYNAMIC_Y            64
#define P3_DYNAMIC_W            (P3_TEXT_BAND_W - 48)
#define P3_DYNAMIC_H            122
#define P3_TEXT_BUFFER_BYTES    (4 + 32 + P3_TEXT_BAND_W * P3_TEXT_BAND_H * 2)
#define P3_LISTENING_TEXT       "Listening..."
#define P3_FONT_W               5
#define P3_FONT_H               7
#define P3_FONT_GAP             1
#define P3_RGB565_BLACK         0x0000u
#define P3_RGB565_WHITE         0xFFFFu
#define P3_RGB565_PANEL         0x0862u
#define P3_RGB565_PANEL_INNER   0x10C4u
#define P3_RGB565_BORDER        0x2E7Fu
#define P3_RGB565_MUTED         0x8410u
#define P3_RGB565_IDLE          0x9E7Fu
#define P3_RGB565_RESULT        0xFFFFu
#define PROJECT3_LCD_PAUSE_DURING_INFERENCE 0

#define PROJECT3_PI                         3.14159265358979323846f

/* Hardware stream: use 20 kHz ADC, then decimate 5 -> 4 to match the PC training rate of 16 kHz. */
#define PROJECT3_ADC_RATE                   ADC_20KHZ
#define PROJECT3_DAC_RATE                   DAC_20KHZ
#define PROJECT3_BLOCK_SAMPLES              ADC_SAMPLE_1024
#define PROJECT3_DAC_CHANNEL_MASK           DAC_CHANNEL_1
#define PROJECT3_HW_SAMPLE_RATE             20000
#define PROJECT3_MODEL_SAMPLE_RATE          16000
#define PROJECT3_DSP_CLOCK_HZ               456000000u

/* One command is normalized to the same 1-second waveform used by train_bcresnet.py. */
#define PROJECT3_RAW_MAX_SAMPLES            PROJECT3_HW_SAMPLE_RATE
#define PROJECT3_MODEL_SAMPLES              16000
#define PROJECT3_VAD_FRAME_LEN              400
#define PROJECT3_VAD_HOP                    200
#define PROJECT3_PREROLL_SAMPLES            (PROJECT3_HW_SAMPLE_RATE / 20)
#define PROJECT3_WIN_SIZE                   480
#define PROJECT3_HOP_SIZE                   160
#define PROJECT3_FFT_LEN                    512
#define PROJECT3_FREQ_NUM                   (PROJECT3_FFT_LEN / 2 + 1)
#define PROJECT3_MELS_NUM                   40
#define PROJECT3_MODEL_FRAMES               101
#define PROJECT3_CMD_COUNT                  12
#define PROJECT3_FEATURE_COUNT              (PROJECT3_MELS_NUM * PROJECT3_MODEL_FRAMES)

#define PROJECT3_UI_TEXT_LEN                64
#define PROJECT3_RESULT_TEXT_LEN            32
#define PROJECT3_PASS_THROUGH_ENABLE        0
#define PROJECT3_INFERENCE_CONF_THRESHOLD     0.55f
#define PROJECT3_INFERENCE_MARGIN_THRESHOLD   0.12f
#define PROJECT3_UNKNOWN_COMPETITOR_THRESHOLD 0.25f
#define PROJECT3_UNKNOWN_COMMAND_OVERRIDE     0.65f
#define PROJECT3_FEATURE_NORM_EPS             1.0e-6f
#define PROJECT3_WAVE_TARGET_RMS            0.10f
#define PROJECT3_WAVE_MAX_GAIN              16.0f
#define PROJECT3_WAVE_PEAK_LIMIT            0.98f
#define PROJECT3_WAVE_MIN_RMS               1.0e-5f
#define PROJECT3_MIN_UTTERANCE_SAMPLES      (PROJECT3_HW_SAMPLE_RATE / 5)
#define PROJECT3_MIN_ACTIVE_SPEECH_SAMPLES  (PROJECT3_HW_SAMPLE_RATE / 8)
#define PROJECT3_VAD_ABS_START_ENERGY       2.5e-6f
#define PROJECT3_POST_INFER_IGNORE_BLOCKS   20
#define PROJECT3_MESSAGE_HOLD_BLOCKS        20
#define PROJECT3_RESULT_HOLD_BLOCKS         ((PROJECT3_HW_SAMPLE_RATE * 3 / PROJECT3_BLOCK_SAMPLES) + 1)
#define PROJECT3_ERROR_HOLD_BLOCKS          50
#define PROJECT3_VAD_CALIB_FRAMES           40
#define PROJECT3_VAD_MIN_FLOOR              1.0e-8f

#define PROJECT3_CENTER_X                   400
#define PROJECT3_CENTER_Y                   215
#define PROJECT3_CENTER_W                   520
#define PROJECT3_CENTER_H                   150

#define PROJECT3_BN_EPS                     1.0e-5f
#define PROJECT3_ACT_A_MAX                  (16 * 20 * PROJECT3_MODEL_FRAMES)
#define PROJECT3_ACT_B_MAX                  (8 * 20 * PROJECT3_MODEL_FRAMES)
#define PROJECT3_ACT_C_MAX                  (8 * 20 * PROJECT3_MODEL_FRAMES)
#define PROJECT3_GUARD_WORDS                64
#define PROJECT3_GUARD_BASE                 0x5A17C0DEu
#define PROJECT3_GUARD_STEP                 0x00100193u

typedef enum {
    PROJECT3_APP_BOOT = 0,
    PROJECT3_APP_LISTENING,
    PROJECT3_APP_SPEECH,
    PROJECT3_APP_INFERENCING,
    PROJECT3_APP_RESULT,
    PROJECT3_APP_ERROR
} PROJECT3_APP_STATE;

typedef enum {
    PROJECT3_MODEL_READY = 0
} PROJECT3_MODEL_STATE;

typedef enum {
    PROJECT3_CLASS_SILENCE = 0,
    PROJECT3_CLASS_UNKNOWN = 1,
    PROJECT3_CLASS_DOWN = 2,
    PROJECT3_CLASS_GO = 3,
    PROJECT3_CLASS_LEFT = 4,
    PROJECT3_CLASS_NO = 5,
    PROJECT3_CLASS_OFF = 6,
    PROJECT3_CLASS_ON = 7,
    PROJECT3_CLASS_RIGHT = 8,
    PROJECT3_CLASS_STOP = 9,
    PROJECT3_CLASS_UP = 10,
    PROJECT3_CLASS_YES = 11
} PROJECT3_CLASS_ID;

typedef struct {
    float noise_floor;
    float smooth_energy;
    unsigned short speech_hold_frames;
    unsigned short silence_hold_frames;
    unsigned short calibrate_frames;
    unsigned char active;
} PROJECT3_VAD_STATE;

typedef struct {
    short raw[PROJECT3_RAW_MAX_SAMPLES];
    unsigned int count;
    unsigned char ready;
} PROJECT3_UTTERANCE_BUFFER;

typedef struct {
    unsigned char class_id;
    unsigned char second_class_id;
    float confidence;
    float second_confidence;
    float margin;
    unsigned char valid;
} PROJECT3_INFER_RESULT;

typedef struct {
    float logits[PROJECT3_CMD_COUNT];
    volatile unsigned int guard[PROJECT3_GUARD_WORDS];
} PROJECT3_LOGITS_STORAGE;

typedef struct {
    PROJECT3_APP_STATE app_state;
    PROJECT3_MODEL_STATE model_state;
    PROJECT3_VAD_STATE vad;
    PROJECT3_UTTERANCE_BUFFER utter;
    PROJECT3_INFER_RESULT last_result;
    char main_text[PROJECT3_RESULT_TEXT_LEN];
    char line1[PROJECT3_UI_TEXT_LEN];
    char line2[PROJECT3_UI_TEXT_LEN];
    char last_main_text[PROJECT3_RESULT_TEXT_LEN];
    char last_line1[PROJECT3_UI_TEXT_LEN];
    char last_line2[PROJECT3_UI_TEXT_LEN];
    char stable_text[PROJECT3_RESULT_TEXT_LEN];
    unsigned long recognized_count;
    unsigned long frame_counter;
    unsigned long lcd_retry_frame;
    unsigned long lcd_fault_count;
    unsigned long lcd_last_redraw_frame;
    unsigned int active_speech_samples;
    unsigned char pass_through_enable;
    unsigned char input_gate_blocks;
    unsigned char message_hold_blocks;
    unsigned char message_return_to_listening;
    unsigned char error_hold_blocks;
    unsigned char redraw_needed;
    unsigned char stable_text_valid;
} PROJECT3_CONTEXT;

// LCD防重入标志
static volatile int s_lcd_busy = 0;
static volatile int s_lcd_infer_paused = 0;
static unsigned char s_lcd_layout_ready = 0u;

static void Project3_InitContext(PROJECT3_CONTEXT *ctx);
static void Project3_InitUi(PROJECT3_CONTEXT *ctx);
static void Project3_HandleKeys(PROJECT3_CONTEXT *ctx);
static void Project3_HandleTouch(PROJECT3_CONTEXT *ctx);
static void Project3_ProcessAudioBlock(PROJECT3_CONTEXT *ctx, short *block, unsigned int block_samples);
static void Project3_UpdateUi(PROJECT3_CONTEXT *ctx, unsigned char force_redraw);
static void Project3_ModelInit(PROJECT3_CONTEXT *ctx);
static PROJECT3_INFER_RESULT Project3_RunInference(PROJECT3_CONTEXT *ctx, const PROJECT3_UTTERANCE_BUFFER *utter);
static const char *Project3_GetLabel(unsigned char class_id);
static void Project3_ResetUtterance(PROJECT3_CONTEXT *ctx);
static void Project3_ResetPreroll(void);

#define P3_IDX3(c, h, w, H, W)       ((((c) * (H)) + (h)) * (W) + (w))
#define P3_W4(o, i, kh, kw, IC, KH, KW) (((((o) * (IC)) + (i)) * (KH) + (kh)) * (KW) + (kw))
#define P3_DW(c, kh, kw, KH, KW)     (((c) * (KH) + (kh)) * (KW) + (kw))

static const char *g_project3_labels[PROJECT3_CMD_COUNT] = {
    "silence",
    "unknown",
    "down",
    "go",
    "left",
    "no",
    "off",
    "on",
    "right",
    "stop",
    "up",
    "yes"
};

#pragma DATA_ALIGN(g_project3_input_block, 8)
static short g_project3_input_block[PROJECT3_BLOCK_SAMPLES];

#pragma DATA_ALIGN(g_project3_preroll, 8)
static short g_project3_preroll[PROJECT3_PREROLL_SAMPLES];
static unsigned int g_project3_preroll_pos = 0;
static unsigned int g_project3_preroll_count = 0;

#pragma DATA_ALIGN(g_project3_output_block, 8)
static short g_project3_output_block[PROJECT3_BLOCK_SAMPLES];

#pragma DATA_ALIGN(g_project3_model_wave, 8)
static float g_project3_model_wave[PROJECT3_MODEL_SAMPLES];

#pragma DATA_ALIGN(g_project3_window, 8)
static float g_project3_window[PROJECT3_WIN_SIZE];

#pragma DATA_ALIGN(g_project3_fft_re, 8)
static float g_project3_fft_re[PROJECT3_FFT_LEN];

#pragma DATA_ALIGN(g_project3_fft_im, 8)
static float g_project3_fft_im[PROJECT3_FFT_LEN];

#pragma DATA_ALIGN(g_project3_spec, 8)
static float g_project3_spec[PROJECT3_FREQ_NUM];

#pragma DATA_ALIGN(g_project3_mel_filter, 8)
static float g_project3_mel_filter[PROJECT3_FREQ_NUM * PROJECT3_MELS_NUM];

#pragma DATA_ALIGN(g_project3_logmel, 8)
static float g_project3_logmel[PROJECT3_MELS_NUM * PROJECT3_MODEL_FRAMES];

#pragma DATA_SECTION(g_project3_act_a, "project3_l2_data")
#pragma DATA_ALIGN(g_project3_act_a, 8)
static float g_project3_act_a[PROJECT3_ACT_A_MAX];

#pragma DATA_SECTION(g_project3_act_b, "project3_l2_data")
#pragma DATA_ALIGN(g_project3_act_b, 8)
static float g_project3_act_b[PROJECT3_ACT_B_MAX];

#pragma DATA_SECTION(g_project3_act_c, "project3_l2_data")
#pragma DATA_ALIGN(g_project3_act_c, 8)
static float g_project3_act_c[PROJECT3_ACT_C_MAX];

#pragma DATA_ALIGN(g_project3_fc_in, 8)
static float g_project3_fc_in[32];

#pragma DATA_ALIGN(g_project3_logits_store, 8)
static PROJECT3_LOGITS_STORAGE g_project3_logits_store;
#define g_project3_logits (g_project3_logits_store.logits)

#pragma DATA_ALIGN(g_project3_tw_re, 8)
static float g_project3_tw_re[PROJECT3_FFT_LEN / 2];

#pragma DATA_ALIGN(g_project3_tw_im, 8)
static float g_project3_tw_im[PROJECT3_FFT_LEN / 2];

static unsigned short g_project3_mel_first_bin[PROJECT3_MELS_NUM];
static unsigned short g_project3_mel_last_bin[PROJECT3_MELS_NUM];
static unsigned char g_project3_tables_ready = 0;
static volatile unsigned char g_project3_memory_fault = 0;
volatile unsigned long g_project3_last_inference_ms = 0u;
volatile unsigned long g_project3_last_utterance_samples = 0u;
volatile unsigned long g_project3_audio_block_count = 0u;
volatile unsigned long g_project3_vad_start_count = 0u;
volatile unsigned long g_project3_vad_end_count = 0u;
volatile unsigned long g_project3_current_utterance_samples = 0u;
volatile float g_project3_last_frame_energy = 0.0f;
volatile float g_project3_vad_noise_floor = 0.0f;
volatile float g_project3_vad_smooth_energy = 0.0f;
volatile short g_project3_audio_peak = 0;
volatile unsigned char g_project3_raw_top_class = PROJECT3_CLASS_SILENCE;
volatile unsigned char g_project3_raw_second_class = PROJECT3_CLASS_SILENCE;
volatile unsigned char g_project3_result_accepted = 0u;
volatile unsigned int g_project3_raw_top_permille = 0u;
volatile unsigned int g_project3_raw_second_permille = 0u;
volatile unsigned int g_project3_raw_margin_permille = 0u;

extern unsigned char Lcd_Buffer[];

#pragma DATA_ALIGN(g_project3_text_buffer, 128)
static unsigned char g_project3_text_buffer[P3_TEXT_BUFFER_BYTES];

static void Project3_SetAppState(PROJECT3_CONTEXT *ctx, PROJECT3_APP_STATE state);
static void Project3_SetUiText(PROJECT3_CONTEXT *ctx, const char *main_text, const char *line1, const char *line2);
static const char *Project3_GetStableDisplayText(const PROJECT3_CONTEXT *ctx);
static void Project3_ClearStableDisplay(PROJECT3_CONTEXT *ctx);
static void Project3_CommitStableWord(PROJECT3_CONTEXT *ctx, const char *label);
static void Project3_RestoreStableDisplay(PROJECT3_CONTEXT *ctx);
static void Project3_InitTables(void);
static void Project3_Fft512(float *re, float *im);
static float Project3_PaddedModelSample(int idx);
static void Project3_BuildModelWave(const PROJECT3_UTTERANCE_BUFFER *utter);
static void Project3_ExtractLogMel(const PROJECT3_UTTERANCE_BUFFER *utter, float *out_logmel);
static void Project3_NormalizeLogMel(float *logmel);
static void Project3_FormatRawResult(const PROJECT3_INFER_RESULT *result, char *text, unsigned int text_len);
static float Project3_ComputeFrameEnergy(const short *frame, unsigned int len);
static unsigned char Project3_UpdateVad(PROJECT3_CONTEXT *ctx, float frame_energy);
static unsigned char Project3_ShouldEndUtterance(PROJECT3_CONTEXT *ctx);
static void Project3_AppendRawBlock(PROJECT3_CONTEXT *ctx, const short *block, unsigned int block_samples);
static void Project3_UpdatePreroll(const short *block, unsigned int block_samples);
static void Project3_AppendPreroll(PROJECT3_CONTEXT *ctx);
static void Project3_CopyInputBlock(short *dst);
static void Project3_FillDacOutput(const PROJECT3_CONTEXT *ctx, const short *src);
static void Project3_WriteOutputBlockToDac(const short *src);
static void Project3_HandleSpeechEnd(PROJECT3_CONTEXT *ctx);
static unsigned char Project3_RenderScreen(PROJECT3_CONTEXT *ctx, unsigned char force_redraw);
static void Project3_RequestTextRedraw(PROJECT3_CONTEXT *ctx);
static void Project3_LcdPauseForInference(void);
static void Project3_LcdResumeAfterInference(void);
static void Project3_ServiceMessageHold(PROJECT3_CONTEXT *ctx);
static void Project3_ServiceErrorHold(PROJECT3_CONTEXT *ctx);
static void Project3_ResetMemoryGuard(void);
static unsigned char Project3_CheckMemoryGuard(void);
static PROJECT3_INFER_RESULT Project3_MemoryFaultResult(void);

static void Project3_Conv2d(const float *in, float *out,
                            int in_c, int in_h, int in_w,
                            int out_c, int out_h, int out_w,
                            int k_h, int k_w, int s_h, int s_w,
                            int p_h, int p_w, const float *weight);
static void Project3_DepthwiseConv2d(const float *in, float *out,
                                     int channels, int in_h, int in_w,
                                     int out_h, int out_w,
                                     int k_h, int k_w, int s_h, int s_w,
                                     int p_h, int p_w, const float *weight);
static void Project3_BatchNorm(float *x, int channels, int h, int w,
                               const float *gamma, const float *beta,
                               const float *mean, const float *var,
                               unsigned char do_relu);
static void Project3_AddRelu(float *x, const float *identity, int count);
static void Project3_BCResBlock(const float *in, float *out,
                                int in_c, int in_h, int in_w,
                                int out_c, int out_h, int out_w,
                                int stride_h, int stride_w,
                                const float *conv1_w,
                                const float *bn1_w, const float *bn1_b, const float *bn1_m, const float *bn1_v,
                                const float *dw_w,
                                const float *bn2_w, const float *bn2_b, const float *bn2_m, const float *bn2_v,
                                const float *conv2_w,
                                const float *bn3_w, const float *bn3_b, const float *bn3_m, const float *bn3_v,
                                const float *shortcut_w,
                                const float *shortcut_bn_w, const float *shortcut_bn_b,
                                const float *shortcut_bn_m, const float *shortcut_bn_v);
static void Project3_BCResNetForward(const float *logmel, float *logits);
static PROJECT3_INFER_RESULT Project3_LogitsToResult(const float *logits);
static unsigned char Project3_AcceptResult(const PROJECT3_INFER_RESULT *result);
static unsigned char Project3_IsCommandClass(unsigned char class_id);

static void Project3_SetAppState(PROJECT3_CONTEXT *ctx, PROJECT3_APP_STATE state)
{
    ctx->app_state = state;
}

static void Project3_SetUiText(PROJECT3_CONTEXT *ctx, const char *main_text, const char *line1, const char *line2)
{
    unsigned char changed = 0;

    if (main_text != NULL && strncmp(ctx->main_text, main_text, PROJECT3_RESULT_TEXT_LEN) != 0) {
        strncpy(ctx->main_text, main_text, PROJECT3_RESULT_TEXT_LEN - 1);
        ctx->main_text[PROJECT3_RESULT_TEXT_LEN - 1] = '\0';
        changed = 1;
    }
    if (line1 != NULL && strncmp(ctx->line1, line1, PROJECT3_UI_TEXT_LEN) != 0) {
        strncpy(ctx->line1, line1, PROJECT3_UI_TEXT_LEN - 1);
        ctx->line1[PROJECT3_UI_TEXT_LEN - 1] = '\0';
        changed = 1;
    }
    if (line2 != NULL && strncmp(ctx->line2, line2, PROJECT3_UI_TEXT_LEN) != 0) {
        strncpy(ctx->line2, line2, PROJECT3_UI_TEXT_LEN - 1);
        ctx->line2[PROJECT3_UI_TEXT_LEN - 1] = '\0';
        changed = 1;
    }
    if (changed) {
        ctx->redraw_needed = 1;
    }
}

static void Project3_RequestTextRedraw(PROJECT3_CONTEXT *ctx)
{
    ctx->last_main_text[0] = '\0';
    ctx->redraw_needed = 1;
    ctx->lcd_retry_frame = 0u;
}

static const char *Project3_GetStableDisplayText(const PROJECT3_CONTEXT *ctx)
{
    if (ctx->stable_text_valid && ctx->stable_text[0] != '\0') {
        return ctx->stable_text;
    }
    return P3_LISTENING_TEXT;
}

static void Project3_ClearStableDisplay(PROJECT3_CONTEXT *ctx)
{
    strncpy(ctx->stable_text, P3_LISTENING_TEXT, PROJECT3_RESULT_TEXT_LEN - 1);
    ctx->stable_text[PROJECT3_RESULT_TEXT_LEN - 1] = '\0';
    ctx->stable_text_valid = 0u;
    Project3_SetAppState(ctx, PROJECT3_APP_LISTENING);
    Project3_SetUiText(ctx, P3_LISTENING_TEXT, "", "");
    Project3_RequestTextRedraw(ctx);
}

static void Project3_CommitStableWord(PROJECT3_CONTEXT *ctx, const char *label)
{
    unsigned char changed;

    if (label == NULL || label[0] == '\0') {
        return;
    }

    changed = (!ctx->stable_text_valid ||
               strncmp(ctx->stable_text, label, PROJECT3_RESULT_TEXT_LEN) != 0);
    strncpy(ctx->stable_text, label, PROJECT3_RESULT_TEXT_LEN - 1);
    ctx->stable_text[PROJECT3_RESULT_TEXT_LEN - 1] = '\0';
    ctx->stable_text_valid = 1u;
    Project3_SetAppState(ctx, PROJECT3_APP_RESULT);
    Project3_SetUiText(ctx, ctx->stable_text, "", "");

    /* Repainting an unchanged word needlessly touches both DMA buffers and is
     * visible as a flash.  A changed label is already marked dirty by
     * Project3_SetUiText(); only repair the dirty flag if the saved state was
     * invalid but the character data happened to compare equal. */
    if (changed) {
        ctx->redraw_needed = 1u;
    }
}

static void Project3_RestoreStableDisplay(PROJECT3_CONTEXT *ctx)
{
    if (ctx->stable_text_valid) {
        Project3_SetAppState(ctx, PROJECT3_APP_RESULT);
    } else {
        Project3_SetAppState(ctx, PROJECT3_APP_LISTENING);
    }
    Project3_SetUiText(ctx, Project3_GetStableDisplayText(ctx), "", "");
}

static void Project3_ResetMemoryGuard(void)
{
    unsigned int i;

    for (i = 0; i < PROJECT3_GUARD_WORDS; i++) {
        g_project3_logits_store.guard[i] = PROJECT3_GUARD_BASE ^ (PROJECT3_GUARD_STEP * (i + 1u));
    }
    g_project3_memory_fault = 0;
}

static unsigned char Project3_CheckMemoryGuard(void)
{
    unsigned int i;

    for (i = 0; i < PROJECT3_GUARD_WORDS; i++) {
        if (g_project3_logits_store.guard[i] != (PROJECT3_GUARD_BASE ^ (PROJECT3_GUARD_STEP * (i + 1u)))) {
            g_project3_memory_fault = 1;
            return 0;
        }
    }

    return 1;
}

static PROJECT3_INFER_RESULT Project3_MemoryFaultResult(void)
{
    PROJECT3_INFER_RESULT result;

    result.class_id = PROJECT3_CLASS_UNKNOWN;
    result.second_class_id = PROJECT3_CLASS_UNKNOWN;
    result.confidence = 0.0f;
    result.second_confidence = 0.0f;
    result.margin = 0.0f;
    result.valid = 0;
    return result;
}

static void Project3_CacheWriteBackAligned(unsigned int base_addr, unsigned int byte_size)
{
    unsigned int aligned_base;
    unsigned int aligned_end;

    if (byte_size == 0u) {
        return;
    }

    aligned_base = base_addr & ~(P3_CACHE_LINE_BYTES - 1u);
    aligned_end = (base_addr + byte_size + P3_CACHE_LINE_BYTES - 1u) &
                  ~(P3_CACHE_LINE_BYTES - 1u);
    CacheWB(aligned_base, aligned_end - aligned_base);
}

static void Project3_LcdPauseForInference(void)
{
#if PROJECT3_LCD_PAUSE_DURING_INFERENCE
    if (!s_lcd_infer_paused) {
        Project3_CacheWriteBackAligned((unsigned int)Lcd_Buffer, P3_LCD_BUFFER_BYTES);
        (void)Lcd_WaitForEndOfFrame();
        RasterDisable(SOC_LCDC_0_REGS);
        s_lcd_infer_paused = 1;
    }
#endif
}

static void Project3_LcdResumeAfterInference(void)
{
#if PROJECT3_LCD_PAUSE_DURING_INFERENCE
    if (s_lcd_infer_paused) {
        Project3_CacheWriteBackAligned((unsigned int)Lcd_Buffer, P3_LCD_BUFFER_BYTES);
        RasterEnable(SOC_LCDC_0_REGS);
        s_lcd_infer_paused = 0;
        (void)Lcd_GetRasterFaultStatus();
    }
#endif
}

static void Project3_DrawSafeTextToBand(const char *text, unsigned long color)
{
    tDisplay display;
    tContext context;
    tRectangle rect;
    const tFont *main_font;
    unsigned long main_color;

    GrOffScreen16BPPInit(&display, g_project3_text_buffer,
                         P3_TEXT_BAND_W, P3_TEXT_BAND_H);
    GrContextInit(&context, &display);

    rect.sXMin = 0;
    rect.sYMin = 0;
    rect.sXMax = P3_TEXT_BAND_W - 1;
    rect.sYMax = P3_TEXT_BAND_H - 1;
    GrContextForegroundSet(&context, ClrBlack);
    GrRectFill(&context, &rect);

    rect.sXMin = 8;
    rect.sYMin = 8;
    rect.sXMax = P3_TEXT_BAND_W - 9;
    rect.sYMax = P3_TEXT_BAND_H - 9;
    GrContextForegroundSet(&context, 0x081A2Au);
    GrRectFill(&context, &rect);
    GrContextForegroundSet(&context, 0x32C9D6u);
    GrRectDraw(&context, &rect);

    rect.sXMin = 14;
    rect.sYMin = 14;
    rect.sXMax = P3_TEXT_BAND_W - 15;
    rect.sYMax = P3_TEXT_BAND_H - 15;
    GrContextForegroundSet(&context, 0x17405Cu);
    GrRectDraw(&context, &rect);

    GrContextFontSet(&context, &g_sFontCm20);
    GrContextForegroundSet(&context, 0x86A9C0u);
    GrContextBackgroundSet(&context, 0x081A2Au);
    GrStringDrawCentered(&context, "VOICE COMMAND", -1,
                         P3_TEXT_BAND_W / 2, 36, 0);

    GrContextForegroundSet(&context, 0x17405Cu);
    GrLineDraw(&context, 28, 60, P3_TEXT_BAND_W - 29, 60);

    if (text == NULL || text[0] == '\0') {
        return;
    }

    main_font = &g_sFontCm48b;
    GrContextFontSet(&context, main_font);
    if (GrStringWidthGet(&context, text, -1) > P3_DYNAMIC_W - 40) {
        main_font = &g_sFontCm36b;
        GrContextFontSet(&context, main_font);
    }
    if (GrStringWidthGet(&context, text, -1) > P3_DYNAMIC_W - 40) {
        main_font = &g_sFontCm30b;
        GrContextFontSet(&context, main_font);
    }
    if (GrStringWidthGet(&context, text, -1) > P3_DYNAMIC_W - 40) {
        main_font = &g_sFontCm24b;
        GrContextFontSet(&context, main_font);
    }

    main_color = (strcmp(text, P3_LISTENING_TEXT) == 0) ?
                 0x8FD8E8u : color;
    GrContextForegroundSet(&context, main_color);
    GrContextBackgroundSet(&context, 0x081A2Au);
    GrStringDrawCentered(&context, text, -1,
                         P3_TEXT_BAND_W / 2, 128, 0);

    GrContextForegroundSet(&context,
                           (strcmp(text, P3_LISTENING_TEXT) == 0) ?
                           0x8FD8E8u : 0x32C9D6u);
    GrLineDraw(&context,
               P3_TEXT_BAND_W / 2 - 28, P3_TEXT_BAND_H - 25,
               P3_TEXT_BAND_W / 2 + 28, P3_TEXT_BAND_H - 25);
}

static unsigned char Project3_ClearAndDrawText(const char *text, unsigned long color)
{
    unsigned char *src;
    const unsigned int row_bytes = P3_TEXT_BAND_W * 2u;
    unsigned char updated;

    Project3_DrawSafeTextToBand(text, color);

    src = g_project3_text_buffer + P3_LCD_PIXEL_OFFSET;
    if (!s_lcd_layout_ready) {
        updated = Lcd_UpdateRegionToBothBuffers(src,
                                                P3_TEXT_BAND_X,
                                                P3_TEXT_BAND_Y,
                                                P3_TEXT_BAND_W,
                                                P3_TEXT_BAND_H,
                                                row_bytes);
        if (updated) {
            s_lcd_layout_ready = 1u;
        }
        return updated;
    }

    /* Keep the frame, title and separators untouched after initialization.
     * Only the center result window is copied for each recognition event. */
    src += ((P3_DYNAMIC_Y * P3_TEXT_BAND_W + P3_DYNAMIC_X) * 2u);
    return Lcd_UpdateRegionToBothBuffers(src,
                                         P3_TEXT_BAND_X + P3_DYNAMIC_X,
                                         P3_TEXT_BAND_Y + P3_DYNAMIC_Y,
                                         P3_DYNAMIC_W,
                                         P3_DYNAMIC_H,
                                         row_bytes);
}

static void Project3_InitTables(void)
{
    int i, j;
    float all_freq;
    float m_min;
    float m_max;
    float m_pts[PROJECT3_MELS_NUM + 2];
    float f_pts[PROJECT3_MELS_NUM + 2];
    float f_diff[PROJECT3_MELS_NUM + 1];

    if (g_project3_tables_ready) {
        return;
    }

    for (i = 0; i < PROJECT3_WIN_SIZE; i++) {
        g_project3_window[i] = 0.5f * (1.0f - cosf(2.0f * PROJECT3_PI * (float)i / (float)PROJECT3_WIN_SIZE));
    }

    for (i = 0; i < PROJECT3_FFT_LEN / 2; i++) {
        float angle = 2.0f * PROJECT3_PI * (float)i / (float)PROJECT3_FFT_LEN;
        g_project3_tw_re[i] = cosf(angle);
        g_project3_tw_im[i] = -sinf(angle);
    }

    m_min = 2595.0f * log10f(1.0f);
    m_max = 2595.0f * log10f(1.0f + ((float)PROJECT3_MODEL_SAMPLE_RATE / 2.0f) / 700.0f);

    for (i = 0; i < PROJECT3_MELS_NUM + 2; i++) {
        m_pts[i] = m_min + (m_max - m_min) * (float)i / (float)(PROJECT3_MELS_NUM + 1);
        f_pts[i] = 700.0f * (powf(10.0f, m_pts[i] / 2595.0f) - 1.0f);
    }

    for (i = 0; i < PROJECT3_MELS_NUM + 1; i++) {
        f_diff[i] = f_pts[i + 1] - f_pts[i];
    }

    for (j = 0; j < PROJECT3_MELS_NUM; j++) {
        g_project3_mel_first_bin[j] = PROJECT3_FREQ_NUM;
        g_project3_mel_last_bin[j] = 0;
    }

    for (i = 0; i < PROJECT3_FREQ_NUM; i++) {
        all_freq = ((float)i) * ((float)PROJECT3_MODEL_SAMPLE_RATE / 2.0f) / (float)(PROJECT3_FREQ_NUM - 1);
        for (j = 0; j < PROJECT3_MELS_NUM; j++) {
            float slope_j = f_pts[j] - all_freq;
            float slope_j2 = f_pts[j + 2] - all_freq;
            float left = -slope_j / f_diff[j];
            float right = slope_j2 / f_diff[j + 1];
            float val = (left < right) ? left : right;
            if (val < 0.0f) val = 0.0f;
            g_project3_mel_filter[i * PROJECT3_MELS_NUM + j] = val;
            if (val > 0.0f) {
                if (i < g_project3_mel_first_bin[j]) {
                    g_project3_mel_first_bin[j] = (unsigned short)i;
                }
                g_project3_mel_last_bin[j] = (unsigned short)i;
            }
        }
    }

    for (j = 0; j < PROJECT3_MELS_NUM; j++) {
        if (g_project3_mel_first_bin[j] >= PROJECT3_FREQ_NUM) {
            g_project3_mel_first_bin[j] = 0;
            g_project3_mel_last_bin[j] = 0;
        }
    }

    g_project3_tables_ready = 1;
}

static void Project3_Fft512(float *re, float *im)
{
    int i, j, bit, len, half, step, base;

    j = 0;
    for (i = 1; i < PROJECT3_FFT_LEN; i++) {
        bit = PROJECT3_FFT_LEN >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            float tr = re[i];
            float ti = im[i];
            re[i] = re[j];
            im[i] = im[j];
            re[j] = tr;
            im[j] = ti;
        }
    }

    for (len = 2; len <= PROJECT3_FFT_LEN; len <<= 1) {
        half = len >> 1;
        step = PROJECT3_FFT_LEN / len;
        for (base = 0; base < PROJECT3_FFT_LEN; base += len) {
            for (j = 0; j < half; j++) {
                int even = base + j;
                int odd = even + half;
                int tw = j * step;
                float wr = g_project3_tw_re[tw];
                float wi = g_project3_tw_im[tw];
                float tr = wr * re[odd] - wi * im[odd];
                float ti = wr * im[odd] + wi * re[odd];
                float ur = re[even];
                float ui = im[even];
                re[even] = ur + tr;
                im[even] = ui + ti;
                re[odd] = ur - tr;
                im[odd] = ui - ti;
            }
        }
    }

}

static float Project3_PaddedModelSample(int idx)
{
    if (idx < PROJECT3_WIN_SIZE / 2) {
        int src = PROJECT3_WIN_SIZE / 2 - idx;
        if (src >= PROJECT3_MODEL_SAMPLES) src = PROJECT3_MODEL_SAMPLES - 1;
        return g_project3_model_wave[src];
    }
    if (idx < PROJECT3_WIN_SIZE / 2 + PROJECT3_MODEL_SAMPLES) {
        return g_project3_model_wave[idx - PROJECT3_WIN_SIZE / 2];
    }
    {
        int tail = idx - (PROJECT3_WIN_SIZE / 2 + PROJECT3_MODEL_SAMPLES);
        int src = PROJECT3_MODEL_SAMPLES - 2 - tail;
        if (src < 0) src = 0;
        return g_project3_model_wave[src];
    }
}

static void Project3_BuildModelWave(const PROJECT3_UTTERANCE_BUFFER *utter)
{
    int n;
    double mean = 0.0;
    double power = 0.0;
    float rms;
    float gain;
    float peak = 0.0f;

    if (utter->count > 0u) {
        unsigned int i;
        for (i = 0; i < utter->count; i++) {
            mean += (double)utter->raw[i];
        }
        mean /= (double)utter->count;
    }

    for (n = 0; n < PROJECT3_MODEL_SAMPLES; n++) {
        int q = n * 5;
        int base = q >> 2;
        int rem = q & 3;
        float frac = (float)rem * 0.25f;
        float sample = (float)mean;

        if ((unsigned int)(base + 1) < utter->count) {
            sample = (1.0f - frac) * (float)utter->raw[base] + frac * (float)utter->raw[base + 1];
        } else if ((unsigned int)base < utter->count) {
            sample = (float)utter->raw[base];
        }
        sample = (sample - (float)mean) / 32768.0f;
        g_project3_model_wave[n] = sample;
        power += (double)sample * (double)sample;
        if (fabsf(sample) > peak) {
            peak = fabsf(sample);
        }
    }

    rms = sqrtf((float)(power / (double)PROJECT3_MODEL_SAMPLES));
    if (rms > PROJECT3_WAVE_MIN_RMS && rms < PROJECT3_WAVE_TARGET_RMS) {
        gain = PROJECT3_WAVE_TARGET_RMS / rms;
        if (gain > PROJECT3_WAVE_MAX_GAIN) {
            gain = PROJECT3_WAVE_MAX_GAIN;
        }
        if (peak > 0.0f && peak * gain > PROJECT3_WAVE_PEAK_LIMIT) {
            gain = PROJECT3_WAVE_PEAK_LIMIT / peak;
        }
        if (gain > 1.0f) {
            for (n = 0; n < PROJECT3_MODEL_SAMPLES; n++) {
                g_project3_model_wave[n] *= gain;
            }
        }
    }
}

static void Project3_ExtractLogMel(const PROJECT3_UTTERANCE_BUFFER *utter, float *out_logmel)
{
    int frame, i, m, f;

    Project3_InitTables();
    Project3_BuildModelWave(utter);

    for (frame = 0; frame < PROJECT3_MODEL_FRAMES; frame++) {
        int start = frame * PROJECT3_HOP_SIZE;
        for (i = 0; i < PROJECT3_FFT_LEN; i++) {
            g_project3_fft_re[i] = 0.0f;
            g_project3_fft_im[i] = 0.0f;
        }
        for (i = 0; i < PROJECT3_WIN_SIZE; i++) {
            g_project3_fft_re[i] = Project3_PaddedModelSample(start + i) * g_project3_window[i];
        }

        Project3_Fft512(g_project3_fft_re, g_project3_fft_im);

        for (f = 0; f < PROJECT3_FREQ_NUM; f++) {
            g_project3_spec[f] = g_project3_fft_re[f] * g_project3_fft_re[f]
                                + g_project3_fft_im[f] * g_project3_fft_im[f];
        }

        for (m = 0; m < PROJECT3_MELS_NUM; m++) {
            float mel_energy = 1.0e-6f;
            int first_bin = (int)g_project3_mel_first_bin[m];
            int last_bin = (int)g_project3_mel_last_bin[m];
            for (f = first_bin; f <= last_bin; f++) {
                mel_energy += g_project3_spec[f] * g_project3_mel_filter[f * PROJECT3_MELS_NUM + m];
            }
            out_logmel[m * PROJECT3_MODEL_FRAMES + frame] = logf(mel_energy);
        }
    }

    Project3_NormalizeLogMel(out_logmel);
}

static void Project3_NormalizeLogMel(float *logmel)
{
    int i;
    double sum = 0.0;
    double variance_sum = 0.0;
    float mean;
    float inv_std;

    for (i = 0; i < PROJECT3_FEATURE_COUNT; i++) {
        sum += (double)logmel[i];
    }
    mean = (float)(sum / (double)PROJECT3_FEATURE_COUNT);

    for (i = 0; i < PROJECT3_FEATURE_COUNT; i++) {
        double centered = (double)logmel[i] - (double)mean;
        variance_sum += centered * centered;
    }

    inv_std = 1.0f /
              (sqrtf((float)(variance_sum / (double)PROJECT3_FEATURE_COUNT)) +
               PROJECT3_FEATURE_NORM_EPS);

    for (i = 0; i < PROJECT3_FEATURE_COUNT; i++) {
        logmel[i] = (logmel[i] - mean) * inv_std;
    }
}

static float Project3_ComputeFrameEnergy(const short *frame, unsigned int len)
{
    unsigned int i;
    float mean = 0.0f;
    float energy = 0.0f;

    for (i = 0; i < len; i++) {
        mean += (float)frame[i];
    }
    mean /= (float)len;

    for (i = 0; i < len; i++) {
        float sample = ((float)frame[i] - mean) / 32768.0f;
        energy += sample * sample;
    }

    return energy / (float)len;
}

static unsigned char Project3_UpdateVad(PROJECT3_CONTEXT *ctx, float frame_energy)
{
    PROJECT3_VAD_STATE *vad = &ctx->vad;
    const float smooth_alpha = 0.85f;
    const float floor_rise_alpha = 0.95f;
    const float floor_fall_alpha = 0.92f;
    const float start_ratio = 4.5f;
    const float stop_ratio = 1.8f;
    const unsigned short start_frames = 4;
    float start_threshold;
    float stop_threshold;

    if (frame_energy < PROJECT3_VAD_MIN_FLOOR) {
        frame_energy = PROJECT3_VAD_MIN_FLOOR;
    }

    vad->smooth_energy = smooth_alpha * vad->smooth_energy + (1.0f - smooth_alpha) * frame_energy;

    if (vad->calibrate_frames < PROJECT3_VAD_CALIB_FRAMES) {
        float cal_alpha = (vad->calibrate_frames == 0u) ? 0.0f : 0.70f;
        vad->noise_floor = cal_alpha * vad->noise_floor + (1.0f - cal_alpha) * frame_energy;
        vad->smooth_energy = vad->noise_floor;
        vad->calibrate_frames++;
        vad->speech_hold_frames = 0;
        vad->silence_hold_frames = 0;
        return 0;
    }

    if (!vad->active) {
        if (vad->smooth_energy > vad->noise_floor) {
            vad->noise_floor = floor_rise_alpha * vad->noise_floor + (1.0f - floor_rise_alpha) * vad->smooth_energy;
        } else {
            vad->noise_floor = floor_fall_alpha * vad->noise_floor + (1.0f - floor_fall_alpha) * vad->smooth_energy;
        }
        if (vad->noise_floor < PROJECT3_VAD_MIN_FLOOR) {
            vad->noise_floor = PROJECT3_VAD_MIN_FLOOR;
        }
    }

    start_threshold = vad->noise_floor * start_ratio;
    if (start_threshold < PROJECT3_VAD_ABS_START_ENERGY) {
        start_threshold = PROJECT3_VAD_ABS_START_ENERGY;
    }
    stop_threshold = vad->noise_floor * stop_ratio;
    if (stop_threshold < PROJECT3_VAD_ABS_START_ENERGY * 0.45f) {
        stop_threshold = PROJECT3_VAD_ABS_START_ENERGY * 0.45f;
    }

    if (vad->smooth_energy > start_threshold) {
        if (vad->speech_hold_frames < 255) vad->speech_hold_frames++;
    } else {
        vad->speech_hold_frames = 0;
    }

    if (vad->active) {
        if (vad->smooth_energy < stop_threshold) {
            if (vad->silence_hold_frames < 255) vad->silence_hold_frames++;
        } else {
            vad->silence_hold_frames = 0;
        }
    }

    if (!vad->active && vad->speech_hold_frames >= start_frames) {
        vad->active = 1;
        vad->speech_hold_frames = 0;
        vad->silence_hold_frames = 0;
        Project3_SetAppState(ctx, PROJECT3_APP_SPEECH);
        g_project3_vad_start_count++;
        return 1;
    }

    return 0;
}

static unsigned char Project3_ShouldEndUtterance(PROJECT3_CONTEXT *ctx)
{
    const unsigned short stop_frames = 10;
    const unsigned int min_samples = PROJECT3_MIN_UTTERANCE_SAMPLES;

    if (ctx->vad.active && ctx->vad.silence_hold_frames >= stop_frames) {
        ctx->vad.active = 0;
        ctx->vad.silence_hold_frames = 0;
        if (ctx->utter.count >= min_samples) {
            ctx->utter.ready = 1;
            return 1;
        }
        Project3_ResetUtterance(ctx);
    }

    if (ctx->utter.count >= PROJECT3_RAW_MAX_SAMPLES) {
        ctx->vad.active = 0;
        ctx->utter.ready = 1;
        return 1;
    }

    return 0;
}

static void Project3_AppendRawBlock(PROJECT3_CONTEXT *ctx, const short *block, unsigned int block_samples)
{
    unsigned int i;
    for (i = 0; i < block_samples && ctx->utter.count < PROJECT3_RAW_MAX_SAMPLES; i++) {
        ctx->utter.raw[ctx->utter.count++] = block[i];
    }
}

static void Project3_ResetPreroll(void)
{
    g_project3_preroll_pos = 0;
    g_project3_preroll_count = 0;
}

static void Project3_UpdatePreroll(const short *block, unsigned int block_samples)
{
    unsigned int i;

    for (i = 0; i < block_samples; i++) {
        g_project3_preroll[g_project3_preroll_pos] = block[i];
        g_project3_preroll_pos++;
        if (g_project3_preroll_pos >= PROJECT3_PREROLL_SAMPLES) {
            g_project3_preroll_pos = 0;
        }
        if (g_project3_preroll_count < PROJECT3_PREROLL_SAMPLES) {
            g_project3_preroll_count++;
        }
    }
}

static void Project3_AppendPreroll(PROJECT3_CONTEXT *ctx)
{
    unsigned int i;
    unsigned int start;

    if (g_project3_preroll_count == 0u) {
        return;
    }

    start = (g_project3_preroll_pos + PROJECT3_PREROLL_SAMPLES - g_project3_preroll_count) %
            PROJECT3_PREROLL_SAMPLES;
    for (i = 0; i < g_project3_preroll_count && ctx->utter.count < PROJECT3_RAW_MAX_SAMPLES; i++) {
        unsigned int idx = start + i;
        if (idx >= PROJECT3_PREROLL_SAMPLES) {
            idx -= PROJECT3_PREROLL_SAMPLES;
        }
        ctx->utter.raw[ctx->utter.count++] = g_project3_preroll[idx];
    }
}

static void Project3_CopyInputBlock(short *dst)
{
    short *src = (AD_Ping_Pong == AD_BUFFER_PONG) ? AD_CH1_Buf0 : AD_CH1_Buf1;

    /* EDMA writes the ADC ping-pong buffers without maintaining the DSP
     * cache.  Invalidate before the CPU reads them or VAD can repeatedly see
     * an old/zero cache line even though fresh audio has arrived. */
    CacheInv((unsigned int)src,
             sizeof(short) * PROJECT3_BLOCK_SAMPLES);
    memcpy(dst, src, sizeof(short) * PROJECT3_BLOCK_SAMPLES);
}

static void Project3_FillDacOutput(const PROJECT3_CONTEXT *ctx, const short *src)
{
    unsigned int i;
    if (ctx->pass_through_enable) {
        memcpy(g_project3_output_block, src, sizeof(short) * PROJECT3_BLOCK_SAMPLES);
    } else {
        for (i = 0; i < PROJECT3_BLOCK_SAMPLES; i++) {
            g_project3_output_block[i] = 0;
        }
    }
}

static void Project3_WriteOutputBlockToDac(const short *src)
{
    short *dst;

    if (DA_Ping_Pong == DA_BUFFER_PONG) {
        dst = DA_CH1_Buf0;
    } else {
        dst = DA_CH1_Buf1;
    }
    memcpy(dst, src, sizeof(short) * PROJECT3_BLOCK_SAMPLES);
    CacheWB((unsigned int)dst,
            sizeof(short) * PROJECT3_BLOCK_SAMPLES);
}

static void Project3_Conv2d(const float *in, float *out,
                            int in_c, int in_h, int in_w,
                            int out_c, int out_h, int out_w,
                            int k_h, int k_w, int s_h, int s_w,
                            int p_h, int p_w, const float *weight)
{
    int oc, oh, ow, ic, kh, kw;
    for (oc = 0; oc < out_c; oc++) {
        for (oh = 0; oh < out_h; oh++) {
            for (ow = 0; ow < out_w; ow++) {
                float sum = 0.0f;
                for (ic = 0; ic < in_c; ic++) {
                    for (kh = 0; kh < k_h; kh++) {
                        int ih = oh * s_h + kh - p_h;
                        if (ih < 0 || ih >= in_h) continue;
                        for (kw = 0; kw < k_w; kw++) {
                            int iw = ow * s_w + kw - p_w;
                            if (iw < 0 || iw >= in_w) continue;
                            sum += in[P3_IDX3(ic, ih, iw, in_h, in_w)] * weight[P3_W4(oc, ic, kh, kw, in_c, k_h, k_w)];
                        }
                    }
                }
                out[P3_IDX3(oc, oh, ow, out_h, out_w)] = sum;
            }
        }
    }
}

static void Project3_DepthwiseConv2d(const float *in, float *out,
                                     int channels, int in_h, int in_w,
                                     int out_h, int out_w,
                                     int k_h, int k_w, int s_h, int s_w,
                                     int p_h, int p_w, const float *weight)
{
    int c, oh, ow, kh, kw;
    for (c = 0; c < channels; c++) {
        for (oh = 0; oh < out_h; oh++) {
            for (ow = 0; ow < out_w; ow++) {
                float sum = 0.0f;
                for (kh = 0; kh < k_h; kh++) {
                    int ih = oh * s_h + kh - p_h;
                    if (ih < 0 || ih >= in_h) continue;
                    for (kw = 0; kw < k_w; kw++) {
                        int iw = ow * s_w + kw - p_w;
                        if (iw < 0 || iw >= in_w) continue;
                        sum += in[P3_IDX3(c, ih, iw, in_h, in_w)] * weight[P3_DW(c, kh, kw, k_h, k_w)];
                    }
                }
                out[P3_IDX3(c, oh, ow, out_h, out_w)] = sum;
            }
        }
    }
}

static void Project3_BatchNorm(float *x, int channels, int h, int w,
                               const float *gamma, const float *beta,
                               const float *mean, const float *var,
                               unsigned char do_relu)
{
    int c, i;
    int hw = h * w;
    for (c = 0; c < channels; c++) {
        float scale = gamma[c] / sqrtf(var[c] + PROJECT3_BN_EPS);
        float bias = beta[c] - mean[c] * scale;
        float *ptr = &x[c * hw];
        for (i = 0; i < hw; i++) {
            float v = ptr[i] * scale + bias;
            if (do_relu && v < 0.0f) v = 0.0f;
            ptr[i] = v;
        }
    }
}

static void Project3_AddRelu(float *x, const float *identity, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        float v = x[i] + identity[i];
        x[i] = (v < 0.0f) ? 0.0f : v;
    }
}

static void Project3_BCResBlock(const float *in, float *out,
                                int in_c, int in_h, int in_w,
                                int out_c, int out_h, int out_w,
                                int stride_h, int stride_w,
                                const float *conv1_w,
                                const float *bn1_w, const float *bn1_b, const float *bn1_m, const float *bn1_v,
                                const float *dw_w,
                                const float *bn2_w, const float *bn2_b, const float *bn2_m, const float *bn2_v,
                                const float *conv2_w,
                                const float *bn3_w, const float *bn3_b, const float *bn3_m, const float *bn3_v,
                                const float *shortcut_w,
                                const float *shortcut_bn_w, const float *shortcut_bn_b,
                                const float *shortcut_bn_m, const float *shortcut_bn_v)
{
    Project3_Conv2d(in, out, in_c, in_h, in_w, out_c, in_h, in_w, 1, 1, 1, 1, 0, 0, conv1_w);
    Project3_BatchNorm(out, out_c, in_h, in_w, bn1_w, bn1_b, bn1_m, bn1_v, 1);

    Project3_DepthwiseConv2d(out, g_project3_act_c, out_c, in_h, in_w, out_h, out_w, 3, 3, stride_h, stride_w, 1, 1, dw_w);
    Project3_BatchNorm(g_project3_act_c, out_c, out_h, out_w, bn2_w, bn2_b, bn2_m, bn2_v, 1);

    Project3_Conv2d(g_project3_act_c, out, out_c, out_h, out_w, out_c, out_h, out_w, 1, 1, 1, 1, 0, 0, conv2_w);
    Project3_BatchNorm(out, out_c, out_h, out_w, bn3_w, bn3_b, bn3_m, bn3_v, 0);

    Project3_Conv2d(in, g_project3_act_c, in_c, in_h, in_w, out_c, out_h, out_w, 1, 1, stride_h, stride_w, 0, 0, shortcut_w);
    Project3_BatchNorm(g_project3_act_c, out_c, out_h, out_w, shortcut_bn_w, shortcut_bn_b, shortcut_bn_m, shortcut_bn_v, 0);

    Project3_AddRelu(out, g_project3_act_c, out_c * out_h * out_w);
}

static void Project3_BCResNetForward(const float *logmel, float *logits)
{
    int c, w, cls, i;

    Project3_Conv2d(logmel, g_project3_act_a, 1, 40, PROJECT3_MODEL_FRAMES,
                    16, 20, PROJECT3_MODEL_FRAMES, 3, 3, 2, 1, 1, 1, conv1_weight);
    Project3_BatchNorm(g_project3_act_a, 16, 20, PROJECT3_MODEL_FRAMES,
                       bn1_weight, bn1_bias, bn1_running_mean, bn1_running_var, 1);

    Project3_BCResBlock(g_project3_act_a, g_project3_act_b,
                        16, 20, PROJECT3_MODEL_FRAMES, 8, 20, PROJECT3_MODEL_FRAMES, 1, 1,
                        layer1_conv1_weight, layer1_bn1_weight, layer1_bn1_bias, layer1_bn1_running_mean, layer1_bn1_running_var,
                        layer1_dwconv_weight, layer1_bn2_weight, layer1_bn2_bias, layer1_bn2_running_mean, layer1_bn2_running_var,
                        layer1_conv2_weight, layer1_bn3_weight, layer1_bn3_bias, layer1_bn3_running_mean, layer1_bn3_running_var,
                        layer1_shortcut_0_weight, layer1_shortcut_1_weight, layer1_shortcut_1_bias,
                        layer1_shortcut_1_running_mean, layer1_shortcut_1_running_var);

    Project3_BCResBlock(g_project3_act_b, g_project3_act_a,
                        8, 20, PROJECT3_MODEL_FRAMES, 12, 10, PROJECT3_MODEL_FRAMES, 2, 1,
                        layer2_conv1_weight, layer2_bn1_weight, layer2_bn1_bias, layer2_bn1_running_mean, layer2_bn1_running_var,
                        layer2_dwconv_weight, layer2_bn2_weight, layer2_bn2_bias, layer2_bn2_running_mean, layer2_bn2_running_var,
                        layer2_conv2_weight, layer2_bn3_weight, layer2_bn3_bias, layer2_bn3_running_mean, layer2_bn3_running_var,
                        layer2_shortcut_0_weight, layer2_shortcut_1_weight, layer2_shortcut_1_bias,
                        layer2_shortcut_1_running_mean, layer2_shortcut_1_running_var);

    Project3_BCResBlock(g_project3_act_a, g_project3_act_b,
                        12, 10, PROJECT3_MODEL_FRAMES, 16, 5, PROJECT3_MODEL_FRAMES, 2, 1,
                        layer3_conv1_weight, layer3_bn1_weight, layer3_bn1_bias, layer3_bn1_running_mean, layer3_bn1_running_var,
                        layer3_dwconv_weight, layer3_bn2_weight, layer3_bn2_bias, layer3_bn2_running_mean, layer3_bn2_running_var,
                        layer3_conv2_weight, layer3_bn3_weight, layer3_bn3_bias, layer3_bn3_running_mean, layer3_bn3_running_var,
                        layer3_shortcut_0_weight, layer3_shortcut_1_weight, layer3_shortcut_1_bias,
                        layer3_shortcut_1_running_mean, layer3_shortcut_1_running_var);

    Project3_DepthwiseConv2d(g_project3_act_b, g_project3_act_a,
                             16, 5, PROJECT3_MODEL_FRAMES, 5, PROJECT3_MODEL_FRAMES,
                             3, 3, 1, 1, 1, 1, dwconv_weight);
    Project3_BatchNorm(g_project3_act_a, 16, 5, PROJECT3_MODEL_FRAMES,
                       bn_dw_weight, bn_dw_bias, bn_dw_running_mean, bn_dw_running_var, 1);

    Project3_Conv2d(g_project3_act_a, g_project3_act_b,
                    16, 5, PROJECT3_MODEL_FRAMES, 20, 5, PROJECT3_MODEL_FRAMES,
                    1, 1, 1, 1, 0, 0, pwconv_weight);
    Project3_BatchNorm(g_project3_act_b, 20, 5, PROJECT3_MODEL_FRAMES,
                       bn_pw_weight, bn_pw_bias, bn_pw_running_mean, bn_pw_running_var, 1);

    Project3_Conv2d(g_project3_act_b, g_project3_act_a,
                    20, 5, PROJECT3_MODEL_FRAMES, 20, 1, PROJECT3_MODEL_FRAMES,
                    5, 1, 1, 1, 0, 0, conv2_weight);
    Project3_BatchNorm(g_project3_act_a, 20, 1, PROJECT3_MODEL_FRAMES,
                       bn2_weight, bn2_bias, bn2_running_mean, bn2_running_var, 1);

    Project3_Conv2d(g_project3_act_a, g_project3_act_b,
                    20, 1, PROJECT3_MODEL_FRAMES, 32, 1, PROJECT3_MODEL_FRAMES,
                    1, 1, 1, 1, 0, 0, conv_expand_weight);
    Project3_BatchNorm(g_project3_act_b, 32, 1, PROJECT3_MODEL_FRAMES,
                       bn_expand_weight, bn_expand_bias, bn_expand_running_mean, bn_expand_running_var, 1);

    for (c = 0; c < 32; c++) {
        float sum = 0.0f;
        for (w = 0; w < PROJECT3_MODEL_FRAMES; w++) {
            sum += g_project3_act_b[P3_IDX3(c, 0, w, 1, PROJECT3_MODEL_FRAMES)];
        }
        g_project3_fc_in[c] = sum / (float)PROJECT3_MODEL_FRAMES;
    }

    for (cls = 0; cls < PROJECT3_CMD_COUNT; cls++) {
        float sum = fc_bias[cls];
        for (i = 0; i < 32; i++) {
            sum += g_project3_fc_in[i] * fc_weight[cls * 32 + i];
        }
        logits[cls] = sum;
    }
}

static PROJECT3_INFER_RESULT Project3_LogitsToResult(const float *logits)
{
    PROJECT3_INFER_RESULT result;
    int i;
    int best = 0;
    int second = 1;
    float max_logit = logits[0];
    float second_logit;
    float sum_exp = 0.0f;
    float best_prob;
    float second_prob;

    for (i = 1; i < PROJECT3_CMD_COUNT; i++) {
        if (logits[i] > max_logit) {
            max_logit = logits[i];
            best = i;
        }
    }

    second = (best == 0) ? 1 : 0;
    second_logit = logits[second];
    for (i = 0; i < PROJECT3_CMD_COUNT; i++) {
        if (i != best && logits[i] > second_logit) {
            second_logit = logits[i];
            second = i;
        }
    }

    for (i = 0; i < PROJECT3_CMD_COUNT; i++) {
        sum_exp += expf(logits[i] - max_logit);
    }

    best_prob = expf(logits[best] - max_logit) / sum_exp;
    second_prob = expf(logits[second] - max_logit) / sum_exp;
    result.class_id = (unsigned char)best;
    result.second_class_id = (unsigned char)second;
    result.confidence = best_prob;
    result.second_confidence = second_prob;
    result.margin = best_prob - second_prob;
    result.valid = 1;

    if (best == PROJECT3_CLASS_SILENCE) {
        result.valid = 0;
    } else if (best == PROJECT3_CLASS_UNKNOWN) {
        /* Unknown is a real model output and is useful feedback to the user. */
        result.valid = 1;
    } else if (second == PROJECT3_CLASS_UNKNOWN &&
               second_prob >= PROJECT3_UNKNOWN_COMPETITOR_THRESHOLD &&
               best_prob < PROJECT3_UNKNOWN_COMMAND_OVERRIDE) {
        /* A strong unknown runner-up means the model is unsure whether this
         * audio belongs to the command vocabulary at all. */
        result.valid = 0;
    } else if (best_prob < PROJECT3_INFERENCE_CONF_THRESHOLD) {
        result.valid = 0;
    } else if (result.margin < PROJECT3_INFERENCE_MARGIN_THRESHOLD) {
        result.valid = 0;
    }

    return result;
}

static unsigned char Project3_AcceptResult(const PROJECT3_INFER_RESULT *result)
{
    return result->valid ? 1u : 0u;
}

static unsigned char Project3_IsCommandClass(unsigned char class_id)
{
    return (class_id >= PROJECT3_CLASS_DOWN && class_id < PROJECT3_CMD_COUNT) ? 1u : 0u;
}

static const char *Project3_GetShortLabel(unsigned char class_id)
{
    static const char *short_labels[PROJECT3_CMD_COUNT] = {
        "sil",
        "unk",
        "down",
        "go",
        "left",
        "no",
        "off",
        "on",
        "right",
        "stop",
        "up",
        "yes"
    };

    if (class_id >= PROJECT3_CMD_COUNT) {
        return "unk";
    }
    return short_labels[class_id];
}

static void Project3_FormatRawResult(const PROJECT3_INFER_RESULT *result, char *text, unsigned int text_len)
{
    unsigned int p1;
    unsigned int p2;

    if (text == NULL || text_len == 0u) {
        return;
    }

    p1 = (unsigned int)(result->confidence * 100.0f + 0.5f);
    p2 = (unsigned int)(result->second_confidence * 100.0f + 0.5f);
    if (p1 > 99u) p1 = 99u;
    if (p2 > 99u) p2 = 99u;

    snprintf(text, text_len, "raw %s%u/%s%u",
             Project3_GetShortLabel(result->class_id), p1,
             Project3_GetShortLabel(result->second_class_id), p2);
    text[text_len - 1u] = '\0';
}

static void Project3_HandleSpeechEnd(PROJECT3_CONTEXT *ctx)
{
    PROJECT3_INFER_RESULT result;
    const char *label;
    unsigned char accepted;
    unsigned long long infer_start;
    unsigned long long infer_end;
    unsigned long long infer_cycles;

    Project3_SetAppState(ctx, PROJECT3_APP_INFERENCING);
    Project3_SetUiText(ctx, "Processing...", "", "");
    Project3_UpdateUi(ctx, 1u);

    g_project3_last_utterance_samples = ctx->utter.count;
    g_project3_result_accepted = 0u;
    infer_start = _itoll(TSCH, TSCL);
    result = Project3_RunInference(ctx, &ctx->utter);
    infer_end = _itoll(TSCH, TSCL);
    infer_cycles = infer_end - infer_start;
    g_project3_last_inference_ms =
        (unsigned long)(infer_cycles / (PROJECT3_DSP_CLOCK_HZ / 1000u));
    g_project3_raw_top_class = result.class_id;
    g_project3_raw_second_class = result.second_class_id;
    g_project3_raw_top_permille =
        (unsigned int)(result.confidence * 1000.0f + 0.5f);
    g_project3_raw_second_permille =
        (unsigned int)(result.second_confidence * 1000.0f + 0.5f);
    g_project3_raw_margin_permille =
        (unsigned int)(result.margin * 1000.0f + 0.5f);

    if (g_project3_memory_fault) {
        ctx->last_result = result;
        Project3_SetAppState(ctx, PROJECT3_APP_ERROR);
        Project3_SetUiText(ctx, "MEMORY ERROR", "", "");
        ctx->error_hold_blocks = PROJECT3_ERROR_HOLD_BLOCKS;
        ctx->input_gate_blocks = PROJECT3_POST_INFER_IGNORE_BLOCKS;
        Project3_ResetUtterance(ctx);
        return;
    }
    accepted = Project3_AcceptResult(&result);
    g_project3_result_accepted = accepted;
    ctx->input_gate_blocks = PROJECT3_POST_INFER_IGNORE_BLOCKS;
    Project3_ResetUtterance(ctx);
    Project3_ResetPreroll();

    if (!accepted) {
        ctx->last_result = result;
        if (result.class_id != PROJECT3_CLASS_SILENCE) {
            /* A low-confidence command is presented as unknown instead of
             * leaving the user with an apparently frozen Listening screen. */
            Project3_CommitStableWord(ctx, "unknown");
            ctx->message_hold_blocks = PROJECT3_RESULT_HOLD_BLOCKS;
            ctx->message_return_to_listening = 1u;
        } else {
            Project3_SetAppState(ctx, PROJECT3_APP_LISTENING);
            Project3_SetUiText(ctx, P3_LISTENING_TEXT, "", "");
        }
        return;
    }

    ctx->last_result = result;
    label = Project3_GetLabel(result.class_id);
    Project3_CommitStableWord(ctx, label);
    ctx->recognized_count++;
    ctx->message_hold_blocks = PROJECT3_RESULT_HOLD_BLOCKS;
    ctx->message_return_to_listening = 1u;
}

#if 0
static unsigned char Project3_RenderScreen(PROJECT3_CONTEXT *ctx, unsigned char force_redraw)
{
    unsigned long color = ClrWhite;

    // 防重入检查
    if (s_lcd_busy) {
        return 0;
    }

    // 只在文字真正改变时重绘
    if (!force_redraw && strncmp(ctx->main_text, ctx->last_main_text, PROJECT3_RESULT_TEXT_LEN) == 0) {
        return 1;
    }

    // 设置忙标志
    s_lcd_busy = 1;

    // 设置文字颜色
    if (ctx->app_state == PROJECT3_APP_RESULT) {
        color = ClrWhite;
    } else {
        color = ClrLightSteelBlue;
    }

    // 原子操作：清空并绘制（不可分割）
    Project3_ClearAndDrawText(ctx->main_text, color);

    // 记录当前文字，避免重复绘制
    strncpy(ctx->last_main_text, ctx->main_text, PROJECT3_RESULT_TEXT_LEN);
    ctx->last_main_text[PROJECT3_RESULT_TEXT_LEN - 1] = '\0';

    // 清除忙标志
    s_lcd_busy = 0;
    return 1;
}

#endif

static unsigned char Project3_RenderScreen(PROJECT3_CONTEXT *ctx, unsigned char force_redraw)
{
    unsigned long color;

    if (s_lcd_busy) {
        return 0;
    }

    if (!force_redraw &&
        strncmp(ctx->main_text, ctx->last_main_text, PROJECT3_RESULT_TEXT_LEN) == 0) {
        return 1;
    }

    s_lcd_busy = 1;

    color = (strcmp(ctx->main_text, P3_LISTENING_TEXT) == 0) ? ClrLightSteelBlue : ClrWhite;
    if (!Project3_ClearAndDrawText(ctx->main_text, color)) {
        s_lcd_busy = 0;
        return 0;
    }

    strncpy(ctx->last_main_text, ctx->main_text, PROJECT3_RESULT_TEXT_LEN);
    ctx->last_main_text[PROJECT3_RESULT_TEXT_LEN - 1] = '\0';

    s_lcd_busy = 0;
    return 1;
}

static void Project3_InitContext(PROJECT3_CONTEXT *ctx)
{
    /* Writing TSCL starts the free-running C674x cycle counter used for
     * non-intrusive inference timing. */
    TSCL = 0;
    TSCH = 0;
    memset(ctx, 0, sizeof(PROJECT3_CONTEXT));
    ctx->pass_through_enable = PROJECT3_PASS_THROUGH_ENABLE;
    ctx->model_state = PROJECT3_MODEL_READY;
    ctx->app_state = PROJECT3_APP_BOOT;
    ctx->vad.noise_floor = PROJECT3_VAD_MIN_FLOOR;
    ctx->vad.smooth_energy = PROJECT3_VAD_MIN_FLOOR;
    strcpy(ctx->main_text, "Booting...");
    strcpy(ctx->line1, "Initializing system");
    strcpy(ctx->line2, "Please wait");
    strcpy(ctx->stable_text, P3_LISTENING_TEXT);
    ctx->stable_text_valid = 0u;
    ctx->redraw_needed = 1;
    Project3_ResetPreroll();
    Project3_ResetMemoryGuard();
    Project3_InitTables();
}

static void Project3_InitUi(PROJECT3_CONTEXT *ctx)
{
    Lcd_Init();

    // 防重入标志初始化
    s_lcd_busy = 0;
    s_lcd_layout_ready = 0u;

    // 全屏清屏一次（只在初始化时）- 使用固定布局宏

    // 初始化 last_main_text 为空，确保第一次一定绘制
    ctx->last_main_text[0] = '\0';

    // 等待LCD稳定
    volatile int i;
    for (i = 0; i < 100000; i++);

    // 绘制初始界面 - 强制刷新
    Project3_RenderScreen(ctx, 1);
}

static void Project3_HandleKeys(PROJECT3_CONTEXT *ctx)
{
    if (FLAG_KEY1) {
        FLAG_KEY1 = 0;
        // KEY1: 清除当前结果，回到 listening
        Project3_ResetUtterance(ctx);
        Project3_ResetPreroll();
        ctx->last_result.valid = 0;
        ctx->input_gate_blocks = 0;
        ctx->message_hold_blocks = 0;
        ctx->message_return_to_listening = 0;
        ctx->error_hold_blocks = 0;
        Project3_ClearStableDisplay(ctx);
    }
    if (FLAG_KEY2) {
        FLAG_KEY2 = 0;
        // KEY2: 降低VAD灵敏度（减少误触发）
        ctx->vad.noise_floor *= 1.15f;
        Project3_SetUiText(ctx, "VAD less sensitive", "", "");
        ctx->message_hold_blocks = PROJECT3_MESSAGE_HOLD_BLOCKS;
        ctx->message_return_to_listening = 0;
    }
    if (FLAG_KEY3) {
        FLAG_KEY3 = 0;
        // KEY3: 提高VAD灵敏度（更容易触发）
        ctx->vad.noise_floor *= 0.85f;
        Project3_SetUiText(ctx, "VAD more sensitive", "", "");
        ctx->message_hold_blocks = PROJECT3_MESSAGE_HOLD_BLOCKS;
        ctx->message_return_to_listening = 0;
    }
    if (FLAG_KEY4) {
        FLAG_KEY4 = 0;
        // KEY4: 重置VAD基线
        Project3_ResetUtterance(ctx);
        Project3_ResetPreroll();
        ctx->vad.noise_floor = PROJECT3_VAD_MIN_FLOOR;
        ctx->vad.smooth_energy = PROJECT3_VAD_MIN_FLOOR;
        ctx->vad.calibrate_frames = 0;
        ctx->vad.speech_hold_frames = 0;
        ctx->vad.silence_hold_frames = 0;
        ctx->vad.active = 0;
        Project3_SetUiText(ctx, "Calibrating...", "", "");
        ctx->message_hold_blocks = PROJECT3_MESSAGE_HOLD_BLOCKS;
        ctx->message_return_to_listening = 0;
    }
    if (FLAG_KEY5) {
        FLAG_KEY5 = 0;
        // KEY5: 系统重启
        Project3_InitContext(ctx);
        Project3_ModelInit(ctx);
    }
}

static void Project3_HandleTouch(PROJECT3_CONTEXT *ctx)
{
    (void)ctx;
    FLAG_TOUCH = 0;
    FLAG_BUTTON_1 = 0;
    FLAG_BUTTON_2 = 0;
    FLAG_BUTTON_3 = 0;
    FLAG_BUTTON_4 = 0;
    FLAG_BUTTON_5 = 0;
    FLAG_BUTTON_6 = 0;
    FLAG_BUTTON_7 = 0;
    FLAG_BUTTON_8 = 0;
}

static void Project3_ProcessAudioBlock(PROJECT3_CONTEXT *ctx, short *block, unsigned int block_samples)
{
    unsigned int offset;
    unsigned int i;
    unsigned char block_has_speech = 0;
    unsigned char end_pending = 0;
    int peak = 0;

    g_project3_audio_block_count++;
    for (i = 0u; i < block_samples; i++) {
        int sample = (int)block[i];
        if (sample < 0) sample = -sample;
        if (sample > peak) peak = sample;
    }
    if (peak > 32767) peak = 32767;
    g_project3_audio_peak = (short)peak;

    Project3_ServiceErrorHold(ctx);
    Project3_ServiceMessageHold(ctx);

    if (ctx->error_hold_blocks > 0) {
        return;
    }

    if (ctx->message_hold_blocks > 0) {
        return;
    }

    if (ctx->input_gate_blocks > 0) {
        ctx->input_gate_blocks--;
        return;
    }

    for (offset = 0; offset + PROJECT3_VAD_FRAME_LEN <= block_samples; offset += PROJECT3_VAD_HOP) {
        float frame_energy = Project3_ComputeFrameEnergy(&block[offset], PROJECT3_VAD_FRAME_LEN);
        unsigned char started = Project3_UpdateVad(ctx, frame_energy);

        g_project3_last_frame_energy = frame_energy;
        g_project3_vad_noise_floor = ctx->vad.noise_floor;
        g_project3_vad_smooth_energy = ctx->vad.smooth_energy;

        ctx->frame_counter++;

        if (started) {
            ctx->utter.ready = 0;
            Project3_AppendPreroll(ctx);
        }

        if (ctx->vad.active || started) {
            block_has_speech = 1;
        }

        if (Project3_ShouldEndUtterance(ctx)) {
            end_pending = 1;
            block_has_speech = 1;
            break;
        }
    }

    if (block_has_speech) {
        Project3_AppendRawBlock(ctx, block, block_samples);
        ctx->active_speech_samples += block_samples;
        g_project3_current_utterance_samples = ctx->utter.count;
    } else {
        Project3_UpdatePreroll(block, block_samples);
    }

    if (end_pending || ctx->utter.count >= PROJECT3_RAW_MAX_SAMPLES) {
        g_project3_vad_end_count++;
        if (ctx->utter.count >= PROJECT3_MIN_UTTERANCE_SAMPLES &&
            ctx->active_speech_samples >= PROJECT3_MIN_ACTIVE_SPEECH_SAMPLES) {
            Project3_HandleSpeechEnd(ctx);
        } else {
            Project3_ResetUtterance(ctx);
            ctx->input_gate_blocks = 0;
        }
    }
}

static void Project3_ModelInit(PROJECT3_CONTEXT *ctx)
{
    ctx->model_state = PROJECT3_MODEL_READY;
    Project3_ClearStableDisplay(ctx);
}

static PROJECT3_INFER_RESULT Project3_RunInference(PROJECT3_CONTEXT *ctx, const PROJECT3_UTTERANCE_BUFFER *utter)
{
    PROJECT3_INFER_RESULT result;

    ctx->model_state = PROJECT3_MODEL_READY;
    Project3_ResetMemoryGuard();
    Project3_LcdPauseForInference();

    Project3_ExtractLogMel(utter, g_project3_logmel);
    if (!Project3_CheckMemoryGuard()) {
        result = Project3_MemoryFaultResult();
        Project3_LcdResumeAfterInference();
        return result;
    }
    Project3_BCResNetForward(g_project3_logmel, g_project3_logits);
    if (!Project3_CheckMemoryGuard()) {
        result = Project3_MemoryFaultResult();
        Project3_LcdResumeAfterInference();
        return result;
    }
    result = Project3_LogitsToResult(g_project3_logits);
    Project3_LcdResumeAfterInference();
    return result;
}

static const char *Project3_GetLabel(unsigned char class_id)
{
    if (class_id >= PROJECT3_CMD_COUNT) {
        return "_unknown_";
    }
    return g_project3_labels[class_id];
}

static void Project3_ResetUtterance(PROJECT3_CONTEXT *ctx)
{
    ctx->utter.count = 0;
    ctx->utter.ready = 0;
    ctx->vad.active = 0;
    ctx->vad.speech_hold_frames = 0;
    ctx->vad.silence_hold_frames = 0;
    ctx->active_speech_samples = 0u;
    g_project3_current_utterance_samples = 0u;
}

static void Project3_ServiceMessageHold(PROJECT3_CONTEXT *ctx)
{
    if (ctx->message_hold_blocks > 0) {
        ctx->message_hold_blocks--;
        if (ctx->message_hold_blocks == 0) {
            if (ctx->message_return_to_listening) {
                ctx->message_return_to_listening = 0u;
                ctx->input_gate_blocks = 0u;
                Project3_SetAppState(ctx, PROJECT3_APP_LISTENING);
                Project3_SetUiText(ctx, P3_LISTENING_TEXT, "", "");
                Project3_ResetPreroll();
            } else {
                Project3_RestoreStableDisplay(ctx);
            }
        }
    }
}

static void Project3_ServiceErrorHold(PROJECT3_CONTEXT *ctx)
{
    if (ctx->error_hold_blocks > 0) {
        ctx->error_hold_blocks--;
        if (ctx->error_hold_blocks == 0) {
            Project3_RestoreStableDisplay(ctx);
        }
    }
}

static void Project3_UpdateUi(PROJECT3_CONTEXT *ctx, unsigned char force_redraw)
{
    if (force_redraw || ctx->redraw_needed) {
        if (!force_redraw && ctx->lcd_retry_frame != 0u && ctx->frame_counter < ctx->lcd_retry_frame) {
            return;
        }
        if (Project3_RenderScreen(ctx, force_redraw)) {
            ctx->redraw_needed = 0;
            ctx->lcd_retry_frame = 0u;
            ctx->lcd_last_redraw_frame = ctx->frame_counter;
        } else {
            ctx->lcd_retry_frame = ctx->frame_counter + 4u;
        }
    }
}

#endif
