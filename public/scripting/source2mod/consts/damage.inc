/**
 * Source2Mod SourcePawn API: damage types, take-damage flags and hit groups.
 */

#if defined _source2mod_consts_damage_included
 #endinput
#endif
#define _source2mod_consts_damage_included

// Damage types (shareddefs.h DamageTypes_t): CTakeDamageInfo.m_bitsDamageType,
// TakeDamage(..., damageType).
#define DMG_GENERIC         0
#define DMG_CRUSH           (1 << 0)
#define DMG_BULLET          (1 << 1)
#define DMG_SLASH           (1 << 2)
#define DMG_BURN            (1 << 3)
#define DMG_VEHICLE         (1 << 4)
#define DMG_FALL            (1 << 5)
#define DMG_BLAST           (1 << 6)
#define DMG_CLUB            (1 << 7)
#define DMG_SHOCK           (1 << 8)
#define DMG_SONIC           (1 << 9)
#define DMG_ENERGYBEAM      (1 << 10)
#define DMG_BUCKSHOT        (1 << 11)
#define DMG_BLAST_SURFACE   (1 << 12)
#define DMG_DISSOLVE        (1 << 13)
#define DMG_DROWN           (1 << 14)
#define DMG_POISON          (1 << 15)
#define DMG_RADIATION       (1 << 16)
#define DMG_DROWNRECOVER    (1 << 17)
#define DMG_ACID            (1 << 18)
#define DMG_HEADSHOT        (1 << 19)

// Take-damage flags (shareddefs.h TakeDamageFlags_t, 64-bit): bit numbers for
// CTakeDamageInfo.m_nDamageFlags (two cells), with Bits64_Has / Add / Remove.
#define DFLAG_SUPPRESS_HEALTH_CHANGES_BIT           0
#define DFLAG_SUPPRESS_PHYSICS_FORCE_BIT            1
#define DFLAG_SUPPRESS_EFFECTS_BIT                  2
#define DFLAG_PREVENT_DEATH_BIT                     3
#define DFLAG_FORCE_DEATH_BIT                       4
#define DFLAG_ALWAYS_GIB_BIT                        5
#define DFLAG_NEVER_GIB_BIT                         6
#define DFLAG_REMOVE_NO_RAGDOLL_BIT                 7
#define DFLAG_SUPPRESS_DAMAGE_MODIFICATION_BIT      8
#define DFLAG_ALWAYS_FIRE_DAMAGE_EVENTS_BIT         9
#define DFLAG_RADIUS_DMG_BIT                        10
#define DFLAG_FORCEREDUCEARMOR_DMG_BIT              11
#define DFLAG_SUPPRESS_INTERRUPT_FLINCH_BIT         12
#define DFLAG_IGNORE_DESTRUCTIBLE_PARTS_BIT         13
#define DFLAG_SUPPRESS_BREAKABLES_BIT               14
#define DFLAG_FORCE_PHYSICS_FORCE_BIT               15
#define DFLAG_SUPPRESS_SCREENSPACE_DAMAGE_FX_BIT    16
#define DFLAG_ALLOW_NON_AUTHORITATIVE_BIT           17
#define DFLAG_IGNORE_ARMOR_BIT                      18
#define DFLAG_SUPPRESS_UTILREMOVE_BIT               19

// m_takedamage modes (shareddefs.h).
#define DAMAGE_NO           0
#define DAMAGE_EVENTS_ONLY  1
#define DAMAGE_YES          2
#define DAMAGE_AIM          3

// Hit groups (gametrace.h HitGroup_t): CTakeDamageInfo.GetHitGroup(), traces.
#define HITGROUP_INVALID    -1
#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD       1
#define HITGROUP_CHEST      2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4
#define HITGROUP_RIGHTARM   5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG   7
#define HITGROUP_NECK       8
#define HITGROUP_UNUSED     9
#define HITGROUP_GEAR       10
#define HITGROUP_SPECIAL    11
