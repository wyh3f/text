#include "PID1.h"
#include <math.h>

/*
 * 作者：LiuYuanlin
 * 日期：2021年12月29日
 * 时间：10点49分
 * 项目：PID 库 2.0 版
 */

/**
 * 获取符号位（仅适用于小端模式）
 * 如果 x >= 0 则返回  1
 * 如果 x <  0 则返回 -1
 */
#define Sign(x) ((x) >= 0 ? 1 : -1)

/*==============================================================================
 *  PID 结构体初始化
 *==============================================================================*/

/**
 * @brief 初始化 PID 结构体的所有成员为默认值
 * @param self PID 对象指针
 * @details
 *   - 目标值、采样值置 0
 *   - 误差历史清零
 *   - PID 系数置 0（需用户后续设置）
 *   - 输出 F 置 0，限幅设为 [0, 1]
 *   - 衰减函数指针置为 NULL（即不使用衰减）
 */
void IncPIDInit(pPID self)
{
    self->Target = 0;          // 设定值（即“黑箱”的期望输出）
    self->iSampling = 0;       // 采样值（反馈量）
    self->Step = 1;            // 步长比例（保留字段，暂未使用）

    self->LastError = 0;       // 前两次误差 e(k-2)
    self->PrevError = 0;       // 前一次误差 e(k-1)

    self->P = 0.f;             // 比例系数
    self->I = 0.f;             // 积分系数
    self->D = 0.f;             // 微分系数

    self->iError = 0;          // 当前误差 e(k)

    self->F = 0;               // 归一化输出值（控制量）
    self->Fmax = 1.0f;          // 输出上限
    self->Fmin = 0;            // 输出下限
    self->dt = 1;              // 采样周期（默认1，用户可自行维护）
    self->sysArg = 0;          // 系统参数基数（需用户设置）

    self->pidDecayByAbsErrorFunc = PID_DECAY_FUNC_NULL; // 衰减函数指针为空
}

/*==============================================================================
 *  位置式 PID 计算（无衰减）
 *==============================================================================*/

/**
 * @brief 位置式 PID 计算，输出绝对控制量
 * @param self PID 对象指针
 * @details
 *  公式：F = P*e(k) + I*∑e(i) + D*[e(k)-e(k-1)]
 *  积分项 Integral 会持续累加，注意积分饱和问题（本库未作防饱和处理）。
 *  计算后直接限幅到 [Fmin, Fmax]。
 */
void PosPIDCalc_NormalizedF(pPID self)
{
    float F = self->F;  // 暂存旧值（用于后续比较，但实际未使用，此句可优化）

    // 计算当前误差 e(k) = Target - iSampling
    self->iError = self->Target - self->iSampling;

    // 比例项
    float Pout = self->P * self->iError;

    // 积分项：累加误差
    self->Integral += self->iError;
    float Iout = self->I * self->Integral;

    // 微分项：当前误差 - 前一次误差
    float Dout = self->D * (self->iError - self->PrevError);

    // 计算总输出
    self->F = Pout + Iout + Dout;
    self->PrevError = self->iError;  // 更新前一次误差供下次使用

    // 限幅处理
    if (F >= self->Fmax)
        F = self->Fmax;
    else if (F <= self->Fmin)
        F = self->Fmin;

    self->F = F;
}

/*==============================================================================
 *  基本增量式 PID（无衰减）
 *==============================================================================*/

/**
 * @brief 基本增量式 PID，输出为累加控制量
 * @param self PID 对象指针
 * @details
 *  公式：ΔF = P*[e(k)-e(k-1)] + I*e(k) + D*[e(k)-2e(k-1)+e(k-2)]
 *  新输出：F(k) = F(k-1) + ΔF，并限幅到 [Fmin, Fmax]。
 *  若目标大于采样值（误差为正），则 F 增大；反之减小。
 */
void IncPIDCalc_NormalizedF(pPID self)
{
    // 计算当前误差
    self->iError = self->Target - self->iSampling;

    float delta = 0;
    float F = 0;

    // 计算增量 delta
    delta = self->P * (self->iError - self->LastError) +
            self->I * self->iError +
            self->D * (self->iError - 2 * self->LastError + self->PrevError);

    // 更新误差历史（注意顺序：先更新 PrevError，再更新 LastError）
    self->PrevError = self->LastError;   // e(k-2) = 旧 e(k-1)
    self->LastError = self->iError;      // e(k-1) = 当前 e(k)

    // 累加增量得到新输出
    F = self->F + delta;

    // 限幅
    if (F >= self->Fmax)
        F = self->Fmax;
    else if (F <= self->Fmin)
        F = self->Fmin;

    self->F = F;
}

