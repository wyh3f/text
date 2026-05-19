#ifndef ALGORITHMS_FILTER_H_
#define ALGORITHMS_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>





/**
 * @file algorithms_filter.h
 * @brief 数字滤波算法库（支持多实例复用）
 * @note  所有滤波器均采用结构体上下文设计，可同时创建多个独立实例
 */



/*==================== 宏定义（可根据需求调整） ====================*/
#define MEDIAN_MAX_WINDOW   10   // 中位值滤波最大窗口（奇数）
#define ARITH_MAX_WINDOW    16   // 算术平均滤波最大窗口
#define WAVG_MAX_WINDOW     8    // 加权平均滤波最大窗口

/*==================== 1. 限幅滤波 ====================*/
/**
 * 限幅滤波器上下文
 */
typedef struct {
    int16_t last_valid;    ///< 上一次的有效值
} LimitChange_Filter_CTX;

/**
 * @brief 初始化限幅滤波器
 * @param ctx           滤波器上下文指针
 * @param initial_value 初始有效值
 */
void LimitChange_Filter_Init(LimitChange_Filter_CTX *ctx, int16_t initial_value);

/**
 * @brief 限幅滤波（消除突发脉冲干扰）
 * @param ctx        滤波器上下文指针
 * @param new_value  新的采样值
 * @param max_change 与上次有效值的最大允许误差
 * @return           滤波后的值
 */
int16_t LimitChange_Filter_Update(LimitChange_Filter_CTX *ctx, int16_t new_value, int16_t max_change);

/*==================== 2. 中位值滤波（滑动窗口） ====================*/
/**
 * 中位值滤波器上下文
 */
typedef struct {
    int16_t buf[MEDIAN_MAX_WINDOW];   // 环形缓冲区
    int16_t cur_size;                 // 实际窗口大小（已调整为奇数）
    int16_t count;                    // 已存入的有效样本数
    int16_t pos;                      // 下一个写入位置
} Median_Filter_CTX;

/**
 * @brief 初始化中位值滤波器
 * @param ctx         滤波器上下文指针
 * @param window_size 窗口大小（自动转为奇数，且不超过 MEDIAN_MAX_WINDOW）
 */
void Median_Filter_Init(Median_Filter_CTX *ctx, int16_t window_size);

/**
 * @brief 中位值滤波（滑动窗口中位数）
 * @param ctx 滤波器上下文指针
 * @param dat 当前采样值
 * @return    当前窗口的中位数值
 */
int16_t Median_Filter_Update(Median_Filter_CTX *ctx, int16_t dat);

/*==================== 3. 三数中值滤波（无状态，纯算法） ====================*/
/**
 * @brief 三数中值滤波（轻量级）
 * @param a 第一个样本值
 * @param b 第二个样本值
 * @param c 第三个样本值
 * @return  三个数中的中位数值
 */
int16_t Median_Filter_Of_3(int16_t a, int16_t b, int16_t c);

/*==================== 4. 算术平均滤波（滑动平均，可调窗口） ====================*/
/**
 * 算术平均滤波器上下文
 */
typedef struct {
    int16_t buf[ARITH_MAX_WINDOW]; // 环形缓冲区
    int16_t sum;                   // 窗口内和
    uint8_t idx;                   // 写入位置
    uint8_t cnt;                   // 当前有效样本数（≤ window_size）
    uint8_t window_size;           // 实际窗口大小（1 ~ ARITH_MAX_WINDOW）
} Arithmetic_Filter_CTX;

/**
 * @brief 初始化算术平均滤波器
 * @param ctx         滤波器上下文指针
 * @param window_size 窗口大小（1 ~ ARITH_MAX_WINDOW）
 */
void Arithmetic_Filter_Init(Arithmetic_Filter_CTX *ctx, uint8_t window_size);

/**
 * @brief 算术平均滤波（滑动窗口平均值）
 * @param ctx         滤波器上下文指针
 * @param new_sample  新采集的样本值
 * @return            当前窗口内的算术平均值（整数，四舍五入）
 */
