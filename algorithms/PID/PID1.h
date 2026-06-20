#ifndef __PID1_H
#define __PID1_H

/**
 * @brief PID 衰减函数类型定义：接收当前误差（float），返回一个缩放因子（float）
 */
typedef float (*PIDDecayFun)(float);

#define PID_DECAY_FUNC_NULL ((PIDDecayFun)0)   /* 空函数指针，表示不使用衰减 */

/**
 * @brief PID 控制器结构体
 *
 * 支持位置式与增量式两种算法，并提供多种非线性衰减（变增益）选项。
 * 所有输出值 F 均归一化到 [Fmin, Fmax] 区间（通常为 [0, 1]），
 * 可通过宏 PIDUpdateValue_P/N 转换为实际物理量（如 PWM 比较值）。
 */
typedef struct PID
{
    float Target;          /* 目标值（设定值）                                     */
    float iSampling;       /* 系统观测值 / 采样值（即反馈量）                      */
    float P;               /* 比例系数                                            */
    float I;               /* 积分系数                                            */
    float D;               /* 微分系数                                            */

    float iError;          /* 当前误差 = Target - iSampling                       */
    float LastError;       /* 前一次误差 e(k-1)                                   */
    float PrevError;       /* 前两次误差 e(k-2)                                   */

    float Integral;        /* 积分累加项（仅位置式 PID 使用）                     */

    float F;               /* 当前归一化控制输出，范围 [Fmin, Fmax]               */
    float Fmax;            /* 输出上限（归一化值，通常为 1.0）                    */
    float Fmin;            /* 输出下限（归一化值，通常为 0.0）                    */

    float dt;              /* 采样周期（未在本库中使用，可由用户自行维护）         */
    float Step;            /* 步长缩放（保留字段，暂未使用）                      */

    float sysArg;          /* 系统参数：用于将归一化输出 F 转换为实际物理量的基数 */
                           /* 例如 PWM 周期计数值，或电机最大转速等               */

    PIDDecayFun pidDecayByAbsErrorFunc; /* 基于误差绝对值的衰减函数指针            */
                                         /* 若为 NULL，则不进行衰减（即恒为 1）    */

} PID, *pPID;

/*------------------ 初始化函数 ------------------*/
/**
 * @brief 初始化 PID 结构体，将所有状态清零并设为默认值
 * @param self PID 对象指针
 */
void IncPIDInit(pPID self);

/*------------------ 位置式 PID ------------------*/
/**
 * @brief 位置式 PID 计算（无衰减）
 * @details
 *  直接计算控制量 F = P*e + I*∑e + D*(e - e_prev)，
 *  输出为绝对值，执行器应能直接接受该位置信号。
 *  注意：积分项 Integral 会持续累加，需注意积分饱和问题。
 * @param self PID 对象指针
 * @return 无
 */
void PosPIDCalc_NormalizedF(pPID self);

/*------------------ 基本增量式 PID --------------*/
/**
 * @brief 基本增量式 PID 计算（无衰减）
 * @details
 *  计算增量 ΔF = P*(e - e_prev) + I*e + D*(e - 2*e_prev + e_prev2)，
 *  然后累加至上一输出 F = F + ΔF，并限幅到 [Fmin, Fmax]。
 *  若 Target > iSampling（误差为正），则 F 增大；反之 F 减小。
 *  适用于步进电机、PWM 占空比等积分型执行器。
 * @param self PID 对象指针
 * @return 无
 */
void IncPIDCalc_NormalizedF(pPID self);

/*------------------ 两阶段增量式 PID ------------*/
/**
 * @brief 两阶段增量式 PID（分段控制，无衰减）
 * @details
 *  仅当误差绝对值位于 (LasttContrlPoint, FirstContrlPoint) 区间内时，
 *  才计算增量，并将增量乘以缩放系数 FirstContrlScale；
 *  否则增量置为 0，且当前误差清零（防止积分累积）。
 *  适用于死区控制或仅在特定误差范围内进行调节的场景。
 * @param self              PID 对象指针
 * @param FirstContrlPoint  误差绝对值上限（大于此值不调节）
 * @param LasttContrlPoint  误差绝对值下限（小于此值不调节）
 * @param FirstContrlScale  区间内的缩放系数
 * @return 无
 */
void IncPIDCalcDelta_NormalizedF_TwoStage(pPID self, float FirstContrlPoint, float LasttContrlPoint, float FirstContrlScale);

/*------------------ 带外部衰减函数的增量式 PID --*/
/**
 * @brief 增量式 PID，带外部传入的衰减函数（作用于误差绝对值）
 * @details
 *  计算增量后，乘以衰减函数 deacyfun(iError)，实现变增益控制。
 *  衰减函数应返回一个浮点数（通常 ≤ 1），用于缩放 P、I、D 的整体效果。
 *  例如可用 dsigmoidn、tanhabsx 等函数抑制大误差时的控制量，防止超调。
 * @param self     PID 对象指针
 * @param deacyfun 衰减函数指针，输入为当前误差，输出为缩放因子
 * @note 必须确保 deacyfun 不为 NULL
 * @return 无
 */
