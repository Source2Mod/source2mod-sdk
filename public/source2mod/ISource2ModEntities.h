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

* @file ISource2ModEntities.h
* @brief Interface for entity lookup and Source2 Entity I/O system.
*
* Provides:
* * Entity searching (classname, name, picker)
* * Entity creation
* * Input triggering
* * Output listening
*
* @note Entity I/O is the system used by maps to connect entities together.
  */

#ifndef _INCLUDE_ISOURCE2MOD_ENTITIES_H
#define _INCLUDE_ISOURCE2MOD_ENTITIES_H

#pragma once
#include "ISource2ModPlugin.h"
#include "source2mod/schema/schema.h"

#include <engine2/isource2server.h>
#include <engine2/ivengineserver2.h>
#include <igameevents.h>

#include <cstring>
#include <vector>

/* =========================
Forward declarations
========================= */

class CBaseEntity;
class CBasePlayerController;
class CEntityInstance;
class CCSGameRules;
class IEntityFindFilter;
class IEntityListener;

/**
 * Entity outputs firing (CEntityIOOutput::FireOutputInternal), for
 * ISource2ModEntities::AddEntityOutputListener. Called pre (post = false),
 * before the output's targets get it, and post, after; pre may answer
 * Action::Supersede and the output does not fire. Override and Ignore both
 * let it fire. The strongest answer of all listeners wins.
 */
class IEntityOutputListener
{
public:
    virtual Action OnEntityOutput(CEntityInstance* pCaller, const char* pszOutput, CEntityInstance* pActivator, float flDelay, bool post) = 0;

protected:
    ~IEntityOutputListener() = default;
};

/**

* @brief Interface for interacting with entities and Entity I/O system.
  */
#define SOURCE2MOD_ENTITIES_INTERFACE "ISource2ModEntities003"

class ISource2ModEntities
{
public:
    virtual ~ISource2ModEntities() = default;

    /**

    * @brief The active CCSGameRules, or null before the entity exists.
    *
    * Source2Mod captures this from cs_gamerules as the entity is created, so
    * it is live for the whole map and a plugin does not have to scan for the
    * proxy itself.
      */
    virtual CCSGameRules* GetGameRules() = 0;

    /**

    * @brief Finds entity under player's crosshair.
      */
    virtual CBaseEntity* FindPickerEntity(CBasePlayerController* pPlayer,
                                          CCSGameRules* pGameRules = nullptr) = 0;

    /**

    * @brief Finds entity by classname.
      */
    virtual CBaseEntity* FindEntityByClassname(CEntityInstance* pStart,
                                               const char* name) = 0;

    /**

    * @brief Finds entity by name.
      */
    virtual CBaseEntity* FindEntityByName(CEntityInstance* pStartEntity,
                                          const char* szName,
                                          CEntityInstance* pSearchingEntity = nullptr,
                                          CEntityInstance* pActivator = nullptr,
                                          CEntityInstance* pCaller = nullptr,
                                          IEntityFindFilter* pFilter = nullptr) = 0;

    /**

    * @brief Creates entity by classname.
      */
    virtual CBaseEntity* CreateEntityByName(const char* pszClassName) = 0;

    /**

    * @brief Sends input to entity.
    *
    * @note Equivalent to Source engine AcceptInput()
      */
    virtual void AcceptInput(CEntityInstance* pTarget,
                             const char* pszInput,
                             CEntityInstance* pActivator,
                             CEntityInstance* pCaller,
                             const char* pszValue = "") = 0;

    /**

    * @brief Queues an entity I/O event.
      */
    virtual void AddEntityIOEvent(CEntityInstance* pTarget,
                                  const char* pszInput,
                                  CEntityInstance* pActivator = nullptr,
                                  CEntityInstance* pCaller = nullptr,
                                  const char* pszValue = "",
                                  float flDelay = 0.0f) = 0;

    /**
     * @brief Entity events: the IEntityListener's OnEntityCreated (keyvalues not
     *        applied, not spawned yet), OnEntitySpawned, OnEntityDeleted (the
     *        last call for it), OnEntityParentChanged. Override what you need.
     *
     * @param pListener      The plugin whose class it is owns it; dropped for
     *                       you when that plugin unloads.
     * @param pszDesignerName Only entities of this designer name ("player",
     *                       "smokegrenade_projectile"); nullptr for every one.
     * @param pEntity        Only this one entity; nullptr for every one. Gone
     *                       with the entity, after its OnEntityDeleted.
     *
     * @note The same listener may be added again with other filters; it is then
     *       called once per matching registration. Remove drops them all.
     */
    virtual void AddEntityListener(IEntityListener* pListener,
                                   const char* pszDesignerName = nullptr,
                                   CEntityInstance* pEntity = nullptr) = 0;

