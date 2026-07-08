// #include "project1.h"

// #pragma DATA_ALIGN(h, 8)
// float h[ORD2] = {
//     -6.315349e-04f, -5.510414e-04f, -3.259621e-04f,  2.340440e-06f,
//      3.642858e-04f,  6.766050e-04f,  8.611797e-04f,  8.640147e-04f,
//      6.698439e-04f,  3.085988e-04f, -1.483775e-04f, -6.022076e-04f,
//     -9.478917e-04f, -1.098731e-03f, -1.008196e-03f, -6.838552e-04f,
//     -1.894605e-04f,  3.662389e-04f,  8.523396e-04f,  1.145797e-03f,
//      1.162045e-03f,  8.789646e-04f,  3.478767e-04f, -3.122778e-04f,
//     -9.377814e-04f, -1.356247e-03f, -1.428731e-03f, -1.088462e-03f,
//     -3.666099e-04f,  6.024938e-04f,  1.600312e-03f,  2.365728e-03f,
//      2.652446e-03f,  2.290888e-03f,  1.240943e-03f, -3.776170e-04f,
//     -2.286936e-03f, -4.089015e-03f, -5.336370e-03f, -5.624458e-03f,
//     -4.688907e-03f, -2.487475e-03f,  7.526787e-04f,  4.537047e-03f,
//      8.161600e-03f,  1.082239e-02f,  1.176229e-02f,  1.043122e-02f,
//      6.631116e-03f,  6.163657e-04f, -6.874063e-03f, -1.466670e-02f,
//     -2.128611e-02f, -2.515561e-02f, -2.483840e-02f, -1.928255e-02f,
//     -8.029233e-03f,  8.652438e-03f,  2.973120e-02f,  5.349194e-02f,
//      7.772069e-02f,  9.997327e-02f,  1.178896e-01f,  1.295080e-01f,
//      1.335319e-01f,  1.295080e-01f,  1.178896e-01f,  9.997327e-02f,
//      7.772069e-02f,  5.349194e-02f,  2.973120e-02f,  8.652438e-03f,
//     -8.029233e-03f, -1.928255e-02f, -2.483840e-02f, -2.515561e-02f,
//     -2.128611e-02f, -1.466670e-02f, -6.874063e-03f,  6.163657e-04f,
//      6.631116e-03f,  1.043122e-02f,  1.176229e-02f,  1.082239e-02f,
//      8.161600e-03f,  4.537047e-03f,  7.526787e-04f, -2.487475e-03f,
//     -4.688907e-03f, -5.624458e-03f, -5.336370e-03f, -4.089015e-03f,
//     -2.286936e-03f, -3.776170e-04f,  1.240943e-03f,  2.290888e-03f,
//      2.652446e-03f,  2.365728e-03f,  1.600312e-03f,  6.024938e-04f,
//     -3.666099e-04f, -1.088462e-03f, -1.428731e-03f, -1.356247e-03f,
//     -9.377814e-04f, -3.122778e-04f,  3.478767e-04f,  8.789646e-04f,
//      1.162045e-03f,  1.145797e-03f,  8.523396e-04f,  3.662389e-04f,
//     -1.894605e-04f, -6.838552e-04f, -1.008196e-03f, -1.098731e-03f,
//     -9.478917e-04f, -6.022076e-04f, -1.483775e-04f,  3.085988e-04f,
//      6.698439e-04f,  8.640147e-04f,  8.611797e-04f,  6.766050e-04f,
//      3.642858e-04f,  2.340440e-06f, -3.259621e-04f, -5.510414e-04f,
//     -6.315349e-04f
// };

// // 【硬件对齐优化】多相子滤波器矩阵与旋转因子查找表，强制 8 字节对齐激活 64位宽高速访存
// #pragma DATA_ALIGN(h_poly, 8)
// float h_poly[D][P_TAPS];

// #pragma DATA_ALIGN(twiddle_cos, 8)
// float twiddle_cos[D][D];

// #pragma DATA_ALIGN(twiddle_sin, 8)
// float twiddle_sin[D][D];

// // ==================== 【子带处理模块：轻量级降噪】 ====================
// // 当前先默认旁路，避免在 analysis/synthesis 还未完全校准前引入额外增益起伏。
// #define ENABLE_SUBBAND_DENOISE        0
// #define SUBBAND_ENERGY_SMOOTH_ALPHA   0.85f
// #define SUBBAND_NOISE_RISE_ALPHA      0.995f
// #define SUBBAND_NOISE_FALL_ALPHA      0.900f
// #define SUBBAND_GAIN_MIN              0.25f
// #define SUBBAND_GAIN_MAX              1.00f
// #define SUBBAND_NOISE_MARGIN          1.50f

