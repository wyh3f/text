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

/**
 * 项目名称：EPID（嵌入式比例-积分-微分（PID）控制器）
 * 语义版本：`EPID_LIB_VERSION`
 * 版本日期（ISO-8601）：2020-09-25
 * C 标准：C99（ISO/IEC 9899:1999）或更高版本。
 *
 * 描述：
 * 本库为“C 型 PID 控制器”的便携实现，适用于宿主环境和独立 C 环境，
 * 提供灵活的 API，允许使用第三方外部和/或内部滤波器以实现更好的控制，
 * 并支持错误和异常处理。
 *
 * 简要公式 [*]：
 * `P[k] = Kp * (x[k-1] - x[k])`
 * `I[k] = Ki * e[k] = Ki * (SP - x[k])`
 * `D[k] = Kd * (2*x[k-1] - x[k-2] - x[k])`
 * `y[k] = y[k-1] + delta[k] = y[k-1] + P[k] + I[k] + D[k]`
 * `x`：被测量的过程变量（PV）。
 * `y`：控制变量（CV/CO）。
 *
 * [*] 参考文献：D. M. Auslander, Y. Takahashi and M. Tomizuka,
 * "Direct digital process control: Practice and algorithms for microprocessor
 * application," in Proceedings of the IEEE, vol. 66, no. 2, pp. 199-208, Feb. 1978,
 * doi: 10.1109/PROC.1978.10870.
 */


#ifndef EPID_H
#define EPID_H 1


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h> /* 用于 NULL 定义 */

/* 开关宏：启用 `EPID_FEATURE_VALID_FLT` 功能，用于检测浮点错误 */
#if 1
# define EPID_FEATURE_VALID_FLT 1 /* 检查浮点溢出或非数 */
/* 需要 `isfinite(), isnan()` 函数，这些函数由 `math.h` 提供 */
# include <math.h>
#endif

/* API 和行为语义版本号 */
#define EPID_LIB_VERSION "1.1.3"

/* 辅助定义，便于将来切换数据类型（整数、浮点、双精度） */
#define EPID_FP_ZERO 0.0f
#define EPID_FP_ONE 1.0f

/* 错误码定义，类型为 `epid_info_t` */
#define EPID_ERR_INIT (0U) /* 初始化错误 */
#define EPID_ERR_FLT  (1U) /* 浮点运算错误（如 NaN、Inf） */
#define EPID_ERR_NONE (2U) /* 无错误 */


typedef uint_fast8_t epid_info_t; /* 用于错误标志的无符号快速整数类型 */

/**
 * @brief PID 控制器上下文结构体
 *
 * 存储控制器参数、状态和中间计算结果。
 * 用户通常只需要关心 `kp, ki, kd` 的设定，以及 `y_out` 的输出值。
 */
typedef struct {
    /* 控制器参数 */
    float kp; /* 比例增益常数 Kp */
    float ki; /* 积分增益常数 Ki */
    float kd; /* 微分增益常数 Kd */

    /* 控制器状态（历史测量值） */
    float xk_1; /* 上一次过程变量 PV[k-1] */
    float xk_2; /* 上上次过程变量 PV[k-2]（用于微分项） */

    /* 控制器中间计算结果 */
    float p_term; /* 比例项计算值 P[k] */
    float i_term; /* 积分项计算值 I[k] */
    float d_term; /* 微分项计算值 D[k] */

    float y_out; /* 控制器输出（控制变量） y[k] = y[k-1] + delta[k] */
} epid_t;

/**
 * @brief 一阶低通滤波器（LPF）上下文结构体
 *
 * 用于对测量值或设定值进行平滑滤波，减少噪声对微分项的影响。
 */
typedef struct {
    float smoothing_factor; /* 平滑因子，取值范围 0 < a < 1 */
    float y;                /* 滤波后的输出值 y[k] = FILTER(x[k]) */
} epid_lpf_t;


/**
 * @brief 通过直接指定增益来初始化或重置 PID 上下文
 *
 * 设置初始测量值历史（xk_1, xk_2）和上次输出（y_previous），
 * 并直接赋值比例、积分、微分增益。
 *
 * @param ctx       指向 epid_t 上下文的指针
 * @param xk_1      上一次测量值 PV[k-1]
 * @param xk_2      上上次测量值 PV[k-2]（用于微分项）
 * @param y_previous 上一次控制输出 CV[k-1]
 * @param kp        比例增益 Kp
 * @param ki        积分增益 Ki
 * @param kd        微分增益 Kd
 *
 * @note 约束条件：
 *       - {kp, ki, kd} 不能为负，且 kp、ki 不能为零（kd 可以为 0，表示 PI 控制）
 *       - 所有浮点参数不能是 NaN 或 Inf
 *
 * @return 错误码：
 *         - `EPID_ERR_NONE` 成功
 *         - `EPID_ERR_INIT` 初始化参数非法
 *         - `EPID_ERR_FLT`  浮点异常（若启用检查）
 */
epid_info_t epid_init(epid_t *ctx,
                      float xk_1, float xk_2, float y_previous,
                      float kp, float ki, float kd);


