/**
* vim: set ts=4 sw=4 tw=99 noet:
 * =============================================================================
 * Source2Mod
 * Copyright (C) 2025-2026 Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl.
 * All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 * gives you permission to link the code of this program
 * (as well as its derivative works) to "Counter-Strike 2," "Source 2,"
 * "Steam," and any Game MODs or server software running on software by
 * Valve Corporation. You must obey the GNU General Public License in all
 * respects for all other code used.
 *
 * Additionally, this exception applies to all derivative works unless
 * otherwise stated in LICENSE.txt.
 *
 * Authors:
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

#include "source2mod/sdk/game/shared/takedamageinfo.h"

#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
SOURCE2MOD_GLOBALVARS();

// The group of the hitbox the damage's trace hit: CTakeDamageInfo's trace (m_pTrace, not in the schema) -> CGameTrace::m_pHitbox -> CHitBox::m_nGroupId. No trace (fall, world, burn damage): invalid; a trace that hit no hitbox: generic.
HitGroup_t CTakeDamageInfo::GetHitGroup() const
{
    const CGameTrace* trace = m_pTrace();
    if (!trace)
        return HitGroup_t::HITGROUP_INVALID;

    const CHitBox* hitbox = trace->m_pHitbox;
    if (!hitbox)
        return HitGroup_t::HITGROUP_GENERIC;

    return static_cast<HitGroup_t>(hitbox->m_nGroupId);
}
