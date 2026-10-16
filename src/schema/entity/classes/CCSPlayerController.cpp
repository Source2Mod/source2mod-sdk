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

#include "source2mod/schema/entity/classes/CCSPlayerController.h"

#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModMenus.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/schema/entity/classes/CCSObserverPawn.h"
#include "source2mod/schema/entity/classes/CCSPlayerPawn.h"
#include "source2mod/schema/takedamageinfo.h"
#include "source2mod/schema/takedamageresult.h"
#include "source2mod/utils/virtual.h"

#include <tier0/dbg.h>
SOURCE2MOD_GLOBALVARS();

#include "engine2/networkgameserverbase.h"
#include "engine2/inetworkserverservice.h"

CCSPlayerController *CCSPlayerController::FromPawn(CCSPlayerPawn* pPawn)
{
    return static_cast<CCSPlayerController*>(pPawn->m_hController().Get());
}

CCSPlayerController *CCSPlayerController::FromSlot(int iSlot)
{
    // Not bounded by maxClients: some servers report 0 there with players in
    // game (a Windows DS started through metamod-launcher). The class check
    // below is what keeps a slot past the players from handing out another
    // entity as a controller.
    if (iSlot < 0 || iSlot >= ABSOLUTE_PLAYER_LIMIT || !GetEntitySystem())
        return nullptr;

    CBaseEntity* entity = FromIndex(iSlot + 1);
    const char* classname = entity ? entity->GetClassname() : nullptr;
    if (!classname || V_strcmp(classname, "cs_player_controller") != 0)
        return nullptr;

    return static_cast<CCSPlayerController*>(entity);
}

CCSPlayerController *CCSPlayerController::FromSlot(CPlayerSlot slot)
{
    if (!slot.IsValid())
        return nullptr;

    return FromSlot(slot.Get());
}

CCSPlayerController *CCSPlayerController::FromUserId(int iUserId)
{
    for (int i = 0; i < GetGlobalVars()->maxClients; ++i)
    {
        CCSPlayerController* controller = FromSlot(i);
        if (!controller)
            continue;

        if (iUserId == GetEngineServer()->GetPlayerUserId(i).Get()) return controller;
    }
    return nullptr;
}

CCSPlayerController *CCSPlayerController::FromUserId(CPlayerUserId userId)
{
    return FromUserId(userId.Get());
}

CCSPlayerController *CCSPlayerController::FromSteamId(uint64 uSteamId)
{
    for (int i = 0; i < GetGlobalVars()->maxClients; ++i)
    {
        CCSPlayerController* controller = FromSlot(i);
        if (!controller)
            continue;

        if (uSteamId == controller->m_steamID()) return controller;
    }
    return nullptr;
}

CCSPlayerController *CCSPlayerController::FromSteamId(CSteamID steamId)
{
    return FromSteamId(steamId.ConvertToUint64());
}

void CCSPlayerController::PrintToCenterHtml(const char* pszMessage, int iDuration, bool bMenu)
{
    if (!bMenu && g_pSource2ModMenus->GetActiveMenu(this))
        return;

    IGameEvent *event = GetGameEventManager()->CreateEvent("show_survival_respawn_status", true);
    if (!event) return;

    event->SetString("loc_token", pszMessage);
    event->SetInt("duration", iDuration);
    event->SetPlayer("userid", GetPlayerSlot());

    FireEventToClient(event);
    GetGameEventManager()->FreeEvent(event);
}

void CCSPlayerController::TakeDamage(CCSPlayerController* pAttacker, int iDamage, DamageTypes_t bitsDamageType, HookChain eChain)
{
    if (!m_bPawnIsAlive || m_iConnected() != PlayerConnectedState::Connected || !pAttacker || pAttacker->m_iConnected() != PlayerConnectedState::Connected)
        return;

    CCSPlayerPawn* pVictimPawn = GetPlayerPawn();
    if (!pVictimPawn) return;

    CCSPlayerPawn* pAttackerPawn = pAttacker->GetPlayerPawn();
    if (!pAttackerPawn) return;

    auto flDamage = static_cast<float>(iDamage);

    CTakeDamageInfo info(pVictimPawn, pAttackerPawn, nullptr, flDamage, bitsDamageType);
    info.m_nDamageFlags = static_cast<TakeDamageFlags_t>(static_cast<int>(info.m_nDamageFlags) | static_cast<int>(TakeDamageFlags_t::DFLAG_SUPPRESS_DAMAGE_MODIFICATION));

    CTakeDamageResult result(iDamage);
    result.CopyFrom(&info);

    auto pfn = ADDR_TAKE_DAMAGE_OLD();

    // Bypass by default: damage Source2Mod was asked to deal is not the game
    // dealing damage, so a plugin hooking TakeDamage to police the game's own
    // hits should not have to tell the two apart -- and a handler that
    // supercedes would otherwise silently swallow this. HookChain::Run is for
    // a caller standing in for a weapon.
    //
    // On the pawn, not the controller: the tracing showed a real hit arriving
    // on classname 'player' while this call arrived on 'cs_player_controller',
    // whose m_iHealth is 0.
    ResolveHookChain(pfn, eChain)(pVictimPawn, &info, &result);
}

void CCSPlayerController::Respawn(HookChain eChain)
{
    if (!m_hPlayerPawn()) return;

    // The Call To Arms update appears to have invalidated the need for CCSPlayerPawn_Respawn.
    SetPawn(m_hPlayerPawn(), eChain);
    static int offset = g_pSource2ModGameConfig->GetOffset("CCSPlayerController::Respawn");
    CALL_VIRTUAL_CHAIN(void, offset, eChain, this);
}

void CCSPlayerController::SwitchTeam(int nTeam, HookChain eChain)
{
    ResolveHookChain(ADDR_SWITCH_TEAM(), eChain)(this, nTeam);
}

void CCSPlayerController::ChangeTeam(int nTeam, HookChain eChain)
{
    static int offset = g_pSource2ModGameConfig->GetOffset("CCSPlayerController::ChangeTeam");
    CALL_VIRTUAL_CHAIN(void, offset, eChain, this, nTeam);
}

CCSPlayerPawn* CCSPlayerController::GetPlayerPawn()
{
    if (auto handle = m_hPlayerPawn(); handle.IsValid())
        return handle.Get();
    return nullptr;
}

CCSObserverPawn* CCSPlayerController::GetObserverPawn()
{
    if (auto handle = m_hObserverPawn(); handle.IsValid())
        return handle.Get();
    return nullptr;
}