/*==============================================================================
 *  两阶段增量式 PID（分段控制，无衰减）
 *==============================================================================*/

/**
 * @brief 两阶段增量式 PID，仅当误差在指定区间内才进行调节
 * @param self              PID 对象指针
 * @param FirstContrlPoint  误差绝对值上限（大于此值不调节）
 * @param LasttContrlPoint  误差绝对值下限（小于此值不调节）
 * @param FirstContrlScale  区间内的缩放系数
 * @details
 *  仅当  LasttContrlPoint < |error| < FirstContrlPoint  时，
 *  计算增量并乘以 FirstContrlScale；否则增量置0，且误差被清零（防止积分累积）。
 *  适用于死区控制或仅在特定误差范围内进行精细调节。
 */
void IncPIDCalcDelta_NormalizedF_TwoStage(pPID self, float FirstContrlPoint, float LasttContrlPoint, float FirstContrlScale)
{
    float F = 0;
    float absError = 0;

    // 计算当前误差及其绝对值
    self->iError = self->Target - self->iSampling;
    absError = fabsf(self->iError);

    /*
     * 请注意：此文件请勿使用自动格式化工具，
     * 因为 PID 计算代码较长，自动格式化后不便查看。
     */

    // 此处将 F 视为增量 delta
    F = self->P * (self->iError - self->LastError) +
        self->I * self->iError +
        self->D * (self->iError - 2 * self->LastError + self->PrevError);

    // 判断误差是否在有效区间内
    if (absError >= LasttContrlPoint && absError <= FirstContrlPoint)
    {
        F *= FirstContrlScale;   // 缩放增量
    }
    else
    {
        F = 0;                  // 区间外不调节
        self->iError = 0;       // 清零当前误差，避免积分项累积
    }

    // 更新误差历史
    self->PrevError = self->LastError;
    self->LastError = self->iError;

    // 累加增量得到新输出
    F = self->F + F;

    // 限幅
    if (F >= self->Fmax)
        F = self->Fmax;
    else if (F <= self->Fmin)
        F = self->Fmin;

    // 最后更新 self->F，避免中间结果影响系统运行
    self->F = F;
}

/*==============================================================================
 *  带外部衰减函数的增量式 PID（变增益）
 *==============================================================================*/

/**
 * @brief 增量式 PID，整体乘以外部传入的衰减函数（作用于误差）
 * @param self     PID 对象指针
 * @param deacyfun 衰减函数指针，输入为当前误差，输出为缩放因子（通常 ≤ 1）
 * @details
 *  计算增量后，乘以 deacyfun(iError)，实现非线性变增益控制。
 *  需确保 deacyfun 不为 NULL。
 */
void IncPIDCalcDelta_NormalizedF_Decay(pPID self, PIDDecayFun deacyfun)
{
    float F = 0;

    // 计算当前误差
    self->iError = self->Target - self->iSampling;

    /*
     * 请勿使用自动格式化
     */

    // 计算增量 delta
    F = self->P * (self->iError - self->LastError) +
        self->I * self->iError +
        self->D * (self->iError - 2 * self->LastError + self->PrevError);

    // 应用衰减函数
    F *= deacyfun(self->iError);

    // 更新误差历史
    self->PrevError = self->LastError;
    self->LastError = self->iError;

    // 累加增量
    F = self->F + F;

    // 限幅
    if (F >= self->Fmax)
        F = self->Fmax;
    else if (F <= self->Fmin)
        F = self->Fmin;

    // 最后更新输出
    self->F = F;
}

/*==============================================================================
 *  带误差归一化的衰减增量式 PID（不推荐）
 *==============================================================================*/

/**
 * @brief 增量式 PID，衰减函数输入为归一化误差 (iError / sysArg)
 * @param self     PID 对象指针
 * @param deacyfun 衰减函数指针
 * @details
 *  与上一个函数类似，但将误差除以 sysArg 后再传入衰减函数。
 *  @warning 不推荐使用，因为 sysArg 的物理意义是“调制幅度最大值”，
 *           并非误差最大值，容易引起混淆。
 *           若需归一化，建议用户自行构造单参数衰减函数并在内部处理。
 */
