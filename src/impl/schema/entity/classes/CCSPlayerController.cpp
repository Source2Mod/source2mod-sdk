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
#include "source2mod/schema/entity/classes/CCSPlayer_ItemServices.h"
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
    // 64-bit flags: the old int casts dropped everything above bit 31.
    info.m_nDamageFlags = static_cast<TakeDamageFlags_t>(static_cast<uint64_t>(info.m_nDamageFlags()) | DFLAG_SUPPRESS_DAMAGE_MODIFICATION);

    CTakeDamageResult result(&info);

    // Bypass by default: damage Source2Mod was asked to deal is not the game
    // dealing damage, so a plugin hooking TakeDamage to police the game's own
    // hits should not have to tell the two apart -- and a handler that
    // supercedes would otherwise silently swallow this. HookChain::Run is for
    // a caller standing in for a weapon.
    //
    // On the pawn, not the controller: the tracing showed a real hit arriving
    // on classname 'player' while this call arrived on 'cs_player_controller',
    // whose m_iHealth is 0.
    SOURCE2MOD_CALL_FUNCTION(int64_t(CBaseEntity*, CTakeDamageInfo*, CTakeDamageResult*), "CBaseEntity::TakeDamageOld", eChain, pVictimPawn,
                             &info, &result);
}

void CCSPlayerController::Respawn(HookChain eChain)
{
    if (!m_hPlayerPawn()) return;

    // The Call To Arms update appears to have invalidated the need for CCSPlayerPawn_Respawn.
    SetPawn(m_hPlayerPawn(), eChain);
    SOURCE2MOD_CALL_FUNCTION(void(CCSPlayerController*), "CCSPlayerController::Respawn", eChain, this);
}

void CCSPlayerController::SwitchTeam(int nTeam, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CCSPlayerController*, int), "CCSPlayerController::SwitchTeam", eChain, this, nTeam);
}

void CCSPlayerController::ChangeTeam(int nTeam, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CCSPlayerController*, int), "CCSPlayerController::ChangeTeam", eChain, this, nTeam);
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

CSPlayerState CCSPlayerController::GetPlayerPawnState()
{
    CCSPlayerPawn* pPawn = GetPlayerPawn();
    return pPawn ? pPawn->m_iPlayerState() : CSPlayerState::STATE_WELCOME;
}

// The player pawn while it lives: what the health and armor helpers work on.
static CCSPlayerPawn* LivingPlayerPawn(CCSPlayerController* pController)
{
    CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
    return pPawn && pController->m_bPawnIsAlive() ? pPawn : nullptr;
}

int CCSPlayerController::GetHealth()
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    return pPawn ? pPawn->m_iHealth() : 0;
}

int CCSPlayerController::GetMaxHealth()
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    return pPawn ? pPawn->m_iMaxHealth() : 0;
}

void CCSPlayerController::SetHealth(int nHealth)
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    if (!pPawn || nHealth < 1)
        return;
    pPawn->m_iHealth(nHealth);
    m_iPawnHealth(static_cast<uint32_t>(nHealth));
    if (nHealth > pPawn->m_iMaxHealth())
        pPawn->m_iMaxHealth(nHealth);
}

void CCSPlayerController::AddHealth(int nDelta)
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    if (!pPawn)
        return;
    const int nHealth = pPawn->m_iHealth() + nDelta;
    if (nHealth < 1)
        return;
    pPawn->m_iHealth(nHealth);
    m_iPawnHealth(static_cast<uint32_t>(nHealth));
}

void CCSPlayerController::SetMaxHealth(int nMaxHealth)
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    if (pPawn && nMaxHealth >= 1)
        pPawn->m_iMaxHealth(nMaxHealth);
}

void CCSPlayerController::SetArmor(int nArmor, bool bHelmet)
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    if (!pPawn || nArmor < 0)
        return;
    pPawn->m_ArmorValue(nArmor);
    m_iPawnArmor(nArmor);
    if (bHelmet && pPawn->m_pItemServices())
        static_cast<CCSPlayer_ItemServices*>(pPawn->m_pItemServices())->m_bHasHelmet(true);
}

void CCSPlayerController::AddArmor(int nDelta)
{
    CCSPlayerPawn* pPawn = LivingPlayerPawn(this);
    if (!pPawn)
        return;
    int nArmor = pPawn->m_ArmorValue() + nDelta;
    if (nArmor < 0)
        nArmor = 0;
    pPawn->m_ArmorValue(nArmor);
    m_iPawnArmor(nArmor);
}
