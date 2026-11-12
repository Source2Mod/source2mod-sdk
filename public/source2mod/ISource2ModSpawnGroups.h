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
 * @file ISource2ModSpawnGroups.h
 * @brief Extra spawn groups: another map, a prefab or a skybox loaded into the
 * running map, wherever it is wanted.
 *
 * A spawn group is what the engine loads a world into: the map itself is one,
 * its skybox and every point_prefab are more (the "spawngroups" list of the
 * status command). Loading one more puts its world geometry, its resources and
 * -- unless told otherwise -- its entities into the running map, at an offset,
 * and the clients load it on their own.
 *
 * The world name is the map's path the way the engine takes it: "de_mirage" for
 * a whole map, "maps/prefabs/de_dust2/de_dust2_skybox" or "prefabs/misc/team_select"
 * for a prefab. It has to be on the server and the clients alike (the game's own
 * files, or the server's workshop addon).
 *
 * A plugin's spawn groups are unloaded when it unloads; the engine itself
 * unloads them all when the map changes.
 */

#ifndef _INCLUDE_ISOURCE2MOD_SPAWNGROUPS_H
#define _INCLUDE_ISOURCE2MOD_SPAWNGROUPS_H

#pragma once

#include <source2mod/ISource2ModTypes.h>
#include "ISource2ModTypes.h"

#include <mathlib/vector.h>

#include <cstdint>

class CEntityInstance;

/// SpawnGroupHandle_t; 0 is none (the engine's SPAWN_GROUP_HANDLE_INVALID is 0xFFFFFFFF and never returned here).
using Source2ModSpawnGroup = uint32_t;

/* =========================
Types
========================= */

/// What SpawnGroupLoadInfo::flags may hold.
enum SpawnGroupLoadFlags : uint32_t
{
    SPAWNGROUP_LOAD_NONE = 0,
    /// Loads the world and its resources, but none of its entities.
    SPAWNGROUP_LOAD_DONT_SPAWN_ENTITIES = (1 << 0),
    /// Spawns no entity of the map's own entity lump (entities a prefab adds still come).
    SPAWNGROUP_LOAD_SUPPRESS_MAP_ENTITIES = (1 << 1),
    /// Asks the engine to load it in one go. On CS2 14197 Load() still returns
    /// before the group is ready (it is a frame later): wait for the callback.
    SPAWNGROUP_LOAD_BLOCK_UNTIL_LOADED = (1 << 2),
    /// Makes it the active spawn group once loaded (what the map itself is).
    SPAWNGROUP_LOAD_SET_ACTIVE = (1 << 3),
    /// Leaves the world_layer_base layer of the new world disabled.
    SPAWNGROUP_LOAD_DISABLE_BASE_LAYER = (1 << 4),
    /// Does not load the world's streaming data (static props, lighting).
    SPAWNGROUP_LOAD_NO_STREAMING_DATA = (1 << 5),
};

/// One spawn group to load.
struct SpawnGroupLoadInfo
{
    /// The world: "de_mirage", "maps/prefabs/de_dust2/de_dust2_skybox", ... Required.
    const char* worldName = nullptr;
    /// The name the status command lists and FindByName() looks up; worldName when null.
    const char* name = nullptr;
    /// Where the world's origin goes in the running map, and how it is turned.
    Vector origin = Vector(0.0f, 0.0f, 0.0f);
    QAngle angles = QAngle(0.0f, 0.0f, 0.0f);
    /// SpawnGroupLoadFlags.
    uint32_t flags = SPAWNGROUP_LOAD_NONE;
    /// An entity lump other than the world's main one; null for that one.
    const char* entityLumpName = nullptr;
    /// Entity filter the world's entities go through; null for none.
    const char* entityFilterName = nullptr;
    /// Prefix the group's own targetnames get ("" or null for none), so two
    /// copies of one world do not answer to each other's inputs.
    const char* localNameFixup = nullptr;
};

/**
 * @brief Told once when a spawn group a plugin loaded is ready (@p loaded true,
 * its entities spawned), and once when it is gone again (@p loaded false: it
 * was unloaded, or the map changed under it). A group unloaded by Unload() or
 * by the plugin unloading is not reported.
 */
using SpawnGroupCallback = Source2ModCallback<void(Source2ModSpawnGroup group, bool loaded)>;

/* =========================
Core Source2Mod Spawn Groups
========================= */

#define SOURCE2MOD_SPAWNGROUPS_INTERFACE "ISource2ModSpawnGroups001"

/**
 * @brief Loads and unloads spawn groups.
 *
 * @code
 * SpawnGroupLoadInfo info;
 * info.worldName = "maps/prefabs/de_mirage/de_mirage_skybox";
 * info.origin = Vector(0, 0, -4000);
 * g_pSource2ModSpawnGroups->Load(info, [](Source2ModSpawnGroup group, bool loaded) {
 *     META_CONPRINTF("spawn group %u %s\n", group, loaded ? "ready" : "gone");
 * });
 * @endcode
 */
class ISource2ModSpawnGroups
{
public:
    virtual ~ISource2ModSpawnGroups() = default;

    /**
     * @brief Starts loading a spawn group into the running map.
     *
     * @param info     What to load and where
     * @param callback Told when it is ready and when it is gone; may be empty
     * @return The group, 0 when there is no map running, the world does not
     *         exist (no maps/<world>.vpk or maps/<world>/world.vwrld_c -- the
     *         engine would take it and fail the next map load with it) or the
     *         engine refused it.
     */
    virtual Source2ModSpawnGroup Load(const SpawnGroupLoadInfo& info, SpawnGroupCallback callback = SpawnGroupCallback()) = 0;

    /**
     * @brief Unloads a spawn group with its entities -- one of this plugin's,
     * or any other the engine has, the map's own included (careful with that).
     *
     * @return false when there is no such group.
     */
    virtual bool Unload(Source2ModSpawnGroup group) = 0;

    /// A loaded or loading group by the name it was loaded under through
    /// Source2Mod (SpawnGroupLoadInfo::name), else by its world name -- the
    /// map's own ("de_dust2"), its skybox, the engine's prefabs; 0 for none.
    virtual Source2ModSpawnGroup FindByName(const char* name) = 0;

    /// Whether the group is fully loaded, entities spawned.
    virtual bool IsLoaded(Source2ModSpawnGroup group) = 0;

    /// Whether the group is still on its way.
    virtual bool IsLoading(Source2ModSpawnGroup group) = 0;

    /// Makes the group the active one, like SPAWNGROUP_LOAD_SET_ACTIVE.
    virtual bool MakeActive(Source2ModSpawnGroup group) = 0;

    /// The group an entity belongs to; 0 for none.
    virtual Source2ModSpawnGroup GetEntitySpawnGroup(CEntityInstance* entity) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_SPAWNGROUPS_H
