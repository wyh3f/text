#include "algorithms_filter.h"

/**
 * 限幅滤波算法
 * @param new_value 新的滤波值
 * @param last_valid 上一次的值
 * @param max_change 新的滤波值与上一次的值最大允许的误差
 * @return 返回滤波后的值
 */
int16_t LimitChange_Filter(int16_t new_value, int16_t last_valid, int16_t max_change)
{
    int32_t diff = (int32_t)new_value - (int32_t)last_valid;
    if (diff < 0) diff = -diff;
    if (diff > (int32_t)max_change)
    {
        return last_valid;
    }
    else
    {
        return new_value;
    }
}


// 最大支持的窗口大小（根据内存调整）
#define MEDIAN_MAX_WINDOW  15

/**
 * 中位值滤波
 * @param dat         当前输入的原始值（未滤波）
 * @param window_size 窗口大小（会自动转为奇数，且不能超过 MEDIAN_MAX_WINDOW）
 * @return            当前滑动窗口的中位数值
 */
int16_t Median_Filter(int16_t dat, int16_t window_size)
{
    static int16_t buf[MEDIAN_MAX_WINDOW];   // 环形缓冲区
    static int16_t cur_size = 0;             // 当前实际使用的窗口大小（奇数）
    static int16_t count = 0;                // 已存入的有效样本数（≤ cur_size）
    static int16_t pos = 0;                  // 下一个写入位置

    int16_t i, j, temp;
    int16_t tmp_buf[MEDIAN_MAX_WINDOW];      // 用于排序的临时缓冲区

    // 参数约束：确保为奇数，且不越界
    if (window_size > MEDIAN_MAX_WINDOW) window_size = MEDIAN_MAX_WINDOW;
    if ((window_size & 1) == 0) window_size++;

    // 若窗口大小发生变化，则重置滤波器状态
    if (window_size != cur_size) {
        cur_size = window_size;
        count = 0;
        pos = 0;
        // 缓冲区无需清零，未满时不会使用无效数据
    }

    // 将新样本存入环形缓冲区
    buf[pos] = dat;
    pos = (pos + 1) % cur_size;
    if (count < cur_size) count++;

    // 将缓冲区中有效的样本复制到临时数组（按时间顺序，但排序不需要顺序）
    for (i = 0; i < count; i++) {
        tmp_buf[i] = buf[i];
    }

    // 冒泡排序（升序）
    for (j = 0; j < count - 1; j++) {
        for (i = 0; i < count - j - 1; i++) {
            if (tmp_buf[i] > tmp_buf[i + 1]) {
                temp = tmp_buf[i];
                tmp_buf[i] = tmp_buf[i + 1];
                tmp_buf[i + 1] = temp;
            }
        }
    }

    // 返回中位数（注意：当 count 为偶数时，取中间偏左的值，与奇数窗口行为一致）
    return tmp_buf[(count - 1) / 2];
}


/**
 * 三数中值滤波（纯算法）
 * @param a 第一个样本值
 * @param b 第二个样本值
 * @param c 第三个样本值
 * @return  三个数中的中位数值
 */
int16_t Median_Filter_Of_3(int16_t a, int16_t b, int16_t c)
{
    if (a > b) { int16_t t = a; a = b; b = t; }
    if (b > c) { int16_t t = b; b = c; c = t; }
    if (a > b) { int16_t t = a; a = b; b = t; }
    return b;
}


// 最大窗口大小（根据内存调整，建议为2的幂以便用移位优化）
#define ARITH_MAX_WINDOW  8

/**
 * 算术平均滤波（滑动平均）
 * @param new_sample  新采集的样本值
 * @return            当前窗口内的算术平均值
 */
int16_t Arithmetic_Filter(int16_t new_sample)
{
    static int16_t buf[ARITH_MAX_WINDOW];
    static int16_t sum = 0;
    static uint8_t idx = 0;
    static uint8_t cnt = 0;      // 已填充的样本数（窗口未满时用于正确平均）

    // 更新环形缓冲区：减去即将被覆盖的旧值，加上新值
    sum = sum - buf[idx] + new_sample;
    buf[idx] = new_sample;
    idx = (idx + 1) % ARITH_MAX_WINDOW;

    // 窗口未满时，实际有效样本数为 cnt+1
    if (cnt < ARITH_MAX_WINDOW) {
        cnt++;
    }

    // 计算平均值（整数除法，自动截断）
    // 若希望四舍五入，可改为 (sum + cnt/2) / cnt
    return (int16_t)(sum / cnt);
}





