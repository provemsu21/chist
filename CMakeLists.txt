cmake_minimum_required(VERSION 3.20)
project(chist VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chist_core STATIC
    src/core/Hasher.cpp
    src/core/Analyzer.cpp
    src/core/Deduplicator.cpp
    src/core/FsWalker.cpp
)
target_include_directories(chist_core PUBLIC src/)
target_compile_options(chist_core PRIVATE
    $<$<BOOL:${APPLE}>:-Wno-deprecated-declarations>
)

add_library(chist_platform STATIC)
if(APPLE)
  target_sources(chist_platform PRIVATE src/platform/macos.cpp)
elseif(UNIX)
  target_sources(chist_platform PRIVATE src/platform/linux.cpp)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()
target_include_directories(chist_platform PUBLIC src/)

add_library(chist_system STATIC
  src/system/SystemInfo.cpp
)
target_include_directories(chist_system PUBLIC src/)

add_library(chist_cleaner STATIC
    src/cleaner/Cleaner.cpp
)
target_include_directories(chist_cleaner PUBLIC src/)
target_link_libraries(chist_cleaner PRIVATE chist_core chist_platform)

add_library(chist_cli STATIC
    src/cli/CmdClean.cpp
    src/cli/CmdDupes.cpp
    src/cli/CmdJunk.cpp
    src/cli/CmdScan.cpp
    src/cli/CmdSysinf.cpp
    src/cli/CmdTop.cpp
    src/cli/CommandRegistry.cpp)
target_include_directories(chist_cli PUBLIC src/)
target_link_libraries(chist_cli PRIVATE chist_core
                                        chist_platform
                                        chist_cleaner
                                        chist_system)

add_executable(chist src/main.cpp)


target_link_libraries(chist PRIVATE chist_core
                                    chist_platform
                                    chist_system
                                    chist_cleaner
                                    chist_cli)
