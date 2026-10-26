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

#include "source2mod/sdk/schema/classes/CPlayer_WeaponServices.h"

#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/utils/virtual.h"
SOURCE2MOD_GLOBALVARS();

// The gamedata keys still name CCSPlayer_WeaponServices: that is the class the
// vtable indices and the Destroy signature were read off. Every weapon services
// object the game hands out is one, so the calls are valid through the base
// pointer CBasePlayerPawn stores.

bool CPlayer_WeaponServices::CanUse(CBasePlayerWeapon* pWeapon)
{
    return SOURCE2MOD_CALL_FUNCTION(bool(CPlayer_WeaponServices*, CBasePlayerWeapon*), "CCSPlayer_WeaponServices::CanUse", HookChain::Run, this,
                                    pWeapon);
}

void CPlayer_WeaponServices::DropWeapon(CBasePlayerWeapon *pWeapon, Vector *pVecTarget, Vector *pVelocity, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CPlayer_WeaponServices*, CBasePlayerWeapon*, Vector*, Vector*), "CCSPlayer_WeaponServices::DropWeapon", eChain,
                             this, pWeapon, pVecTarget, pVelocity);
}

int CPlayer_WeaponServices::BumpWeapon(CBasePlayerWeapon* pWeapon, HookChain eChain)
{
    return SOURCE2MOD_CALL_FUNCTION(int(CPlayer_WeaponServices*, CBasePlayerWeapon*), "CCSPlayer_WeaponServices::BumpWeapon", eChain, this, pWeapon);
}

void CPlayer_WeaponServices::SelectItem(CBasePlayerWeapon* pWeapon, int unk1, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(bool(CPlayer_WeaponServices*, CBasePlayerWeapon*, int), "CCSPlayer_WeaponServices::SelectItem", eChain, this, pWeapon,
                             unk1);
}

void CPlayer_WeaponServices::Destroy(CBasePlayerWeapon* pWeapon, HookChain eChain)
{
#ifndef _WIN32
    if(pWeapon)
#endif
    {
        SOURCE2MOD_CALL_FUNCTION(void(CPlayer_WeaponServices*, CBasePlayerWeapon*), "CCSPlayer_WeaponServices::Destroy", eChain, this, pWeapon);
    }
}
