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

// Constructors and the destructor stay in every binary (a plugin builds the
// object itself). The game's default constructor and destructor through the
// schema class ops, then the damage set as the game's own results start out.

#include "source2mod/sdk/game/shared/takedamageresult.h"
#include "source2mod/sdk/game/shared/takedamageinfo.h"
#include "source2mod/ISource2ModPlugin.h"

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
    static const bool s_bHasOps = (CheckStorage("CTakeDamageResult", sizeof(*this)), schema::GetClassOps("CTakeDamageResult") != nullptr);

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
