/**
 * Source2Mod SourcePawn API: entity flags, move types, solidity, collision
 * groups, effects and render modes.
 */

#if defined _source2mod_consts_entity_included
 #endinput
#endif
#define _source2mod_consts_entity_included

// Entity flags (const.h Flags_t): CBaseEntity.m_fFlags.
#define FL_ONGROUND                 (1 << 0)
#define FL_DUCKING                  (1 << 1)
#define FL_WATERJUMP                (1 << 2)
#define FL_BOT                      (1 << 4)
#define FL_FROZEN                   (1 << 5)
#define FL_ATCONTROLS               (1 << 6)
#define FL_CLIENT                   (1 << 7)
#define FL_FAKECLIENT               (1 << 8)
#define FL_FLY                      (1 << 10)
#define FL_SUPPRESS_SAVE            (1 << 11)
#define FL_IN_VEHICLE               (1 << 12)
#define FL_IN_VEHICLE_TRANSPORT     (1 << 13)
#define FL_GODMODE                  (1 << 14)
#define FL_NOTARGET                 (1 << 15)
#define FL_AIMTARGET                (1 << 16)
#define FL_STATICPROP               (1 << 18)
#define FL_GRENADE                  (1 << 20)
#define FL_DONTTOUCH                (1 << 22)
#define FL_BASEVELOCITY             (1 << 23)
#define FL_CONVEYOR                 (1 << 24)
#define FL_OBJECT                   (1 << 25)
#define FL_ONFIRE                   (1 << 27)
#define FL_DISSOLVING               (1 << 28)
#define FL_TRANSRAGDOLL             (1 << 29)
#define FL_UNBLOCKABLE_BY_PLAYER    (1 << 30)

// Move types (const.h MoveType_t): m_MoveType, SetMoveType().
#define MOVETYPE_NONE           0
#define MOVETYPE_OBSOLETE       1
#define MOVETYPE_WALK           2
#define MOVETYPE_FLY            3
#define MOVETYPE_FLYGRAVITY     4
#define MOVETYPE_VPHYSICS       5
#define MOVETYPE_PUSH           6
#define MOVETYPE_NOCLIP         7
#define MOVETYPE_OBSERVER       8
#define MOVETYPE_LADDER         9
#define MOVETYPE_CUSTOM         10

// Move collide (const.h MoveCollide_t): m_MoveCollide.
#define MOVECOLLIDE_DEFAULT     0
#define MOVECOLLIDE_FLY_BOUNCE  1
#define MOVECOLLIDE_FLY_CUSTOM  2
#define MOVECOLLIDE_FLY_SLIDE   3

// Solid types (const.h SolidType_t): CCollisionProperty.m_nSolidType.
#define SOLID_NONE          0
#define SOLID_BSP           1
#define SOLID_BBOX          2
#define SOLID_OBB           3
#define SOLID_SPHERE        4
#define SOLID_POINT         5
#define SOLID_VPHYSICS      6
#define SOLID_CAPSULE       7
#define SOLID_CYLINDER      8

// Solid flags (const.h SolidFlags_t): CCollisionProperty.m_usSolidFlags.
#define FSOLID_NOT_STANDABLE        0x0001
#define FSOLID_USE_TRIGGER_BOUNDS   0x0002
#define FSOLID_NOT_SOLID            0x0004

// Collision groups (const.h BuiltInCollisionGroup_t + StandardCollisionGroups_t):
// m_CollisionGroup, SetCollisionGroup().
#define COLLISION_GROUP_ALWAYS                  0
#define COLLISION_GROUP_NONPHYSICAL             1
#define COLLISION_GROUP_TRIGGER                 2
#define COLLISION_GROUP_CONDITIONALLY_SOLID     3
#define COLLISION_GROUP_DEFAULT                 4
#define COLLISION_GROUP_DEBRIS                  5
#define COLLISION_GROUP_INTERACTIVE_DEBRIS      6
#define COLLISION_GROUP_INTERACTIVE             7
#define COLLISION_GROUP_PLAYER                  8
#define COLLISION_GROUP_BREAKABLE_GLASS         9
#define COLLISION_GROUP_VEHICLE                 10
#define COLLISION_GROUP_PLAYER_MOVEMENT         11
#define COLLISION_GROUP_NPC                     12
#define COLLISION_GROUP_IN_VEHICLE              13
#define COLLISION_GROUP_WEAPON                  14
#define COLLISION_GROUP_VEHICLE_CLIP            15
#define COLLISION_GROUP_PROJECTILE              16
#define COLLISION_GROUP_DOOR_BLOCKER            17
#define COLLISION_GROUP_PASSABLE_DOOR           18
#define COLLISION_GROUP_DISSOLVING              19
#define COLLISION_GROUP_PUSHAWAY                20
#define COLLISION_GROUP_NPC_ACTOR               21
#define COLLISION_GROUP_NPC_SCRIPTED            22
#define COLLISION_GROUP_PZ_CLIP                 23
#define COLLISION_GROUP_PROPS                   24

// Collision functions (gametrace.h CollisionFunctionMask_t):
// ShapeCollisionFunctionMask().
#define FCOLLISION_FUNC_ENABLE_SOLID_CONTACT    (1 << 0)
#define FCOLLISION_FUNC_ENABLE_TRACE_QUERY      (1 << 1)
#define FCOLLISION_FUNC_ENABLE_TOUCH_EVENT      (1 << 2)
#define FCOLLISION_FUNC_ENABLE_SELF_COLLISIONS  (1 << 3)
#define FCOLLISION_FUNC_IGNORE_FOR_HITBOX_TEST  (1 << 4)
#define FCOLLISION_FUNC_ENABLE_TOUCH_PERSISTS   (1 << 5)
#define FCOLLISION_FUNC_DEFAULT                 (FCOLLISION_FUNC_ENABLE_SOLID_CONTACT | FCOLLISION_FUNC_ENABLE_TRACE_QUERY | FCOLLISION_FUNC_ENABLE_TOUCH_EVENT)

// Effects (const.h EntityEffects_t): m_fEffects.
#define EF_NOSHADOW             (1 << 4)
#define EF_NODRAW               (1 << 5)
#define EF_NORECEIVESHADOW      (1 << 6)
#define EF_PARENT_ANIMATES      (1 << 9)
#define EF_NODRAW_BUT_TRANSMIT  (1 << 10)

// Render modes (const.h RenderMode_t): CBaseModelEntity.m_nRenderMode.
#define kRenderNormal       0
#define kRenderTransAlpha   1
#define kRenderNone         2

// Render effects (const.h RenderFx_t): CBaseModelEntity.m_nRenderFX.
#define kRenderFxNone           0
#define kRenderFxPulseSlow      1
#define kRenderFxPulseFast      2
#define kRenderFxPulseSlowWide  3
#define kRenderFxPulseFastWide  4
#define kRenderFxFadeSlow       5
#define kRenderFxFadeFast       6
#define kRenderFxSolidSlow      7
#define kRenderFxSolidFast      8
#define kRenderFxStrobeSlow     9
#define kRenderFxStrobeFast     10
#define kRenderFxStrobeFaster   11
#define kRenderFxFlickerSlow    12
#define kRenderFxFlickerFast    13
#define kRenderFxFadeOut        14
#define kRenderFxFadeIn         15
