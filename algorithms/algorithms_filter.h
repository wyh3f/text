#ifndef ALGORITHMS_FILTER_H_
#define ALGORITHMS_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * 限幅滤波算法
 * @param new_value 新的滤波值
 * @param last_valid 上一次的值
 * @param max_change 新的滤波值与上一次的值最大允许的误差
 * @return 返回滤波后的值
 */
int16_t LimitChange_Filter(int16_t new_value, int16_t last_valid, int16_t max_change);


// 最大支持的窗口大小（根据内存调整）
#define MEDIAN_MAX_WINDOW  15

/**
 * 中位值滤波
 * @param dat         当前输入的原始值（未滤波）
 * @param window_size 窗口大小（会自动转为奇数，且不能超过 MEDIAN_MAX_WINDOW）
 * @return            当前滑动窗口的中位数值
 */
int16_t Median_Filter(int16_t dat, int16_t window_size);


/**
 * 三数中值滤波（纯算法）
 * @param a 第一个样本值
 * @param b 第二个样本值
 * @param c 第三个样本值
 * @return  三个数中的中位数值
 */
int16_t Median_Filter_Of_3(int16_t a, int16_t b, int16_t c);


// 最大窗口大小（根据内存调整，建议为2的幂以便用移位优化）
#define ARITH_MAX_WINDOW  8

/**
 * 递推算术平均滤波（滑动平均）
 * @param new_sample  新采集的样本值
 * @return            当前窗口内的算术平均值
 */
int16_t Arithmetic_Filter(int16_t new_sample);


#ifdef __cplusplus
}
#endif

#endif

