/**
 * Source2Mod SourcePawn API: voice chat (ISource2ModVoice).
 *
 * Who hears whom. Each plugin keeps its own flags and overrides, the core
 * combines them (a mute wins) and drops a plugin's when it unloads, and a
 * player's when they leave. In order, the first that applies decides: the
 * receiver muted the sender themselves, the sender is Muted, an override, the
 * sender's All or the receiver's ListenAll, the sender's Team or the receiver's
 * ListenTeam, what the game decided.
 */

#if defined _source2mod_voice_included
	#endinput
#endif
#define _source2mod_voice_included

enum Source2ModVoiceFlags
{
	Source2ModVoice_Normal = 0,
	Source2ModVoice_Muted = 1 << 0,
	Source2ModVoice_All = 1 << 1,
	Source2ModVoice_ListenAll = 1 << 2,
	Source2ModVoice_Team = 1 << 3,
	Source2ModVoice_ListenTeam = 1 << 4
};

enum Source2ModListen
{
	Source2ModListen_Default = 0,
	Source2ModListen_Mute,
	Source2ModListen_Hear
};

methodmap Source2ModVoice
{
	/** This plugin's flags for the player in this slot, replacing what it set before. */
	public static native void SetVoiceFlags(int slot, int flags);

	/** The player's flags from every plugin together. */
	public static native int GetVoiceFlags(int slot);

	/** This plugin's override for what the receiver hears of the sender. */
	public static native void SetListenOverride(int receiver, int sender, Source2ModListen listen);

	/** The override from every plugin together. */
	public static native Source2ModListen GetListenOverride(int receiver, int sender);

	/** Whether the receiver muted the sender themselves. Clients report the first 32 slots only. */
	public static native bool IsSelfMuted(int receiver, int sender);
}
