/**
 * Source2Mod SourcePawn API: hooks (ISource2ModHooks).
 *
 * One way to hook anything, the same as C++:
 *
 *     Hook = Point (what) + Target (on which object) + Handler (with its context)
 *
 * Points (HookPoint):
 *   - game functions with a typed context: TakeDamage, Touch, PostThink,
 *     AcceptInput, the movement functions, ... (source2mod/hookcontexts);
 *   - engine callbacks: GameFrame, StartupServer, ClientConnect, ...;
 *   - any function: HookFunction on a Source2ModFunction (Find on a gamedata
 *     "function" entry, FromSignature, FromSymbol, FromVirtualTable,
 *     FromVirtual, FromAddress) -- SourceMod's DHooks.
 *
 * Targets: Hook -- every call; HookInstance -- one object (an entity, or a
 * SchemaObject: services, the game rules, ...); HookClass -- every object of
 * a C++ class ("CCSPlayerPawn") or one derived from it. Engine callbacks
 * take Hook only.
 *
 * Filters (optional): a point whose object is an entity compares its
 * designer name ("logic_relay", "player"); AcceptInput also the input's name.
 *
 * Entity events and entity outputs are listeners of Source2ModEntities
 * (source2mod/entities), the way commands, convars and game events have
 * theirs: this is for functions.
 *
 * A handler answers Action_Ignore, Action_Override or Action_Supersede (pre:
 * the original does not run). Every Hook* returns an id for Unhook; what a
 * script still holds when it unloads goes with it, and a hook on one entity
 * goes with the entity.
 */

#if defined _source2mod_hooks_included
 #endinput
#endif
#define _source2mod_hooks_included

#include <source2mod/types>
#include <source2mod/schema>
#include <source2mod/gameconfig>
#include <source2mod/commands>
#include <source2mod/hookcontexts>

/** Every point; the numbers are C++'s HookPointId. */
enum HookPoint
{
	// Game functions (source2mod/hookcontexts).
	HookPoint_TakeDamage = 0,
	HookPoint_CanAcquire,
	HookPoint_CanMove,
	HookPoint_CanUse,
	HookPoint_PostThink,
	HookPoint_ProcessUsercmds,
	HookPoint_SimulateUserCommands,
	HookPoint_RunCommand,
	HookPoint_AcceptInput,
	HookPoint_Touch,
	HookPoint_DropWeapon,
	HookPoint_AirAccelerate,
	HookPoint_AirMove,
	HookPoint_CanUnduck,
	HookPoint_CategorizePosition,
	HookPoint_CheckFalling,
	HookPoint_CheckParameters,
	HookPoint_CheckVelocity,
	HookPoint_CheckWater,
	HookPoint_Duck,
	HookPoint_Friction,
	HookPoint_FullWalkMove,
	HookPoint_GroundAccelerate,
	HookPoint_LadderMove,
	HookPoint_MoveInit,
	HookPoint_PlayerMove,
	HookPoint_ProcessMovement,
	HookPoint_SetupMove,
	HookPoint_TryPlayerMove,
	HookPoint_WalkMove,
	HookPoint_WaterMove,
	HookPoint_OnJumpLegacy,
	HookPoint_OnJumpModern,
	HookPoint_CheckJumpButtonLegacy,
	HookPoint_CheckJumpButtonModern,
	HookPoint_PlayerChat,

	// Entity virtuals (SDKHooks): HookInstance(entity) / HookClass(name); not Hook.
	HookPoint_Spawn = 50,
	HookPoint_Use,
	HookPoint_StartTouch,
	HookPoint_EndTouch,
	HookPoint_Blocked,
	HookPoint_Think,
	HookPoint_Killed,
	HookPoint_WeaponPickup,             /**< Hook works: every player's weapon services */
	HookPoint_WeaponSwitch,             /**< Hook works: every player's weapon services */

	// Engine callbacks.
	HookPoint_GameFrame = 100,
	HookPoint_StartupServer,
	HookPoint_ClientConnect,            /**< pre may refuse: result = false, Action_Supersede */
	HookPoint_ClientConnected,
	HookPoint_ClientPutInServer,
	HookPoint_ClientDisconnect,         /**< pre: still in game; post: gone */
	HookPoint_ClientVoice,
	HookPoint_ClientSettingsChanged,
	HookPoint_GameServerSteamAPIActivated,
	HookPoint_GameServerSteamAPIDeactivated,
	HookPoint_LoadEventsFromFile,

	// Source2ModHooks.HookFunction.
	HookPoint_Function = 300
};

/* ---- engine callbacks ------------------------------------------------------------- */

/** ISource2Server::GameFrame -- every tick. */
methodmap GameFrameContext
{
	property bool simulating
	{
		public native get();
	}
	property bool firstTick
	{
		public native get();
	}
	property bool lastTick
	{
		public native get();
	}
}

/** INetworkServerService::StartupServer -- a new map's server is up. */
methodmap StartupServerContext
{
	public native int GetMapName(char[] buffer, int maxlen);
}

