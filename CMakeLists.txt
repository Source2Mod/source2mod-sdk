cmake_minimum_required(VERSION 3.18)
project(source2mod-sdk CXX)

option(SOURCE2MOD_CORE "Build core source2mod" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# =========================
# PLATFORM
# =========================

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (WIN32 AND NOT MSVC)
    message(FATAL_ERROR "MSVC restricted.")
endif()

if (LINUX)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs=libprotobuf.a")
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
endif ()

# =========================
# DEPENDENCY PATHS
# =========================

# s2sdk (alliedmodders/s2sdk, the CS2 SDK that used to be hl2sdk's cs2 branch).
# In order:
#  - SOURCESDK from deps.cmake, which tools/deps.py writes for an explicit
#    choice (the submodule, or a typed path);
#  - the first of these variables that is set -- the newer names first, so a
#    stale HL2SDKCS2 left over from the hl2sdk days does not shadow them;
#  - the vendor/s2sdk submodule.
include("${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake" OPTIONAL)

if (DEFINED SOURCESDK AND NOT "${SOURCESDK}" STREQUAL "")
    message(STATUS "s2sdk -> ${SOURCESDK} (deps.cmake)")
else()
    set(SOURCESDK "${CMAKE_CURRENT_SOURCE_DIR}/vendor/s2sdk")
    foreach (_env S2SDK_CS2 S2SDKCS2 S2SDK HL2SDK_CS2 HL2SDKCS2)
        if (DEFINED ENV{${_env}} AND NOT "$ENV{${_env}}" STREQUAL "")
            file(TO_CMAKE_PATH "$ENV{${_env}}" SOURCESDK)
            break()
        endif()
    endforeach()
    message(STATUS "s2sdk -> ${SOURCESDK}")
endif()

if (NOT EXISTS "${SOURCESDK}/CMakeLists.txt")
    message(FATAL_ERROR
            "s2sdk not found at ${SOURCESDK}, or too old to have its CMake support. Set S2SDK "
            "(or HL2SDKCS2) to your checkout, or fetch the submodule: "
            "git submodule update --init --recursive vendor/s2sdk")
endif()

# A plugin that adds this SDK with add_subdirectory() gets the same path.
get_directory_property(_sdk_has_parent PARENT_DIRECTORY)
if (_sdk_has_parent)
    set(SOURCESDK "${SOURCESDK}" PARENT_SCOPE)
endif()

# s2sdk-manifests, which gives s2sdk::s2sdk (s2sdk's include paths, defines
# and libraries) and s2sdk_protobufs(): S2SDK_MANIFESTS_DIR as a cache or
# environment variable, else the vendor/s2sdk-manifests submodule -- the
# checkout the AMBuild side (AMBuildScript, SdkPlugin.ambuild) uses too --
# else fetched.
if (NOT S2SDK_MANIFESTS_DIR)
    if (DEFINED ENV{S2SDK_MANIFESTS_DIR} AND NOT "$ENV{S2SDK_MANIFESTS_DIR}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{S2SDK_MANIFESTS_DIR}" _manifests)
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/s2sdk-manifests/s2sdk-manifests.cmake")
        set(_manifests "${CMAKE_CURRENT_SOURCE_DIR}/vendor/s2sdk-manifests")
    endif ()
    if (_manifests)
        set(S2SDK_MANIFESTS_DIR "${_manifests}" CACHE PATH "s2sdk-manifests checkout (fetched when empty)" FORCE)
    endif ()
endif ()
message(STATUS "s2sdk-manifests -> ${S2SDK_MANIFESTS_DIR}")

# =========================
# KHOOK
# =========================
#
# KHook, metamod's detour library, is what Source2Mod and its plugins hook
# with. Header-only on this side: the engine lives in metamod.so and reaches a
# plugin through SOURCE2MOD_SAVEVARS() (see ISource2ModPlugin.h), so nothing here is
# built -- the headers are all the SDK needs.
#
# They come from vendor/khook, a submodule of this repo, so a source2mod plugin
# depends on the SDK alone and not on a metamod-source checkout. The one
# exception is Source2Mod core itself (SOURCE2MOD_CORE): it is a metamod
# plugin, its engine is the KHook compiled into metamod, and it has to use
# that copy's headers so what it compiles against is what it runs on.
#
# SOURCE2MOD_KHOOK_HEADER is a hash of that copy's include/khook.hpp -- the whole
# contract between a plugin's hooks and metamod's engine; KHook changes that
# stay in its src/ do not touch it. The core answers
# SOURCE2MOD_KHOOK_HEADER_INTERFACE with its own and KHOOK_INIT() compares the
# two, so a plugin can never hook through a KHook with a different header
# (see ISource2ModPlugin.h). SOURCE2MOD_KHOOK_COMMIT, the copy's commit, is what
# plugins built before the header hash compare, and what a new plugin falls
# back to on a core that does not report the hash.

if (SOURCE2MOD_CORE AND DEFINED ENV{MMSOURCE_DEV} AND NOT "$ENV{MMSOURCE_DEV}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{MMSOURCE_DEV}" _metamod)
    set(KHOOK_DIR "${_metamod}/third_party/khook")
    message(STATUS "KHook (core build) -> ${KHOOK_DIR}")
else ()
    set(KHOOK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/khook")
endif ()

if (NOT EXISTS "${KHOOK_DIR}/include/khook.hpp")
    message(FATAL_ERROR
            "KHook headers not found at ${KHOOK_DIR}/include. "
            "Initialise the submodule: git submodule update --init --recursive")
endif ()

find_package(Git QUIET)
set(SOURCE2MOD_KHOOK_COMMIT "unknown")
if (GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            WORKING_DIRECTORY ${KHOOK_DIR}
            OUTPUT_VARIABLE _khook_commit
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _khook_rc
    )
    if (_khook_rc EQUAL 0 AND NOT "${_khook_commit}" STREQUAL "")
        set(SOURCE2MOD_KHOOK_COMMIT "${_khook_commit}")
    endif ()
endif ()
message(STATUS "KHook commit: ${SOURCE2MOD_KHOOK_COMMIT}")

# First 16 hex digits of the SHA-256 of khook.hpp with CRs dropped, so a
# CRLF checkout hashes like an LF one. SdkPlugin.ambuild computes the same.
function(source2mod_khook_header_hash header out)
    file(READ "${header}" _content)
    string(REPLACE "\r" "" _content "${_content}")
    string(SHA256 _hash "${_content}")
    string(SUBSTRING "${_hash}" 0 16 _hash)
    set(${out} "${_hash}" PARENT_SCOPE)
endfunction()

source2mod_khook_header_hash("${KHOOK_DIR}/include/khook.hpp" SOURCE2MOD_KHOOK_HEADER)
message(STATUS "KHook header: ${SOURCE2MOD_KHOOK_HEADER}")

if (SOURCE2MOD_CORE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/khook/include/khook.hpp")
    # The core compiles against metamod's copy, plugins against the submodule.
    # Their khook.hpp has to be the same, or every hooking plugin fails its
    # load check; a KHook bump that only touched src/ is fine.
    source2mod_khook_header_hash("${CMAKE_CURRENT_SOURCE_DIR}/vendor/khook/include/khook.hpp" _sdk_khook_header)
    if (NOT "${_sdk_khook_header}" STREQUAL "${SOURCE2MOD_KHOOK_HEADER}")
        message(WARNING
                "KHook header drift: metamod-source's khook.hpp (${SOURCE2MOD_KHOOK_HEADER}) "
                "differs from the SDK submodule vendor/khook's (${_sdk_khook_header}). "
                "Plugins built from this SDK that hook will refuse to load on this "
                "core until vendor/khook is bumped to a commit with metamod's header.")
    endif ()
endif ()

# =========================
# GLOBAL CONFIG
# =========================
#
# On top of what s2sdk::headers already defines for its headers (from
# s2sdk-manifests: META_IS_SOURCE2, _GLIBCXX_USE_CXX11_ABI=0 on Linux,
# _ITERATOR_DEBUG_LEVEL=0 on Windows, ...).

add_library(source2mod-config INTERFACE)

target_compile_definitions(source2mod-config INTERFACE
        $<$<PLATFORM_ID:Linux>:
        _FILE_OFFSET_BITS=64
        _snprintf=snprintf
        >

        $<$<PLATFORM_ID:Windows>:
        _WIN32 _WINDOWS
        _ALLOW_KEYWORD_MACROS
        __STDC_LIMIT_MACROS

        _CRT_SECURE_NO_WARNINGS=1
        _CRT_SECURE_NO_DEPRECATE=1
        _CRT_NONSTDC_NO_DEPRECATE=1
        >
)

target_compile_options(source2mod-config INTERFACE
        $<$<PLATFORM_ID:Linux>:
        -fPIC
        -fno-strict-aliasing
        -fno-threadsafe-statics

        -Wall
        -Wno-uninitialized
        -Wno-switch
        -Wno-unused
        -Wno-non-virtual-dtor
        -Wno-overloaded-virtual
        -Wno-conversion-null
        -Wno-write-strings
        -Wno-invalid-offsetof
        -Wno-reorder

        -mfpmath=sse
        -msse
        -fvisibility=default
        >

        $<$<PLATFORM_ID:Windows>:
        /wd4819
        /wd4828
        /wd5033
        /wd4005
        /permissive-
        /utf-8
        /MP
        >
)

target_link_options(source2mod-config INTERFACE
        $<$<PLATFORM_ID:Linux>:
        -Wl,--exclude-libs=libprotobuf.a
        >

        $<$<PLATFORM_ID:Windows>:
        $<$<CONFIG:RELEASE>:/OPT:REF>
        $<$<CONFIG:RELEASE>:/OPT:ICF>
        $<$<CONFIG:DEBUG>:/NODEFAULTLIB:libcmt>
        >
)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# =========================
# S2SDK
# =========================
#
# Its include paths, defines and libraries (the Steam API among them) as
# s2sdk::s2sdk, from s2sdk-manifests -- what SdkHelpers.configureCxx() adds on
# the AMBuild side -- and s2sdk_protobufs() for the game protobufs.

if (NOT COMMAND s2sdk_manifest)
    if (NOT S2SDK_MANIFESTS_DIR OR NOT EXISTS "${S2SDK_MANIFESTS_DIR}/s2sdk-manifests.cmake")
        include(FetchContent)
        FetchContent_Declare(s2sdk_manifests
                GIT_REPOSITORY https://github.com/Source2Mod/s2sdk-manifests.git
                GIT_TAG master
                GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(s2sdk_manifests)
        set(S2SDK_MANIFESTS_DIR "${s2sdk_manifests_SOURCE_DIR}")
    endif()
    include("${S2SDK_MANIFESTS_DIR}/s2sdk-manifests.cmake")
endif()
if (NOT TARGET s2sdk)
    s2sdk_manifest(cs2 "${SOURCESDK}")
endif()

# The game protobufs, generated from s2sdk's own protos into the Protobufs
# library Source2Mod and everything linking it share.
add_library(Protobufs STATIC)
target_link_libraries(Protobufs PUBLIC s2sdk::s2sdk)
s2sdk_protobufs(Protobufs
        common/network_connection.proto
        common/networkbasetypes.proto
        game/shared/cs/cs_gameevents.proto
        game/shared/cs/cs_usercmd.proto
        game/shared/base_gcmessages.proto
        game/shared/econ/econ_gcmessages.proto
        common/engine_gcmessages.proto
        gcsdk/gcsdk_gcmessages.proto
        gcsdk/gcsystemmsgs.proto
        game/shared/cstrike15/cstrike15_gcmessages.proto
        game/shared/cstrike15/cstrike15_usermessages.proto
        common/source2_steam_stats.proto
        common/netmessages.proto
        networksystem/networksystem_protomessages.proto
        gcsdk/steammessages.proto
        game/shared/usercmd.proto
        game/shared/usermessages.proto
        game/shared/gameevents.proto
        game/shared/clientmessages.proto
        game/shared/te.proto
        common/valveextensions.proto
)
set_target_properties(Protobufs PROPERTIES FOLDER SDK POSITION_INDEPENDENT_CODE ON)

# =========================
# SDK LIB
# =========================

file(GLOB_RECURSE SOURCE_FILES
        public/*.cpp
        public/*.h
        public/*.hpp
        src/*.cpp
        src/*.h
        src/*.hpp
)

list(APPEND SOURCE_FILES
        ${SOURCESDK}/public/tier0/memoverride.cpp
        ${SOURCESDK}/tier0_perproject/bitbuf.cpp
        ${SOURCESDK}/tier0_perproject/convar.cpp
        ${SOURCESDK}/tier0_perproject/keyvalues3.cpp
        ${SOURCESDK}/entity2/entityidentity.cpp
        ${SOURCESDK}/entity2/entitysystem.cpp
        ${SOURCESDK}/entity2/entitykeyvalues.cpp
)

# The schema helpers' code (src/impl) is the core's; a plugin gets the
# generated forwarders (src/forward) that call it (ISource2ModSchemaMethods.h,
# tools/schemamethods/generate.py).
if (SOURCE2MOD_CORE)
    list(FILTER SOURCE_FILES EXCLUDE REGEX "/src/forward/")
else()
    list(FILTER SOURCE_FILES EXCLUDE REGEX "/src/impl/")
endif()

add_library(source2mod-sdk STATIC ${SOURCE_FILES})

if (SOURCE2MOD_CORE)
    target_include_directories(source2mod-sdk PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    )
endif()

target_link_libraries(source2mod-sdk PUBLIC
        source2mod-config
)

# What Source2Mod headers reach into beyond s2sdk's own include paths.
target_include_directories(source2mod-sdk PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ${SOURCESDK}
        ${SOURCESDK}/public/schemasystem
        ${KHOOK_DIR}/include
)

# SourcePawn's public headers, for ISource2ModNatives.h. The core has the
# VM's own copy (vendor/sourcepawn/include) and must not see a second one.
if (NOT SOURCE2MOD_CORE)
    target_include_directories(source2mod-sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public/sourcepawn)
endif()

# Which KHook this binary -- and everything linking it -- was compiled
# against. Read back by SOURCE2MOD_SAVEVARS() and by the core's Source2ModFactory.
target_compile_definitions(source2mod-sdk PUBLIC
        SOURCE2MOD_KHOOK_COMMIT="${SOURCE2MOD_KHOOK_COMMIT}"
        SOURCE2MOD_KHOOK_HEADER="${SOURCE2MOD_KHOOK_HEADER}"
)

# =========================
# LINK SDK DEPENDENCIES
# =========================

target_link_libraries(source2mod-sdk PUBLIC
        s2sdk::s2sdk
        Protobufs
)

# =========================
# HELPER
# =========================

function(add_source2mod_plugin name)
    add_library(${name} SHARED ${ARGN})

    target_link_libraries(${name}
            source2mod-sdk
    )

    set_target_properties(${name} PROPERTIES
            PREFIX ""
            SUFFIX ".smx"
    )
endfunction()
