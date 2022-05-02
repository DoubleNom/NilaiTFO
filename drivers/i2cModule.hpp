/**
 * @addtogroup  drivers
 * @{
 * @addtogroup  i2c
 * @{
 * @file        i2cModule.hpp
 * @author      Lou-Gabriel Gaucher
 * @author      Pascal-Emmanuel Lachance
 * @author      Samuel Martel
 * @date        2020/08/13  -  09:25
 *
 * @brief       I2C communication module
 */
#ifndef NILAI_I2CMODULE_HPP
#    define NILAI_I2CMODULE_HPP
/*************************************************************************************************/
/* Includes
 * ------------------------------------------------------------------------------------
 */
#    if defined(NILAI_USE_I2C)
#        include "../defines/internalConfig.h"
#        include NILAI_HAL_HEADER
#        if defined(HAL_I2C_MODULE_ENABLED)
#            include "../defines/macros.hpp"
#            include "../defines/misc.hpp"
#            include "../defines/module.hpp"

#            if defined(NILAI_USE_EVENTS) && defined(NILAI_USE_I2C_EVENTS)
#                include "../defines/Events/Events.h"
#            endif

#            include "I2C/Enums.h"
#            include "I2C/Structs.h"

#            include <string>
#            include <utility>
#            include <vector>


class I2cModule : public cep::Module
{
public:
    I2cModule(I2C_HandleTypeDef* handle, std::string label);
    ~I2cModule() override = default;

    bool                             DoPost() override;
    void                             Run() override;
    [[nodiscard]] const std::string& GetLabel() const { return m_label; }

#            if defined(NILAI_USE_EVENTS)
    bool OnEvent(cep::Events::Event* event) override;
#            endif

    void TransmitFrame(uint8_t addr, const uint8_t* data, size_t len);
    void TransmitFrame(uint8_t addr, const std::vector<uint8_t>& data)
    {
        TransmitFrame(addr, data.data(), data.size());
    }
    void TransmitFrame(const cep::I2C::Frame& frame)
    {
        TransmitFrame(frame.deviceAddress, frame.data.data(), frame.data.size());
    }

    void TransmitFrameToRegister(uint8_t addr, uint8_t regAddr, const uint8_t* data, size_t len);
    void TransmitFrameToRegister(uint8_t addr, uint8_t regAddr, const std::vector<uint8_t>& data)
    {
        TransmitFrameToRegister(addr, regAddr, data.data(), data.size());
    }
    void TransmitFrameToRegister(const cep::I2C::Frame& frame)
    {
        TransmitFrameToRegister(
          frame.deviceAddress, frame.registerAddress, frame.data.data(), frame.data.size());
    }

    cep::I2C::Frame ReceiveFrame(uint8_t addr, size_t len);
    cep::I2C::Frame ReceiveFrameFromRegister(uint8_t addr, uint8_t regAddr, size_t len);

    /**
     * @brief Checks if a device with the specified address is active on the I2C bus.
     * @param addr The address of the target.
     * @param attempts The number of attempts that should be made to contact the target.
     * @param timeout The maximum amount of time to wait for an acknowledge.
     * @return True if the target acknowledged, false otherwise.
     */
    [[maybe_unused]] [[nodiscard]] bool CheckIfDevOnBus(uint8_t addr,
                                                        size_t  attempts = 5,
                                                        size_t  timeout  = 2);

protected:
    I2C_HandleTypeDef* m_handle = nullptr;
    std::string        m_label;

    static constexpr uint16_t TIMEOUT = 200;
};
#        else
#            if WARN_MISSING_STM_DRIVERS
#                warning NilaiTFO I2C Module enabled, but HAL_I2C_MODULE_ENABLED is not defined!
#            endif
#        endif
#    endif
/**
 * @}
 * @}
 */
#endif
/* ----- END OF FILE ----- */
