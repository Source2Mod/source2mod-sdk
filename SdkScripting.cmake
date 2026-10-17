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
#   SOURCE2MOD_SPCOMP             the compiler: -DSOURCE2MOD_SPCOMP=..., else
#                                 the SPCOMP environment variable, else spcomp
#                                 on the PATH -- the way a C++ plugin finds its
#                                 compiler. A Source2Mod build or release has it
#                                 in addons/source2mod/scripting.

get_filename_component(SOURCE2MOD_SCRIPTING_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/public/scripting" ABSOLUTE)

if (NOT EXISTS "${SOURCE2MOD_SCRIPTING_INCLUDE}/source2mod/api.inc")
    message(FATAL_ERROR "The SourcePawn include set is not at ${SOURCE2MOD_SCRIPTING_INCLUDE}")
endif ()

if (NOT SOURCE2MOD_SPCOMP AND DEFINED ENV{SPCOMP} AND NOT "$ENV{SPCOMP}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{SPCOMP}" SOURCE2MOD_SPCOMP)
endif ()

if (NOT SOURCE2MOD_SPCOMP)
    find_program(SOURCE2MOD_SPCOMP NAMES spcomp spcomp64)
endif ()

if (NOT SOURCE2MOD_SPCOMP OR NOT EXISTS "${SOURCE2MOD_SPCOMP}")
    message(FATAL_ERROR "spcomp not found: put it on the PATH, set SPCOMP or pass "
                        "-DSOURCE2MOD_SPCOMP=<path> (a Source2Mod build or release has it "
                        "in addons/source2mod/scripting)")
endif ()

message(STATUS "Source2Mod scripting: ${SOURCE2MOD_SCRIPTING_INCLUDE}, ${SOURCE2MOD_SPCOMP}")
