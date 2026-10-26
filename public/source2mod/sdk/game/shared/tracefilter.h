//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: The game's trace filters, rebuilt from the 2026-10-23 server
//          binaries (Linux libserver.so, checked against Windows server.dll):
//          the same defaults the game's constructors write and the same
//          ShouldHitEntity rules, without signatures.
//
//=============================================================================

#ifndef TRACEFILTER_H
#define TRACEFILTER_H
#ifdef _WIN32
#pragma once
#endif

#include <bspflags.h>
#include <const.h>
#include <gametrace.h>

#include <cstdint>
#include <vector>

class CBaseEntity;
class CEntityInstance;

/*
 * What every game filter starts from (its inlined CTraceFilter constructor):
 * the s2sdk defaults, except that the collision group and interacts-with
 * mask come from the caller and m_bIterateEntities is off -- a filter whose
 * ShouldHitEntity is only `return true` does not need to be asked per
 * entity. The classes below with a rule of their own turn it on.
 */
class CTraceFilterBase : public CTraceFilter
{
public:
	explicit CTraceFilterBase(uint64 nInteractsWith = MASK_ALL, int nCollisionGroup = COLLISION_GROUP_DEFAULT);

	/*
	 * The game's pass-entity rule for slot 0 or 1: the entity is ignored, so
	 * is its owner unless the entity is not solid (FSOLID_NOT_SOLID), and its
	 * hierarchy id is the one of the first entity up its move-parent chain
	 * that has a collision property. Null clears the slot.
	 */
	void SetPassEntity(int nSlot, CBaseEntity* pEntity);

	/*
	 * Trace as pEntity does (the game's helper for pushes and sweeps): the
	 * pass-entity rule for slot 0, then its collision attribute's
	 * interacts-as / -with / -exclude and collision group.
	 */
	void SetFromEntity(CBaseEntity* pEntity);
};

/*
 * The game's CCSTraceFilterSimple: ignore one entity (and its owner and
 * hierarchy, see SetPassEntity). For a player pawn the team layers it was
 * asked to interact with go into the exclude mask, so the pawn's own team
 * clips do not block -- pass the mask you trace with to the constructor for
 * that to be right. The plain "ignore this entity" filter.
 */
class CTraceFilterSimple : public CTraceFilterBase
{
public:
	explicit CTraceFilterSimple(CBaseEntity* pPassEntity, uint64 nInteractsWith = MASK_ALL, int nCollisionGroup = COLLISION_GROUP_DEFAULT);
	CTraceFilterSimple();
};

/*
 * The game's CTraceFilterLOS: line of sight. Up to two entities ignored,
 * collision group COLLISION_GROUP_CONDITIONALLY_SOLID; the game's callers
 * trace against CONTENTS_BLOCK_LOS (| CONTENTS_HITBOX to be stopped by
 * hitboxes too).
 */
class CTraceFilterLOS : public CTraceFilterBase
{
public:
	explicit CTraceFilterLOS(CBaseEntity* pPassEntity1, CBaseEntity* pPassEntity2 = nullptr, uint64 nInteractsWith = CONTENTS_BLOCK_LOS);
};

/*
 * "Trace as this entity": its own collision layers and group, it and its
 * owner ignored (CTraceFilterBase::SetFromEntity).
 */
class CTraceFilterAsEntity : public CTraceFilterBase
{
public:
	explicit CTraceFilterAsEntity(CBaseEntity* pEntity);
};

/*
 * The game's CTraceFilterList (VScript TraceLine's ignore list): any number
 * of entities ignored, by pointer.
 */
class CTraceFilterList : public CTraceFilterBase
{
public:
	explicit CTraceFilterList(uint64 nInteractsWith = MASK_ALL, int nCollisionGroup = COLLISION_GROUP_CONDITIONALLY_SOLID);

	void AddEntityToIgnore(CEntityInstance* pEntity) { m_Ignore.push_back(pEntity); }
	bool ShouldHitEntity(CEntityInstance* pEntity) override;

private:
	std::vector<CEntityInstance*> m_Ignore;
};

/*
 * The game's CTraceFilterPlayerMovementCS, what player movement traces with:
 * the pawn's own layers and COLLISION_GROUP_PLAYER_MOVEMENT, interacting as
 * a player. Projectiles never block; a teammate (same team layer) blocks only
 * with mp_solid_teammates 2, and then only when one stands on the other.
 */
