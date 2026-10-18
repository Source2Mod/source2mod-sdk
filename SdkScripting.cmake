# Helper for building a Source2Mod SourcePawn plugin with CMake -- the script
# counterpart of add_subdirectory(${SOURCE2MOD_SDK}) for a C++ one, and the
# CMake twin of SdkPlugin.ambuild's ScriptingIncludePath() / FindSpcomp().
#
# A script plugin needs nothing built from the SDK, only its include set and a
# compiler, so it includes this file instead of adding the SDK as a
# subdirectory (which would need a C++ toolchain and s2sdk):
#
#   include(${SOURCE2MOD_SDK}/SdkScripting.cmake)
#
# and gets:
#
#   SOURCE2MOD_SCRIPTING_INCLUDE  the SDK's public/scripting, the root of
#                                 `#include <source2mod/...>`; the same files a
#                                 Source2Mod build ships in
#                                 addons/source2mod/scripting/include
#   SOURCE2MOD_SPCOMP             the compiler, the first of:
#                                 - -DSOURCE2MOD_SPCOMP=<path>
#                                 - the SPCOMP environment variable
#                                 - the SDK's own copy, bin/<linux|windows>/ next
#                                   to this file: fetched once from a Source2Mod
#                                   release (SOURCE2MOD_SPCOMP_RELEASE, "latest"
#                                   by default) and kept there; delete it, or
#                                   name another release, to fetch again;
#                                   -DSOURCE2MOD_SPCOMP_DOWNLOAD=OFF never fetches
#                                 - spcomp on the PATH (last: an old SourceMod
#                                   spcomp there must not win)

