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
* @file ISource2ModMaps.h
* @brief The current map, the next one, map changes and map lists.
*
* What a map vote, a map chooser or an admin "map" command builds on; none of
* those are in the core.
*
* A map is named the way the console takes it:
*
* * "de_dust2"                 -- a map the server has (IsMapValid()),
*                                 changed to with `changelevel`;
* * "workshop/3070212801" or
*   "3070212801"               -- a Workshop map by its ID, changed to with
*                                 `host_workshop_map`;
* * any other name             -- assumed to be a map of the server's Workshop
*                                 collection, changed to with
*                                 `ds_workshop_changelevel`.
*
* Map lists come from configs/maplists.json, SourceMod's maplists.cfg as JSON:
*
* @code
* {
*   "mapcyclefile": { "file": "mapcycle.txt" },     // relative to the game directory
*   "default":      { "target": "mapcyclefile" },   // another list's name
*   "mapchooser":   { "maps": [ "de_dust2", "workshop/3070212801" ] }
* }
* @endcode
*
* A list that is not in the file falls back to "default", "default" to
* "mapcyclefile", and that to mapcycle.txt in the game directory. In a list
* file every line is a map; empty lines and lines starting with //, # or ;
* are skipped.
  */

#ifndef _INCLUDE_ISOURCE2MOD_MAPS_H
#define _INCLUDE_ISOURCE2MOD_MAPS_H

#pragma once
#include "ISource2ModPlugin.h"

#include <string>
#include <vector>

/// SourceMod's FindMapResult, the same numbers.
enum Source2ModFindMapResult
{
    /// The server has a map by exactly this name.
    SOURCE2MOD_FINDMAP_FOUND = 0,
    /// Nothing by this name.
    SOURCE2MOD_FINDMAP_NOT_FOUND = 1,
    /// One map of the map lists starts with or contains this name; `found` holds it.
    SOURCE2MOD_FINDMAP_FUZZY_MATCH = 2,
    /// Not used by this core (SourceMod's workshop name normalisation).
    SOURCE2MOD_FINDMAP_NON_CANONICAL = 3,
    /// A Workshop ID or Workshop name: may well exist, cannot be checked before changing.
    SOURCE2MOD_FINDMAP_POSSIBLY_AVAILABLE = 4,
};

/// ReadMapList() flags, SourceMod's MAPLIST_FLAG_* values.
enum Source2ModMapListFlags
{
    /// When the list cannot be read, every map in the maps folder instead.
    SOURCE2MOD_MAPLIST_MAPSFOLDER = 1 << 0,
    /// Do not fall back to "default" / "mapcyclefile".
    SOURCE2MOD_MAPLIST_NO_DEFAULT = 1 << 2,
};

/// One map that was played, newest first in GetMapHistory().
struct Source2ModMapHistoryEntry
{
    std::string map;
    /// Why the server left it: the reason given to ChangeLevel(), or
    /// "Normal level change".
    std::string reason;
    /// When it started, Unix time.
    int64_t startTime = 0;
};

#define SOURCE2MOD_MAPS_INTERFACE "ISource2ModMaps001"

class ISource2ModMaps
{
public:
    virtual ~ISource2ModMaps() = default;

    /// The map running now ("de_dust2"); "" before the first one.
    virtual const char* GetCurrentMap() = 0;

    /// Whether the server has this map (the engine's IsMapValid). Workshop
    /// maps that are not downloaded yet are not valid.
    virtual bool IsMapValid(const char* map) = 0;

    /**
     * @brief SourceMod's FindMap: the map a typed name means.
     *
     * @param map   What was typed.
     * @param found Receives the map's full name (or `map` itself).
     */
    virtual Source2ModFindMapResult FindMap(const char* map, std::string& found) = 0;

    /**
     * @brief The map the match moves to when it ends.
     *
     * Sets the game's `nextlevel` as well; for a Workshop map the core
     * changes level itself when the match ends (cs_win_panel_match, after
     * mp_match_restart_delay).
     *
     * @return false for a name FindMap() does not find at all.
     */
    virtual bool SetNextMap(const char* map) = 0;

    /**
     * @brief The next map: what SetNextMap() set, else the game's
     * `nextlevel`, else the one after the current map in "mapcyclefile".
     *
     * @return "" when none of those gives one.
     */
    virtual const char* GetNextMap() = 0;

    /**
     * @brief Changes the map now (SourceMod's ForceChangeLevel).
     *
     * @param map    A map in any of the forms above.
     * @param reason Kept in the map history.
     * @return false for an empty name.
     */
    virtual bool ChangeLevel(const char* map, const char* reason) = 0;

    /// How many earlier maps the history holds (up to 32).
    virtual int GetMapHistorySize() = 0;

    /// An earlier map, 0 = the one before this one. false when out of range.
    virtual bool GetMapHistory(int index, Source2ModMapHistoryEntry& entry) = 0;

    /**
     * @brief Reads a map list (see the file comment).
     *
     * @param name  The list: "default", "mapcyclefile", or a plugin's own.
     * @param maps  The maps are appended here.
     * @param flags Source2ModMapListFlags.
     * @return false when no list could be read (`maps` is left as it was).
     */
    virtual bool ReadMapList(const char* name, std::vector<std::string>& maps, int flags) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_MAPS_H
