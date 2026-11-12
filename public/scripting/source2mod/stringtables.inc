/**
 * Source2Mod SourcePawn API: network string tables, on the engine's own
 * interfaces (spawn groups are in source2mod/spawngroups).
 *
 * A NetworkStringTable is the table's id: tables are made anew for every map,
 * so find it again after a map change. INVALID_STRING_TABLE when there is none.
 */

#if defined _source2mod_stringtables_included
 #endinput
#endif
#define _source2mod_stringtables_included

#define INVALID_STRING_INDEX -1

methodmap NetworkStringTable
{
	public static native NetworkStringTable Find(const char[] name);

	/** Tables on the server; ids are 0 to this minus 1. */
	public static native int GetCount();

	/** @return Bytes written */
	public native int GetName(char[] buffer, int maxlen);

	property int NumStrings
	{
		public native get();
	}

	/** @return The index, INVALID_STRING_INDEX when it is not in the table */
	public native int FindString(const char[] value);

	/** @return Bytes written; 0 for a bad index */
	public native int GetString(int index, char[] buffer, int maxlen);

	/** @return The index, INVALID_STRING_INDEX on failure */
	public native int AddString(const char[] value, const char[] data = "", int length = 0);

	/**
	 * A string's user data, bytes as they are.
	 * @return Its whole size, even when the buffer took less
	 */
	public native int GetUserData(int index, char[] buffer, int maxlen);

	/** @return false for no such table */
	public native bool SetUserData(int index, const char[] data, int length, bool force = true);
}

#define INVALID_STRING_TABLE view_as<NetworkStringTable>(-1)
