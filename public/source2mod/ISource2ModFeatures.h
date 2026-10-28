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
* @file ISource2ModFeatures.h
* @brief Asking whether something optional is there before using it.
*
* SourceMod's GetFeatureStatus, for both kinds of plugin: a native C++ plugin
* asks before it calls into another plugin's interface or relies on a part of
* the core that may be switched off (GeoIP without a database), a SourcePawn
* plugin asks the same through GetFeatureStatus() / Source2ModFeatures.
*
* What can be asked about:
*
* * a native        -- a SourcePawn native, the core's own or one a plugin
*                      registered (CreateNative, ISource2ModNatives). A plugin
*                      native is Unavailable while its plugin is unloaded.
* * a capability    -- a named part of the core that can be missing at
*                      runtime; the names are the SOURCE2MOD_CAPABILITY_* below.
* * an interface    -- an interface Source2ModFactory() would hand out: the
*                      core's (at the exact revision asked for) or one a plugin
*                      serves through OnSource2ModQuery.
* * a library       -- a name a plugin registered (RegPluginLibrary,
*                      ISource2ModNatives::RegisterLibrary).
  */

#ifndef _INCLUDE_ISOURCE2MOD_FEATURES_H
#define _INCLUDE_ISOURCE2MOD_FEATURES_H

#pragma once
#include "ISource2ModPlugin.h"

/// The same numbers as SourceMod's FeatureType, plus the two Source2Mod adds.
enum Source2ModFeatureType
{
    SOURCE2MOD_FEATURE_NATIVE = 0,
    SOURCE2MOD_FEATURE_CAPABILITY = 1,
    SOURCE2MOD_FEATURE_INTERFACE = 2,
    SOURCE2MOD_FEATURE_LIBRARY = 3,
};

/// The same numbers as SourceMod's FeatureStatus.
enum Source2ModFeatureStatus
{
    /// There, and usable.
    SOURCE2MOD_FEATURE_AVAILABLE = 0,
    /// Known, but not usable right now (its plugin unloaded, no database, ...).
    SOURCE2MOD_FEATURE_UNAVAILABLE = 1,
    /// Not known at all.
    SOURCE2MOD_FEATURE_UNKNOWN = 2,
};

/// The SourcePawn VM: always there in this core.
#define SOURCE2MOD_CAPABILITY_SOURCEPAWN    "sourcepawn"
/// GeoIP lookups: a GeoLite2 database was found and opened.
#define SOURCE2MOD_CAPABILITY_GEOIP         "geoip"
/// Client preferences: the cookie database is connected.
#define SOURCE2MOD_CAPABILITY_CLIENTPREFS   "clientprefs"
/// Registering game systems (ISource2ModGameSystems::IsAvailable()).
#define SOURCE2MOD_CAPABILITY_GAMESYSTEMS   "gamesystems"

#define SOURCE2MOD_FEATURES_INTERFACE "ISource2ModFeatures001"

class ISource2ModFeatures
{
public:
    virtual ~ISource2ModFeatures() = default;

    /**
     * @brief The status of a native, capability, interface or library.
     *
     * @param type What `name` names.
     * @param name The native, capability, interface ("ISource2ModMenus005")
     *             or library name.
     */
    virtual Source2ModFeatureStatus GetFeatureStatus(Source2ModFeatureType type, const char* name) = 0;

    /// GetFeatureStatus(SOURCE2MOD_FEATURE_NATIVE, name) == AVAILABLE.
    virtual bool IsNativeAvailable(const char* name) = 0;

    /// Whether Source2ModFactory() would hand this interface out right now.
    /// Unlike asking the factory, a miss is not logged.
    virtual bool IsInterfaceAvailable(const char* iface) = 0;

    /// Whether some plugin registered this library and is still loaded.
    virtual bool IsLibraryAvailable(const char* name) = 0;

    /// Whether a SOURCE2MOD_CAPABILITY_* is usable.
    virtual bool HasCapability(const char* name) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_FEATURES_H
