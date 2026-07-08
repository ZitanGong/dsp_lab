
#include "driver_include.h"
#include "user_include.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI          3.14159265358979323846f
#define D           8       // 子带数目
#define ORD2        129     // 原始平方根升余弦滤波器阶数
#define ORD2_PAD    136     // 【多相优化】填充至 8 的整数倍 (17 * 8 = 136)
#define P_TAPS      17      // 每个多相子滤波器的阶数 (136 / 8 = 17)

// ==================== 【多相分解轻量化状态结构体】 ====================
// 第一阶段优化：把原来每次整体搬移状态数组，改成环形缓冲索引。
// 这样不改变算法结果，只减少无效内存搬运。
typedef struct {
    float ana_line[ORD2_PAD]; // 分析滤波器时域平铺延迟线 (136个点)
    unsigned int head;        // 环形缓冲写指针
} POLY_ANA_STATE;

typedef struct {
    float syn_line[D][P_TAPS]; // 合成滤波器 8 个通道、各 17 阶的分相延迟线
    unsigned int head[D];      // 每个子带支路各自的环形缓冲写指针
} POLY_SYN_STATE;

// ==================== 【子带处理模块状态】 ====================
// 先实现轻量级降噪框架，后续压缩编码可在此基础上继续扩展。
typedef struct {
    float smooth_energy[D];
    float noise_floor[D];
    float gain[D];
} SUBBAND_PROC_STATE;

/* 外部引用的低通原型滤波器系数 */
extern float h[ORD2];

/* 核心快速算法函数声明 */
void init_polyphase_system(void);
void polyphase_analysis(POLY_ANA_STATE *restrict st, const short input_samples[D], float subband_out[D][2]);
void polyphase_synthesis(POLY_SYN_STATE *restrict st, const float subband_in[D][2], short output_samples[D]);
void initialize_subband_processor(SUBBAND_PROC_STATE *st);
void subband_process_frame(SUBBAND_PROC_STATE *restrict st, float subband_data[D][2]);
