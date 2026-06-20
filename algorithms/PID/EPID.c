/* SPDX-License-Identifier: ISC */
/**
 * 版权所有 (c) 2020 Abderraouf Adjal
 *
 * 特此授予任何获得本软件及相关文档（“软件”）副本的人免费使用、
 * 复制、修改、合并、出版、分发、再许可和/或销售软件的权利，
 * 但须满足以下条件：
 * - 上述版权声明和本许可声明应出现在所有副本中。
 *
 * 本软件按“原样”提供，作者不作任何明示或暗示的保证，包括但不限于
 * 适销性和特定用途适用性的暗示保证。在任何情况下，作者均不对任何
 * 特殊的、直接的、间接的或后果性的损害，或任何因使用、数据或利润
 * 损失（无论是合同、疏忽还是其他侵权行为）引起的损害承担责任，
 * 即使已被告知此类损害的可能性。
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "EPID.h"

/*==============================================================================
 *  PID 初始化（直接指定增益）
 *============================================================================*/

/**
 * @brief 初始化或重置 PID 上下文（直接指定比例、积分、微分增益）
 *
 * 设置初始历史测量值（xk_1, xk_2）和上一次输出值（y_previous），
 * 并直接将 Kp, Ki, Kd 存入上下文。
 *
 * @param ctx        指向 epid_t 结构的指针
 * @param xk_1       上一次测量值 PV[k-1]
 * @param xk_2       上上次测量值 PV[k-2]（用于微分项）
 * @param y_previous 上一次控制输出 CV[k-1]
 * @param kp         比例增益 Kp（必须 > 0）
 * @param ki         积分增益 Ki（必须 > 0）
 * @param kd         微分增益 Kd（可以 ≥ 0，0 表示无微分）
 *
 * @retval EPID_ERR_NONE  初始化成功
 * @retval EPID_ERR_INIT  参数非法（如指针为空、增益为负等）
 * @retval EPID_ERR_FLT   浮点异常（如传入 NaN/Inf）
 */
epid_info_t epid_init(epid_t *ctx,
                      float xk_1, float xk_2, float y_previous,
                      float kp, float ki, float kd)
{
#ifdef EPID_FEATURE_VALID_FLT
    /* 检查所有浮点参数是否为有限值（非 NaN/Inf） */
    if ((isfinite(xk_1) == 0)
     || (isfinite(xk_2) == 0)
     || (isfinite(y_previous) == 0)
     || (isfinite(kp) == 0)
     || (isfinite(ki) == 0)
     || (isfinite(kd) == 0) /* kd=0 允许，用于 PI 控制器 */
    ) {
        return EPID_ERR_FLT;
    }
#endif

    /* 参数合法性检查 */
    if ((ctx == NULL)
     || (kp <= EPID_FP_ZERO)  /* Kp 必须大于 0 */
     || (ki <= EPID_FP_ZERO)  /* Ki 必须大于 0 */
     || (kd < EPID_FP_ZERO)   /* Kd 可以等于 0 */
    ) {
        return EPID_ERR_INIT;
    }

    /* 设置历史状态（供后续公式使用） */
    ctx->xk_1 = xk_1; /* 保存 x[k-1] */
    ctx->xk_2 = xk_2; /* 保存 x[k-2]（供微分项使用） */
    ctx->y_out = y_previous; /* 保存 y[k-1] */

    /* 直接赋值增益常数 */
    ctx->kp = kp;
    ctx->ki = ki;
    ctx->kd = kd;

    return EPID_ERR_NONE;
}

/*==============================================================================
 *  PID 初始化（通过时间常数，更符合工程习惯）
 *============================================================================*/

/**
 * @brief 通过比例增益和时间常数初始化 PID
 *
 * 根据积分时间 Ti、微分时间 Td 和采样周期 Ts，自动计算 Ki 和 Kd：
 *   `Ki = (Kp * Ts) / Ti`
 *   `Kd = Kp * (Td / Ts)`
 *
 * 这种初始化方式避免直接记忆 Ki、Kd 的数值，调节 Ti 和 Td 更直观。
 *
 * @param ctx           指向 epid_t 结构的指针
 * @param xk_1          上一次测量值 PV[k-1]
 * @param xk_2          上上次测量值 PV[k-2]（用于微分项）
 * @param y_previous    上一次控制输出 CV[k-1]
 * @param kp            比例增益 Kp（必须 > 0）
 * @param ti            积分时间常数 Ti（单位：秒，必须 > 0）
 * @param td            微分时间常数 Td（单位：秒，可以 ≥ 0，0 表示无微分）
 * @param sample_period 采样周期 Ts（单位：秒，必须 > 0）
 *
 * @retval 同 epid_init()
 */
