/**
 * @file algorithms_filter.c
 * @brief 数字滤波算法库实现
 */

#include "algorithms_filter.h"


/*==================== 1. 限幅滤波 ====================*/
void LimitChange_Filter_Init(LimitChange_Filter_CTX *ctx, int16_t initial_value)
{
    ctx->last_valid = initial_value;
}

int16_t LimitChange_Filter_Update(LimitChange_Filter_CTX *ctx, int16_t new_value, int16_t max_change)
{
    int32_t diff = (int32_t)new_value - (int32_t)ctx->last_valid;
    if (diff < 0) diff = -diff;
    if (diff > (int32_t)max_change) {
        return ctx->last_valid;
    } else {
        ctx->last_valid = new_value;
        return new_value;
    }
}

/*==================== 2. 中位值滤波 ====================*/
void Median_Filter_Init(Median_Filter_CTX *ctx, int16_t window_size)
{
    if (window_size > MEDIAN_MAX_WINDOW) window_size = MEDIAN_MAX_WINDOW;
    if ((window_size & 1) == 0) window_size++;    // 强制奇数
    ctx->cur_size = window_size;
    ctx->count = 0;
    ctx->pos = 0;
}

int16_t Median_Filter_Update(Median_Filter_CTX *ctx, int16_t dat)
{
    int16_t tmp_buf[MEDIAN_MAX_WINDOW];
    int16_t i, j, temp;

    // 存入新样本
    ctx->buf[ctx->pos] = dat;
    ctx->pos = (ctx->pos + 1) % ctx->cur_size;
    if (ctx->count < ctx->cur_size) {
        ctx->count++;
    }

    // 复制有效数据到临时缓冲区
    for (i = 0; i < ctx->count; i++) {
        tmp_buf[i] = ctx->buf[i];
    }

    // 冒泡排序（升序）
    for (j = 0; j < ctx->count - 1; j++) {
        for (i = 0; i < ctx->count - j - 1; i++) {
            if (tmp_buf[i] > tmp_buf[i + 1]) {
                temp = tmp_buf[i];
                tmp_buf[i] = tmp_buf[i + 1];
                tmp_buf[i + 1] = temp;
            }
        }
    }

    // 返回中位数（当 count 为偶数时取中间偏左）
    return tmp_buf[(ctx->count - 1) / 2];
}

/*==================== 3. 三数中值滤波（无状态） ====================*/
int16_t Median_Filter_Of_3(int16_t a, int16_t b, int16_t c)
{
    if (a > b) { int16_t t = a; a = b; b = t; }
    if (b > c) { int16_t t = b; b = c; c = t; }
    if (a > b) { int16_t t = a; a = b; b = t; }
    return b;
}

/*==================== 4. 算术平均滤波 ====================*/
void Arithmetic_Filter_Init(Arithmetic_Filter_CTX *ctx, uint8_t window_size)
{
    if (window_size == 0) window_size = 1;
    if (window_size > ARITH_MAX_WINDOW) window_size = ARITH_MAX_WINDOW;
    ctx->window_size = window_size;
    ctx->sum = 0;
    ctx->idx = 0;
    ctx->cnt = 0;
    // 清空缓冲区（可选，避免残留数据影响首次平均）
    for (uint8_t i = 0; i < ARITH_MAX_WINDOW; i++) {
        ctx->buf[i] = 0;
    }
}

int16_t Arithmetic_Filter_Update(Arithmetic_Filter_CTX *ctx, int16_t new_sample)
{
    // 如果窗口已满，减去即将被覆盖的旧值
    if (ctx->cnt == ctx->window_size) {
        ctx->sum -= ctx->buf[ctx->idx];
    } else {
        ctx->cnt++;
    }

    // 写入新值
    ctx->buf[ctx->idx] = new_sample;
    ctx->sum += new_sample;
    ctx->idx = (ctx->idx + 1) % ctx->window_size;

    // 整数平均（四舍五入）
    return (int16_t)((ctx->sum + ctx->cnt / 2) / ctx->cnt);
}

/*==================== 5. 一阶低通滤波 ====================*/
void LowPass_Filter_Init(LowPass_Filter_CTX *ctx, int16_t initial_output)
{
    ctx->last_output = initial_output;
}

int16_t LowPass_Filter_Update(LowPass_Filter_CTX *ctx, int16_t new_sample, uint16_t alpha)
{
    int32_t output = (int32_t)alpha * new_sample + (int32_t)(256 - alpha) * ctx->last_output;
    output = (output + 128) / 256;   // 四舍五入
    ctx->last_output = (int16_t)output;
    return ctx->last_output;
}

/*==================== 6. 一阶高通滤波 ====================*/
void HighPass_Filter_Init(HighPass_Filter_CTX *ctx, int16_t initial_lowpass)
{
    ctx->lowpass_last = initial_lowpass;
}

