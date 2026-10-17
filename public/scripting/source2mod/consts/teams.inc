/**
 * Source2Mod SourcePawn API: teams, round end reasons, HUD print
 * destinations and chat colours.
 */

#if defined _source2mod_consts_teams_included
 #endinput
#endif
#define _source2mod_consts_teams_included

// Teams (shareddefs.h SharedTeams_t): m_iTeamNum, SwitchTeam, ChangeTeam.
#define TEAM_ANY            -1
#define TEAM_INVALID        -1
#define TEAM_UNASSIGNED     0
#define TEAM_SPECTATOR      1
#define CS_TEAM_NONE        0
#define CS_TEAM_SPECTATOR   1
#define CS_TEAM_T           2
#define CS_TEAM_CT          3

// Why a round ended (source2mod/consts.h CSRoundEndReason): TerminateRound,
// CCSGameRules.m_eRoundWinReason / m_eRoundEndReason.
#define CSRoundEnd_TargetBombed         1
#define CSRoundEnd_VIPEscaped           2
#define CSRoundEnd_VIPKilled            3
#define CSRoundEnd_TerroristsEscaped    4
#define CSRoundEnd_CTStoppedEscape      5
#define CSRoundEnd_TerroristsStopped    6
#define CSRoundEnd_BombDefused          7
#define CSRoundEnd_CTWin                8
#define CSRoundEnd_TerroristWin         9
#define CSRoundEnd_Draw                 10
#define CSRoundEnd_HostagesRescued      11
#define CSRoundEnd_TargetSaved          12
#define CSRoundEnd_HostagesNotRescued   13
#define CSRoundEnd_TerroristsNotEscaped 14
#define CSRoundEnd_VIPNotEscaped        15
#define CSRoundEnd_GameStart            16
#define CSRoundEnd_TerroristsSurrender  17
#define CSRoundEnd_CTSurrender          18
#define CSRoundEnd_TerroristsPlanted    19
#define CSRoundEnd_CTsReachedHostage    20
#define CSRoundEnd_SurvivalWin          21
#define CSRoundEnd_SurvivalDraw         22

// Where a TextMsg goes (shareddefs.h TextMsgDestinations).
#define HUD_PRINTNOTIFY     1
#define HUD_PRINTCONSOLE    2
#define HUD_PRINTTALK       3
#define HUD_PRINTCENTER     4
#define HUD_PRINTTALK2      5
#define HUD_PRINTALERT      6

// Chat colours (source2mod/consts.h ChatColors): put one in front of the text
// it colours, e.g. Format(buffer, sizeof(buffer), "%s[Shop]%s text",
// CHAT_GREEN, CHAT_DEFAULT). A message that starts with a colour needs a
// leading space or CHAT_DEFAULT before it, or the client drops the first one.
#define CHAT_DEFAULT        "\x01"
#define CHAT_WHITE          "\x01"
#define CHAT_DARKRED        "\x02"
#define CHAT_LIGHTPURPLE    "\x03"
#define CHAT_GREEN          "\x04"
#define CHAT_OLIVE          "\x05"
#define CHAT_LIME           "\x06"
#define CHAT_RED            "\x07"
#define CHAT_GREY           "\x08"
#define CHAT_YELLOW         "\x09"
#define CHAT_SILVER         "\x0A"
#define CHAT_BLUEGREY       "\x0A"
#define CHAT_BLUE           "\x0B"
#define CHAT_DARKBLUE       "\x0C"
#define CHAT_PURPLE         "\x0E"
#define CHAT_MAGENTA        "\x0E"
#define CHAT_LIGHTRED       "\x0F"
#define CHAT_GOLD           "\x10"
#define CHAT_ORANGE         "\x10"
