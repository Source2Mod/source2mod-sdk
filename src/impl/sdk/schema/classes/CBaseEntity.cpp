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

#include "source2mod/sdk/schema/classes/CBaseEntity.h"

#include "source2mod/ISource2ModAddresses.h"
#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModEntities.h"
#include "source2mod/ISource2ModGameConfig.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/sdk/schema/classes/CBodyComponent.h"
#include "source2mod/sdk/schema/classes/CCSPlayerController.h"
#include "source2mod/sdk/schema/classes/CCSPlayerPawn.h"
#include "source2mod/sdk/schema/classes/CCollisionProperty.h"
#include "source2mod/sdk/schema/classes/CEntitySubclassVDataBase.h"
#include "source2mod/sdk/schema/classes/CGameSceneNode.h"
#include "source2mod/utils/virtual.h"
SOURCE2MOD_GLOBALVARS();

CBaseEntity* CBaseEntity::CreateEntityByName(const char* pszClassName)
{
    return g_pSource2ModEntities->CreateEntityByName(pszClassName);
}

// Called here rather than through ISource2ModEntities, which has no way to say
// which chain to take; the gamedata functions (ISource2ModFunctions) are the
// same ones that interface calls.
void CBaseEntity::AcceptInput(const char* pszInput, CEntityInstance* pActivator, CEntityInstance* pCaller, const char* pszValue, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(bool(CEntityInstance*, const char*, CEntityInstance*, CEntityInstance*, const variant_t&),
                             "CEntityInstance::AcceptInput", eChain, this, pszInput, pActivator, pCaller, variant_t(pszValue));
}

void CBaseEntity::AddEntityIOEvent(const char* pszInput, CEntityInstance* pActivator, CEntityInstance* pCaller, const char* pszValue, float flDelay, HookChain eChain)
{
    CGameEntitySystem* pEntitySystem = GetEntitySystem();
    if (!pEntitySystem)
        return;

    SOURCE2MOD_CALL_FUNCTION(void(CEntitySystem*, CEntityInstance*, const char*, CEntityInstance*, CEntityInstance*, const variant_t&, float,
                                  void*, void*),
                             "CEntitySystem::AddEntityIOEvent", eChain, reinterpret_cast<CEntitySystem*>(pEntitySystem), this,
                             pszInput, pActivator, pCaller, variant_t(pszValue), flDelay, nullptr, nullptr);
}

Vector CBaseEntity::GetAbsOrigin()
{
    return m_CBodyComponent->m_pSceneNode->m_vecAbsOrigin();
}

QAngle CBaseEntity::GetAngRotation()
{
    return m_CBodyComponent->m_pSceneNode->m_angRotation();
}

QAngle CBaseEntity::GetAbsRotation()
{
    return m_CBodyComponent->m_pSceneNode->m_angAbsRotation();
}

Vector CBaseEntity::GetAbsVelocity()
{
    return m_vecAbsVelocity;
}

void CBaseEntity::SetAbsOrigin(Vector vecOrigin)
{
    m_CBodyComponent->m_pSceneNode->m_vecAbsOrigin = vecOrigin;
}

void CBaseEntity::SetAbsRotation(QAngle angAbsRotation)
{
    m_CBodyComponent->m_pSceneNode->m_angAbsRotation = angAbsRotation;
}

void CBaseEntity::SetAngRotation(QAngle angRotation)
{
    m_CBodyComponent->m_pSceneNode->m_angRotation = angRotation;
}

void CBaseEntity::SetAbsVelocity(Vector vecVelocity)
{
    m_vecAbsVelocity = vecVelocity;
}

void CBaseEntity::SetBaseVelocity(Vector vecVelocity)
{
    m_vecBaseVelocity = vecVelocity;
}

CEntitySubclassVDataBase* CBaseEntity::GetVData()
{
    return *reinterpret_cast<CEntitySubclassVDataBase**>(reinterpret_cast<uint8*>(m_nSubclassID()) + 4);
}