int16_t HighPass_Filter_Update(HighPass_Filter_CTX *ctx, int16_t new_sample, uint16_t alpha)
{
    int32_t lowpass = (int32_t)alpha * new_sample + (int32_t)(256 - alpha) * ctx->lowpass_last;
    lowpass = (lowpass + 128) / 256;
    ctx->lowpass_last = (int16_t)lowpass;
    return new_sample - (int16_t)lowpass;
}

/*==================== 7. 加权递推平均滤波 ====================*/
void WeightedAverage_Filter_Init(WeightedAverage_Filter_CTX *ctx, uint8_t window_size, const uint8_t *weights)
{
    if (window_size > WAVG_MAX_WINDOW) window_size = WAVG_MAX_WINDOW;
    if (window_size == 0) window_size = 1;
    ctx->window_size = window_size;
    ctx->idx = 0;
    ctx->filled = 0;
    ctx->weight_sum = 0;

    for (uint8_t i = 0; i < window_size; i++) {
        ctx->weights[i] = weights[i];
        ctx->weight_sum += weights[i];
        ctx->buffer[i] = 0;   // 清空缓冲区
    }
}

int16_t WeightedAverage_Filter_Update(WeightedAverage_Filter_CTX *ctx, int16_t new_sample)
{
    // 存入新样本
    ctx->buffer[ctx->idx] = new_sample;
    ctx->idx = (ctx->idx + 1) % ctx->window_size;
    if (ctx->filled < ctx->window_size) {
        ctx->filled++;
    }

    // 加权求和（从最新到最旧）
    int32_t sum_wx = 0;
    for (uint8_t i = 0; i < ctx->filled; i++) {
        // 计算样本位置：最新样本在 (idx-1) 处，依次向前
        uint8_t pos = (ctx->idx - 1 - i + ctx->window_size) % ctx->window_size;
        sum_wx += (int32_t)ctx->buffer[pos] * ctx->weights[i];
    }

    // 计算实际有效的权重总和（窗口未满时只使用前 filled 个权重）
    uint8_t sum_w = 0;
    for (uint8_t i = 0; i < ctx->filled; i++) {
        sum_w += ctx->weights[i];
    }

    // 四舍五入返回
    return (int16_t)((sum_wx + sum_w / 2) / sum_w);
}




/**
 * @brief 数字滤波算法测试程序（仅用于调试）
 * 实际嵌入式应用中可移除此文件或改为硬件输出（如DAC、串口）
 */



/*==================== 主测试入口 ====================*/
/**
 * @brief 测试所有数字滤波算法（打印输出）
 * 实际嵌入式应用中可移除此函数或改为硬件输出（如DAC、串口）
 */
