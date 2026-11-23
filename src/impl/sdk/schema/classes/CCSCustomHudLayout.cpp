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

#include "source2mod/sdk/schema/classes/CCSCustomHudLayout.h"

#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModCustomHud.h"
#include "source2mod/ISource2ModFunctions.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/sdk/schema/classes/CCSPlayerController.h"

#include <entitykeyvalues.h>

#include <cstring>
#include <string>
#include <unordered_set>
SOURCE2MOD_GLOBALVARS();

namespace
{
    /// Panorama resolves layouts out of this directory; callers name the layout,
    /// not the path.
    constexpr const char* LAYOUT_PATH_PREFIX = "panorama/layout/custom_game/";
    constexpr const char* LAYOUT_PATH_SUFFIX = ".vxml_c";

    // Values go through the game's setters on the state: only they mark the changed
    // element of a nested vector, which a state change from out here cannot do, and
    // the client applies a value by that element. The string tables stay ours: the
    // game's setters above these cap them at 1024 entries and drop the change silently.

    /// The tables are networked with the entity and a client needs every name the states point at, so
    /// a name can never be dropped during a map: the tables only grow. Past what the serializer can pack
    /// (one plugin's class per picture reached m_vecClassNames[7080]) the engine stops the server with
    /// "PackEntity: g_pFlattenedSerializers->Encode false". A plugin bug must not do that: a full table
    /// takes no new names, the change is dropped and logged once per layout and table.
    constexpr int MAX_TABLE_STRINGS = 4096;
    constexpr int MAX_TABLE_CHARS = 32 * 1024;

    /// Index of a string in one of the layout's string tables, appending it if
    /// it is not there yet. The per-player states store these indices, not the
    /// strings themselves, so a new string has to reach the clients with them:
    /// without the table's own state change a client gets a state pointing past
    /// the end of its copy of the table. -1 when the table is full.
    template <typename Field>
    int InternString(CCSCustomHudLayout* pLayout, Field& field, const char* pszTable, const char* pszValue)
    {
        CUtlVector<CUtlString>& vecTable = field();
        int index = vecTable.Find(pszValue);
        if (index != -1)
            return index;

        int nChars = static_cast<int>(strlen(pszValue)) + 1;
        for (int i = 0; i < vecTable.Count(); ++i)
            nChars += vecTable[i].Length() + 1;

        if (vecTable.Count() >= MAX_TABLE_STRINGS || nChars > MAX_TABLE_CHARS)
        {
            static std::unordered_set<std::string> s_reported;
            const std::string sKey = std::to_string(pLayout->GetHandle().ToInt()) + '|' + pszTable;
            if (s_reported.size() > 256)
                s_reported.clear();
            if (s_reported.insert(sKey).second)
            {
                g_pSource2ModAPI->Log("custom_hud_layout '%s' (%s): %s is full (%d names, %d chars), \"%s\" and every further new name "
                                      "are dropped until the map changes. A plugin is making names up per value (one class per "
                                      "picture, per player, ...): use a fixed set.\n",
                                      pLayout->m_strLayout().String(), pLayout->GetName(), pszTable, vecTable.Count(),
                                      nChars - static_cast<int>(strlen(pszValue)) - 1,
                                      pszValue);
            }
            return -1;
        }

        index = vecTable.AddToTail(pszValue);
        field.NetworkStateChanged();
        return index;
    }
}

CCSCustomHudLayout* CCSCustomHudLayout::Create(const char* pszLayout, const char* pszTargetName)
{
    if (!pszLayout || !*pszLayout)
        return nullptr;

    const std::string sLayoutPath = LAYOUT_PATH_PREFIX + std::string(pszLayout) + LAYOUT_PATH_SUFFIX;

    auto* pKeyValues = g_pSource2ModEntities->CreateEntityKeyValues("custom_hud_layout");
    if (!pKeyValues)
        return nullptr;

    pKeyValues->SetString("layout", sLayoutPath.c_str());

    if (pszTargetName && *pszTargetName)
        pKeyValues->SetString("targetname", pszTargetName);

    return reinterpret_cast<CCSCustomHudLayout*>(g_pSource2ModEntities->SpawnEntityByClassname("custom_hud_layout", pKeyValues));
}

// A player's state is an element of m_vecPlayerLayoutStates, embedded without a NetworkVar of its own: the
// state change its fields raise finds no slot and goes nowhere, so the entity's field carries it.
void CCSCustomHudLayout::StateChanged(CCSPlayerController* pController)
{
    if (pController)
        m_vecPlayerLayoutStates.NetworkStateChanged();
    else
        m_globalLayoutState.NetworkStateChanged();
}