// static float subband_compute_energy(const float sample_re, const float sample_im)
// {
//     return sample_re * sample_re + sample_im * sample_im;
// }

// static float subband_clamp_gain(float gain)
// {
//     if (gain < SUBBAND_GAIN_MIN) return SUBBAND_GAIN_MIN;
//     if (gain > SUBBAND_GAIN_MAX) return SUBBAND_GAIN_MAX;
//     return gain;
// }

// void initialize_subband_processor(SUBBAND_PROC_STATE *st)
// {
//     int k;
//     memset(st, 0, sizeof(SUBBAND_PROC_STATE));
//     for (k = 0; k < D; k++) {
//         st->gain[k] = 1.0f;
//     }
// }

// void subband_process_frame(SUBBAND_PROC_STATE *restrict st, float subband_data[D][2])
// {
//     int k;

// #if ENABLE_SUBBAND_DENOISE
//     for (k = 0; k < D; k++) {
//         float energy;
//         float target_gain;

//         energy = subband_compute_energy(subband_data[k][0], subband_data[k][1]);
//         st->smooth_energy[k] = SUBBAND_ENERGY_SMOOTH_ALPHA * st->smooth_energy[k]
//                             + (1.0f - SUBBAND_ENERGY_SMOOTH_ALPHA) * energy;

//         if (st->noise_floor[k] == 0.0f) {
//             st->noise_floor[k] = st->smooth_energy[k];
//         } else if (st->smooth_energy[k] < st->noise_floor[k]) {
//             st->noise_floor[k] = SUBBAND_NOISE_FALL_ALPHA * st->noise_floor[k]
//                                + (1.0f - SUBBAND_NOISE_FALL_ALPHA) * st->smooth_energy[k];
//         } else {
//             st->noise_floor[k] = SUBBAND_NOISE_RISE_ALPHA * st->noise_floor[k]
//                                + (1.0f - SUBBAND_NOISE_RISE_ALPHA) * st->smooth_energy[k];
//         }

//         if (st->smooth_energy[k] <= st->noise_floor[k] * SUBBAND_NOISE_MARGIN) {
//             target_gain = SUBBAND_GAIN_MIN;
//         } else {
//             target_gain = 1.0f - st->noise_floor[k] / (st->smooth_energy[k] + 1.0e-12f);
//             target_gain = subband_clamp_gain(target_gain);
//         }

//         st->gain[k] = 0.80f * st->gain[k] + 0.20f * target_gain;
//         st->gain[k] = subband_clamp_gain(st->gain[k]);

//         subband_data[k][0] *= st->gain[k];
//         subband_data[k][1] *= st->gain[k];
//     }
// #else
//     (void)st;
//     (void)subband_data;
//     for (k = 0; k < D; k++) {
//         /* 默认旁路：当前不对子带做任何增益修改 */
//     }
// #endif
// }

// // ==================== 【系统离线预拆解与预计算】 ====================
// void init_polyphase_system(void) {
//     int k, l, m;
    
//     // 1. 将 129 阶原型低通滤波器按 M 抽样规律，拆解组合到 8 个 17 阶的多相子滤波器中
//     // 参考多相实现中常见的相位对齐（等价于 MATLAB 里的 circshift(h,[1,0])），
//     // 这里只做一个相位的保守平移，尽量缓解重构时的轻微沙哑感。
//     for (l = 0; l < D; l++) {
//         int shifted_phase = (l + 1) % D;
//         for (m = 0; m < P_TAPS; m++) {
//             int original_idx = m * D + l;
//             if (original_idx < ORD2) {
//                 h_poly[shifted_phase][m] = h[original_idx]; // 导入 RRC 资产系数并做相位对齐
//             } else {
//                 h_poly[shifted_phase][m] = 0.0f;            // 尾部 7 个点自动补 0 填充
//             }
//         }
//     }
    
//     // 2. 预计算 8 点 变换矩阵的旋转因子，彻底把 cosf/sinf 赶出实时运行大循环
//     for (k = 0; k < D; k++) {
//         for (l = 0; l < D; l++) {
//             twiddle_cos[k][l] = cosf(2.0f * PI * l * k / (float)D);
//             twiddle_sin[k][l] = sinf(2.0f * PI * l * k / (float)D);
//         }
//     }
// }

