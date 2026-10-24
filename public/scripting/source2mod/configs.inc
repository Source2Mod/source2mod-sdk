/**
 * Source2Mod SourcePawn API: plugin configs (ISource2ModConfigs).
 *
 * A plugin registers a config by name with its defaults as JSON text
 * (comments allowed). Source2Mod keeps the files under
 * configs/<plugin name>/: <name>.example.json is the defaults verbatim,
 * <name>.json is the admin's file, created from the defaults when it is
 * missing. What Root() reads is the admin's file merged over the defaults --
 * every key the defaults have is there -- and the callback runs again
 * whenever the file changes or "s2m config reload" re-reads it.
 *
 * The defaults are JSON text, or built key by key with Source2ModConfigDefaults
 * (below), which writes the text for you -- comments included.
 *
 * The JSON values Root() and Find() hand out belong to the current document:
 * a reload or a Set* makes a new one and the old values are gone. Read what
 * you need in the callback; do not keep the values.
 */

#if defined _source2mod_configs_included
 #endinput
#endif
#define _source2mod_configs_included

#include <source2mod/json>

/** Register flags. */
enum
{
	SOURCE2MOD_CONFIG_DEFAULT       = 0,
	SOURCE2MOD_CONFIG_REQUIRED      = 1 << 0,  /**< Register fails when <name>.json does not parse. */
	SOURCE2MOD_CONFIG_NO_WATCH      = 1 << 1,  /**< No reload when the file changes on disk. */
	SOURCE2MOD_CONFIG_NO_EXAMPLE    = 1 << 2,  /**< No <name>.example.json. */
	SOURCE2MOD_CONFIG_WRITE_MISSING = 1 << 3,  /**< Settings new in the defaults are written back (comments are lost). */
	SOURCE2MOD_CONFIG_NO_MERGE      = 1 << 4,  /**< The defaults only seed the first file. */
};

/** Why the callback runs. */
enum ConfigChange
{
	ConfigChange_Loaded = 0,     /**< The first read, from Register(). */
	ConfigChange_FileChanged,    /**< The file changed on disk. */
	ConfigChange_Command,        /**< "s2m config reload". */
	ConfigChange_Reload,         /**< Source2ModConfig.Reload(). */
};

/**
 * A new document of the config is live.
 *
 * @param config    The config.
 * @param reason    Why.
 */
typeset Source2ModConfigCallback
{
	function void (Source2ModConfig config, ConfigChange reason);
};

/**
 * One config file of this plugin. Freed with the plugin; delete it to forget
 * the config earlier (the files stay).
 */
methodmap Source2ModConfig < Handle
{
	/**
	 * Registers configs/<plugin name>/<name>.json and reads it. The callback
	 * runs once before this returns (ConfigChange_Loaded), then on every reload.
	 *
	 * @param name      File stem: letters, digits, '_', '-', '.' and '/' for a subfolder.
	 * @param defaults  The defaults as JSON text; the root must be an object.
	 * @param callback  Called with every new document; INVALID_FUNCTION for none.
	 * @param flags     SOURCE2MOD_CONFIG_* flags.
	 * @return          The config. A bad name or defaults, a duplicate, or a
	 *                  REQUIRED file that does not parse is a script error.
	 */
	public static native Source2ModConfig Register(const char[] name, const char[] defaults,
	                                               Source2ModConfigCallback callback = INVALID_FUNCTION,
	                                               int flags = SOURCE2MOD_CONFIG_DEFAULT);

	/** The admin's file merged over the defaults. */
	public native ISource2ModJSONValue Root();

	/** The defaults as registered. */
	public native ISource2ModJSONValue Defaults();

	/** The value at a JSON pointer ("/Api/Url"), null when there is none. */
	public native ISource2ModJSONValue Find(const char[] pointer);

	public native int GetName(char[] buffer, int maxlen);

	/** Absolute path of <name>.json. */
	public native int GetPath(char[] buffer, int maxlen);

	/** What the last read reported (a parse error, wrong-typed values); "" when clean. */
	public native int GetLastError(char[] buffer, int maxlen);

	/** Bumped by every reload and Set*. */
	property int Generation
	{
		public native get();
	}

	/**
	 * Changes a value in memory (objects on the way are created); Save()
	 * writes it out. The callback does not run.
	 *
	 * @return  False for a malformed pointer or one through a non-object.
	 */
	public native bool SetString(const char[] pointer, const char[] value);
	public native bool SetInt(const char[] pointer, int value);
	public native bool SetFloat(const char[] pointer, float value);
	public native bool SetBool(const char[] pointer, bool value);
	/** Any JSON value as text: an object, an array, null. */
	public native bool SetJSON(const char[] pointer, const char[] json);
	public native bool Remove(const char[] pointer);

	/** Writes <name>.json (the old file kept as <name>.json.bak; comments are lost). */
	public native bool Save(char[] error = "", int maxlen = 0);

	/** Reads the file again; the callback runs (ConfigChange_Reload) when it went live. */
	public native bool Reload(char[] error = "", int maxlen = 0);
};

/**
 * Config defaults built key by key instead of written as JSON text.
 *
 * A key is a plain member name ("RelayControl") or a JSON pointer into
 * nested objects ("/Feature/Enabled"), whose objects are created on the way.
 * Keys keep the order they were added in; adding one again replaces its
 * value. A comment, when given, goes above the key in the generated text and
 * so into <name>.example.json and the first <name>.json.
 *
 *     Source2ModConfigDefaults defaults = new Source2ModConfigDefaults();
 *     defaults.AddString("RelayControl", "127.0.0.1:27100");
 *     defaults.AddInt("RelayTimeoutMs", 300, "milliseconds");
 *     defaults.AddBool("/Feature/Enabled", true);
 *     defaults.AddJSON("Admins", "[]");
 *     g_Config = defaults.Register("config", OnConfig);
 *     delete defaults;
 */
methodmap Source2ModConfigDefaults < Handle
{
	public native Source2ModConfigDefaults();

	public native void AddString(const char[] key, const char[] value, const char[] comment = "");
	public native void AddInt(const char[] key, int value, const char[] comment = "");
	public native void AddFloat(const char[] key, float value, const char[] comment = "");
	public native void AddBool(const char[] key, bool value, const char[] comment = "");
	/** Any JSON value as text: an array, an object, null. */
	public native void AddJSON(const char[] key, const char[] json, const char[] comment = "");

	/** The defaults as JSON text, comments included. Returns the bytes written. */
	public native int ToString(char[] buffer, int maxlen);

	/**
	 * Source2ModConfig.Register() with these defaults. The defaults are copied:
	 * delete this handle afterwards.
	 */
	public native Source2ModConfig Register(const char[] name, Source2ModConfigCallback callback = INVALID_FUNCTION,
	                                        int flags = SOURCE2MOD_CONFIG_DEFAULT);
};
