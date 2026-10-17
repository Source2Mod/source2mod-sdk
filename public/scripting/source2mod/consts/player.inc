/**
 * Source2Mod SourcePawn API: life states, observer modes, HUD hiding flags.
 */

#if defined _source2mod_consts_player_included
 #endinput
#endif
#define _source2mod_consts_player_included

// Life states (const.h LifeState_t): m_lifeState.
#define LIFE_ALIVE          0
#define LIFE_DYING          1
#define LIFE_DEAD           2
#define LIFE_RESPAWNABLE    3
#define LIFE_RESPAWNING     4

// Observer modes (shareddefs.h ObserverMode_t): CPlayer_ObserverServices.m_iObserverMode.
#define OBS_MODE_NONE       0
#define OBS_MODE_FIXED      1
#define OBS_MODE_IN_EYE     2
#define OBS_MODE_CHASE      3
#define OBS_MODE_ROAMING    4

// HUD hiding flags (shareddefs.h HIDEHUD_*): m_iHideHUD.
#define HIDEHUD_WEAPONSELECTION     (1 << 0)
#define HIDEHUD_FLASHLIGHT          (1 << 1)
#define HIDEHUD_ALL                 (1 << 2)
#define HIDEHUD_HEALTH              (1 << 3)
#define HIDEHUD_PLAYERDEAD          (1 << 4)
#define HIDEHUD_NEEDSUIT            (1 << 5)
#define HIDEHUD_MISCSTATUS          (1 << 6)
#define HIDEHUD_CHAT                (1 << 7)
#define HIDEHUD_CROSSHAIR           (1 << 8)
#define HIDEHUD_VEHICLE_CROSSHAIR   (1 << 9)
#define HIDEHUD_INVEHICLE           (1 << 10)
#define HIDEHUD_BONUS_PROGRESS      (1 << 11)
