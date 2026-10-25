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

* @file ISource2ModGameSystems.h
* @brief Game systems: the engine's per-session event set (IGameSystem) for
* plugins, through Source2Mod.
*
* A game system is how the engine runs code at fixed points of a session --
* BuildGameSessionManifest for precaching, SpawnGroupPrecache for a spawn group's
* resources, GameFrameBoundary once a frame, and the rest of the IGameSystem
* events. The engine only calls a system for the events it overrides (it
* compares the system's vtable with the default one), and a system it holds
* cannot go away under it -- so a plugin never registers one of its own: the
* engine sees one game system of Source2Mod's, and a plugin's system is a set of
* handlers on it. A plugin can unload whenever; its systems go with it.
  */

#ifndef _INCLUDE_ISOURCE2MOD_GAMESYSTEMS_H
#define _INCLUDE_ISOURCE2MOD_GAMESYSTEMS_H

#pragma once
#include "ISource2ModTypes.h"

#include <igamesystem.h>

/// A game system made with ISource2ModGameSystems::Create(); 0 is none.
using Source2ModGameSystemId = int;

/**

* @brief The IGameSystem events a game system can hook: the value is the
* event's vtable slot in IGameSystem, the id the engine dispatches it by.
*
* Not hookable: the three slots whose event is unknown, client-only events a
* server never posts, and OutOfGameFrameBoundary (another dispatcher).
  */
enum class GameSystemEvent : int
{
    GameInit = 3,
    GameShutdown = 4,
    GamePostInit = 5,
    GamePreShutdown = 6,
    BuildGameSessionManifest = 7,
    GameActivate = 8,
    ClientFullySignedOn = 9,
    ClientDisconnect = 10,
    GameDeactivate = 12,
    SpawnGroupPrecache = 13,
    SpawnGroupUncache = 14,
    PreSpawnGroupLoad = 15,
    PostSpawnGroupLoad = 16,
    PreSpawnGroupUnload = 17,
    PostSpawnGroupUnload = 18,
    ActiveSpawnGroupChanged = 19,
    ServerPreEntityThink = 31,
    ServerPostEntityThink = 32,
    ServerPostPhysicsSimulate = 33,
    ServerPreClientUpdate = 34,
    ServerAdvanceTick = 35,
    ServerGamePostSimulate = 37,
    ServerPostAdvanceTick = 39,
    ServerBeginAsyncPostTickWork = 41,
    ServerGameBeginAsyncPostTickWork = 42,
    ServerPreEndAsyncPostTickWork = 43,
    ServerPostEndAsyncPostTickWork = 44,
    GameFrameBoundary = 48,
    SaveGame = 50,
    RestoreGame = 51,
    NewLevelPlayerConnect = 52,
    WorldGroupCreated = 53,
    WorldGroupDestroyed = 54,
    ServerPrePackEntities = 58,
};

/// The event a message type belongs to, for the typed Hook<EVENT>().
template <typename EVENT>
struct GameSystemEventOf;

#define SOURCE2MOD_GAMESYSTEM_EVENT(name) \
    template <> struct GameSystemEventOf<Event##name##_t> { static constexpr GameSystemEvent value = GameSystemEvent::name; };
SOURCE2MOD_GAMESYSTEM_EVENT(GameInit)
SOURCE2MOD_GAMESYSTEM_EVENT(GameShutdown)
SOURCE2MOD_GAMESYSTEM_EVENT(GamePostInit)
SOURCE2MOD_GAMESYSTEM_EVENT(GamePreShutdown)
SOURCE2MOD_GAMESYSTEM_EVENT(BuildGameSessionManifest)
SOURCE2MOD_GAMESYSTEM_EVENT(GameActivate)
SOURCE2MOD_GAMESYSTEM_EVENT(ClientFullySignedOn)
SOURCE2MOD_GAMESYSTEM_EVENT(ClientDisconnect)
SOURCE2MOD_GAMESYSTEM_EVENT(GameDeactivate)
SOURCE2MOD_GAMESYSTEM_EVENT(SpawnGroupPrecache)
SOURCE2MOD_GAMESYSTEM_EVENT(SpawnGroupUncache)
SOURCE2MOD_GAMESYSTEM_EVENT(PreSpawnGroupLoad)
SOURCE2MOD_GAMESYSTEM_EVENT(PostSpawnGroupLoad)
SOURCE2MOD_GAMESYSTEM_EVENT(PreSpawnGroupUnload)
SOURCE2MOD_GAMESYSTEM_EVENT(PostSpawnGroupUnload)
SOURCE2MOD_GAMESYSTEM_EVENT(ActiveSpawnGroupChanged)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPreEntityThink)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPostEntityThink)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPostPhysicsSimulate)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPreClientUpdate)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerAdvanceTick)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerGamePostSimulate)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPostAdvanceTick)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerBeginAsyncPostTickWork)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerGameBeginAsyncPostTickWork)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPreEndAsyncPostTickWork)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPostEndAsyncPostTickWork)
SOURCE2MOD_GAMESYSTEM_EVENT(GameFrameBoundary)
SOURCE2MOD_GAMESYSTEM_EVENT(SaveGame)
SOURCE2MOD_GAMESYSTEM_EVENT(RestoreGame)
SOURCE2MOD_GAMESYSTEM_EVENT(NewLevelPlayerConnect)
SOURCE2MOD_GAMESYSTEM_EVENT(WorldGroupCreated)
SOURCE2MOD_GAMESYSTEM_EVENT(WorldGroupDestroyed)
SOURCE2MOD_GAMESYSTEM_EVENT(ServerPrePackEntities)
#undef SOURCE2MOD_GAMESYSTEM_EVENT

