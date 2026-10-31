/**
 * Source2Mod SourcePawn API: gamedata (ISource2ModGameConfig) and calling
 * game functions by address.
 *
 * A Source2ModFunction is a game function the script calls: found by a
 * gamedata entry, a signature or symbol of its own, or a vtable slot, with
 * its parameters described once. What C++ does with ADDR_* /
 * GAMECONFIG_RESOLVE and a function pointer type.
 *
 * Arguments: on Windows at most 14; on Linux 6 integer-class (ints, entities,
 * objects, pointers, strings) and 8 floating-point ones in registers, then 8
 * more of either kind on the stack.
 */

#if defined _source2mod_gameconfig_included
 #endinput
#endif
#define _source2mod_gameconfig_included

#include <source2mod/types>
#include <source2mod/addresses>

/**
 * How a value crosses into a game function and back.
 */
enum Source2ModType
{
	Source2ModType_Void = 0,
	Source2ModType_Bool,
	Source2ModType_Int8,
	Source2ModType_Int16,
	Source2ModType_Int32,           /**< also enums, uint32 */
	Source2ModType_Int64,           /**< passed as decimal text ("123"); returned through CallInt64 */
	Source2ModType_Float,
	Source2ModType_Double,          /**< passed as float */
	Source2ModType_Entity,          /**< CEntityInstance*: an entity (handle) on the script side, NULL_ENTITY = nullptr */
	Source2ModType_Object,          /**< a schema object pointer: a SchemaObject */
	Source2ModType_String,          /**< const char*; returned through CallString */
	Source2ModType_Vector,          /**< Vector* / const Vector&: float[3], written back after the call; returned through CallVector */
	Source2ModType_VectorValue,     /**< Vector by value: float[3]; returned through CallVector */
	Source2ModType_Pointer,         /**< any other pointer: an opaque SchemaObject */
	Source2ModType_BufferString,    /**< CBufferString*: text (a hook can set it with SetBufferString) */
	Source2ModType_Command,         /**< const CCommand&: in a hook, GetCommandArgs; not callable */
	Source2ModType_IntRef,          /**< int32* out-parameter: an int variable, written back after the call */
	Source2ModType_BoolRef,         /**< bool* out-parameter: a bool variable, written back after the call */
	Source2ModType_FloatRef         /**< float* out-parameter: a float variable, written back after the call */
};

/**
 * The platform the server runs on, for functions whose parameters differ
 * between the two (CCSGameRules::TerminateRound, GroundAccelerate).
 */
enum Source2ModPlatform
{
	Platform_Linux = 0,
	Platform_Windows
};

methodmap Source2ModGameConfig
{
	/** @return false when the entry does not exist */
	public static native bool GetLibrary(const char[] name, char[] buffer, int maxlen);
	public static native bool GetSignature(const char[] name, char[] buffer, int maxlen);
	public static native bool GetSymbol(const char[] name, char[] buffer, int maxlen);
	public static native bool GetPatch(const char[] name, char[] buffer, int maxlen);

	/** @return The offset, -1 when there is none */
	public static native int GetOffset(const char[] name);

	public static native bool IsSymbol(const char[] name);

	/** Whether the entry's signature or symbol is found on the running game. */
	public static native bool Resolves(const char[] name);

	/**
	 * Whether a signature of the script's own matches in a module ("server"),
	 * optionally only in one section (".text").
	 */
	public static native bool SignatureMatches(const char[] module, const char[] signature, const char[] section = "");

	public static native Source2ModPlatform GetPlatform();

	/**
	 * Where the entry's signature or symbol is on the running game, as an
	 * address (C++ ResolveSignature): read and write at it with
	 * Source2ModSchema's *At natives, step from it with Source2ModMemory.
	 *
	 * @return 0 when the entry does not resolve
	 */
	public static native SchemaObject ResolveSignature(const char[] name);
}

/**
 * A module of the game ("server", "engine2", "tier0", ...), for what gamedata
 * does not have (ISource2ModModule). Addresses come back as SchemaObjects, 0
 * when there is nothing. Close it (delete) when done.
 *
 *   Source2ModModule server = new Source2ModModule("server");
 *   SchemaObject insn = server.FindPattern("48 8B 05 ? ? ? ? 48 8B 08");
 *   SchemaObject globals = Source2ModMemory.Deref(Source2ModMemory.ResolveRelativeAddress(insn, 3, 7));
 *   delete server;
 */
