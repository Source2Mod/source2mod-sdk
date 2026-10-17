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
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl gives
 * you permission to link the code of this program (as well as its derivative
 * works) to "Counter-Strike 2," "Source 2," "Steam," and any Game MODs or
 * server software running on software by Valve Corporation. You must obey the
 * GNU General Public License in all respects for all other code used.
 *
 * Additionally, this exception applies to all derivative works unless
 * otherwise stated in LICENSE.txt.
 *
 * Authors:
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

#ifndef _INCLUDE_SOURCE2MOD_CONSTS_H
#define _INCLUDE_SOURCE2MOD_CONSTS_H

/*
 * Game constants s2sdk has no header for. Everything else a plugin needs
 * (DamageTypes_t, TakeDamageFlags_t, SharedTeams_t, MoveType_t, Flags_t,
 * HitGroup_t, InputBitMask_t, ...) is s2sdk's own; the SourcePawn include set
 * has all of them under the same names (<source2mod/consts>).
 */

/**
 * Why a round ended: CCSGameRules::TerminateRound, m_eRoundWinReason /
 * m_eRoundEndReason. SourcePawn: CSRoundEnd_*.
 */
enum CSRoundEndReason : int
{
    CSRoundEnd_TargetBombed = 1,
    CSRoundEnd_VIPEscaped,
    CSRoundEnd_VIPKilled,
    CSRoundEnd_TerroristsEscaped,
    CSRoundEnd_CTStoppedEscape,
    CSRoundEnd_TerroristsStopped,
    CSRoundEnd_BombDefused,
    CSRoundEnd_CTWin,
    CSRoundEnd_TerroristWin,
    CSRoundEnd_Draw,
    CSRoundEnd_HostagesRescued,
    CSRoundEnd_TargetSaved,
    CSRoundEnd_HostagesNotRescued,
    CSRoundEnd_TerroristsNotEscaped,
    CSRoundEnd_VIPNotEscaped,
    CSRoundEnd_GameStart,
    CSRoundEnd_TerroristsSurrender,
    CSRoundEnd_CTSurrender,
    CSRoundEnd_TerroristsPlanted,
    CSRoundEnd_CTsReachedHostage,
    CSRoundEnd_SurvivalWin,
    CSRoundEnd_SurvivalDraw,
};

/**
 * Chat colours: put one in front of the text it colours. A message that
 * starts with a colour needs a leading space or CHAT_DEFAULT before it, or
 * the client drops the first one. SourcePawn: the same CHAT_* names.
 */
#define CHAT_DEFAULT        "\x01"
#define CHAT_WHITE          "\x01"
#define CHAT_DARKRED        "\x02"
#define CHAT_LIGHTPURPLE    "\x03"
#define CHAT_GREEN          "\x04"
#define CHAT_OLIVE          "\x05"
#define CHAT_LIME           "\x06"
#define CHAT_RED            "\x07"
#define CHAT_GREY           "\x08"
#define CHAT_YELLOW         "\x09"
#define CHAT_SILVER         "\x0A"
#define CHAT_BLUEGREY       "\x0A"
#define CHAT_BLUE           "\x0B"
#define CHAT_DARKBLUE       "\x0C"
#define CHAT_PURPLE         "\x0E"
#define CHAT_MAGENTA        "\x0E"
#define CHAT_LIGHTRED       "\x0F"
#define CHAT_GOLD           "\x10"
#define CHAT_ORANGE         "\x10"

#endif // _INCLUDE_SOURCE2MOD_CONSTS_H
