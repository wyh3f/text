//ALGORITHMS_CRC16
#ifndef ALGORITHMS_CRC16_H_
#define ALGORITHMS_CRC16_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
// #include <stdio.h>

//为1时高8位与低8位互换
#define CRC_EXCHANGE 1

//标准的 CRC-16/Modbus（通常也叫 Modbus RTU CRC）
uint16_t CRC16(const uint8_t *data, uint16_t len);








#ifdef __cplusplus
}
#endif

#endif


