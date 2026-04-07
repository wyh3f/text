/**
 * @file algorithms_analog_waveform.c
 * @brief 模拟波形生成函数库（单精度浮点优化版）
 * 
 * 提供正弦波、余弦波、三角波、合成波（正弦+二次谐波）和白噪声的生成。
 * 所有波形均采用步进角度累加方式，输出范围归一化到 [-1.0f, 1.0f]。
 * 适用于嵌入式平台（尤其带单精度 FPU 的 MCU）。
 */

#include <algorithms/algorithms_analog_waveform.h>
#include <math.h>       // sinf, cosf, fmodf
#include <stdint.h>     // uint32_t, UINT32_MAX
#include <stdio.h>      // printf（仅用于测试，实际嵌入式可移除）

#define PI 3.14159265358979323846f   // 圆周率，单精度常量

/**
 * 角度转弧度（单精度）
 * @param degrees 角度值（单位：度）
 * @return 弧度值
 */
static float degrees_to_radians(float degrees) {
    return degrees * PI / 180.0f;
}

/**
 * 弧度转角度（单精度）
 * @param radians 弧度值
 * @return 角度值（单位：度）
 */
static float radians_to_degrees(float radians) {
    return radians * 180.0f / PI;
}

/**
 * 正弦波输出
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 正弦值，范围 [-1.0f, 1.0f]
 */
float out_sine_value(float step_size) {
    static float angle = 0.0f;          // 静态累积角度（度）
    float buf = step_size;
    if (buf < 0.05f) buf = 0.05f;       // 步长下限保护，避免角度变化过慢
    angle = fmodf(angle + buf, 360.0f); // 取模运算，确保角度在 [0, 360)
    return sinf(degrees_to_radians(angle));
}

/**
 * 余弦波输出
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 余弦值，范围 [-1.0f, 1.0f]
 */
float out_cosine_value(float step_size) {
    static float angle = 0.0f;
    float buf = step_size;
    if (buf < 0.05f) buf = 0.05f;
    angle = fmodf(angle + buf, 360.0f);
    return cosf(degrees_to_radians(angle));
}

/**
 * 三角波核心函数（角度 → 三角值）
 * @param degrees 角度（度），范围任意（内部会取模）
 * @return 三角波值，范围 [-1.0f, 1.0f]
 */
static float triangle_wave(float degrees) {
    float phase = fmodf(degrees, 360.0f);       // 相位归一化到 [0, 360)
    float t = phase / 180.0f;                   // 映射到 [0, 2)
    if (t < 1.0f) {
        return 2.0f * t - 1.0f;                 // 上升沿：-1 → 1
    } else {
        return 3.0f - 2.0f * t;                 // 下降沿：1 → -1
    }
}

/**
 * 三角波输出
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 三角波值，范围 [-1.0f, 1.0f]
 */
float out_triangle_value(float step_size) {
    static float angle = 0.0f;
    float buf = step_size;
    if (buf < 0.05f) buf = 0.05f;
    angle = fmodf(angle + buf, 360.0f);
    return triangle_wave(angle);
}

/**
 * 合成波形输出：正弦波 + 0.5倍二次谐波（归一化）
 * 原始表达式：f(θ) = sinθ + 0.5*sin(2θ)
 * 理论最大幅值：3√3/4 ≈ 1.299038105676658
 * 归一化后范围严格 [-1.0f, 1.0f]
 * @param step_size 每次调用的角度步长（度），小于0.05时强制设为0.05
 * @return 归一化合成波值，范围 [-1.0f, 1.0f]
 */
float out_composite_value(float step_size) {
    static float angle = 0.0f;
    float buf = step_size;
    if (buf < 0.05f) buf = 0.05f;
    angle = fmodf(angle + buf, 360.0f);

    float rad = degrees_to_radians(angle);
    float fundamental = sinf(rad);              // 基波
    float second_harmonic = sinf(2.0f * rad);   // 二次谐波
    const float amplitude_ratio = 0.5f;         // 二次谐波幅度比例
    float raw = fundamental + amplitude_ratio * second_harmonic;

    // 归一化因子：1 / (3√3/4) ≈ 0.769800358919501
    const float NORM_FACTOR = 0.769800358919501f;
    return raw * NORM_FACTOR;
}

// ==================== 白噪声生成（Xorshift）====================

static uint32_t rng_state = 0x12345678;   ///< 随机数生成器状态（不可为0）

/**
 * 设置白噪声随机种子
 * @param seed 种子值（若为0则忽略，保持原状态）
 */
void set_white_noise_seed(uint32_t seed) {
    if (seed != 0) {
        rng_state = seed;
    }
}

/**
 * Xorshift 随机数生成器（32位）
 * 周期 2^32-1，仅使用移位和异或操作，适合嵌入式环境
 * @return 32位无符号随机数
 */
static uint32_t xorshift_rand(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

/**
 * 白噪声输出（单精度浮点，范围 [-1.0f, 1.0f]）
 * 每次调用返回一个独立的随机数，不依赖角度步进。
 * @return 随机值，范围 [-1.0f, 1.0f]
 */
float out_white_noise_value(void) {
    uint32_t r = xorshift_rand();
    // 映射：r ∈ [0, UINT32_MAX] → [0.0f, 1.0f] → [-1.0f, 1.0f]
    return (r / (float)UINT32_MAX) * 2.0f - 1.0f;
}

// ==================== 测试函数（仅用于调试）====================
/**
 * @brief 测试所有波形生成函数（打印输出）
 * 实际嵌入式应用中可移除此函数或改为硬件输出（如DAC、PWM）
 */
void test_analog_waveform(void) {
    printf("---------------\n");
    printf("start_analog_waveform (float version)\n");

    // 示例：连续输出 450 个白噪声值
    for (uint16_t i = 0; i < 450; i++) {
        printf("%f\n", out_white_noise_value());
    }

    // 可添加其他波形测试，例如：
    // for (int i = 0; i < 360; i++) {
    //     printf("sine: %f\n", out_sine_value(1.0f));
    // }

    printf("test_analog_waveform end");
}