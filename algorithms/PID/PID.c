#include "PID.h"
#include <math.h>   // 用于 isnan, isinf

/**
 * @brief 初始化 PID 控制器，将内部状态变量清零
 * 
 * 该函数必须在第一次调用 PIDController_Update() 之前执行。
 * 它不会修改用户的配置参数（Kp, Ki, Kd, limMin, limMax, limMinInt, limMaxInt），
 * 仅清空积分累加器、微分器状态、上一次误差和上一次测量值，并将输出置零。
 * 
 * @param pid 指向 PIDController 结构体的指针（必须已分配内存且配置好参数）
 */
void PIDController_Init(PIDController *pid)
{
    /* 清空积分累加器，防止前一次控制遗留的积分值干扰 */
    pid->integrator = 0.0f;

    /* 清空上一次误差，梯形积分依赖此值，初始时为 0 */
    pid->prevError = 0.0f;

    /* 清空微分项状态，避免启动时突变 */
    pid->differentiator = 0.0f;

    /* 清空上一次测量值，微分计算需要此值，初始时设为 0 */
    pid->prevMeasurement = 0.0f;

    /* 清空控制器输出，确保启动时输出为安全值（通常为 0） */
    pid->out = 0.0f;
}

/**
 * @brief 执行 PID 控制器的单次计算，返回控制输出
 * 
 * 该函数实现了位置式 PID 算法，包含以下特性：
 * - 比例项：基于误差的比例控制。
 * - 积分项：采用梯形积分（Tustin 近似），具有抗积分饱和（积分限幅）。
 * - 微分项：基于测量值微分（避免设定值突变引起的微分冲击），无低通滤波。
 * - 输出限幅：将最终输出限制在 [limMin, limMax] 范围内。
 * 
 * 增益 Ki 和 Kd 已隐含采样周期，用户整定时需确保数值正确。
 * 
 * @param pid        指向 PIDController 结构体的指针，包含控制器参数和状态。
 * @param setpoint   期望的目标值（设定值）。
 * @param measurement 当前系统状态的测量值（反馈值）。
 * @return float     控制器输出值，已被限幅在 [limMin, limMax] 范围内。
 */
float PIDController_Update(PIDController *pid, float setpoint, float measurement)
{
    /* 检查输入是否有效（防止 NaN 或无穷大导致控制器失控） */
    if (isnan(setpoint) || isinf(setpoint) ||
        isnan(measurement) || isinf(measurement))
    {
        /* 输入无效，保持上一次输出，不改变内部状态 */
        return pid->out;
    }

    /*
     * 1. 计算误差
     *    误差 = 设定值 - 测量值（负反馈控制）
     */
    float error = setpoint - measurement;

    /*
     * 2. 比例项 (P)
     *    直接乘以比例增益 Kp，无滞后
     */
    float proportional = pid->Kp * error;

    /*
     * 3. 积分项 (I) - 梯形积分法（未乘 Ki）
     *    梯形积分公式：integrator += 0.5 * (当前误差 + 上一次误差)
     *    与原始版本不同，此处不再乘以 Ki * T，因为 Ki 已隐含了采样时间。
     *    最终积分项贡献为 pid->Ki * integrator。
     *    相比矩形积分，梯形积分能提供更高的精度，减少积分误差。
     */
    pid->integrator += 0.5f * (error + pid->prevError);

    /*
     * 抗积分饱和（积分限幅）
     *    将积分累加器限制在 [limMinInt, limMaxInt] 范围内，
     *    防止积分项过大导致输出饱和后无法快速退出。
     *    注意：限幅对象是积分累加器（未乘 Ki），因此限幅值应与 Ki 协调。
     *    通常 limMinInt = limMin / Ki, limMaxInt = limMax / Ki（若 Ki 不为零）。
     */
		if(pid->limMaxInt!=0.0f&&pid->limMinInt!=0.0f)
    if (pid->integrator > pid->limMaxInt) 
		{
        pid->integrator = pid->limMaxInt;
    }
		else if (pid->integrator < pid->limMinInt) 
		{
        pid->integrator = pid->limMinInt;
    }

    /*
     * 4. 微分项 (D) - 基于测量值微分（无低通滤波）
     *    采用测量值微分而非误差微分的原因：
     *      - 避免设定值阶跃变化时微分项产生尖峰（无微分冲击）
     *      - 对测量噪声有一定抑制作用（但无额外滤波，噪声敏感时需前端滤波）
     *    公式：differentiator = -Kd * (measurement - prevMeasurement)
     *    负号是由于“测量值微分”在负反馈中实际需要的是 -d(measurement)/dt。
     */
    pid->differentiator = -pid->Kd * (measurement - pid->prevMeasurement);

    /*
     * 5. 总输出 = 比例项 + 积分项（Ki * 积分累加器） + 微分项
     */
    pid->out = proportional + pid->Ki * pid->integrator + pid->differentiator;

    /*
     * 输出限幅：将最终输出限制在 [limMin, limMax] 范围内，
     * 防止超出执行器能力范围（例如电机 PWM 占空比）。
     */
    if (pid->out > pid->limMax) {
        pid->out = pid->limMax;
    } else if (pid->out < pid->limMin) {
        pid->out = pid->limMin;
    }

    /*
     * 6. 保存本次计算的误差和测量值，用于下一周期的积分和微分计算
     *    prevError：用于梯形积分
     *    prevMeasurement：用于测量值微分
     */
    pid->prevError = error;
    pid->prevMeasurement = measurement;

    /* 返回控制器输出 */
    return pid->out;
}