get_filename_component(SOURCE2MOD_SCRIPTING_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/public/scripting" ABSOLUTE)

if (NOT EXISTS "${SOURCE2MOD_SCRIPTING_INCLUDE}/source2mod/api.inc")
    message(FATAL_ERROR "The SourcePawn include set is not at ${SOURCE2MOD_SCRIPTING_INCLUDE}")
endif ()

set(SOURCE2MOD_SPCOMP_RELEASE "latest" CACHE STRING "Source2Mod release the SDK takes spcomp from (a tag, or latest)")
option(SOURCE2MOD_SPCOMP_DOWNLOAD "Fetch spcomp from a Source2Mod release into the SDK when it has none" ON)

# Fetches spcomp from the Source2Mod release into <dir>/<name>: its spcomp-<os>
# asset, or out of the release archive for releases from before those assets.
function(_source2mod_fetch_spcomp dir name)
    set(repo "https://github.com/Source2Mod/source2mod")
    if (WIN32)
        set(os "windows")
        set(asset "spcomp-windows.exe")
    else ()
        set(os "linux")
        set(asset "spcomp-linux")
    endif ()

    set(tag "${SOURCE2MOD_SPCOMP_RELEASE}")
    if (tag STREQUAL "latest")
        set(json "${CMAKE_CURRENT_BINARY_DIR}/source2mod-latest-release.json")
        file(DOWNLOAD "https://api.github.com/repos/Source2Mod/source2mod/releases/latest" "${json}" STATUS st)
        list(GET st 0 code)
        if (NOT code EQUAL 0)
            message(WARNING "Could not ask GitHub for the latest Source2Mod release: ${st}")
            return()
        endif ()
        file(READ "${json}" body)
        if (NOT body MATCHES "\"tag_name\" *: *\"([^\"]+)\"")
            message(WARNING "No tag in GitHub's answer for the latest Source2Mod release")
            return()
        endif ()
        set(tag "${CMAKE_MATCH_1}")
    endif ()

    message(STATUS "Source2Mod scripting: fetching spcomp from ${tag}")
    set(staging "${CMAKE_CURRENT_BINARY_DIR}/source2mod-spcomp")
    file(REMOVE_RECURSE "${staging}")
    file(DOWNLOAD "${repo}/releases/download/${tag}/${asset}" "${staging}/${name}" STATUS st)
    list(GET st 0 code)
    if (code EQUAL 0)
        set(found "${staging}/${name}")
    else ()
        # No separate asset: take it out of the release archive.
        file(REMOVE_RECURSE "${staging}")
        set(archive "${CMAKE_CURRENT_BINARY_DIR}/source2mod_${tag}-${os}.tar.gz")
        file(DOWNLOAD "${repo}/releases/download/${tag}/source2mod_${tag}-${os}.tar.gz" "${archive}" STATUS st)
        list(GET st 0 code)
        if (NOT code EQUAL 0)
            message(WARNING "Could not fetch spcomp from Source2Mod ${tag}: ${st}")
            return()
        endif ()
        file(ARCHIVE_EXTRACT INPUT "${archive}" DESTINATION "${staging}" PATTERNS "*scripting/${name}")
        file(REMOVE "${archive}")
        file(GLOB_RECURSE found "${staging}/*/${name}")
        if (NOT found)
            message(WARNING "The Source2Mod ${tag} archive has no ${name}")
            return()
        endif ()
        list(GET found 0 found)
    endif ()

    # Into the SDK, executable: release assets do not keep the bit.
    file(COPY "${found}" DESTINATION "${dir}"
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    file(REMOVE_RECURSE "${staging}")
    file(WRITE "${dir}/release.txt" "${tag}\n")
endfunction()

if (NOT SOURCE2MOD_SPCOMP AND DEFINED ENV{SPCOMP} AND NOT "$ENV{SPCOMP}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{SPCOMP}" SOURCE2MOD_SPCOMP)
endif ()

if (NOT SOURCE2MOD_SPCOMP)
    if (WIN32)
        set(_s2m_spcomp_dir "${CMAKE_CURRENT_LIST_DIR}/bin/windows")
        set(_s2m_spcomp_name "spcomp.exe")
    else ()
        set(_s2m_spcomp_dir "${CMAKE_CURRENT_LIST_DIR}/bin/linux")
        set(_s2m_spcomp_name "spcomp")
    endif ()

    # A named release other than the one fetched before replaces it.
    set(_s2m_have "")
    if (EXISTS "${_s2m_spcomp_dir}/release.txt")
        file(STRINGS "${_s2m_spcomp_dir}/release.txt" _s2m_have LIMIT_COUNT 1)
    endif ()
    if (NOT SOURCE2MOD_SPCOMP_RELEASE STREQUAL "latest" AND NOT _s2m_have STREQUAL SOURCE2MOD_SPCOMP_RELEASE)
        file(REMOVE "${_s2m_spcomp_dir}/${_s2m_spcomp_name}")
    endif ()

    if (NOT EXISTS "${_s2m_spcomp_dir}/${_s2m_spcomp_name}" AND SOURCE2MOD_SPCOMP_DOWNLOAD)
        _source2mod_fetch_spcomp("${_s2m_spcomp_dir}" "${_s2m_spcomp_name}")
    endif ()
    if (EXISTS "${_s2m_spcomp_dir}/${_s2m_spcomp_name}")
        set(SOURCE2MOD_SPCOMP "${_s2m_spcomp_dir}/${_s2m_spcomp_name}")
    endif ()
endif ()

if (NOT SOURCE2MOD_SPCOMP)
    find_program(SOURCE2MOD_SPCOMP NAMES spcomp spcomp64)
endif ()

if (NOT SOURCE2MOD_SPCOMP OR NOT EXISTS "${SOURCE2MOD_SPCOMP}")
    message(FATAL_ERROR "spcomp not found: pass -DSOURCE2MOD_SPCOMP=<path>, set SPCOMP, let the SDK "
                        "fetch it (needs network, SOURCE2MOD_SPCOMP_DOWNLOAD=ON) or put it on the PATH "
                        "(a Source2Mod build or release has it in addons/source2mod/scripting)")
endif ()

message(STATUS "Source2Mod scripting: ${SOURCE2MOD_SCRIPTING_INCLUDE}, ${SOURCE2MOD_SPCOMP}")
