#pragma once
#include "defines/circular_buffer.hpp"

struct IoBuffer {
    CircularBuffer<uint8_t> tx; 
    CircularBuffer<uint8_t> rx; 
};
