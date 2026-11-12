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

/**
 * @file ISource2ModSnapshots.h
 * @brief Snapshot inspector: what an entity sends to the clients, as the
 * engine encodes it.
 *
 * Schema reads tell what a field holds on the server. What a client gets is
 * something else: only networked fields, through their encoders (quantized
 * floats, packed vectors, handles), with the components (CBodyComponent, ...)
 * flattened in. These calls encode the entity the way the engine does for a
 * snapshot -- IFlattenedSerializers::Encode with the entity class's flattened
 * serializer -- and read that back: every field that goes out, by its network
 * name ("m_iHealth", "CBodyComponent.m_skeletonInstance.m_vecOrigin"), with the
 * value as the engine prints it.
 *
 * That is the full state a client would get for the entity now (a full
 * update), before send proxies leave fields out for particular players.
 * Encoding costs about what a snapshot of one entity does: fine for commands
 * and checks, not for every entity every tick. Game thread only.
 *
 * Got through the factory (g_pSource2ModSnapshots, filled by
 * SOURCE2MOD_FILLGAMEVARS); null on a core without it.
 */

#ifndef _INCLUDE_ISOURCE2MOD_SNAPSHOTS_H
#define _INCLUDE_ISOURCE2MOD_SNAPSHOTS_H

#pragma once

#include <source2mod/ISource2ModTypes.h>
#include "ISource2ModTypes.h"

#include <cstddef>
#include <cstdint>

class CEntityInstance;

/**
 * @brief One field an entity sends.
 *
 * @param path  The packed field path (index in the class's flattened
 *              serializer, then in nested serializers / arrays)
 * @param name  The network name, components and arrays included
 *              ("m_pInGameMoneyServices.m_iAccount", "m_perRoundStats[0].m_iKills")
 * @param type  The field's type ("int32", "CHandle< CBasePlayerPawn >")
 * @param value The value as the engine prints it ("16000", "Solman", "true")
 */
using SnapshotFieldCallback = Source2ModCallback<void(uint32_t path, const char* name, const char* type, const char* value)>;

/// One line of the engine's own dump of a serialized entity.
using SnapshotLineCallback = Source2ModCallback<void(const char* line)>;

#define SOURCE2MOD_SNAPSHOTS_INTERFACE "ISource2ModSnapshots001"

class ISource2ModSnapshots
{
public:
    virtual ~ISource2ModSnapshots() = default;

    /// Whether the engine's serializer interfaces are there.
    virtual bool IsAvailable() = 0;

    /// The entity's network class ("CCSPlayerPawn"); null for an entity that is
    /// not networked.
    virtual const char* GetNetworkClassName(CEntityInstance* entity) = 0;

    /**
     * @brief Encodes the entity now and hands over every field that goes out.
     *
     * @return The number of fields, -1 when the entity cannot be encoded.
     */
    virtual int GetFields(CEntityInstance* entity, SnapshotFieldCallback callback) = 0;

    /**
     * @brief One field's encoded value by its network name (as GetFields() names it).
     *
     * @return false when the entity does not send that field now.
     */
    virtual bool GetField(CEntityInstance* entity, const char* name, char* value, size_t maxlen) = 0;

    /**
     * @brief The engine's own dump of the encoded entity, line by line.
     *
     * @return The number of lines, -1 when the entity cannot be encoded.
     */
    virtual int Dump(CEntityInstance* entity, SnapshotLineCallback callback) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_SNAPSHOTS_H
