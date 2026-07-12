#include "stm32f1xx_hal.h"
#include "shell.h"
#include "stdio.h"
#include "gpio.h"


extern SHELL_TypeDef shell;

void func(int i, char ch, char *str)
{
    shellPrint(&shell,"input int: %d, char: %c, string: %s\r\n", i, ch, str);
}

SHELL_EXPORT_CMD(func, func, test);


void reboot(void)
{
		NVIC_SystemReset(); // Reset the microcontroller
}

SHELL_EXPORT_CMD(reboot, reboot, reboot the mcu);

int plus(int a, int b)
{
		return a + b;
}

SHELL_EXPORT_CMD(plus, plus, plus);

void led(int state)
{
		if (state ==0)
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

SHELL_EXPORT_CMD(led, led, set led state);

int var1 = 123;
SHELL_EXPORT_VAR_INT(var1, var1, var for test);

void show_var1(void)
{
	shellPrint(&shell,"var1: %d\r\n", var1);
}
SHELL_EXPORT_CMD(show_var1, show_var1, show vart value);

