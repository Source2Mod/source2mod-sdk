/**
 * Source2Mod SourcePawn API: player helpers (ISource2ModClients).
 *
 * What the controller methodmaps already do (name, IP, SteamID64, user id,
 * console commands, team changes, Disconnect) stays there.
 */

#if defined _source2mod_clients_included
 #endinput
#endif
#define _source2mod_clients_included

#include <source2mod/types>
#include <source2mod/events>

/** SteamID forms (ESource2ModAuthId). */
enum Source2ModAuthIdForm
{
	AuthIdForm_Engine = 0,    /**< as the engine shows it (Steam2) */
	AuthIdForm_Steam2,        /**< STEAM_1:0:12345 */
	AuthIdForm_Steam3,        /**< [U:1:24690] */
	AuthIdForm_SteamID64      /**< 76561197960290418 */
}

/** The engine's signon states (SignonState_t). */
enum SignonState
{
	SignonState_None = 0,
	SignonState_Challenge,
	SignonState_Connected,
	SignonState_New,
	SignonState_Prespawn,
	SignonState_Spawn,
	SignonState_Full,        /**< fully in game */
	SignonState_ChangeLevel
}

/** What a client answered a QueryConVar (ESource2ModConVarQueryResult). */
enum ConVarQueryResult
{
	ConVarQuery_Cancelled = -1,  /**< the client left before it answered */
	ConVarQuery_Okay = 0,        /**< the value is the client's */
	ConVarQuery_NotFound,        /**< the client has no such ConVar */
	ConVarQuery_NotValid,        /**< a ConCommand, not a ConVar */
	ConVarQuery_Protected        /**< FCVAR_PROTECTED, the value is withheld */
}

/**
 * The answer to a QueryConVar.
 * @param cookie    What QueryConVar returned.
 * @param cvarValue The client's value; empty unless result is ConVarQuery_Okay.
 * @param value     What QueryConVar was handed.
 */
typedef ConVarQueryFinished = function void (int cookie, CPlayerSlot slot, ConVarQueryResult result, const char[] cvarName, const char[] cvarValue, any value);

/**
 * The engine's client of a slot (CServerSideClient): what the server knows of
 * a connection before (and after) it has a controller -- signon state, net
 * channel, fake flag. Only the slot is kept; every call looks the client up
 * again, so a client that left is IsValid false rather than a stale pointer.
 */
methodmap CServerSideClient
{
	public CServerSideClient(CPlayerSlot slot)
	{
		return view_as<CServerSideClient>(slot);
	}

	property CPlayerSlot Slot
	{
		public get() { return view_as<CPlayerSlot>(this); }
	}

	/** The slot has an engine client, connecting or in game (not the empty one the engine keeps between clients). */
	property bool IsValid
	{
		public native get();
	}

	/** SignonState_None when there is no client. */
	property SignonState SignonState
	{
		public native get();
	}

	/** Signon state Connected or later. */
	public bool IsConnected() { return this.SignonState >= SignonState_Connected; }
	/** Signon state Full: fully in game. */
	public bool IsActive() { return this.SignonState == SignonState_Full; }

	property bool IsFakePlayer
	{
		public native get();
	}

	property bool IsHLTV
	{
		public native get();
	}

	/** The client has a net channel to answer over (no bot, no HLTV). */
	property bool HasNetChannel
	{
		public native get();
	}

	/** Steam has confirmed the client's SteamID. */
	property bool IsFullyAuthenticated
	{
		public native get();
	}

	/** -1 when there is no client. */
	property int UserId
	{
		public native get();
	}

	/** @return Bytes written */
	public native int GetName(char[] buffer, int maxlen);

	/** The SteamID as the engine shows it ([U:1:N]). @return Bytes written */
	public native int GetNetworkIDString(char[] buffer, int maxlen);

	/** The 64-bit SteamID as decimal text. @return Bytes written */
	public native int GetSteamID(char[] buffer, int maxlen);

	/** The remote address with its port. @return Bytes written */
	public native int GetAddress(char[] buffer, int maxlen);

	/**
	 * Asks the client for any of its ConVars (SVC_GetCvarValue). Unlike
	 * Source2ModClients.GetClientConVar, which reads the copy of a userinfo
	 * cvar the client sent on its own, this goes over the wire and back:
	 * the callback runs when the client answers, or with ConVarQuery_Cancelled
	 * when it disconnects first. A client that never answers never calls it.
	 *
	 * @return A cookie the callback is handed back, or -1 (a bot, no client,
	 *         no net channel) -- the callback is then never called.
	 */
	public native int QueryConVar(const char[] name, ConVarQueryFinished callback, any value = 0);

	/**
	 * Sets a replicated ConVar's value on this client only (NET_SetConVar);
	 * the server's own value and every other client stay as they are.
	 * @return false when the client has no net channel (a bot, no client).
	 */
	public native bool ReplicateConVar(const char[] name, const char[] value);

	/** The IPv4 address without the port; empty for a bot. @return Bytes written */
	public native int GetIpAddress(char[] buffer, int maxlen);

	/** Text on this client (TextMsg): its console (a newline is added), chat, center text or center alert. */
	public native void PrintToConsole(const char[] message);
	public native void PrintToChat(const char[] message);
	public native void PrintToCenter(const char[] message);
	public native void PrintToCenterAlert(const char[] message);

	/**
	 * CS2's HTML center text (show_survival_respawn_status) on this client.
	 * @param menu false: left out while the player has a menu open.
	 */
	public native void PrintToCenterHtml(const char[] message, int duration = 5, bool menu = false);

	/** Runs a command on the client's own console, as if typed there. */
	public native void ExecuteClientCommand(const char[] command);

	/** Runs a server ConCommand on the client's behalf; nothing goes over the wire. */
	public native void ExecuteClientCommandFromServer(const char[] command);

	/** Fires the event to this client's legacy listener only; the event stays the script's. */
	public native void FireEventToClient(IGameEvent event);

	/**
	 * Disconnects the client on the next frame (safe from inside its own hooks);
	 * its open menu is closed right away.
	 * @param reason         NETWORK_DISCONNECT_* (source2mod/consts/disconnect): what CS2 shows the client
	 * @param internalReason What the server logs
	 */
	public native void Disconnect(int reason, const char[] internalReason = "");
}