void IncPIDCalcDelta_NormalizedFAndDecayFunInput_Decay(pPID self, PIDDecayFun deacyfun)
{
    float F = 0;

    // 计算当前误差
    self->iError = self->Target - self->iSampling;

    /*
     * 请勿使用自动格式化
     */

    // 计算增量
    F = self->P * (self->iError - self->LastError) +
        self->I * self->iError +
        self->D * (self->iError - 2 * self->LastError + self->PrevError);

    // 应用衰减函数，输入为归一化误差
    F *= deacyfun(self->iError / self->sysArg);

    // 更新误差历史
    self->PrevError = self->LastError;
    self->LastError = self->iError;

    // 累加增量
    F = self->F + F;

    // 限幅
    if (F >= self->Fmax)
        F = self->Fmax;
    else if (F <= self->Fmin)
        F = self->Fmin;

    // 最后更新输出
    self->F = F;
}

/*==============================================================================
 *  自动衰减增量式 PID（推荐）
 *==============================================================================*/

/**
 * @brief 增量式 PID，使用结构体内部预设的衰减函数
 * @param self PID 对象指针
 * @details
 *  衰减函数指针保存在 self->pidDecayByAbsErrorFunc 中。
 *  若该指针为 NULL，则不进行衰减（等同于基本增量式）。
 *  此函数为推荐使用的变增益入口，调用简洁且可动态切换衰减策略。
 */
void IncPIDCalcDeltaAutoDecay(pPID self)
{
    float F = 0;

    // 计算当前误差
    self->iError = self->Target - self->iSampling;

    /*
     * 请勿使用自动格式化
     */

    // 计算增量
    F = self->P * (self->iError - self->LastError) +
        self->I * self->iError +
        self->D * (self->iError - 2 * self->LastError + self->PrevError);

    // 如果衰减函数指针非空，则应用衰减
    if (self->pidDecayByAbsErrorFunc != PID_DECAY_FUNC_NULL)
        F *= self->pidDecayByAbsErrorFunc(self->iError);

    // 更新误差历史
    self->PrevError = self->LastError;
    self->LastError = self->iError;

    // 累加增量
    F = self->F + F;

    // 限幅
    if (F >= self->Fmax)
        F = self->Fmax;
    else if (F <= self->Fmin)
        F = self->Fmin;

    // 最后更新输出
    self->F = F;
}

/*==============================================================================
 *  内置衰减函数实现
 *==============================================================================*/

/**
 * @brief 钟形衰减函数 y = tanh²(az/2) = 1 - 4e^{-az}/(1+e^{-az})²
 * @param z 输入（误差）
 * @param a 衰减系数（a > 0）
 * @return 在 z=0 处为 1，|z| 增大时单调递减至 0
 * @note  适用于“大误差时减小增益”的场景，有效抑制超调
 */
float dsigmoidn(float z, float a)
{
    float e_ = (float)exp((double)(-a * z));
    float q_ = 1.0f + e_;
    return 1 - 4 * e_ / (q_ * q_);
}

/**
 * @brief 绝对值 S 型衰减函数 y = 1 / (1 + b*e^{-a|z|})
 * @param z 输入（误差）
 * @param a 指数系数（a > 0）
 * @param b 偏移系数（b > 0），决定 z=0 处的值：1/(1+b)
 * @return 值域 (0, 1]，|z| 越大越趋近于 1
 * @note  可实现“小误差时增益低，大误差时增益高”
 */
float sigmoidabsx(float z, float a, float b)
{
    z = fabsf(z);
    return 1.0f / (b * (float)exp((double)(-a * z)));
}

/**
 * @brief 绝对值双曲正切衰减 y = tanh(a|z|)
 * @param z 输入（误差）
 * @param a 系数（a > 0）
 * @return 值域 [0, 1)，|z| 增大时趋近于 1
 * @note  适合“小误差时削弱控制，大误差时增强控制”
 */
float tanhabsx(float z, float a)
{
    z = fabsf(z);
    return (float)tanh((double)(a * z));
}

/**
 * @brief 幂律型衰减 y = 1 - 1/sqrt(a|z|+1)
 * @param z 输入（误差）
 * @param a 系数（a > 0）
 * @return 值域 [0, 1)，|z| 增大时缓慢趋近于 1
 * @note  变化平缓，适用于对噪声不敏感的场合
 */
float px1(float z, float a)
{
    z = fabsf(z);
    return 1 - 1.0f / (float)sqrt((double)(a * z + 1));
}

/**
 * @brief 分段线性饱和函数 y = |z|/x  当 |z| < x；y = 1 当 |z| ≥ x
 * @param z 输入（误差）
 * @param x 阈值（x > 0）
 * @return 线性增长后饱和，可视为软限幅或误差比例增益
 */
float obliquestepfun(float z, float x)
{
    z = fabsf(z);
    if (z > x)
    {
        return 1;
    }
    else
    {
        return 1 / x * z;
    }
}


