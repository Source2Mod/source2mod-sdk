//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: CTakeDamageResult
//
//=============================================================================

#ifndef TAKEDAMAGERESULT_H
#define TAKEDAMAGERESULT_H
#ifdef _WIN32
#pragma once
#endif

#include "source2mod/sdk/schemasystem_helper.h"

#include <ehandle.h>
#include <gametrace.h>
#include <shareddefs.h>
#include <tier0/mempool.h>

#include <cstdint>

class CTakeDamageInfo;

/*
 * The fields are the schema's, the object is the memory in m_Storage (see
 * CTakeDamageInfo). Built with the game's default constructor (the class
 * ops), then the damage set as before; the game's destructor runs when it
 * goes out of scope.
 */
class CTakeDamageResult
{
	// Not an entity: a networked write would take the embedded route.
	DECLARE_SCHEMA_CLASS_INLINE(CTakeDamageResult);

public:
	SCHEMA_FIELD(CTakeDamageInfo*, m_pOriginatingInfo)
	SCHEMA_FIELD(CUtlLeanVector<int>, m_DestructibleHitGroupRequests) // CUtlLeanVector<DestructiblePartDamageRequest_t>
	SCHEMA_FIELD(int32_t, m_nHealthLost)
	SCHEMA_FIELD(int32_t, m_nHealthBefore)
	SCHEMA_FIELD(float, m_flDamageDealt)
	SCHEMA_FIELD(float, m_flPreModifiedDamage)
	SCHEMA_FIELD(Vector, m_vDamagePosition)
	SCHEMA_FIELD(int32_t, m_nTotalledHealthLost)
	SCHEMA_FIELD(float, m_flTotalledDamageDealt)
	SCHEMA_FIELD(float, m_flTotalledPreModifiedDamage)
	SCHEMA_FIELD(float, m_flNewDamageAccumulatorValue)
	SCHEMA_FIELD(TakeDamageFlags_t, m_nDamageFlags)
	SCHEMA_FIELD(bool, m_bWasDamageSuppressed)
	SCHEMA_FIELD(bool, m_bSuppressFlinch)
	SCHEMA_FIELD(HitGroup_t, m_nOverrideFlinchHitGroup)

	CTakeDamageResult() = delete;
	// The result of `damage` that nothing reduced.
	explicit CTakeDamageResult(float damage);
	// The same for pInfo's m_flDamage, with pInfo as m_pOriginatingInfo: the
	// result to hand the game together with that info.
	explicit CTakeDamageResult(CTakeDamageInfo* pInfo);
	~CTakeDamageResult();

	// Owns m_DestructibleHitGroupRequests' memory, see CTakeDamageInfo.
	CTakeDamageResult(const CTakeDamageResult&) = delete;
	CTakeDamageResult& operator=(const CTakeDamageResult&) = delete;

private:
	void Init(CTakeDamageInfo* pInfo, float damage);

	// The game's object: 96 bytes on 2026-10-23, room left for it to grow.
	alignas(16) uint8_t m_Storage[0x100];
};

#endif // TAKEDAMAGERESULT_H
