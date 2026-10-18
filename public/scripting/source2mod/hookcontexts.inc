/**
 * Source2Mod SourcePawn API: the contexts of the game-function hook points
 * (HookPoint_TakeDamage .. HookPoint_PlayerChat, source2mod/hooks;
 * C++ ISource2ModHookContexts.h).
 *
 * Game functions the core hooks from its own gamedata. A handler gets the
 * call's context -- valid inside the handler -- and post, and answers an
 * action; for a function with a return value, `result` is what the caller
 * gets when the handler answers Action_Override or Action_Supersede (in post
 * it holds what the original returned). CallOriginal() (pre) runs the game's
 * function now, past every hook, and returns its value; answer
 * Action_Supersede after it, or it runs twice.
 *
 * What the game passes by pointer (the damage info, the move data, a vector)
 * can be changed in place and the original sees the change; what it passes
 * by value (a speed, a flag) is read-only, as in C++.
 *
 * Objects that are not entities (the services, the jump component, the
 * econ item, the user command) are SchemaObjects; CUserCmd has no schema
 * class, its SchemaObject is opaque (for Source2ModFunction calls).
 */

#if defined _source2mod_hookcontexts_included
 #endinput
#endif
#define _source2mod_hookcontexts_included

#include <source2mod/types>
#include <source2mod/schema>
#include <source2mod/trace>

/** What CCSPlayer_ItemServices::CanAcquire answers. */
enum AcquireResult
{
	AcquireResult_Allowed = 0,
	AcquireResult_InvalidItem,
	AcquireResult_AlreadyOwned,
	AcquireResult_AlreadyPurchased,
	AcquireResult_ReachedGrenadeTypeLimit,
	AcquireResult_ReachedGrenadeTotalLimit,
	AcquireResult_NotAllowedByTeam,
	AcquireResult_NotAllowedByMap,
	AcquireResult_NotAllowedByMode,
	AcquireResult_NotAllowedForPurchase,
	AcquireResult_NotAllowedByProhibition
};

/** How an item is being acquired in CCSPlayer_ItemServices::CanAcquire. */
enum AcquireMethod
{
	AcquireMethod_PickUp = 0,
	AcquireMethod_Buy,
	AcquireMethod_BuyWithCtrl
};

/** The damage being dealt (CTakeDamageInfo); valid inside the hook. */
methodmap CTakeDamageInfo
{
	property float m_flDamage
	{
		public native get();
		public native set(float value);
	}
	property float m_flOriginalDamage
	{
		public native get();
	}
	property float m_flTotalledDamage
	{
		public native get();
		public native set(float value);
	}
	property int m_bitsDamageType
	{
		public native get();
		public native set(int value);
	}
	property int m_iDamageCustom
	{
		public native get();
		public native set(int value);
	}
	property int m_iAmmoType
	{
		public native get();
		public native set(int value);
	}
	property bool m_bShouldBleed
	{
		public native get();
		public native set(bool value);
	}
	property bool m_bShouldSpark
	{
		public native get();
		public native set(bool value);
	}
	property bool m_bStoppedBullet
	{
		public native get();
		public native set(bool value);
	}
	property int m_nNumObjectsPenetrated
	{
		public native get();
		public native set(int value);
	}
	property float m_flFriendlyFireDamageReductionRatio
	{
		public native get();
		public native set(float value);
	}
	property bool m_bInTakeDamageFlow
	{
		public native get();
	}
	property CBaseEntity m_hInflictor
	{
		public native get();
		public native set(CBaseEntity entity);
	}
	property CBaseEntity m_hAttacker
	{
		public native get();
		public native set(CBaseEntity entity);
	}
	property CBaseEntity m_hAbility
	{
		public native get();
		public native set(CBaseEntity entity);
	}
	/** TakeDamageFlags_t (64-bit) as two cells: flags[0] the low 32 bits. */
	public native void m_nDamageFlags(int flags[2]);
	public native void Set_m_nDamageFlags(const int flags[2]);
	public native void m_vecDamageForce(float vec[3]);
	public native void Set_m_vecDamageForce(const float vec[3]);
	public native void m_vecDamagePosition(float vec[3]);
	public native void Set_m_vecDamagePosition(const float vec[3]);
	public native void m_vecReportedPosition(float vec[3]);
	public native void Set_m_vecReportedPosition(const float vec[3]);
	public native void m_vecDamageDirection(float vec[3]);
	public native void Set_m_vecDamageDirection(const float vec[3]);
	/**
	 * Where the hit landed (HITGROUP_*, CTakeDamageInfo::GetHitGroup()), read
	 * from the damage's bullet result. The struct's own m_iHitGroupId is not
	 * filled in by the game, so there is no property for it.
	 */
	public native int GetHitGroup();
	/** The trace of the hit (m_pTrace), a copy; null when the damage had none. Close it. */
	public native TraceResult GetTrace();
	/**
	 * m_AttackerInfo: whether the attacker is a pawn / the world, its player
	 * slot and team.
	 * @return The attacker's pawn, NULL_ENTITY when none
	 */
	public native CCSPlayerPawn GetAttackerInfo(bool &isPawn, bool &isWorld, int &playerSlot, int &team);
}

