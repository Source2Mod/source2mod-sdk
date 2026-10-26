//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: CTakeDamageInfo
//
//=============================================================================

#ifndef TAKEDAMAGEINFO_H
#define TAKEDAMAGEINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "attackerinfo.h"
#include "source2mod/sdk/schemasystem_helper.h"

// DamageTypes_t comes from the game headers; the SDK no longer emits a scoped
// copy of it (see HARD_SKIP_ENUMS in tools/schemagen/generator.py).
#include "shareddefs.h"

#include "ehandle.h"
#include "gametrace.h"
#include "tier0/mempool.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

class CCSPlayerPawn;

/*
 * A member the game's class has but its schema does not list, at a fixed
 * offset (checked against the 2026-10-23 binaries). Like a SCHEMA_FIELD it
 * holds no data: it is a view onto the object at `offset`.
 */
#define DAMAGE_FIXED_FIELD(type, varName, offset)                                                       \
	class varName##_prop                                                                                \
	{                                                                                                   \
	public:                                                                                             \
		varName##_prop() = default;                                                                     \
		std::add_lvalue_reference_t<type> Get()                                                         \
		{                                                                                               \
			return *reinterpret_cast<std::add_pointer_t<type>>(reinterpret_cast<uintptr_t>(this) - offsetof(ThisClass, varName) + (offset)); \
		}                                                                                               \
		std::add_lvalue_reference_t<std::add_const_t<type>> Get() const                                 \
		{                                                                                               \
			return *reinterpret_cast<std::add_pointer_t<std::add_const_t<type>>>(reinterpret_cast<uintptr_t>(this) - offsetof(ThisClass, varName) + (offset)); \
		}                                                                                               \
		operator std::add_lvalue_reference_t<type>() { return Get(); }                                  \
		operator std::add_lvalue_reference_t<std::add_const_t<type>>() const { return Get(); }          \
		std::add_lvalue_reference_t<type> operator()() { return Get(); }                                \
		std::add_lvalue_reference_t<std::add_const_t<type>> operator()() const { return Get(); }        \
		std::add_pointer_t<type> operator&() { return &Get(); }                                         \
		varName##_prop& operator=(std::add_lvalue_reference_t<std::add_const_t<type>> val) { Get() = val; return *this; } \
	private:                                                                                            \
		varName##_prop(const varName##_prop&) = delete;                                                 \
	} varName;

/*
 * The fields are the schema's (offsets and size follow the running game); the
 * object is the memory in m_Storage, so one can still be built on the stack.
 * The default constructor is the game's own (the class ops), the one taking
 * the damage the game's CTakeDamageInfo constructor from gamedata -- it also
 * fills m_AttackerInfo from the attacker the way the game does. The game's
 * destructor runs when it goes out of scope.
 */
class CTakeDamageInfo
{
	// Not an entity: a networked write would take the embedded route.
	DECLARE_SCHEMA_CLASS_INLINE(CTakeDamageInfo);

public:
	CTakeDamageInfo();
	CTakeDamageInfo(CBaseEntity* pInflictor, CBaseEntity* pAttacker, CBaseEntity* pAbility, float flDamage, DamageTypes_t bitsDamageType);
	~CTakeDamageInfo();

	// The game's object owns allocations (m_DestructibleHitGroupRequests, the
	// script instance), so a byte copy would free them twice.
	CTakeDamageInfo(const CTakeDamageInfo&) = delete;
	CTakeDamageInfo& operator=(const CTakeDamageInfo&) = delete;

    /// Where the hit landed, read from the damage's bullet result. Use this,
    /// not m_iHitGroupId: the game does not fill that field in.
    HitGroup_t GetHitGroup() const;

	SCHEMA_FIELD(Vector, m_vecDamageForce)
	SCHEMA_FIELD(Vector, m_vecDamagePosition)
	SCHEMA_FIELD(Vector, m_vecReportedPosition)
	SCHEMA_FIELD(Vector, m_vecDamageDirection)
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hInflictor)
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hAttacker)
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hAbility)
	SCHEMA_FIELD(float, m_flDamage)
	SCHEMA_FIELD(float, m_flTotalledDamage)
	SCHEMA_FIELD(int32_t, m_bitsDamageType)
	SCHEMA_FIELD(int32_t, m_iDamageCustom)
	SCHEMA_FIELD(int8_t, m_iAmmoType)
	SCHEMA_FIELD(float, m_flOriginalDamage)
	SCHEMA_FIELD(bool, m_bShouldBleed)
	SCHEMA_FIELD(bool, m_bShouldSpark)
	SCHEMA_FIELD(TakeDamageFlags_t, m_nDamageFlags)
	SCHEMA_FIELD(int32_t, m_nNumObjectsPenetrated)
	SCHEMA_FIELD(float, m_flFriendlyFireDamageReductionRatio)
	SCHEMA_FIELD(bool, m_bStoppedBullet)
	SCHEMA_FIELD(CUtlLeanVector<int>, m_DestructibleHitGroupRequests) // CUtlLeanVector<DestructiblePartDamageRequest_t>
	SCHEMA_FIELD(bool, m_bInTakeDamageFlow)

	// Not in the schema. m_pTrace is also what GetHitGroup() reads
	// ("CTakeDamageInfo::HitGroup" in gamedata).
	DAMAGE_FIXED_FIELD(CGameTrace*, m_pTrace, 0x68)
	DAMAGE_FIXED_FIELD(void*, m_hScriptInstance, 0xe0)
	DAMAGE_FIXED_FIELD(AttackerInfo_t, m_AttackerInfo, 0xe8)

	// m_iHitGroupId (0x78) is left out on purpose: the game does not fill it
	// in, GetHitGroup() is the hit group.

private:
	// The game's object: 280 bytes on 2026-10-23, room left for it to grow.
	// The constructors warn once when the schema says it no longer fits.
	alignas(16) uint8_t m_Storage[0x200];
};

#endif // TAKEDAMAGEINFO_H
