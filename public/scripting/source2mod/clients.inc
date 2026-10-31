/**
 * Source2Mod SourcePawn API: a player by slot (ISource2ModClients), off the
 * engine's own client -- the CServerSideClient methodmap below.
 *
 * Nothing here needs a controller, so it also answers for a client that has
 * none (yet). The controller methodmaps' per-client calls (PrintTo*,
 * ReplicateConVar, GetIpAddress, Disconnect, ...) are these with their slot;
 * what only the entity has (networked name and SteamID, pawn, team) stays there.
 */

#if defined _source2mod_clients_included
 #endinput
#endif
#define _source2mod_clients_included

#include <source2mod/types>
#include <source2mod/events>

/**
 * The server's player slots (-maxplayers, at most 64): slots are 0 to
 * MaxClients - 1. Set when the script loads and again at every level init;
 * 0 for a script loaded before the server existed, until the first map.
 */
public const int MaxClients;

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

/** Which way a net channel statistic counts. */
enum NetFlow
{
	NetFlow_Outgoing = 0,    /**< server to client */
	NetFlow_Incoming         /**< client to server */
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

	/*
	 * The net channel (INetChannelInfo, INetChannel). All answer 0 or false for
	 * a client without one: a bot, HLTV, no client.
	 */

	/** Average packets lost, 0 to 1. */
	public native float GetAvgLoss(NetFlow flow);
	/** Average packets choked, 0 to 1. */
	public native float GetAvgChoke(NetFlow flow);
	/** Peak jitter in seconds. */
	public native float GetPeakJitter(NetFlow flow);
	/** Bytes per second. */
	public native float GetAvgData(NetFlow flow);
	/** Packets per second. */
	public native float GetAvgPackets(NetFlow flow);
	/** Bytes since the channel opened, as two cells: bytes[0] the low 32 bits. */
	public native void GetTotalData(NetFlow flow, int bytes[2]);
	public native int GetTotalPackets(NetFlow flow);
	/** The last sequence number used on the flow. */
	public native int GetSequenceNr(NetFlow flow);
	/** The engine's own latency estimate in seconds (Source2ModClients.GetLatency is the average round trip). */
	public native float GetEngineLatency();
	/** What the client reported of its frame rate: seconds per frame and how much it varies. */
	public native void GetRemoteFramerate(float &frameTime, float &stdDev, float &startTimeStdDev);

	/** The channel's data rate in bytes per second. */
	property int DataRate
	{
		public native get();
	}

	property bool IsTimingOut
	{
		public native get();
	}

	property float TimeSinceLastReceived
	{
		public native get();
	}

	property float TimeoutSeconds
	{
		public native get();
	}

	property float TimeUntilTimeout
	{
		public native get();
	}

	property bool IsOverflowed
	{
		public native get();
	}

	property bool HasPendingReliableData
	{
		public native get();
	}

	/** How long the client may stay silent before it times out. */
	public native void SetTimeout(float seconds);

	/* What the engine keeps on the client itself. */

	/** The client's rate setting (bytes per second). */
	property int Rate
	{
		public native get();
		public native set(int value);
	}

	public native void SetUpdateRate(float rate);

	/** Changes a userinfo cvar as if the client had sent it. */
	public native void SetUserCVar(const char[] name, const char[] value);

	/** The tick the client acknowledged, what its next snapshot deltas from; -1 for a full update. */
	property int DeltaTick
	{
		public native get();
	}

	/** Loading progress the client reported, in percent. */
	property int LoadingProgress
	{
		public native get();
	}

	/** Failed commands in a row; the engine kicks the client after 16. */
	property int FailedCommands
	{
		public native get();
	}

	/** Sends the client its next snapshot whole, no delta. */
	public native void ForceFullUpdate();

	/**
	 * Bans the client on the engine's own list, what "banid" adds to; plugins
	 * keep any list of their own that must survive a restart. A sv_lan server
	 * has no such list.
	 * @param duration Minutes, 0 for good
	 */
	public native void Ban(float duration, bool kick = true);

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

	/** CServerSideClient.Ban by SteamID (any form), whether connected or not. @return false when steamId is none */
	public static native bool BanSteamId(const char[] steamId, float duration, bool kick = true);

	/**
	 * A fake client from the engine: in game with a controller. For a playing
	 * bot use ServerCommand("bot_add"). CPlayerSlot(-1) when no slot is free.
	 */
	public static native CPlayerSlot CreateFakeClient(const char[] name);

	/** A userinfo ConVar of a fake client. */
	public static native void SetFakeClientConVar(CPlayerSlot slot, const char[] name, const char[] value);

	/** The slot of a connected client by SteamID (any form); CPlayerSlot(-1) when there is none. */
	public static native CPlayerSlot FromSteamId(const char[] steamId);
}