/** What TakeDamage did (CTakeDamageResult); valid inside the hook, mainly in post. */
methodmap CTakeDamageResult
{
	property int m_nHealthLost
	{
		public native get();
		public native set(int value);
	}
	property int m_nHealthBefore
	{
		public native get();
	}
	property float m_flDamageDealt
	{
		public native get();
		public native set(float value);
	}
	property float m_flPreModifiedDamage
	{
		public native get();
	}
	property int m_nTotalledHealthLost
	{
		public native get();
		public native set(int value);
	}
	property float m_flTotalledDamageDealt
	{
		public native get();
		public native set(float value);
	}
	property float m_flTotalledPreModifiedDamage
	{
		public native get();
	}
	property float m_flNewDamageAccumulatorValue
	{
		public native get();
	}
	property bool m_bWasDamageSuppressed
	{
		public native get();
		public native set(bool value);
	}
	property bool m_bSuppressFlinch
	{
		public native get();
		public native set(bool value);
	}
	property int m_nOverrideFlinchHitGroup
	{
		public native get();
		public native set(int value);
	}
	/** TakeDamageFlags_t (64-bit) as two cells. */
	public native void m_nDamageFlags(int flags[2]);
	/** The info that caused it (m_pOriginatingInfo); null when unset. */
	public native CTakeDamageInfo GetOriginatingInfo();
}

/**
 * The movement data (CMoveData) a movement function works on; valid inside
 * the hook. Vectors and angles are methods filling a buffer, Set_<name>()
 * writes them.
 */
