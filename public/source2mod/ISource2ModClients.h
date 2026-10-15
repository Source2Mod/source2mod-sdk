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
 * @brief Player helpers the controller classes do not have: SteamIDs in every
 *        form, connection details, client cvars, kicking with a message.
 *
 * What a CBasePlayerController already does (name, IP, SteamID64, user id,
 * console commands, team changes, Disconnect) stays there.
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

/* =========================
Core Source2Mod Clients
========================= */

#define SOURCE2MOD_CLIENTS_INTERFACE "ISource2ModClients001"

class ISource2ModClients
{
public:
    virtual ~ISource2ModClients() = default;

    /// A slot has a client (connecting or in game).
    virtual bool IsConnected(CPlayerSlot slot) = 0;
    /// A slot's client is fully in game.
    virtual bool IsInGame(CPlayerSlot slot) = 0;
    virtual bool IsFakeClient(CPlayerSlot slot) = 0;
    /// Steam has confirmed the client's SteamID.
    virtual bool IsAuthorized(CPlayerSlot slot) = 0;

    /**
     * @brief A client's SteamID as text.
     * @param validated Only once Steam has confirmed it (else empty).
     * @return Empty for a bot or no client.
     */
    virtual std::string GetAuthId(CPlayerSlot slot, ESource2ModAuthId form, bool validated = true) = 0;

    /// Average round trip in seconds (0 for a bot or no client).
    virtual float GetLatency(CPlayerSlot slot) = 0;
    /// Seconds connected (0 for a bot or no client).
    virtual float GetConnectionTime(CPlayerSlot slot) = 0;

    /// A cvar the client sent (cl_language, ...); empty when it did not.
    virtual std::string GetClientConVar(CPlayerSlot slot, const char* name) = 0;

    /// Disconnects a client with a message they are shown.
    virtual void Kick(CPlayerSlot slot, const char* message) = 0;

    /// Clients on the server, bots included.
    virtual int GetClientCount(bool inGameOnly = true) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_CLIENTS_H