    /// @brief Drops every registration of the listener.
    virtual void RemoveEntityListener(IEntityListener* pListener) = 0;

    /**
     * @brief Entity outputs: IEntityOutputListener::OnEntityOutput, pre and post.
     *
     * @param pListener      The plugin whose class it is owns it; dropped for
     *                       you when that plugin unloads.
     * @param pszDesignerName Only outputs of entities of this designer name
     *                       (the caller's); nullptr for every one.
     * @param pszOutput      Only this output ("OnTrigger", "OnStartTouch");
     *                       nullptr for every one.
     * @param pEntity        Only outputs of this one entity; nullptr for every
     *                       one. Gone with the entity.
     *
     * @note The same listener may be added again with other filters. Remove
     *       drops them all.
     */
    virtual void AddEntityOutputListener(IEntityOutputListener* pListener,
                                         const char* pszDesignerName = nullptr,
                                         const char* pszOutput = nullptr,
                                         CEntityInstance* pEntity = nullptr) = 0;

    /// @brief Drops every registration of the listener.
    virtual void RemoveEntityOutputListener(IEntityOutputListener* pListener) = 0;
};

#define GET_GAME_RULES()                          g_pSource2ModEntities->GetGameRules()
#define FIND_PICKER_ENTITY(player, ...)           g_pSource2ModEntities->FindPickerEntity(player, ##__VA_ARGS__)
#define FIND_ENTITY_BY_CLASSNAME(start, name)     g_pSource2ModEntities->FindEntityByClassname(start, name)
#define FIND_ENTITY_BY_NAME(start, name, ...)     g_pSource2ModEntities->FindEntityByName(start, name, ##__VA_ARGS__)
#define CREATE_ENTITY(classname)                  g_pSource2ModEntities->CreateEntityByName(classname)
#define ADD_ENTITY_LISTENER(l, ...)               g_pSource2ModEntities->AddEntityListener(l, ##__VA_ARGS__)
#define REMOVE_ENTITY_LISTENER(l)                 g_pSource2ModEntities->RemoveEntityListener(l)
#define ADD_ENTITY_OUTPUT_LISTENER(l, ...)        g_pSource2ModEntities->AddEntityOutputListener(l, ##__VA_ARGS__)
#define REMOVE_ENTITY_OUTPUT_LISTENER(l)          g_pSource2ModEntities->RemoveEntityOutputListener(l)
#define ACCEPT_INPUT(target, input, ...)          g_pSource2ModEntities->AcceptInput(target, input, ##__VA_ARGS__)
#define ADD_ENTITY_IO_EVENT(target, input, ...)   g_pSource2ModEntities->AddEntityIOEvent(target, input, ##__VA_ARGS__)


/**

* @brief Every entity whose designer name matches, cast to T.
*
* Walks the entity system's active list directly rather than going through the
* engine's finder, so it sees exactly the entities that are live right now and
* matches on m_designerName -- the name the map gave the entity, which is not
* always the classname the engine reports.
*
* @tparam T Type to cast each match to. No check is performed that the entity
*           really is a T, so pass the designer name and type consistently.
* @param pszDesignerName Designer name to match, exactly.
*
* @return Matches in list order; empty if the entity system is not up yet.
*
* @code
* for (auto* pProp : UTIL_FindAllEntitiesByDesignerName<CDynamicProp>("prop_dynamic"))
*     pProp->AcceptInput("Kill");
* @endcode
  */
template <typename T = CBaseEntity>
inline std::vector<T*> UTIL_FindAllEntitiesByDesignerName(const char* pszDesignerName)
{
    std::vector<T*> results;

    CGameEntitySystem* pEntitySystem = GetEntitySystem();
    if (!pszDesignerName || !pEntitySystem)
        return results;

    for (CEntityIdentity* pIdentity = pEntitySystem->m_EntityList.m_pFirstActiveEntity;
         pIdentity; pIdentity = pIdentity->m_pNext)
    {
        if (!pIdentity->m_pInstance)
            continue;

        const char* pszDesigner = pIdentity->m_designerName.String();
        if (!pszDesigner)
            continue;

        if (std::strcmp(pszDesigner, pszDesignerName) == 0)
            results.push_back(reinterpret_cast<T*>(pIdentity->m_pInstance));
    }

    return results;
}

#endif //_INCLUDE_ISOURCE2MOD_ENTITIES_H
