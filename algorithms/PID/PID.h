#ifndef PID_H
#define PID_H

#include <stdint.h>

/**
 * @brief PID 控制器结构体
 * 
 * 本版本移除了采样周期 T 和微分滤波时间常数 tau。
 * 增益 Ki 和 Kd 已经包含了采样时间的影响（即 Ki 等效于原 Ki * T，Kd 等效于原 Kd / T），
 * 因此用户整定时需根据实际采样周期调整这些增益值。
 */
typedef struct {

    /* 控制器增益 */
    float Kp;               /**< 比例增益 */
    float Ki;               /**< 积分增益（已隐含采样周期，不再额外乘以 T） */
    float Kd;               /**< 微分增益（已隐含采样周期，不再额外除以 T） */

    /* 输出限幅 */
    float limMin;           /**< 控制器输出下限 */
    float limMax;           /**< 控制器输出上限 */

    /* 积分器限幅（抗积分饱和） */
    float limMinInt;        /**< 积分项下限 */
    float limMaxInt;        /**< 积分项上限 */

    /* 控制器内部状态（“记忆”） */
    float integrator;       /**< 积分累加器（存储误差积分值，尚未乘以 Ki） */
    float prevError;        /**< 上一次误差（用于梯形积分） */
    float differentiator;   /**< 微分项状态（此处直接存储最近一次微分值，无滤波） */
    float prevMeasurement;  /**< 上一次测量值（用于微分计算） */

    /* 控制器输出 */
    float out;              /**< 控制器输出值（限幅后） */

} PIDController;

/**
 * @brief 初始化 PID 控制器，将内部状态变量清零
 * 
 * 该函数必须在第一次调用 PIDController_Update() 之前执行。
 * 它不会修改用户的配置参数（Kp, Ki, Kd, limMin, limMax, limMinInt, limMaxInt），
 * 仅清空积分累加器、微分器状态、上一次误差和上一次测量值，并将输出置零。
 * 
 * @param pid 指向 PIDController 结构体的指针（必须已分配内存且配置好参数）
 */
void PIDController_Init(PIDController *pid);

/**
 * @brief 执行 PID 控制器的单次计算，返回控制输出
 * 
 * 该函数实现了位置式 PID 算法，包含以下特性：
 * - 比例项：基于误差的比例控制。
 * - 积分项：采用梯形积分（Tustin 近似），具有抗积分饱和（积分限幅）。
 * - 微分项：基于测量值微分（避免设定值突变引起的微分冲击），**无低通滤波**。
 * - 输出限幅：将最终输出限制在 [limMin, limMax] 范围内。
 * 
 * 注意：由于移除了采样周期 T，增益 Ki 和 Kd 需用户根据实际采样周期进行换算。
 * 例如，若原控制器（带 T）的增益为 Ki_old、Kd_old，则本版本应设为：
 *     Ki = Ki_old * T
 *     Kd = Kd_old / T
 * 
 * 调用该函数前必须确保：
 * - 已调用 PIDController_Init() 进行初始化。
 * - 已正确配置 PIDController 结构体的所有参数（Kp, Ki, Kd, limMin, limMax, limMinInt, limMaxInt）。
 * 
 * @param pid        指向 PIDController 结构体的指针，包含控制器参数和状态。
 * @param setpoint   期望的目标值（设定值）。
 * @param measurement 当前系统状态的测量值（反馈值）。
 * @return float     控制器输出值，已被限幅在 [limMin, limMax] 范围内。
 */
float PIDController_Update(PIDController *pid, float setpoint, float measurement);






/**
 * @brief 增量式 PID 控制器结构体
 * 
 * 增量式 PID 输出的是控制量的增量 Δu，实际输出 = 上一次输出 + Δu。
 * 该结构体保存了控制器参数、输出限幅以及历史误差和上一次输出值。
 */
typedef struct {
    /* 控制器参数（增益已隐含采样周期） */
    float Kp;       ///< 比例增益
    float Ki;       ///< 积分增益（已隐含采样周期）
    float Kd;       ///< 微分增益（已隐含采样周期）

    /* 输出限幅 */
    float limMin;   ///< 输出下限
    float limMax;   ///< 输出上限

    /* 状态变量（用于增量计算） */
    float prevError1;   ///< e(k-1)，即前一次误差
    float prevError2;   ///< e(k-2)，即前两次误差
    float prevOutput;   ///< u(k-1)，即上一次输出值
} IncrementalPID;

/**
 * @brief 初始化增量式 PID 控制器，清空状态变量
 * 
 * @param pid 指向 IncrementalPID 结构体的指针（必须已分配内存）
 */
void IncrementalPID_Init(IncrementalPID *pid);

/**
 * @brief 执行增量式 PID 单次计算，返回当前控制输出
 * 
 * 计算公式：
 *   Δu(k) = Kp * [e(k) - e(k-1)] + Ki * e(k) + Kd * [e(k) - 2*e(k-1) + e(k-2)]
 *   u(k) = u(k-1) + Δu(k)
 * 
 * 输出经过限幅，确保在 [limMin, limMax] 范围内。
 * 
 * @param pid         指向 IncrementalPID 结构体的指针
 * @param setpoint    目标设定值
 * @param measurement 当前测量值（反馈值）
 * @return float      限幅后的控制输出
 */
float IncrementalPID_Update(IncrementalPID *pid, float setpoint, float measurement);







#endif


