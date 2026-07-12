//shell_port
#ifndef SHELL_PORT_H
#define SHELL_PORT_H

#include <stdint.h>
#include <stddef.h>
#include "shell_cfg.h"
#include "shell.h"
#include "shell_ext.h"
#include "shell_ring.h"


void shell_port_init(void);

void shell_port_write(uint8_t *data,uint16_t Size);

void shell_port_Task(uint16_t time);

void shell_port_Task_NoTime(void);

#endif /* SHELL_RING_H */


