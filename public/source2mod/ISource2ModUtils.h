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
 * @file ISource2ModUtils.h
 * @brief Text, time, regex, profiling and file logging helpers.
 *
 * The same helpers SourcePawn scripts get (source2mod/utils.inc), so a C++
 * plugin and a script that both write to a log, format a time or match a
 * pattern get the same result.
 *
 * Regex is ECMAScript (std::regex), not PCRE: SOURCE2MOD_REGEX_CASELESS and
 * _DOTALL always apply, _MULTILINE where the C++ library has it.
 */

#ifndef _INCLUDE_ISOURCE2MOD_UTILS_H
#define _INCLUDE_ISOURCE2MOD_UTILS_H

#pragma once

#include <source2mod/ISource2ModTypes.h>
#include "ISource2ModPlugin.h"

#include <cstdint>
#include <string>
#include <vector>

#define SOURCE2MOD_REGEX_CASELESS  0x01
#define SOURCE2MOD_REGEX_MULTILINE 0x02
#define SOURCE2MOD_REGEX_DOTALL    0x04

/**
 * @brief A compiled pattern and its last matches.
 */
class ISource2ModRegex
{
public:
    /**
     * @brief Finds the first match at or after @p offset.
     * @return Substrings of the match (the whole match + its captures); 0 for no match.
     */
    virtual int Match(const char* text, size_t offset = 0) = 0;
    /// Finds every match. @return How many.
    virtual int MatchAll(const char* text) = 0;
    /// Matches found by the last Match / MatchAll.
    virtual int MatchCount() = 0;
    /// Substrings of a match (the whole match + its captures).
    virtual int CaptureCount(int match = 0) = 0;
    /// Substring @p index of a match (0 = the whole match); nullptr when out of range.
    virtual const char* GetSubString(int index, int match = 0) = 0;
    /// Where a match ends in the text.
    virtual size_t MatchEnd(int match = 0) = 0;
    virtual void Release() = 0;

protected:
    ~ISource2ModRegex() = default;
};

/* =========================
Core Source2Mod Utils
========================= */

#define SOURCE2MOD_UTILS_INTERFACE "ISource2ModUtils002"

class ISource2ModUtils
{
public:
    virtual ~ISource2ModUtils() = default;

    /* -------- text -------- */

    /// Splits on @p separator (not on an empty one); @p maxParts > 0 caps the parts, the last keeps the rest.
    virtual std::vector<std::string> Explode(const char* text, const char* separator, int maxParts = 0) = 0;
    virtual std::string Implode(const std::vector<std::string>& parts, const char* separator) = 0;
    /// Replaces every @p search. @return How many.
    virtual int Replace(std::string& text, const char* search, const char* replace, bool caseSensitive = true) = 0;
    /// Without the whitespace at both ends.
    virtual std::string Trim(const char* text) = 0;
    /// Where @p search starts in @p text; -1 when it is not there.
    virtual int Find(const char* text, const char* search, bool caseSensitive = true) = 0;
    virtual bool StrEqual(const char* a, const char* b, bool caseSensitive = true) = 0;

    /* -------- time -------- */

    /// Seconds since 1970.
    virtual int64_t GetTime() = 0;
    /// strftime in the server's local time; @p stamp -1 is now.
    virtual std::string FormatTime(const char* format, int64_t stamp = -1) = 0;
    /// The opposite, as local time. @return -1 when @p text does not fit @p format.
    virtual int64_t ParseTime(const char* text, const char* format) = 0;
    /// Milliseconds on a steady clock (only differences mean anything).
    virtual int64_t GetTickCount() = 0;

    /* -------- regex -------- */

    /// nullptr (and why in @p error) for a bad pattern; Release() it.
    virtual ISource2ModRegex* CompileRegex(const char* pattern, int flags = 0, std::string* error = nullptr) = 0;

    /* -------- logging -------- */

    /**
     * @brief Appends "L <date> - <time>: [<plugin>] <message>" to a file
     *        (under the game folder unless absolute); 0 for @p owner leaves the
     *        plugin out. Missing folders are made.
     */
    virtual bool LogToFile(const char* path, const char* message) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_UTILS_H