/**
 * ISource2GameClients::ClientConnect. Pre: set result = false and answer
 * Action_Supersede to turn the client away (SetRejectReason first). Not
 * called for bots.
 */
methodmap ClientConnectContext
{
	property CPlayerSlot slot
	{
		public native get();
	}
	public native int GetName(char[] buffer, int maxlen);
	/** The SteamID64 as decimal text. */
	public native int GetXuid(char[] buffer, int maxlen);
	public native int GetNetworkId(char[] buffer, int maxlen);
	public native void SetRejectReason(const char[] reason);
}

/** ISource2GameClients::OnClientConnected -- bots too (fakePlayer). */
methodmap ClientConnectedContext
{
	property CPlayerSlot slot
	{
		public native get();
	}
	public native int GetName(char[] buffer, int maxlen);
	public native int GetXuid(char[] buffer, int maxlen);
	public native int GetNetworkId(char[] buffer, int maxlen);
	public native int GetAddress(char[] buffer, int maxlen);
	property bool fakePlayer
	{
		public native get();
	}
}

/** ISource2GameClients::ClientPutInServer. */
methodmap ClientPutInServerContext
{
	property CPlayerSlot slot
	{
		public native get();
	}
	public native int GetName(char[] buffer, int maxlen);
	/** 0 a player, 1 a bot. */
	property int type
	{
		public native get();
	}
	public native int GetXuid(char[] buffer, int maxlen);
}

/** ISource2GameClients::ClientDisconnect: pre while the client is still in game, post once it is gone. */
methodmap ClientDisconnectContext
{
	property CPlayerSlot slot
	{
		public native get();
	}
	/** NETWORK_DISCONNECT_* (source2mod/consts). */
	property int reason
	{
		public native get();
	}
	public native int GetName(char[] buffer, int maxlen);
	public native int GetXuid(char[] buffer, int maxlen);
	public native int GetNetworkId(char[] buffer, int maxlen);
}

/** ClientVoice, ClientSettingsChanged. */
methodmap ClientSlotContext
{
	property CPlayerSlot slot
	{
		public native get();
	}
}

/** GameServerSteamAPIActivated / Deactivated: nothing to pass. */
methodmap EmptyContext
{
}

/** IGameEventManager2::LoadEventsFromFile -- a resource file of game events. */
methodmap LoadEventsFromFileContext
{
	public native int GetFilename(char[] buffer, int maxlen);
	property bool searchAll
	{
		public native get();
	}
}

/* ---- function points --------------------------------------------------------------- */

/**
 * One call of a hooked Source2ModFunction: the arguments by index, as the
 * function describes them (`this` of a member function is 0), and the
 * return value -- C++'s FunctionContext. Valid inside the handler.
 *
 * A Set* on an argument in a pre handler changes what the original (and the
 * later handlers) get. SetReturn* with Action_Override / Action_Supersede is
 * what the caller gets; in post GetReturn* is what the original returned.
 */
methodmap FunctionContext < Handle
{
	property int ParamCount
	{
		public native get();
	}

	public native int GetInt(int index);
	public native bool GetBool(int index);
	public native float GetFloat(int index);
	public native CEntityInstance GetEntity(int index);
	public native SchemaObject GetObject(int index);
	/** A string, a CBufferString, a command's text, or an int64 as decimal text. */
	public native int GetString(int index, char[] buffer, int maxlen);
	/** A Vector* / const Vector& or a Vector by value. */
	public native void GetVector(int index, float vec[3]);

	public native void SetInt(int index, int value);
	public native void SetBool(int index, bool value);
	public native void SetFloat(int index, float value);
	public native void SetEntity(int index, CEntityInstance entity);
	public native void SetObject(int index, SchemaObject value);
	public native void SetVector(int index, const float vec[3]);
	/** A String parameter: points at a copy that lives until the call is over. */
	public native void SetString(int index, const char[] value);
	/** An Int64 parameter, from decimal text. */
	public native void SetInt64(int index, const char[] value);
	/** A CBufferString parameter, written in place. */
	public native void SetBufferString(int index, const char[] value);
	/** A Command parameter (const CCommand&) as Source2ModCommandArgs, valid inside the handler. */
	public native Source2ModCommandArgs GetCommandArgs(int index);

	public native int GetReturnInt();
	public native bool GetReturnBool();
	public native float GetReturnFloat();
	public native CEntityInstance GetReturnEntity();
	public native SchemaObject GetReturnObject();
	/** A const char* return value, or an integer as decimal text. */
	public native int GetReturnString(char[] buffer, int maxlen);

	public native void SetReturnInt(int value);
	public native void SetReturnBool(bool value);
	public native void SetReturnFloat(float value);
	public native void SetReturnEntity(CEntityInstance entity);
	public native void SetReturnObject(SchemaObject value);
	public native void SetReturnInt64(const char[] value);

	/**
	 * Pre: the function now, with the current arguments, past every hook; its
	 * value becomes the return value. Answer Action_Supersede afterwards or
	 * the game calls it a second time.
	 */
	public native any CallOriginal();
}

