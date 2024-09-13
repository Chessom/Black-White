cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(BlackWhite CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################



























set(PROJECT_NAME BlackWhite)

################################################################################
# Source groups
################################################################################
include_directories("include/")
set(__ataxx
    "include/ataxx/board.hpp"
    "include/ataxx/components.hpp"
    "include/ataxx/move.hpp"
    "include/ataxx/moves.hpp"
)
source_group("头文件\\ataxx" FILES ${__ataxx})

set(__bw__net
    "include/net/message.hpp"
)
source_group("头文件\\bw\\net" FILES ${__bw__net})

set(__bw__online
    "include/online/basic_online_gamer.hpp"
    "include/online/basic_user.hpp"
    "include/online/components.hpp"
    "include/online/room.hpp"
    "include/online/signals.hpp"
    "include/online/user.hpp"
    "include/online/user_info.hpp"
)
source_group("头文件\\bw\\online" FILES ${__bw__online})

set(__bw__shared
    "include/basic_Game.hpp"
    "include/basic_gamer.hpp"
    "include/config.hpp"
    "include/core.hpp"
    "include/env.hpp"
    "include/exceptions.hpp"
    "include/game.hpp"
    "include/games.hpp"
    "include/globals.hpp"
    "include/initialize.hpp"
    "include/macros.hpp"
    "include/move.hpp"
    "include/practise.hpp"
    "include/utility.hpp"
)
source_group("头文件\\bw\\shared" FILES ${__bw__shared})

set(__bw__shared__gamer
    "include/gamer/basic_lua_gamer.hpp"
    "include/gamer/basic_python_gamer.hpp"
    "include/gamer/basic_ws_gamer.hpp"
)
source_group("头文件\\bw\\shared\\gamer" FILES ${__bw__shared__gamer})

set(__bw__tui
    "include/tui/bwlocale.hpp"
    "include/tui/components.hpp"
    "include/tui/ftxui_screen.hpp"
    "include/tui/screen.hpp"
    "include/tui/scroller.hpp"
    "include/tui/settings_page.hpp"
    "include/tui/text_editor.hpp"
    "include/tui/ui.hpp"
)
source_group("头文件\\bw\\tui" FILES ${__bw__tui})

set(__bw__tui__game
    "include/tui/game/gamer_prepare.hpp"
)
source_group("头文件\\bw\\tui\\game" FILES ${__bw__tui__game})

set(__external
    "include/grid-container.hpp"
    "include/ipv4.hpp"
    "include/safe_vector.hpp"
    "include/stdafx.h"
    "include/tree.hpp"
)
source_group("头文件\\external" FILES ${__external})

set(__gobang
    "include/gobang/board.hpp"
)
source_group("头文件\\gobang" FILES ${__gobang})

set(__othello
    "include/othello.hpp"
    "include/othello/board.hpp"
    "include/othello/game.hpp"
    "include/othello/gamer.hpp"
    "include/othello/move.hpp"
    "include/othello/moves.hpp"
)
source_group("头文件\\othello" FILES ${__othello})

set(__othello__othello.ai
    "include/othello/ai.hpp"
    "include/othello/ai/alphabeta.hpp"
    "include/othello/ai/common.hpp"
    "include/othello/ai/eval_methods.hpp"
    "include/othello/ai/mcts.hpp"
    "include/othello/ai/minmax.hpp"
    "include/othello/ai/practise.hpp"
)
source_group("头文件\\othello\\othello.ai" FILES ${__othello__othello.ai})

set(__othello__othello.gamer
    "include/othello/gamer/computer.hpp"
    "include/othello/gamer/human.hpp"
    "include/othello/gamer/lua_gamer.hpp"
    "include/othello/gamer/online.hpp"
    "include/othello/gamer/python_gamer.hpp"
    "include/othello/gamer/remote_tcp.hpp"
)
source_group("头文件\\othello\\othello.gamer" FILES ${__othello__othello.gamer})

set(__othello__tui
    "include/othello/components.hpp"
    "include/othello/gamer_prepare.hpp"
)
source_group("头文件\\othello\\tui" FILES ${__othello__tui})

set(__tictactoe
    "include/tictactoe.hpp"
    "include/tictactoe/board.hpp"
    "include/tictactoe/components.hpp"
    "include/tictactoe/game.hpp"
    "include/tictactoe/gamer.hpp"
    "include/tictactoe/move.hpp"
    "include/tictactoe/moves.hpp"
)
source_group("头文件\\tictactoe" FILES ${__tictactoe})

set(__tictactoe__tictactoe.gamers
    "include/tictactoe/gamer/computer.hpp"
    "include/tictactoe/gamer/human.hpp"
    "include/tictactoe/gamer/lua_gamer.hpp"
    "include/tictactoe/gamer/online.hpp"
)
source_group("头文件\\tictactoe\\tictactoe.gamers" FILES ${__tictactoe__tictactoe.gamers})

set(__tools
    "include/tools.hpp"
)
source_group("头文件\\tools" FILES ${__tools})

set(
    "src/main.cpp"
)
source_group("源文件" FILES ${})

set(__tui
    "src/tui/grid-container.cpp"
    "src/tui/ipv4.cpp"
    "src/tui/scroller.cpp"
    "src/tui/text_editor.cpp"
)
source_group("源文件\\tui" FILES ${__tui})

set(
    "locale/zh_CN/LC_MESSAGES/zh_CN.mo"
    "locale/zh_CN/LC_MESSAGES/zh_CN.po"
    "README.md"
)
source_group("资源文件" FILES ${})

set(ALL_FILES
    ${__ataxx}
    ${__bw__net}
    ${__bw__online}
    ${__bw__shared}
    ${__bw__shared__gamer}
    ${__bw__tui}
    ${__bw__tui__game}
    ${__external}
    ${__gobang}
    ${__othello}
    ${__othello__othello.ai}
    ${__othello__othello.gamer}
    ${__othello__tui}
    ${__tictactoe}
    ${__tictactoe__tictactoe.gamers}
    ${__tools}
    ${}
    ${__tui}
    ${}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    use_props(${PROJECT_NAME} Debug   "BlackWhite.cmake")
    use_props(${PROJECT_NAME} Release "BlackWhite.cmake")
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    use_props(${PROJECT_NAME} Debug   "BlackWhite.cmake")
    use_props(${PROJECT_NAME} Release "BlackWhite.cmake")
endif()

set(ROOT_NAMESPACE BlackWhite)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebug
        >
        $<$<CONFIG:Release>:
            MultiThreaded
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "_WIN32_WINNT=0x0601;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od;
                /GR;
                /wd4293
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi-;
                /GT;
                /Gy;
                /wd4819;
                /wd4275
            >
            /permissive-;
            /MP;
            /std:c++latest;
            /Gm-;
            /sdl;
            /w;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /wd4996;
            /utf-8;
            /bigobj;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi-;
                /Gy
            >
            /permissive-;
            /MP;
            /std:c++latest;
            /Gm-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /wd4996;
            /wd4819;
            /utf-8;
            /bigobj;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
link_directories("E:\\Python3_10\\libs\\")
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "python310_d"
        ">"
        "$<$<CONFIG:Release>:"
            "python310"
        ">"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