void IncPIDCalcDelta_NormalizedF_Decay(pPID self, PIDDecayFun deacyfun);

/*------------------ 带误差归一化的衰减增量式 PID -*/
/**
 * @brief 增量式 PID，衰减函数输入为归一化误差 (iError / sysArg)
 * @details
 *  与 IncPIDCalcDelta_NormalizedF_Decay 类似，但将误差除以 sysArg 后
 *  再传给衰减函数，使得衰减函数可适应不同量程。
 *  @warning 不推荐使用此函数，因为 sysArg 在设计中是“调制幅度最大值”，
 *           而非“输出最大值”，其物理意义可能不匹配，容易造成混淆。
 * @param self     PID 对象指针
 * @param deacyfun 衰减函数指针
 * @note 必须确保 deacyfun 不为 NULL
 * @return 无
 */
void IncPIDCalcDelta_NormalizedFAndDecayFunInput_Decay(pPID self, PIDDecayFun deacyfun);

/*------------------ 自动衰减增量式 PID ----------*/
/**
 * @brief 增量式 PID，使用结构体内部设置的衰减函数（推荐使用）
 * @details
 *  与上述带衰减函数类似，但衰减函数指针已保存在 self->pidDecayByAbsErrorFunc 中。
 *  若该指针为 NULL，则等效于无衰减的基本增量式 PID。
 *  使用此函数可使调用代码更简洁，且方便运行时动态切换衰减策略。
 * @param self PID 对象指针
 * @return 无
 */
void IncPIDCalcDeltaAutoDecay(pPID self);

/*------------------ 输出转换宏 ------------------*/
/**
 * @brief 将归一化输出 F 转换为正向物理量（例如 PWM 正占空比计数值）
 * @param self PID 对象指针
 * @return sysArg * F
 */
#define PIDUpdateValue_P(self) ((self)->sysArg * (self)->F)

/**
 * @brief 将归一化输出 F 转换为反向物理量（例如互补 PWM 或负逻辑）
 * @param self PID 对象指针
 * @return sysArg * (1.0f - F)
 */
#define PIDUpdateValue_N(self) ((self)->sysArg * (1.0f - (self)->F))

/**
 * @brief 便捷宏：设置采样值（反馈量）
 * @param self  PID 对象指针
 * @param value 采样值
 */
#define PIDSetSampleValue(self,value) ((self)->iSampling = (value))

/*------------------ 内置衰减函数库 ---------------*/
/**
 * @brief 钟形衰减函数 y = tanh²(az/2) = 1 - 4e^{-az}/(1+e^{-az})²
 * @param z 输入（误差）
 * @param a 衰减系数（a>0），值越大衰减越快
 * @return 在 z=0 处为 1，|z| 增大时单调递减至 0
 * @note 适用于“大误差时减小增益”的场合，可有效抑制超调
 */
float dsigmoidn(float z, float a);

/**
 * @brief 绝对值 S 型衰减 y = 1 / (1 + b*e^{-a|z|})
 * @param z 输入（误差）
 * @param a 指数系数（a>0）
 * @param b 偏移系数（b>0），决定 z=0 时的初始值：1/(1+b)
 * @return 值域 (0, 1]，|z| 越大越趋近于 1
 */
float sigmoidabsx(float z, float a, float b);

/**
 * @brief 绝对值双曲正切衰减 y = tanh(a|z|)
 * @param z 输入（误差）
 * @param a 系数（a>0）
 * @return 值域 [0, 1)，|z| 增大时趋近于 1
 * @note 适用于“小误差时削弱控制，大误差时增强控制”的非线性增益
 */
float tanhabsx(float z, float a);

/**
 * @brief 幂律型衰减 y = 1 - 1/sqrt(a|z|+1)
 * @param z 输入（误差）
 * @param a 系数（a>0）
 * @return 值域 [0, 1)，|z| 增大时缓慢趋近于 1
 */
float px1(float z, float a);

/**
 * @brief 分段线性饱和函数 y = |z|/x  当 |z| < x；y = 1 当 |z| ≥ x
 * @param z 输入（误差）
 * @param x 阈值（x>0）
 * @return 线性增长后饱和，可用于软限幅或误差比例增益
 */
float obliquestepfun(float z, float x);

/*------------------ 用户自定义初始化接口 -------*/
/**
 * @brief 用户自定义 PID 初始化函数（需由用户实现）
 * @details 此函数在头文件中仅声明，用户应在自己的源文件中实现，
 *          用于统一配置所有 PID 实例的参数。
 */
void UserPIDInit(void);

#endif /* __PID_H */

