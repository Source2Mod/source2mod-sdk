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

// The engine interfaces the SDK's code reaches through the API. In every
// binary (plugin and core): each is one call into the API, nothing to
// forward.

#include "source2mod/sdk/schemasystem_helper.h"

#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModPlugin.h"

SOURCE2MOD_GLOBALVARS();

IGameEventManager2* GetGameEventManager()
{
    return g_pSource2ModAPI->GetGameEventManager();
}

CGlobalVars* GetGlobalVars()
{
    return g_pSource2ModAPI->GetGlobalVars();
}

ICvar* GetCVar()
{
    return g_pSource2ModAPI->GetCVar();
}

ISource2Server* GetSource2Server()
{
    return g_pSource2ModAPI->GetSource2Server();
}

IVEngineServer2* GetEngineServer()
{
    return g_pSource2ModAPI->GetEngineServer();
}

IGameEventSystem* GetGameEventSystem()
{
    return g_pSource2ModAPI->GetGameEventSystem();
}
INetworkMessages* GetNetworkMessages()
{
    return g_pSource2ModAPI->GetNetworkMessages();
}

INetworkServerService* GetNetworkServerService()
{
    return g_pSource2ModAPI->GetNetworkServerService();
}

CGameEntitySystem* GetEntitySystem()
{
    return g_pSource2ModAPI->GetEntitySystem();
}

CSchemaSystem* GetSchemaSystem()
{
    return g_pSource2ModAPI->GetSchemaSystem();
}
