#pragma once

#include <cstddef>
#include <cstdint>
#include "stm32l4xx_hal.h"
#include "../DMA/dma.h"
#include "../STM32L452/io_buffer.h"
#include <string>
#include <map>
#include <vector>

typedef uint32_t HAL_UART_StateTypeDef;


#define  HAL_UART_STATE_READY         0x00000020U    /*!< Peripheral Initialized and ready for use Value is allowed for gState and RxState */

struct UART_HandleTypeDef {
    size_t id;
    HAL_UART_StateTypeDef gState;
    DMA_HandleTypeDef* hdmarx;
};

extern std::map<size_t, IoBuffer*> s_uart_buffers;

HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef*);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef*, uint8_t*, size_t);
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef*, uint8_t*, size_t);

void Nilai_UART_Init(UART_HandleTypeDef*, size_t = 512, size_t = 512);HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef*);
void Nilai_UART_Inject_DMA(UART_HandleTypeDef*, const uint8_t*, size_t);
void Nilai_UART_Inject_DMA(UART_HandleTypeDef*, const std::string&);
void Nilai_UART_Inject_DMA(UART_HandleTypeDef*, const std::vector<uint8_t>&);
IoBuffer* Nilai_UART_Get_Buffer(UART_HandleTypeDef*);
