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

* @file ISource2ModPlugin.h
* @brief Plugin interface and integration layer for Source2Mod.
*
* This file defines:
* * Plugin lifecycle interface (ISource2ModPlugin)
* * Listener system (ISource2ModListener)
* * Global variables and exposure macros
* * Interface helper macros
*
* @note This is the main entry point for plugin developers.
  */

#ifndef _INCLUDE_ISOURCE2MOD_PLUGIN_H
#define _INCLUDE_ISOURCE2MOD_PLUGIN_H

#pragma once
#include "interfaces/interfaces.h"
// The engine callbacks on ISource2ModListener take the engine's own types.
#include "engine2/isource2server.h"
#include "engine2/ivengineserver2.h"
// s2sdk's netmessages.h by its full path first: gamesessionconfiguration.h (via networkgameserverbase.h) includes
// it as "netmessages.h", and MSVC searches the directories of every open include for that, so from under
// source2mod/sdk/game/shared/ it would get our netmessages.h instead and miss CBaseCmdKeyValues.
#include "common/netmessages.h"
#include "engine2/networkgameserverbase.h"
#include "engine2/inetworkserverservice.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>

// Action, the result every source2mod callback hands back, and KHook -- the
// detour library (metamod's) plugins hook with.
#include "ISource2ModTypes.h"
#include "khook.hpp"

/* =========================
Forward declarations
========================= */

using PluginId = int;
class ISource2ModAPI;

// The SOURCE2MOD_EXPOSE/SOURCE2MOD_GLOBALVARS globals below are plain pointers, so
// forward declarations are enough -- and they have to be here rather than rely
// on the includes underneath. A TU that reaches this header *through* one of
// those interface headers (e.g. ISource2ModAddresses.h -> here) finds that
// header's guard already set, so its include below is a no-op and the class
// would otherwise still be unknown when the macros expand.
class ISource2ModAddresses;
class ISource2ModClients;
class ISource2ModFiles;
class ISource2ModTargeting;
class ISource2ModFunctions;
class ISource2ModHooks;
class ISource2ModTranslations;
class ISource2ModUtils;
class ISource2ModWebSockets;
class ISource2ModCommands;
class ISource2ModConfigs;
class ISource2ModCrash;
class ISource2ModConVars;
class ISource2ModEntities;
class ISource2ModEvents;
class ISource2ModGameConfig;
class ISource2ModMenus;
class ISource2ModSQL;
class ISource2ModNetworkMessages;
class ISource2ModPermissions;
class ISource2ModScheduler;
class ISource2ModScripts;
class ISource2ModSounds;
class ISource2ModTrace;
class ISource2ModTransmit;
class ISource2ModNatives;
class ISource2ModSchemaMethods;

// The natives interface's name, here so SOURCE2MOD_FILLVARS() fills
// g_pSource2ModNatives without pulling SourcePawn's headers into every plugin;
// ISource2ModNatives.h (needed to call it) defines the same.
#define SOURCE2MOD_NATIVES_INTERFACE "ISource2ModNatives002"
// Same for the schema helpers the core serves (ISource2ModSchemaMethods.h).
#define SOURCE2MOD_SCHEMAMETHODS_INTERFACE "ISource2ModSchemaMethods001"

// Everything below this line may use Action and the forward declarations
// above. Nothing above it can: the interface headers include this
// one back, and by then its guard is already set, so anything declared after
// them would be invisible to them.

#include "ISource2ModAddresses.h"
#include "ISource2ModCommands.h"
#include "ISource2ModConfigs.h"
#include "ISource2ModCrash.h"
#include "ISource2ModConVars.h"
#include "ISource2ModCookies.h"
#include "ISource2ModCustomHud.h"
#include "ISource2ModEntities.h"
#include "ISource2ModEvents.h"
#include "ISource2ModFeatures.h"
#include "ISource2ModGameConfig.h"
#include "ISource2ModHooks.h"
#include "ISource2ModGameSystems.h"
#include "ISource2ModGeoIP.h"
#include "ISource2ModHTTP.h"
#include "ISource2ModJSON.h"
#include "ISource2ModMaps.h"
#include "ISource2ModMenus.h"
#include "ISource2ModSQL.h"
#include "ISource2ModNetworkMessages.h"
#include "ISource2ModPaths.h"
#include "ISource2ModPermissions.h"
#include "ISource2ModScheduler.h"
#include "ISource2ModScripts.h"
#include "ISource2ModSounds.h"
#include "ISource2ModTrace.h"
#include "ISource2ModTransmit.h"
#include "ISource2ModClients.h"
#include "ISource2ModFiles.h"
#include "ISource2ModTargeting.h"
#include "ISource2ModFunctions.h"
#include "ISource2ModTranslations.h"
#include "ISource2ModUtils.h"
#include "ISource2ModWebSockets.h"

/* =========================
Interface status
========================= */

enum
{
    /// Interface query successful
    SOURCE2MOD_IFACE_OK = 0,
    /// Interface query failed
    SOURCE2MOD_IFACE_FAILED
};

/* =========================
Export system
========================= */

