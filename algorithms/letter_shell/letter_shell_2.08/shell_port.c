//shell_port
#include "shell_port.h"

SHELL_TypeDef shell;
#include "usart.h"
#include "gpio.h"


extern UART_HandleTypeDef huart1;


 signed char myShellRead(char *c)
{
	if (shell_ring_dequeue((uint8_t*)c,1) != 0)
		return 0;
	else
		return -1;
}

 void myShellWrite(const char c)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 1000);
}


void shell_port_init(void)
{
	shell.read = myShellRead;
	shell.write = myShellWrite;
	shellInit(&shell);
	shell_ring_init();
}

void shell_port_write(uint8_t *data,uint16_t Size)
{
	shell_ring_enqueue(data,Size);
}





void shell_port_Task(uint16_t time)
{
    static uint32_t shellTick = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        shellTick = HAL_GetTick();
        initialized = 1;
    }
    if (HAL_GetTick() - shellTick >= time) {
        shellTick = HAL_GetTick();
        shellTask(&shell);
    }
}


void shell_port_Task_NoTime(void)
{

        shellTask(&shell);

}





/**
 * @file shell_commands_help.h
 * @brief Letter Shell 指令与变量使用说明（基于当前实现）
 * @version 2.0.8
 * @date 2026-07-12
 * 
 * ===========================================================
 *  Letter Shell v2.0.8 命令与变量快速参考
 * ===========================================================
 * 
 * 本文件描述了当前 shell 支持的命令注册方式、变量访问方法、
 * 内置命令用法及交互特性。适用于基于 STM32 的嵌入式调试。
 */

/* ==================== 一、命令注册 ==================== */

/**
 * @defgroup CMD_REG 命令注册（SHELL_EXPORT_CMD）
 * @brief 将 C 函数导出为 Shell 可执行命令。
 * 
 * 用法：
 *   SHELL_EXPORT_CMD(cmd_name, function, description);
 *   SHELL_EXPORT_CMD_EX(cmd_name, function, description, long_help);
 * 
 * @param cmd_name  用户在终端输入的命令字符串
 * @param function  要调用的 C 函数指针（支持带参或无参）
 * @param description  简短描述（出现在 help 列表）
 * @param long_help    长帮助信息（输入 help cmd_name 时显示）
 * 
 * @note 带参函数需遵循标准 C 调用约定，支持 int、char、char* 等类型，
 *       自动参数解析由 shell_ext 实现（需开启 SHELL_AUTO_PRASE）。
 * 
 * 示例：
 *   void reboot(void) { NVIC_SystemReset(); }
 *   SHELL_EXPORT_CMD(reboot, reboot, reboot the mcu);
 * 
 *   int plus(int a, int b) { return a + b; }
 *   SHELL_EXPORT_CMD(plus, plus, add two numbers);
 */

/* ==================== 二、变量导出与访问 ==================== */

/**
 * @defgroup VAR_EXPORT 变量导出（SHELL_EXPORT_VAR_xxx）
 * @brief 将全局变量暴露给 Shell，支持读取和修改。
 * 
 * 宏定义：
 *   SHELL_EXPORT_VAR_INT(var_name, variable, desc)
 *   SHELL_EXPORT_VAR_SHORT(...)
 *   SHELL_EXPORT_VAR_CHAR(...)
 *   SHELL_EXPORT_VAR_POINTER(...)
 *   SHELL_EXPORT_VAL(...)   // 只读常量
 * 
 * @param var_name  在 Shell 中使用的变量名
 * @param variable  实际 C 全局变量（取地址或指针）
 * @param desc      变量描述（显示在 vars 列表）
 * 
 * @note 变量通过前导 '$' 访问，如 $var1。
 * 
 * 示例：
 *   int var1 = 123;
 *   SHELL_EXPORT_VAR_INT(var1, var1, var for test);
 * 
 * 终端操作：
 *   > $var1          # 显示 var1 当前值（十进制 + 十六进制）
 *   > setVar var1 456 # 设置 var1 为 456（内置 setVar 命令）
 */

/* ==================== 三、内置命令 ==================== */

/**
 * @defgroup BUILTIN_CMD 内置命令
 * @brief Shell 默认提供的命令（无需额外注册）。
 */

/**
 * @defgroup CMD_HELP help
 * @brief 显示命令列表或指定命令的帮助信息。
 * 
 * 用法：
 *   help          # 列出所有命令（名称 + 简短描述）
 *   help <cmd>    # 显示 <cmd> 的详细帮助（若支持长帮助）
 * 
 * 示例：
 *   > help
 *   > help setVar
 */

/**
 * @defgroup CMD_CLS cls
 * @brief 清空终端屏幕（发送 ANSI 转义序列）。
 * 
 * 用法：
 *   cls
 */

/**
 * @defgroup CMD_VARS vars
 * @brief 列出所有导出的变量及其描述。
 * 
 * 用法：
 *   vars
 * 
 * @note 此命令仅显示变量名和描述，不显示当前值。
 *       如需查看变量值，请使用 $变量名。
 */

/**
 * @defgroup CMD_SETVAR setVar
 * @brief 设置导出的变量的值。
 * 
 * 用法：
 *   setVar <var_name> <value>
 *   setVar $<var_name> <value>
 * 
 * @param var_name  变量名（可带或不带 $ 前缀）
 * @param value     整数值
 * 
 * 示例：
 *   > setVar var1 150
 *   > setVar $var1 200
 * 
 * @note 仅支持整型变量（int/short/char），不支持指针/常量。
 */

