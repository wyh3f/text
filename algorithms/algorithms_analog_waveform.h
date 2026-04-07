#ifndef ALGORITHMS_ANALOG_WAVWFORM_H_
#define ALGORITHMS_ANALOG_WAVWFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**
 * 正弦波输出
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 正弦值，范围 [-1.0f, 1.0f]
 */
float out_sine_value(float step_size);

/**
 * 余弦波输出
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 余弦值，范围 [-1.0f, 1.0f]
 */
float out_cosine_value(float step_size);



/**
 * 三角波输出
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 三角波值，范围 [-1.0f, 1.0f]
 */
float out_triangle_value(float step_size);

/**
 * 合成波形输出：正弦波 + 0.5倍二次谐波（归一化）
 * 原始表达式：f(θ) = sinθ + 0.5*sin(2θ)
 * 理论最大幅值：3√3/4 ≈ 1.299038105676658
 * 归一化后范围严格 [-1.0f, 1.0f]
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 归一化合成波值，范围 [-1.0f, 1.0f]
 */
float out_composite_value(float step_size);

// ==================== 白噪声生成（Xorshift）====================



/**
 * 设置白噪声随机种子
 * @param seed 种子值（若为0则忽略，保持原状态）
 */
void set_white_noise_seed(uint32_t seed);

/**
 * 白噪声输出（单精度浮点，范围 [-1.0f, 1.0f]）
 * 每次调用返回一个独立的随机数，不依赖角度步进。
 * @return 随机值，范围 [-1.0f, 1.0f]
 */
float out_white_noise_value(void);


void test_analog_waveform(void);

#ifdef __cplusplus
}
#endif

#endif

