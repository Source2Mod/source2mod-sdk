/**
 * Source2Mod SourcePawn API: player buttons.
 *
 * InputBitMask_t (in_buttons.h) is 64-bit: IN_*_BIT are bit numbers for the
 * two-cell button values (CInButtonState.m_pButtonStates, CUserCmd buttons,
 * GetSubtickMove), tested with Bits64_Has from <source2mod/consts>.
 */

#if defined _source2mod_consts_input_included
 #endinput
#endif
#define _source2mod_consts_input_included

#define IN_ATTACK_BIT           0
#define IN_JUMP_BIT             1
#define IN_DUCK_BIT             2
#define IN_FORWARD_BIT          3
#define IN_BACK_BIT             4
#define IN_USE_BIT              5
#define IN_TURNLEFT_BIT         7
#define IN_TURNRIGHT_BIT        8
#define IN_MOVELEFT_BIT         9
#define IN_MOVERIGHT_BIT        10
#define IN_ATTACK2_BIT          11
#define IN_RELOAD_BIT           13
#define IN_SPEED_BIT            16
#define IN_JOYAUTOSPRINT_BIT    17
#define IN_USEORRELOAD_BIT      32
#define IN_SCORE_BIT            33
#define IN_ZOOM_BIT             34
#define IN_LOOK_AT_WEAPON_BIT   35

// The low 32 bits as one-cell masks, for button values that fit in a cell.
#define IN_ATTACK           (1 << IN_ATTACK_BIT)
#define IN_JUMP             (1 << IN_JUMP_BIT)
#define IN_DUCK             (1 << IN_DUCK_BIT)
#define IN_FORWARD          (1 << IN_FORWARD_BIT)
#define IN_BACK             (1 << IN_BACK_BIT)
#define IN_USE              (1 << IN_USE_BIT)
#define IN_TURNLEFT         (1 << IN_TURNLEFT_BIT)
#define IN_TURNRIGHT        (1 << IN_TURNRIGHT_BIT)
#define IN_MOVELEFT         (1 << IN_MOVELEFT_BIT)
#define IN_MOVERIGHT        (1 << IN_MOVERIGHT_BIT)
#define IN_ATTACK2          (1 << IN_ATTACK2_BIT)
#define IN_RELOAD           (1 << IN_RELOAD_BIT)
#define IN_SPEED            (1 << IN_SPEED_BIT)
#define IN_JOYAUTOSPRINT    (1 << IN_JOYAUTOSPRINT_BIT)
