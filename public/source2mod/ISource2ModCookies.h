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
* @file ISource2ModCookies.h
* @brief Client preferences: per-player settings that outlive the session.
*
* SourceMod's clientprefs. A cookie is a named string setting ("quake_sounds",
* "hud_color"); every player has their own value of it, kept in a database
* under their SteamID, and loaded into memory when the player is authorized.
* Until then -- AreCookiesCached() false -- reads return "" and writes are
* kept and saved once the player's values arrived.
*
* The database is an SQLite file, addons/source2mod/data/clientprefs.sq3
* unless core.json's "ClientPrefsDatabase" names another. The
* tables are SourceMod's (sm_cookies, sm_cookie_cache) and players are keyed
* by "STEAM_1:Y:Z", so a SourceMod clientprefs database can be carried over.
*
* Values are saved when they change (a write goes to the database right away,
* asynchronously) -- nothing is lost on a crash or a map change.
  */

#ifndef _INCLUDE_ISOURCE2MOD_COOKIES_H
#define _INCLUDE_ISOURCE2MOD_COOKIES_H

#pragma once
#include "ISource2ModPlugin.h"

/// A cookies-cached listener (AddCookiesCachedListener).
using Source2ModCookiesListenerId = Source2ModHookId;

/// Who may see and change a cookie from the cookie commands. SourceMod's CookieAccess.
enum Source2ModCookieAccess
{
    /// Players can read and change it.
    SOURCE2MOD_COOKIE_PUBLIC = 0,
    /// Players can read it, only plugins change it.
    SOURCE2MOD_COOKIE_PROTECTED = 1,
    /// Hidden from players.
    SOURCE2MOD_COOKIE_PRIVATE = 2,
};

/// A registered cookie; 0 is none.
using Source2ModCookieId = int;

/// Longest cookie name and value, in bytes without the terminator (SourceMod's).
#define SOURCE2MOD_COOKIE_MAX_NAME_LENGTH 30
#define SOURCE2MOD_COOKIE_MAX_VALUE_LENGTH 100

/**
 * @brief A player's cookies were loaded (SourceMod's OnClientCookiesCached).
 *
 * Runs on the main thread, once per connection, after OnClientAuthorized.
 */
using CookiesCachedCallback = Source2ModCallback<void(CPlayerSlot slot)>;

#define SOURCE2MOD_COOKIES_INTERFACE "ISource2ModCookies001"

class ISource2ModCookies
{
public:
    virtual ~ISource2ModCookies() = default;

    /// Whether the cookie database is connected. Everything else works
    /// without it, only nothing is loaded or saved.
    virtual bool IsConnected() = 0;

    /* =========================
    Cookies
    ========================= */

    /**
     * @brief Registers a cookie, or finds it when it already is registered
     * (by any plugin -- cookies are shared, by name).
     *
     * @return The cookie, 0 for an empty name or one longer than
     *         SOURCE2MOD_COOKIE_MAX_NAME_LENGTH.
     */
    virtual Source2ModCookieId RegisterCookie(const char* name, const char* description, Source2ModCookieAccess access) = 0;

    /// A registered cookie by name; 0 when there is none.
    virtual Source2ModCookieId FindCookie(const char* name) = 0;

    /// Its name / description; nullptr for a bad id.
    virtual const char* GetCookieName(Source2ModCookieId cookie) = 0;
    virtual const char* GetCookieDescription(Source2ModCookieId cookie) = 0;
    virtual Source2ModCookieAccess GetCookieAccess(Source2ModCookieId cookie) = 0;

    /// How many cookies are registered; ids run from 1 to that.
    virtual int GetCookieCount() = 0;

    /* =========================
    Values
    ========================= */

    /// Whether the player's values were loaded (or there is no database to load them from).
    virtual bool AreCookiesCached(CPlayerSlot slot) = 0;

    /**
     * @brief The player's value.
     *
     * @return false (and "") when the player has never had one, or the
     *         values are not loaded yet.
     */
    virtual bool GetCookie(CPlayerSlot slot, Source2ModCookieId cookie, char* out, size_t maxlen) = 0;

    /**
     * @brief Sets the player's value and saves it.
     *
     * @return false for an empty slot, a bot, a bad id or a value longer than
     *         SOURCE2MOD_COOKIE_MAX_VALUE_LENGTH.
     */
    virtual bool SetCookie(CPlayerSlot slot, Source2ModCookieId cookie, const char* value) = 0;

    /// When the value was last set, Unix time; 0 when it never was.
    virtual int64_t GetCookieTime(CPlayerSlot slot, Source2ModCookieId cookie) = 0;

    /**
     * @brief Sets the value of a player who need not be connected.
     *
     * @param steamId A SteamID64.
     */
    virtual bool SetCookieBySteamID(uint64 steamId, Source2ModCookieId cookie, const char* value) = 0;

    /* =========================
    Listeners
    ========================= */

    /// Runs `callback` whenever a player's cookies were loaded. Goes with the plugin.
    virtual Source2ModCookiesListenerId AddCookiesCachedListener(CookiesCachedCallback callback) = 0;

    virtual bool RemoveCookiesCachedListener(Source2ModCookiesListenerId id) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_COOKIES_H