methodmap CMoveData
{
	/** The pawn moving (m_nPlayerHandle); NULL_ENTITY when unset. */
	property CCSPlayerPawn m_nPlayerHandle
	{
		public native get();
	}
	property float m_flForwardMove
	{
		public native get();
		public native set(float value);
	}
	/** Moving right is negative (flipped compared to CS:GO). */
	property float m_flSideMove
	{
		public native get();
		public native set(float value);
	}
	property float m_flUpMove
	{
		public native get();
		public native set(float value);
	}
	property bool m_bHasSubtickInputs
	{
		public native get();
		public native set(bool value);
	}
	property int m_nTickCount
	{
		public native get();
	}
	property int m_nTargetTick
	{
		public native get();
	}
	property float m_flSubtickStartFraction
	{
		public native get();
		public native set(float value);
	}
	property float m_flSubtickEndFraction
	{
		public native get();
		public native set(float value);
	}
	property float m_flMaxSpeed
	{
		public native get();
		public native set(float value);
	}
	property float m_flClientMaxSpeed
	{
		public native get();
		public native set(float value);
	}
	property float m_flFrictionDecel
	{
		public native get();
		public native set(float value);
	}
	property bool m_bInAir
	{
		public native get();
		public native set(bool value);
	}
	property bool m_bGameCodeMovedPlayer
	{
		public native get();
		public native set(bool value);
	}
	public native bool HasZeroFrametime();
	public native bool IsLateCommand();
	public native void SetHasZeroFrametime(bool value);
	public native void SetIsLateCommand(bool value);

	public native void m_vecAbsViewAngles(float vec[3]);
	public native void Set_m_vecAbsViewAngles(const float vec[3]);
	public native void m_vecViewAngles(float vec[3]);
	public native void Set_m_vecViewAngles(const float vec[3]);
	public native void m_vecLastMovementImpulses(float vec[3]);
	public native void Set_m_vecLastMovementImpulses(const float vec[3]);
	public native void m_vecVelocity(float vec[3]);
	public native void Set_m_vecVelocity(const float vec[3]);
	public native void m_vecAngles(float vec[3]);
	public native void Set_m_vecAngles(const float vec[3]);
	public native void m_vecCollisionNormal(float vec[3]);
	public native void Set_m_vecCollisionNormal(const float vec[3]);
	public native void m_vecGroundNormal(float vec[3]);
	public native void Set_m_vecGroundNormal(const float vec[3]);
	public native void m_vecAbsOrigin(float vec[3]);
	public native void Set_m_vecAbsOrigin(const float vec[3]);
	public native void m_vecOutWishVel(float vec[3]);
	public native void Set_m_vecOutWishVel(const float vec[3]);
	public native void m_vecOldAngles(float vec[3]);
	public native void Set_m_vecOldAngles(const float vec[3]);
	public native void m_vecAcceleration(float vec[3]);
	public native void Set_m_vecAcceleration(const float vec[3]);
	public native void m_vecContinuousAcceleration(float vec[3]);
	public native void Set_m_vecContinuousAcceleration(const float vec[3]);
	/** Vector2D: vec[2] is set to 0. */
	public native void m_vecWalkWishedVelocity(float vec[3]);
	public native void Set_m_vecWalkWishedVelocity(const float vec[3]);

	/** m_TouchList: what the move touched. */
	public native int GetTouchCount();
	/** One touch: its velocity change, and a copy of its trace (close it). */
	public native TraceResult GetTouch(int index, float deltaVelocity[3]);

	/** m_SubtickMoves (or m_AttackSubtickMoves with attack = true): how many. */
	public native int GetSubtickMoveCount(bool attack = false);
	/**
	 * One subtick move: its button mask (two cells, low 32 bits first) and,
	 * for an analog move (button 0), the deltas forward, left, pitch, yaw.
	 * @return when
	 */
	public native float GetSubtickMove(int index, int button[2], bool &pressed, float analog[4], bool attack = false);
}

/** CBaseEntity::TakeDamageOld(info, result) -> int64 */
methodmap TakeDamageContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	property CTakeDamageInfo info
	{
		public native get();
	}
	/** The damage result being filled; null when the game passed none. */
	property CTakeDamageResult damageResult
	{
		public native get();
	}
	/** Runs the game's TakeDamage now (pre only); returns its result. */
	public native int CallOriginal();
}

/** CCSPlayer_ItemServices::CanAcquire(item, method, unk) -> AcquireResult */
methodmap CanAcquireContext
{
	property CCSPlayer_ItemServices services
	{
		public native get();
	}
	/** The pawn the services belong to. */
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CEconItemView item
	{
		public native get();
	}
	property AcquireMethod method
	{
		public native get();
	}
	public native AcquireResult CallOriginal();
}

/** CCSPlayerPawnBase::CanMove() -> bool; false while frozen, defusing, ... */
methodmap CanMoveContext
{
	property CCSPlayerPawnBase pawn
	{
		public native get();
	}
	public native bool CallOriginal();
}

/** CCSPlayer_WeaponServices::CanUse(weapon) -> bool */
methodmap CanUseContext
{
	property CCSPlayer_WeaponServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CBasePlayerWeapon weapon
	{
		public native get();
	}
	public native bool CallOriginal();
}

/** CCSPlayerPawn::PostThink() */
methodmap PostThinkContext
{
	property CCSPlayerPawnBase pawn
	{
		public native get();
	}
	public native void CallOriginal();
}