/// KHook, served through Source2ModFactory.
///
/// Source2Mod is a metamod plugin and gets metamod's detour engine handed
/// to it at load; this is how that same engine reaches a source2mod plugin, so
/// every hook on the server -- metamod's, Source2Mod's, every plugin's --
/// runs on one instance. SOURCE2MOD_SAVEVARS() does the fetch.
#define SOURCE2MOD_KHOOK_INTERFACE "Source2ModKHook001"

/// The commit of the KHook the core was compiled against, as a const char*
/// (nullptr from a core that predates the check). Plugins built before
/// SOURCE2MOD_KHOOK_HEADER_INTERFACE compare it with their SOURCE2MOD_KHOOK_COMMIT;
/// newer ones only fall back to it on a core that does not report the header.
#define SOURCE2MOD_KHOOK_VERSION_INTERFACE "Source2ModKHookVersion001"

/// A hash of the khook.hpp the core was compiled against, as a const char*
/// (nullptr from a core that predates it). KHOOK_INIT() compares it with
/// SOURCE2MOD_KHOOK_HEADER and refuses the load on a mismatch. khook.hpp is the
/// whole contract between a plugin's hooks and metamod's engine (IKHook and
/// the callback templates), so a KHook update that only touched its src/
/// keeps every plugin loading, while any change to the header -- the one
/// that can make a plugin hook through an engine it does not match -- does
/// not.
#define SOURCE2MOD_KHOOK_HEADER_INTERFACE "Source2ModKHookHeader001"

#ifndef SOURCE2MOD_KHOOK_COMMIT
/// Set by the SDK's CMakeLists.txt / AMBuildScript from vendor/khook (the
/// core: from metamod's copy). A build that bypassed both gets "unknown",
/// which the check reports and lets through.
#define SOURCE2MOD_KHOOK_COMMIT "unknown"
#endif

#ifndef SOURCE2MOD_KHOOK_HEADER
/// Set by the SDK's CMakeLists.txt / AMBuildScript: the first 16 hex digits of
/// the SHA-256 of khook.hpp with CRs dropped. A build that bypassed both gets
/// "unknown", and the check falls back to the commit.
#define SOURCE2MOD_KHOOK_HEADER "unknown"
#endif

/// The load-time half of the KHook check; KHOOK_INIT() calls it.
///
/// Compares the khook.hpp hashes when both sides have one, else the commits
/// (a core older than the header hash). Returns false and fills `error` only
/// when the compared values are known and differ. A side that cannot say --
/// a core too old to answer, a plugin built without git -- makes the check
/// impossible, not failed: that goes out through `note` and the load goes on.
inline bool Source2ModKHookVersionMatches(const char* coreHeader, const char* coreCommit, char* error, size_t maxlen, const char** note)
{
    if (note)
        *note = nullptr;

    if (coreHeader && coreHeader[0] && strcmp(SOURCE2MOD_KHOOK_HEADER, "unknown") != 0)
    {
        if (strcmp(coreHeader, SOURCE2MOD_KHOOK_HEADER) == 0)
            return true;

        if (error && maxlen)
            snprintf(error, maxlen,
                     "KHook mismatch: plugin built against khook.hpp %s, source2mod core runs %s -- "
                     "rebuild the plugin with an SDK whose vendor/khook has the core's header, or update the core",
                     SOURCE2MOD_KHOOK_HEADER, coreHeader);
        return false;
    }

    if (!coreCommit || !coreCommit[0])
    {
        if (note)
            *note = "Source2Mod core does not report its KHook commit (older core), KHook version check skipped";
        return true;
    }

    if (strcmp(SOURCE2MOD_KHOOK_COMMIT, "unknown") == 0)
    {
        if (note)
            *note = "this plugin carries no KHook commit (built without git), KHook version check skipped";
        return true;
    }

    if (strcmp(coreCommit, SOURCE2MOD_KHOOK_COMMIT) == 0)
        return true;

    if (error && maxlen)
        snprintf(error, maxlen,
                 "KHook mismatch: plugin built against %.12s, source2mod core runs %.12s -- "
                 "rebuild the plugin with the SDK's vendor/khook at the core's commit, or update the core",
                 SOURCE2MOD_KHOOK_COMMIT, coreCommit);
    return false;
}

/// Plugin interface name
#define SOURCE2MOD_INTERFACE_NAME "Source2ModPlugin001"

/// Plugin API version
// 2 (v1.0.26): engine callbacks on ISource2ModListener (OnGameFrame ...),
// ISource2ModAPI::GameHooks(). A plugin built against an older version loads
// (the core calls nothing it does not have); one built against a newer
// version than the core's is refused.
// 3 (v1.0.37): OnClientAuthorized, OnClientAuthorizeFailed and
// OnPermissionsChanged on ISource2ModListener.
// 4: OnClientConnect (may refuse the connection), OnClientConnected and
// OnClientDisconnectPost on ISource2ModListener, which is now in the order
// the engine calls it; OnClientDisconnect is called pre instead of post. The
// slots moved, so a plugin built against 3 or older is refused.
// 5: ISource2ModHooks (HookPoint + HookTarget) replaces ISource2ModGameHooks
// and the engine callbacks of ISource2ModListener (now hook points);
// ISource2ModEntities' entity / output listeners take filters and one entity
// (IEntityOutputListener replaces the IEntityIOListener classes); the KHook
// wrappers moved to ISource2ModKHook.h. The listener's slots moved: a plugin built
// against 4 or older is refused.
// 6: ISource2ModCommands004 -- RegisterConListener / UnregisterConListener are
// HookConCommand / UnhookConCommand (REGISTER_CON_LISTENER is
// HOOK_CON_COMMAND). A plugin built against 5 asks for ISource2ModCommands003,
// which is gone, so it is refused instead of running with no commands interface.
#define SOURCE2MOD_PLAPI_VERSION 6

