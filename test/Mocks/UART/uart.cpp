#include "uart.h"
#include <algorithm>

size_t s_uart_buffers_id_count;
std::map<size_t, IoBuffer*> s_uart_buffers;

void Nilai_UART_Init(UART_HandleTypeDef* handle, size_t txl, size_t rxl) {
    s_uart_buffers[s_uart_buffers_id_count] = new IoBuffer{{txl}, {rxl}};
    handle->id = s_uart_buffers_id_count++;
    handle->hdmarx = new DMA_HandleTypeDef;
    __NILAI_DMA_SET_CAPACITY(handle->hdmarx, rxl);
}

HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef* handle) {
    delete s_uart_buffers[handle->id];
    delete handle->hdmarx;
    s_uart_buffers.erase(handle->id);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* handle, uint8_t* buff, size_t len) {
    s_uart_buffers[handle->id]->rx.init(buff, len);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef* handle, uint8_t* buff, size_t len) {
    s_uart_buffers[handle->id]->tx.push(buff, len);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef*) {
    return HAL_OK;
}

void Nilai_UART_Inject_DMA(UART_HandleTypeDef* handle, const uint8_t* buff, size_t len) {
    size_t pushed = s_uart_buffers[handle->id]->rx.push(buff, len);
    s_uart_buffers[handle->id]->rx.pop(pushed);
    __NILAI_DMA_DEC_COUNTER(handle->hdmarx, len);
}

void Nilai_UART_Inject_DMA(UART_HandleTypeDef* handle, const std::string& msg) {
    Nilai_UART_Inject_DMA(handle, reinterpret_cast<const uint8_t*>(msg.data()), msg.length());
}

void Nilai_UART_Inject_DMA(UART_HandleTypeDef* handle, const std::vector<uint8_t>& buff) {
    Nilai_UART_Inject_DMA(handle, buff.data(), buff.size());
}

IoBuffer* Nilai_UART_Get_Buffer(UART_HandleTypeDef* handle) {
    return s_uart_buffers[handle->id];
}
