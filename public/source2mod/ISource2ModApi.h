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

    /**

    * @brief Access address/pattern system.
      */
    virtual ISource2ModAddresses* Addresses() = 0;

    /**

    * @brief Access command system.
      */
    virtual ISource2ModCommands* Commands() = 0;

    /**

    * @brief Access ConVar system.
      */
    virtual ISource2ModConVars* ConVars() = 0;

    /**

    * @brief Access custom HUD layout system.
      */
    virtual ISource2ModCustomHud* CustomHud() = 0;

    /**

    * @brief Access entities and Entity I/O system.
      */
    virtual ISource2ModEntities* Entities() = 0;

    /**

    * @brief Access event system.
      */
    virtual ISource2ModEvents* Events() = 0;

    /**

    * @brief Access game configuration system.
      */
    virtual ISource2ModGameConfig* GameConfig() = 0;

    /**

    * @brief Access game system registry.
      */
    virtual ISource2ModGameSystems* GameSystems() = 0;

    /**

    * @brief Access HTTP system.
      */
    virtual ISource2ModHTTP* HTTP() = 0;

    /**

    * @brief Access menu system.
      */
    virtual ISource2ModMenus* Menus() = 0;

    /**

    * @brief Access the SQL system (MySQL and SQLite).
      */
    virtual ISource2ModSQL* SQL() = 0;

    /**

    * @brief Access network messages system.
      */
    virtual ISource2ModNetworkMessages* NetworkMessages() = 0;

    /**

    * @brief Where Source2Mod keeps things on disk.
      */
    virtual ISource2ModPaths* Paths() = 0;

    /**

    * @brief Access scheduler system.
      */
    virtual ISource2ModScheduler* Scheduler() = 0;

    /**

    * @brief Access sound system.
      */
    virtual ISource2ModSounds* Sounds() = 0;

    /**

    * @brief Access tracing system.
      */
    virtual ISource2ModTrace* Trace() = 0;

    /**

    * @brief Access transmit system (per-viewer entity visibility).
      */
    virtual ISource2ModTransmit* Transmit() = 0;

    /* =========================
    Module factory
    ========================= */

    /**

    * @brief Creates and initialises a module wrapper by name.
    *
    * The returned object must be released with FreeModule().
    *
    * @param name Module name without extension (e.g. "server").
    * @return Heap-allocated ISource2ModModule, or nullptr on failure.
      */
    virtual ISource2ModModule* LoadModule(const char* name) = 0;

    /**

    * @brief Creates and initialises a module wrapper from an address inside it.
    *
    * The returned object must be released with FreeModule().
    *
    * @param ptr Any address that belongs to the target module.
    * @return Heap-allocated ISource2ModModule, or nullptr on failure.
      */
    virtual ISource2ModModule* LoadModuleFromMemory(uintptr_t ptr) = 0;

    /**

    * @brief Releases a module previously obtained from LoadModule() / LoadModuleFromMemory().
      */
    virtual void FreeModule(ISource2ModModule* module) = 0;

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
    Added in plugin API 2 -- at the end, so a plugin built against API 1
    still finds every slot above where it left it.
    ========================= */

    /**

    * @brief The hooks (ISource2ModHooks.h): game functions, entity virtuals,
    *        engine callbacks, any function. Entity events and outputs are
    *        listeners of ISource2ModEntities.
      */
    virtual ISource2ModHooks* Hooks() = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_API_H
