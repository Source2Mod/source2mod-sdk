//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: The game's trace filters (see tracefilter.h), rebuilt from the
//          2026-10-23 server binaries. Out of line because of the include
//          cycle noted in ISource2ModTrace.h, and in every binary.
//
//=============================================================================

#include "source2mod/sdk/game/shared/tracefilter.h"
#include "source2mod/sdk/schema/classes/CBaseEntity.h"
#include "source2mod/sdk/schema/classes/CBaseCSGrenadeProjectile.h"
#include "source2mod/sdk/schema/classes/CBaseGrenade.h"
#include "source2mod/sdk/schema/classes/CBasePlayerPawn.h"
#include "source2mod/sdk/schema/classes/CBodyComponent.h"
#include "source2mod/sdk/schema/classes/CCSWeaponBase.h"
#include "source2mod/sdk/schema/classes/CCollisionProperty.h"
#include "source2mod/sdk/schema/classes/CGameSceneNode.h"
#include "source2mod/sdk/schema/classes/CHostage.h"
#include "source2mod/sdk/schema/classes/VPhysicsCollisionAttribute_t.h"

#include <tier0/convar.h>

#include <algorithm>

namespace
{
    constexpr uint64 TEAM_LAYERS = CONTENTS_CSGO_TEAM1 | CONTENTS_CSGO_TEAM2;

    CBaseEntity* AsEntity(CEntityInstance* pEntity)
    {
        return static_cast<CBaseEntity*>(pEntity);
    }

    uint32 HandleOf(CEntityInstance* pEntity)
    {
        return pEntity ? pEntity->GetRefEHandle().ToInt() : INVALID_EHANDLE_INDEX;
    }

    // The entity's move parent (its scene node's parent's owner).
    CBaseEntity* MoveParent(CBaseEntity* pEntity)
    {
        CBodyComponent* pBody = pEntity->m_CBodyComponent();
        CGameSceneNode* pNode = pBody ? pBody->m_pSceneNode() : nullptr;
        CGameSceneNode* pParent = pNode ? pNode->m_pParent() : nullptr;
        return pParent ? AsEntity(pParent->m_pOwner()) : nullptr;
    }

    // The game's: from the first entity up the move-parent chain with a collision property.
    uint16 HierarchyIdOf(CBaseEntity* pEntity)
    {
        for (CBaseEntity* p = pEntity; p; p = MoveParent(p))
        {
            if (CCollisionProperty* pCollision = p->m_pCollision())
                return pCollision->m_collisionAttribute().m_nHierarchyId();
        }

        return 0;
    }

    // The game's IsPlayerPawn() is true for the whole CBasePlayerPawn tree (observer pawns too).
    bool IsPlayerPawn(CEntityInstance* pEntity)
    {
        return pEntity && dynamic_cast<CBasePlayerPawn*>(AsEntity(pEntity));
    }

    bool IsNPC(CEntityInstance* pEntity)
    {
        return pEntity && dynamic_cast<CHostage*>(AsEntity(pEntity));
    }

    uint64 TeamLayersOf(CBaseEntity* pEntity)
    {
        CCollisionProperty* pCollision = pEntity ? pEntity->m_pCollision() : nullptr;
        return pCollision ? pCollision->m_collisionAttribute().m_nInteractsWith() & TEAM_LAYERS : 0;
    }

    bool SolidTeammates()
    {
        static ConVarRefAbstract s_cvar("mp_solid_teammates");
        return s_cvar.IsValidRef() && s_cvar.GetInt() == 2;
    }

    bool OnLadder(CBaseEntity* pEntity)
    {
        return pEntity->m_nActualMoveType() == MOVETYPE_LADDER;
    }

    // The game's check for one player standing on another (mp_solid_teammates 2):
    // one is the other's ground entity, or one's box is wholly above the other's.
    bool IsStacked(CBaseEntity* pA, CBaseEntity* pB)
    {
        if (!pA || !pB || pA == pB || OnLadder(pA) || OnLadder(pB))
            return false;

        CCollisionProperty* pCollA = pA->m_pCollision();
        CCollisionProperty* pCollB = pB->m_pCollision();
        if (!pCollA || !pCollB)
            return false;

        if (pA->m_hGroundEntity().Get() == pB || pB->m_hGroundEntity().Get() == pA)
            return true;

        const float flBottomA = pA->GetAbsOrigin().z + pCollA->m_vecMins().z;
        const float flTopA = pA->GetAbsOrigin().z + pCollA->m_vecMaxs().z;
        const float flBottomB = pB->GetAbsOrigin().z + pCollB->m_vecMins().z;
        const float flTopB = pB->GetAbsOrigin().z + pCollB->m_vecMaxs().z;
        return flBottomA >= flTopB || flBottomB >= flTopA;
    }