/// Plugin API interface name
#define SOURCE2MOD_PLAPI_NAME "ISource2ModPlugin"

/* =========================
Plugin Interface
========================= */

/**

* @brief Main plugin interface.
*
* Every plugin must implement this interface.
  */
/// Defined by ISource2ModHooks.h for a plugin; the core never asks itself.
int Source2ModRawHookCount();
#ifdef SOURCE2MOD_CORE
inline int Source2ModRawHookCount() { return 0; }
#endif

class ISource2ModPlugin
{
public:
    virtual int GetApiVersion()
    {
        return SOURCE2MOD_PLAPI_VERSION;
    }

    virtual ~ISource2ModPlugin() = default;

public:
    /**
    * @brief Called when plugin is loaded.
    *
    * @param id Plugin identifier
    * @param api Source2Mod API instance
    * @param error Buffer for error message
    * @param maxlen Error buffer size
    * @param late False only when Source2Mod's startup pass loads the plugin.
    *             True for every load after that: "s2m load", "source2mod
    *             refresh" and the file watcher's hot reload.
    *             Players and entities may already exist.
    *
    * @return True on success, false on failure
    *
    * @note This is the main initialization function.
    */
    virtual bool Load(PluginId id, ISource2ModAPI* api, char* error, size_t maxlen, bool late) = 0;

    /**
     * @brief Called when plugin is unloaded.
     *
     * @return True on success
     */
    virtual bool Unload(char* error, size_t maxlen)
    {
        return true;
    }

public:
    /// Plugin author name
    virtual const char* GetAuthor() = 0;

    /// Plugin name
    virtual const char* GetName() = 0;

    /// Plugin description
    virtual const char* GetDescription() = 0;

    /// Plugin version
    virtual const char* GetVersion() = 0;

    /// The KHook commit this plugin was built against (the SDK's
    /// vendor/khook), "" when built without git.
    virtual const char* GetKHookCommit()
    {
#ifdef SOURCE2MOD_KHOOK_COMMIT
        return SOURCE2MOD_KHOOK_COMMIT;
#else
        return "";
#endif
    }

    /// How many KHook hooks the plugin declared through the KHOOK_* macros:
    /// none means the plugin is on the stable tier and an engine update is
    /// the core's problem alone.
    virtual int GetRawHookCount()
    {
        return ::Source2ModRawHookCount();
    }
};

/* =========================
Listener Interface
========================= */

/**

* @brief Source2Mod and Metamod events: plugins coming and going, the map,
* interface queries, the permission system.
*
* The engine's own callbacks (GameFrame, client connect / put in server /
* disconnect, ...) are hook points: g_pSource2ModHooks->Hook(HookPoint::GameFrame,
* HookTarget::All(), ...) -- ISource2ModHooks.h.
  */
class ISource2ModListener
{
public:
    virtual ~ISource2ModListener() = default;

    /// Called when a source2mod plugin (.smx) has loaded, after its Load() returned.
    /// The id is a source2mod plugin id -- the one "s2m list" shows.
    /// Fires for your own plugin too (a listener added in Load() gets it
    /// right after Load() returns); compare against your own id to skip it.
    /// The name is what the plugin's GetName() returns.
    virtual void OnPluginLoad(PluginId id, const char* name)
    {
    }

    /// Called when a source2mod plugin (.smx) is about to unload, before its
    /// Unload() runs. Fires for your own plugin too, like OnPluginLoad().
    virtual void OnPluginUnload(PluginId id, const char* name)
    {
    }

    /// Called when a Metamod:Source plugin is loaded.
    ///
    /// The id is Metamod's, from a different numbering than Source2Mod's --
    /// which is why this is not the same callback as OnPluginLoad().
    virtual void OnMetamodPluginLoad(PluginId id)
    {
    }

    /// Called when a Metamod:Source plugin is unloaded.
    virtual void OnMetamodPluginUnload(PluginId id)
    {
    }

    /// Called when all source2mod plugins are loaded
    virtual void OnAllSource2ModPluginsLoaded()
    {
    }

    /// Called when all Metamod plugins are loaded
    virtual void OnAllMetamodPluginsLoaded()
    {
    }

    /**

    * @brief Called when a map is initialized.
      */
    virtual void OnLevelInit(const char* mapName,
                             const char* mapEntities,
                             const char* oldLevel,
                             const char* landmarkName,
                             bool loadGame,
                             bool background)
    {
    }

    /// Called when a map is shutting down
    virtual void OnLevelShutdown()
    {
    }

