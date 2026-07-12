/*- Coding With UTF-8 -*/


/*******************************************************************************
*   File Name：     shell.h
*   Description：   此文件提供了shell的相关配置，声明
*   Atuhor：        Letter
*   Date:           2018/4/20
*   注意：该版本属于官方版本的修改版本，使用串口的中断接收模式实现
*******************************************************************************/

#ifndef     __SHELL_H__
#define     __SHELL_H__


#include    "usart.h"
#include <stdlib.h>
#include "core_cm3.h"



extern UART_HandleTypeDef huart1;

extern uint32_t myCounter;


/*------------------------------宏定义----------------------------------------*/
#define     SHELL_VERSION               "v1.6"                  //版本

#define     SHELL_USE_PARAMETER         1                       //是否使用带参函数
#define     SHELL_USE_HISTORY           1                       //是否使用历史命令

#define     shellUart                   huart1                  //shell使用的串口

#define     SHELL_COMMAND_MAX_LENGTH    50                      //shell命令最大长度
#define     SHELL_PARAMETER_MAX_LENGTH  10                      //shell命令参数最大长度
#define     SHELL_PARAMETER_MAX_NUMBER  5                       //shell命令参数最大数量

#define     SHELL_HISTORY_MAX_NUMBER    5                       //历史命令记录数量

#define     SHELL_COMMAND               "\r\n\r\nletter>>"

#define     shellDisplay(x)             _ShellDisplay((uint8_t *) (x));


/*---------------------------函数指针定义-------------------------------------*/
typedef void (*shellFunction)();


/*----------------------------结构体定义--------------------------------------*/
typedef struct
{
    uint8_t *name;                                              //shell命令名称
    shellFunction function;                                     //shell命令函数
    uint8_t *desc;                                              //shell命令描述
}SHELL_CommandTypeDef;                                          //shell命令定义


typedef enum
{
    CONTROL_FREE = 0,
    CONTROL_STEP_ONE,
    CONTROL_STEP_TWO,
}CONTROL_Status;

/*-----------------------------函数声明---------------------------------------*/

uint8_t shellReceiveByte(void);                                 //shell接收一字节数据

void shellDisplayByte(uint8_t data);                            //shell显示一字节数据


/*******************************************************************************
*@function  shellInit
*@brief     shell初始化
*@param     None
*@retval    None
*@author    Letter
*******************************************************************************/
void shellInit(void);                                           //shell初始化


         
/*******************************************************************************
*@function  shellHandler
*@brief     shell处理函数
*@param     *huart HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)函数中的当前串口句柄
*@retval    None
*@author    Letter
*@note      在串口输入触发的中断中调用此函数（通常为串口中断）
*******************************************************************************/
void shellHandler(UART_HandleTypeDef *huart); //shell处理函数,在函数中断中调用





#if SHELL_USE_HISTORY == 1
uint8_t shellStringCopy(uint8_t *dest, uint8_t *src);           //字符串复制
#endif

void shellBackspace(uint8_t length);                            //shell退格

void shellShowCommandList(void);                                //显示所有shell命令

void shellLetter(void);                                         //显示shell信息

void shellReboot(void);                                         //重启系统

void shellClear(void);                                          //shell清屏

#if SHELL_USE_PARAMETER == 1
uint32_t shellParaTest(uint32_t argc, uint8_t *argv[]);         //带参函数示例


// 显示变量的函数
void showMyVar(void);


// 设置变量的函数（带参）
uint32_t setMyVar(uint32_t argc, uint8_t *argv[]);


#endif

#endif

