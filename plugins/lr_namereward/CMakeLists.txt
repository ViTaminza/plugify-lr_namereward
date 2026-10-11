# lr_namereward — Plugify C++ port of the Pisex LR module "[LR] Name Reward"
# (the original LR_NameReward module, Metamod plugin lr_namereward).
#
#   lr_namereward_core       engine-free core (namereward.ini model + the kill multiplier; also
#                            used by tests/namereward_tests.cpp)
#   plugify-lr_namereward    the plugin shared library (entry "bin/lr_namereward")

set(LR_NAMEREWARD_VERSION "1.0.0")

# --- core ---------------------------------------------------------------------
add_library(lr_namereward_core STATIC src/namereward_core.cpp)
target_include_directories(lr_namereward_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
# pisexcompat brings the KeyValues parser and, through its own PUBLIC link, the s2sdk binding
# globals (they stay null in the tests, which never call into s2sdk).
target_link_libraries(lr_namereward_core PUBLIC pisexcompat)
target_compile_options(lr_namereward_core PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(lr_namereward_core PUBLIC _GLIBCXX_USE_CXX11_ABI=1)

# --- generated levels_ranks bindings -------------------------------------------
# The __levels_ranks_* globals have to be compiled into every consumer plugin; plugins/levels_ranks
# builds them as the object library `lr_bindings` (it sorts before this directory), this is the
# fallback for a build of this directory alone.
if(NOT TARGET lr_bindings AND EXISTS
        "${CMAKE_SOURCE_DIR}/plugins/levels_ranks/include/external/plugify/include/levels_ranks/levels_ranks.cpp")
    add_library(lr_bindings OBJECT
            "${CMAKE_SOURCE_DIR}/plugins/levels_ranks/include/external/plugify/include/levels_ranks/levels_ranks.cpp")
    target_include_directories(lr_bindings SYSTEM PUBLIC
            "${CMAKE_SOURCE_DIR}/plugins/levels_ranks/include/external/plugify/include"
            "${CMAKE_SOURCE_DIR}/sdk/include")
    target_compile_definitions(lr_bindings PUBLIC _GLIBCXX_USE_CXX11_ABI=1)
endif()

# --- plugin -------------------------------------------------------------------
add_library(plugify-lr_namereward SHARED
        src/plugin.cpp
        $<TARGET_OBJECTS:s2sdk_bindings>
        $<TARGET_OBJECTS:lr_bindings>
)
target_include_directories(plugify-lr_namereward PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
# SYSTEM: warnings from the generated lr/plg/s2sdk headers and lr_api.hpp must not fail -Werror on
# our sources.
target_include_directories(plugify-lr_namereward SYSTEM PRIVATE
        "${CMAKE_SOURCE_DIR}/plugins/levels_ranks/include"
        "${CMAKE_SOURCE_DIR}/plugins/levels_ranks/include/external/plugify/include"
        "${CMAKE_SOURCE_DIR}/sdk/include")
target_link_libraries(plugify-lr_namereward PRIVATE
        lr_namereward_core
        -static-libstdc++ -static-libgcc
)
target_compile_options(plugify-lr_namereward PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(plugify-lr_namereward PRIVATE
        _GLIBCXX_USE_CXX11_ABI=1
        PLUGIFY_MODULE="lr_namereward"
        PLUGIFY_IS_DEBUG=0
        PLUGIFY_IS_RELEASE=1
)
set_target_properties(plugify-lr_namereward PROPERTIES OUTPUT_NAME lr_namereward)

if(UNIX AND NOT APPLE)
    target_link_options(plugify-lr_namereward PRIVATE
            "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
endif()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/lr_namereward.pplugin.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lr_namereward.pplugin"
)

# --- tests ---------------------------------------------------------------------
# Engine-free tests against the shipped namereward.ini (build/tests so ../test.sh runs them).
add_executable(lr_namereward_tests tests/namereward_tests.cpp)
target_link_libraries(lr_namereward_tests PRIVATE lr_namereward_core -static-libstdc++ -static-libgcc)
target_compile_options(lr_namereward_tests PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(lr_namereward_tests PRIVATE
        _GLIBCXX_USE_CXX11_ABI=1
        LR_NAMEREWARD_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
set_target_properties(lr_namereward_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
