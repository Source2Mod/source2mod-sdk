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

/**
 * @file ISource2ModClients.h
 * @brief A player by slot, off the engine's own client (CServerSideClient):
 *        state, SteamIDs in every form, connection details, client cvars and
 *        ConVar queries, messages and commands to one client, disconnecting.
 *
 * Nothing here needs a controller, so it also answers for a client that has
 * none (yet). The controllers' per-client helpers (PrintTo*, ReplicateConVar,
 * GetIpAddress, Disconnect, ...) call these with their slot; what only the
 * entity has (networked name and SteamID, pawn, team) stays on it.
 */

#ifndef _INCLUDE_ISOURCE2MOD_CLIENTS_H
#define _INCLUDE_ISOURCE2MOD_CLIENTS_H

#pragma once
#include "ISource2ModPlugin.h"

#include <string>

/// SteamID forms (SourceMod's AuthIdType values).
enum ESource2ModAuthId
{
    SOURCE2MOD_AUTHID_ENGINE = 0,    ///< as the engine shows it
    SOURCE2MOD_AUTHID_STEAM2,        ///< STEAM_1:0:12345
    SOURCE2MOD_AUTHID_STEAM3,        ///< [U:1:24690]
    SOURCE2MOD_AUTHID_STEAMID64,     ///< 76561197960290418
};

class CServerSideClient;
class IGameEvent;

/* =========================
Core Source2Mod Clients
========================= */

#define SOURCE2MOD_CLIENTS_INTERFACE "ISource2ModClients002"

class ISource2ModClients
{
public:
    virtual ~ISource2ModClients() = default;

    /**
     * @brief The engine's client of a slot: signon state, net channel, fake
     * flag, what the server knows of it before (and after) it has a controller.
     * @return nullptr when the slot has no client (the engine keeps the slot's
     *         object between clients; that one does not count).
     */
    virtual CServerSideClient* GetServerSideClient(CPlayerSlot slot) = 0;

    /// A slot has a client (connecting or in game).
    virtual bool IsConnected(CPlayerSlot slot) = 0;
    /// A slot's client is fully in game.
    virtual bool IsInGame(CPlayerSlot slot) = 0;
    /// A bot, by the engine client's own flag (true before it has a controller).
    virtual bool IsFakeClient(CPlayerSlot slot) = 0;
    /// Steam has confirmed the client's SteamID.
    virtual bool IsAuthorized(CPlayerSlot slot) = 0;

    /**
     * @brief A client's SteamID as text.
     * @param validated Only once Steam has confirmed it (else empty).
     * @return Empty for a bot or no client.
     */
    virtual std::string GetAuthId(CPlayerSlot slot, ESource2ModAuthId form, bool validated = true) = 0;

    /// The client's IPv4 address without the port; empty for a bot or no client.
    virtual std::string GetIpAddress(CPlayerSlot slot) = 0;
    /// Average round trip in seconds (0 for a bot or no client).
    virtual float GetLatency(CPlayerSlot slot) = 0;
    /// Seconds connected (0 for a bot or no client).
    virtual float GetConnectionTime(CPlayerSlot slot) = 0;

    /// A cvar the client sent (cl_language, ...); empty when it did not.
    virtual std::string GetClientConVar(CPlayerSlot slot, const char* name) = 0;

    /**
     * @brief Asks a client for any of its ConVars (SVC_GetCvarValue).
     *
     * Unlike GetClientConVar, which reads the copy of a FCVAR_USERINFO cvar
     * the client sent on its own, this goes over the wire and back: the
     * callback runs when the client answers, or with CANCELLED when it
     * disconnects first. A client that never answers never calls it, so a
     * caller that needs an answer keeps its own timeout.
     * ISource2ModConVars::QueryClientConVar is the same call.
     *
     * @return A cookie the callback is handed back, or -1 (a bot, no client,
     *         no net channel) -- the callback is then never called.
     */
    virtual int QueryClientConVar(CPlayerSlot slot, const char* name, ClientConVarQueryCallback callback) = 0;

    /**
     * @brief Sets a replicated ConVar's value on one client only (NET_SetConVar);
     * the server's own value and every other client stay as they are.
     * ISource2ModConVars::ReplicateConVar is the same call.
     * @return false when the slot has no net channel (a bot, no client).
     */
    virtual bool ReplicateConVar(CPlayerSlot slot, const char* name, const char* value) = 0;

    /// Text on one client (TextMsg): its console (a newline is added), chat,
    /// center text or center alert.
    virtual void PrintToConsole(CPlayerSlot slot, const char* message) = 0;
    virtual void PrintToChat(CPlayerSlot slot, const char* message) = 0;
    virtual void PrintToCenter(CPlayerSlot slot, const char* message) = 0;
    virtual void PrintToCenterAlert(CPlayerSlot slot, const char* message) = 0;

    /**
     * @brief CS2's HTML center text (show_survival_respawn_status) on one client.
     * @param menu false: left out while the player has a menu open, so the
     *             text cannot paint over it; a menu draws itself with true.
     */
    virtual void PrintToCenterHtml(CPlayerSlot slot, const char* message, int duration = 5, bool menu = false) = 0;

    /// Runs a command on the client's own console, as if typed there.
    virtual void ExecuteClientCommand(CPlayerSlot slot, const char* command) = 0;
    /// Runs a server ConCommand on the client's behalf (its slot as the
    /// command context); nothing goes over the wire. Both are on
    /// ISource2ModCommands too.
    virtual void ExecuteClientCommandFromServer(CPlayerSlot slot, const char* command) = 0;

    /// Fires @p event to this client's legacy listener only. Does not take
    /// ownership: the caller still frees it. ISource2ModEvents::FireEventToClient
    /// is the same call.
    virtual void FireEventToClient(CPlayerSlot slot, IGameEvent* event) = 0;

    /**
     * @brief Disconnects a client, on the next frame: safe to call from inside
     * that client's own hooks (its string commands, its net messages). Its open
     * menu is closed right away.
     * @param reason         What CS2 shows the client, in its own text
     *                       (NETWORK_DISCONNECT_KICKED for a kick).
     * @param internalReason What the server logs; nullptr for none.
     */
    virtual void Disconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* internalReason = nullptr) = 0;

    /// Clients on the server, bots included.
    virtual int GetClientCount(bool inGameOnly = true) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_CLIENTS_H