int16_t Arithmetic_Filter_Update(Arithmetic_Filter_CTX *ctx, int16_t new_sample);

/*==================== 5. 一阶低通滤波 ====================*/
/**
 * 一阶低通滤波器上下文
 */
typedef struct {
    int16_t last_output;   ///< 上一次滤波输出
} LowPass_Filter_CTX;

/**
 * @brief 初始化一阶低通滤波器
 * @param ctx            滤波器上下文指针
 * @param initial_output 初始输出值（通常为第一次采样值或0）
 */
void LowPass_Filter_Init(LowPass_Filter_CTX *ctx, int16_t initial_output);

/**
 * @brief 一阶低通滤波（定点整数实现）
 * @param ctx        滤波器上下文指针
 * @param new_sample 当前采样值
 * @param alpha      滤波系数（0~256，实际系数 = alpha/256）
 *                   值越小平滑越强，响应越慢；值越大响应越快。
 * @return           滤波输出值
 */
int16_t LowPass_Filter_Update(LowPass_Filter_CTX *ctx, int16_t new_sample, uint16_t alpha);

/*==================== 6. 一阶高通滤波 ====================*/
/**
 * 一阶高通滤波器上下文
 */
typedef struct {
    int16_t lowpass_last;   ///< 上次低通滤波输出（内部状态）
} HighPass_Filter_CTX;

/**
 * @brief 初始化一阶高通滤波器
 * @param ctx               滤波器上下文指针
 * @param initial_lowpass   初始低通输出（通常设为第一次采样值或0）
 */
void HighPass_Filter_Init(HighPass_Filter_CTX *ctx, int16_t initial_lowpass);

/**
 * @brief 一阶高通滤波（定点整数实现）
 * @param ctx        滤波器上下文指针
 * @param new_sample 当前采样值
 * @param alpha      滤波系数（0~256，实际系数 = alpha/256）
 *                   值越小高通截止频率越低，值越大高通越敏感。
 * @return           高通滤波输出（信号中的高频分量）
 */
int16_t HighPass_Filter_Update(HighPass_Filter_CTX *ctx, int16_t new_sample, uint16_t alpha);

/*==================== 7. 加权递推平均滤波（可自定义权重） ====================*/
/**
 * 加权平均滤波器上下文
 */
typedef struct {
    int16_t buffer[WAVG_MAX_WINDOW];   // 环形缓冲区
    uint8_t window_size;               // 实际窗口大小
    uint8_t weights[WAVG_MAX_WINDOW];  // 权重系数（最新->最旧）
    uint8_t weight_sum;                // 权重总和（缓存，提高速度）
    uint8_t idx;                       // 下一个写入位置
    uint8_t filled;                    // 已填充样本数（≤ window_size）
} WeightedAverage_Filter_CTX;

/**
 * @brief 初始化加权平均滤波器
 * @param ctx         滤波器上下文指针
 * @param window_size 窗口大小（1 ~ WAVG_MAX_WINDOW）
 * @param weights     权重数组，长度为 window_size，索引0对应最新样本的权重，依次递增。
 *                    例如 window_size=4, weights={4,3,2,1} 表示最新样本权重最大。
 */
void WeightedAverage_Filter_Init(WeightedAverage_Filter_CTX *ctx, uint8_t window_size, const uint8_t *weights);

/**
 * @brief 加权递推平均滤波（在线滑动窗口）
 * @param ctx         滤波器上下文指针
 * @param new_sample  新采集的样本值
 * @return            加权平均输出（整数，四舍五入）
 */
int16_t WeightedAverage_Filter_Update(WeightedAverage_Filter_CTX *ctx, int16_t new_sample);



/*==================== 主测试入口 ====================*/
/**
 * @brief 测试所有数字滤波算法（打印输出）
 * 实际嵌入式应用中可移除此函数或改为硬件输出（如DAC、串口）
 */
void test_all_filters(void);


#ifdef __cplusplus
}
#endif

#endif