typedef Source2ModFunctionHandler = function Source2ModAction (FunctionContext ctx, bool post);

/* ---- handlers and Source2ModHooks ------------------------------------------------- */

/** The handler of a point: its context, post, and for a point with a value `result`. */
typeset Source2ModHookHandler
{
	function Source2ModAction (TakeDamageContext ctx, bool post, int &result);
	function Source2ModAction (CanAcquireContext ctx, bool post, AcquireResult &result);
	function Source2ModAction (CanMoveContext ctx, bool post, bool &result);
	function Source2ModAction (CanUseContext ctx, bool post, bool &result);
	function Source2ModAction (PostThinkContext ctx, bool post);
	function Source2ModAction (ProcessUsercmdsContext ctx, bool post);
	function Source2ModAction (SimulateUserCommandsContext ctx, bool post);
	function Source2ModAction (RunCommandContext ctx, bool post);
	function Source2ModAction (AcceptInputContext ctx, bool post, bool &result);
	function Source2ModAction (TouchContext ctx, bool post);
	function Source2ModAction (DropWeaponContext ctx, bool post);
	function Source2ModAction (SpawnContext ctx, bool post);
	function Source2ModAction (UseContext ctx, bool post);
	function Source2ModAction (EntityOtherContext ctx, bool post);
	function Source2ModAction (ThinkContext ctx, bool post);
	function Source2ModAction (KilledContext ctx, bool post, bool &result);
	function Source2ModAction (WeaponPickupContext ctx, bool post, int &result);
	function Source2ModAction (WeaponSwitchContext ctx, bool post, bool &result);
	function Source2ModAction (MovementContext ctx, bool post, bool &result);
	function Source2ModAction (AirAccelerateContext ctx, bool post);
	function Source2ModAction (GroundAccelerateContext ctx, bool post);
	function Source2ModAction (CategorizePositionContext ctx, bool post);
	function Source2ModAction (CheckVelocityContext ctx, bool post);
	function Source2ModAction (FullWalkMoveContext ctx, bool post);
	function Source2ModAction (SetupMoveContext ctx, bool post);
	function Source2ModAction (TryPlayerMoveContext ctx, bool post);
	function Source2ModAction (LegacyJumpContext ctx, bool post);
	function Source2ModAction (ModernJumpContext ctx, bool post);
	function Source2ModAction (PlayerChatContext ctx, bool post);
	function Source2ModAction (GameFrameContext ctx, bool post);
	function Source2ModAction (StartupServerContext ctx, bool post);
	function Source2ModAction (ClientConnectContext ctx, bool post, bool &result);
	function Source2ModAction (ClientConnectedContext ctx, bool post);
	function Source2ModAction (ClientPutInServerContext ctx, bool post);
	function Source2ModAction (ClientDisconnectContext ctx, bool post);
	function Source2ModAction (ClientSlotContext ctx, bool post);
	function Source2ModAction (EmptyContext ctx, bool post);
	function Source2ModAction (LoadEventsFromFileContext ctx, bool post);
};

methodmap Source2ModHooks
{
	/**
	 * Hooks a point on every call. The handler's context must be the point's.
	 *
	 * @param post          false: before the original; true: after it.
	 * @param designerName  The object is an entity: only this designer name ("" any).
	 * @param name          AcceptInput: only this input ("" any).
	 * @return The id Unhook takes.
	 */
	public static native int Hook(HookPoint point, Source2ModHookHandler handler, bool post = false,
	                              const char[] designerName = "", const char[] name = "");

	/** The same, only for calls on one object: an entity or a SchemaObject. */
	public static native int HookInstance(HookPoint point, any instance, Source2ModHookHandler handler, bool post = false,
	                                      const char[] designerName = "", const char[] name = "");

	/** The same, for every object of a C++ class ("CCSPlayerPawn") or one derived from it. */
	public static native int HookClass(HookPoint point, const char[] className, Source2ModHookHandler handler, bool post = false,
	                                   const char[] designerName = "", const char[] name = "");

	/**
	 * Hooks any function (HookPoint_Function). A virtual needs a vtable: one
	 * from FromVirtualTable, or HookFunctionInstance / HookFunctionClass.
	 */
	public static native int HookFunction(Source2ModFunction fn, Source2ModFunctionHandler handler, bool post = false);
	public static native int HookFunctionInstance(Source2ModFunction fn, any instance, Source2ModFunctionHandler handler, bool post = false);
	public static native int HookFunctionClass(Source2ModFunction fn, const char[] className, Source2ModFunctionHandler handler, bool post = false);

	/** Drops one of the script's hooks. */
	public static native bool Unhook(int id);

	/** Whether the point resolves on the running game (its gamedata matched). */
	public static native bool IsAvailable(HookPoint point);
}