methodmap Source2ModClients
{
	/** A slot has a client (connecting or in game). */
	public static native bool IsConnected(CPlayerSlot slot);

	/** A slot's client is fully in game. */
	public static native bool IsInGame(CPlayerSlot slot);
	/** A bot, by the engine client's own flag (true before it has a controller). */
	public static native bool IsFakeClient(CPlayerSlot slot);

	/** Steam has confirmed the client's SteamID. */
	public static native bool IsAuthorized(CPlayerSlot slot);

	/**
	 * A client's SteamID as text; empty for a bot, no client, or (validated)
	 * one Steam has not confirmed yet.
	 * @return Bytes written
	 */
	public static native int GetAuthId(CPlayerSlot slot, Source2ModAuthIdForm form, char[] buffer, int maxlen, bool validated = true);

	/** Average round trip in seconds (0 for a bot or no client). */
	public static native float GetLatency(CPlayerSlot slot);

	/** Seconds connected (0 for a bot or no client). */
	public static native float GetConnectionTime(CPlayerSlot slot);

	/** A cvar the client sent (cl_language, ...). @return Bytes written */
	public static native int GetClientConVar(CPlayerSlot slot, const char[] name, char[] buffer, int maxlen);

	/** CServerSideClient.QueryConVar on a slot's client. */
	public static int QueryClientConVar(CPlayerSlot slot, const char[] name, ConVarQueryFinished callback, any value = 0)
	{
		return CServerSideClient(slot).QueryConVar(name, callback, value);
	}

	/** CServerSideClient.ReplicateConVar on a slot's client. */
	public static bool ReplicateConVar(CPlayerSlot slot, const char[] name, const char[] value)
	{
		return CServerSideClient(slot).ReplicateConVar(name, value);
	}

	/** Disconnect(slot, NETWORK_DISCONNECT_KICKED, message): CS2 shows them the kick reason's own text; the message is what the server logs. */
	public static native void Kick(CPlayerSlot slot, const char[] format, any ...);

	/** CServerSideClient.Disconnect on a slot's client. */
	public static void Disconnect(CPlayerSlot slot, int reason, const char[] internalReason = "")
	{
		CServerSideClient(slot).Disconnect(reason, internalReason);
	}

	/** Clients on the server, bots included. */
	public static native int GetClientCount(bool inGameOnly = true);

	/**
	 * Any SteamID form (STEAM_X:Y:Z, [U:1:N], SteamID64) in another form.
	 * @return false when steamId is none of them
	 */
	public static native bool ConvertSteamId(const char[] steamId, Source2ModAuthIdForm form, char[] buffer, int maxlen);

	/** A connected client's user id (what #<userid> targets); -1 when there is no client. */
	public static native int GetUserId(CPlayerSlot slot);

	/** The slot of a connected client by user id; CPlayerSlot(-1) when there is none. */
	public static native CPlayerSlot FromUserId(int userid);

	/** The slot of a connected client by SteamID (any form); CPlayerSlot(-1) when there is none. */
	public static native CPlayerSlot FromSteamId(const char[] steamId);
}