    /**

    * @brief Called when an interface is requested from source2mod.
    *
    * @return Pointer to interface or nullptr
      */
    virtual void* OnSource2ModQuery(const char* iface, int* ret)
    {
        if (ret)
        {
            *ret = SOURCE2MOD_IFACE_FAILED;
        }

        return nullptr;
    }

    /**

    * @brief Called when a Metamod:Source plugin asks for an interface.
    *
    * A Metamod plugin reaching for something through g_SMAPI->MetaFactory()
    * ends up here, so a source2mod plugin can hand its own interfaces to code
    * that did not load through Source2Mod.
    *
    * By default this answers exactly like OnSource2ModQuery(), which is what a
    * plugin usually wants -- an interface worth exposing is worth exposing to
    * either caller. Override it to answer a Metamod plugin differently, or
    * return nullptr to keep an interface for source2mod plugins only. The two are
    * separate callbacks precisely so that choice exists: OnSource2ModQuery()
    * cannot tell who is asking.
    *
    * @return Pointer to interface or nullptr
      */
    virtual void* OnMetamodQuery(const char* iface, int* ret)
    {
        return OnSource2ModQuery(iface, ret);
    }

    /// Steam validated the player in this slot: from now on they are checked
    /// under `steamId` whatever SteamAuthMode says. Not called for bots.
    virtual void OnClientAuthorized(CPlayerSlot slot, uint64 steamId)
    {
    }

    /// Steam refused the ticket of the player in this slot; `steamId` is the
    /// one they claimed.
    virtual void OnClientAuthorizeFailed(CPlayerSlot slot, uint64 steamId)
    {
    }

    /// What this SteamID64 holds may have changed -- a grant, a group, a
    /// reload. 0 when a change can touch anybody (a group was edited,
    /// permissions.json was re-read).
    virtual void OnPermissionsChanged(uint64 steamId)
    {
    }
};

/* =========================
Plugin exposure
========================= */

/**

* @brief Exposes plugin interface to engine.
  */
#define SOURCE2MOD_EXPOSURE_FUNC(name, var)	EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ISource2ModPlugin, ISource2ModPlugin, SOURCE2MOD_PLAPI_NAME, var);

/* =========================
Globals
========================= */

/**

* @brief Defines Source2Mod globals -- the API, the plugin identity and one
* pointer per source2mod interface.
*
* Part of SOURCE2MOD_EXPOSE for a plugin. The core expands it on its own too and
* fills the pointers with SOURCE2MOD_FILLVARS(), so SDK code reads the same
* globals (ADDR_*, g_pSource2ModGameConfig, ...) whichever binary it is linked into.
  */
#define SOURCE2MOD_DEFINE_GLOBALVARS() \
    ISource2ModAPI*             g_pSource2ModAPI              = nullptr; \
    ISource2ModPlugin*          g_pPluginAPI                  = nullptr; \
    PluginId                    g_PluginID                   = 0; \
    const char*                 g_Source2ModCoreKHookCommit  = nullptr; \
    const char*                 g_Source2ModCoreKHookHeader  = nullptr; \
    ISource2ModAddresses*       g_pSource2ModAddresses       = nullptr; \
    ISource2ModCommands*        g_pSource2ModCommands        = nullptr; \
    ISource2ModConfigs*         g_pSource2ModConfigs         = nullptr; \
    ISource2ModCrash*           g_pSource2ModCrash           = nullptr; \
    ISource2ModConVars*         g_pSource2ModConVars         = nullptr; \
    ISource2ModCookies*         g_pSource2ModCookies         = nullptr; \
    ISource2ModCustomHud*       g_pSource2ModCustomHud       = nullptr; \
    ISource2ModEntities*        g_pSource2ModEntities        = nullptr; \
    ISource2ModEvents*          g_pSource2ModEvents          = nullptr; \
    ISource2ModFeatures*        g_pSource2ModFeatures        = nullptr; \
    ISource2ModGameConfig*      g_pSource2ModGameConfig      = nullptr; \
    ISource2ModHooks*           g_pSource2ModHooks           = nullptr; \
    ISource2ModGameSystems*     g_pSource2ModGameSystems     = nullptr; \
    ISource2ModGeoIP*           g_pSource2ModGeoIP           = nullptr; \
    ISource2ModHTTP*            g_pSource2ModHTTP            = nullptr; \
    ISource2ModJSON*            g_pSource2ModJSON            = nullptr; \
    ISource2ModMaps*            g_pSource2ModMaps            = nullptr; \
    ISource2ModMenus*           g_pSource2ModMenus           = nullptr; \
    ISource2ModSQL*             g_pSource2ModSQL             = nullptr; \
    ISource2ModNetworkMessages* g_pSource2ModNetworkMessages = nullptr; \
    ISource2ModPaths*           g_pSource2ModPaths           = nullptr; \
    ISource2ModPermissions*     g_pSource2ModPermissions     = nullptr; \
    ISource2ModScheduler*       g_pSource2ModScheduler       = nullptr; \
    ISource2ModScripts*         g_pSource2ModScripts         = nullptr; \
    ISource2ModSounds*          g_pSource2ModSounds          = nullptr; \
    ISource2ModTrace*           g_pSource2ModTrace           = nullptr; \
    ISource2ModClients*         g_pSource2ModClients         = nullptr; \
    ISource2ModFiles*           g_pSource2ModFiles           = nullptr; \
    ISource2ModTargeting*       g_pSource2ModTargeting       = nullptr; \
    ISource2ModFunctions*       g_pSource2ModFunctions       = nullptr; \
    ISource2ModTranslations*    g_pSource2ModTranslations    = nullptr; \
    ISource2ModUtils*           g_pSource2ModUtils           = nullptr; \
    ISource2ModWebSockets*      g_pSource2ModWebSockets      = nullptr; \
    ISource2ModTransmit*        g_pSource2ModTransmit        = nullptr; \
    ISource2ModNatives*         g_pSource2ModNatives         = nullptr; \
    ISource2ModSchemaMethods*   g_pSource2ModSchemaMethods   = nullptr;