class CTraceFilterPlayerMovementCS : public CTraceFilterBase
{
public:
	// nInteractsWith 0: the pawn's collision attribute's (as the game's
	// movement code passes it).
	explicit CTraceFilterPlayerMovementCS(CBaseEntity* pPawn, uint64 nInteractsWith = 0, int nCollisionGroup = COLLISION_GROUP_PLAYER_MOVEMENT);

	bool ShouldHitEntity(CEntityInstance* pEntity) override;
};

/*
 * The game's CTraceFilterKnifeIgnoreTeammates / CTraceFilterTaserIgnoreTeammates
 * (the same rule): everything but the pass entity and its teammates. As in
 * the game, nothing is hit once the pass entity is gone.
 */
class CTraceFilterIgnoreTeammates : public CTraceFilterBase
{
public:
	explicit CTraceFilterIgnoreTeammates(CBaseEntity* pPassEntity, uint64 nInteractsWith = MASK_SOLID, int nCollisionGroup = COLLISION_GROUP_DEFAULT);

	bool ShouldHitEntity(CEntityInstance* pEntity) override;
};

// The knife's swing / stab trace (MASK_SOLID, contacts generated).
class CTraceFilterKnifeIgnoreTeammates : public CTraceFilterIgnoreTeammates
{
public:
	explicit CTraceFilterKnifeIgnoreTeammates(CBaseEntity* pPassEntity);
};

// The taser's shot (MASK_SHOT, contacts generated).
class CTraceFilterTaserIgnoreTeammates : public CTraceFilterIgnoreTeammates
{
public:
	explicit CTraceFilterTaserIgnoreTeammates(CBaseEntity* pPassEntity);
};

/*
 * The game's CTraceFilterSkipPlayersAndNPCs: player pawns and NPCs never
 * block. The game's NPC flag has no schema name; CS2's NPCs are its
 * hostages, so that is what is skipped.
 */
class CTraceFilterSkipPlayersAndNPCs : public CTraceFilterBase
{
public:
	explicit CTraceFilterSkipPlayersAndNPCs(CBaseEntity* pPassEntity = nullptr, uint64 nInteractsWith = MASK_SOLID, int nCollisionGroup = COLLISION_GROUP_DEFAULT);

	bool ShouldHitEntity(CEntityInstance* pEntity) override;
};

// CTraceFilterSkipPlayersAndNPCsAndChildren: also nothing attached to them
// (weapons, wearables -- anything with one of them up its move-parent chain).
class CTraceFilterSkipPlayersAndNPCsAndChildren : public CTraceFilterSkipPlayersAndNPCs
{
public:
	using CTraceFilterSkipPlayersAndNPCs::CTraceFilterSkipPlayersAndNPCs;

	bool ShouldHitEntity(CEntityInstance* pEntity) override;
};

/*
 * The game's CTraceFilterIngnoreGrenades (sic, the game's spelling): grenade
 * projectiles never block (molotov ground checks).
 */
class CTraceFilterIgnoreGrenades : public CTraceFilterBase
{
public:
	explicit CTraceFilterIgnoreGrenades(CBaseEntity* pPassEntity = nullptr, uint64 nInteractsWith = MASK_SOLID, int nCollisionGroup = COLLISION_GROUP_DEFAULT);

	bool ShouldHitEntity(CEntityInstance* pEntity) override;
};

/*
 * The game's CTraceFilterHitEntitiesOnly: the static world is left out
 * (object set without RNQUERY_OBJECTS_STATIC) and every entity on the ray
 * is passed to ShouldHitEntity -- the base for "enumerate the entities along
 * a ray". Return false to let the ray go on through an entity.
 */
class CTraceFilterHitEntitiesOnly : public CTraceFilterBase
{
public:
	explicit CTraceFilterHitEntitiesOnly(uint64 nInteractsWith = MASK_SOLID_BRUSHONLY, int nCollisionGroup = COLLISION_GROUP_CONDITIONALLY_SOLID);
};

/*
 * The game's CTraceFilterNoPlayersAndFlashbangPassableAnims, a flashbang's
 * view of a player: weapons, grenades and hostages never block, players are
 * left out by layer (exclude PLAYER | DEBRIS).
 */
class CTraceFilterNoPlayersAndFlashbangPassableAnims : public CTraceFilterBase
{
public:
	CTraceFilterNoPlayersAndFlashbangPassableAnims(CBaseEntity* pPassEntity1, CBaseEntity* pPassEntity2, uint64 nInteractsWith = MASK_SOLID);

	bool ShouldHitEntity(CEntityInstance* pEntity) override;
};

#endif // TRACEFILTER_H
