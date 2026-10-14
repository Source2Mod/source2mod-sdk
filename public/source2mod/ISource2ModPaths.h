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

* @file ISource2ModPaths.h
* @brief Where Source2Mod keeps things on disk.
*
* Every path a plugin needs comes from here rather than being spelled out
* against the game directory, so the layout can move without every plugin
* having to be found and edited.
  */

#ifndef _INCLUDE_ISOURCE2MOD_PATHS_H
#define _INCLUDE_ISOURCE2MOD_PATHS_H

#pragma once
#include "ISource2ModPlugin.h"

/* =========================
Core Source2Mod Paths
========================= */

/**

* @brief Source2Mod's on-disk layout.
*
* Under the game directory:
*
* @code
* addons/source2mod/
*   configs/              core.json, and one directory per plugin
*     <PluginName>/       this plugin's own configs
*   gamedata/             every .json here is read into the game config
*   plugins/              the .smx files themselves
*   shared/               data more than one plugin reads
* @endcode
*
* The returned strings are owned by Source2Mod and stay valid for as long as
* it is loaded; copy them if you need to keep one past that.
  */
#define SOURCE2MOD_PATHS_INTERFACE "ISource2ModPaths001"

class ISource2ModPaths
{
public:
    virtual ~ISource2ModPaths() = default;

    /**

    * @brief The game directory, e.g. ".../game/csgo".
      */
    virtual const char* GameDirectory() = 0;

    /**

    * @brief addons/source2mod.
      */
    virtual const char* RootDirectory() = 0;

    /**

    * @brief addons/source2mod/configs.
      */
    virtual const char* ConfigsDirectory() = 0;

    /**

    * @brief addons/source2mod/gamedata.
      */
    virtual const char* GamedataDirectory() = 0;

    /**

    * @brief addons/source2mod/plugins, where the .smx files live.
      */
    virtual const char* PluginsDirectory() = 0;

    /**

    * @brief addons/source2mod/shared, for data more than one plugin reads.
      */
    virtual const char* SharedDirectory() = 0;

    /**

    * @brief This plugin's own config directory, configs/<PluginName>.
    *
    * The directory is not created for you; it is where the plugin's files are
    * expected to have been installed.
    *
    * @param owner Plugin ID to look up
    * @return The path, or nullptr when there is no such plugin
      */
    virtual const char* PluginConfigDirectory(PluginId owner) = 0;
};

#define SOURCE2MOD_GAME_DIR()          g_pSource2ModPaths->GameDirectory()
#define SOURCE2MOD_ROOT_DIR()          g_pSource2ModPaths->RootDirectory()
#define SOURCE2MOD_CONFIGS_DIR()       g_pSource2ModPaths->ConfigsDirectory()
#define SOURCE2MOD_GAMEDATA_DIR()      g_pSource2ModPaths->GamedataDirectory()
#define SOURCE2MOD_PLUGINS_DIR()       g_pSource2ModPaths->PluginsDirectory()
#define SOURCE2MOD_SHARED_DIR()        g_pSource2ModPaths->SharedDirectory()

/**
 * @brief This plugin's config directory; the plugin ID is filled in for you.
 */
#define PLUGIN_CONFIG_DIR()         g_pSource2ModPaths->PluginConfigDirectory(g_PluginID)

#endif //_INCLUDE_ISOURCE2MOD_PATHS_H
