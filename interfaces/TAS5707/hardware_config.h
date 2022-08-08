/**
 * @file    hardware_config.h
 * @author  Samuel Martel
 * @date    2022-02-07
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
 * not, see <a href=https://www.gnu.org/licenses/>https://www.gnu.org/licenses/</a>.
 */
#ifndef NILAI_HARDTAS5707_WARECONFIG_H
#define NILAI_HARDTAS5707_WARECONFIG_H
#if defined(NILAI_USE_TAS5707)

#    include "../../defines/pin.h"

// Forward declarations of needed classes.
class I2sModule;
class I2cModule;

namespace Nilai::Tas5707
{
/**
 * @struct HardwareConfig
 * @brief Structure describing the hardware connections to and from the TAS5705 amplifier.
 *
 * Some fields are optional, others are mandatory.
 *
 * Mandatory information:
 *  @li I2cModule* I2cHandle
 *  @li Nilai::Pin Reset
 *  @li Nilai::Pin PwrDwn
 *
 * Optional pins:
 *  @li Nilai::Pin Fault
 *  @li Nilai::Pin HeadphoneSelect
 *  @li Nilai::Pin Mute
 *  @li Nilai::Pin PVddEn
 */
struct HardwareConfig
{
    //! Handle to the I2C module controlling the registers.
    I2cModule* I2cHandle = nullptr;

    /**
     * @brief [Output] Reset pin, active low.
     *
     * A system reset is generated by setting this pin in a low state. It restores the DAP to its
     * default conditions, sets the VALID outputs low, and places the PWM in the hard-mute state
     * (stops switching). The master volume is immediately set to full attenuation. Upon the release
     * of Reset, if PwrDwn is high, the chip performs a 4–5-ms device initialization and sets the
     * volume 0.
     */
    Nilai::Pin Reset = {};

    /**
     * @brief [Output] Power down pin, active low.
     *
     * When low, all logic, clocks and output are turned off.
     * When transitioning from low to high, the device powers up all logic, starts all clocks, and
     * performs a soft start that returns to the previous configuration determined by register
     * settings.
     */
    Nilai::Pin PwrDwn = {};

    /**
     * @brief [Input] Fault reporting pin, active low.
     *
     * If this pin is low, the chip has detected an over-temperature, over-current and/or an
     * under-voltage.
     *
     * @note It is recommended to set this pin as an external interrupt, to be aware of any faults
     * as soon as they happen. Otherwise, this pin should be polled frequently.
     */
    Nilai::Pin Fault = {};

    /**
     * @brief [Output] Headphone select pin, active high.
     *
     * When this pin is set to high, the chip enters headphone mode and hard-mutes the speaker
     * outputs. When this pin is low, the headphone output is either a line output or is completely
     * disabled, according to the chip's configuration.
     */
    Nilai::Pin HeadphoneSelect = {};

    /**
     * @brief [Output] Mute pin, active low.
     *
     * Setting this pin to low sets the outputs to 50% duty cycle.
     * Setting this pin to high returns the volume to normal.
     *
     * From the
     * <a
href=https://www.ti.com/lit/ds/symlink/tas5705.pdf?HQS=dis-dk-null-digikeymode-dsf-pf-null-wwe&ts=1643908584509&ref_url=https%253A%252F%252Fwww.ti.com%252Fgeneral%252Fdocs%252Fsuppproductinfo.tsp%253FdistId%253D10%2526gotoUrl%253Dhttps%253A%252F%252Fwww.ti.com%252Flit%252Fgpn%252Ftas5705)>datasheet</a>:
     *  The mute control provides a noiseless volume ramp to silence. Releasing mute provides a
     *  noiseless ramp to previous volume.
     */
    Nilai::Pin Mute = {};

    /**
     * @brief [Output] Pin that controls the PVDD supply of the chip.
     *
     * Setting this pin to low disables PVDD.
     * Setting this pin to high enabled PVDD.
     */
    Nilai::Pin PVddEn = {};

    /**
     * @brief Checks that the mandatory information to use the chip are properly set.
     * @return True if I2cHandle is not null, and that Reset and PwrDwn are not
     * empty.
     */
    [[nodiscard]] bool IsValid() const
    {
        return ((I2cHandle != nullptr) && (Reset != Nilai::Pin {}) && (PwrDwn != Nilai::Pin {}));
    }
};
}    // namespace Nilai::Tas5707

#endif
#endif    // NILAI_TAS5707_HARDWARECONFIG_H
