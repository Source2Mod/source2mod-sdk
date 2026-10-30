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

// The ISource2ModHook helpers that talk to ISource2ModAPI and ISource2ModGameConfig.
// They live here rather than in ISource2ModHooks.h because that header is pulled
// in from the end of ISource2ModPlugin.h, where both are only forward-declared.

#ifndef SOURCE2MOD_CORE

#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/ISource2ModKHook.h"

SOURCE2MOD_GLOBALVARS();

void ISource2ModHook::Warn(const char* pszFormat, const char* pszName)
{
    if (g_pSource2ModAPI)
        g_pSource2ModAPI->Log(pszFormat, pszName ? pszName : "(null)");
}

ISource2ModModule* ISource2ModHook::GetModule(const char* pszName)
{
    return g_pSource2ModModules ? g_pSource2ModModules->Get(pszName) : nullptr;
}

int ISource2ModHook::GetOffset(const char* pszName)
{
    return g_pSource2ModGameConfig ? g_pSource2ModGameConfig->GetOffset(pszName) : -1;
}

ISource2ModMemory ISource2ModHook::ResolveSignature(const char* pszName)
{
    return g_pSource2ModGameConfig ? g_pSource2ModGameConfig->ResolveSignature(pszName) : ISource2ModMemory();
}

#endif // SOURCE2MOD_CORE
