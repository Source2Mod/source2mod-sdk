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

#ifndef _INCLUDE_SOURCE2MOD_PLAYERMASK_H
#define _INCLUDE_SOURCE2MOD_PLAYERMASK_H
#ifdef _WIN32
#pragma once
#endif

#include <const.h>

#include <cstdint>

// A player mask as Source2Mod's APIs take it (bit n = player slot n, 64
// slots) as the engine's CPlayerBitVec, e.g. for s2sdk's
// CRecipientFilter::AddPlayersFromBitMask().
inline CPlayerBitVec PlayerBitVecFromMask(uint64_t mask)
{
    CPlayerBitVec bits;
    bits.ClearAll();
    bits.SetDWord(0, static_cast<uint32>(mask));
    if (bits.GetNumDWords() > 1)
        bits.SetDWord(1, static_cast<uint32>(mask >> 32));
    return bits;
}

inline uint64_t PlayerMaskFromBitVec(const CPlayerBitVec& bits)
{
    uint64_t mask = bits.GetDWord(0);
    if (bits.GetNumDWords() > 1)
        mask |= static_cast<uint64_t>(bits.GetDWord(1)) << 32;
    return mask;
}

#endif // _INCLUDE_SOURCE2MOD_PLAYERMASK_H