CCSCustomHudLayoutState& CCSCustomHudLayout::GetLayoutState(CCSPlayerController* pController)
{
    if (!pController)
        return m_globalLayoutState();

    // m_vecPlayerLayoutStates is a CUtlVectorEmbeddedNetworkVar, not the plain
    // CUtlVector its declaration suggests, so indexing it directly walks the
    // wrong memory. The schema system publishes an element accessor for exactly
    // this case; go through it.
    const SchemaCollectionManipulatorFn_t pfnManipulator = m_vecPlayerLayoutStates.GetManipulator();
    if (!pfnManipulator)
        return m_globalLayoutState();

    void* pElement = pfnManipulator(SCHEMA_COLLECTION_MANIPULATOR_ACTION_GET_ELEMENT,
                                    &m_vecPlayerLayoutStates(),
                                    pController->GetPlayerSlot().Get(), 0);
    if (!pElement)
        return m_globalLayoutState();

    return *static_cast<CCSCustomHudLayoutState*>(pElement);
}

void CCSCustomHudLayout::SetHasClass(const char* pszPanelId, const char* pszClassName, bool bHasClass, CCSPlayerController* pController)
{
    if (!pszPanelId || !pszClassName)
        return;

    const int panelIndex = InternString(this, m_vecPanelIds, "m_vecPanelIds", pszPanelId);
    const int classIndex = panelIndex == -1 ? -1 : InternString(this, m_vecClassNames, "m_vecClassNames", pszClassName);
    if (classIndex == -1)
        return;

    auto& layoutState = GetLayoutState(pController);

    static ISource2ModFunction* s_pSetter = g_pSource2ModFunctions->Find("CCSCustomHudLayoutState::SetHasClass");
    if (s_pSetter)
    {
        s_pSetter->Call<void(CCSCustomHudLayoutState*, int, int, int)>(&layoutState, panelIndex, classIndex, bHasClass ? 1 : 0);
    }
    else
    {
        // Its signature broke with a game update: most changes still get through this way.
        HUDPanelHasClass_t hasClass(static_cast<uint16>(panelIndex), static_cast<uint16>(classIndex), bHasClass);

        const int hasClassIndex = layoutState.m_vecHasClasses().Find(hasClass);

        if (hasClassIndex == -1)
            layoutState.m_vecHasClasses().AddToTail(hasClass);
        else
            layoutState.m_vecHasClasses().Element(hasClassIndex).m_eClassStatus = hasClass.m_eClassStatus;

        layoutState.m_vecHasClasses.NetworkStateChanged();
    }

    StateChanged(pController);
}

void CCSCustomHudLayout::SetDialogVariableString(const char* pszPanelId, const char* pszVariableName, const char* pszValue, CCSPlayerController* pController)
{
    if (!pszPanelId || !pszVariableName || !pszValue)
        return;

    const int panelIndex = InternString(this, m_vecPanelIds, "m_vecPanelIds", pszPanelId);
    const int variableIndex = panelIndex == -1 ? -1 : InternString(this, m_vecDialogVariableNames, "m_vecDialogVariableNames", pszVariableName);
    if (variableIndex == -1)
        return;

    auto& layoutState = GetLayoutState(pController);

    static ISource2ModFunction* s_pSetter = g_pSource2ModFunctions->Find("CCSCustomHudLayoutState::SetDialogVariableString");
    if (s_pSetter)
    {
        const CUtlString sValue(pszValue);
        s_pSetter->Call<void(CCSCustomHudLayoutState*, int, int, const CUtlString*)>(&layoutState, panelIndex, variableIndex, &sValue);
    }
    else
    {
        // Its signature broke with a game update: most texts still get through this way.
        HUDPanelDialogVariableString_t dialogVariable(static_cast<uint16>(panelIndex), static_cast<uint16>(variableIndex), pszValue, true);

        const int dialogVariableIndex = layoutState.m_vecDialogVariableStrings().Find(dialogVariable);

        if (dialogVariableIndex == -1)
            layoutState.m_vecDialogVariableStrings().AddToTail(dialogVariable);
        else
            layoutState.m_vecDialogVariableStrings().Element(dialogVariableIndex).m_sValue = pszValue;

        layoutState.m_vecDialogVariableStrings.NetworkStateChanged();
    }

    StateChanged(pController);
}

void CCSCustomHudLayout::SetInputCaptureEnabled(bool bEnable, CCSPlayerController* pController)
{
    GetLayoutState(pController).m_bInputCaptureEnabled = bEnable;
    StateChanged(pController);
}

bool CCSCustomHudLayout::IsInputCaptureEnabled(CCSPlayerController* pController)
{
    return GetLayoutState(pController).m_bInputCaptureEnabled();
}

void CCSCustomHudLayout::HookClick(std::function<void(CCSPlayerController*, CCSCustomHudLayout*, const char*)> callback)
{
    g_pSource2ModCustomHud->HookCustomHudClick(this, std::move(callback));
}

void CCSCustomHudLayout::UnhookClicks()
{
    g_pSource2ModCustomHud->UnhookCustomHudClick(this);
}
