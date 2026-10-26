/**
 * Source2Mod SourcePawn API: game constants.
 *
 * The numbers a C++ plugin gets from s2sdk's headers (shareddefs.h, const.h,
 * gametrace.h, in_buttons.h, soundflags.h, network_connection.proto) and the
 * SDK (source2mod/consts.h), under the same names, so a value means the same
 * on both sides. All are plain #defines: the natives take them as int.
 *
 * 64-bit sets (DFLAG_*, IN_*) are bit numbers (DFLAG_*_BIT, IN_*_BIT) for the
 * two-cell form the API uses for 64-bit values (v[0] bits 0..31, v[1] bits
 * 32..63), with Bits64_* stocks to test and change them.
 *
 * The game's schema enums (CSWeaponType, gear_slot_t, loadout_slot_t, ...)
 * are generated into <source2mod/sdk/schema/enums/>.
 */

#if defined _source2mod_consts_included
 #endinput
#endif
#define _source2mod_consts_included

#include <source2mod/consts/teams>
#include <source2mod/consts/damage>
#include <source2mod/consts/entity>
#include <source2mod/consts/input>
#include <source2mod/consts/player>
#include <source2mod/consts/sound>
#include <source2mod/consts/disconnect>

/**
 * Whether bit @p bit (0..63) is set in a two-cell 64-bit value.
 */
stock bool Bits64_Has(const int value[2], int bit)
{
	return (value[bit >> 5] & (1 << (bit & 31))) != 0;
}

/**
 * Sets bit @p bit (0..63) in a two-cell 64-bit value.
 */
stock void Bits64_Add(int value[2], int bit)
{
	value[bit >> 5] |= (1 << (bit & 31));
}

/**
 * Clears bit @p bit (0..63) in a two-cell 64-bit value.
 */
stock void Bits64_Remove(int value[2], int bit)
{
	value[bit >> 5] &= ~(1 << (bit & 31));
}
