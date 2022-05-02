/**
 * @file    I2c.cpp
 * @author  Samuel Martel
 * @date    2022-05-02
 * @brief
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <a href=https://www.gnu.org/licenses/>https://www.gnu.org/licenses/<a/>.
 */
#if defined(NILAI_USE_EVENTS) && defined(NILAI_USE_I2S_EVENTS)
#    include "../../defines/Events/Events.h"
#    include "../../processes/application.hpp"

extern "C"
{
    void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef* hi2s)
    {
        cep::Events::I2sEvent e(hi2s, cep::Events::EventTypes::I2S_TxHalfCplt);
        cep::Application::Get()->DispatchEvent(&e);
    }

    void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef* hi2s)
    {
        cep::Events::I2sEvent e(hi2s, cep::Events::EventTypes::I2S_TxCplt);
        cep::Application::Get()->DispatchEvent(&e);
    }

    void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef* hi2s)
    {
        cep::Events::I2sEvent e(hi2s, cep::Events::EventTypes::I2S_RxHalfCplt);
        cep::Application::Get()->DispatchEvent(&e);
    }

    void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef* hi2s)
    {
        cep::Events::I2sEvent e(hi2s, cep::Events::EventTypes::I2S_RxCplt);
        cep::Application::Get()->DispatchEvent(&e);
    }

    void HAL_I2S_ErrorCallback(I2S_HandleTypeDef* hi2s)
    {
        cep::Events::I2sEvent e(hi2s, cep::Events::EventTypes::I2S_Error);
        cep::Application::Get()->DispatchEvent(&e);
    }
}
#endif
