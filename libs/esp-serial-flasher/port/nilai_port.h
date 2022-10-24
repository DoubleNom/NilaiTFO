/* Copyright 2020 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "../include/serial_io.h"
#include "defines/internalConfig.h"
#include "defines/pin.h"
#include "drivers/uart/module.hpp"

#include <stdint.h>
#include NILAI_HAL_HEADER

void loader_port_nilai_init(Nilai::Drivers::Uart::Module* uart, Nilai::Defines::Pin en, Nilai::Defines::Pin boot);
