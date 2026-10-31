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

* @file ISource2ModAPI.h
* @brief Main entry point for Source2Mod API.
*
* This interface provides access to:
* * Core engine interfaces
* * Source2Mod subsystems (Commands, Events, ConVars, etc.)
* * Logging and console output
* * Interface querying and factory access
*
* @note This is the primary interface passed to plugins during initialization.
  */

#ifndef _INCLUDE_ISOURCE2MOD_API_H
#define _INCLUDE_ISOURCE2MOD_API_H

#pragma once
#include "ISource2ModPaths.h"
#include "ISource2ModPlugin.h"

#include <engine2/isource2server.h>
#include <engine2/ivengineserver2.h>

/* =========================
Windows macro collisions
========================= */

/*
 * windows.h, dragged in by the engine headers above, defines FreeModule as a
 * function-like macro aliasing FreeLibrary. Left alone it rewrites the
 * declaration below into FreeLibrary((ISource2ModModule* module)) and every call
 * site with it. Nothing here wants the Win32 original, so drop it -- and drop
 * it at the point of use rather than in a shared header, so it happens after
 * whatever pulled windows.h in.
 */
#ifdef FreeModule
#undef FreeModule
#endif

/* =========================
Forward declarations
========================= */

using PluginId = int;

class IGameEventManager2;
class CGlobalVars;
class ICvar;
class ISource2Server;
class IGameEventSystem;
class INetworkMessages;
class INetworkServerService;
class CGameEntitySystem;
class CSchemaSystem;
class IVEngineServer2;
class ISource2ServerConfig;
class ISource2GameClients;
class ISource2GameEntities;
class IGameResourceService;
class INetworkSystem;
class INetworkStringTableContainer;
class IEngineServiceMgr;
class IHostStateMgr;
class ILocalize;
class IFileSystem;

class ISource2ModAddresses;
class ISource2ModCommands;
class ISource2ModConVars;
class ISource2ModCustomHud;
class ISource2ModGameConfig;
class ISource2ModGameSystems;
class ISource2ModHTTP;
class ISource2ModEntities;
class ISource2ModEvents;
class ISource2ModListener;
class ISource2ModMenus;
class ISource2ModModule;
class ISource2ModSQL;
class ISource2ModNetworkMessages;
class ISource2ModPlugin;
class ISource2ModScheduler;
class ISource2ModSounds;
class ISource2ModTrace;
class ISource2ModTransmit;
class ISource2ModHooks;
class ISource2ModClients;
class ISource2ModConfigs;
class ISource2ModCookies;
class ISource2ModCrash;
class ISource2ModFeatures;
class ISource2ModFiles;
class ISource2ModFunctions;
class ISource2ModGeoIP;
class ISource2ModJSON;
class ISource2ModMaps;
class ISource2ModModules;
class ISource2ModNatives;
class ISource2ModPaths;
class ISource2ModPermissions;
class ISource2ModSchemaMethods;
class ISource2ModScripts;
class ISource2ModTargeting;
class ISource2ModTranslations;
class ISource2ModUtils;
class ISource2ModVoice;
class ISource2ModWebSockets;

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

/* =========================
Core Source2Mod API
========================= */

/**

* @brief Main API interface for Source2Mod.
*
* Provides access to all source2mod subsystems and engine interfaces.
*
* @code
* api->Commands()->RegisterConCommand(...);
* api->Events()->HookGameEvent(...);
* @endcode
  */
class ISource2ModAPI
{
public:
    virtual ~ISource2ModAPI() = default;

    /* =========================
    Logging & console
    ========================= */

    /**

    * @brief Logs a formatted message with plugin context.
    *
    * The prefix is the name of the plugin whose code called this.
    *
    * @param msg Format string
      */
    virtual void Log(const char* msg, ...) = 0;

    /**

    * @brief Prints message to console.
      */
    virtual void ConPrint(const char* msg) = 0;

    /**

    * @brief Prints formatted message to console.
      */
    virtual void ConPrintf(const char* fmt, ...) = 0;

    /* =========================
    Listeners
    ========================= */

    /**

    * @brief Registers a source2mod listener.
    *
    * @param pListener Listener instance; its plugin owns it
      */
    virtual void AddListener(ISource2ModListener* pListener) = 0;

    /* =========================
    Interface system
    ========================= */

    /**

    * @brief Queries interface from factory.
    *
    * @param fn Factory function
    * @param iface Interface name
    * @param min Minimum version (optional)
      */
    virtual void* QueryInterface(CreateInterfaceFn fn, const char* iface, int min = -1) = 0;

    /**

    * @brief Searches for interface by iterating versions.
      */
    virtual void* InterfaceSearch(CreateInterfaceFn fn, const char* iface, int max, int* ret) = 0;

    /**

    * @brief Formats interface name with version.
      */
    virtual int FormatIface(char iface[], size_t maxlength) = 0;

    /* =========================
    Engine factories
    ========================= */

    virtual CreateInterfaceFn GetEngineFactory(bool syn = true) = 0;
    virtual CreateInterfaceFn GetPhysicsFactory(bool syn = true) = 0;
    virtual CreateInterfaceFn GetFileSystemFactory(bool syn = true) = 0;
    virtual CreateInterfaceFn GetServerFactory(bool syn = true) = 0;

