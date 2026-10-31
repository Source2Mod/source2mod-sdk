/**
 * Source2Mod SourcePawn API: the core and the engine -- ISource2ModAPI
 * (logging, the console, formatting, the global variables) and the engine
 * server (IVEngineServer2: server commands, engine time).
 *
 * Format / FormatArgs take SourceMod's format: %d %i %u %b %f %x %X %s %c
 * %t %T %N %L %%, with -, 0, width and precision.
 */

#if defined _source2mod_engine_included
 #endinput
#endif
#define _source2mod_engine_included

#include <source2mod/core>

/**
 * The engine's global variables (CGlobalVars): times, ticks, the map. Read
 * fresh on every access; Source2ModApi.GetGlobalVars() gives the one there
 * is.
 */
methodmap CGlobalVars
{
	property float realtime
	{
		public native get();
	}
	property int framecount
	{
		public native get();
	}
	property float absoluteframetime
	{
		public native get();
	}
	property float absoluteframetime_unbounded
	{
		public native get();
	}
	property int maxClients
	{
		public native get();
	}
	property int m_nCurrentTickThisFrame
	{
		public native get();
	}
	property int m_nTotalTicksThisFrame
	{
		public native get();
	}
	property float curtime
	{
		public native get();
	}
	property float frametime
	{
		public native get();
	}
	property float interpolation_amount
	{
		public native get();
	}
	property bool m_bInSimulation
	{
		public native get();
	}
	property int tickcount
	{
		public native get();
	}
	property float m_flSubtickFraction
	{
		public native get();
	}
	/** MapLoadType_t */
	property int eLoadType
	{
		public native get();
	}
	property bool mp_teamplay
	{
		public native get();
	}
	property int maxEntities
	{
		public native get();
	}
	property int serverCount
	{
		public native get();
	}
	public native int GetMapName(char[] buffer, int maxlen);
	public native int GetStartSpot(char[] buffer, int maxlen);
}

/**
 * ISource2ModAPI. Its subsystem accessors (Commands(), Events(), ...) are the
 * Source2Mod* methodmaps of the other includes.
 */
methodmap Source2ModApi
{
	/** The engine's global variables. */
	public static CGlobalVars GetGlobalVars()
	{
		return view_as<CGlobalVars>(0);
	}

	/**
	 * Logs a line: on the console as "[Plugin name] text", in Source2Mod.log
	 * with the time and the plugin's file (C++ ISource2ModAPI::Log).
	 */
	public static native void Log(const char[] format, any ...);

	/** The same as an error. */
	public static native void LogError(const char[] format, any ...);

	/** Prints to the server console as it is (no newline added). */
	public static native void ConPrint(const char[] message);

	/** Prints a formatted line to the server console. */
	public static native void ConPrintf(const char[] format, any ...);

	/** Formats into buffer. @return Bytes written */
	public static native int Format(char[] buffer, int maxlen, const char[] format, any ...);

	/**
	 * Formats with the calling function's own variadic arguments, starting
	 * at its parameter varpos (1-based) (C++ FormatArgs, SourceMod's VFormat).
	 * SourcePawn 1 plugins only: SourcePawn 2 boxes a function's variadic
	 * arguments, so a plugin built with today's spcomp passes them on --
	 * Source2ModApi.Format(buffer, maxlen, format, ...) -- and this raises
	 * an error.
	 * @return Bytes written
	 */
	public static native int FormatArgs(char[] buffer, int maxlen, const char[] format, int varpos);
}

/** A line the server printed: its logging channel, severity and the text with its newline. */
typedef ConsoleOutputCallback = function void (const char[] channel, int severity, const char[] message);

/** The engine server (IVEngineServer2). */
methodmap Source2ModEngine
{
	/** Queues a server console command (formatted); it runs on the next frame. */
	public static native void ServerCommand(const char[] format, any ...);

	/** Seconds since the server started, not scaled or paused (Plat_FloatTime). */
	public static native float GetEngineTime();

	/**
	 * A launch parameter (ICommandLine) and the value after it.
	 * @return false when the server was not started with it; the buffer is then empty
	 */
	public static native bool GetLaunchParam(const char[] name, char[] buffer, int maxlen);

	public static native bool HasLaunchParam(const char[] name);

	/** The whole command line. @return Bytes written */
	public static native int GetCommandLine(char[] buffer, int maxlen);

	/**
	 * The server's address as "ip:port": its public address, else its local one.
	 * @return false before the network system has one
	 */
	public static native bool GetServerAddress(char[] buffer, int maxlen);

	/** The game's build number. */
	public static native int GetBuildVersion();

	/** Nobody is on the server and it hibernates: frames and timers do not run. */
	public static native bool IsHibernating();

	/** The server was asked to quit. */
	public static native bool IsShuttingDown();

	/**
	 * How the server is doing: CPU use as the engine samples it, and bytes per
	 * second in and out over all players. All 0 without a server.
	 */
	public static native void GetServerPerf(float &cpu, float &inflow, float &outflow);

	/**
	 * The demo being recorded.
	 * @return false when none is
	 */
	public static native bool GetRecordingDemo(char[] buffer, int maxlen);

	/** The server's time scale, 1 is normal (host_timescale without its cheat flag). */
	public static native void SetTimescale(float scale);

	/**
	 * A token from the server's own localization files ("#SFUI_..."). A server
	 * loads few of them: translations for players belong in phrase files.
	 * @return false when the server does not have the token
	 */
	public static native bool Localize(const char[] token, char[] buffer, int maxlen);

	/**
	 * Listens to everything the server prints. Lines from other threads arrive
	 * with the next line printed on the game thread. What the callback prints
	 * itself does not come back to it.
	 * @return The id RemoveConsoleListener takes
	 */
	public static native int AddConsoleListener(ConsoleOutputCallback callback);
	public static native void RemoveConsoleListener(int id);

	/**
	 * Runs a console command now, as the server's console, and captures what it
	 * prints. Unlike ServerCommand a ConVar name is not a command here; a command
	 * that does not exist raises an error.
	 * @return Bytes written
	 */
	public static native int ServerCommandEx(char[] output, int maxlen, const char[] format, any ...);
}