epid_info_t epid_init_T(epid_t *ctx,
                        float xk_1, float xk_2, float y_previous,
                        float kp, float ti, float td,
                        float sample_period)
{
#ifdef EPID_FEATURE_VALID_FLT
    /* 检查 ti 和采样周期是否为有限值 */
    if ((isfinite(ti) == 0)
     || (isfinite(sample_period) == 0)
    ) {
        return EPID_ERR_FLT;
    }
#endif

    /* 参数合法性检查 */
    if ((ti <= EPID_FP_ZERO)           /* Ti 必须 > 0 */
     || (td <  EPID_FP_ZERO)           /* Td 可以 = 0 */
     || (sample_period <= EPID_FP_ZERO) /* Ts 必须 > 0 */
    ) {
        return EPID_ERR_INIT;
    }

    /* 计算积分增益：Ki = Kp * Ts / Ti */
    const float ki = (kp * sample_period) / ti;
    /* 计算微分增益：Kd = Kp * Td / Ts */
    const float kd = kp * (td / sample_period);

    /* 调用直接增益初始化函数完成最终设置 */
    return epid_init(ctx,
                     xk_1, xk_2, y_previous,
                     kp, ki, kd);
}

/*==============================================================================
 *  PI 控制器计算（只计算 P 和 I 项）
 *============================================================================*/

/**
 * @brief 执行 C 型 PI 控制计算，更新比例项 P[k] 和积分项 I[k]
 *
 * 计算公式：
 *   `P[k] = Kp * (x[k-1] - x[k])`   （基于测量值的变化，而非误差变化）
 *   `I[k] = Ki * (SP - x[k])`       （误差 = 设定值 - 测量值）
 *
 * 本函数只更新 ctx->p_term 和 ctx->i_term，不修改 ctx->y_out。
 * 累加输出和限幅需后续调用 epid_pi_sum()。
 *
 * @param ctx       PID 上下文指针
 * @param setpoint  期望设定值 SP
 * @param measure   当前测量值 PV[k]
 */
void epid_pi_calc(epid_t *ctx, float setpoint, float measure)
{
    /* 比例项：P[k] = Kp * (x[k-1] - x[k]) */
    ctx->p_term = ctx->kp * (ctx->xk_1 - measure);
    /* 积分项：I[k] = Ki * (SP - x[k]) */
    ctx->i_term = ctx->ki * (setpoint - measure);

    /* 更新历史测量值：当前值成为下一次的 x[k-1] */
    ctx->xk_1 = measure;
}

/*==============================================================================
 *  PID 控制器计算（计算 P、I、D 三项）
 *============================================================================*/

/**
 * @brief 执行 C 型 PID 控制计算，更新 P[k]、I[k] 和 D[k]
 *
 * 计算公式：
 *   `P[k] = Kp * (x[k-1] - x[k])`
 *   `I[k] = Ki * (SP - x[k])`
 *   `D[k] = Kd * (2*x[k-1] - x[k-2] - x[k])`
 *
 * 微分项采用二阶近似，相当于对测量值变化率再求变化率。
 * 本函数更新三项值，但不更新 y_out，需后续调用 epid_pid_sum()。
 *
 * @warning 微分项对测量噪声敏感，建议对测量值进行低通滤波后再输入。
 *
 * @param ctx       PID 上下文指针
 * @param setpoint  期望设定值 SP
 * @param measure   当前测量值 PV[k]
 */
void epid_pid_calc(epid_t *ctx, float setpoint, float measure)
{
    /* 先计算未乘 Kp 的 P 项（后面再乘） */
    ctx->p_term = ctx->xk_1 - measure;
    /* 微分项：D[k] = Kd * (x[k-1] + (x[k-1]-x[k]) - x[k-2]) */
    ctx->d_term = ctx->kd * (ctx->xk_1 + ctx->p_term - ctx->xk_2);
    /* 比例项乘以 Kp */
    ctx->p_term = ctx->kp * ctx->p_term;
    /* 积分项：I[k] = Ki * (SP - measure) */
    ctx->i_term = ctx->ki * (setpoint - measure);

    /* 更新历史测量值：将当前值保存为下一次的 x[k-1]，旧 x[k-1] 变为 x[k-2] */
    ctx->xk_2 = ctx->xk_1;   /* 保存 x[k-2] = 旧的 x[k-1] */
    ctx->xk_1 = measure;     /* 保存 x[k-1] = 当前测量值 */
}

/*==============================================================================
 *  PI 输出累加与限幅
 *============================================================================*/

/**
 * @brief 将 PI 控制器的增量累加到输出上，并进行限幅
 *
 * 公式：`y[k] = y[k-1] + P[k] + I[k]`
 * 使用前须先调用 epid_pi_calc() 更新 P、I 项。
 * 若输出超出 [out_min, out_max]，则被截断。
 *
 * @param ctx      PID 上下文指针
 * @param out_min  输出下限（例如 0.0）
 * @param out_max  输出上限（例如 1.0）
 */