/**
 * @brief 初始化增量式 PID 控制器
 * 
 * 将历史误差和上一次输出清零，确保启动时输出为 0。
 */
void IncrementalPID_Init(IncrementalPID *pid)
{
    pid->prevError1 = 0.0f;
    pid->prevError2 = 0.0f;
    pid->prevOutput = 0.0f;
}

/**
 * @brief 增量式 PID 更新函数
 * 
 * 实现标准增量式算法，包含：
 * - 输入有效性检查（防止 NaN/Inf 传播）
 * - 增量计算（比例项差分、积分项、微分项二阶差分）
 * - 输出累加与限幅
 * 
 * @param pid         控制器指针
 * @param setpoint    设定值
 * @param measurement 测量值
 * @return float      限幅后的控制输出
 */
float IncrementalPID_Update(IncrementalPID *pid, float setpoint, float measurement)
{
    /* 1. 输入有效性检查 */
    if (isnan(setpoint) || isinf(setpoint) ||
        isnan(measurement) || isinf(measurement))
    {
        /* 输入无效，保持上一次输出，不更新内部状态 */
        return pid->prevOutput;
    }

    /* 2. 计算当前误差 e(k) */
    float error = setpoint - measurement;   // 负反馈

    /* 3. 计算增量 Δu(k) */
    float delta = pid->Kp * (error - pid->prevError1)          // 比例增量
                + pid->Ki * error                              // 积分项（误差本身）
                + pid->Kd * (error - 2.0f * pid->prevError1 + pid->prevError2); // 微分增量

    /* 4. 累加得到新的输出（暂存） */
    float newOutput = pid->prevOutput + delta;

    /* 5. 输出限幅 */
    if (newOutput > pid->limMax) {
        newOutput = pid->limMax;
    } else if (newOutput < pid->limMin) {
        newOutput = pid->limMin;
    }

    /* 6. 更新状态变量，供下一周期使用 */
    pid->prevError2 = pid->prevError1;   // 更新 e(k-2) = e(k-1)
    pid->prevError1 = error;             // 更新 e(k-1) = e(k)
    pid->prevOutput = newOutput;         // 更新 u(k-1) = u(k)

    /* 7. 返回最终输出 */
    return newOutput;
}
