/* ==================== 四、交互特性 ==================== */

/**
 * @defgroup FEATURES 交互特性
 * @brief Shell 提供的编辑、历史、补全功能。
 */

/**
 * @defgroup HISTORY 历史命令
 * @brief 使用上下方向键（↑/↓）浏览历史输入。
 * 
 * - 按 ↑：调出上一条命令
 * - 按 ↓：调出下一条命令
 * - 历史记录数量由 SHELL_HISTORY_MAX_NUMBER 控制（默认 5）
 * 
 * @note 历史记录存储在 RAM 中，掉电丢失。
 */

/**
 * @defgroup TAB_COMPLETE Tab 补全
 * @brief 输入命令前缀后按 Tab 键自动补全。
 * 
 * - 若只有唯一匹配，自动补全完整命令。
 * - 若有多个匹配，列出所有匹配项并补全公共前缀。
 * - 若当前输入为空，按 Tab 相当于输入 help。
 * - 双击 Tab（在 SHELL_DOUBLE_CLICK_TIME 内）显示命令的详细帮助。
 */

/**
 * @defgroup ANSI_CTRL ANSI 控制键
 * @brief 支持方向键移动光标（左右键）及退格/删除。
 * 
 * - 左右方向键：在已输入命令中移动光标。
 * - Backspace/Delete：删除字符。
 * - 支持插入模式，可在任意位置插入/删除字符。
 */

/* ==================== 五、参数解析规则 ==================== */

/**
 * @defgroup PARSE 参数解析
 * @brief Shell 对输入命令的参数拆分与转义。
 * 
 * - 默认以空格、Tab、逗号作为分隔符。
 * - 使用双引号（"）包裹含空格的参数。
 * - 使用反斜杠（\）转义特殊字符（如 \"、\\）。
 * 
 * 示例：
 *   > cmd arg1 "arg with spaces" arg3
 *   > cmd "He said \"Hello\""
 */

/* ==================== 六、返回值显示 ==================== */

/**
 * @defgroup RETURN 返回值
 * @brief 命令执行后会显示返回值（若 SHELL_DISPLAY_RETURN 开启）。
 * 
 * 格式：
 *   Return: <十进制值>, 0x<十六进制值>
 * 
 * @note 对于无返回值的 void 函数，显示为 Return: 0, 0x00000000。
 */

/* ==================== 七、使用示例 ==================== */

/**
 * @defgroup EXAMPLES 常见使用场景
 * @brief 终端交互示例。
 * 
 * 1. 查看变量：
 *   > $var1
 *   var1 = 123, 0x0000007B
 * 
 * 2. 修改变量：
 *   > setVar var1 150
 *   var1 = 150, 0x00000096
 *   Return: 0, 0x00000000
 * 
 * 3. 执行函数命令：
 *   > plus 5 3
 *   Return: 8, 0x00000008
 * 
 * 4. 带字符串参数：
 *   > func 100 A hello
 *   input int: 100, char: A, string: hello
 *   Return: 0, 0x00000000
 * 
 * 5. 使用历史命令：
 *   按 ↑ 键调出上一条命令，按 Enter 执行。
 * 
 * 6. Tab 补全：
 *   输入 "se"，按 Tab 自动补全为 "setVar"。
 */

/* ==================== 八、配置选项（shell_cfg.h） ==================== */

/**
 * @defgroup CFG 配置项说明
 * @brief 关键配置宏及其影响。
 * 
 * - SHELL_USING_CMD_EXPORT      : 启用命令导出（必须）
 * - SHELL_USING_VAR             : 启用变量导出
 * - SHELL_AUTO_PRASE            : 启用参数自动解析（支持多种类型）
 * - SHELL_DISPLAY_RETURN        : 显示函数返回值
 * - SHELL_LONG_HELP             : 启用长帮助（help <cmd>）
 * - SHELL_COMMAND_MAX_LENGTH    : 单行命令最大长度（默认 50）
 * - SHELL_PARAMETER_MAX_NUMBER  : 最大参数个数（默认 8）
 * - SHELL_HISTORY_MAX_NUMBER    : 历史记录条数（默认 5）
 * - SHELL_DOUBLE_CLICK_TIME     : 双击 Tab 间隔（ms）
 * - SHELL_DEFAULT_COMMAND       : 提示符（默认 "\r\nletter>>"）
 */

/* ==================== 九、注意事项 ==================== */

/**
 * @defgroup NOTES 注意事项
 * @brief 使用中的常见问题。
 * 
 * 1. 变量名与命令名不能重复，否则变量会被命令覆盖。
 * 2. 变量使用 '$' 前缀访问，命令直接输入名称。
 * 3. 修改指针变量或常量（SHELL_VAL）会导致错误。
 * 4. 历史记录在复位后丢失，如需持久化需额外实现。
 * 5. Tab 补全仅基于已注册命令，不包含变量名。
 * 6. 参数解析不支持浮点数或结构体，请自行转换。
 * 7. 长帮助需要 SHELL_LONG_HELP 和 SHELL_GET_TICK 支持。
 */