// // ==================== 【快速算法：多相分析滤波器组】 ====================
// // 第一阶段优化：analysis 改成环形缓冲，避免每次整体搬移 136 点状态
// void polyphase_analysis(POLY_ANA_STATE *restrict st, const short input_samples[D], float subband_out[D][2]) {
//     long k, l, m;
//     float V[D] = {0.0f};
//     unsigned int old_head = st->head;
//     unsigned int new_head = (old_head + ORD2_PAD - D) % ORD2_PAD;

//     // 1. 顺控倒序喂入旋转开关（Commutator 机制），只更新 8 个新点
//     for (l = 0; l < D; l++) {
//         st->ana_line[(new_head + l) % ORD2_PAD] = (float)input_samples[D - 1 - l];
//     }
//     st->head = new_head;

//     // 2. 核心多相轻量滤波：进行 8 组独立、且只有 17 阶的轻量小卷积
//     for (l = 0; l < D; l++) {
//         float sum = 0.0f;
//         #pragma MUST_ITERATE(P_TAPS, P_TAPS)
//         for (m = 0; m < P_TAPS; m++) {
//             unsigned int idx = (st->head + m * D + l) % ORD2_PAD;
//             sum += h_poly[l][m] * st->ana_line[idx];
//         }
//         V[l] = sum;
//     }

//     // 3. 高效 8 点 DFT 变换：一次性并行吐出 8 个子带的复数信号
//     for (k = 0; k < D; k++) {
//         float re = 0.0f;
//         float im = 0.0f;
//         #pragma MUST_ITERATE(D, D)
//         for (l = 0; l < D; l++) {
//             re += V[l] * twiddle_cos[k][l];
//             im += V[l] * twiddle_sin[k][l];
//         }
//         subband_out[k][0] = re;
//         subband_out[k][1] = im;
//     }
// }

// // ==================== 【真正的多相合成滤波器组】 ====================
// // 当前先只做最小修正：不推翻整体结构，只微调重构增益和输出换相，目标是缓解“增益偏小”和“沙哑”。
// #define SYNTHESIS_GAIN_TRIM  (1.35f)
// void polyphase_synthesis(POLY_SYN_STATE *restrict st, const float subband_in[D][2], short output_samples[D]) {
//     long k, l, m;
//     float phase_in[D] = {0.0f};
//     float phase_out[D] = {0.0f};

//     // 1. 8 点 IDFT：子带域 -> 多相支路域
//     for (l = 0; l < D; l++) {
//         float re = 0.0f;
//         #pragma MUST_ITERATE(D, D)
//         for (k = 0; k < D; k++) {
//             // analysis 用的是 exp(+j*2*pi*k*l/D)，这里用 exp(-j*2*pi*k*l/D)
//             // 对复数 a+jb 与 (c-js) 相乘的实部：ac + bs
//             re += subband_in[k][0] * twiddle_cos[k][l] + subband_in[k][1] * twiddle_sin[k][l];
//         }
//         phase_in[l] = re / (float)D;
//     }

//     // 2. 各支路分别做 17 阶多相 FIR，使用环形缓冲更新状态
//     for (l = 0; l < D; l++) {
//         unsigned int old_head = st->head[l];
//         unsigned int new_head = (old_head + P_TAPS - 1) % P_TAPS;
//         st->head[l] = new_head;
//         st->syn_line[l][new_head] = phase_in[l];

//         #pragma MUST_ITERATE(P_TAPS, P_TAPS)
//         for (m = 0; m < P_TAPS; m++) {
//             unsigned int idx = (new_head + m) % P_TAPS;
//             phase_out[l] += h_poly[l][m] * st->syn_line[l][idx];
//         }
//     }

//     // 3. 通过 commutator 逆换相恢复 8 个连续时域点
//     for (l = 0; l < D; l++) {
//         float out_val = phase_out[D - 1 - l] * (float)D * SYNTHESIS_GAIN_TRIM;

//         // 保守做法：保留当前逆换相，但补足一个轻微增益修正，先解决声音偏小问题。
//         if (out_val > 0.0f) out_val += 0.5f;
//         else out_val -= 0.5f;

//         if (out_val > 32767.0f)       out_val = 32767.0f;
//         else if (out_val < -32768.0f) out_val = -32768.0f;

//         output_samples[l] = (short)out_val;
//     }
// }

// /* ==================== 【全新重构的无阻塞 MAIN 函数】 ==================== */
// int main(void) {
//     static short User_Buffer1[ADC_SAMPLE_1024];
    
