#pragma once

#include "ISource2ModTypes.h"

#include <playerslot.h>

/**
 * @file ISource2ModVoice.h
 * @brief Who hears whom in voice chat.
 *
 * A player's flags say how they speak and listen; an override decides one
 * receiver and one sender. Each plugin keeps its own, the core combines them
 * (a mute wins) and drops a plugin's when it unloads, and a player's when they
 * leave. A mute the player made themselves (the scoreboard) always holds.
 *
 * In order, the first that applies decides:
 *   1. the receiver muted the sender themselves: not heard;
 *   2. the sender has Source2ModVoice_Muted: not heard;
 *   3. an override: Mute is not heard, Hear is heard;
 *   4. the sender has _All or the receiver _ListenAll: heard;
 *   5. the sender has _Team or the receiver _ListenTeam: heard within the team;
 *   6. what the game decided.
 *
 * @code
 * g_pSource2ModVoice->SetVoiceFlags(slot, Source2ModVoice_Muted);
 * g_pSource2ModVoice->SetListenOverride(warden, prisoner, Source2ModListen::Hear);
 * @endcode
 */

enum Source2ModVoiceFlags : int
{
    Source2ModVoice_Normal = 0,
    Source2ModVoice_Muted = 1 << 0,
    Source2ModVoice_All = 1 << 1,
    Source2ModVoice_ListenAll = 1 << 2,
    Source2ModVoice_Team = 1 << 3,
    Source2ModVoice_ListenTeam = 1 << 4,
};

enum class Source2ModListen : int
{
    Default = 0,
    Mute,
    Hear,
};

#define SOURCE2MOD_VOICE_INTERFACE "ISource2ModVoice001"

class ISource2ModVoice
{
public:
    virtual ~ISource2ModVoice() = default;

    /// This plugin's flags for the player, replacing what it set before.
    virtual void SetVoiceFlags(CPlayerSlot slot, int flags) = 0;
    /// The player's flags from every plugin together.
    virtual int GetVoiceFlags(CPlayerSlot slot) = 0;

    /// This plugin's override for what the receiver hears of the sender.
    virtual void SetListenOverride(CPlayerSlot receiver, CPlayerSlot sender, Source2ModListen listen) = 0;
    /// The override from every plugin together.
    virtual Source2ModListen GetListenOverride(CPlayerSlot receiver, CPlayerSlot sender) = 0;

    /// Whether the receiver muted the sender themselves. Clients report the first
    /// 32 slots only.
    virtual bool IsSelfMuted(CPlayerSlot receiver, CPlayerSlot sender) = 0;
};