void test_all_filters(void)
{
    printf("\n========================================\n");
    /*==================== 1. 限幅滤波测试 ====================*/
    printf("---------------\n");
    printf("test_limit_filter\n");

    // 第一组数据：平稳信号 + 正向毛刺 + 负向跳变
    {
        int16_t raw[] = {100, 102, 101, 105, 200, 103, 101, 99, 50, 102, 104};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        LimitChange_Filter_CTX ctx;
        LimitChange_Filter_Init(&ctx, raw[0]);

        printf("Group1 (limit ±20): raw -> filtered\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = LimitChange_Filter_Update(&ctx, raw[i], 20);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    // 第二组数据：阶跃上升，限制变化速率
    {
        int16_t raw[] = {50, 51, 52, 53, 150, 151, 152, 153, 154, 155};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        LimitChange_Filter_CTX ctx;
        LimitChange_Filter_Init(&ctx, raw[0]);

        printf("\nGroup2 (limit ±5): raw -> filtered\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = LimitChange_Filter_Update(&ctx, raw[i], 5);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    /*==================== 2. 中位值滤波测试 ====================*/
    printf("---------------\n");
    printf("test_median_filter\n");

    // 第一组：含脉冲干扰，窗口=5
    {
        int16_t raw[] = {20, 22, 21, 23, 1000, 22, 20, 19, 21, 20, 19, -500, 21, 22};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        Median_Filter_CTX ctx;
        Median_Filter_Init(&ctx, 5);

        printf("Group1 (window=5): raw -> median\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = Median_Filter_Update(&ctx, raw[i]);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    // 第二组：平稳噪声，窗口=7
    {
        int16_t raw[] = {100, 102, 101, 105, 200, 103, 101, 99, 102, 104, 101, 103, 300, 102};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        Median_Filter_CTX ctx;
        Median_Filter_Init(&ctx, 7);

        printf("\nGroup2 (window=7): raw -> median\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = Median_Filter_Update(&ctx, raw[i]);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    /*==================== 3. 三数中值滤波测试 ====================*/
    printf("---------------\n");
    printf("test_median3_filter\n");

    printf("Group1: (10, 500, 12) -> %d\n", Median_Filter_Of_3(10, 500, 12));
    printf("        (5, 3, 9)     -> %d\n", Median_Filter_Of_3(5, 3, 9));
    printf("Group2: (-1, -5, -2)  -> %d\n", Median_Filter_Of_3(-1, -5, -2));
    printf("        (100,100,100) -> %d\n", Median_Filter_Of_3(100, 100, 100));

    /*==================== 4. 算术平均滤波测试 ====================*/
    printf("---------------\n");
    printf("test_arithmetic_filter\n");

    // 第一组：平稳+毛刺，窗口=4
    {
        int16_t raw[] = {100, 102, 101, 105, 200, 103, 101, 99, 102, 104, 25};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        Arithmetic_Filter_CTX ctx;
        Arithmetic_Filter_Init(&ctx, 4);

        printf("Group1 (window=4): raw -> average\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = Arithmetic_Filter_Update(&ctx, raw[i]);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    // 第二组：阶跃信号，窗口=8
    {
        int16_t raw[] = {50, 51, 52, 53, 150, 151, 152, 153, 154, 155, 156, 157};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        Arithmetic_Filter_CTX ctx;
        Arithmetic_Filter_Init(&ctx, 8);

        printf("\nGroup2 (window=8): raw -> average\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = Arithmetic_Filter_Update(&ctx, raw[i]);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    /*==================== 5. 一阶低通滤波测试 ====================*/
    printf("---------------\n");
    printf("test_lowpass_filter\n");

    // 第一组：平稳+噪声，alpha=32
    {
        int16_t raw[] = {100, 102, 101, 105, 200, 103, 101, 99, 102, 104, 101};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        LowPass_Filter_CTX ctx;
        LowPass_Filter_Init(&ctx, raw[0]);

        printf("Group1 (alpha=32): raw -> lowpass\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = LowPass_Filter_Update(&ctx, raw[i], 32);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    // 第二组：阶跃响应，alpha=128
    {
        int16_t raw[] = {50, 51, 52, 53, 150, 151, 152, 153, 154};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        LowPass_Filter_CTX ctx;
        LowPass_Filter_Init(&ctx, raw[0]);

        printf("\nGroup2 (alpha=128): raw -> lowpass\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = LowPass_Filter_Update(&ctx, raw[i], 128);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    /*==================== 6. 一阶高通滤波测试 ====================*/
    printf("---------------\n");
    printf("test_highpass_filter\n");

    // 第一组：缓慢变化趋势，alpha=32
    {
        int16_t raw[] = {100, 102, 104, 103, 101, 99, 98, 100, 102, 104, 106, 105};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        HighPass_Filter_CTX ctx;
        HighPass_Filter_Init(&ctx, raw[0]);

        printf("Group1 (alpha=32): raw -> highpass\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = HighPass_Filter_Update(&ctx, raw[i], 32);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    // 第二组：阶跃+噪声，alpha=64
    {
        int16_t raw[] = {50, 51, 52, 53, 150, 151, 152, 153, 154, 155};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        HighPass_Filter_CTX ctx;
        HighPass_Filter_Init(&ctx, raw[0]);

        printf("\nGroup2 (alpha=64): raw -> highpass\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = HighPass_Filter_Update(&ctx, raw[i], 64);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    /*==================== 7. 加权递推平均滤波测试 ====================*/
    printf("---------------\n");
    printf("test_weighted_filter\n");

    // 第一组：窗口4，权重 {4,3,2,1}
    {
        uint8_t weights[4] = {4, 3, 2, 1};
        int16_t raw[] = {100, 102, 101, 105, 200, 103, 101, 99, 102, 104, 25};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        WeightedAverage_Filter_CTX ctx;
        WeightedAverage_Filter_Init(&ctx, 4, weights);

        printf("Group1 (weights 4,3,2,1): raw -> weighted\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = WeightedAverage_Filter_Update(&ctx, raw[i]);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    // 第二组：窗口5，权重 {5,4,3,2,1}
    {
        uint8_t weights[5] = {5, 4, 3, 2, 1};
        int16_t raw[] = {50, 51, 52, 53, 150, 151, 152, 153, 154, 155, 156};
        int16_t len = sizeof(raw) / sizeof(raw[0]);
        WeightedAverage_Filter_CTX ctx;
        WeightedAverage_Filter_Init(&ctx, 5, weights);

        printf("\nGroup2 (weights 5,4,3,2,1): raw -> weighted\n");
        for (uint16_t i = 0; i < len; i++) {
            int16_t out = WeightedAverage_Filter_Update(&ctx, raw[i]);
            printf("%4d -> %4d\n", raw[i], out);
        }
    }

    printf("---------------\n");
    printf("test_all_filters end\n");
}

