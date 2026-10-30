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
* @file ISource2ModConfigs.h
* @brief JSON config files a plugin declares once and reads already merged.
*
* A plugin names its configs and gives each one its defaults as JSON text
* (comments allowed). Source2Mod then owns the files:
*
* @code
* addons/source2mod/configs/<PluginName>/
*   <name>.example.json   the defaults, verbatim; rewritten whenever they change
*   <name>.json           the admin's file; created from the defaults when missing
* @endcode
*
* What the plugin reads is the admin's file merged over the defaults: a key
* the admin's file lacks (a setting added in a newer plugin version) comes from
* the defaults, a value of the wrong type is reported and replaced by the
* default, and everything the admin wrote stays as written. So every key the
* defaults have is always there, and the plugin needs no fallback values of
* its own.
*
* The usual shape: the plugin class derives from ISource2ModConfigListener next
* to ISource2ModPlugin / ISource2ModListener, declares its configs in
* OnDeclareConfigs() and copies the values it needs in OnConfigChanged():
*
* @code
* bool MyPlugin::Load(ISource2ModAPI* api, char* error, size_t maxlen, bool late)
* {
*     SOURCE2MOD_SAVEVARS();
*     if (!g_pSource2ModConfigs || !g_pSource2ModConfigs->AddListener(this, error, maxlen))
*         return false;
*     ...
* }
*
* void MyPlugin::OnDeclareConfigs(ISource2ModConfigDeclarations* configs)
* {
*     configs->Add("config", R"json({ "Prefix": "[MY]", "Enabled": true })json");
* }
*
* void MyPlugin::OnConfigChanged(ISource2ModConfig* config, ConfigChange reason)
* {
*     m_bEnabled = config->Root()->GetBool("Enabled");
* }
* @endcode
*
* The defaults can also be built key by key (Source2ModConfigDefaults below)
* instead of written as one JSON text.
*
* Everything here runs on the game thread.
*/

#ifndef _INCLUDE_ISOURCE2MOD_CRASH_H
#define _INCLUDE_ISOURCE2MOD_CRASH_H

#ifdef _WIN32
#pragma once
#endif

class ISource2ModPlugin;

/* =========================
ISource2ModCrash
========================= */

/**

* @brief A plugin's last words when the server dies on a signal.
*
* Both callbacks run inside the crash handler's signal handler, on the thread
* that crashed, while every other thread keeps running. The process is about to
* die and its state may be what killed it: the heap can be corrupt and any lock
* can be held. Keep them short, and prepare what they need (sockets, buffers)
* beforehand. There is no SourcePawn counterpart: the VM may be the crash.
*
* A second fault in here only ends the process sooner, and the minidump is safe
* by then for OnServerCrash(). OnServerCrashing() runs before the dump, so it
* may only use async-signal-safe calls (send(), write()).
*
* Linux only; on Windows nothing calls these.
  */
class ISource2ModCrashListener
{
public:
    virtual ~ISource2ModCrashListener() = default;

    /// Before the minidump is written. Async-signal-safe calls only.
    virtual void OnServerCrashing()
    {
    }

    /// After the minidump is written (or failed). Best effort, anything may be
    /// broken; all listeners share one second, after which the process is killed.
    virtual void OnServerCrash()
    {
    }
};

#define SOURCE2MOD_CRASH_INTERFACE "ISource2ModCrash001"

class ISource2ModCrash
{
public:
    virtual ~ISource2ModCrash() = default;

    /**

    * @brief Registers a listener. Removed by RemoveListener() or when the plugin unloads.
    *
    * @return False when every slot is taken.
      */
    virtual bool AddListener(ISource2ModCrashListener* listener) = 0;

    virtual void RemoveListener(ISource2ModCrashListener* listener) = 0;
};

#endif // _INCLUDE_ISOURCE2MOD_CRASH_H
