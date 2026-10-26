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

#include "source2mod/sdk/schema/classes/CCSGameRules.h"

#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModEntities.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/sdk/schema/classes/CCSPlayerController.h"
#include "source2mod/sdk/schema/classes/CCSPlayerPawn.h"
#include "source2mod/utils/virtual.h"
SOURCE2MOD_GLOBALVARS();

void CCSGameRules::TerminateRound(float flDelay, int32_t eRoundEndReason, uint32 nTeamId, HookChain eChain)
{
    // The team is taken by pointer, and null is what "no winning team" looks
    // like -- passing a pointer to zero is not the same thing.
    uint32 nTeam = nTeamId;
    uint32* pnTeamId = nTeamId > 0 ? &nTeam : nullptr;

    // Bypass by default, for the same reason as CCSPlayerController::TakeDamage:
    // ending the round because the server was told to is not the game ending
    // the round, and a gamemode's TerminateRound hook exists to police the
    // latter. HookChain::Run lets those hooks see (and veto) this one too.
    //
    // The argument order is not the same on both platforms (the gamedata
    // entry's "function" object lists both).
#ifdef _WIN32
    SOURCE2MOD_CALL_FUNCTION(void(CCSGameRules*, float, uint32, uint32*), "CCSGameRules::TerminateRound", eChain, this, flDelay,
                             static_cast<uint32>(eRoundEndReason), pnTeamId);
#else
    SOURCE2MOD_CALL_FUNCTION(void(CCSGameRules*, uint32, uint32*, float), "CCSGameRules::TerminateRound", eChain, this,
                             static_cast<uint32>(eRoundEndReason), pnTeamId, flDelay);
#endif
}

CBaseEntity* CCSGameRules::FindPickerEntity(CBasePlayerController* pPlayer)
{
    return g_pSource2ModEntities->FindPickerEntity(pPlayer, this);
}

CCSPlayerController* CCSGameRules::GetClientAimTarget(CCSPlayerController* pPlayer)
{
    auto* pPawn = static_cast<CCSPlayerPawn*>(FindPickerEntity(pPlayer));
    if (!pPawn) return nullptr;

    return V_strcmp(pPawn->GetClassname(), "player") == 0 ? pPawn->m_hOriginalController().Get() : nullptr;
}

void CCSGameRules::GoToIntermission(bool bAbortedMatch, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CCSGameRules*, bool), "CCSGameRules::GoToIntermission", eChain, this, bAbortedMatch);
}