    /**

    * @brief Source2Mod interface factory.
      */
    virtual void* Source2ModFactory(const char* iface, int* ret, PluginId* id) = 0;

    /**

    * @brief Metamod interface factory.
      */
    virtual void* MetaFactory(const char* iface, int* ret, PluginId* id) = 0;

    /* =========================
    Source2Mod subsystems
    ========================= */

    // The interfaces SOURCE2MOD_SAVEVARS() puts in the g_pSource2Mod* globals, this plugin's own.
    virtual ISource2ModAddresses* Addresses() = 0;
    virtual ISource2ModClients* Clients() = 0;
    virtual ISource2ModCommands* Commands() = 0;
    virtual ISource2ModConfigs* Configs() = 0;
    virtual ISource2ModConVars* ConVars() = 0;
    virtual ISource2ModCookies* Cookies() = 0;
    virtual ISource2ModCrash* Crash() = 0;
    virtual ISource2ModCustomHud* CustomHud() = 0;
    virtual ISource2ModEntities* Entities() = 0;
    virtual ISource2ModEvents* Events() = 0;
    virtual ISource2ModFeatures* Features() = 0;
    virtual ISource2ModFiles* Files() = 0;
    virtual ISource2ModFunctions* Functions() = 0;
    virtual ISource2ModGameConfig* GameConfig() = 0;
    virtual ISource2ModGameSystems* GameSystems() = 0;
    virtual ISource2ModGeoIP* GeoIP() = 0;
    virtual ISource2ModHooks* Hooks() = 0;
    virtual ISource2ModHTTP* HTTP() = 0;
    virtual ISource2ModJSON* JSON() = 0;
    virtual ISource2ModMaps* Maps() = 0;
    virtual ISource2ModMenus* Menus() = 0;
    virtual ISource2ModModules* Modules() = 0;
    virtual ISource2ModNatives* Natives() = 0;
    virtual ISource2ModNetworkMessages* NetworkMessages() = 0;
    virtual ISource2ModPaths* Paths() = 0;
    virtual ISource2ModPermissions* Permissions() = 0;
    virtual ISource2ModScheduler* Scheduler() = 0;
    virtual ISource2ModSchemaMethods* SchemaMethods() = 0;
    virtual ISource2ModScripts* Scripts() = 0;
    virtual ISource2ModSounds* Sounds() = 0;
    virtual ISource2ModSQL* SQL() = 0;
    virtual ISource2ModTargeting* Targeting() = 0;
    virtual ISource2ModTrace* Trace() = 0;
    virtual ISource2ModTranslations* Translations() = 0;
    virtual ISource2ModTransmit* Transmit() = 0;
    virtual ISource2ModUtils* Utils() = 0;
    virtual ISource2ModVoice* Voice() = 0;
    virtual ISource2ModWebSockets* WebSockets() = 0;

    /* =========================
    Engine interfaces
    ========================= */

    virtual ISource2Server* GetSource2Server() = 0;
    virtual ISource2ServerConfig* GetSource2ServerConfig() = 0;
    virtual ISource2GameClients* GetSource2GameClients() = 0;
    virtual ISource2GameEntities* GetSource2GameEntities() = 0;
    virtual IVEngineServer2* GetEngineServer() = 0;
    virtual ICvar* GetCVar() = 0;
    virtual CSchemaSystem* GetSchemaSystem() = 0;
    virtual IGameResourceService* GetGameResourceService() = 0;
    virtual INetworkServerService* GetNetworkServerService() = 0;
    virtual INetworkSystem* GetNetworkSystem() = 0;
    virtual INetworkMessages* GetNetworkMessages() = 0;
    virtual INetworkStringTableContainer* GetNetworkStringTableServer() = 0;
    virtual IEngineServiceMgr* GetEngineServiceMgr() = 0;
    virtual IHostStateMgr* GetHostStateMgr() = 0;
    virtual ILocalize* GetLocalize() = 0;
    virtual IFileSystem* GetFileSystem() = 0;

    virtual IGameEventSystem* GetGameEventSystem() = 0;
    virtual IGameEventManager2* GetGameEventManager() = 0;
    // These two are read from the engine on every call (the entity system is
    // null before the first map), so ask each time rather than keep the pointer.
    virtual CGameEntitySystem* GetEntitySystem() = 0;
    virtual CGlobalVars* GetGlobalVars() = 0;

    /**

    * @brief Returns base directory of the server.
      */
    virtual const char* GetBaseDir() = 0;

    /* =========================
    Formatting utilities
    ========================= */

    /**

    * @brief Formats string into buffer.
      */
    virtual size_t Format(char* buffer, size_t maxlength, const char* format, ...) = 0;

    /**

    * @brief Formats string using va_list.
      */
    virtual size_t FormatArgs(char* buffer, size_t maxlength, const char* format, va_list ap) = 0;

    /* =========================
    Server
    ========================= */

    /**
     * @brief Whether core.json's "FollowCS2ServerGuidelines" is on: then the
     * core refuses what is known to get a server's GSLTs banned (setting the
     * fields in schema::IsGuidelineField, giving knife skins).
     */
    virtual bool FollowsServerGuidelines() = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_API_H