/**
 * CCSPlayerController::ProcessUserCmd(cmds, count, paused, margin) -> void*.
 * The commands are CUserCmd (protobuf-backed, no schema class): opaque.
 */
methodmap ProcessUsercmdsContext
{
	property CCSPlayerController controller
	{
		public native get();
	}
	/** The command array, opaque. */
	property SchemaObject cmds
	{
		public native get();
	}
	property int count
	{
		public native get();
	}
	property bool paused
	{
		public native get();
	}
	property float margin
	{
		public native get();
	}
	/** Runs the original; what it returns is opaque, so only whether it was null. */
	public native bool CallOriginal();
}

/** CBasePlayerController::OnSimulateUserCommands() */
methodmap SimulateUserCommandsContext
{
	property CBasePlayerController controller
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CPlayer_MovementServices::RunCommand(cmd); the command is opaque (CUserCmd). */
methodmap RunCommandContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property SchemaObject cmd
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CEntityIdentity::AcceptInput(name, activator, caller, value, ...) -> bool */
methodmap AcceptInputContext
{
	/** The entity receiving the input. */
	property CEntityInstance entity
	{
		public native get();
	}
	property CEntityInstance activator
	{
		public native get();
	}
	property CEntityInstance caller
	{
		public native get();
	}
	public native int GetInputName(char[] buffer, int maxlen);
	/** The value (variant_t) as text; "" when there is none. */
	public native int GetValue(char[] buffer, int maxlen);
	/** Replaces the value with a string; the original (and later handlers) see it. */
	public native bool SetValue(const char[] value);
	public native bool CallOriginal();
}

/** CBaseEntity::Touch(other), on CBaseEntity's own vtable. */
methodmap TouchContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	property CBaseEntity other
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CCSPlayer_WeaponServices::DropWeapon(weapon, target, velocity) */
methodmap DropWeaponContext
{
	property CCSPlayer_WeaponServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CBasePlayerWeapon weapon
	{
		public native get();
	}
	/** False when the game passed no target / velocity (the getters then leave the buffer). */
	public native bool GetTarget(float vec[3]);
	public native bool SetTarget(const float vec[3]);
	public native bool GetVelocity(float vec[3]);
	public native bool SetVelocity(const float vec[3]);
	public native void CallOriginal();
}

/**
 * The CCSPlayer_MovementServices functions that take the move data alone:
 * AirMove, CheckFalling, CheckParameters, Duck, Friction, PlayerMove,
 * ProcessMovement, WalkMove, WaterMove (void: `result` is unused), and
 * CanUnduck, CheckWater, LadderMove, MoveInit (bool).
 */
methodmap MovementContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	/** The void functions answer false. */
	public native bool CallOriginal();
}

/** CCSPlayer_MovementServices::AirAccelerate(move, wishDirection, wishSpeed, acceleration) */
methodmap AirAccelerateContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	property float wishSpeed
	{
		public native get();
	}
	property float acceleration
	{
		public native get();
	}
	public native void GetWishDirection(float vec[3]);
	public native void SetWishDirection(const float vec[3]);
	public native void CallOriginal();
}

/** CCSPlayer_MovementServices::GroundAccelerate(move, wishDirection, frameTime, wishSpeed, acceleration) */
methodmap GroundAccelerateContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	property float frameTime
	{
		public native get();
	}
	property float wishSpeed
	{
		public native get();
	}
	property float acceleration
	{
		public native get();
	}
	public native void GetWishDirection(float vec[3]);
	public native void SetWishDirection(const float vec[3]);
	public native void CallOriginal();
}

