/**
* vim: set ts=4 sw=4 tw=99 noet:
 * =============================================================================
 * Source2Mod
 * Copyright (C) 2025-2026 Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl.
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
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
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
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

#ifndef _INCLUDE_CBASEPLAYERCONTROLLER_H
#define _INCLUDE_CBASEPLAYERCONTROLLER_H
#ifdef _WIN32
#pragma once
#endif

#include "../enums/ChatIgnoreType_t.h"
#include "../enums/CSPlayerState.h"
#include "../enums/PlayerConnectedState.h"
#include "CBaseEntity.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/sdk/game/server/entityio.h"
#include "source2mod/sdk/schemasystem_helper.h"

#include <ehandle.h>
#include <entityhandle.h>
#include <igameevents.h>
#include <utlbinaryblock.h>
#include <utlstring.h>
#include <utlstringtoken.h>
#include <utlsymbol.h>
#include <utlsymbollarge.h>
#include <vector.h>

#include <cstdint>

class CBasePlayerPawn;
class CCSPlayerPawn;
class CServerSideClient;

class CBasePlayerController : public CBaseEntity
{
public:
    DECLARE_SCHEMA_CLASS(CBasePlayerController);

    SCHEMA_FIELD(uint64_t, m_nInButtonsWhichAreToggles);
    SCHEMA_FIELD(uint32_t, m_nTickBase);
    SCHEMA_FIELD(CHandle<CBasePlayerPawn>, m_hPawn);
    SCHEMA_FIELD(bool, m_bKnownTeamMismatch);
    SCHEMA_FIELD(int32_t, m_nSplitScreenSlot);
    SCHEMA_FIELD(CHandle<CBasePlayerController>, m_hSplitOwner);
    SCHEMA_FIELD(CUtlVector<CHandle<CBasePlayerController>>, m_hSplitScreenPlayers);
    SCHEMA_FIELD(bool, m_bIsHLTV);
    SCHEMA_FIELD(PlayerConnectedState, m_iConnected);
    SCHEMA_FIELD(PlayerConnectedState, m_iMostConnected);
    SCHEMA_FIELD_POINTER(char, m_iszPlayerName);
    SCHEMA_FIELD(CUtlString, m_szNetworkIDString);
    SCHEMA_FIELD(float, m_fLerpTime);
    SCHEMA_FIELD(bool, m_bLagCompensation);
    SCHEMA_FIELD(bool, m_bPredict);
    SCHEMA_FIELD(bool, m_bIsLowViolence);
    SCHEMA_FIELD(bool, m_bGamePaused);
    SCHEMA_FIELD(ChatIgnoreType_t, m_iIgnoreGlobalChat);
    SCHEMA_FIELD(float, m_flLastPlayerTalkTime);
    SCHEMA_FIELD(float, m_flLastEntitySteadyState);
    SCHEMA_FIELD(int32_t, m_nAvailableEntitySteadyState);
    SCHEMA_FIELD(bool, m_bHasAnySteadyStateEnts);
    SCHEMA_FIELD(uint64_t, m_steamID);
    SCHEMA_FIELD(bool, m_bNoClipEnabled);
    SCHEMA_FIELD(uint32_t, m_iDesiredFOV);

public:
    /// <summary>Set pawn for controller.</summary>
    void SetPawn(CBasePlayerPawn* pPawn, HookChain eChain = HookChain::Run);
    /// <summary>Get pawn.</summary>
    CCSPlayerPawn* GetPawn();
    /// <summary>Print to console.</summary>
    void PrintToConsole(const char* pszMessage);
    /// <summary>Print to chat.</summary>
    void PrintToChat(const char* pszMessage);
    /// <summary>Print to center.</summary>
    void PrintToCenter(const char* pszMessage);
    /// <summary>Print alert.</summary>
    void PrintToCenterAlert(const char* pszMessage);
    /// <summary>Is bot.</summary>
    bool IsBot();
    /// <summary>Disconnect player.</summary>
    void Disconnect(ENetworkDisconnectionReason eReason, const char* pszInternalReason = nullptr);
    /// <summary>Execute client command.</summary>
    void ExecuteClientCommand(const char* pszCommand);
    /// <summary>Execute command from server.</summary>
    void ExecuteClientCommandFromServer(const char* pszCommand);
    /// <summary>Get server side engine client.</summary>
    CServerSideClient* GetServerSideClient();
    /// <summary>Get player index.</summary>
    CEntityIndex GetPlayerIndex();
    /// <summary>Get slot.</summary>
    int GetSlot();
    /// <summary>Get player slot.</summary>
    CPlayerSlot GetPlayerSlot();
    /// <summary>Get steamid.</summary>
    int GetUserID();
    /// <summary>Get player userid.</summary>
    CPlayerUserId GetPlayerUserID();
    /// <summary>Get steamid.</summary>
    uint64 GetSteamID();
    /// <summary>Get player steamid.</summary>
    CSteamID GetPlayerSteamID();
    /// <summary>Get player name.</summary>
    const char* GetPlayerName();
    /// <summary>Get IP address.</summary>
    CUtlString GetIpAddress();
    /// <summary>Replicate convar.</summary>
    void ReplicateConVar(const char* pszConVar, const char* pszValue);
    /// <summary>Fires gameEvent to client's legacy listener. Does not take ownership: the caller still frees pEvent.</summary>
    void FireEventToClient(IGameEvent* pEvent);
    /// <summary>Overwrites the networked player name (m_iszPlayerName, 128 bytes, cut to fit).</summary>
    void SetPlayerName(const char* pszName);
    /// <summary>Whether m_iConnected is PlayerConnectedState::Connected.</summary>
    bool IsConnected();
    /// <summary>m_iPlayerState of the current pawn (m_hPawn: the player or the observer pawn); STATE_WELCOME without one.</summary>
    CSPlayerState GetPawnState();
    /// <summary>Who the current pawn is spectating (its observer services' m_hObserverTarget), null when nobody.</summary>
    CBaseEntity* GetObserverTarget();

public:
    static CBasePlayerController* New(const char* className)
    {
        return CBaseEntity::New<CBasePlayerController>(className);
    }

    static CBasePlayerController* FromIndex(int iIndex)
    {
        return CBaseEntity::FromIndex<CBasePlayerController>(iIndex);
    }

    static CBasePlayerController* FromIndex(CEntityIndex index)
    {
        return FromIndex(index.Get());
    }

    CHandle<CBasePlayerController> GetHandle()
    {
        return CBaseEntity::GetHandle<CBasePlayerController>();
    }
};

#endif // _INCLUDE_CBASEPLAYERCONTROLLER_H
