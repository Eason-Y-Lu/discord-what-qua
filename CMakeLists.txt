cmake_minimum_required(VERSION 3.23)

project(discord-what-qua-bot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Opus CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(bot src/main.cpp)

target_link_libraries(bot PRIVATE dpp::dpp OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json ZLIB::ZLIB fmt::fmt)