/**

* @brief Defines global plugin variables.
*
* Creates:
* * everything SOURCE2MOD_DEFINE_GLOBALVARS() does
* * KHook::__exported__khook, the detour engine (see SOURCE2MOD_KHOOK_INTERFACE)
    */
#define SOURCE2MOD_EXPOSE(name, var) \
    SOURCE2MOD_DEFINE_GLOBALVARS() \
    namespace KHook { KHook::IKHook* __exported__khook = nullptr; } \
    SOURCE2MOD_EXPOSURE_FUNC(name, var)

/**

* @brief Declares external globals.
  */
#define SOURCE2MOD_GLOBALVARS() \
    extern ISource2ModAPI*             g_pSource2ModAPI; \
    extern ISource2ModPlugin*          g_pPluginAPI; \
    extern PluginId                    g_PluginID; \
    extern const char*                 g_Source2ModCoreKHookCommit; \
    extern const char*                 g_Source2ModCoreKHookHeader; \
    extern ISource2ModAddresses*       g_pSource2ModAddresses; \
    extern ISource2ModCommands*        g_pSource2ModCommands; \
    extern ISource2ModConfigs*         g_pSource2ModConfigs; \
    extern ISource2ModCrash*           g_pSource2ModCrash; \
    extern ISource2ModConVars*         g_pSource2ModConVars; \
    extern ISource2ModCookies*         g_pSource2ModCookies; \
    extern ISource2ModCustomHud*       g_pSource2ModCustomHud; \
    extern ISource2ModEntities*        g_pSource2ModEntities; \
    extern ISource2ModEvents*          g_pSource2ModEvents; \
    extern ISource2ModFeatures*        g_pSource2ModFeatures; \
    extern ISource2ModGameConfig*      g_pSource2ModGameConfig; \
    extern ISource2ModHooks*           g_pSource2ModHooks; \
    extern ISource2ModGameSystems*     g_pSource2ModGameSystems; \
    extern ISource2ModGeoIP*           g_pSource2ModGeoIP; \
    extern ISource2ModHTTP*            g_pSource2ModHTTP; \
    extern ISource2ModJSON*            g_pSource2ModJSON; \
    extern ISource2ModMaps*            g_pSource2ModMaps; \
    extern ISource2ModMenus*           g_pSource2ModMenus; \
    extern ISource2ModSQL*             g_pSource2ModSQL; \
    extern ISource2ModNetworkMessages* g_pSource2ModNetworkMessages; \
    extern ISource2ModPaths*           g_pSource2ModPaths; \
    extern ISource2ModPermissions*     g_pSource2ModPermissions; \
    extern ISource2ModScheduler*       g_pSource2ModScheduler; \
    extern ISource2ModScripts*         g_pSource2ModScripts; \
    extern ISource2ModSounds*          g_pSource2ModSounds; \
    extern ISource2ModTrace*           g_pSource2ModTrace; \
    extern ISource2ModClients*         g_pSource2ModClients; \
    extern ISource2ModFiles*           g_pSource2ModFiles; \
    extern ISource2ModTargeting*       g_pSource2ModTargeting; \
    extern ISource2ModFunctions*       g_pSource2ModFunctions; \
    extern ISource2ModTranslations*    g_pSource2ModTranslations; \
    extern ISource2ModUtils*           g_pSource2ModUtils; \
    extern ISource2ModWebSockets*      g_pSource2ModWebSockets; \
    extern ISource2ModTransmit*        g_pSource2ModTransmit; \
    extern ISource2ModNatives*         g_pSource2ModNatives; \
    extern ISource2ModSchemaMethods*   g_pSource2ModSchemaMethods; \
    namespace KHook { extern KHook::IKHook* __exported__khook; }

/**

* @brief Points every source2mod interface global at what the API hands out.
*
* Part of SOURCE2MOD_SAVEVARS() for a plugin; the core calls it once its
* interfaces exist (see SOURCE2MOD_DEFINE_GLOBALVARS()).
  */