methodmap Source2ModModule < Handle
{
	/** null when no such module is loaded. */
	public native Source2ModModule(const char[] name);

	/** The first match of an IDA-style pattern ("48 8B ? ? 89"), optionally in one section (".text"). */
	public native SchemaObject FindPattern(const char[] pattern, const char[] section = "");

	/** An exported or debug symbol. */
	public native SchemaObject GetFunctionByName(const char[] symbol);

	/** The vtable of a class by its RTTI name ("CCSPlayerController"); decorated: the name is mangled. */
	public native SchemaObject GetVirtualTable(const char[] className, bool decorated = false);

	/** The vtable of a non-virtual base's sub-object, found through the RTTI. */
	public native SchemaObject GetVirtualTableByBase(const char[] className, const char[] baseName, bool decorated = false);

	/** The vtable of the sub-object at an offset inside the class. */
	public native SchemaObject GetVirtualTableByOffset(const char[] className, int offset, bool decorated = false);

	/** Where the module is loaded. */
	property SchemaObject Base
	{
		public native get();
	}

	/** @return Bytes written */
	public native int GetName(char[] buffer, int maxlen);
	public native int GetPath(char[] buffer, int maxlen);

	/** A section (".text", ".rdata", ".data"): where it starts and how many bytes it has. @return false when there is none */
	public native bool GetSection(const char[] name, SchemaObject &start, int &size);
}

/**
 * Steps from an address (ISource2ModMemory). Every one takes and gives a
 * SchemaObject; a null one is an error.
 */
methodmap Source2ModMemory
{
	/** The address `offset` bytes on (negative goes back). */
	public static native SchemaObject Offset(SchemaObject address, int offset);

	/** The pointer stored at the address, `times` over; 0 when one is null. */
	public static native SchemaObject Deref(SchemaObject address, int times = 1);

	/** The target of the call / jmp rel32 at the address. */
	public static native SchemaObject FollowNearCall(SchemaObject address, int opcodeOffset = 1, int nextInstructionOffset = 5);

	/** What a rip-relative operand at registerOffset points at (lea/mov reg, [rip+x]: 3 and 7). */
	public static native SchemaObject ResolveRelativeAddress(SchemaObject address, int registerOffset = 0, int nextInstructionOffset = 4);

	/** "0x7f12..." for a log line; "0x0" for null. @return Bytes written */
	public static native int Format(SchemaObject address, char[] buffer, int maxlen);

	/** to - from in bytes (an offset inside one module or object). */
	public static native int Distance(SchemaObject from, SchemaObject to);
}

/**
 * A game function to call. Close it (delete) when done; it goes with the
 * plugin otherwise.
 */
methodmap Source2ModPatch < Handle
{
	/**
	 * Bytes to write over code, "90 90 E9" (what GetPatch gives). Nothing is
	 * written until Enable; the original comes back with Disable, on delete and
	 * when the script goes.
	 */
	public native Source2ModPatch(SchemaObject address, const char[] bytes);

	/** @return false when another enabled patch, of any script, covers these bytes */
	public native bool Enable();
	public native void Disable();

	property bool Enabled
	{
		public native get();
	}
}

