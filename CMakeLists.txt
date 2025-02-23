#SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
cmake_minimum_required(VERSION 3.23)

set(VCPKG_LIBRARY_LINKAGE static CACHE STRING "Linkage of vcpkg libraries" FORCE)

project(discord-what-qua-bot)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Opus CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include_directories(DPP/include)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Do not build shared libraries" FORCE)
add_subdirectory(DPP)

if (NOT TARGET mlspp)
    add_subdirectory(DPP/mlspp)
endif ()
if (NOT TARGET bytes)
    add_subdirectory(DPP/mlspp/lib/bytes)
endif ()
if (NOT TARGET hpke)
    add_subdirectory(DPP/mlspp/lib/hpke)
endif ()
if (NOT TARGET mls_vectors)
    add_subdirectory(DPP/mlspp/lib/mls_vectors)
endif ()
if (NOT TARGET tls_syntax)
    add_subdirectory(DPP/mlspp/lib/tls_syntax)
endif ()

link_directories(${CMAKE_BINARY_DIR}/DPP/library/mlspp)
link_directories(${CMAKE_BINARY_DIR}/DPP/library/mlspp/lib/bytes)
link_directories(${CMAKE_BINARY_DIR}/DPP/library/mlspp/lib/hpke)
link_directories(${CMAKE_BINARY_DIR}/DPP/library/mlspp/lib/mls_vectors)
link_directories(${CMAKE_BINARY_DIR}/DPP/library/mlspp/lib/tls_syntax)

if (EXISTS ${CMAKE_SOURCE_DIR}/src/token.txt)
    configure_file(src/token.txt ${CMAKE_BINARY_DIR}/token.txt COPYONLY)
endif ()

add_dependencies(dpp mlspp bytes hpke mls_vectors tls_syntax Opus::opus OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB nlohmann_json::nlohmann_json)

add_executable(bot src/main.cpp)

add_dependencies(bot dpp OpenSSL::Crypto)

target_link_libraries(bot PRIVATE dpp OpenSSL::Crypto)
