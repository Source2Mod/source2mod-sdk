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

// Constructors stay in every binary (a plugin builds the object itself); the
// engine's constructor they call is the gamedata function.

#include "source2mod/schema/takedamageinfo.h"
#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/ISource2ModFunctions.h"

SOURCE2MOD_GLOBALVARS();

CTakeDamageInfo::CTakeDamageInfo()
{
    SOURCE2MOD_CALL_FUNCTION(void(CTakeDamageInfo*, CBaseEntity*, CBaseEntity*, CBaseEntity*, const Vector*, const Vector*, float, int, int, void*), "CTakeDamageInfo::Constructor", HookChain::Run,
                             this, nullptr, nullptr, nullptr, &vec3_origin, &vec3_origin, 0.f, 0, 0, nullptr);
}

CTakeDamageInfo::CTakeDamageInfo(CBaseEntity* pInflictor, CBaseEntity* pAttacker, CBaseEntity* pAbility, float flDamage, DamageTypes_t bitsDamageType)
{
    SOURCE2MOD_CALL_FUNCTION(void(CTakeDamageInfo*, CBaseEntity*, CBaseEntity*, CBaseEntity*, const Vector*, const Vector*, float, int, int, void*), "CTakeDamageInfo::Constructor", HookChain::Run,
                             this, pInflictor, pAttacker, pAbility, &vec3_origin, &vec3_origin, flDamage, (int)bitsDamageType, 0, nullptr);
}