methodmap Source2ModFunction < Handle
{
	/**
	 * A function described in gamedata: an entry with a "function" object
	 * (its `this`, return and parameter types; see ISource2ModFunctions.h),
	 * the same one C++ plugins call through ISource2ModFunctions::Find. No
	 * AddParam: the types come from the gamedata. A plugin's own gamedata
	 * file in addons/source2mod/gamedata is read with the core's.
	 *
	 * @return null when there is no such entry or it does not resolve (the
	 *         reason is on the console).
	 */
	public static native Source2ModFunction Find(const char[] name);

	/** A function by gamedata entry ("CCSPlayerController::SwitchTeam"); null when it does not resolve. */
	public static native Source2ModFunction FromGameConfig(const char[] name, Source2ModType returnType);

	/**
	 * A function by a signature of the script's own, in a module ("server"),
	 * optionally only in one section (".text"); null when it does not match.
	 */
	public static native Source2ModFunction FromSignature(const char[] module, const char[] signature, Source2ModType returnType, const char[] section = "");

	/**
	 * A function a matched instruction refers to: the signature matches a
	 * call / jmp / lea, the function is its rel32 target. The default is a
	 * call or jmp at the start of the match (ISource2ModMemory::FollowNearCall);
	 * for others give where the rel32 is and where the next instruction starts,
	 * both from the start of the match (ResolveRelativeAddress).
	 */
	public static native Source2ModFunction FromSignatureRelative(const char[] module, const char[] signature, Source2ModType returnType,
	                                                              int offset = 1, int nextInstructionOffset = 5, const char[] section = "");

	/** A function by its exported or debug symbol in a module ("server"); null when there is none. */
	public static native Source2ModFunction FromSymbol(const char[] module, const char[] symbol, Source2ModType returnType);

	/**
	 * A think, touch or use function by its name in an entity class's datamap, the
	 * class's parents searched too: no signature needed. Add its parameters with
	 * AddParam. Null when it is unknown, or before a map is up.
	 */
	public static native Source2ModFunction FromDatamap(const char[] className, const char[] functionName, Source2ModType returnType);

	/** A virtual function: called on the object passed as the first parameter, by vtable index. */
	public static native Source2ModFunction FromVirtual(int vtableIndex, Source2ModType returnType);

	/**
	 * A slot of one class's vtable (an RTTI class name in a module), called
	 * whatever the object's own class is; baseName picks the vtable of a
	 * secondary base. The object is still the first parameter.
	 */
	public static native Source2ModFunction FromVirtualTable(const char[] module, const char[] vtableName, int vtableIndex, Source2ModType returnType, const char[] baseName = "");

	/** A function at a core address (ISource2ModAddresses); null when it is not resolved. */
	public static native Source2ModFunction FromAddress(Source2ModAddress address, Source2ModType returnType);

	/** A function at any address; a wrong one crashes the server when called. Parameters with AddParam. */
	public static native Source2ModFunction FromPointer(SchemaObject address, Source2ModType returnType);

	/** Where the function is; null for a virtual, which is read off the object's vtable. */
	property SchemaObject Address
	{
		public native get();
	}

	/** The next parameter; `this` of a member function is the first. */
	public native void AddParam(Source2ModType type);

	/**
	 * What the function is: for one from Find, as its gamedata entry has it;
	 * for one the script built, as built (FromVirtual* are virtuals and
	 * members). `this` of a member function is parameter 0.
	 */
	property Source2ModType ReturnType
	{
		public native get();
	}

	property int ParamCount
	{
		public native get();
	}

	public native Source2ModType GetParamType(int index);

	/** Called through the vtable of the object it is called on. */
	property bool IsVirtual
	{
		public native get();
	}

	/** A member function: `this` is its first parameter. */
	property bool IsMember
	{
		public native get();
	}

	/** The gamedata name; "" for one the script built. @return Bytes written */
	public native int GetName(char[] buffer, int maxlen);

	/**
	 * Whether every Call* goes past the hooks on the function (all plugins',
	 * raw and game hooks), as CallOriginal does: C++'s HookChain::Bypass.
	 */
	property bool BypassHooks
	{
		public native get();
		public native set(bool bypass);
	}

	/**
	 * Calls it. Arguments follow the parameters (entities and objects as
	 * themselves, vectors as float[3], strings as text, out-parameters as
	 * variables).
	 *
	 * @return The return value (int, bool, float, entity, object); 0 for void.
	 *         An Int64 gives its low 32 bits (CallInt64 gives all); a String
	 *         or Vector return is an error (CallString, CallVector).
	 */
	public native any Call(any ...);

	/**
	 * Calls the function past every hook on it (SOURCE2MOD_ORIGINAL): what a
	 * hook of this function calls to run the game's code without recursing.
	 */
	public native any CallOriginal(any ...);

	/** Calls it and puts an integer return value in buffer as decimal text. @return bytes written */
	public native int CallInt64(char[] buffer, int maxlen, any ...);

	/** Calls a function returning a String; "" for null. @return bytes written */
	public native int CallString(char[] buffer, int maxlen, any ...);

	/**
	 * Calls a function returning a Vector (by value) or Vector* (0,0,0 for
	 * null). On Windows a Vector by value comes back through a hidden pointer
	 * placed after `this` when the function is FromVirtual / FromVirtualTable
	 * or its first parameter is an Entity, Object or Pointer, first otherwise.
	 */
	public native void CallVector(float vec[3], any ...);
}