#define SOURCE2MOD_FILLVARS(api) \
    g_pSource2ModAddresses       = (ISource2ModAddresses*)      (api)->Source2ModFactory(SOURCE2MOD_ADDRESSES_INTERFACE,       nullptr, nullptr); \
    g_pSource2ModCommands        = (ISource2ModCommands*)       (api)->Source2ModFactory(SOURCE2MOD_COMMANDS_INTERFACE,        nullptr, nullptr); \
    g_pSource2ModConfigs         = (ISource2ModConfigs*)        (api)->Source2ModFactory(SOURCE2MOD_CONFIGS_INTERFACE,         nullptr, nullptr); \
    g_pSource2ModCrash           = (ISource2ModCrash*)          (api)->Source2ModFactory(SOURCE2MOD_CRASH_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModConVars         = (ISource2ModConVars*)        (api)->Source2ModFactory(SOURCE2MOD_CONVARS_INTERFACE,         nullptr, nullptr); \
    g_pSource2ModCookies         = (ISource2ModCookies*)        (api)->Source2ModFactory(SOURCE2MOD_COOKIES_INTERFACE,         nullptr, nullptr); \
    g_pSource2ModCustomHud       = (ISource2ModCustomHud*)      (api)->Source2ModFactory(SOURCE2MOD_CUSTOMHUD_INTERFACE,       nullptr, nullptr); \
    g_pSource2ModEntities        = (ISource2ModEntities*)       (api)->Source2ModFactory(SOURCE2MOD_ENTITIES_INTERFACE,        nullptr, nullptr); \
    g_pSource2ModEvents          = (ISource2ModEvents*)         (api)->Source2ModFactory(SOURCE2MOD_EVENTS_INTERFACE,          nullptr, nullptr); \
    g_pSource2ModFeatures        = (ISource2ModFeatures*)       (api)->Source2ModFactory(SOURCE2MOD_FEATURES_INTERFACE,        nullptr, nullptr); \
    g_pSource2ModGameConfig      = (ISource2ModGameConfig*)     (api)->Source2ModFactory(SOURCE2MOD_GAMECONFIG_INTERFACE,      nullptr, nullptr); \
    g_pSource2ModHooks           = (ISource2ModHooks*)          (api)->Source2ModFactory(SOURCE2MOD_HOOKS_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModGameSystems     = (ISource2ModGameSystems*)    (api)->Source2ModFactory(SOURCE2MOD_GAMESYSTEMS_INTERFACE,     nullptr, nullptr); \
    g_pSource2ModGeoIP           = (ISource2ModGeoIP*)          (api)->Source2ModFactory(SOURCE2MOD_GEOIP_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModHTTP            = (ISource2ModHTTP*)           (api)->Source2ModFactory(SOURCE2MOD_HTTP_INTERFACE,            nullptr, nullptr); \
    g_pSource2ModMenus           = (ISource2ModMenus*)          (api)->Source2ModFactory(SOURCE2MOD_MENUS_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModSQL             = (ISource2ModSQL*)            (api)->Source2ModFactory(SOURCE2MOD_SQL_INTERFACE,             nullptr, nullptr); \
    g_pSource2ModNetworkMessages = (ISource2ModNetworkMessages*)(api)->Source2ModFactory(SOURCE2MOD_NETWORKMESSAGES_INTERFACE, nullptr, nullptr); \
    g_pSource2ModPaths           = (ISource2ModPaths*)          (api)->Source2ModFactory(SOURCE2MOD_PATHS_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModPermissions     = (ISource2ModPermissions*)    (api)->Source2ModFactory(SOURCE2MOD_PERMISSIONS_INTERFACE,     nullptr, nullptr); \
    g_pSource2ModJSON            = (ISource2ModJSON*)           (api)->Source2ModFactory(SOURCE2MOD_JSON_INTERFACE,            nullptr, nullptr); \
    g_pSource2ModMaps            = (ISource2ModMaps*)           (api)->Source2ModFactory(SOURCE2MOD_MAPS_INTERFACE,            nullptr, nullptr); \
    g_pSource2ModScheduler       = (ISource2ModScheduler*)      (api)->Source2ModFactory(SOURCE2MOD_SCHEDULER_INTERFACE,       nullptr, nullptr); \
    g_pSource2ModScripts         = (ISource2ModScripts*)        (api)->Source2ModFactory(SOURCE2MOD_SCRIPTS_INTERFACE,         nullptr, nullptr); \
    g_pSource2ModSounds          = (ISource2ModSounds*)         (api)->Source2ModFactory(SOURCE2MOD_SOUNDS_INTERFACE,          nullptr, nullptr); \
    g_pSource2ModTrace           = (ISource2ModTrace*)          (api)->Source2ModFactory(SOURCE2MOD_TRACE_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModClients         = (ISource2ModClients*)        (api)->Source2ModFactory(SOURCE2MOD_CLIENTS_INTERFACE,         nullptr, nullptr); \
    g_pSource2ModFiles           = (ISource2ModFiles*)          (api)->Source2ModFactory(SOURCE2MOD_FILES_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModTargeting       = (ISource2ModTargeting*)      (api)->Source2ModFactory(SOURCE2MOD_TARGETING_INTERFACE,       nullptr, nullptr); \
    g_pSource2ModFunctions       = (ISource2ModFunctions*)      (api)->Source2ModFactory(SOURCE2MOD_FUNCTIONS_INTERFACE,       nullptr, nullptr); \
    g_pSource2ModTranslations    = (ISource2ModTranslations*)   (api)->Source2ModFactory(SOURCE2MOD_TRANSLATIONS_INTERFACE,    nullptr, nullptr); \
    g_pSource2ModUtils           = (ISource2ModUtils*)          (api)->Source2ModFactory(SOURCE2MOD_UTILS_INTERFACE,           nullptr, nullptr); \
    g_pSource2ModWebSockets      = (ISource2ModWebSockets*)     (api)->Source2ModFactory(SOURCE2MOD_WEBSOCKETS_INTERFACE,      nullptr, nullptr); \
    g_pSource2ModTransmit        = (ISource2ModTransmit*)       (api)->Source2ModFactory(SOURCE2MOD_TRANSMIT_INTERFACE,        nullptr, nullptr); \
    g_pSource2ModNatives         = (ISource2ModNatives*)        (api)->Source2ModFactory(SOURCE2MOD_NATIVES_INTERFACE,         nullptr, nullptr); \
    g_pSource2ModSchemaMethods   = (ISource2ModSchemaMethods*)  (api)->Source2ModFactory(SOURCE2MOD_SCHEMAMETHODS_INTERFACE,   nullptr, nullptr);

/**

* @brief Points s2sdk's engine globals at the engine's interfaces.
*
* The globals s2sdk declares in interfaces/interfaces.h and defines in its
* interfaces library, which every plugin links: the ones SDK code and s2sdk's
* own sources read (g_pCVar for ConVars and commands,
* g_pGameResourceServiceServer for GameEntitySystem()) and the ones a plugin
* usually wants (g_pSource2Server for a hook, g_pNetworkMessages,
* g_pSchemaSystem, ...), all from what the core already holds -- a plugin
* connects nothing itself.
*
* Part of SOURCE2MOD_SAVEVARS(); the core sets its own.
  */
#define SOURCE2MOD_FILLGAMEVARS(api) \
    g_pSource2Server             = (api)->GetSource2Server(); \
    g_pSource2ServerConfig       = (api)->GetSource2ServerConfig(); \
    g_pSource2GameClients        = (api)->GetSource2GameClients(); \
    g_pSource2GameEntities       = (api)->GetSource2GameEntities(); \
    g_pEngineServer              = (api)->GetEngineServer(); \
    g_pCVar                      = (api)->GetCVar(); \
    g_pSchemaSystem              = (ISchemaSystem*)(api)->GetSchemaSystem(); \
    g_pGameResourceServiceServer = (api)->GetGameResourceService(); \
    g_pNetworkServerService      = (api)->GetNetworkServerService(); \
    g_pNetworkSystem             = (api)->GetNetworkSystem(); \
    g_pNetworkMessages           = (api)->GetNetworkMessages(); \
    g_pNetworkStringTableServer  = (api)->GetNetworkStringTableServer(); \
    g_pEngineServiceMgr          = (api)->GetEngineServiceMgr(); \
    g_pHostStateMgr              = (api)->GetHostStateMgr(); \
    g_pLocalize                  = (api)->GetLocalize(); \
    g_pFullFileSystem            = (api)->GetFileSystem();

/**

* @brief Initializes global variables inside Load().
*
* @note Must be called in plugin Load(), with its parameters under their
*       declared names (id, api, error, maxlen).
  */
/// The flags SOURCE2MOD_SAVEVARS() registers the plugin's ConVars and
/// ConCommands with; define it before including the SDK for others.
#ifndef SOURCE2MOD_CONVAR_FLAGS
#define SOURCE2MOD_CONVAR_FLAGS (FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL)
#endif

/**

* @brief Registers the plugin's ConVar<T>s and ConCommands with the engine and
* reports each to the core, which takes them out again when the plugin
* unloads (ISource2ModConVars::TrackConVar / TrackConCommand) -- what Metamod's
* META_CONVAR_REGISTER does for a metamod plugin. Part of SOURCE2MOD_SAVEVARS():
* a plugin calls neither ConVar_Register nor ConVar_Unregister itself. Ones
* made later (a ConCommand created at runtime) are registered and reported
* the moment they are made.
  */
#define SOURCE2MOD_CONVAR_REGISTER() \
    ConVar_Register(SOURCE2MOD_CONVAR_FLAGS, \
        [](ConVarRefAbstract* ref) { if (g_pSource2ModConVars) g_pSource2ModConVars->TrackConVar(g_PluginID, ref); }, \
        [](ConCommandRef* ref) { if (g_pSource2ModConVars) g_pSource2ModConVars->TrackConCommand(g_PluginID, ref); });

// The KHook check is not here: a plugin that never hooks (nothing but the
// source2mod's own interfaces) does not care which KHook the core runs.
// KHOOK_INIT() makes it, the first time KHook is used, from the two values
// fetched here (see SOURCE2MOD_KHOOK_HEADER_INTERFACE).
#define SOURCE2MOD_SAVEVARS() \
    g_pSource2ModAPI = api; \
    SOURCE2MOD_FILLGAMEVARS(api) \
    g_pPluginAPI  = static_cast<ISource2ModPlugin*>(this); \
    g_PluginID   = id; \
    KHook::__exported__khook = static_cast<KHook::IKHook*>(api->Source2ModFactory(SOURCE2MOD_KHOOK_INTERFACE, nullptr, nullptr)); \
    g_Source2ModCoreKHookCommit = static_cast<const char*>(api->Source2ModFactory(SOURCE2MOD_KHOOK_VERSION_INTERFACE, nullptr, nullptr)); \
    g_Source2ModCoreKHookHeader = static_cast<const char*>(api->Source2ModFactory(SOURCE2MOD_KHOOK_HEADER_INTERFACE, nullptr, nullptr)); \
    SOURCE2MOD_FILLVARS(api) \
    SOURCE2MOD_CONVAR_REGISTER()

/* =========================
Logging helpers
========================= */

/// Shortcut for logging
#define SOURCE2MOD_LOG        g_pSource2ModAPI->Log

/// Console print
#define SOURCE2MOD_CONPRINT   g_pSource2ModAPI->ConPrint

/// Console printf
#define SOURCE2MOD_CONPRINTF  g_pSource2ModAPI->ConPrintf

/* =========================
Interface helpers
========================= */

/**
 * @brief Macro for automatically getting a current or newer Valve interface.
 *
 * @param v_factory		Factory method to use from ISource2ModApi (such as engineFactory).
 * @param v_var			Variable name to store into.
 * @param v_type		Interface type (do not include the pointer/asterisk).
 * @param v_name		Interface name.
 */
#define GET_VALVE_IFACE_CURRENT(v_factory, v_var, v_type, v_name) \
	v_var = (v_type *)g_pSource2ModAPI->QueryInterface(g_pSource2ModAPI->v_factory(), v_name); \
	if (!v_var) \
	{ \
		if (error && maxlen) \
		{ \
			g_pSource2ModAPI->Format(error, maxlen, "Could not find interface: %s", v_name); \
		} \
		return false; \
	}

/**
 * @brief Same as GET_IFACE, except searches for any.
 *
 * @param v_factory	Factory method to use from ISource2ModApi (such as engineFactory).
 * @param v_var		Variable name to store into.
 * @param v_type		Interface type (do not include the pointer/asterisk).
 * @param v_name		Interface name.
 */
#define GET_VALVE_IFACE_ANY(v_factory, v_var, v_type, v_name) \
	v_var = (v_type *)g_pSource2ModAPI->QueryInterface(g_pSource2ModAPI->v_factory(), v_name, 0); \
	if (!v_var) \
	{ \
		if (error && maxlen) \
		{ \
			g_pSource2ModAPI->Format(error, maxlen, "Could not find interface: %s", v_name); \
		} \
		return false; \
	}

/**
 * @brief Macro for automatically getting a Source2Mod plugin exposed interface.
 *
 * Unlike the GET_VALVE_IFACE_* macros this one never returns on its own, so it
 * is usable outside Load() -- in OnAllSource2ModPluginsLoaded(), say, where the
 * interface belongs to another plugin and its absence is an ordinary outcome
 * rather than a fatal one. Test v_ret against SOURCE2MOD_IFACE_OK and decide.
 *
 * @param v_var		Variable name to store into.
 * @param v_type		Interface type (do not include the pointer/asterisk).
 * @param v_name		Interface name.
 * @param v_ret		int receiving SOURCE2MOD_IFACE_OK or SOURCE2MOD_IFACE_FAILED.
 */
#define GET_SOURCE2MOD_IFACE(v_var, v_type, v_name, v_ret) \
	v_var = (v_type *)g_pSource2ModAPI->Source2ModFactory(v_name, &(v_ret), nullptr); \
	if (!v_var) \
	{ \
		(v_ret) = SOURCE2MOD_IFACE_FAILED; \
	}

/**
 * @brief Macro for automatically getting a Metamod:Source plugin exposed interface.
 *
 * Same contract as GET_SOURCE2MOD_IFACE: it never returns on its own, it only
 * reports through v_ret. Metamod fills v_ret with META_IFACE_OK/META_IFACE_FAILED,
 * which are binary-compatible with the SOURCE2MOD_IFACE_* values, so compare
 * against those rather than pulling a metamod header in here.
 *
 * @param v_var		Variable name to store into.
 * @param v_type		Interface type (do not include the pointer/asterisk).
 * @param v_name		Interface name.
 * @param v_ret		int receiving SOURCE2MOD_IFACE_OK or SOURCE2MOD_IFACE_FAILED.
 */
#define GET_METAMOD_IFACE(v_var, v_type, v_name, v_ret) \
	v_var = (v_type *)g_pSource2ModAPI->MetaFactory(v_name, &(v_ret), nullptr); \
	if (!v_var) \
	{ \
		(v_ret) = SOURCE2MOD_IFACE_FAILED; \
	}

// Last, on purpose: the hook macros need the globals SOURCE2MOD_GLOBALVARS() above
// declares, and are meant to be there wherever this header is.
#define SOURCE2MOD_KHOOK_WRAPPERS
#include "ISource2ModKHook.h"

#endif //_INCLUDE_ISOURCE2MOD_PLUGIN_H