/**
 * @brief 通过比例增益和时间常数来初始化或重置 PID 上下文
 *
 * 根据积分时间 Ti 和微分时间 Td 以及采样周期 Ts，自动计算 Ki 和 Kd：
 *   `Ki = (Kp * Ts) / Ti`
 *   `Kd = Kp * (Td / Ts)`
 * 这种初始化方式更符合工程习惯，通常使用 Ti（积分时间常数）和 Td（微分时间常数）。
 *
 * @param ctx          指向 epid_t 上下文的指针
 * @param xk_1         上一次测量值 PV[k-1]
 * @param xk_2         上上次测量值 PV[k-2]（用于微分项）
 * @param y_previous   上一次控制输出 CV[k-1]
 * @param kp           比例增益 Kp
 * @param ti           积分时间常数 Ti（单位：时间），Ti > 0
 * @param td           微分时间常数 Td（单位：时间），Td >= 0（0 表示不使用微分）
 * @param sample_period 采样周期 Ts（单位：时间），Ts > 0
 *
 * @note 约束条件：
 *       - {kp, ti, td, sample_period} 不能为负，且 ti 和 sample_period 必须 > 0
 *       - 所有浮点参数不能是 NaN 或 Inf
 *
 * @return 错误码同 `epid_init()`
 */
epid_info_t epid_init_T(epid_t *ctx,
                        float xk_1, float xk_2, float y_previous,
                        float kp, float ti, float td,
                        float sample_period);


/**
 * @brief 执行 C 型 PI 控制器计算，更新比例项 P[k] 和积分项 I[k]
 *
 * 公式：
 *   `P[k] = Kp * (x[k-1] - x[k])`    （注意：这里使用测量值的变化，而非误差）
 *   `I[k] = Ki * (SP - x[k])`        （误差 e = SP - 测量值）
 * 调用后，`ctx->p_term` 和 `ctx->i_term` 被更新。
 * 此函数不更新输出 y_out，需要后续调用 `epid_pi_sum()` 完成累加和限幅。
 *
 * @param ctx      指向 epid_t 上下文的指针
 * @param setpoint 期望设定值（SP）
 * @param measure  当前测量值（PV）
 *
 * @note 本函数不包含微分项，若需要 D 项请使用 `epid_pid_calc()`
 */
void epid_pi_calc(epid_t *ctx, float setpoint, float measure);


/**
 * @brief 执行 C 型 PID 控制器计算，更新比例项 P[k]、积分项 I[k] 和微分项 D[k]
 *
 * 公式：
 *   `P[k] = Kp * (x[k-1] - x[k])`
 *   `I[k] = Ki * (SP - x[k])`
 *   `D[k] = Kd * (2*x[k-1] - x[k-2] - x[k])`
 * 调用后，`ctx->p_term`、`ctx->i_term` 和 `ctx->d_term` 被更新。
 * 之后需调用 `epid_pid_sum()` 来累加并限幅输出。
 *
 * @param ctx      指向 epid_t 上下文的指针
 * @param setpoint 期望设定值（SP）
 * @param measure  当前测量值（PV）
 *
 * @warning 微分项对测量噪声敏感，建议先对测量值进行低通滤波。
 */
void epid_pid_calc(epid_t *ctx, float setpoint, float measure);


/**
 * @brief 将 PI 控制器的增量累加到输出上，并进行限幅
 *
 * 公式：`y[k] = y[k-1] + P[k] + I[k]`
 * 如果 `ctx->y_out` 之前是 NaN（例如初始化时未正确处理），则累加结果可能仍为 NaN，
 * 因此建议在初始化后检查错误码。
 *
 * @param ctx     指向 epid_t 上下文的指针
 * @param out_min 输出下限（例如 0.0）
 * @param out_max 输出上限（例如 1.0 或 100.0）
 */
void epid_pi_sum(epid_t *ctx, float out_min, float out_max);


/**
 * @brief 将 PID 控制器的增量累加到输出上，并进行限幅
 *
 * 公式：`y[k] = y[k-1] + P[k] + I[k] + D[k]`
 * 同样，若之前输出为 NaN，结果可能异常，建议检查错误。
 *
 * @param ctx     指向 epid_t 上下文的指针
 * @param out_min 输出下限
 * @param out_max 输出上限
 */
void epid_pid_sum(epid_t *ctx, float out_min, float out_max);


/**
 * @brief 对积分项 I[k] 进行限幅（抗积分饱和）
 *
 * 在调用 `epid_pi_calc()` 或 `epid_pid_calc()` 之后，使用此函数限制 `i_term`
 * 的大小，防止积分项过大导致输出饱和后难以恢复。
 *
 * @param ctx   指向 epid_t 上下文的指针
 * @param i_min 积分项允许的最小值（通常为负值）
 * @param i_max 积分项允许的最大值（通常为正值）
 */
void epid_util_ilim(epid_t *ctx, float i_min, float i_max);


/**
 * @brief 初始化一阶低通滤波器上下文
 *
 * 滤波器采用指数加权移动平均（EMA），即一阶 IIR：
 *   `y[0] = a * x_0`
 *   其中 `a` 为平滑因子，取值范围 (0, 1)。
 *
 * @param ctx              指向 epid_lpf_t 上下文的指针
 * @param smoothing_factor 平滑因子 a（0 < a < 1）
 * @param x_0              初始输入值 x[0]
 *
 * @note 平滑因子 a 可依据截止频率 fc 计算：
 *       `a = (2*PI*Ts*fc) / (2*PI*Ts*fc + 1)`
 *
 * @return 错误码同 `epid_init()`
 */
epid_info_t epid_util_lpf_init(epid_lpf_t *ctx, float smoothing_factor, float x_0);


/**
 * @brief 对输入应用一阶低通滤波
 *
 * 公式：`y[k] = y[k-1] + a * (x[k] - y[k-1])`
 * 其中 a 为平滑因子，由初始化时设定。
 * 常用于对测量值或微分项进行滤波，降低噪声。
 *
 * @param ctx   指向 epid_lpf_t 上下文的指针
 * @param input 当前输入值 x[k]
 */
void epid_util_lpf_calc(epid_lpf_t *ctx, float input);


#ifdef __cplusplus
}
#endif

#endif /* EPID_H */