/// A hooked event: `msg` is the event's message (Event<Name>_t), never null.
using GameSystemHandler = Source2ModCallback<void(const void* msg)>;

/* =========================
Core Source2Mod Game Systems
========================= */

/**

* @brief Game systems owned by Source2Mod, with the events a plugin hooks.
*
* @code
* Source2ModGameSystemId gs = g_pSource2ModGameSystems->Create(g_PluginID, "MyPlugin");
* g_pSource2ModGameSystems->Hook<EventBuildGameSessionManifest_t>(gs, [](const EventBuildGameSessionManifest_t* msg)
* {
*     msg->m_pResourceManifest->AddResource("models/example/thing.vmdl");
* });
* g_pSource2ModGameSystems->Register(gs);
* // Destroy(gs) -- or nothing: it goes with the plugin
* @endcode
*
* A registered system gets its events from now on (the ones of the running map
* that are still to come) and on every map after. The engine is only told about
* the events that have a handler, so an unhooked event costs nothing. Hooking an
* event on a registered system, or unhooking its last handler, takes effect
* right away. Everything a plugin made goes when it unloads.
*
* @note Handlers run on the thread the engine posts the event on; the
*       Server*AsyncPostTickWork ones may be a worker thread.
  */
#define SOURCE2MOD_GAMESYSTEMS_INTERFACE "ISource2ModGameSystems002"

class ISource2ModGameSystems
{
public:
    virtual ~ISource2ModGameSystems() = default;

    /**

    * @brief Whether game systems work: the engine's lists were found.
    *
    * False usually means a game update moved them; Register() then fails.
      */
    virtual bool IsAvailable() const = 0;

    /**

    * @brief One of the engine's own game systems, by the name its factory has
    *        ("CCSGameRulesGameSystem", ...); nullptr when there is none.
      */
    virtual IGameSystem* FindByName(const char* name) = 0;

    /**

    * @brief Makes a game system for @p owner, not registered yet.
    * @param name What GetName() answers, for logs.
    * @return Its id, 0 on failure.
      */
    virtual Source2ModGameSystemId Create(PluginId owner, const char* name) = 0;

    /**

    * @brief Adds a handler for @p event. Several are called in the order
    *        they were added.
    * @return The handler's id (for Unhook()), 0 on failure.
      */
    virtual Source2ModHookId Hook(Source2ModGameSystemId system, GameSystemEvent event, GameSystemHandler handler) = 0;

    /// Removes a handler by the id Hook() returned.
    virtual bool Unhook(Source2ModHookId hook) = 0;

    /// Hands the system to the engine. False when unavailable or unknown.
    virtual bool Register(Source2ModGameSystemId system) = 0;

    /// Takes it out of the engine again; it can be registered later.
    virtual bool Unregister(Source2ModGameSystemId system) = 0;

    virtual bool IsRegistered(Source2ModGameSystemId system) = 0;

    /// Unregisters it and removes it with its handlers.
    virtual bool Destroy(Source2ModGameSystemId system) = 0;

    /**

    * @brief Hook() with the event taken from the message type.
    *
    * @code
    * g_pSource2ModGameSystems->Hook<EventGameFrameBoundary_t>(gs, [](const EventGameFrameBoundary_t*) { ... });
    * @endcode
      */
    template <typename EVENT>
    Source2ModHookId Hook(Source2ModGameSystemId system, Source2ModCallback<void(const EVENT*)> handler)
    {
        GameSystemHandler wrapped = [handler](const void* msg) { handler(static_cast<const EVENT*>(msg)); };
        wrapped.OwnedAs(handler);
        return Hook(system, GameSystemEventOf<EVENT>::value, std::move(wrapped));
    }
};

#endif //_INCLUDE_ISOURCE2MOD_GAMESYSTEMS_H