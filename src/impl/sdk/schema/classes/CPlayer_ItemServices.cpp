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

#include "source2mod/sdk/schema/classes/CPlayer_ItemServices.h"

#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/utils/virtual.h"
SOURCE2MOD_GLOBALVARS();

// The gamedata keys still name CCSPlayer_ItemServices: that is the class the
// vtable index was read off. Every item services object the game hands out is
// one, so the call is valid through the base pointer CBasePlayerPawn stores.

void CPlayer_ItemServices::DropActivePlayerWeapon(CBasePlayerWeapon* pActiveWeapon, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CPlayer_ItemServices*, CBasePlayerWeapon*), "CCSPlayer_ItemServices::DropActivePlayerWeapon", eChain, this,
                             pActiveWeapon);
}

void CPlayer_ItemServices::RemoveWeapons(bool bRemoveSuit, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CPlayer_ItemServices*, bool), "CCSPlayer_ItemServices::RemoveWeapons", eChain, this, bRemoveSuit);
}

CBasePlayerWeapon* CPlayer_ItemServices::GiveNamedItem(const char* pszItem, HookChain eChain)
{
    // A knife skin; weapon_knife and weapon_knife_t are what anybody may have.
    if (pszItem && g_pSource2ModAPI->FollowsServerGuidelines()
        && ((!strncmp(pszItem, "weapon_knife_", 13) && strcmp(pszItem, "weapon_knife_t")) || !strcmp(pszItem, "weapon_bayonet")))
    {
        g_pSource2ModAPI->Log("Cannot give %s with \"FollowCS2ServerGuidelines\" enabled.\n", pszItem);
        return nullptr;
    }

    // The implementation behind the virtual, called with the arguments the
    // virtual's thunk passes. Going through it rather than the vtable is what
    // lets HookChain::Run reach a hook on "CCSPlayer_ItemServices::GiveNamedItem".
    auto* pThis = reinterpret_cast<CCSPlayer_ItemServices*>(this);
    return SOURCE2MOD_CALL_FUNCTION(CBasePlayerWeapon*(CCSPlayer_ItemServices*, const char*, int, CEconItemView*, bool, void*),
                                    "CCSPlayer_ItemServices::GiveNamedItem", eChain, pThis, pszItem, 0, nullptr, false, nullptr);
}
