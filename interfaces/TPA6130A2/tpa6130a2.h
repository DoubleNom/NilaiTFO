/**
 * @file    tpa6130a2.h
 * @author  Paul Thomas
 * @date    5/18/2023
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
#ifndef NILAITFO_INTERFACES_TPA6130A2_TPA6130A2_H
#define NILAITFO_INTERFACES_TPA6130A2_TPA6130A2_H

#if defined(NILAI_USE_TPA6130A2)
#    if !defined(NILAI_USE_I2S) && !defined(NILAI_USE_SAI)
#        error An audio device must be enabled to use the TAS5760 module!
#    endif

/**
 * @addtogroup Nilai
 * @{
 */

/**
 * @addtogroup Interfaces
 * @{
 */

/**
 * @addtogroup TPA6130A2
 * @{
 */

namespace Nilai::Interefaces
{

/**
 * @class Tpa6130a2
 * @brief Class representing a TPA6130A2 digital audio amplifier
 *
 * Refer to the <a
 * href=https://www.ti.com/lit/ds/symlink/tpa6130a2.pdf?ts=1684429745200>datasheet</a> for more
 * information.
 */

class Tpa6130a2 : public I2sModule
{
};


}    // namespace Nilai::Interefaces

#endif
#endif    // NILAITFO_INTERFACES_TPA6130A2_TPA6130A2_H