/** CCSPlayer_MovementServices::CategorizePosition(move, stayOnGround) */
methodmap CategorizePositionContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	property bool stayOnGround
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CCSPlayer_MovementServices::CheckVelocity(move, unk); the second argument is unknown and not exposed. */
methodmap CheckVelocityContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CCSPlayer_MovementServices::FullWalkMove(move, onGround) */
methodmap FullWalkMoveContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	property bool onGround
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CCSPlayer_MovementServices::SetupMove(cmd, move); the command is opaque (CUserCmd). */
methodmap SetupMoveContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property SchemaObject cmd
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CCSPlayer_MovementServices::TryPlayerMove(move, firstDest, firstTrace, isSurfing) */
methodmap TryPlayerMoveContext
{
	property CCSPlayer_MovementServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	/** False when the game passed no first destination. */
	public native bool GetFirstDest(float vec[3]);
	public native bool SetFirstDest(const float vec[3]);
	/** A copy of the first trace; null when the game passed none. Close it. */
	public native TraceResult GetFirstTrace();
	/** *isSurfing; false when the game passed no pointer. */
	public native bool GetIsSurfing();
	public native bool SetIsSurfing(bool value);
	public native void CallOriginal();
}

/** CCSPlayerLegacyJump::OnJump / CheckJumpButton(move) */
methodmap LegacyJumpContext
{
	property CCSPlayerLegacyJump jump
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CCSPlayerModernJump::OnJump / CheckJumpButton(move) */
methodmap ModernJumpContext
{
	property CCSPlayerModernJump jump
	{
		public native get();
	}
	property CMoveData moveData
	{
		public native get();
	}
	public native void CallOriginal();
}

/**
 * Host_Say: a say / say_team line the game is about to print and send.
 * Lines a silent chat trigger hid for a Source2Mod command never get here;
 * "!command" lines do. Pre: Action_Supersede drops the line (no chat, no
 * player_say). Source2Mod fires no player_chat game event -- this is the
 * way to see chat.
 */
methodmap PlayerChatContext
{
	/** Who said it; NULL_ENTITY for the server console. */
	property CCSPlayerController controller
	{
		public native get();
	}
	/** say_team. */
	property bool teamOnly
	{
		public native get();
	}
	/** The line as typed (the say command's first argument). */
	public native int GetMessage(char[] buffer, int maxlen);
	public native void CallOriginal();
}

/* ---- entity virtuals (SourceMod's SDKHooks) ----------------------------------------
 * Spawn, Use, StartTouch, EndTouch, Blocked, Think and Killed are overridden by most
 * entity classes: hook them on one entity (Source2ModHooks.HookInstance, SDKHook) or
 * on a class (HookClass, "CCSPlayerPawn"); Hook (every call) is refused.
 * WeaponPickup and WeaponSwitch take Hook too. */

/** CBaseEntity::Spawn(keyValues) */
methodmap SpawnContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CBaseEntity::Use(info): a player's +use, a map's Use input. */
methodmap UseContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	property CBaseEntity activator
	{
		public native get();
	}
	property CBaseEntity caller
	{
		public native get();
	}
	/** 0 off, 1 on, 2 set, 3 toggle. */
	property int useType
	{
		public native get();
		public native set(int value);
	}
	property float value
	{
		public native get();
		public native set(float value);
	}
	public native void CallOriginal();
}

/** CBaseEntity::StartTouch / EndTouch / Blocked(other) */
methodmap EntityOtherContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	property CBaseEntity other
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CBaseEntity::Think(): the entity's think function, each time it runs. */
methodmap ThinkContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	public native void CallOriginal();
}

/** CBaseEntity::Event_Killed(damage) -> bool: health reached 0, m_lifeState already "dying". Supersede skips the death handling. */
methodmap KilledContext
{
	property CBaseEntity entity
	{
		public native get();
	}
	property CTakeDamageResult damage
	{
		public native get();
	}
	public native bool CallOriginal();
}

/** CCSPlayer_WeaponServices::PickupWeapon(weapon) -> 0 refused, 1 taken, 2 taken as ammo. */
methodmap WeaponPickupContext
{
	property CCSPlayer_WeaponServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CBasePlayerWeapon weapon
	{
		public native get();
	}
	public native int CallOriginal();
}

