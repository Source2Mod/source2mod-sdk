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

#include "source2mod/sdk/schema/classes/CBasePlayerController.h"

#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/sdk/schema/classes/CCSPlayerPawn.h"
#include "source2mod/sdk/schema/classes/CCSPlayerPawnBase.h"
#include "source2mod/sdk/schema/classes/CPlayer_ObserverServices.h"

#include <engine2/serversideclient.h>
#include <tier0/dbg.h>
SOURCE2MOD_GLOBALVARS();

#include "engine2/networkgameserverbase.h"
#include "engine2/inetworkserverservice.h"
#include "networksystem/inetworkmessages.h"
#include "usermessages.pb.h"

void CBasePlayerController::SetPawn(CBasePlayerPawn* pPawn, HookChain eChain){
    SOURCE2MOD_CALL_FUNCTION(void(CBasePlayerController*, CBasePlayerPawn*, bool, bool, bool, bool), "CBasePlayerController::SetPawn", eChain,
                             this, pPawn, true, false, false, false);
}

void CBasePlayerController::PrintToConsole(const char* pszMessage)
{
    g_pSource2ModClients->PrintToConsole(GetPlayerSlot(), pszMessage);
}

void CBasePlayerController::PrintToChat(const char* pszMessage)
{
    g_pSource2ModClients->PrintToChat(GetPlayerSlot(), pszMessage);
}

void CBasePlayerController::PrintToCenter(const char* pszMessage)
{
    g_pSource2ModClients->PrintToCenter(GetPlayerSlot(), pszMessage);
}

void CBasePlayerController::PrintToCenterAlert(const char* pszMessage)
{
    g_pSource2ModClients->PrintToCenterAlert(GetPlayerSlot(), pszMessage);
}

bool CBasePlayerController::IsBot()
{
    return (m_fFlags & FL_FAKECLIENT) != 0;
}

void CBasePlayerController::Disconnect(ENetworkDisconnectionReason eReason, const char* pszInternalReason)
{
    g_pSource2ModClients->Disconnect(GetPlayerSlot(), eReason, pszInternalReason);
}

void CBasePlayerController::ExecuteClientCommand(const char* pszCommand)
{
    g_pSource2ModClients->ExecuteClientCommand(GetPlayerSlot(), pszCommand);
}

void CBasePlayerController::ExecuteClientCommandFromServer(const char* pszCommand)
{
    g_pSource2ModClients->ExecuteClientCommandFromServer(GetPlayerSlot(), pszCommand);
}

CServerSideClient* CBasePlayerController::GetServerSideClient()
{
    return g_pSource2ModClients->GetServerSideClient(GetPlayerSlot());
}

CEntityIndex CBasePlayerController::GetPlayerIndex()
{
    return GetEntityIndex();
}

int CBasePlayerController::GetSlot()
{
    return GetIndex() - 1;
}

CPlayerSlot CBasePlayerController::GetPlayerSlot()
{
    return CPlayerSlot(GetSlot());
}

int CBasePlayerController::GetUserID()
{
    return GetEngineServer()->GetPlayerUserId(GetPlayerSlot()).Get();
}

CPlayerUserId CBasePlayerController::GetPlayerUserID()
{
    return GetEngineServer()->GetPlayerUserId(GetPlayerSlot());
}

uint64 CBasePlayerController::GetSteamID()
{
    return m_steamID();
}

CSteamID CBasePlayerController::GetPlayerSteamID()
{
    return CSteamID(static_cast<uint64>(m_steamID()));
}

const char* CBasePlayerController::GetPlayerName()
{
    return m_iszPlayerName();
}

CUtlString CBasePlayerController::GetIpAddress()
{
    return CUtlString(g_pSource2ModClients->GetIpAddress(GetPlayerSlot()).c_str());
}

void CBasePlayerController::ReplicateConVar(const char* pszConVar, const char* pszValue)
{
    g_pSource2ModClients->ReplicateConVar(GetPlayerSlot(), pszConVar, pszValue);
}

void CBasePlayerController::FireEventToClient(IGameEvent* pEvent)
{
    g_pSource2ModClients->FireEventToClient(GetPlayerSlot(), pEvent);
}

// m_hPawn is typed CHandle<CBasePlayerPawn>, but every pawn this game puts on a
// controller is a CCSPlayerPawn -- the downcast is what callers would otherwise
// have to write at each use.
CCSPlayerPawn* CBasePlayerController::GetPawn()
{
    if (auto handle = m_hPawn(); handle.IsValid())
        return static_cast<CCSPlayerPawn*>(handle.Get());
    return nullptr;
}

void CBasePlayerController::SetPlayerName(const char* pszName)
{
    char* pszBuffer = m_iszPlayerName();
    if (!pszBuffer)
        return;
    V_strncpy(pszBuffer, pszName ? pszName : "", 128);
    m_iszPlayerName.NetworkStateChanged();
}

bool CBasePlayerController::IsConnected()
{
    return m_iConnected() == PlayerConnectedState::Connected;
}

CSPlayerState CBasePlayerController::GetPawnState()
{
    // m_hPawn is the player pawn or the observer pawn; both are CCSPlayerPawnBase.
    auto handle = m_hPawn();
    CBasePlayerPawn* pPawn = handle.IsValid() ? handle.Get() : nullptr;
    if (!pPawn)
        return CSPlayerState::STATE_WELCOME;
    return static_cast<CCSPlayerPawnBase*>(pPawn)->m_iPlayerState();
}

CBaseEntity* CBasePlayerController::GetObserverTarget()
{
    auto handle = m_hPawn();
    CBasePlayerPawn* pPawn = handle.IsValid() ? handle.Get() : nullptr;
    CPlayer_ObserverServices* pObserver = pPawn ? pPawn->m_pObserverServices() : nullptr;
    if (!pObserver)
        return nullptr;
    auto target = pObserver->m_hObserverTarget();
    return target.IsValid() ? target.Get() : nullptr;
}
