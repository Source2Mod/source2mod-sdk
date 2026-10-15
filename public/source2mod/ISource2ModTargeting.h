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
 * @file ISource2ModTargeting.h
 * @brief Command text to players: "@ct", "#12", "STEAM_1:0:123", "bob".
 *
 * Patterns:
 * - @all, @ct, @t, @spec, @alive, @dead, @bots, @humans, @me, @!me, @aim
 *   (the player under the caller's crosshair), and filters plugins add;
 * - #<userid>, a SteamID in any form (#STEAM_1:0:1, [U:1:2], 7656119...),
 *   #<exact name>;
 * - otherwise part of a name (case-insensitive); a name that matches exactly
 *   wins over partial matches.
 *
 * The caller's immunity is checked through ISource2ModPermissions (CanTarget)
 * unless SOURCE2MOD_TARGET_NO_IMMUNITY; the console (slot -1) targets anyone.
 *
 * @code
 * std::vector<CPlayerSlot> targets;
 * Source2ModTargetName name;
 * int n = g_pSource2ModTargeting->ProcessTargetString(arg, caller, SOURCE2MOD_TARGET_ALIVE, targets, &name);
 * if (n <= 0)
 *     return ctx.Reply(g_pSource2ModTargeting->GetTargetErrorText(n));
 * @endcode
 */

#ifndef _INCLUDE_ISOURCE2MOD_TARGETING_H
#define _INCLUDE_ISOURCE2MOD_TARGETING_H

#pragma once
#include "ISource2ModPlugin.h"

#include <string>
#include <vector>

/* -------- filters (SourceMod's COMMAND_FILTER_* values) -------- */

#define SOURCE2MOD_TARGET_ALIVE        (1 << 0)   ///< only alive players
#define SOURCE2MOD_TARGET_DEAD         (1 << 1)   ///< only dead players
#define SOURCE2MOD_TARGET_CONNECTED    (1 << 2)   ///< also players not in game yet
#define SOURCE2MOD_TARGET_NO_IMMUNITY  (1 << 3)   ///< ignore immunity
#define SOURCE2MOD_TARGET_NO_MULTI     (1 << 4)   ///< one player only (@ patterns refused)
#define SOURCE2MOD_TARGET_NO_BOTS      (1 << 5)   ///< no bots

/* -------- results: > 0 is the number of players (SourceMod's COMMAND_TARGET_*) -------- */

#define SOURCE2MOD_TARGET_NONE         0    ///< nobody matched
#define SOURCE2MOD_TARGET_NOT_ALIVE    -1
#define SOURCE2MOD_TARGET_NOT_DEAD     -2
#define SOURCE2MOD_TARGET_NOT_IN_GAME  -3
#define SOURCE2MOD_TARGET_IMMUNE       -4
#define SOURCE2MOD_TARGET_EMPTY_FILTER -5   ///< a @filter that matched nobody
#define SOURCE2MOD_TARGET_NOT_HUMAN    -6
#define SOURCE2MOD_TARGET_AMBIGUOUS    -7   ///< more than one player for a NO_MULTI target

/// How to name what was targeted ("all terrorists", or the one player's name).
struct Source2ModTargetName
{
    std::string text;
    /// text is a phrase name in common.phrases ("all terrorists") rather than a name.
    bool isPhrase = false;
};

/**
 * @brief A filter a plugin adds ("@admins"): fills @p targets with whom it means.
 * @return false when it cannot be used now (it then counts as matching nobody).
 */
using Source2ModTargetFilter = Source2ModCallback<bool(CPlayerSlot caller, std::vector<CPlayerSlot>& targets)>;

/* =========================
Core Source2Mod Targeting
========================= */

#define SOURCE2MOD_TARGETING_INTERFACE "ISource2ModTargeting001"

class ISource2ModTargeting
{
public:
    virtual ~ISource2ModTargeting() = default;

    /**
     * @brief Finds the players a pattern means.
     *
     * @param caller  Who is targeting (immunity, @me, @aim); -1 for the console.
     * @param flags   SOURCE2MOD_TARGET_* filters.
     * @param targets Filled with the players, in slot order.
     * @param name    How to name them (optional).
     * @return The number of players, or a SOURCE2MOD_TARGET_* error (<= 0).
     */
    virtual int ProcessTargetString(const char* pattern, CPlayerSlot caller, int flags,
                                    std::vector<CPlayerSlot>& targets, Source2ModTargetName* name = nullptr) = 0;

    /// One player, or an error: ProcessTargetString with NO_MULTI.
    virtual int FindTarget(const char* pattern, CPlayerSlot caller, int flags, CPlayerSlot& target) = 0;

    /// The phrase (in common.phrases) for an error: "No matching client", ...
    virtual const char* GetTargetErrorPhrase(int error) = 0;
    /// The same in English, for a reply that is not translated.
    virtual const char* GetTargetErrorText(int error) = 0;

    /**
     * @brief Adds "@<name>". @p phrase names the targets ("all admins").
     * @return An id for RemoveTargetFilter, 0 when the name is taken.
     */
    virtual Source2ModHookId AddTargetFilter(const char* name, Source2ModTargetFilter filter, const char* phrase, bool phraseIsTranslation) = 0;
    virtual bool RemoveTargetFilter(Source2ModHookId id) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_TARGETING_H
