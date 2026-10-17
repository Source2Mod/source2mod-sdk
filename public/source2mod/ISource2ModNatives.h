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
 * @file ISource2ModNatives.h
 * @brief Natives for SourcePawn plugins, from a native (C++) plugin.
 *
 * What a SourceMod extension does for its scripts: register natives a .sp
 * declares and calls, a library its scripts can test for, and handles of a
 * type of its own. The natives are SourcePawn's -- SPVM_NATIVE_FUNC, called
 * with the script's IPluginContext and its parameters -- so the SourcePawn
 * headers come with this one (public/sourcepawn, the core's VM version).
 *
 * SOURCE2MOD_SAVEVARS() fills g_pSource2ModNatives like every other global
 * (null on a core without script support); including this header is only
 * needed to call it.
 *
 * @code
 * g_pSource2ModNatives->AddNatives(g_PluginID, g_MyNatives);
 * @endcode
 *
 * Everything registered here goes when the plugin unloads: its natives
 * become unbound (a script calling one gets an error), its libraries are
 * removed (OnLibraryRemoved) and its handle types are destroyed together
 * with every handle of them.
 */

#ifndef _INCLUDE_ISOURCE2MOD_NATIVES_H
#define _INCLUDE_ISOURCE2MOD_NATIVES_H

#pragma once
#include "ISource2ModTypes.h"

#include <sp_vm_api.h>

#include <cstddef>
#include <cstdint>

#define SOURCE2MOD_NATIVES_INTERFACE "ISource2ModNatives001"

/// A handle type registered through CreateHandleType(); 0 is none.
using Source2ModHandleType = uint32_t;

/// Takes a handle's object when its last handle is closed.
using Source2ModHandleDestructor = void (*)(void* object);

class ISource2ModNatives
{
public:
    virtual ~ISource2ModNatives() = default;

    /**
     * @brief Registers natives for scripts.
     *
     * Scripts already loaded get the ones they were missing; a script that
     * failed to load for want of one of them is loaded again. A name that a
     * live plugin already registered is refused (and logged), the rest of the
     * list still goes in.
     *
     * @param owner     The plugin's id (g_PluginID)
     * @param natives   Name and function pairs, ending in { nullptr, nullptr };
     *                  copied, the array need not outlive the call
     * @return          false when any of them was refused
     */
    virtual bool AddNatives(PluginId owner, const sp_nativeinfo_t* natives) = 0;

    /**
     * @brief Registers a library, what LibraryExists() and OnLibraryAdded()
     * in a script see. Scripts that are running get OnLibraryAdded now.
     *
     * Several plugins may register the same library; it is removed
     * (OnLibraryRemoved) when the last of them unloads.
     *
     * @return false on an empty name
     */
    virtual bool RegisterLibrary(PluginId owner, const char* name) = 0;

    /// @brief Whether a plugin (script or native) registered this library.
    virtual bool LibraryExists(const char* name) = 0;

    /**
     * @brief The script plugin a native was called from.
     *
     * @return Its plugin id, 0 when the context is no script's
     */
    virtual PluginId GetScriptPlugin(SourcePawn::IPluginContext* ctx) = 0;

    /**
     * @brief SourceMod's format, as Format() and PrintToServer() do it: the
     * format at params[fmtParam], its arguments after it.
     *
     * @return false after reporting the error to the script
     */
    virtual bool FormatString(SourcePawn::IPluginContext* ctx, const cell_t* params, int fmtParam,
                              char* buffer, size_t maxlen, size_t* written = nullptr) = 0;

    /**
     * @brief Registers a handle type for the objects this plugin hands out.
     *
     * @param destroy   Called with the object when its last handle is closed
     *                  (CloseHandle / delete, the owning script unloading);
     *                  may be nullptr
     */
    virtual Source2ModHandleType CreateHandleType(PluginId owner, const char* name, Source2ModHandleDestructor destroy) = 0;

    /**
     * @brief A new handle to `object`, owned by the script that `ctx` is --
     * closed when that script unloads.
     *
     * @return The handle, 0 on an unknown type or a full handle table
     */
    virtual cell_t CreateHandle(Source2ModHandleType type, void* object, SourcePawn::IPluginContext* ctx) = 0;

    /**
     * @brief The object behind a handle of this type.
     *
     * @return The object, or nullptr after reporting SourceMod's
     *         "Invalid Handle" error to the script
     */
    virtual void* ReadHandle(SourcePawn::IPluginContext* ctx, cell_t handle, Source2ModHandleType type) = 0;

    /// @brief Closes a handle whoever owns it.
    virtual bool CloseHandle(cell_t handle) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_NATIVES_H