/** CCSPlayer_WeaponServices::SwitchToWeapon(weapon, viewModelIndex) -> bool */
methodmap WeaponSwitchContext
{
	property CCSPlayer_WeaponServices services
	{
		public native get();
	}
	property CCSPlayerPawn pawn
	{
		public native get();
	}
	property CBasePlayerWeapon weapon
	{
		public native get();
	}
	property int viewModelIndex
	{
		public native get();
	}
	public native bool CallOriginal();
}

// The handler of each point (Source2ModHookHandler in source2mod/hooks lists them all).
typedef Source2ModTakeDamageHandler = function Source2ModAction (TakeDamageContext ctx, bool post, int &result);
typedef Source2ModCanAcquireHandler = function Source2ModAction (CanAcquireContext ctx, bool post, AcquireResult &result);
typedef Source2ModCanMoveHandler = function Source2ModAction (CanMoveContext ctx, bool post, bool &result);
typedef Source2ModCanUseHandler = function Source2ModAction (CanUseContext ctx, bool post, bool &result);
typedef Source2ModPostThinkHandler = function Source2ModAction (PostThinkContext ctx, bool post);
/** ProcessUserCmd's return value is opaque: Override / Supersede keep ctx.result (null in pre, the original's in post). */
typedef Source2ModProcessUsercmdsHandler = function Source2ModAction (ProcessUsercmdsContext ctx, bool post);
typedef Source2ModSimulateUserCommandsHandler = function Source2ModAction (SimulateUserCommandsContext ctx, bool post);
typedef Source2ModRunCommandHandler = function Source2ModAction (RunCommandContext ctx, bool post);
typedef Source2ModAcceptInputHandler = function Source2ModAction (AcceptInputContext ctx, bool post, bool &result);
typedef Source2ModTouchHandler = function Source2ModAction (TouchContext ctx, bool post);
typedef Source2ModDropWeaponHandler = function Source2ModAction (DropWeaponContext ctx, bool post);
/** `result` matters for CanUnduck, CheckWater, LadderMove and MoveInit only. */
typedef Source2ModMovementHandler = function Source2ModAction (MovementContext ctx, bool post, bool &result);
typedef Source2ModAirAccelerateHandler = function Source2ModAction (AirAccelerateContext ctx, bool post);
typedef Source2ModGroundAccelerateHandler = function Source2ModAction (GroundAccelerateContext ctx, bool post);
typedef Source2ModCategorizePositionHandler = function Source2ModAction (CategorizePositionContext ctx, bool post);
typedef Source2ModCheckVelocityHandler = function Source2ModAction (CheckVelocityContext ctx, bool post);
typedef Source2ModFullWalkMoveHandler = function Source2ModAction (FullWalkMoveContext ctx, bool post);
typedef Source2ModSetupMoveHandler = function Source2ModAction (SetupMoveContext ctx, bool post);
typedef Source2ModTryPlayerMoveHandler = function Source2ModAction (TryPlayerMoveContext ctx, bool post);
typedef Source2ModLegacyJumpHandler = function Source2ModAction (LegacyJumpContext ctx, bool post);
typedef Source2ModModernJumpHandler = function Source2ModAction (ModernJumpContext ctx, bool post);
typedef Source2ModPlayerChatHandler = function Source2ModAction (PlayerChatContext ctx, bool post);
typedef Source2ModSpawnHandler = function Source2ModAction (SpawnContext ctx, bool post);
typedef Source2ModUseHandler = function Source2ModAction (UseContext ctx, bool post);
/** StartTouch, EndTouch, Blocked. */
typedef Source2ModEntityOtherHandler = function Source2ModAction (EntityOtherContext ctx, bool post);
typedef Source2ModThinkHandler = function Source2ModAction (ThinkContext ctx, bool post);
typedef Source2ModKilledHandler = function Source2ModAction (KilledContext ctx, bool post, bool &result);
typedef Source2ModWeaponPickupHandler = function Source2ModAction (WeaponPickupContext ctx, bool post, int &result);
typedef Source2ModWeaponSwitchHandler = function Source2ModAction (WeaponSwitchContext ctx, bool post, bool &result);
