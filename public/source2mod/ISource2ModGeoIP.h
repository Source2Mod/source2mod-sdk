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
* @file ISource2ModGeoIP.h
* @brief Country, city and the rest of where an IP address is.
*
* Lookups go to a MaxMind GeoLite2 database (.mmdb) through libmaxminddb.
* The database is not shipped: download GeoLite2-City.mmdb (or the smaller
* GeoLite2-Country.mmdb, which has no city, region, time zone or location)
* from MaxMind and put it where core.json's "GeoIPDatabase" says --
* addons/source2mod/geoip/GeoLite2-City.mmdb by default. Without it every
* lookup returns false and the core says so once in its log.
*
* An address may carry a port ("1.2.3.4:27005"); it is cut off.
  */

#ifndef _INCLUDE_ISOURCE2MOD_GEOIP_H
#define _INCLUDE_ISOURCE2MOD_GEOIP_H

#pragma once
#include "ISource2ModPlugin.h"

/// What Lookup() reads.
enum Source2ModGeoIPField
{
    /// ISO 3166-1 alpha-2, "US".
    SOURCE2MOD_GEOIP_COUNTRY_CODE = 0,
    /// "United States".
    SOURCE2MOD_GEOIP_COUNTRY = 1,
    /// Two letters, "NA".
    SOURCE2MOD_GEOIP_CONTINENT_CODE = 2,
    /// "North America".
    SOURCE2MOD_GEOIP_CONTINENT = 3,
    /// The first subdivision's ISO code, "CA". City database only.
    SOURCE2MOD_GEOIP_REGION_CODE = 4,
    /// "California". City database only.
    SOURCE2MOD_GEOIP_REGION = 5,
    /// "Mountain View". City database only.
    SOURCE2MOD_GEOIP_CITY = 6,
    /// IANA time zone, "America/Los_Angeles". City database only.
    SOURCE2MOD_GEOIP_TIMEZONE = 7,
    /// Postal code. City database only.
    SOURCE2MOD_GEOIP_POSTAL_CODE = 8,
};

#define SOURCE2MOD_GEOIP_INTERFACE "ISource2ModGeoIP001"

class ISource2ModGeoIP
{
public:
    virtual ~ISource2ModGeoIP() = default;

    /// Whether a database is open.
    virtual bool IsLoaded() = 0;

    /// The database's type, "GeoLite2-City" / "GeoLite2-Country"; "" when none is open.
    virtual const char* GetDatabaseType() = 0;

    /**
     * @brief Looks one field of an address up.
     *
     * @param ip       IPv4 or IPv6, a port is allowed.
     * @param field    What to read.
     * @param out      Receives it, "" on failure.
     * @param maxlen   Size of `out`.
     * @param language For the names: "en", "de", "es", "fr", "ja", "pt-BR",
     *                 "ru", "zh-CN"; nullptr = "en". Falls back to English
     *                 when the database has no name in that language.
     * @return false when there is no database, the address is not in it, or
     *         the database has no such field for it.
     */
    virtual bool Lookup(const char* ip, Source2ModGeoIPField field, char* out, size_t maxlen, const char* language = nullptr) = 0;

    /// Lookup() for the address of the player in this slot.
    virtual bool LookupPlayer(CPlayerSlot slot, Source2ModGeoIPField field, char* out, size_t maxlen, const char* language = nullptr) = 0;

    /// Latitude and longitude. City database only.
    virtual bool GetLocation(const char* ip, double& latitude, double& longitude) = 0;

    /**
     * @brief Any string, number or bool of the record, by its path.
     *
     * @code
     * g_pSource2ModGeoIP->LookupPath("8.8.8.8", "country/names/de", buf, sizeof(buf));
     * g_pSource2ModGeoIP->LookupPath("8.8.8.8", "subdivisions/0/iso_code", buf, sizeof(buf));
     * @endcode
     *
     * @param path Keys and array indices separated by '/'.
     */
    virtual bool LookupPath(const char* ip, const char* path, char* out, size_t maxlen) = 0;

    /// The player's IP address without the port; false for an empty slot or a bot.
    virtual bool GetPlayerIP(CPlayerSlot slot, char* out, size_t maxlen) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_GEOIP_H