//     // 【架构巨变】现在子带数据 subband_data 只需要保存当前时刻 8 个通道的复数即可！
//     static float subband_data[D][2];

//     static POLY_ANA_STATE ana_st;
//     static POLY_SYN_STATE syn_st;
//     static SUBBAND_PROC_STATE proc_st;

//     long offset, i;
//     unsigned char FLAG_DSP_READY = 0;
//     unsigned int startup_counter = 0;
//     Sys_Init();

//     // 硬件继续稳健跑 1024 点乒乓
//     Adc_Init(ADC_50KHZ, ADC_SAMPLE_1024);
//     Dac_Init(DAC_50KHZ, ADC_SAMPLE_1024, DAC_CHANNEL_ALL);

//     // 点火多相分解预计算状态机
//     printf("Compiling Polyphase Matrix & Twiddle Tables...\n");
//     init_polyphase_system();
//     memset(&ana_st, 0, sizeof(POLY_ANA_STATE));
//     memset(&syn_st, 0, sizeof(POLY_SYN_STATE));
//     initialize_subband_processor(&proc_st);
//     ana_st.head = 0;
//     for (i = 0; i < D; i++) syn_st.head[i] = 0;
//     printf("Polyphase Engine Ready.\n");

//     // 预填充 DAC 硬件静音缓冲
//     for(i = 0; i < ADC_SAMPLE_1024; i++) {
//         DA_CH1_Buf0[i] = 0; DA_CH1_Buf1[i] = 0;
//         DA_CH2_Buf0[i] = 0; DA_CH2_Buf1[i] = 0;
//         DA_CH3_Buf0[i] = 0; DA_CH3_Buf1[i] = 0;
//         DA_CH4_Buf0[i] = 0; DA_CH4_Buf1[i] = 0;
//         DA_CH5_Buf0[i] = 0; DA_CH5_Buf1[i] = 0;
//         DA_CH6_Buf0[i] = 0; DA_CH6_Buf1[i] = 0;
//         DA_CH7_Buf0[i] = 0; DA_CH7_Buf1[i] = 0;
//         DA_CH8_Buf0[i] = 0; DA_CH8_Buf1[i] = 0;
//     }

//     Adc_Start();
//     Dac_Start();

//     while(1) {
        
//         if (FLAG_AD == 1) {
//             FLAG_AD = 0; 

//             short *p_ad_src = (AD_Ping_Pong == AD_BUFFER_PONG) ? AD_CH1_Buf0 : AD_CH1_Buf1;
//             memcpy(User_Buffer1, p_ad_src, 2 * ADC_SAMPLE_1024); 

//             if (FLAG_DSP_READY == 0) {
//                 startup_counter++;
//                 if (startup_counter > 10) FLAG_DSP_READY = 1;
//             } else {
//                 // ========== 【多相分解黄金主循环】 ==========
//                 // 每次大步长向前推进 D=8 个时域点。
//                 // 1024 / 8 = 128 次极其轻量的小循环，运行耗时极低。
//                 //t1 = _itoll(TSCH, TSCL);

//                 for (offset = 0; offset < ADC_SAMPLE_1024; offset += D) {

//                     // 1. 输入 8 个点 -> 多相分析滤波器组，得到当前时刻 8 个复数子带
//                     polyphase_analysis(&ana_st, &User_Buffer1[offset], subband_data);

//                     // 2. 子带域处理：当前先接入轻量级降噪框架
//                     subband_process_frame(&proc_st, subband_data);

//                     // 3. 8 通道复数子带 -> 多相合成滤波器组，恢复 8 个时域输出点
//                     polyphase_synthesis(&syn_st, subband_data, &User_Buffer1[offset]);
//                 }
//                 // ===========================================
//                 //t2 = _itoll(TSCH, TSCL);
//                 //printf("Cycle count: %u", t2 - t1);
//             }

//             // 分发全量处理完的 1024 点到音频各输出通道
//             if (DA_Ping_Pong == DA_BUFFER_PONG) {
//                 memcpy(DA_CH1_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH2_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH3_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH4_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH5_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH6_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH7_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH8_Buf0, User_Buffer1, 2 * ADC_SAMPLE_1024);
//             } else {
//                 memcpy(DA_CH1_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH2_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH3_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH4_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH5_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH6_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH7_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//                 // memcpy(DA_CH8_Buf1, User_Buffer1, 2 * ADC_SAMPLE_1024);
//             }
            
//             FLAG_DA = 0; 
//         }
//     }
//     // Adda_Example();
//     // while(1){
        
//     // }
// }