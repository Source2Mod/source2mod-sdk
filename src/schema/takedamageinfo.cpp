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

// Constructors and destructors stay in every binary (a plugin builds the
// object itself). The default constructor and the destructors are the game's
// own through the schema class ops; the damage constructor is the game's
// CTakeDamageInfo constructor from gamedata, which also fills m_AttackerInfo
// from the attacker (pawn, world, slot, team) exactly as the game does.

#include "source2mod/schema/takedamageinfo.h"
#include "source2mod/schema/takedamageresult.h"
#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/ISource2ModFunctions.h"

#include <cstring>

SOURCE2MOD_GLOBALVARS();

namespace
{
    // The game's object is built into the class's own storage: say so, once,
    // if the running game's class has outgrown it.
    void CheckStorage(const char* pszClassName, size_t nStorage)
    {
        const int32_t nSize = schema::GetClassSize(pszClassName);
        if (nSize > static_cast<int32_t>(nStorage))
            Warning("%s is %d bytes in the running game, the SDK reserves %zu: update the SDK\n", pszClassName, nSize, nStorage);
    }

    bool HasClassOps(const char* pszClassName)
    {
        return schema::GetClassOps(pszClassName) != nullptr;
    }

    void ConstructTakeDamageInfo(CTakeDamageInfo* pThis, CBaseEntity* pInflictor, CBaseEntity* pAttacker, CBaseEntity* pAbility, float flDamage, DamageTypes_t bitsDamageType)
    {
        SOURCE2MOD_CALL_FUNCTION(void(CTakeDamageInfo*, CBaseEntity*, CBaseEntity*, CBaseEntity*, const Vector*, const Vector*, float, int, int, void*), "CTakeDamageInfo::Constructor", HookChain::Run,
                                 pThis, pInflictor, pAttacker, pAbility, &vec3_origin, &vec3_origin, flDamage, (int)bitsDamageType, 0, nullptr);
    }
}

CTakeDamageInfo::CTakeDamageInfo()
{
    static const bool s_bHasOps = (CheckStorage("CTakeDamageInfo", sizeof(*this)), HasClassOps("CTakeDamageInfo"));

    if (s_bHasOps)
        SchemaConstruct();
    else
        ConstructTakeDamageInfo(this, nullptr, nullptr, nullptr, 0.f, static_cast<DamageTypes_t>(0));
}

CTakeDamageInfo::CTakeDamageInfo(CBaseEntity* pInflictor, CBaseEntity* pAttacker, CBaseEntity* pAbility, float flDamage, DamageTypes_t bitsDamageType)
{
    static const bool s_bChecked = (CheckStorage("CTakeDamageInfo", sizeof(*this)), true);
    (void)s_bChecked;

    ConstructTakeDamageInfo(this, pInflictor, pAttacker, pAbility, flDamage, bitsDamageType);
}

CTakeDamageInfo::~CTakeDamageInfo()
{
    // Without the class ops there is no game destructor to call, as before.
    SchemaDestruct();
}

CTakeDamageResult::CTakeDamageResult(float damage)
{
    Init(nullptr, damage);
}

CTakeDamageResult::CTakeDamageResult(CTakeDamageInfo* pInfo)
{
    Init(pInfo, pInfo ? static_cast<float>(pInfo->m_flDamage) : 0.f);
}

void CTakeDamageResult::Init(CTakeDamageInfo* pInfo, float damage)
{
    static const bool s_bHasOps = (CheckStorage("CTakeDamageResult", sizeof(*this)), HasClassOps("CTakeDamageResult"));

    // The game's defaults first; the old hand-made layout left the rest of
    // the object uninitialised.
    if (s_bHasOps)
        SchemaConstruct();
    else
        std::memset(static_cast<void*>(this), 0, sizeof(*this));

    m_pOriginatingInfo = pInfo;
    m_nHealthLost = static_cast<int32_t>(damage);
    m_nHealthBefore = 0;
    m_flDamageDealt = damage;
    m_flPreModifiedDamage = damage;
    m_nTotalledHealthLost = static_cast<int32_t>(damage);
    m_flTotalledDamageDealt = damage;
    m_bWasDamageSuppressed = false;
}

CTakeDamageResult::~CTakeDamageResult()
{
    SchemaDestruct();
}