void CBaseEntity::DispatchSpawn(CEntityKeyValues* pEntityKeyValues, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CBaseEntity*, CEntityKeyValues*), "CBaseEntity::DispatchSpawn", eChain, this, pEntityKeyValues);
}

void CBaseEntity::Teleport(const Vector* pPosition, const QAngle* pAngles, const Vector* pVelocity, HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CBaseEntity*, const Vector*, const QAngle*, const Vector*), "CBaseEntity::Teleport", eChain, this,
                             pPosition, pAngles, pVelocity);
}

void CBaseEntity::SetMoveType(MoveType_t nMoveType)
{
    m_MoveType = nMoveType;
    m_nActualMoveType = nMoveType;
}

uint8 CBaseEntity::GetCollisionGroup()
{
    if (!m_pCollision())
        return 0;

    return m_pCollision->m_collisionAttribute().m_nCollisionGroup();
}

void CBaseEntity::SetCollisionGroup(uint8 nCollisionGroup)
{
    if (!m_pCollision())
        return;

    m_pCollision->m_collisionAttribute().m_nCollisionGroup = nCollisionGroup;
    m_pCollision->m_CollisionGroup = nCollisionGroup;
    CollisionRulesChanged();
}

void CBaseEntity::CollisionRulesChanged()
{
    SOURCE2MOD_CALL_FUNCTION(void(CBaseEntity*), "CBaseEntity::CollisionRulesChanged", HookChain::Run, this);
}

int CBaseEntity::GetIndex()
{
    return GetEntityIndex().Get();
}

const char* CBaseEntity::GetName() const
{
    return m_pEntity->m_name.String();
}

void CBaseEntity::Remove(HookChain eChain)
{
    SOURCE2MOD_CALL_FUNCTION(void(CEntityInstance*), "UTIL::Remove", eChain, this);
}

void CBaseEntity::DispatchParticleEffect(const char* pszParticleName, ParticleAttachment_t nAttachType, uint8 nAttachmentPoint,
                                         bool bResetAllParticlesOnEntity, IRecipientFilter* pFilter, HookChain eChain)
{
    // No attachment name token and the default split-screen slot, as the
    // game's own entity-attached calls pass them.
    SOURCE2MOD_CALL_FUNCTION(void(const char*, int, CBaseEntity*, uint8, uint32, bool, int, IRecipientFilter*, uint8), "DispatchParticleEffect",
                             eChain, pszParticleName, static_cast<int>(nAttachType), this, nAttachmentPoint, 0u,
                             bResetAllParticlesOnEntity, -1, pFilter, static_cast<uint8>(0));
}

CCSPlayerController* CBaseEntity::IsPlayerController()
{
    const char* pszClassname = GetClassname();
    return pszClassname && !V_strcmp(pszClassname, "cs_player_controller") ? reinterpret_cast<CCSPlayerController*>(this) : nullptr;
}

CCSPlayerPawn* CBaseEntity::IsPlayerPawn()
{
    const char* pszClassname = GetClassname();
    return pszClassname && !V_strcmp(pszClassname, "player") ? reinterpret_cast<CCSPlayerPawn*>(this) : nullptr;
}

SharedTeams_t CBaseEntity::GetTeam()
{
    const uint8_t nTeam = m_iTeamNum();
    return nTeam <= CS_TEAM_CT ? static_cast<SharedTeams_t>(nTeam) : CS_TEAM_NONE;
}

bool CBaseEntity::IsAlive()
{
    return m_lifeState() == LIFE_ALIVE;
}

void CBaseEntity::AddFlags(uint32_t nMask)
{
    m_fFlags(m_fFlags() | nMask);
}

void CBaseEntity::ClearFlags(uint32_t nMask)
{
    m_fFlags(m_fFlags() & ~nMask);
}

bool CBaseEntity::HasFlags(uint32_t nMask)
{
    return (m_fFlags() & nMask) == nMask;
}
