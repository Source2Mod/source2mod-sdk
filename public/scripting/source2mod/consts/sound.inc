/**
 * Source2Mod SourcePawn API: sound channels, levels and attenuations
 * (soundflags.h).
 */

#if defined _source2mod_consts_sound_included
 #endinput
#endif
#define _source2mod_consts_sound_included

#define CHAN_REPLACE        -1
#define CHAN_AUTO           0
#define CHAN_WEAPON         1
#define CHAN_VOICE          2
#define CHAN_ITEM           3
#define CHAN_BODY           4
#define CHAN_STREAM         5
#define CHAN_STATIC         6
#define CHAN_VOICE_BASE     7
#define CHAN_USER_BASE      135

#define VOL_NORM            1.0

#define ATTN_NONE           0.0
#define ATTN_NORM           0.8
#define ATTN_IDLE           2.0
#define ATTN_STATIC         1.25
#define ATTN_RICOCHET       1.5
#define ATTN_GUNFIRE        0.27

#define SNDLVL_NONE         0
#define SNDLVL_20dB         20
#define SNDLVL_25dB         25
#define SNDLVL_30dB         30
#define SNDLVL_35dB         35
#define SNDLVL_40dB         40
#define SNDLVL_45dB         45
#define SNDLVL_50dB         50
#define SNDLVL_55dB         55
#define SNDLVL_IDLE         60
#define SNDLVL_60dB         60
#define SNDLVL_65dB         65
#define SNDLVL_STATIC       66
#define SNDLVL_70dB         70
#define SNDLVL_NORM         75
#define SNDLVL_75dB         75
#define SNDLVL_80dB         80
#define SNDLVL_TALKING      80
#define SNDLVL_85dB         85
#define SNDLVL_90dB         90
#define SNDLVL_95dB         95
#define SNDLVL_100dB        100
#define SNDLVL_105dB        105
#define SNDLVL_110dB        110
#define SNDLVL_120dB        120
#define SNDLVL_130dB        130
#define SNDLVL_GUNFIRE      140
#define SNDLVL_140dB        140
#define SNDLVL_150dB        150
#define SNDLVL_180dB        180
