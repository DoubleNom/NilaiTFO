/**
 * @file    GenericEvent.h
 * @author  Samuel Martel
 * @date    2022-03-03
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
#ifndef NILAI_EVENTS_GENERICEVENT_H
#define NILAI_EVENTS_GENERICEVENT_H

#if defined(NILAI_USE_EVENTS)

#    include "../../defines/internalConfig.h"
#    include "../../services/Time.h"
#    include "Types.h"

#    include <cstdint>

namespace cep::Events
{
/**
 * @brief Generic event structure.
 */
struct Event
{
    Event(EventTypes t, EventCategories c) : Timestamp(cep::GetTime()), Type(t), Category(c) {}
    virtual ~Event() = default;

    uint32_t        Timestamp = 0;
    EventTypes      Type;
    EventCategories Category;
};
}    // namespace cep::Events
#endif

#endif    // NILAI_EVENTS_GENERICEVENT_H