    CBaseEntity* EntityFromId(uint32 nHandle)
    {
        return nHandle == INVALID_EHANDLE_INDEX ? nullptr : CHandle<CBaseEntity>(nHandle).Get();
    }
}

/* =========================
CTraceFilterBase
========================= */

CTraceFilterBase::CTraceFilterBase(uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilter(nInteractsWith, nCollisionGroup, false)
{
}

void CTraceFilterBase::SetPassEntity(int nSlot, CBaseEntity* pEntity)
{
    if (nSlot < 0 || nSlot > 1)
        return;

    m_nEntityIdsToIgnore[nSlot] = HandleOf(pEntity);
    m_nHierarchyIds[nSlot] = HierarchyIdOf(pEntity);

    CCollisionProperty* pCollision = pEntity ? pEntity->m_pCollision() : nullptr;
    if (!pEntity || (pCollision && (pCollision->m_usSolidFlags() & FSOLID_NOT_SOLID)))
        m_nOwnerIdsToIgnore[nSlot] = INVALID_EHANDLE_INDEX;
    else
        m_nOwnerIdsToIgnore[nSlot] = HandleOf(pEntity->m_hOwnerEntity().Get());
}

void CTraceFilterBase::SetFromEntity(CBaseEntity* pEntity)
{
    SetPassEntity(0, pEntity);

    CCollisionProperty* pCollision = pEntity ? pEntity->m_pCollision() : nullptr;
    if (!pCollision)
        return;

    VPhysicsCollisionAttribute_t& attr = pCollision->m_collisionAttribute();
    m_nInteractsWith = attr.m_nInteractsWith();
    m_nInteractsAs = attr.m_nInteractsAs();
    m_nInteractsExclude = attr.m_nInteractsExclude();
    m_nCollisionGroup = attr.m_nCollisionGroup();
}

/* =========================
Plain filters
========================= */

CTraceFilterSimple::CTraceFilterSimple(CBaseEntity* pPassEntity, uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    SetPassEntity(0, pPassEntity);

    if (IsPlayerPawn(pPassEntity))
        m_nInteractsExclude |= nInteractsWith & TEAM_LAYERS;
}

CTraceFilterSimple::CTraceFilterSimple()
    : CTraceFilterBase(MASK_ALL, COLLISION_GROUP_DEFAULT)
{
}

CTraceFilterLOS::CTraceFilterLOS(CBaseEntity* pPassEntity1, CBaseEntity* pPassEntity2, uint64 nInteractsWith)
    : CTraceFilterBase(nInteractsWith, COLLISION_GROUP_CONDITIONALLY_SOLID)
{
    SetPassEntity(0, pPassEntity1);
    SetPassEntity(1, pPassEntity2);
}

CTraceFilterAsEntity::CTraceFilterAsEntity(CBaseEntity* pEntity)
{
    SetFromEntity(pEntity);
}

CTraceFilterHitEntitiesOnly::CTraceFilterHitEntitiesOnly(uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    m_nObjectSetMask = RNQUERY_OBJECTS_KEYFRAMED | RNQUERY_OBJECTS_DYNAMIC | RNQUERY_OBJECTS_LOCATABLE;
    m_bIterateEntities = true;
}

/* =========================
Filters with a rule of their own
========================= */

CTraceFilterList::CTraceFilterList(uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    m_bIterateEntities = true;
}

bool CTraceFilterList::ShouldHitEntity(CEntityInstance* pEntity)
{
    return std::find(m_Ignore.begin(), m_Ignore.end(), pEntity) == m_Ignore.end();
}

CTraceFilterPlayerMovementCS::CTraceFilterPlayerMovementCS(CBaseEntity* pPawn, uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    if (!nInteractsWith)
    {
        CCollisionProperty* pCollision = pPawn ? pPawn->m_pCollision() : nullptr;
        m_nInteractsWith = pCollision ? pCollision->m_collisionAttribute().m_nInteractsWith() : MASK_PLAYERSOLID;
    }

    SetPassEntity(0, pPawn);
    m_nInteractsAs = CONTENTS_PLAYER;
    m_bHitSolidRequiresGenerateContacts = true;
    m_bIterateEntities = true;
}

bool CTraceFilterPlayerMovementCS::ShouldHitEntity(CEntityInstance* pEntity)
{
    if (!pEntity)
        return false;

    CBaseEntity* pPass = EntityFromId(m_nEntityIdsToIgnore[0]);
    if (!pPass)
        return false;

    if (m_nCollisionGroup != COLLISION_GROUP_PLAYER_MOVEMENT)
        return true;

    CBaseEntity* pOther = AsEntity(pEntity);
    CCollisionProperty* pCollision = pOther->m_pCollision();
    if (pCollision && pCollision->m_CollisionGroup() == COLLISION_GROUP_PROJECTILE)
        return false;

    const uint64 nPassTeam = TeamLayersOf(pPass);
    if (TeamLayersOf(pOther) != nPassTeam || !nPassTeam)
        return true;

    if (!SolidTeammates())
        return false;

    return IsStacked(pOther, pPass);
}

CTraceFilterIgnoreTeammates::CTraceFilterIgnoreTeammates(CBaseEntity* pPassEntity, uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    SetPassEntity(0, pPassEntity);
    m_bIterateEntities = true;
}

bool CTraceFilterIgnoreTeammates::ShouldHitEntity(CEntityInstance* pEntity)
{
    CBaseEntity* pPass = EntityFromId(m_nEntityIdsToIgnore[0]);
    if (!pPass || !pEntity || pEntity == pPass)
        return false;

    return AsEntity(pEntity)->m_iTeamNum() != pPass->m_iTeamNum();
}

CTraceFilterKnifeIgnoreTeammates::CTraceFilterKnifeIgnoreTeammates(CBaseEntity* pPassEntity)
    : CTraceFilterIgnoreTeammates(pPassEntity, MASK_SOLID, COLLISION_GROUP_DEFAULT)
{
    m_bHitSolidRequiresGenerateContacts = true;
}

CTraceFilterTaserIgnoreTeammates::CTraceFilterTaserIgnoreTeammates(CBaseEntity* pPassEntity)
    : CTraceFilterIgnoreTeammates(pPassEntity, MASK_SHOT, COLLISION_GROUP_DEFAULT)
{
    m_bHitSolidRequiresGenerateContacts = true;
}

CTraceFilterSkipPlayersAndNPCs::CTraceFilterSkipPlayersAndNPCs(CBaseEntity* pPassEntity, uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    SetPassEntity(0, pPassEntity);
    m_bIterateEntities = true;
}

bool CTraceFilterSkipPlayersAndNPCs::ShouldHitEntity(CEntityInstance* pEntity)
{
    return !pEntity || (!IsPlayerPawn(pEntity) && !IsNPC(pEntity));
}

bool CTraceFilterSkipPlayersAndNPCsAndChildren::ShouldHitEntity(CEntityInstance* pEntity)
{
    for (CBaseEntity* p = AsEntity(pEntity); p; p = MoveParent(p))
    {
        if (IsNPC(p) || IsPlayerPawn(p))
            return false;
    }

    return true;
}

CTraceFilterIgnoreGrenades::CTraceFilterIgnoreGrenades(CBaseEntity* pPassEntity, uint64 nInteractsWith, int nCollisionGroup)
    : CTraceFilterBase(nInteractsWith, nCollisionGroup)
{
    SetPassEntity(0, pPassEntity);
    m_bIterateEntities = true;
}

bool CTraceFilterIgnoreGrenades::ShouldHitEntity(CEntityInstance* pEntity)
{
    return !pEntity || !dynamic_cast<CBaseCSGrenadeProjectile*>(AsEntity(pEntity));
}

CTraceFilterNoPlayersAndFlashbangPassableAnims::CTraceFilterNoPlayersAndFlashbangPassableAnims(CBaseEntity* pPassEntity1, CBaseEntity* pPassEntity2, uint64 nInteractsWith)
    : CTraceFilterBase(nInteractsWith, COLLISION_GROUP_DEFAULT)
{
    SetPassEntity(0, pPassEntity1);
    SetPassEntity(1, pPassEntity2);
    m_nInteractsExclude |= CONTENTS_PLAYER | CONTENTS_DEBRIS;
    m_bIterateEntities = true;
}

bool CTraceFilterNoPlayersAndFlashbangPassableAnims::ShouldHitEntity(CEntityInstance* pEntity)
{
    if (!pEntity)
        return true;

    CBaseEntity* pOther = AsEntity(pEntity);
    if (dynamic_cast<CCSWeaponBase*>(pOther) || dynamic_cast<CBaseGrenade*>(pOther))
        return false;

    return !dynamic_cast<CHostage*>(pOther);
}