void epid_pi_sum(epid_t *ctx, float out_min, float out_max)
{
#ifdef EPID_FEATURE_VALID_FLT
    /* 保存上一次输出，以备恢复 */
    const float y_prev = ctx->y_out;
#endif

    /* 累加增量：y[k] = y[k-1] + P[k] + I[k] */
    ctx->y_out += ctx->p_term + ctx->i_term;

#ifdef EPID_FEATURE_VALID_FLT
    /* 若累加后出现 NaN，则回退到上一次有效值 */
    if ((isnan(ctx->y_out) != 0)
     || (isnan(ctx->p_term) != 0)
     || (isnan(ctx->i_term) != 0)
    ) {
        ctx->y_out = y_prev;
    }
#endif

    /* 限幅输出到指定范围 */
    if (ctx->y_out > out_max) {
        ctx->y_out = out_max;
    }
    else if (ctx->y_out < out_min) {
        ctx->y_out = out_min;
    }
}

/*==============================================================================
 *  PID 输出累加与限幅（含微分项）
 *============================================================================*/

/**
 * @brief 将 PID 控制器的增量累加到输出上，并进行限幅
 *
 * 公式：`y[k] = y[k-1] + P[k] + I[k] + D[k]`
 * 使用前须先调用 epid_pid_calc() 更新 P、I、D 项。
 *
 * @param ctx      PID 上下文指针
 * @param out_min  输出下限
 * @param out_max  输出上限
 */
void epid_pid_sum(epid_t *ctx, float out_min, float out_max)
{
#ifdef EPID_FEATURE_VALID_FLT
    const float y_prev = ctx->y_out;
#endif

    /* 累加增量：y[k] = y[k-1] + P[k] + I[k] + D[k] */
    ctx->y_out += ctx->p_term + ctx->i_term + ctx->d_term;

#ifdef EPID_FEATURE_VALID_FLT
    /* 检查 NaN，若出现则回退 */
    if ((isnan(ctx->y_out) != 0)
     || (isnan(ctx->p_term) != 0)
     || (isnan(ctx->i_term) != 0)
     || (isnan(ctx->d_term) != 0)
    ) {
        ctx->y_out = y_prev;
    }
#endif

    /* 限幅输出 */
    if (ctx->y_out > out_max) {
        ctx->y_out = out_max;
    }
    else if (ctx->y_out < out_min) {
        ctx->y_out = out_min;
    }
}

/*==============================================================================
 *  积分项限幅（抗积分饱和）
 *============================================================================*/

/**
 * @brief 对积分项 I[k] 进行限幅（抗积分饱和）
 *
 * 在调用 epid_pi_calc() 或 epid_pid_calc() 之后，使用此函数限制
 * ctx->i_term 的绝对值，防止积分项过大导致输出饱和后难以退出。
 * 建议在累加（*_sum）之前调用。
 *
 * @param ctx    PID 上下文指针
 * @param i_min  积分项允许的最小值（通常为负值）
 * @param i_max  积分项允许的最大值（通常为正值）
 */
void epid_util_ilim(epid_t *ctx, float i_min, float i_max)
{
    if (ctx->i_term > i_max) {
        ctx->i_term = i_max;
    }
    else if (ctx->i_term < i_min) {
        ctx->i_term = i_min;
    }
}

/*==============================================================================
 *  一阶低通滤波器（EMA）初始化
 *============================================================================*/

/**
 * @brief 初始化一阶低通滤波器（指数加权移动平均）
 *
 * 滤波器传递函数：`y[k] = y[k-1] + a * (x[k] - y[k-1])`
 * 其中 a 为平滑因子（0 < a < 1），a 越大滤波效果越弱（响应越快）。
 * 初始输出 y[0] = a * x_0。
 *
 * @param ctx               滤波器上下文指针
 * @param smoothing_factor  平滑因子 a（必须介于 0 和 1 之间）
 * @param x_0               初始输入值 x[0]
 *
 * @retval EPID_ERR_NONE  成功
 * @retval EPID_ERR_INIT  参数非法（如指针为空或 a 不在 (0,1) 范围）
 * @retval EPID_ERR_FLT   浮点异常
 */
epid_info_t epid_util_lpf_init(epid_lpf_t *ctx, float smoothing_factor, float x_0)
{
#ifdef EPID_FEATURE_VALID_FLT
    if ((isfinite(smoothing_factor) == 0)
     || (isfinite(x_0) == 0)
    ) {
        return EPID_ERR_FLT;
    }
#endif

    if ((ctx == NULL)
     || (smoothing_factor <= EPID_FP_ZERO)
     || (smoothing_factor >= EPID_FP_ONE)
    ) {
        return EPID_ERR_INIT;
    }

    ctx->smoothing_factor = smoothing_factor;
    ctx->y = smoothing_factor * x_0;  /* 初始输出 */

    return EPID_ERR_NONE;
}

/*==============================================================================
 *  低通滤波器计算
 *============================================================================*/

/**
 * @brief 对输入应用一阶低通滤波（EMA）
 *
 * 更新滤波器输出：`y = y + a * (input - y)`
 * 可用于平滑测量值，或滤波微分项以抑制噪声。
 *
 * @param ctx   滤波器上下文指针
 * @param input 当前输入值 x[k]
 */
void epid_util_lpf_calc(epid_lpf_t *ctx, float input)
{
    const float y_prev = ctx->y;
    ctx->y = y_prev + ctx->smoothing_factor * (input - y_prev);
}

#ifdef __cplusplus
}
#endif




